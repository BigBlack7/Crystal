#pragma once
#include "../../application/film.hpp"
#include "../ray.hpp"

enum class Direction
{
    Forward,
    Backward,
    Left,
    Right,
    Up,
    Down
};

class Camera
{
public:
    Camera(Film &film, const glm::vec3 &position, const glm::vec3 &viewpoint, float fovy);
    Ray generateRay(const glm::ivec2 &pixelCoord, const glm::vec2 &offset = {0.5f, 0.5f}) const;

    Film &getFilm() { return mFilm; }
    const Film &getFilm() const { return mFilm; }

    // 相机移动
    void move(float dt, Direction direction); // 调整位置，wasd ⬆⬇
    void turn(const glm::vec2 &delta);        // 调整朝向，鼠标移动
    void zoom(float delta);                   // 调整视角大小，鼠标滚轮

private:
    void update(); // 更新相机矩阵

private:
    Film &mFilm;
    glm::vec3 mPosition;

    glm::mat4 cameraFromClip;  // 将剪裁空间转换到相机空间
    glm::mat4 worldFromCamera; // 将相机空间转换到世界空间

    // 相机移动
    glm::vec3 mViewDirection;           // 相机朝向
    float theta, phi;                   // 相机的俯仰角和偏航角
    float mFovy;                        // 相机的垂直视角
    float mMoveSpeed{2.f};              // 移动速度
    glm::vec2 mTurnSpeed{0.15f, 0.07f}; // 转向速度
};