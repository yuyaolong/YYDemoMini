//
// Created by yuyaolong on 2018/8/11.
//

#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

/*
camera translation
camera rotation by mouse or scroll
take camera data as creation data
*/
class YYGLCamera {
public:
    enum YYGLCameraMovement {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };
    struct YYGLCameraData
    {
        // camera internal attributes
        float FOV = 45.0f;
        float Ratio = 1.333f;
        float Near = 0.1f;
        float Far = 100.0f;
        // camera outside attributes
        glm::vec3 Position = glm::vec3(0.0f, 0.0f, 0.0f);
        glm::vec3 Front = glm::vec3(0.0f, 0.0f, -1.0f);
        glm::vec3 Up = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 Right = glm::vec3(1.0f, 0.0f, 0.0f);
        glm::vec3 WorldUp = glm::vec3(0.0f, 1.0f, 0.0f);
        // euler Angles
        float Yaw = -90.0f; // 0 means x positive direction
        float Pitch = 0.0f;
        // camera options
        float MovementSpeed = 5.0f;
        float MouseSensitivity = 0.1f;
    };
    YYGLCamera(const YYGLCameraData& data);
    ~YYGLCamera();
    // return projection matrix by perspective function call
    glm::mat4 GetProjectionMatrix();
    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 GetViewMatrix();

    void processMovement(YYGLCameraMovement direction, float deltaTime);

    void processRotation(float xoffset, float yoffset);

    void processZoom(float offset);

private:
    YYGLCameraData mData;
    void updateCameraVectors();

};
