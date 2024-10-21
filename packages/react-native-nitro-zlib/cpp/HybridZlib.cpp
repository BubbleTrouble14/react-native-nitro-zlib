#include "HybridZlib.hpp"
#include <NitroModules/NitroLogger.hpp>
#include <stdexcept>
#include <vector>
#include "HybridZlibStream.hpp"

namespace margelo::nitro::rnzlib
{
    std::string HybridZlib::getVersion()
    {
        return zlibVersion();
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::processZlib(
        const std::shared_ptr<ArrayBuffer> &data,
        int (*zlibFunc)(z_stream *, int),
        std::function<int(z_stream *, int)> zlibInit,
        int windowBits)
    {
        Logger::log(LogLevel::Debug, "HybridZlib", "processZlib started: input size = %zu bytes", data->size());

        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = data->size();
        strm.next_in = const_cast<Bytef *>(static_cast<const Bytef *>(data->data()));

        int ret = zlibInit(&strm, windowBits);
        if (ret != Z_OK)
        {
            Logger::log(LogLevel::Error, "HybridZlib", "Failed to initialize zlib function: ret = %d", ret);
            throw std::runtime_error("Failed to initialize zlib function");
        }

        std::vector<unsigned char> buffer;
        const size_t INITIAL_CHUNK = 256 * 1024; // Start with a larger initial chunk (256KB)
        size_t CHUNK = INITIAL_CHUNK;
        int iterations = 0;
        const int MAX_ITERATIONS = 1000;

        do
        {
            buffer.resize(buffer.size() + CHUNK);
            strm.avail_out = CHUNK;
            strm.next_out = buffer.data() + buffer.size() - CHUNK;

            ret = zlibFunc(&strm, Z_FINISH);
            Logger::log(LogLevel::Debug, "HybridZlib", "Zlib iteration %d: avail_in = %u, avail_out = %u, ret = %d",
                        iterations, strm.avail_in, strm.avail_out, ret);

            if (ret == Z_BUF_ERROR)
            {
                // Instead of throwing an error, increase the buffer size and continue
                CHUNK *= 2; // Double the chunk size
                Logger::log(LogLevel::Warning, "HybridZlib", "Z_BUF_ERROR: Increasing chunk size to %zu bytes", CHUNK);
                continue;
            }

            if (ret == Z_STREAM_ERROR)
            {
                deflateEnd(&strm);
                Logger::log(LogLevel::Error, "HybridZlib", "Z_STREAM_ERROR in zlib function");
                throw std::runtime_error("Z_STREAM_ERROR in zlib function");
            }

            if (ret != Z_STREAM_END && ret != Z_OK)
            {
                deflateEnd(&strm);
                Logger::log(LogLevel::Error, "HybridZlib", "Error in zlib function: %d", ret);
                throw std::runtime_error("Error in zlib function: " + std::to_string(ret));
            }

            iterations++;
            if (iterations > MAX_ITERATIONS)
            {
                deflateEnd(&strm);
                Logger::log(LogLevel::Error, "HybridZlib", "Maximum iteration count exceeded");
                throw std::runtime_error("Maximum iteration count exceeded");
            }
        } while (ret != Z_STREAM_END);

        buffer.resize(buffer.size() - strm.avail_out);
        deflateEnd(&strm);

        Logger::log(LogLevel::Debug, "HybridZlib", "processZlib completed: output size = %zu bytes", buffer.size());

        uint8_t *bufferData = new uint8_t[buffer.size()];
        std::memcpy(bufferData, buffer.data(), buffer.size());
        return std::make_shared<NativeArrayBuffer>(bufferData, buffer.size(), [=]()
                                                   { delete[] bufferData; });
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::inflate(const std::shared_ptr<ArrayBuffer> &data, std::optional<FlushMode> flush)
    {
        return processZlib(data, ::inflate, [](z_streamp strm, int windowBits)
                           { return inflateInit2(strm, windowBits); }, 15);
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::inflateRaw(const std::shared_ptr<ArrayBuffer> &data, std::optional<FlushMode> flush)
    {
        return processZlib(data, ::inflate, [](z_streamp strm, int windowBits)
                           { return inflateInit2(strm, -windowBits); }, 15);
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::deflate(const std::shared_ptr<ArrayBuffer> &data, std::optional<double> level, std::optional<FlushMode> flush)
    {
        int compressionLevel = level.has_value() ? static_cast<int>(level.value()) : Z_DEFAULT_COMPRESSION;
        return processZlib(data, ::deflate, [compressionLevel](z_streamp strm, int windowBits)
                           { return deflateInit2(strm, compressionLevel, Z_DEFLATED, windowBits, 8, Z_DEFAULT_STRATEGY); }, 15);
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::deflateRaw(const std::shared_ptr<ArrayBuffer> &data, std::optional<double> level, std::optional<FlushMode> flush)
    {
        int compressionLevel = level.has_value() ? static_cast<int>(level.value()) : Z_DEFAULT_COMPRESSION;
        return processZlib(data, ::deflate, [compressionLevel](z_streamp strm, int windowBits)
                           { return deflateInit2(strm, compressionLevel, Z_DEFLATED, -windowBits, 8, Z_DEFAULT_STRATEGY); }, 15);
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::gzip(const std::shared_ptr<ArrayBuffer> &data, std::optional<double> level)
    {
        int compressionLevel = level.has_value() ? static_cast<int>(level.value()) : Z_DEFAULT_COMPRESSION;
        return processZlib(data, ::deflate, [compressionLevel](z_streamp strm, int windowBits)
                           { return deflateInit2(strm, compressionLevel, Z_DEFLATED, windowBits | 16, 8, Z_DEFAULT_STRATEGY); }, 15);
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::gunzip(const std::shared_ptr<ArrayBuffer> &data)
    {
        return processZlib(data, ::inflate, [](z_streamp strm, int windowBits)
                           { return inflateInit2(strm, windowBits | 16); }, 15);
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::compress(const std::shared_ptr<ArrayBuffer> &data, std::optional<double> level)
    {
        int compressionLevel = level.has_value() ? static_cast<int>(level.value()) : Z_DEFAULT_COMPRESSION;
        return processZlib(data, ::deflate, [compressionLevel](z_streamp strm, int windowBits)
                           { return deflateInit(strm, compressionLevel); }, 15);
    }

    std::shared_ptr<HybridZlibStreamSpec> HybridZlib::createDeflateStream(std::optional<double> level, std::optional<double> strategy)
    {
        int compressionLevel = level.has_value() ? static_cast<int>(level.value()) : Z_DEFAULT_COMPRESSION;
        int compressionStrategy = strategy.has_value() ? static_cast<int>(strategy.value()) : Z_DEFAULT_STRATEGY;

        Logger::log(LogLevel::Debug, "HybridZlib", "Creating deflate stream: level = %d, strategy = %d", compressionLevel, compressionStrategy);

        auto stream = HybridZlibStream::create(compressionLevel, true);

        if (strategy.has_value())
        {
            stream->params(compressionLevel, compressionStrategy);
        }

        return stream;
    }

    std::shared_ptr<HybridZlibStreamSpec> HybridZlib::createInflateStream()
    {
        Logger::log(LogLevel::Debug, "HybridZlib", "Creating inflate stream");

        return HybridZlibStream::create(Z_DEFAULT_COMPRESSION, false);
    }

} // namespace margelo::nitro::rnzlib
