#include "bounds.hpp"

/**
 * @brief 判断射线是否与轴对齐包围盒相交
 *
 * @param ray 待检测的射线
 * @param t_min 射线参数 t 的最小值
 * @param t_max 射线参数 t 的最大值
 * @return true 射线与包围盒相交
 * @return false 射线与包围盒不相交
 */
bool Bounds::hasIntersection(const Ray &ray, float t_min, float t_max) const
{
    // 注释：对于垂直于 y 轴的平面 y = A，射线方程 (o + td).y = A 可推导出 t = (A - o.y) / d.y
    // 同理，对于 x 轴：t = (A - o.x) / d.x，对于 z 轴：t = (A - o.z) / d.z
    // 计算射线方向的倒数，避免重复计算除法
    glm::vec3 inv_direction = 1.0f / ray.mDirection;
    // 计算射线与包围盒最小面相交的参数 t 值
    glm::vec3 t1 = (b_min - ray.mOrigin) * inv_direction;
    // 计算射线与包围盒最大面相交的参数 t 值
    glm::vec3 t2 = (b_max - ray.mOrigin) * inv_direction;

    // 对于每个坐标轴，找出射线进入包围盒的最小 t 值
    glm::vec3 tmin = glm::min(t1, t2);
    // 对于每个坐标轴，找出射线离开包围盒的最大 t 值
    glm::vec3 tmax = glm::max(t1, t2);

    // 找出所有坐标轴上的最大 tmin 值，即射线进入包围盒的最近参数 t
    float t_near = glm::max(tmin.x, glm::max(tmin.y, tmin.z));
    // 找出所有坐标轴上的最小 tmax 值，即射线离开包围盒的最远参数 t
    float t_far = glm::min(tmax.x, glm::min(tmax.y, tmax.z));
    // 判断射线进入和离开包围盒的参数 t 是否在给定范围内，如果是则相交
    return glm::max(t_near, t_min) <= glm::min(t_far, t_max);
}

// 将方向的倒数作为参数传入，避免重复计算, 除法比较消耗性能.
bool Bounds::hasIntersection(const Ray &ray, const glm::vec3 &inv_direction, float t_min, float t_max) const
{
    glm::vec3 t1 = (b_min - ray.mOrigin) * inv_direction;
    glm::vec3 t2 = (b_max - ray.mOrigin) * inv_direction;

    glm::vec3 tmin = glm::min(t1, t2);
    glm::vec3 tmax = glm::max(t1, t2);

    float t_near = glm::max(tmin.x, glm::max(tmin.y, tmin.z));
    float t_far = glm::min(tmax.x, glm::min(tmax.y, tmax.z));
    return glm::max(t_near, t_min) <= glm::min(t_far, t_max);
}