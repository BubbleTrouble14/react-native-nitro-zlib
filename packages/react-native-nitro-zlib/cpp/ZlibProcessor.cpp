#include "ZlibProcessor.hpp"
#include <stdexcept>
#include <cstring>

namespace margelo::nitro::rnzlib
{

    ZlibProcessor::ZlibProcessor(const std::shared_ptr<ArrayBuffer> &data)
    {
        // Copy data while on JS thread
        size_t size = data->size();
        const uint8_t *ptr = static_cast<const uint8_t *>(data->data());
        inputData.assign(ptr, ptr + size);

        // Initialize zlib stream
        memset(&strm, 0, sizeof(strm));
        strm.zalloc = Z_NULL;
        strm.zfree = Z_NULL;
        strm.opaque = Z_NULL;
    }

    ZlibProcessor::~ZlibProcessor()
    {
        deflateEnd(&strm);
    }

    std::shared_ptr<ArrayBuffer> ZlibProcessor::process(
        int (*zlibFunc)(z_stream *, int),
        std::function<int(z_stream *, int)> zlibInit,
        int windowBits,
        const std::optional<ZlibOptions> &options)
    {

        // Setup input
        strm.avail_in = static_cast<uInt>(inputData.size());
        strm.next_in = inputData.data();

        // Initialize
        int ret = zlibInit(&strm, windowBits);
        if (ret != Z_OK)
        {
            throw std::runtime_error("Failed to initialize zlib");
        }

        // Prepare output buffer - estimate size based on input
        size_t bufferSize = inputData.size() * 2; // Initial estimate
        uint8_t *outputBuffer = new uint8_t[bufferSize];
        strm.avail_out = static_cast<uInt>(bufferSize);
        strm.next_out = outputBuffer;

        // Process
        ret = zlibFunc(&strm, Z_FINISH);

        if (ret == Z_STREAM_ERROR)
        {
            delete[] outputBuffer;
            deflateEnd(&strm);
            throw std::runtime_error("Stream error");
        }

        if (ret != Z_STREAM_END)
        {
            delete[] outputBuffer;
            deflateEnd(&strm);
            throw std::runtime_error("Incomplete stream");
        }

        // Calculate actual size
        size_t actualSize = bufferSize - strm.avail_out;

        // Create final buffer as an owning NativeArrayBuffer
        return std::make_shared<NativeArrayBuffer>(
            outputBuffer,
            actualSize,
            [bufferToDelete = outputBuffer]()
            {
                delete[] bufferToDelete;
            });
    }

} // namespace margelo::nitro::rnzlib