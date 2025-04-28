#include "bvh.hpp"
#include <array>
#include "../until/debugMacro.hpp"
#include <iostream>
#pragma warning(push)
#pragma warning(disable : 4267)
#pragma warning(disable : 4244)

void BVH::build(std::vector<Triangle> &&triangles)
{
    auto *root = mAllocator.allocate();
    root->mTriangles = std::move(triangles);
    root->updateBounds();
    root->depth = 1;
    BVHState state{};
    float triangles_count = static_cast<float>(root->mTriangles.size()); // 在初始三角形列表被清空前记录一下三角形数量，后面会被划分去叶节点
    recursiveSplit(root, state);

    // std::cout << "Total node count: " << state.total_node_count << std::endl;
    // std::cout << "Leaf node count: " << state.leaf_node_count << std::endl;
    // std::cout << "Triangles count: " << triangles_count << std::endl;
    // std::cout << "Mean leaf node triangle count: " << triangles_count / static_cast<float>(state.leaf_node_count) << std::endl;
    // std::cout << "Max leaf node triangle count: " << state.max_leaf_node_triangle_count << std::endl;
    // std::cout << "Max leaf node depth: " << state.max_leaf_node_depth << std::endl;

    // 给vector预留足够的空间，避免频繁的重新分配
    mNodes.reserve(state.total_node_count);
    mOrderedTriangles.reserve(triangles_count);
    recursiveFlatten(root);
}

std::optional<HitInfo> BVH::intersect(const Ray &ray, float t_min, float t_max) const
{
    std::optional<HitInfo> closestHitInfo;

    DEBUG_LINE(size_t bounds_test_count = 0, triangles_test_count = 0)

    glm::bvec3 dir_is_neg = glm::bvec3(ray.mDirection.x < 0, ray.mDirection.y < 0, ray.mDirection.z < 0); // 记录射线方向的正负来判断先检测哪个子节点，x正左负右，y正下负上，z正后负前

    glm::vec3 inv_dir = 1.0f / ray.mDirection;

    std::array<int, 32> stack;
    auto ptr = stack.begin();
    size_t current_node_index = 0;
    while (true)
    {
        auto &node = mNodes[current_node_index];
        DEBUG_LINE(bounds_test_count++) // 在相交测试前加一

        if (!node.bounds.hasIntersection(ray, inv_dir, t_min, t_max))
        {
            if (ptr == stack.begin()) // 没有交点且栈为空, 说明已经遍历完所有节点, 退出循环
                break;
            // 否则弹出栈顶元素, 继续遍历
            current_node_index = *(--ptr);
            continue;
        }
        if (node.triangles_count == 0) // 不是叶子节点, 就直接进入它的下一个节点, 因为它的下一个节点就是它的左孩子, 而右孩子不相邻
        {
            if (dir_is_neg[node.split_axis]) // 如果射线方向在当前轴的负方向, 就先左后右
            {
                // 将左孩子入栈
                *(ptr++) = current_node_index + 1;
                // 将当前节点更新为右孩子
                current_node_index = node.child;
            }
            else // 如果射线方向在当前轴的正方向, 就先右后左
            {
                // 将右孩子入栈
                current_node_index++;
                *(ptr++) = node.child;
            }
        }
        else // 是叶子节点, 就遍历它的三角形
        {
            auto triangles_iter = mOrderedTriangles.begin() + node.triangles_index; // 节点的三角形索引起始位置
            DEBUG_LINE(triangles_test_count += node.triangles_count)                // 在三角形相交测试前加上叶子节点三角形数量

            for (size_t i = 0; i < node.triangles_count; ++i)
            {
                auto hitInfo = triangles_iter->intersect(ray, t_min, t_max);
                triangles_iter++;
                if (hitInfo)
                {
                    // 如果有交点, 更新最近交点信息
                    t_max = hitInfo->mT;
                    closestHitInfo = hitInfo;
                }
            }
            // 遍历完三角形后, 弹出栈顶元素, 继续遍历下一个节点
            if (ptr == stack.begin())
                break;
            current_node_index = *(--ptr);
        }
    }
    DEBUG_LINE(ray.bounds_test_count += bounds_test_count)
    DEBUG_LINE(ray.triangles_test_count += triangles_test_count)

    return closestHitInfo;
}

