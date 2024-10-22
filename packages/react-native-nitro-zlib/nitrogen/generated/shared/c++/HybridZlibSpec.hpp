///
/// HybridZlibSpec.hpp
/// This file was generated by nitrogen. DO NOT MODIFY THIS FILE.
/// https://github.com/mrousavy/nitro
/// Copyright © 2024 Marc Rousavy @ Margelo
///

#pragma once

#if __has_include(<NitroModules/HybridObject.hpp>)
#include <NitroModules/HybridObject.hpp>
#else
#error NitroModules cannot be found! Are you sure you installed NitroModules properly?
#endif

// Forward declaration of `ArrayBuffer` to properly resolve imports.
namespace NitroModules { class ArrayBuffer; }
// Forward declaration of `ZlibOptions` to properly resolve imports.
namespace margelo::nitro::rnzlib { struct ZlibOptions; }
// Forward declaration of `HybridZlibStreamSpec` to properly resolve imports.
namespace margelo::nitro::rnzlib { class HybridZlibStreamSpec; }

#include <string>
#include <NitroModules/ArrayBuffer.hpp>
#include <optional>
#include "ZlibOptions.hpp"
#include <future>
#include <memory>
#include "HybridZlibStreamSpec.hpp"

namespace margelo::nitro::rnzlib {

  using namespace margelo::nitro;

  /**
   * An abstract base class for `Zlib`
   * Inherit this class to create instances of `HybridZlibSpec` in C++.
   * You must explicitly call `HybridObject`'s constructor yourself, because it is virtual.
   * @example
   * ```cpp
   * class HybridZlib: public HybridZlibSpec {
   * public:
   *   HybridZlib(...): HybridObject(TAG) { ... }
   *   // ...
   * };
   * ```
   */
  class HybridZlibSpec: public virtual HybridObject {
    public:
      // Constructor
      explicit HybridZlibSpec(): HybridObject(TAG) { }

      // Destructor
      virtual ~HybridZlibSpec() { }

    public:
      // Properties
      virtual std::string getVersion() = 0;

    public:
      // Methods
      virtual std::shared_ptr<ArrayBuffer> inflateSync(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<ArrayBuffer> inflateRawSync(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<ArrayBuffer> compressSync(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<ArrayBuffer> deflateSync(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<ArrayBuffer> deflateRawSync(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<ArrayBuffer> gzipSync(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<ArrayBuffer> gunzipSync(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::future<std::shared_ptr<ArrayBuffer>> inflate(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::future<std::shared_ptr<ArrayBuffer>> inflateRaw(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::future<std::shared_ptr<ArrayBuffer>> compress(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::future<std::shared_ptr<ArrayBuffer>> deflate(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::future<std::shared_ptr<ArrayBuffer>> deflateRaw(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::future<std::shared_ptr<ArrayBuffer>> gzip(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::future<std::shared_ptr<ArrayBuffer>> gunzip(const std::shared_ptr<ArrayBuffer>& data, const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<margelo::nitro::rnzlib::HybridZlibStreamSpec> createDeflateStream(const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<margelo::nitro::rnzlib::HybridZlibStreamSpec> createInflateStream(const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<margelo::nitro::rnzlib::HybridZlibStreamSpec> createGzipStream(const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<margelo::nitro::rnzlib::HybridZlibStreamSpec> createGunzipStream(const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<margelo::nitro::rnzlib::HybridZlibStreamSpec> createDeflateRawStream(const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<margelo::nitro::rnzlib::HybridZlibStreamSpec> createInflateRawStream(const std::optional<ZlibOptions>& options) = 0;
      virtual std::shared_ptr<margelo::nitro::rnzlib::HybridZlibStreamSpec> createUnzipStream(const std::optional<ZlibOptions>& options) = 0;

    protected:
      // Hybrid Setup
      void loadHybridMethods() override;

    protected:
      // Tag for logging
      static constexpr auto TAG = "Zlib";
  };

} // namespace margelo::nitro::rnzlib