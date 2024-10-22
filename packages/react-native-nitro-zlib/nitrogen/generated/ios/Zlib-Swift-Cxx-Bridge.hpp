///
/// Zlib-Swift-Cxx-Bridge.hpp
/// This file was generated by nitrogen. DO NOT MODIFY THIS FILE.
/// https://github.com/mrousavy/nitro
/// Copyright © 2024 Marc Rousavy @ Margelo
///

#pragma once

// Forward declarations of C++ defined types
// Forward declaration of `ArrayBufferHolder` to properly resolve imports.
namespace NitroModules { class ArrayBufferHolder; }
// Forward declaration of `ArrayBuffer` to properly resolve imports.
namespace NitroModules { class ArrayBuffer; }
// Forward declaration of `Error` to properly resolve imports.
namespace margelo::nitro::rnzlib { struct Error; }
// Forward declaration of `ZlibOptions` to properly resolve imports.
namespace margelo::nitro::rnzlib { struct ZlibOptions; }

// Include C++ defined types
#if __has_include("Error.hpp")
 #include "Error.hpp"
#endif
#if __has_include("ZlibOptions.hpp")
 #include "ZlibOptions.hpp"
#endif
#if __has_include(<NitroModules/ArrayBuffer.hpp>)
 #include <NitroModules/ArrayBuffer.hpp>
#endif
#if __has_include(<NitroModules/ArrayBufferHolder.hpp>)
 #include <NitroModules/ArrayBufferHolder.hpp>
#endif
#if __has_include(<NitroModules/PromiseHolder.hpp>)
 #include <NitroModules/PromiseHolder.hpp>
#endif
#if __has_include(<functional>)
 #include <functional>
#endif
#if __has_include(<future>)
 #include <future>
#endif
#if __has_include(<memory>)
 #include <memory>
#endif
#if __has_include(<optional>)
 #include <optional>
#endif
#if __has_include(<string>)
 #include <string>
#endif

/**
 * Contains specialized versions of C++ templated types so they can be accessed from Swift,
 * as well as helper functions to interact with those C++ types from Swift.
 */
namespace margelo::nitro::rnzlib::bridge::swift {

  /**
   * Specialized version of `std::optional<double>`.
   */
  using std__optional_double_ = std::optional<double>;
  inline std::optional<double> create_std__optional_double_(const double& value) {
    return std::optional<double>(value);
  }
  
  /**
   * Specialized version of `std::function<void(const std::shared_ptr<ArrayBuffer>&)>`.
   */
  using Func_void_std__shared_ptr_ArrayBuffer_ = std::function<void(const std::shared_ptr<ArrayBuffer>& /* chunk */)>;
  /**
   * Wrapper class for a `std::function<void(const std::shared_ptr<ArrayBuffer>& / * chunk * /)>`, this can be used from Swift.
   */
  class Func_void_std__shared_ptr_ArrayBuffer__Wrapper {
  public:
    explicit Func_void_std__shared_ptr_ArrayBuffer__Wrapper(const std::function<void(const std::shared_ptr<ArrayBuffer>& /* chunk */)>& func): function(func) {}
    explicit Func_void_std__shared_ptr_ArrayBuffer__Wrapper(std::function<void(const std::shared_ptr<ArrayBuffer>& /* chunk */)>&& func): function(std::move(func)) {}
  
    void call(ArrayBufferHolder chunk) const {
      function(chunk.getArrayBuffer());
    }
  
    std::function<void(const std::shared_ptr<ArrayBuffer>& /* chunk */)> function;
  };
  inline Func_void_std__shared_ptr_ArrayBuffer_ create_Func_void_std__shared_ptr_ArrayBuffer_(void* closureHolder, void(*call)(void* /* closureHolder */, ArrayBufferHolder), void(*destroy)(void*)) {
    std::shared_ptr<void> sharedClosureHolder(closureHolder, destroy);
    return Func_void_std__shared_ptr_ArrayBuffer_([sharedClosureHolder, call](const std::shared_ptr<ArrayBuffer>& chunk) -> void {
      call(sharedClosureHolder.get(), ArrayBufferHolder(chunk));
    });
  }
  inline std::shared_ptr<Func_void_std__shared_ptr_ArrayBuffer__Wrapper> share_Func_void_std__shared_ptr_ArrayBuffer_(const Func_void_std__shared_ptr_ArrayBuffer_& value) {
    return std::make_shared<Func_void_std__shared_ptr_ArrayBuffer__Wrapper>(value);
  }
  
