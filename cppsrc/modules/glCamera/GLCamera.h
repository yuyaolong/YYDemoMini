//
// Created by yuyaolong on 2018/8/11.
//

#ifndef GLCAMERA_H
#define GLCAMERA_H

#include <android/log.h>
#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/quaternion.hpp"

class GLCamera {

public:
    GLCamera(float fovy = 60.0f, float aspect = 0.704501f, float nearPlan = 0.1f,
                  float farPlan = 100.0f, glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 4.0f),
                  glm::vec3 viewCenter = glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f));

    inline void setAspect(float aspect) {this->aspect = aspect;}

    void calculateMVPMat4();

    glm::mat4 getMVPMat4();

    glm::mat4 getModelMat4();

    glm::mat4 getViewMat4();

    glm::mat4 getProjectionMat4();

    glm::vec3 getCameraPosition();

    void objectScale(float scaleFactor);

    void objectTranslateOnce(float distanceX, float distanceY, float distanceZ);

    void objectTranslate(float distanceX, float distanceY, float distanceZ);

    // coordinate range is (-1 1)
    void objectFingerRotate(float distanceX, float distanceY, float endPositionX, float endPositionY);

    void cameraLookAt(glm::vec3 cameraPosition, glm::vec3 viewCenter, glm::vec3 up);

    void cameraRotate(float distanceX, float distanceY, float endPositionX, float endPositionY);

    void cameraScale(float distanceX, float distanceY, float endPositionX, float endPositionY);

    void resetCamera();


private:
    float aspect;
    float nearPlan;
    float farPlan;
    float fovy;

    float screenWidth;
    float screenHeight;

    glm::vec3 cameraPosition;
    glm::vec3 cameraInitPosition;
    glm::vec3 viewCenter;
    glm::vec3 cameraDirection;
    glm::vec3 cameraUp;

    //object scale -> rotate -> translate
    glm::mat4 objectTranslationMat4;
    glm::quat objectRotationQuat;
    glm::mat4 objectScaleMat4;

    //MVP matrix
    glm::mat4 mvpMat4;

    //Model matrix
    glm::mat4 modelMat4;

    //View matrix
    glm::mat4 viewMat4;

    //Projection matrix
    glm::mat4 projectionMat4;

    glm::quat sphereRotate(float distanceX,
                           float distanceY,
                           float endPositionX,
                           float endPositionY);

    void cameraViewRotate(float disX,
                          float disY,
                          float endPosX,
                          float endPosY);

    void cameraPositionRotate(float disX,
                          float disY,
                          float endPosX,
                          float endPosY);

    // Identity Matrix
    static const glm::mat4 identityMat4;

    // Identity Quaternion
    static const glm::quat identifyQuat;

};

#endif //GLCAMERA_H
