#include "camera.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

Camera::Camera(Film &film, const glm::vec3 &position, const glm::vec3 &viewpoint, float fovy)
    : mFilm(film), mPosition(position)
{
    /* 剪裁空间到相机空间的变换矩阵
        相机到剪裁空间需要进行透视投影，故获得透视矩阵的逆矩阵
     */
    cameraFromClip = glm::inverse(glm::perspective(
        glm::radians(fovy),
        static_cast<float>(mFilm.getWidth()) / static_cast<float>(mFilm.getHeight()),
        1.f,
        2.f));
    /* 相机空间到世界空间的变换矩阵
        物体从世界坐标到相机坐标需要进行相机的变换，故获得相机的逆矩阵
    */
    worldFromCamera = glm::inverse(glm::lookAt(position, viewpoint, {0, 1, 0}));
}

Ray Camera::generateRay(const glm::ivec2 &pixelCoord, const glm::vec2 &offset) const
{
    // 计算像素坐标对应的ndc坐标（offset默认是像素中心点）
    glm::vec2 ndc = (glm::vec2(pixelCoord) + offset) / glm::vec2(mFilm.getWidth(), mFilm.getHeight());
    // 由于ndc坐标的原点在左下角，而胶片的原点在左上角，故需要进行转换
    ndc.y = 1.f - ndc.y;
    ndc = ndc * 2.f - 1.f;
    // 计算剪裁空间的坐标，由于默认near为1，这里直接使用ndc
    glm::vec4 clip = glm::vec4(ndc, 0, 1);
    glm::vec3 world = worldFromCamera * cameraFromClip * clip;
    return Ray{mPosition, glm::normalize(world - mPosition)}; // 光线方向是相机指向像素
}
