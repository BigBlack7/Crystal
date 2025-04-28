#pragma once
#include "../ray.hpp"
#include <optional>
#include "../accelerate/bounds.hpp"

struct Shape
{
    // t_min, t_max: 光线的交点和光线起点的距离
    // 交点在t_min和t_max之间才是有效的
    virtual std::optional<HitInfo> intersect(
        const Ray &ray,
        float t_min,
        float t_max) const = 0;

    virtual Bounds getBounds() const { return {}; }
};