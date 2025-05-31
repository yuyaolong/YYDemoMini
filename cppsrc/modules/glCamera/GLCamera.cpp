//
// Created by yuyaolong on 2018/8/11.
//

#include "GLCamera.h"
#include "miscUtils.h"

const glm::quat GLCamera::identifyQuat(1.0f, 0.0f, 0.0f, 0.0f);
const glm::mat4 GLCamera::identityMat4(1.0f);
GLCamera::GLCamera(float fovy, float aspect, float nearPlan, float farPlan,
                   glm::vec3 cameraPosition, glm::vec3 viewCenter, glm::vec3 cameraUp) : fovy(fovy),
                                                                                         aspect(aspect),
                                                                                         nearPlan(nearPlan),
                                                                                         farPlan(farPlan),
                                                                                         cameraPosition(cameraPosition),
                                                                                         cameraInitPosition(cameraPosition),
                                                                                         viewCenter(viewCenter),
                                                                                         cameraUp(cameraUp){
    objectTranslationMat4 = identityMat4;
    objectRotationQuat    = identifyQuat;
    objectScaleMat4       = identityMat4;
    mvpMat4               = identityMat4;
    modelMat4             = identityMat4;
    viewMat4              = identityMat4;
    projectionMat4        = identityMat4;

    //ALOGD("hehe in constructor, fov: %f", fovy);
}

glm::mat4 GLCamera::getMVPMat4() {
    return mvpMat4;
}

glm::mat4 GLCamera::getModelMat4() {
    return modelMat4;
}

glm::mat4 GLCamera::getViewMat4() {
    return viewMat4;
}

glm::mat4 GLCamera::getProjectionMat4() {
    return projectionMat4;
}

glm::vec3 GLCamera::getCameraPosition() {
    return cameraPosition;
}

void GLCamera::objectScale(float scaleFactor) {
    objectScaleMat4 = glm::scale(glm::mat4(1.0f), glm::vec3(scaleFactor)) * objectScaleMat4;
}

void GLCamera::objectTranslateOnce(float distanceX, float distanceY, float distanceZ) {
    objectTranslationMat4 = glm::translate(glm::mat4(1.0f), glm::vec3(distanceX, distanceY, distanceZ)) * identityMat4;
}

void GLCamera::objectTranslate(float distanceX, float distanceY, float distanceZ) {
    objectTranslationMat4 = glm::translate(objectTranslationMat4, glm::vec3(distanceX, distanceY, distanceZ));
}



glm::quat GLCamera::sphereRotate(float distanceX,
                        float distanceY,
                        float endPositionX,
                        float endPositionY)
{
    // origin of screen resolution coordinate is at left-top, move origin to center,
    // set up is y positive, right is x positive
    endPositionX = fmax(-1, fmin(1, endPositionX));
    endPositionY = fmax(-1, fmin(1, endPositionY));
    //endPositionX = 0;
    //ALOGD("!! disX %f, disY %f, endX %f, endY %f", distanceX, distanceY, endPositionX, endPositionY);

    float endPositionZ = sqrt(1 - fmin(1, endPositionX * endPositionX + endPositionY * endPositionY));
    glm::vec3 endVec = glm::normalize(glm::vec3(endPositionX, endPositionY, endPositionZ));

    float startPositionX = fmax(-1, fmin(1, endPositionX + distanceX));
    float startPositionY = fmax(-1, fmin(1, endPositionY + distanceY));
    float startPositionZ = sqrt(1 - fmin(1, startPositionX * startPositionX + startPositionY* startPositionY));
    glm::vec3 startVec = glm::normalize(glm::vec3(startPositionX, startPositionY, startPositionZ));

    glm::vec3 axis;
    float cosAngle = fmax(fmin(glm::dot(startVec, endVec), 1.0f), -1.0f);
    if (cosAngle < (-1.0f + 0.001f)) {
        // special case when vectors in opposite directions:
        // there is no "ideal" rotation axis
        // So guess one; any will do as long as it's perpendicular to start
        axis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), startVec);
        if (glm::length(axis) < 0.01 ) // bad luck, they were parallel, try again!
            axis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), startVec);

        axis = normalize(axis);
    }
    else
    {
        axis = glm::normalize(glm::cross(startVec, endVec));
    }

    float angle = 2 * acos(cosAngle);

    // ALOGD("!! axis: [%f, %f, %f]", axis.x, axis.y, axis.z);
    // ALOGD("!! angle: %f", angle / 3.1415926 * 180);
    if ((glm::any(glm::isnan(axis)) == false) && (isnan(angle) == false))
    {
        //cameraRotationMat4 = glm::rotate(glm::mat4(1.0f), angle, axis) * cameraRotationMat4;
        return glm::angleAxis(angle, axis);
    }
    return identifyQuat;
}

