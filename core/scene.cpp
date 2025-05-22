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
    return mSceneBVH.intersect(ray, t_min, t_max);
}
