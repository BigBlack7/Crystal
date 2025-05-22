#pragma once
#include "shape.hpp"

struct Sphere : public Shape
{
    Sphere(const glm::vec3 &center, float radius) : mCenter(center), mRadius(radius) {}
    glm::vec3 mCenter;
    float mRadius;

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override; // 相交检测
    Bounds getBounds() const override { return {mCenter - mRadius, mCenter + mRadius}; }
};