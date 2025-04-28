#pragma once
#include "bounds.hpp"
#include "../mesh/triangle.hpp"
#include <vector>

struct BVHTreeNode
{
    Bounds bounds{};                  // 存储自己的包围盒
    std::vector<Triangle> mTriangles; // 存储三角形
    BVHTreeNode *children[2];         // 存储子节点
    size_t depth;                     // 存储本节点深度
    size_t split_axis;

    void updateBounds()
    {
        bounds = {};
        for (const auto &triangle : mTriangles)
        {
            bounds.expand(triangle.getBounds());
        }
    }
};

// 尽可能的减小这个结构体的大小，使得在递归过程中能提高cache的命中率
struct alignas(32) BVHNode
{
    Bounds bounds{};

    union
    {
        int child;           // 只有非叶子节点才用这个
        int triangles_index; // 只有叶子节点才用这个
    };
    uint16_t triangles_count; // 记录三角形的索引和数量在数组中定位该节点的三角形
    uint8_t split_axis;       // 记录分割轴xyz
};

struct BVHState // BVH构建状态
{
    size_t total_node_count{};             // 总节点数
    size_t leaf_node_count{};              // 叶子节点数
    size_t max_leaf_node_triangle_count{}; // 最大叶子节点三角形数
    size_t max_leaf_node_depth{};          // 最大叶子节点深度

    void addLeafNode(BVHTreeNode *node) // 更新叶子节点信息
    {
        leaf_node_count++;                                                                              // 叶子节点数加1
        max_leaf_node_triangle_count = glm::max(max_leaf_node_triangle_count, node->mTriangles.size()); // 更新最大叶子节点三角形数
        max_leaf_node_depth = glm::max(max_leaf_node_depth, node->depth);                               // 更新最大叶子节点深度
    }
};

// 如果每一个节点都在建树时new就会有巨大的开销，提前将内存池分配好，避免频繁的new和delete
struct BVHTreeNodeAllcator
{
public:
    // 初始化 ptr 为 4096，使得第一次调用 allocate 时会分配新的内存块
    BVHTreeNodeAllcator() : ptr(4096) {}
    BVHTreeNode *allocate()
    {
        // 检查当前内存块是否已分配完 4096 个节点
        if (ptr == 4096)
        {
            // 分配一个新的包含 4096 个 BVHTreeNode 对象的数组，并将其指针添加到 nodes_list 中
            nodes_list.push_back(new BVHTreeNode[4096]);
            // 重置 ptr 为 0，开始记录新内存块的分配位置
            ptr = 0;
        }
        // 返回当前内存块中第 ptr 个 BVHTreeNode 对象的地址，并将 ptr 加 1
        return &nodes_list.back()[ptr++];
    }
    ~BVHTreeNodeAllcator()
    {
        for (auto *nodes : nodes_list)
        {
            delete[] nodes;
        }
        nodes_list.clear();
    }

private:
    size_t ptr;
    std::vector<BVHTreeNode *> nodes_list; // 存储一块块的内存块，每块4096
};

class BVH : public Shape
{
public:
    void build(std::vector<Triangle> &&triangles);
    std::optional<HitInfo> intersect(const Ray &ray, float t_min, float t_max) const override;
    Bounds getBounds() const override { return mNodes[0].bounds; }

private:
    void recursiveSplit(BVHTreeNode *node, BVHState &state);
    size_t recursiveFlatten(BVHTreeNode *node);

private:
    BVHTreeNodeAllcator mAllocator{};
    std::vector<BVHNode> mNodes;             // 将BVHTreeNode转化为BVHNode, 减少内存占用，从树形结构转化为线性结构
    std::vector<Triangle> mOrderedTriangles; // 总三角形数组，用于存储所有三角形，方便快速访问
};