#include "triangle.hpp"

std::optional<HitInfo> Triangle::intersect(const Ray &ray, float t_min, float t_max) const
{
    // o + td = (1 - u - v)p0 + u * p0 + v * p1
    // o - p0 = -td + u(p1 - p0) + v(p2 - p0)
    // e0 = p1 - p0, e1 = p2 - p0, s = o - p0
    // (-d, e1, e0)dot(t, u, v) = s
    // s1 = cross(d, e1), s2 = cross(s, e0)
    // (t , u , v) = inv(s1 dot e0) * (s2 dot e1, s1 dot s, s2 dot d)

    glm::vec3 e0 = p1 - p0;
    glm::vec3 e1 = p2 - p0;
    glm::vec3 s1 = glm::cross(ray.mDirection, e1);
    float invDet = 1.f / glm::dot(s1, e0);

    glm::vec3 s = ray.mOrigin - p0;
    float u = glm::dot(s1, s) * invDet;
    if (u < 0.f || u > 1.f)
        return {};

    glm::vec3 s2 = glm::cross(s, e0);
    float v = glm::dot(s2, ray.mDirection) * invDet;
    if (v < 0.f || u + v > 1.f)
        return {};

    float hit_t = glm::dot(s2, e1) * invDet;
    if (hit_t > t_min && hit_t < t_max)
    {
        glm::vec3 hitPoint = ray.hit(hit_t);
        glm::vec3 normal = (1.f - u - v) * n0 + u * n1 + v * n2; // 插值计算法线
        return HitInfo{hit_t, hitPoint, glm::normalize(normal)};
    }
    return {};
}
