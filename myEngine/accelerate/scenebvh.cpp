#include "sceneBVH.hpp"
#include <array>
#include "../until/debugMacro.hpp"
#include <iostream>
#pragma warning(push)
#pragma warning(disable : 4267)
#pragma warning(disable : 4244)

void SceneBVH::build(std::vector<ShapeInstance> &&instances)
{
    root = mAllocator.allocate();
    auto temp_instances = std::move(instances);
    for (auto &instance : temp_instances)
    {
        if (instance.mShape.getBounds().isValid())
        {
            instance.updateBounds();
            root->mInstances.push_back(instance);
        }
        else
        {
            mInfinityInstances.push_back(instance);
        }
    }

    root->updateBounds();
    root->depth = 1;
    SceneBVHState state{};
    float instances_count = static_cast<float>(root->mInstances.size());
    recursiveSplit(root, state);

    // std::cout << "Total node count: " << state.total_node_count << std::endl;
    // std::cout << "Leaf node count: " << state.leaf_node_count << std::endl;
    // std::cout << "ShapeInstances count: " << instances_count << std::endl;
    // std::cout << "Mean leaf node ShapeInstance count: " << instances_count / static_cast<float>(state.leaf_node_count) << std::endl;
    // std::cout << "Max leaf node ShapeInstance count: " << state.max_leaf_node_instance_count << std::endl;
    // std::cout << "Max leaf node depth: " << state.max_leaf_node_depth << std::endl;

    mNodes.reserve(state.total_node_count);
    mOrderedInstances.reserve(instances_count);
    recursiveFlatten(root);
}

std::optional<HitInfo> SceneBVH::intersect(const Ray &ray, float t_min, float t_max) const
{
    std::optional<HitInfo> closestHitInfo;
    const ShapeInstance *closestInstance = nullptr;

    DEBUG_LINE(size_t bounds_test_count = 0)

    glm::bvec3 dir_is_neg = glm::bvec3(ray.mDirection.x < 0, ray.mDirection.y < 0, ray.mDirection.z < 0);

    glm::vec3 inv_dir = 1.0f / ray.mDirection;

    std::array<int, 32> stack;
    auto ptr = stack.begin();
    size_t current_node_index = 0;
    while (true)
    {
        auto &node = mNodes[current_node_index];
        DEBUG_LINE(bounds_test_count++)

        if (!node.bounds.hasIntersection(ray, inv_dir, t_min, t_max))
        {
            if (ptr == stack.begin())
                break;
            current_node_index = *(--ptr);
            continue;
        }
        if (node.instances_count == 0)
        {
            if (dir_is_neg[node.split_axis])
            {
                *(ptr++) = current_node_index + 1;
                current_node_index = node.child;
            }
            else
            {
                current_node_index++;
                *(ptr++) = node.child;
            }
        }
        else
        {
            auto instances_iter = mOrderedInstances.begin() + node.instances_index;
            for (size_t i = 0; i < node.instances_count; ++i)
            {
                auto localRay = ray.objectFromWorld(instances_iter->mObjectFromWorld);
                auto hitInfo = instances_iter->mShape.intersect(localRay, t_min, t_max);
                DEBUG_LINE(ray.bounds_test_count += localRay.bounds_test_count)
                DEBUG_LINE(ray.triangles_test_count += localRay.triangles_test_count)
                if (hitInfo)
                {
                    t_max = hitInfo->mT;
                    closestHitInfo = hitInfo;
                    closestInstance = &(*instances_iter);
                }
                instances_iter++;
            }
            if (ptr == stack.begin())
                break;
            current_node_index = *(--ptr);
        }
    }

    for (const auto &infinityInstance: mInfinityInstances)
    {
        auto localRay = ray.objectFromWorld(infinityInstance.mObjectFromWorld);
        auto hitInfo = infinityInstance.mShape.intersect(localRay, t_min, t_max);
        DEBUG_LINE(ray.bounds_test_count += localRay.bounds_test_count)
        DEBUG_LINE(ray.triangles_test_count += localRay.triangles_test_count)
        if (hitInfo)
        {
            t_max = hitInfo->mT;
            closestHitInfo = hitInfo;
            closestInstance = &infinityInstance;
        }
    }

    if (closestInstance)
    {
        closestHitInfo->mHitPoint = closestInstance->mWorldFromObject * glm::vec4(closestHitInfo->mHitPoint, 1);
        closestHitInfo->mNormal = glm::normalize(glm::vec3(glm::transpose(closestInstance->mObjectFromWorld) * glm::vec4(closestHitInfo->mNormal, 0)));
        closestHitInfo->mMaterial = closestInstance->mMaterial;
    }

    DEBUG_LINE(ray.bounds_test_count += bounds_test_count)
    return closestHitInfo;
}

