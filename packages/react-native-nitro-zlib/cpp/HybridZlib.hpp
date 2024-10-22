#pragma once

#include <zlib.h>
#include "HybridZlibSpec.hpp"
#include <functional>
#include <memory>
#include <optional>
#include <future>

namespace margelo::nitro::rnzlib
{
    using namespace facebook;

    class HybridZlib : public virtual HybridZlibSpec
    {
    public:
        HybridZlib() : HybridObject(TAG) {}

    public:
        std::string getVersion() override;

        // Sync methods
        std::shared_ptr<ArrayBuffer> inflateSync(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::shared_ptr<ArrayBuffer> inflateRawSync(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::shared_ptr<ArrayBuffer> compressSync(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::shared_ptr<ArrayBuffer> deflateSync(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::shared_ptr<ArrayBuffer> deflateRawSync(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::shared_ptr<ArrayBuffer> gzipSync(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::shared_ptr<ArrayBuffer> gunzipSync(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        // Async methods
        std::future<std::shared_ptr<ArrayBuffer>> inflate(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::future<std::shared_ptr<ArrayBuffer>> inflateRaw(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::future<std::shared_ptr<ArrayBuffer>> compress(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::future<std::shared_ptr<ArrayBuffer>> deflate(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::future<std::shared_ptr<ArrayBuffer>> deflateRaw(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::future<std::shared_ptr<ArrayBuffer>> gzip(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::future<std::shared_ptr<ArrayBuffer>> gunzip(
            const std::shared_ptr<ArrayBuffer> &data,
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        // Stream methods (unchanged)
        std::shared_ptr<HybridZlibStreamSpec> createDeflateStream(
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::shared_ptr<HybridZlibStreamSpec> createInflateStream(
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::shared_ptr<HybridZlibStreamSpec> createGzipStream(
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::shared_ptr<HybridZlibStreamSpec> createGunzipStream(
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::shared_ptr<HybridZlibStreamSpec> createDeflateRawStream(
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::shared_ptr<HybridZlibStreamSpec> createInflateRawStream(
            const std::optional<ZlibOptions> &options = std::nullopt) override;

        std::shared_ptr<HybridZlibStreamSpec> createUnzipStream(
            const std::optional<ZlibOptions> &options = std::nullopt) override;

    private:
        // Helper function with additional options parameter
        std::shared_ptr<ArrayBuffer> processZlib(
            const std::shared_ptr<ArrayBuffer> &data,
            int (*zlibFunc)(z_stream *, int),
            std::function<int(z_stream *, int)> zlibInit,
            int windowBits,
            const std::optional<ZlibOptions> &options = std::nullopt);

        // std::vector<uint8_t> copyBufferData(const std::shared_ptr<ArrayBuffer> &buffer)
        // {
        //     if (!buffer)
        //     {
        //         throw std::runtime_error("Null buffer provided");
        //     }
        //     // Access size and data on the JS thread
        //     size_t size = buffer->size();
        //     const uint8_t *data = static_cast<const uint8_t *>(buffer->data());

        //     // Create a copy of the data
        //     return std::vector<uint8_t>(data, data + size);
        // }

        // // Helper function to create ArrayBuffer from vector
        // std::shared_ptr<ArrayBuffer> createArrayBuffer(const std::vector<uint8_t> &data)
        // {
        //     uint8_t *bufferData = new uint8_t[data.size()];
        //     std::memcpy(bufferData, data.data(), data.size());
        //     return std::make_shared<NativeArrayBuffer>(
        //         bufferData,
        //         data.size(),
        //         [=]()
        //         { delete[] bufferData; });
        // }

        // Helper to extract values from ZlibOptions with defaults
        static int getCompressionLevel(const std::optional<ZlibOptions> &options)
        {
            if (!options.has_value() || !options->level.has_value())
            {
                return Z_DEFAULT_COMPRESSION;
            }
            return static_cast<int>(options->level.value());
        }

        static int getStrategy(const std::optional<ZlibOptions> &options)
        {
            if (!options.has_value() || !options->strategy.has_value())
            {
                return Z_DEFAULT_STRATEGY;
            }
            return static_cast<int>(options->strategy.value());
        }

        static int getWindowBits(const std::optional<ZlibOptions> &options, int defaultBits = 15)
        {
            if (!options.has_value() || !options->windowBits.has_value())
            {
                return defaultBits;
            }
            return static_cast<int>(options->windowBits.value());
        }

        static int getMemLevel(const std::optional<ZlibOptions> &options)
        {
            if (!options.has_value() || !options->memLevel.has_value())
            {
                return 8; // Default memory level
            }
            return static_cast<int>(options->memLevel.value());
        }

        static int getChunkSize(const std::optional<ZlibOptions> &options)
        {
            if (!options.has_value() || !options->chunkSize.has_value())
            {
                return 16 * 1024; // Default chunk size (16KB)
            }
            return static_cast<int>(options->chunkSize.value());
        }
    };

} // namespace margelo::nitro::rnzlib