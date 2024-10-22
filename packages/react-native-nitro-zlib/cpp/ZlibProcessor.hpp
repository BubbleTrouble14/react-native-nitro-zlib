#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include <zlib.h>
#include "HybridZlibSpec.hpp"

namespace margelo::nitro::rnzlib
{

    using DeleteFn = std::function<void()>;

    class ZlibProcessor
    {
    public:
        explicit ZlibProcessor(const std::shared_ptr<ArrayBuffer> &data);
        ~ZlibProcessor();

        // Prevent copying
        ZlibProcessor(const ZlibProcessor &) = delete;
        ZlibProcessor &operator=(const ZlibProcessor &) = delete;

        std::shared_ptr<ArrayBuffer> process(
            int (*zlibFunc)(z_stream *, int),
            std::function<int(z_stream *, int)> zlibInit,
            int windowBits,
            const std::optional<ZlibOptions> &options);

    private:
        std::vector<uint8_t> inputData;
        z_stream strm;
    };

} // namespace margelo::nitro::rnzlib