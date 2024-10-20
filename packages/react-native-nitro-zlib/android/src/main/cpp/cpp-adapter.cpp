#include <jni.h>
#include "ZlibOnLoad.hpp"

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *vm, void *)
{
  return margelo::nitro::rnzlib::initialize(vm);
}
