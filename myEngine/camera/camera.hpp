#pragma once
#include "../../application/film.hpp"
#include "../ray.hpp"

class Camera
{
public:
    Camera(Film &film, const glm::vec3 &position, const glm::vec3 &viewpoint, float fovy);
    Ray generateRay(const glm::ivec2 &pixelCoord, const glm::vec2 &offset = {0.5f, 0.5f}) const;

    Film &getFilm() { return mFilm; }
    const Film &getFilm() const { return mFilm; }

private:
    Film &mFilm;
    glm::vec3 mPosition;

    glm::mat4 cameraFromClip;  // 将剪裁空间转换到相机空间
    glm::mat4 worldFromCamera; // 将相机空间转换到世界空间
};