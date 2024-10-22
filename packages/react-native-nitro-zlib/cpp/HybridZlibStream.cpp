#include "HybridZlibStream.hpp"
#include <zlib.h>
#include <vector>
#include <stdexcept>
#include <NitroModules/NitroLogger.hpp>

namespace margelo::nitro::rnzlib
{

    void HybridZlibStream::initStream(int level, bool deflate)
    {
        Logger::log(LogLevel::Debug, "HybridZlibStream", "Initializing stream: level = %d, deflate = %d", level, deflate);

        if (_initialized)
        {
            Logger::log(LogLevel::Debug, "HybridZlibStream", "Stream already initialized, resetting");
            if (_zstream)
            {
                if (_deflate)
                    deflateEnd(_zstream.get());
                else
                    inflateEnd(_zstream.get());
            }
        }

        _zstream = std::make_unique<z_stream>();
        memset(_zstream.get(), 0, sizeof(z_stream));

        int ret = deflate ? deflateInit(_zstream.get(), level) : inflateInit(_zstream.get());
        if (ret != Z_OK)
        {
            Logger::log(LogLevel::Error, "HybridZlibStream", "Failed to initialize zlib stream: %d", ret);
            _zstream.reset();
            reportError("Failed to initialize zlib stream");
            _initialized = false;
            return;
        }

        _initialized = true;
        _deflate = deflate;
        Logger::log(LogLevel::Debug, "HybridZlibStream", "Stream initialized successfully. Initialized: %d, Deflate: %d", _initialized, _deflate);
    }

    void HybridZlibStream::initWithWindowBits(int windowBits)
    {
        if (_initialized)
        {
            if (_zstream)
            {
                if (_deflate)
                    deflateEnd(_zstream.get());
                else
                    inflateEnd(_zstream.get());
            }
            _initialized = false;
        }

        _zstream = std::make_unique<z_stream>();
        memset(_zstream.get(), 0, sizeof(z_stream));
    }

    void HybridZlibStream::initDeflateWithWindowBits(int level, int windowBits)
    {
        initWithWindowBits(windowBits);

        int ret = deflateInit2(_zstream.get(), level, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY);
        if (ret != Z_OK)
        {
            Logger::log(LogLevel::Error, "HybridZlibStream", "Failed to initialize deflate stream with window bits: %d", ret);
            _zstream.reset();
            reportError("Failed to initialize deflate stream");
            return;
        }

        _initialized = true;
        _deflate = true;
    }

    void HybridZlibStream::initInflateWithWindowBits(int windowBits)
    {
        initWithWindowBits(windowBits);

        int ret = inflateInit2(_zstream.get(), windowBits);
        if (ret != Z_OK)
        {
            Logger::log(LogLevel::Error, "HybridZlibStream", "Failed to initialize inflate stream with window bits: %d", ret);
            _zstream.reset();
            reportError("Failed to initialize inflate stream");
            return;
        }

        _initialized = true;
        _deflate = false;
    }

    void HybridZlibStream::reportError(const std::string &message)
    {
        if (_errorCallback)
        {
            Error error{"ZlibError", message, std::nullopt};
            _errorCallback(error);
        }
    }

    bool HybridZlibStream::write(const std::shared_ptr<ArrayBuffer> &chunk)
    {
        Logger::log(LogLevel::Debug, "HybridZlibStream", "Write called. Initialized: %d, Deflate: %d", _initialized, _deflate);

        if (!_initialized || !_zstream)
        {
            reportError("Stream not initialized");
            return false;
        }

        if (!chunk || chunk->size() == 0)
        {
            Logger::log(LogLevel::Debug, "HybridZlibStream", "Empty chunk, returning");
            return true;
        }

        Logger::log(LogLevel::Debug, "HybridZlibStream", "Writing chunk of size: %zu", chunk->size());

        _zstream->avail_in = static_cast<uInt>(chunk->size());
        _zstream->next_in = static_cast<Bytef *>(chunk->data());

        do
        {
            _outBuffer.resize(CHUNK_SIZE);
            _zstream->avail_out = static_cast<uInt>(_outBuffer.size());
            _zstream->next_out = _outBuffer.data();

            int ret = _deflate ? deflate(_zstream.get(), Z_NO_FLUSH) : inflate(_zstream.get(), Z_NO_FLUSH);

            if (ret == Z_STREAM_ERROR)
            {
                throw std::runtime_error("Z_STREAM_ERROR: inconsistent stream state");
            }

            if (ret == Z_BUF_ERROR && _zstream->avail_in == 0)
            {
                // If Z_BUF_ERROR occurs and there's no input left, it might mean we need more output space.
                break;
            }

            unsigned have = static_cast<unsigned>(_outBuffer.size()) - _zstream->avail_out;
            if (have > 0 && _dataCallback)
            {
                uint8_t *buffer = new uint8_t[have];
                std::memcpy(buffer, _outBuffer.data(), have);
                auto outChunk = std::make_shared<NativeArrayBuffer>(buffer, have, [=]()
                                                                    { delete[] buffer; });
                _dataCallback(outChunk);
            }
        } while (_zstream->avail_out == 0);

        return _zstream->avail_in == 0;
    }

