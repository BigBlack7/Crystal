#pragma once
#include "triangle.hpp"
#include "../accelerate/bvh.hpp"
#include <vector>
#include <filesystem>
class Model : public Shape
{
public:
    Model(const std::vector<Triangle> &triangles)
    {
        auto ts = triangles;
        mBVH.build(std::move(ts));
    }
    Model(const std::filesystem::path &fileName);

    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;
    Bounds getBounds() const override { return mBVH.getBounds(); }

private:
    BVH mBVH{};
};