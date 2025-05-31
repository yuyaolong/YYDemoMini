#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include "YYProgram.h"
#include "YYGLCamera.h"

#include "cppUtils.h"
#include "yyDeferredShading.h"
#include "yySimpleDraw.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
// settings
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1200;

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float g_deltaTime = 0.0f; // time between current frame and last frame
float g_lastFrame = 0.0f;

std::shared_ptr<YYGLCamera> g_camera_ptr;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(0); // vsync off
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // different libs need to init glad to fetch all function points again
    if (!cppUtils::loadGLFuncAddressByGlad())
    {
        std::cout << "Failed to load GL function address" << std::endl;
        return -1;
    }

    // create camera
    YYGLCamera::YYGLCameraData cameraData;
    //cameraData.Position = glm::vec3(0.0f, 15.0f, 17.0f);
    //cameraData.Front = glm::vec3(0.0f, 0.0f, -17.0f);
    g_camera_ptr = std::make_shared<YYGLCamera>(cameraData);

    int srcTexWidth = 0;
    int srcTexHeight = 0;
    unsigned int srcTexID = cppUtils::loadTextureFromFile(nullptr, "grid1920.jpg", &srcTexWidth, &srcTexHeight);

    std::shared_ptr<YYGLModule3D> deferredShadingPtr = yyInitDeferredShading(nullptr, "3DModels/nanosuit/nanosuit.obj", true);
    std::vector<glm::vec3> objectPositions;
    objectPositions.push_back(glm::vec3(-3.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3( 0.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3( 3.0, -0.5, -3.0));
    objectPositions.push_back(glm::vec3(-3.0, -0.5,  0.0));
    objectPositions.push_back(glm::vec3( 0.0, -0.5,  0.0));
    objectPositions.push_back(glm::vec3( 3.0, -0.5,  0.0));
    objectPositions.push_back(glm::vec3(-3.0, -0.5,  3.0));
    objectPositions.push_back(glm::vec3( 0.0, -0.5,  3.0));
    objectPositions.push_back(glm::vec3( 3.0, -0.5,  3.0));

    std::shared_ptr<YYGLModule> simpledrawPtr = yyInitSimpleDraw(nullptr, false);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        std::vector<glm::mat4> modelMat4s;
        float currentFrame = static_cast<float>(glfwGetTime());
        g_deltaTime = currentFrame - g_lastFrame;
        g_lastFrame = currentFrame;
        // input
        // -----
        processInput(window);

        // view/projection transformations
        glm::mat4 projection = g_camera_ptr->GetProjectionMatrix();
        glm::mat4 view = g_camera_ptr->GetViewMatrix();
        for (unsigned int i = 0; i < objectPositions.size(); i++)
        {
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, objectPositions[i]);
            model = glm::scale(model, glm::vec3(0.25f));
            modelMat4s.push_back(model);
        }
        

        unsigned int testTex = yyProcessDeferredShading(deferredShadingPtr, 0, SCR_WIDTH, SCR_HEIGHT, projection, view, modelMat4s, cameraData.Position);
        //yyProcessSimpleDraw(simpledrawPtr, testTex, 0, 0, SCR_WIDTH, SCR_HEIGHT, false);
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        g_camera_ptr->processMovement(YYGLCamera::FORWARD, g_deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        g_camera_ptr->processMovement(YYGLCamera::BACKWARD, g_deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        g_camera_ptr->processMovement(YYGLCamera::LEFT, g_deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        g_camera_ptr->processMovement(YYGLCamera::RIGHT, g_deltaTime);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    //YYLog::D("clicked");
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    g_camera_ptr->processRotation(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    //YYLog::D("scrolled");
    g_camera_ptr->processZoom(static_cast<float>(-yoffset));
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}