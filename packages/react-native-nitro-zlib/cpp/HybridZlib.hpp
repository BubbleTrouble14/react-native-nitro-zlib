#pragma once

#include <zlib.h>
#include "HybridZlibSpec.hpp"
#include <functional> // for std::function
#include <memory>     // for std::shared_ptr
#include <optional>

namespace margelo::nitro::rnzlib
{

    using namespace facebook;

    class HybridZlib : public virtual HybridZlibSpec
    {
    public:
        HybridZlib() : HybridObject(TAG) {}

    public:
        std::string getVersion() override;
        std::shared_ptr<ArrayBuffer> inflate(const std::shared_ptr<ArrayBuffer> &data, std::optional<FlushMode> flush) override;
        std::shared_ptr<ArrayBuffer> inflateRaw(const std::shared_ptr<ArrayBuffer> &data, std::optional<FlushMode> flush) override;
        std::shared_ptr<ArrayBuffer> compress(const std::shared_ptr<ArrayBuffer> &data, std::optional<double> level) override;
        std::shared_ptr<ArrayBuffer> deflate(const std::shared_ptr<ArrayBuffer> &data, std::optional<double> level, std::optional<FlushMode> flush) override;
        std::shared_ptr<ArrayBuffer> deflateRaw(const std::shared_ptr<ArrayBuffer> &data, std::optional<double> level, std::optional<FlushMode> flush) override;
        std::shared_ptr<ArrayBuffer> gzip(const std::shared_ptr<ArrayBuffer> &data, std::optional<double> level) override;
        std::shared_ptr<ArrayBuffer> gunzip(const std::shared_ptr<ArrayBuffer> &data) override;

    private:
        // Helper functions
        std::shared_ptr<ArrayBuffer> processZlib(const std::shared_ptr<ArrayBuffer> &data, int (*zlibFunc)(z_stream *, int), std::function<int(z_stream *, int)> zlibInit, int windowBits);
    };

} // namespace margelo::nitro::rnzlib