void SceneBVH::recursiveSplit(SceneBVHTreeNode *node, SceneBVHState &state)
{
    state.total_node_count++;
    if (node->mInstances.size() == 1 || node->depth > 32)
    {
        state.addLeafNode(node);
        return;
    }
    auto diag = node->bounds.diagonal();
    float min_cost = std::numeric_limits<float>::infinity();
    size_t min_split_index = 0;
    Bounds min_leftBounds{}, min_rightBounds{};
    size_t min_leftInstanceCount = 0, min_rightInstanceCount = 0;
    constexpr size_t bucket_count = 12;
    std::vector<size_t> instance_indices_buckets[3][bucket_count] = {};
    for (size_t axis = 0; axis < 3; axis++)
    {
        Bounds bounds_bucket[bucket_count] = {};
        size_t instance_count_buckets[bucket_count] = {};
        size_t instance_idx = 0;
        for (const auto &instance : node->mInstances)
        {
            size_t bucket_idx = glm::clamp<size_t>(glm::floor((instance.mCenter[axis] - node->bounds.b_min[axis]) * bucket_count / diag[axis]), 0.f, bucket_count - 1);
            bounds_bucket[bucket_idx].expand(instance.bounds);
            instance_count_buckets[bucket_idx]++;
            instance_indices_buckets[axis][bucket_idx].push_back(instance_idx);
            instance_idx++;
        }

        Bounds leftBounds = bounds_bucket[0];
        size_t leftInstanceCount = instance_count_buckets[0];
        for (size_t i = 1; i <= bucket_count - 1; i++)
        {
            Bounds rightBounds{};
            size_t rightInstanceCount = 0;
            for (size_t j = bucket_count - 1; j >= i; j--)
            {
                rightBounds.expand(bounds_bucket[j]);
                rightInstanceCount += instance_count_buckets[j];
            }
            if (rightInstanceCount == 0)
            {
                break;
            }
            if (leftInstanceCount != 0)
            {
                float cost = leftBounds.area() * static_cast<float>(leftInstanceCount) + rightBounds.area() * static_cast<float>(rightInstanceCount);
                if (cost < min_cost)
                {
                    min_cost = cost;
                    node->split_axis = axis;
                    min_split_index = i;
                    min_leftBounds = leftBounds;
                    min_rightBounds = rightBounds;
                    min_leftInstanceCount = leftInstanceCount;
                    min_rightInstanceCount = rightInstanceCount;
                }
            }
            leftBounds.expand(bounds_bucket[i]);
            leftInstanceCount += instance_count_buckets[i];
        }
        // for (size_t i = 0; i < 11; i++)
        // {
        //     float mid = node->bounds.b_min[axis] + diag[axis] * (i + 1.f) * 0.0833333f;
        //     Bounds leftBounds{}, rightBounds{};
        //     std::vector<ShapeInstance> leftShapeInstances_temp, rightShapeInstances_temp;
        //     for (const auto &ShapeInstance : node->mShapeInstances)
        //     {
        //         if ((ShapeInstance.p0[axis] + ShapeInstance.p1[axis] + ShapeInstance.p2[axis]) / 3.0f < mid)
        //         {
        //             leftBounds.expand(ShapeInstance.p0);
        //             leftBounds.expand(ShapeInstance.p1);
        //             leftBounds.expand(ShapeInstance.p2);
        //             leftShapeInstances_temp.push_back(ShapeInstance);
        //         }
        //         else
        //         {
        //             rightBounds.expand(ShapeInstance.p0);
        //             rightBounds.expand(ShapeInstance.p1);
        //             rightBounds.expand(ShapeInstance.p2);
        //             rightShapeInstances_temp.push_back(ShapeInstance);
        //         }
        //     }
        //     if (leftShapeInstances_temp.empty() || rightShapeInstances_temp.empty())
        //     {
        //         continue;
        //     }
        //     float cost = leftBounds.area() * static_cast<float>(leftShapeInstances_temp.size()) + rightBounds.area() * static_cast<float>(rightShapeInstances_temp.size());
        //     if (cost < min_cost)
        //     {
        //         min_cost = cost;
        //         leftShapeInstances = std::move(leftShapeInstances_temp);
        //         rightShapeInstances = std::move(rightShapeInstances_temp);
        //         node->split_axis = axis;
        //     }
        // }
    }

    if (min_split_index == 0)
    {
        state.addLeafNode(node);
        return;
    }

    auto *leftNode = mAllocator.allocate();
    auto *rightNode = mAllocator.allocate();
    node->children[0] = leftNode;
    node->children[1] = rightNode;

    leftNode->mInstances.reserve(min_leftInstanceCount);
    rightNode->mInstances.reserve(min_rightInstanceCount);
    for (size_t i = 0; i < min_split_index; i++)
    {
        for (size_t idx : instance_indices_buckets[node->split_axis][i])
        {
            leftNode->mInstances.push_back(node->mInstances[idx]);
        }
    }
    for (size_t i = min_split_index; i < bucket_count; i++)
    {
        for (size_t idx : instance_indices_buckets[node->split_axis][i])
        {
            rightNode->mInstances.push_back(node->mInstances[idx]);
        }
    }

    node->mInstances.clear();
    node->mInstances.shrink_to_fit();
    leftNode->depth = node->depth + 1;
    rightNode->depth = node->depth + 1;
    leftNode->bounds = min_leftBounds;
    rightNode->bounds = min_rightBounds;
    recursiveSplit(leftNode, state);
    recursiveSplit(rightNode, state);
}

size_t SceneBVH::recursiveFlatten(SceneBVHTreeNode *node)
{
    SceneBVHNode sceneBVHNode{node->bounds, 0, static_cast<uint16_t>(node->mInstances.size()), static_cast<uint8_t>(node->split_axis)};
    auto index = mNodes.size();
    mNodes.push_back(sceneBVHNode);
    if (sceneBVHNode.instances_count == 0)
    {
        recursiveFlatten(node->children[0]);
        mNodes[index].child = recursiveFlatten(node->children[1]);
    }
    else
    {
        mNodes[index].instances_index = mOrderedInstances.size();
        for (const auto &instance : node->mInstances)
        {
            mOrderedInstances.push_back(instance);
        }
    }
    return index;
}