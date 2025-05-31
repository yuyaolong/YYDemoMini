#include <jni.h>

//
// Created by yuyao on 2023/11/12.
//

#include "yySimpleDraw.h"
#include "yyGaussianSplatRender.h"
#include "YYGLCamera.h"
#include <android/native_window_jni.h>
#include "imgui.h"
#include "imgui_impl_android.h"
#include "imgui_impl_opengl3.h"

namespace {
    std::shared_ptr<YYGLModule> g_yySimpleDrawPtr1;
    std::shared_ptr<YYGLModule> g_yySimpleDrawPtr2;
    std::shared_ptr<YYGLModule> g_yyGaussianSplatRenderPtr;
    std::shared_ptr<YYGLCamera> g_camera;
    GLuint g_srcTexID = 0;
    GLuint g_midTexID = 0;
    int g_srcTexWidth = 0;
    int g_srcTexHeight = 0;
    int g_screenWidth = 0;
    int g_screenHeight = 0;
    ANativeWindow* g_window = nullptr;

    void initImgUI(ANativeWindow * window) {

        assert(window != nullptr);
        EGLSurface curSurface = eglGetCurrentSurface(EGL_DRAW);
        EGLContext curCtx = eglGetCurrentContext();
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();

        ImFontConfig font_cfg;
        font_cfg.SizePixels = 30.0f;
        io.Fonts->AddFontDefault(&font_cfg);

        // todo IniFilename setting

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();

        // Setup Platform/Renderer backends
        ImGui_ImplAndroid_Init(window);
        ImGui_ImplOpenGL3_Init("#version 300 es");
    }

    void cleanUpImgUI() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplAndroid_Shutdown();
        ImGui::DestroyContext();
    }
}



extern "C"
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_general3DRover_gaussianSplattingRover_GaussianSplattingRoverJNICalls_renderInitNative(
        JNIEnv *env, jobject thiz, jobject asset_manager, jobject surface) {
    YYLog::D("GaussianSplatting Init");
    g_window = ANativeWindow_fromSurface(env, surface);
    int32_t width =  ANativeWindow_getWidth(g_window);
    int32_t height =  ANativeWindow_getHeight(g_window);

    initImgUI(g_window);
    AAssetManager *nativeAsset = AAssetManager_fromJava(env, asset_manager);
    //g_yySimpleDrawPtr1 = yyInitSimpleDraw(nativeAsset, false);
    // g_yySimpleDrawPtr2 = yyInitSimpleDraw(nativeAsset, false);

    g_yyGaussianSplatRenderPtr = yyInitGaussianSplatRender(nativeAsset, false);


    g_srcTexID = cppUtils::loadTextureFromFile(nativeAsset, "grid1920.jpg", &g_srcTexWidth, &g_srcTexHeight);
    g_midTexID = cppUtils::createTexture(g_srcTexWidth, g_srcTexHeight);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_general3DRover_gaussianSplattingRover_GaussianSplattingRoverJNICalls_renderResizeNative(
        JNIEnv *env, jobject thiz, jint width, jint height) {
    YYLog::D("GaussianSplatting Resize");
    g_screenWidth = width;
    g_screenHeight = height;
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_general3DRover_gaussianSplattingRover_GaussianSplattingRoverJNICalls_renderDrawNative(
        JNIEnv *env, jobject thiz) {
    YYLog::D("GaussianSplatting Render");
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplAndroid_NewFrame();
    ImGui::NewFrame();
    {
        static float f = 0.0f;
        static int counter = 0;
        ImGuiIO& io = ImGui::GetIO();

        ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", io.DeltaTime * 1000.0f, io.Framerate);
        ImGui::End();
    }

    yyProcessGaussianSplatRender(g_yyGaussianSplatRenderPtr, 0, 0, 0, g_screenWidth, g_screenHeight, false);
    // yyProcessSimpleDraw(g_yySimpleDrawPtr1, g_srcTexID, g_midTexID, 0, g_srcTexWidth, g_srcTexHeight, false);
    // yyProcessSimpleDraw(g_yySimpleDrawPtr2, g_midTexID, 0, 0, g_screenWidth, g_screenHeight, false);
    // Start the Dear ImGui frame


    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_general3DRover_gaussianSplattingRover_GaussianSplattingRoverJNICalls_renderDestroyNative(
        JNIEnv *env, jobject thiz) {
    YYLog::D("GaussianSplatting Destroy");
    g_yyGaussianSplatRenderPtr.reset();
    cleanUpImgUI();
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_general3DRover_gaussianSplattingRover_GaussianSplattingRoverJNICalls_cameraZoomNative(
        JNIEnv *env, jobject thiz, jfloat scaleFactor) {
    yyGaussianSplatRenderCameraZoom(g_yyGaussianSplatRenderPtr, scaleFactor);
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_general3DRover_gaussianSplattingRover_GaussianSplattingRoverJNICalls_cameraTranslateNative(
        JNIEnv *env, jobject thiz) {
    // TODO: implement cameraTranslateNative()
}
extern "C"
JNIEXPORT void JNICALL
Java_com_example_yuyaolong_yyalDemo_general3DRover_gaussianSplattingRover_GaussianSplattingRoverJNICalls_cameraRotateNative(
        JNIEnv *env, jobject thiz) {
    // TODO: implement cameraRotateNative()
}
