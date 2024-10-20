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
        HybridZlibStream();
        explicit HybridZlibStream(int level, bool deflate);
        ~HybridZlibStream() override;

        bool write(const std::shared_ptr<ArrayBuffer> &chunk) override;
        void end() override;
        void flush(std::optional<double> kind) override;
        void onData(const std::function<void(const std::shared_ptr<ArrayBuffer> &chunk)> &callback) override;
        void onEnd(const std::function<void()> &callback) override;
        void onError(const std::function<void(const Error &error)> &callback) override;
        void params(double level, double strategy) override;
        void reset() override;
        double getMemorySize() override;

    private:
        void initStream(int level, bool deflate);
        std::shared_ptr<ArrayBuffer> createArrayBuffer(size_t size);

        std::unique_ptr<z_stream> _zstream;
        bool _deflate;
        bool _initialized;
        std::function<void(const std::shared_ptr<ArrayBuffer> &chunk)> _dataCallback;
        std::function<void()> _endCallback;
        std::function<void(const Error &error)> _errorCallback;

        static constexpr auto TAG = "HybridZlibStream";
    };

} // namespace margelo::nitro::rnzlib