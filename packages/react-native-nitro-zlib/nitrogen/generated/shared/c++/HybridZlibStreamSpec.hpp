///
/// HybridZlibStreamSpec.hpp
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
// Forward declaration of `Error` to properly resolve imports.
namespace margelo::nitro::rnzlib { struct Error; }

#include <NitroModules/ArrayBuffer.hpp>
#include <optional>
#include <functional>
#include "Error.hpp"

namespace margelo::nitro::rnzlib {

  using namespace margelo::nitro;

  /**
   * An abstract base class for `ZlibStream`
   * Inherit this class to create instances of `HybridZlibStreamSpec` in C++.
   * You must explicitly call `HybridObject`'s constructor yourself, because it is virtual.
   * @example
   * ```cpp
   * class HybridZlibStream: public HybridZlibStreamSpec {
   * public:
   *   HybridZlibStream(...): HybridObject(TAG) { ... }
   *   // ...
   * };
   * ```
   */
  class HybridZlibStreamSpec: public virtual HybridObject {
    public:
      // Constructor
      explicit HybridZlibStreamSpec(): HybridObject(TAG) { }

      // Destructor
      virtual ~HybridZlibStreamSpec() { }

    public:
      // Properties
      

    public:
      // Methods
      virtual bool write(const std::shared_ptr<ArrayBuffer>& chunk) = 0;
      virtual void end() = 0;
      virtual void flush(std::optional<double> kind) = 0;
      virtual void onData(const std::function<void(const std::shared_ptr<ArrayBuffer>& /* chunk */)>& callback) = 0;
      virtual void onEnd(const std::function<void()>& callback) = 0;
      virtual void onError(const std::function<void(const Error& /* error */)>& callback) = 0;
      virtual void params(double level, double strategy) = 0;
      virtual void reset() = 0;
      virtual double getMemorySize() = 0;

    protected:
      // Hybrid Setup
      void loadHybridMethods() override;

    protected:
      // Tag for logging
      static constexpr auto TAG = "ZlibStream";
  };

} // namespace margelo::nitro::rnzlib