  /**
   * Specialized version of `std::function<void()>`.
   */
  using Func_void = std::function<void()>;
  /**
   * Wrapper class for a `std::function<void()>`, this can be used from Swift.
   */
  class Func_void_Wrapper {
  public:
    explicit Func_void_Wrapper(const std::function<void()>& func): function(func) {}
    explicit Func_void_Wrapper(std::function<void()>&& func): function(std::move(func)) {}
  
    void call() const {
      function();
    }
  
    std::function<void()> function;
  };
  inline Func_void create_Func_void(void* closureHolder, void(*call)(void* /* closureHolder */), void(*destroy)(void*)) {
    std::shared_ptr<void> sharedClosureHolder(closureHolder, destroy);
    return Func_void([sharedClosureHolder, call]() -> void {
      call(sharedClosureHolder.get());
    });
  }
  inline std::shared_ptr<Func_void_Wrapper> share_Func_void(const Func_void& value) {
    return std::make_shared<Func_void_Wrapper>(value);
  }
  
  /**
   * Specialized version of `std::optional<std::string>`.
   */
  using std__optional_std__string_ = std::optional<std::string>;
  inline std::optional<std::string> create_std__optional_std__string_(const std::string& value) {
    return std::optional<std::string>(value);
  }
  
  /**
   * Specialized version of `std::function<void(const Error&)>`.
   */
  using Func_void_Error = std::function<void(const Error& /* error */)>;
  /**
   * Wrapper class for a `std::function<void(const Error& / * error * /)>`, this can be used from Swift.
   */
  class Func_void_Error_Wrapper {
  public:
    explicit Func_void_Error_Wrapper(const std::function<void(const Error& /* error */)>& func): function(func) {}
    explicit Func_void_Error_Wrapper(std::function<void(const Error& /* error */)>&& func): function(std::move(func)) {}
  
    void call(Error error) const {
      function(error);
    }
  
    std::function<void(const Error& /* error */)> function;
  };
  inline Func_void_Error create_Func_void_Error(void* closureHolder, void(*call)(void* /* closureHolder */, Error), void(*destroy)(void*)) {
    std::shared_ptr<void> sharedClosureHolder(closureHolder, destroy);
    return Func_void_Error([sharedClosureHolder, call](const Error& error) -> void {
      call(sharedClosureHolder.get(), error);
    });
  }
  inline std::shared_ptr<Func_void_Error_Wrapper> share_Func_void_Error(const Func_void_Error& value) {
    return std::make_shared<Func_void_Error_Wrapper>(value);
  }
  
  /**
   * Specialized version of `std::optional<std::shared_ptr<ArrayBuffer>>`.
   */
  using std__optional_std__shared_ptr_ArrayBuffer__ = std::optional<std::shared_ptr<ArrayBuffer>>;
  inline std::optional<std::shared_ptr<ArrayBuffer>> create_std__optional_std__shared_ptr_ArrayBuffer__(const std::shared_ptr<ArrayBuffer>& value) {
    return std::optional<std::shared_ptr<ArrayBuffer>>(value);
  }
  
  /**
   * Specialized version of `std::optional<bool>`.
   */
  using std__optional_bool_ = std::optional<bool>;
  inline std::optional<bool> create_std__optional_bool_(const bool& value) {
    return std::optional<bool>(value);
  }
  
  /**
   * Specialized version of `std::optional<ZlibOptions>`.
   */
  using std__optional_ZlibOptions_ = std::optional<ZlibOptions>;
  inline std::optional<ZlibOptions> create_std__optional_ZlibOptions_(const ZlibOptions& value) {
    return std::optional<ZlibOptions>(value);
  }
  
  /**
   * Specialized version of `PromiseHolder<std::shared_ptr<ArrayBuffer>>`.
   */
  using PromiseHolder_std__shared_ptr_ArrayBuffer__ = PromiseHolder<std::shared_ptr<ArrayBuffer>>;
  inline PromiseHolder<std::shared_ptr<ArrayBuffer>> create_PromiseHolder_std__shared_ptr_ArrayBuffer__() {
    return PromiseHolder<std::shared_ptr<ArrayBuffer>>();
  }

} // namespace margelo::nitro::rnzlib::bridge::swift