void GLCamera::objectFingerRotate(float disX,
                                  float disY,
                                  float endPosX,
                                  float endPosY) {

    objectRotationQuat = sphereRotate(disX, disY, endPosX, endPosY) * objectRotationQuat;
}

void GLCamera::cameraViewRotate(float disX,
                                float disY,
                                float endPosX,
                                float endPosY)
{
    // camera total pitch and yaw
    static float total_pitch = 0.0f;
    static float total_yaw = 0.0f;
    // camera only do yaw and pitch here
    static const float sensitive = 50;
    disX *= sensitive;
    disY *= sensitive;
    //ALOGD("!! disX: %f, disY: %f", disX, disY);
    total_yaw += disX;
    total_pitch += disY;
    total_pitch = fmax(-89.0f, fmin(total_pitch, 89.0f));
    glm::vec3 front;
    front.x = cos(glm::radians(total_pitch)) * sin(glm::radians(total_yaw));
    front.y = sin(glm::radians(total_pitch));
    front.z = -1 * cos(glm::radians(total_pitch)) * cos(glm::radians(total_yaw));

    front = glm::normalize(front);
    //ALOGD("!! front [%f, %f, %f]", front.x, front.y, front.z);

    cameraDirection = glm::length(cameraDirection) * front;

    //ALOGD("!! cameraDir [%f, %f, %f]", cameraDirection.x, cameraDirection.y, cameraDirection.z);
}

void GLCamera::cameraPositionRotate(float disX,
                          float disY,
                          float endPosX,
                          float endPosY)
{
    static float degX = 0.0f;
    ALOGD("disX: %f", disX);
    degX += disX * 20.0f;
    float R = glm::length(cameraPosition);
    float posX = R * sin(glm::radians(degX));
    float posZ = R * cos(glm::radians(degX));
    cameraPosition = glm::vec3(posX, 0, posZ);
    cameraDirection = viewCenter - cameraPosition;

}

void GLCamera::cameraRotate(float disX,
                            float disY,
                            float endPosX,
                            float endPosY) {
    // cameraViewRotate(disX, disY, endPosX, endPosY);
    cameraPositionRotate(disX, disY, endPosX, endPosY);

}

void GLCamera::cameraScale(float disX,
                            float disY,
                            float endPosX,
                            float endPosY) {
    ALOGD("!! disY: %f", disY);
    float diff = 40 * disY;
    fovy = fmax(10, fmin(fovy + diff, 90));
}

void GLCamera::cameraLookAt(glm::vec3 cameraPosition, glm::vec3 viewCenter, glm::vec3 up) {
    this->cameraPosition = cameraPosition;
    this->viewCenter = viewCenter;
    cameraDirection = viewCenter - cameraPosition;
    glm::vec3 cameraLeft = glm::normalize(glm::cross(up, glm::normalize(cameraDirection)));
    this->cameraUp = glm::cross(glm::normalize(cameraDirection), cameraLeft);
}

void GLCamera::calculateMVPMat4() {

    modelMat4 = objectTranslationMat4 *
                glm::mat4_cast(objectRotationQuat) *
                objectScaleMat4;
    viewMat4 = glm::lookAt(cameraPosition, cameraPosition + cameraDirection, cameraUp);

    projectionMat4 = glm::perspective(glm::radians(fovy), aspect, nearPlan, farPlan);

    mvpMat4 = projectionMat4 * viewMat4 * modelMat4;
}

void GLCamera::resetCamera() {
    objectScaleMat4       = identityMat4;
    objectRotationQuat    = identifyQuat;
    objectTranslationMat4 = identityMat4;
    cameraDirection       = viewCenter - cameraPosition;
    fovy                  = 60;
    cameraPosition = cameraInitPosition;
}