    void HybridZlibStream::processOutput(int ret)
    {
        if (ret == Z_STREAM_ERROR)
        {
            reportError("Z_STREAM_ERROR: inconsistent stream state");
            return;
        }

        unsigned have = static_cast<unsigned>(_outBuffer.size()) - _zstream->avail_out;
        if (have > 0 && _dataCallback)
        {
            auto outChunk = std::make_shared<NativeArrayBuffer>(_outBuffer.data(), have, [=]()
                                                                { delete[] _outBuffer.data(); });
            _dataCallback(outChunk);
        }
    }

    void HybridZlibStream::end()
    {

        Logger::log(LogLevel::Debug, "HybridZlibStream", "End called. Initialized: %d, Deflate: %d", _initialized, _deflate);

        if (!_initialized || !_zstream)
        {
            reportError("Stream not initialized or already ended");
            return;
        }

        _outBuffer.resize(CHUNK_SIZE);
        int ret;
        do
        {
            _zstream->avail_out = static_cast<uInt>(_outBuffer.size());
            _zstream->next_out = _outBuffer.data();

            ret = _deflate ? deflate(_zstream.get(), Z_FINISH) : inflate(_zstream.get(), Z_FINISH);

            if (ret == Z_STREAM_ERROR)
            {
                reportError("Z_STREAM_ERROR: inconsistent stream state during end()");
                break;
            }

            unsigned have = static_cast<unsigned>(_outBuffer.size()) - _zstream->avail_out;
            if (have > 0 && _dataCallback)
            {
                auto outChunk = std::make_shared<NativeArrayBuffer>(_outBuffer.data(), have, [=]() { /* No need to delete _outBuffer, it's reused */ });
                _dataCallback(outChunk);
            }
        } while (ret != Z_STREAM_END);

        // Clean up
        if (_deflate)
        {
            deflateEnd(_zstream.get());
        }
        else
        {
            inflateEnd(_zstream.get());
        }

        _zstream.reset();
        _initialized = false;

        if (_endCallback)
        {
            _endCallback();
        }

        Logger::log(LogLevel::Debug, "HybridZlibStream", "Stream ended successfully");
    }

    void HybridZlibStream::flush(std::optional<double> kind)
    {
        int flushKind = kind.has_value() ? static_cast<int>(kind.value()) : Z_SYNC_FLUSH;
        std::vector<uint8_t> outBuffer(16384); // 16KB buffer

        do
        {
            _zstream->avail_out = static_cast<uInt>(outBuffer.size());
            _zstream->next_out = outBuffer.data();

            int ret = _deflate ? deflate(_zstream.get(), flushKind) : inflate(_zstream.get(), flushKind);

            if (ret == Z_STREAM_ERROR)
            {
                throw std::runtime_error("Z_STREAM_ERROR: inconsistent stream state");
            }

            unsigned have = static_cast<unsigned>(outBuffer.size()) - _zstream->avail_out;
            if (have > 0)
            {
                uint8_t *buffer = new uint8_t[have];
                std::memcpy(buffer, outBuffer.data(), have);
                auto outChunk = std::make_shared<NativeArrayBuffer>(buffer, have, [=]()
                                                                    { delete[] buffer; });
                _dataCallback(outChunk);
            }
        } while (_zstream->avail_out == 0);
    }

    void HybridZlibStream::onData(const std::function<void(const std::shared_ptr<ArrayBuffer> &chunk)> &callback)
    {
        _dataCallback = callback;
    }

    void HybridZlibStream::onEnd(const std::function<void()> &callback)
    {
        _endCallback = callback;
    }

    void HybridZlibStream::onError(const std::function<void(const Error &error)> &callback)
    {
        _errorCallback = callback;
    }

    void HybridZlibStream::params(double level, double strategy)
    {
        if (!_deflate)
        {
            throw std::runtime_error("params() is only supported for deflate streams");
        }

        int ret = deflateParams(_zstream.get(), static_cast<int>(level), static_cast<int>(strategy));
        if (ret != Z_OK)
        {
            Error error{"ZlibError", "Failed to set parameters", std::nullopt};
            _errorCallback(error);
        }
    }

    void HybridZlibStream::reset()
    {
        int ret;
        if (_deflate)
        {
            ret = deflateReset(_zstream.get());
        }
        else
        {
            ret = inflateReset(_zstream.get());
        }

        if (ret != Z_OK)
        {
            Error error{"ZlibError", "Failed to reset stream", std::nullopt};
            _errorCallback(error);
        }
    }

    std::shared_ptr<ArrayBuffer> HybridZlibStream::createArrayBuffer(size_t size)
    {
        uint8_t *buffer = new uint8_t[size];
        return std::make_shared<NativeArrayBuffer>(buffer, size, [=]()
                                                   { delete[] buffer; });
    }

    double HybridZlibStream::getMemorySize()
    {
        return static_cast<double>(_zstream->total_in + _zstream->total_out);
    }

} // namespace margelo::nitro::rnzlib
