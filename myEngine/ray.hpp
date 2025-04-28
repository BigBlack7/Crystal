#pragma once
#include <glm/glm.hpp>
#include "material/material.hpp"
#include "until/debugMacro.hpp"

struct Ray
{
    glm::vec3 mOrigin;
    glm::vec3 mDirection;

    glm::vec3 hit(float t) const
    {
        return mOrigin + t * mDirection;
    }

    Ray objectFromWorld(const glm::mat4 &_objectFromWorld) const;

    // mutable 关键字表示该const变量可以被修改
    DEBUG_LINE(mutable size_t bounds_test_count = 0)    // 包围盒相交测试次数
    DEBUG_LINE(mutable size_t triangles_test_count = 0) // 三角形相交测试次数
};

struct HitInfo
{
    float mT;
    glm::vec3 mHitPoint;
    glm::vec3 mNormal;
    const Material *mMaterial{nullptr};
};