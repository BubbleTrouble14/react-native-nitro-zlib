#include "HybridZlibStream.hpp"
#include <zlib.h>
#include <vector>
#include <stdexcept>

namespace margelo::nitro::rnzlib
{

    HybridZlibStream::HybridZlibStream()
        : HybridZlibStreamSpec(), _zstream(nullptr), _deflate(true), _initialized(false)
    {
    }

    HybridZlibStream::HybridZlibStream(int level, bool deflate)
        : HybridZlibStreamSpec(), _deflate(deflate), _initialized(false)
    {
        initStream(level, deflate);
    }

    void HybridZlibStream::initStream(int level, bool deflate)
    {
        if (_initialized)
        {
            return;
        }

        _zstream = std::make_unique<z_stream>();
        _zstream->zalloc = Z_NULL;
        _zstream->zfree = Z_NULL;
        _zstream->opaque = Z_NULL;

        int ret;
        if (deflate)
        {
            ret = deflateInit(_zstream.get(), level);
        }
        else
        {
            ret = inflateInit(_zstream.get());
        }

        if (ret != Z_OK)
        {
            _zstream.reset();
            throw std::runtime_error("Failed to initialize zlib stream");
        }

        _initialized = true;
    }

    HybridZlibStream::~HybridZlibStream()
    {
        if (_initialized)
        {
            if (_deflate)
            {
                deflateEnd(_zstream.get());
            }
            else
            {
                inflateEnd(_zstream.get());
            }
        }
    }

    bool HybridZlibStream::write(const std::shared_ptr<ArrayBuffer> &chunk)
    {
        if (!_initialized)
        {
            initStream(Z_DEFAULT_COMPRESSION, _deflate);
        }

        if (chunk->size() == 0)
            return true;

        _zstream->avail_in = static_cast<uInt>(chunk->size());
        _zstream->next_in = static_cast<Bytef *>(chunk->data());

        std::vector<uint8_t> outBuffer(chunk->size() * 2); // Output buffer size estimation

        do
        {
            _zstream->avail_out = static_cast<uInt>(outBuffer.size());
            _zstream->next_out = outBuffer.data();

            int ret = _deflate ? deflate(_zstream.get(), Z_NO_FLUSH) : inflate(_zstream.get(), Z_NO_FLUSH);

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

        return _zstream->avail_in == 0;
    }

    void HybridZlibStream::end()
    {
        std::vector<uint8_t> outBuffer(16384); // 16KB buffer

        int ret;
        do
        {
            _zstream->avail_out = static_cast<uInt>(outBuffer.size());
            _zstream->next_out = outBuffer.data();

            ret = _deflate ? deflate(_zstream.get(), Z_FINISH) : inflate(_zstream.get(), Z_FINISH);

            unsigned have = static_cast<unsigned>(outBuffer.size()) - _zstream->avail_out;
            if (have > 0)
            {
                uint8_t *buffer = new uint8_t[have];
                std::memcpy(buffer, outBuffer.data(), have);
                auto outChunk = std::make_shared<NativeArrayBuffer>(buffer, have, [=]()
                                                                    { delete[] buffer; });
                _dataCallback(outChunk);
            }
        } while (ret != Z_STREAM_END);

        _endCallback();
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
