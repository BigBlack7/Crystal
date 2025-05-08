#include "camera.hpp"
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>

Camera::Camera(Film &film, const glm::vec3 &position, const glm::vec3 &viewpoint, float fovy)
    : mFilm(film), mPosition(position), mFovy(fovy)
{
    mViewDirection = glm::normalize(viewpoint - position);
    update();
    theta = glm::degrees(glm::acos(mViewDirection.y));
    if (glm::abs(mViewDirection.y) == 1)
    {
        phi = 0;
    }
    else
    {
        phi = glm::degrees(glm::acos(mViewDirection.x / glm::sqrt(mViewDirection.x * mViewDirection.x + mViewDirection.z * mViewDirection.z)));
    }
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

void Camera::move(float dt, Direction direction)
{
    glm::vec3 forward = mViewDirection;
    forward.y = 0; // 忽略y轴的影响
    forward = glm::normalize(forward);
    glm::vec3 moveDirection{};
    switch (direction)
    {
    case Direction::Forward:
        moveDirection = forward;
        break;
    case Direction::Backward:
        moveDirection = -forward;
        break;
    case Direction::Left:
        moveDirection = glm::normalize(glm::cross(forward, glm::vec3{0, 1, 0}));
        break;
    case Direction::Right:
        moveDirection = -glm::normalize(glm::cross(forward, glm::vec3{0, 1, 0}));
        break;
    case Direction::Up:
        moveDirection = glm::vec3{0, 1, 0};
        break;
    case Direction::Down:
        moveDirection = glm::vec3{0, -1, 0};
        break;
    default:
        break;
    }
    mPosition += moveDirection * mMoveSpeed * dt;
    update();
}

void Camera::turn(const glm::vec2 &delta)
{
    phi -= delta.x * mTurnSpeed.x; // 绕y轴旋转
    if (phi > 360)
        phi -= 360;
    if (phi < 0)
        phi += 360;
    theta += delta.y * mTurnSpeed.y; // 绕x轴旋转
    theta = glm::clamp(theta, -179.f, 179.f);

    float sin_theta = glm::sin(glm::radians(theta));
    float cos_theta = glm::cos(glm::radians(theta));
    float sin_phi = glm::sin(glm::radians(phi));
    float cos_phi = glm::cos(glm::radians(phi));
    mViewDirection = glm::vec3{sin_theta * cos_phi, cos_theta, sin_theta * sin_phi};
    update();
}

void Camera::zoom(float delta)
{
    mFovy = glm::clamp(mFovy - delta, 1.f, 179.f);
    update();
}

void Camera::update()
{
    auto viewpoint = mPosition + mViewDirection;
    /* 剪裁空间到相机空间的变换矩阵
        相机到剪裁空间需要进行透视投影，故获得透视矩阵的逆矩阵
     */
    cameraFromClip = glm::inverse(glm::perspective(
        glm::radians(mFovy),
        static_cast<float>(mFilm.getWidth()) / static_cast<float>(mFilm.getHeight()),
        1.f,
        2.f));
    /* 相机空间到世界空间的变换矩阵
        物体从世界坐标到相机坐标需要进行相机的变换，故获得相机的逆矩阵
    */
    worldFromCamera = glm::inverse(glm::lookAt(mPosition, viewpoint, {0, 1, 0}));
}