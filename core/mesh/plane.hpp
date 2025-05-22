#pragma once
#include "shape.hpp"

struct Plane : public Shape 
{
    Plane(const glm::vec3 &point,const glm::vec3 &normal) : mPoint(point), mNormal(glm::normalize(normal)) {};
    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;

    // 平面定义
    // 点法式方程: (x - x0) * n.x + (y - y0) * n.y + (z - z0) * n.z = 0
    glm::vec3 mPoint;
    glm::vec3 mNormal;
};