#include "HybridZlib.hpp"
#include <NitroModules/NitroLogger.hpp>
#include <stdexcept>
#include <vector>

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
        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = data->size();
        strm.next_in = const_cast<Bytef *>(static_cast<const Bytef *>(data->data()));

        int ret = zlibInit(&strm, windowBits);
        if (ret != Z_OK)
        {
            throw std::runtime_error("Failed to initialize zlib function");
        }

        std::vector<unsigned char> buffer;
        const size_t CHUNK = 16384;

        do
        {
            buffer.resize(buffer.size() + CHUNK);
            strm.avail_out = CHUNK;
            strm.next_out = buffer.data() + buffer.size() - CHUNK;

            ret = zlibFunc(&strm, Z_FINISH);
            if (ret == Z_STREAM_ERROR)
            {
                deflateEnd(&strm);
                throw std::runtime_error("Z_STREAM_ERROR in zlib function");
            }

            if (ret != Z_STREAM_END && ret != Z_OK)
            {
                deflateEnd(&strm);
                throw std::runtime_error("Error in zlib function: " + std::to_string(ret));
            }
        } while (strm.avail_out == 0);

        buffer.resize(buffer.size() - strm.avail_out);
        deflateEnd(&strm);

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

} // namespace margelo::nitro::rnzlib
