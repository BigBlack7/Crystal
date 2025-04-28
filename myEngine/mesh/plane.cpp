#include "plane.hpp"

std::optional<HitInfo> Plane::intersect(const Ray &ray, float t_min, float t_max) const
{
    // (o + td - p) dot n = 0
    // (o - p) dot n = -t(d dot n)
    // t = (p - o) dot n / (d dot n)
    // o是射线起点，d是射线方向，p是平面上的点，n是平面法向量
    float hit_t = glm::dot(mPoint - ray.mOrigin, mNormal) / glm::dot(ray.mDirection, mNormal);
    if (hit_t > t_min && hit_t < t_max)
    {
        return HitInfo{hit_t, ray.hit(hit_t), mNormal};
    }
    return {};
}
