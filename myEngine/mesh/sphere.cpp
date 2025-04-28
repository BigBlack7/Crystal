#include "sphere.hpp"

std::optional<HitInfo> Sphere::intersect(const Ray &ray, float t_min, float t_max) const
{
    glm::vec3 oc = ray.mOrigin - mCenter;//光线起点到圆心
    // at^2 + bt + c = 0, 代入射线方程(ray = o + td)到圆方程(x^2 + y^2 = r^2)
    float a = glm::dot(ray.mDirection, ray.mDirection);
    float b = 2.f * glm::dot(ray.mDirection, oc);
    float c = glm::dot(oc, oc) - mRadius * mRadius;
    float discriminant = b * b - 4.f * a * c;

    // Δ = b² - 4ac小于0, 没有交点
    if (discriminant < 0.f)
    {
        return {};
    }

    // Δ = b² - 4ac大于0，有交点
    // 取较小的t值, 即交点离射线起点更近的交点
    float hit_t = (-b - glm::sqrt(discriminant)) * 0.5f / a;
    if (hit_t < 0.f)
    {
        // 注意: 如果t小于0, 说明交点在射线起点的前面, 不符合要求, 需要考虑较大的根.
        hit_t = (-b + glm::sqrt(discriminant)) * 0.5f / a;
    }
    // 检测交点是否在有效范围
    if (hit_t > t_min && hit_t < t_max)
    {
        glm::vec3 hitPoint = ray.hit(hit_t); // 交点坐标
        glm::vec3 normal = glm::normalize(hitPoint - mCenter); // 交点法向量
        return HitInfo{hit_t, hitPoint, normal};
    }
    return {};
}