void BVH::recursiveSplit(BVHTreeNode *node, BVHState &state)
{
    state.total_node_count++;                             // 每递归进来一次, 就增加一个节点
    if (node->mTriangles.size() == 1 || node->depth > 32) // 节点只有一个三角形或者深度超过32就不再分割(相交测试的栈深度为32)，认为它是叶子节点
    {
        state.addLeafNode(node);
        return;
    }

    auto diag = node->bounds.diagonal();                          // 计算包围盒的对角线长度
    float min_cost = std::numeric_limits<float>::infinity();      // SAH算法的最小成本
    size_t min_split_index = 0;                                   // 记录最小成本的分割索引
    Bounds min_leftBounds{}, min_rightBounds{};                   // 记录最小成本的左右子树的包围盒
    size_t min_leftTriangleCount = 0, min_rightTriangleCount = 0; // 记录最小成本的左右子树的三角形数量
    constexpr size_t bucket_count = 12;
    std::vector<size_t> triangle_indices_buckets[3][bucket_count] = {}; // 记录每个桶中的三角形索引, 三维数组, 第一维是轴, 第二维是桶, 第三维是三角形索引

    for (size_t axis = 0; axis < 3; axis++) // 遍历三个轴，计算哪一个轴的SAH开销最小
    {
        Bounds bounds_bucket[bucket_count] = {};          // 桶的包围盒
        size_t triangle_count_buckets[bucket_count] = {}; // 记录每个桶中的三角形数量
        size_t triangle_idx = 0;
        for (const auto &triangle : node->mTriangles)
        {
            auto triangle_center = (triangle.p0[axis] + triangle.p1[axis] + triangle.p2[axis]) * 0.333333333333f;                                                // 计算三角形的中心坐标
            size_t bucket_idx = glm::clamp<size_t>(glm::floor((triangle_center - node->bounds.b_min[axis]) * bucket_count / diag[axis]), 0.f, bucket_count - 1); // 计算三角形的桶索引
            // 拓展桶的包围盒
            bounds_bucket[bucket_idx].expand(triangle.p0);
            bounds_bucket[bucket_idx].expand(triangle.p1);
            bounds_bucket[bucket_idx].expand(triangle.p2);

            triangle_count_buckets[bucket_idx]++;                               // 对应桶中的三角形数量加1
            triangle_indices_buckets[axis][bucket_idx].push_back(triangle_idx); // 记录三角形索引
            triangle_idx++;                                                     // 三角形索引加1
        }

        // 将所有桶按索引视为左右两边
        Bounds leftBounds = bounds_bucket[0];
        size_t leftTriangleCount = triangle_count_buckets[0];
        for (size_t i = 1; i <= bucket_count - 1; i++)
        {
            Bounds rightBounds{};
            size_t rightTriangleCount = 0;
            // 更新一下右边桶
            for (size_t j = bucket_count - 1; j >= i; j--)
            {
                rightBounds.expand(bounds_bucket[j]);
                rightTriangleCount += triangle_count_buckets[j];
            }
            if (rightTriangleCount == 0) // 右桶
            {
                break;
            }
            if (leftTriangleCount != 0) // 左桶
            {
                // 计算cost
                float cost = leftBounds.area() * static_cast<float>(leftTriangleCount) + rightBounds.area() * static_cast<float>(rightTriangleCount);
                if (cost < min_cost) // 找到最小成本的分割
                {
                    min_cost = cost;                             // 更新最小成本
                    node->split_axis = axis;                     // 更新分割轴
                    min_split_index = i;                         // 更新分割索引
                    min_leftBounds = leftBounds;                 // 更新左子树的包围盒
                    min_rightBounds = rightBounds;               // 更新右子树的包围盒
                    min_leftTriangleCount = leftTriangleCount;   // 更新左子树的三角形数量
                    min_rightTriangleCount = rightTriangleCount; // 更新右子树的三角形数量
                }
            }
            // 更新一下左边桶
            leftBounds.expand(bounds_bucket[i]);
            leftTriangleCount += triangle_count_buckets[i];
        }
    }

    if (min_split_index == 0) // 表明不可被分割为两个子节点，认为它是叶子节点
    {
        state.addLeafNode(node);
        return;
    }

    auto *leftNode = mAllocator.allocate();
    auto *rightNode = mAllocator.allocate();
    node->children[0] = leftNode;
    node->children[1] = rightNode;

    // 为左右子节点的三角形列表分配足够的空间, 避免频繁的重新分配
    leftNode->mTriangles.reserve(min_leftTriangleCount);
    rightNode->mTriangles.reserve(min_rightTriangleCount);

    // 根据桶内的索引初始化左右子节点的三角形列表
    for (size_t i = 0; i < min_split_index; i++)
    {
        for (size_t idx : triangle_indices_buckets[node->split_axis][i])
        {
            leftNode->mTriangles.push_back(node->mTriangles[idx]);
        }
    }
    for (size_t i = min_split_index; i < bucket_count; i++)
    {
        for (size_t idx : triangle_indices_buckets[node->split_axis][i])
        {
            rightNode->mTriangles.push_back(node->mTriangles[idx]);
        }
    }

    // 清空父节点的三角形列表, 因为它已经被分割为两个子节点了, 不需要再存储
    node->mTriangles.clear();
    node->mTriangles.shrink_to_fit();
    // 更新孩子节点深度
    leftNode->depth = node->depth + 1;
    rightNode->depth = node->depth + 1;
    // 更新孩子节点的包围盒
    leftNode->bounds = min_leftBounds;
    rightNode->bounds = min_rightBounds;
    recursiveSplit(leftNode, state);
    recursiveSplit(rightNode, state);
}

// BVHTreeNode的大小远大于BVHNode, 会使cache miss严重, 深度dfs树形结构(左孩子会与父节点相邻，故父节点只需存储右孩子的索引)，转化为线性结构, 可以减少cache miss.
size_t BVH::recursiveFlatten(BVHTreeNode *node)
{
    BVHNode bvhNode{node->bounds, 0, static_cast<uint16_t>(node->mTriangles.size()), static_cast<uint8_t>(node->split_axis)};
    auto index = mNodes.size();
    mNodes.push_back(bvhNode);
    if (bvhNode.triangles_count == 0) // 如果不是叶子节点, 就递归遍历它的子节点
    {
        recursiveFlatten(node->children[0]);                       // 遍历左孩子，左孩子与父节点相邻，故不需要存储左孩子的索引
        mNodes[index].child = recursiveFlatten(node->children[1]); // 遍历右孩子，并将右孩子的索引存储在父节点的child字段中
    }
    else // 如果是叶子节点
    {
        mNodes[index].triangles_index = mOrderedTriangles.size(); // 这个节点的三角形起始索引位置就是当前的三角形数量
        for (const auto &triangle : node->mTriangles)
        {
            // 遍历三角形, 并将它们的索引存储在mOrderedTriangles数组中
            mOrderedTriangles.push_back(triangle);
        }
    }
    return index;
}