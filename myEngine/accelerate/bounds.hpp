#pragma once
#include <glm/glm.hpp>

#include "../ray.hpp"

struct Bounds
{
    Bounds() : b_min(std::numeric_limits<float>::infinity()), b_max(-std::numeric_limits<float>::infinity()) {};
    Bounds(const glm::vec3 &b_min, const glm::vec3 &b_max) : b_min(b_min), b_max(b_max) {};

    void expand(const glm::vec3 &pos) // 拓展包围盒
    {
        b_min = glm::min(b_min, pos);
        b_max = glm::max(b_max, pos);
    }

    void expand(const Bounds &bounds) // 包围盒拓展自身
    {
        b_min = glm::min(bounds.b_min, b_min);
        b_max = glm::max(bounds.b_max, b_max);
    }

    bool hasIntersection(const Ray &ray, float t_min, float t_max) const; // 射线与包围盒是否相交
    bool hasIntersection(const Ray &ray, const glm::vec3 &inv_direction, float t_min, float t_max) const;

    glm::vec3 diagonal() const { return b_max - b_min; } // 计算包围盒的对角线

    float area() const // 计算包围盒的表面积
    {
        auto diag = diagonal();
        return (diag.x * (diag.y + diag.z) + diag.y * diag.z) * 2.f;
    }

    glm::vec3 getCorner(size_t index) const
    {
        auto corner = b_max;
        if ((index & 0b1) == 0)
            corner.x = b_min.x;
        if ((index & 0b10) == 0)
            corner.y = b_min.y;
        if ((index & 0b100) == 0)
            corner.z = b_min.z;
        return corner;
    }

    bool isValid() const { return b_max.x >= b_min.x && b_max.y >= b_min.y && b_max.z >= b_min.z; }

    glm::vec3 b_min;
    glm::vec3 b_max;
};