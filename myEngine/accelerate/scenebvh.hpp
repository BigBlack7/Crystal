#pragma once
#include "bounds.hpp"
#include "../mesh/shape.hpp"
#include <vector>

struct ShapeInstance
{
    const Shape &mShape;
    const Material *mMaterial;
    glm::mat4 mWorldFromObject; // world
    glm::mat4 mObjectFromWorld; // local

    Bounds bounds{};
    glm::vec3 mCenter{};

    void updateBounds()
    {
        bounds = {};
        auto bounds_local = mShape.getBounds();
        for (size_t i = 0; i < 8; i++)
        {
            auto corner_local = bounds_local.getCorner(i);
            glm::vec3 corner_world = mWorldFromObject * glm::vec4(corner_local, 1.f);
            bounds.expand(corner_world);
        }
        mCenter = (bounds.b_min + bounds.b_max) * 0.5f;
    }
};

struct SceneBVHTreeNode
{
    Bounds bounds{};
    std::vector<ShapeInstance> mInstances;
    SceneBVHTreeNode *children[2];
    size_t depth;
    size_t split_axis;

    void updateBounds()
    {
        bounds = {};
        for (const auto &instance : mInstances)
        {
            bounds.expand(instance.bounds);
        }
    }
};

struct alignas(32) SceneBVHNode
{
    Bounds bounds{};
    union
    {
        int child;
        int instances_index;
    };
    uint16_t instances_count;
    uint8_t split_axis;
};

struct SceneBVHState
{
    size_t total_node_count{};
    size_t leaf_node_count{};
    size_t max_leaf_node_instance_count{};
    size_t max_leaf_node_depth{};

    void addLeafNode(SceneBVHTreeNode *node)
    {
        leaf_node_count++;
        max_leaf_node_instance_count = glm::max(max_leaf_node_instance_count, node->mInstances.size());
        max_leaf_node_depth = glm::max(max_leaf_node_depth, node->depth);
    }
};

struct SceneBVHTreeNodeAllcator
{
public:
    SceneBVHTreeNodeAllcator() : ptr(4096) {}
    SceneBVHTreeNode *allocate()
    {
        if (ptr == 4096)
        {
            nodes_list.push_back(new SceneBVHTreeNode[4096]);
            ptr = 0;
        }
        return &nodes_list.back()[ptr++];
    }
    ~SceneBVHTreeNodeAllcator()
    {
        for (auto *nodes : nodes_list)
        {
            delete[] nodes;
        }
        nodes_list.clear();
    }

private:
    size_t ptr;
    std::vector<SceneBVHTreeNode *> nodes_list;
};

class SceneBVH : public Shape
{
public:
    void build(std::vector<ShapeInstance> &&instances);
    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;
    Bounds getBounds() const override { return mNodes[0].bounds; }

private:
    void recursiveSplit(SceneBVHTreeNode *node, SceneBVHState &state);
    size_t recursiveFlatten(SceneBVHTreeNode *node);

private:
    SceneBVHTreeNodeAllcator mAllocator{};
    std::vector<SceneBVHNode> mNodes;
    SceneBVHTreeNode *root;
    std::vector<ShapeInstance> mOrderedInstances;
    std::vector<ShapeInstance> mInfinityInstances;
};