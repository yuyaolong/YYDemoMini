//
// Created by yuyaolong on 2018/8/11.
//

#include "YYGLCamera.h"
#include "YYLog.h"
YYGLCamera::YYGLCamera(const YYGLCameraData& data){
    mData = data;
    YYLog::D("YY camera created");
}
YYGLCamera::~YYGLCamera()
{
    YYLog::D("YY camera destroied");
}

glm::mat4 YYGLCamera::GetProjectionMatrix()
{
    return glm::perspective(glm::radians(mData.FOV), mData.Ratio, mData.Near, mData.Far);
}

glm::mat4 YYGLCamera::GetViewMatrix()
{
    return glm::lookAt(mData.Position, mData.Position + mData.Front, mData.Up);
}

void YYGLCamera::processMovement(YYGLCameraMovement direction, float deltaTime)
{
    float velocity = mData.MovementSpeed * deltaTime;
    if (direction == FORWARD)
        mData.Position += mData.Front * velocity;
    if (direction == BACKWARD)
        mData.Position -= mData.Front * velocity;
    if (direction == LEFT)
        mData.Position -= mData.Right * velocity;
    if (direction == RIGHT)
        mData.Position += mData.Right * velocity;
}

void YYGLCamera::updateCameraVectors()
{
    // calculate the new Front vector
    glm::vec3 front;
    front.x = cos(glm::radians(mData.Yaw)) * cos(glm::radians(mData.Pitch));
    front.y = sin(glm::radians(mData.Pitch));
    front.z = sin(glm::radians(mData.Yaw)) * cos(glm::radians(mData.Pitch));
    mData.Front = glm::normalize(front);
    // also re-calculate the Right and Up vector
    mData.Right = glm::normalize(glm::cross(mData.Front, mData.WorldUp));  // normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
    mData.Up    = glm::normalize(glm::cross(mData.Right, mData.Front));
}

void YYGLCamera::processRotation(float xoffset, float yoffset)
{
    xoffset *= mData.MouseSensitivity;
    yoffset *= mData.MouseSensitivity;

    mData.Yaw   += xoffset;
    mData.Pitch += yoffset;
    if (mData.Pitch > 89.0f)
        mData.Pitch = 89.0f;
    if (mData.Pitch < -89.0f)
        mData.Pitch = -89.0f;
    // update Front, Right and Up Vectors using the updated Euler angles
    updateCameraVectors();
}

void YYGLCamera::processZoom(float offset)
{
    mData.FOV += offset;
    if (mData.FOV < 1.0f)
        mData.FOV = 1.0f;
    if (mData.FOV > 45.0f)
        mData.FOV = 45.0f;

}