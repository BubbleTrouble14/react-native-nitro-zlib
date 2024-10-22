#include "HybridZlib.hpp"
#include <NitroModules/NitroLogger.hpp>
#include <stdexcept>
#include <vector>
#include "HybridZlibStream.hpp"
#include "ZlibProcessor.hpp"

namespace margelo::nitro::rnzlib
{
    std::string HybridZlib::getVersion()
    {
        return zlibVersion();
    }

    struct SafeBufferData
    {
        std::vector<uint8_t> data;

        static SafeBufferData copyFromBuffer(const std::shared_ptr<ArrayBuffer> &buffer)
        {
            SafeBufferData result;
            // This runs on JS thread
            size_t size = buffer->size();
            const uint8_t *ptr = static_cast<const uint8_t *>(buffer->data());
            result.data.assign(ptr, ptr + size);
            return result;
        }
    };

    std::shared_ptr<ArrayBuffer> HybridZlib::processZlib(
        const std::shared_ptr<ArrayBuffer> &data,
        int (*zlibFunc)(z_stream *, int),
        std::function<int(z_stream *, int)> zlibInit,
        int windowBits,
        const std::optional<ZlibOptions> &options)
    {
        Logger::log(LogLevel::Debug, "HybridZlib", "processZlib started: input size = %zu bytes", data->size());

        z_stream strm;
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
        strm.avail_in = data->size();
        strm.next_in = const_cast<Bytef *>(static_cast<const Bytef *>(data->data()));

        // Initialize with custom window bits if provided
        int actualWindowBits = getWindowBits(options, windowBits);
        int ret = zlibInit(&strm, actualWindowBits);
        if (ret != Z_OK)
        {
            Logger::log(LogLevel::Error, "HybridZlib", "Failed to initialize zlib function: ret = %d", ret);
            throw std::runtime_error("Failed to initialize zlib function");
        }

        // Get chunk size from options or use default
        const size_t CHUNK = getChunkSize(options);
        std::vector<unsigned char> buffer;
        int iterations = 0;
        const int MAX_ITERATIONS = 1000;

        // Get maxOutputLength if specified
        size_t maxOutputLength = options.has_value() && options->maxOutputLength.has_value()
                                     ? static_cast<size_t>(options->maxOutputLength.value())
                                     : SIZE_MAX;

        do
        {
            if (buffer.size() >= maxOutputLength)
            {
                deflateEnd(&strm);
                Logger::log(LogLevel::Error, "HybridZlib", "Output exceeds maxOutputLength");
                throw std::runtime_error("Output exceeds maxOutputLength");
            }

            buffer.resize(std::min(buffer.size() + CHUNK, maxOutputLength));
            strm.avail_out = CHUNK;
            strm.next_out = buffer.data() + buffer.size() - CHUNK;

            // Get flush mode from options or use default
            int flushMode = Z_FINISH;
            if (options.has_value() && options->flush.has_value())
            {
                flushMode = static_cast<int>(options->flush.value());
            }

            ret = zlibFunc(&strm, flushMode);
            Logger::log(LogLevel::Debug, "HybridZlib", "Zlib iteration %d: avail_in = %u, avail_out = %u, ret = %d",
                        iterations, strm.avail_in, strm.avail_out, ret);

            if (ret == Z_BUF_ERROR)
            {
                if (buffer.size() >= maxOutputLength)
                {
                    deflateEnd(&strm);
                    throw std::runtime_error("Buffer error: Output would exceed maxOutputLength");
                }
                continue;
            }

            if (ret == Z_STREAM_ERROR)
            {
                deflateEnd(&strm);
                Logger::log(LogLevel::Error, "HybridZlib", "Z_STREAM_ERROR in zlib function");
                throw std::runtime_error("Z_STREAM_ERROR in zlib function");
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

    // Sync Methods
    std::shared_ptr<ArrayBuffer> HybridZlib::inflateSync(const std::shared_ptr<ArrayBuffer> &data, const std::optional<ZlibOptions> &options)
    {
        return processZlib(data, ::inflate, [options](z_streamp strm, int windowBits)
                           { return inflateInit2(strm, windowBits); }, 15, options);
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::inflateRawSync(const std::shared_ptr<ArrayBuffer> &data, const std::optional<ZlibOptions> &options)
    {
        return processZlib(data, ::inflate, [options](z_streamp strm, int windowBits)
                           { return inflateInit2(strm, -windowBits); }, 15, options);
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::compressSync(const std::shared_ptr<ArrayBuffer> &data, const std::optional<ZlibOptions> &options)
    {
        int compressionLevel = getCompressionLevel(options);
        return processZlib(data, ::deflate, [compressionLevel](z_streamp strm, int windowBits)
                           { return deflateInit(strm, compressionLevel); }, 15, options);
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::deflateSync(const std::shared_ptr<ArrayBuffer> &data, const std::optional<ZlibOptions> &options)
    {
        int compressionLevel = getCompressionLevel(options);
        int memLevel = getMemLevel(options);
        int strategy = getStrategy(options);

        return processZlib(data, ::deflate, [compressionLevel, memLevel, strategy](z_streamp strm, int windowBits)
                           { return deflateInit2(strm, compressionLevel, Z_DEFLATED, windowBits, memLevel, strategy); }, 15, options);
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::deflateRawSync(const std::shared_ptr<ArrayBuffer> &data, const std::optional<ZlibOptions> &options)
    {
        int compressionLevel = getCompressionLevel(options);
        int memLevel = getMemLevel(options);
        int strategy = getStrategy(options);

        return processZlib(data, ::deflate, [compressionLevel, memLevel, strategy](z_streamp strm, int windowBits)
                           { return deflateInit2(strm, compressionLevel, Z_DEFLATED, -windowBits, memLevel, strategy); }, 15, options);
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::gzipSync(const std::shared_ptr<ArrayBuffer> &data, const std::optional<ZlibOptions> &options)
    {
        int compressionLevel = getCompressionLevel(options);
        int memLevel = getMemLevel(options);
        int strategy = getStrategy(options);

        return processZlib(data, ::deflate, [compressionLevel, memLevel, strategy](z_streamp strm, int windowBits)
                           { return deflateInit2(strm, compressionLevel, Z_DEFLATED, windowBits | 16, memLevel, strategy); }, 15, options);
    }

    std::shared_ptr<ArrayBuffer> HybridZlib::gunzipSync(const std::shared_ptr<ArrayBuffer> &data, const std::optional<ZlibOptions> &options)
    {
        return processZlib(data, ::inflate, [](z_streamp strm, int windowBits)
                           { return inflateInit2(strm, windowBits | 16); }, 15, options);
    }

    // Async Methods
    std::future<std::shared_ptr<ArrayBuffer>> HybridZlib::inflate(
        const std::shared_ptr<ArrayBuffer> &data,
        const std::optional<ZlibOptions> &options)
    {
        auto processor = std::make_shared<ZlibProcessor>(data);
        return std::async(std::launch::async, [processor, options]()
                          { return processor->process(
                                ::inflate,
                                [](z_streamp strm, int windowBits)
                                {
                                    return inflateInit2(strm, windowBits);
                                },
                                15,
                                options); });
    }

    std::future<std::shared_ptr<ArrayBuffer>> HybridZlib::inflateRaw(
        const std::shared_ptr<ArrayBuffer> &data,
        const std::optional<ZlibOptions> &options)
    {
        auto processor = std::make_shared<ZlibProcessor>(data);
        return std::async(std::launch::async, [processor, options]()
                          { return processor->process(
                                ::inflate,
                                [](z_streamp strm, int windowBits)
                                {
                                    return inflateInit2(strm, -windowBits);
                                },
                                15,
                                options); });
    }

    std::future<std::shared_ptr<ArrayBuffer>> HybridZlib::compress(
        const std::shared_ptr<ArrayBuffer> &data,
        const std::optional<ZlibOptions> &options)
    {
        auto processor = std::make_shared<ZlibProcessor>(data);
        int compressionLevel = getCompressionLevel(options);

        return std::async(std::launch::async, [processor, compressionLevel, options]()
                          { return processor->process(
                                ::deflate,
                                [compressionLevel](z_streamp strm, int windowBits)
                                {
                                    return deflateInit(strm, compressionLevel);
                                },
                                15,
                                options); });
    }

    std::future<std::shared_ptr<ArrayBuffer>> HybridZlib::deflate(
        const std::shared_ptr<ArrayBuffer> &data,
        const std::optional<ZlibOptions> &options)
    {
        auto processor = std::make_shared<ZlibProcessor>(data);
        int compressionLevel = getCompressionLevel(options);
        int memLevel = getMemLevel(options);
        int strategy = getStrategy(options);

        return std::async(std::launch::async, [processor, compressionLevel, memLevel, strategy, options]()
                          { return processor->process(
                                ::deflate,
                                [compressionLevel, memLevel, strategy](z_streamp strm, int windowBits)
                                {
                                    return deflateInit2(strm, compressionLevel, Z_DEFLATED, windowBits,
                                                        memLevel, strategy);
                                },
                                15,
                                options); });
    }

    std::future<std::shared_ptr<ArrayBuffer>> HybridZlib::deflateRaw(
        const std::shared_ptr<ArrayBuffer> &data,
        const std::optional<ZlibOptions> &options)
    {
        auto processor = std::make_shared<ZlibProcessor>(data);
        int compressionLevel = getCompressionLevel(options);
        int memLevel = getMemLevel(options);
        int strategy = getStrategy(options);

        return std::async(std::launch::async, [processor, compressionLevel, memLevel, strategy, options]()
                          { return processor->process(
                                ::deflate,
                                [compressionLevel, memLevel, strategy](z_streamp strm, int windowBits)
                                {
                                    return deflateInit2(strm, compressionLevel, Z_DEFLATED, -windowBits,
                                                        memLevel, strategy);
                                },
                                15,
                                options); });
    }

    std::future<std::shared_ptr<ArrayBuffer>> HybridZlib::gzip(
        const std::shared_ptr<ArrayBuffer> &data,
        const std::optional<ZlibOptions> &options)
    {
        auto processor = std::make_shared<ZlibProcessor>(data);
        int compressionLevel = getCompressionLevel(options);
        int memLevel = getMemLevel(options);
        int strategy = getStrategy(options);

        return std::async(std::launch::async, [processor, compressionLevel, memLevel, strategy, options]()
                          { return processor->process(
                                ::deflate,
                                [compressionLevel, memLevel, strategy](z_streamp strm, int windowBits)
                                {
                                    return deflateInit2(strm, compressionLevel, Z_DEFLATED, windowBits | 16,
                                                        memLevel, strategy);
                                },
                                15,
                                options); });
    }

    std::future<std::shared_ptr<ArrayBuffer>> HybridZlib::gunzip(
        const std::shared_ptr<ArrayBuffer> &data,
        const std::optional<ZlibOptions> &options)
    {
        auto processor = std::make_shared<ZlibProcessor>(data);
        return std::async(std::launch::async, [processor, options]()
                          { return processor->process(
                                ::inflate,
                                [](z_streamp strm, int windowBits)
                                {
                                    return inflateInit2(strm, windowBits | 16);
                                },
                                15,
                                options); });
    }

    // Streams
    std::shared_ptr<HybridZlibStreamSpec> HybridZlib::createDeflateStream(const std::optional<ZlibOptions> &options)
    {
        int compressionLevel = getCompressionLevel(options);
        int strategy = getStrategy(options);

        Logger::log(LogLevel::Debug, "HybridZlib", "Creating deflate stream: level = %d, strategy = %d",
                    compressionLevel, strategy);

        auto stream = HybridZlibStream::create(compressionLevel, true);
        stream->params(compressionLevel, strategy);
        return stream;
    }

    std::shared_ptr<HybridZlibStreamSpec> HybridZlib::createInflateStream(const std::optional<ZlibOptions> &options)
    {
        Logger::log(LogLevel::Debug, "HybridZlib", "Creating inflate stream");
        return HybridZlibStream::create(Z_DEFAULT_COMPRESSION, false);
    }

    std::shared_ptr<HybridZlibStreamSpec> HybridZlib::createGzipStream(const std::optional<ZlibOptions> &options)
    {
        int compressionLevel = getCompressionLevel(options);
        Logger::log(LogLevel::Debug, "HybridZlib", "Creating gzip stream: level = %d", compressionLevel);

        auto stream = HybridZlibStream::create(compressionLevel, true);
        stream->params(compressionLevel, Z_DEFAULT_STRATEGY);
        return stream;
    }

    std::shared_ptr<HybridZlibStreamSpec> HybridZlib::createGunzipStream(const std::optional<ZlibOptions> &options)
    {
        Logger::log(LogLevel::Debug, "HybridZlib", "Creating gunzip stream");
        return HybridZlibStream::create(Z_DEFAULT_COMPRESSION, false);
    }

    std::shared_ptr<HybridZlibStreamSpec> HybridZlib::createDeflateRawStream(const std::optional<ZlibOptions> &options)
    {
        int compressionLevel = getCompressionLevel(options);
        int strategy = getStrategy(options);

        Logger::log(LogLevel::Debug, "HybridZlib", "Creating raw deflate stream: level = %d, strategy = %d",
                    compressionLevel, strategy);

        auto stream = HybridZlibStream::create(compressionLevel, true);
        stream->params(compressionLevel, strategy);
        return stream;
    }

    std::shared_ptr<HybridZlibStreamSpec> HybridZlib::createInflateRawStream(const std::optional<ZlibOptions> &options)
    {
        Logger::log(LogLevel::Debug, "HybridZlib", "Creating raw inflate stream");
        return HybridZlibStream::create(Z_DEFAULT_COMPRESSION, false);
    }

    std::shared_ptr<HybridZlibStreamSpec> HybridZlib::createUnzipStream(const std::optional<ZlibOptions> &options)
    {
        Logger::log(LogLevel::Debug, "HybridZlib", "Creating unzip stream");
        return HybridZlibStream::create(Z_DEFAULT_COMPRESSION, false);
    }

} // namespace margelo::nitro::rnzlib