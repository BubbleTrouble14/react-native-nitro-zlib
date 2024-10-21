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

        static std::shared_ptr<HybridZlibStream> create(int level, bool deflate)
        {
            Logger::log(LogLevel::Debug, "HybridZlibStream", "Creating HybridZlibStream: level = %d, deflate = %d", level, deflate);
            auto instance = std::make_shared<HybridZlibStream>();
            instance->_deflate = deflate;
            instance->initStream(level, deflate);
            return instance;
        }

    private:
        void initStream(int level, bool deflate);
        void processOutput(int ret);
        std::shared_ptr<ArrayBuffer> createArrayBuffer(size_t size);
        void reportError(const std::string &message);

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