#include "scene.hpp"
#include "until/debugMacro.hpp"
#include <glm/ext/matrix_transform.hpp>

void Scene::addShape(const Shape &shape, const Material *material, const glm::vec3 &position, const glm::vec3 &scale, const glm::vec3 &rotation)
{
    glm::mat4 worldFromObject =
        glm::translate(glm::mat4(1.f), position) *
        glm::rotate(glm::mat4(1.f), glm::radians(rotation.z), glm::vec3(0, 0, 1)) *
        glm::rotate(glm::mat4(1.f), glm::radians(rotation.y), glm::vec3(0, 1, 0)) *
        glm::rotate(glm::mat4(1.f), glm::radians(rotation.x), glm::vec3(1, 0, 0)) *
        glm::scale(glm::mat4(1.f), scale);

    mInstances.push_back(ShapeInstance{shape, material, worldFromObject, glm::inverse(worldFromObject)});
}

// 场景求交, 返回最近的交点信息, 如果没有交点, 返回空
std::optional<HitInfo> Scene::intersect(const Ray &ray, float t_min, float t_max) const
{
    // std::optional<HitInfo> closestHitInfo;
    // const ShapeInstance *closestInstance = nullptr;
    // for (const auto &instance : mInstances)
    // {
    //     auto localRay = ray.objectFromWorld(instance.mObjectFromWorld); 转换到局部坐标系, 求交在本地坐标系下
    //     auto hitInfo = instance.mShape.intersect(localRay, t_min, t_max);
    //     DEBUG_LINE(ray.bounds_test_count += localRay.bounds_test_count)
    //     DEBUG_LINE(ray.triangles_test_count += localRay.triangles_test_count)
    //     if (hitInfo.has_value())
    //     {
    //         closestHitInfo = hitInfo; 有交点, 更新最近交点信息
    //         t_max = hitInfo->mT;      更新t_max, 只需要求最近的交点
    //         closestInstance = &instance; 更新最近交点的实例
    //     }
    // }
    // if (closestInstance)
    // {
    //     closestHitInfo->mHitPoint = closestInstance->mWorldFromObject * glm::vec4(closestHitInfo->mHitPoint, 1); 转换交点到世界坐标系下
    //     closestHitInfo->mNormal = glm::normalize(glm::vec3(glm::transpose(closestInstance->mObjectFromWorld) * glm::vec4(closestHitInfo->mNormal, 0))); 转换法线到世界坐标系下, 法线转换需要使用变换矩阵的转置逆矩阵.
    //     closestHitInfo->mMaterial = &closestInstance->mMaterial;
    // }
    // return closestHitInfo;

    return mSceneBVH.intersect(ray, t_min, t_max);
}
