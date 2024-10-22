// HybridZlibStream.hpp

#pragma once

#include "HybridZlibStreamSpec.hpp"
#include <NitroModules/ArrayBuffer.hpp>
#include <zlib.h>
#include <functional>
#include <memory>

namespace margelo::nitro::rnzlib
{

    class HybridZlibStream : public HybridZlibStreamSpec
    {
    public:
        HybridZlibStream() : HybridObject(TAG) {}

        bool write(const std::shared_ptr<ArrayBuffer> &chunk) override;
        void end() override;
        void flush(std::optional<double> kind) override;
        void onData(const std::function<void(const std::shared_ptr<ArrayBuffer> &chunk)> &callback) override;
        void onEnd(const std::function<void()> &callback) override;
        void onError(const std::function<void(const Error &error)> &callback) override;
        void params(double level, double strategy) override;
        void reset() override;
        double getMemorySize() override;

        static std::shared_ptr<HybridZlibStream> create(int level, bool deflate, int windowBits = 15)
        {
            Logger::log(LogLevel::Debug, "HybridZlibStream",
                        "Creating HybridZlibStream: level = %d, deflate = %d, windowBits = %d",
                        level, deflate, windowBits);
            auto instance = std::make_shared<HybridZlibStream>();
            if (deflate)
            {
                instance->initDeflateWithWindowBits(level, windowBits);
            }
            else
            {
                instance->initInflateWithWindowBits(windowBits);
            }
            return instance;
        }

    private:
        void initStream(int level, bool deflate);
        void processOutput(int ret);
        std::shared_ptr<ArrayBuffer> createArrayBuffer(size_t size);
        void reportError(const std::string &message);

        void initWithWindowBits(int windowBits);
        void initDeflateWithWindowBits(int level, int windowBits);
        void initInflateWithWindowBits(int windowBits);

        std::unique_ptr<z_stream> _zstream;
        bool _deflate;
        bool _initialized;
        std::vector<uint8_t> _outBuffer;
        std::function<void(const std::shared_ptr<ArrayBuffer> &chunk)> _dataCallback;
        std::function<void()> _endCallback;
        std::function<void(const Error &error)> _errorCallback;

        static constexpr size_t CHUNK_SIZE = 16384; // 16KB
    };

} // namespace margelo::nitro::rnzlib