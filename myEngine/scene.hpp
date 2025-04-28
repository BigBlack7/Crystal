#pragma once
#include "mesh/shape.hpp"
#include "accelerate/scenebvh.hpp"
#include <vector>

struct Scene : public Shape
{
public:
    void addShape(const Shape &shape,
                  const Material *material = nullptr,
                  const glm::vec3 &position = {0, 0, 0},
                  const glm::vec3 &scale = {1, 1, 1},
                  const glm::vec3 &rotation = {0, 0, 0});

    std::optional<HitInfo> intersect(
        const Ray &ray,
        float t_min = 1e-5,
        float t_max = std::numeric_limits<float>::infinity()) const override;

    void build() { mSceneBVH.build(std::move(mInstances)); }

private:
    std::vector<ShapeInstance> mInstances;
    SceneBVH mSceneBVH{};
};