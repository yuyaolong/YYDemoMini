#ifdef __ANDROID__
#include <EGL/egl.h>
#include <GLES3/gl32.h>
#include <android/asset_manager_jni.h>
#include <unistd.h>
#endif // __ANDROID__

#ifdef _WIN32
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#endif