#include "model.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <rapidobj/rapidobj.hpp>

// Model::Model(const std::filesystem::path &fileName)
// {
//     std::vector<glm::vec3> positions;
//     std::vector<glm::vec3> normals;
//     std::ifstream file(fileName);
//     if (!file.is_open())
//     {
//         std::cerr << "Failed to load Model: " << fileName << std::endl;
//         return;
//     }
//     std::string line;
//     while (std::getline(file, line))
//     {
//         std::istringstream iss(line);
//         std::string prefix;
//         iss >> prefix;
//         if (prefix == "v")
//         {
//             glm::vec3 position;
//             iss >> position.x >> position.y >> position.z;
//             positions.push_back(position);
//         }
//         else if (prefix == "vn")
//         {
//             glm::vec3 normal;
//             iss >> normal.x >> normal.y >> normal.z;
//             normals.push_back(normal);
//         }
//         else if (prefix == "f")
//         {
//             glm::ivec3 idx_v, idx_vn;
//             char sep;
//             for (int i = 0; i < 3; ++i)
//             {
//                 iss >> idx_v[i] >> sep >> sep >> idx_vn[i];
//             }
//             mTriangles.push_back(Triangle(
//                 positions[idx_v.x - 1], positions[idx_v.y - 1], positions[idx_v.z - 1],
//                 normals[idx_vn.x - 1], normals[idx_vn.y - 1], normals[idx_vn.z - 1]));
//         }
//     }
//     if (mTriangles.empty())
//     {
//         std::cerr << "Warning: No triangles loaded from " << fileName << std::endl;
//     }
//     buildBounds();
// }

Model::Model(const std::filesystem::path &fileName)
{
    auto result = rapidobj::ParseFile(fileName, rapidobj::MaterialLibrary::Ignore());
    std::vector<Triangle> triangles;
    for (const auto &shape : result.shapes)
    {
        size_t index_offset = 0;
        for (size_t num_face_vertices : shape.mesh.num_face_vertices)
        {
            if (num_face_vertices == 3)
            {
                auto index = shape.mesh.indices[index_offset];
                glm::vec3 pos0{
                    result.attributes.positions[3 * index.position_index + 0],
                    result.attributes.positions[3 * index.position_index + 1],
                    result.attributes.positions[3 * index.position_index + 2]};

                index = shape.mesh.indices[index_offset + 1];
                glm::vec3 pos1{
                    result.attributes.positions[3 * index.position_index + 0],
                    result.attributes.positions[3 * index.position_index + 1],
                    result.attributes.positions[3 * index.position_index + 2]};

                index = shape.mesh.indices[index_offset + 2];
                glm::vec3 pos2{
                    result.attributes.positions[3 * index.position_index + 0],
                    result.attributes.positions[3 * index.position_index + 1],
                    result.attributes.positions[3 * index.position_index + 2]};

                if (index.normal_index >= 0)
                {
                    index = shape.mesh.indices[index_offset];
                    glm::vec3 normal0{
                        result.attributes.normals[3 * index.normal_index + 0],
                        result.attributes.normals[3 * index.normal_index + 1],
                        result.attributes.normals[3 * index.normal_index + 2]};

                    index = shape.mesh.indices[index_offset + 1];
                    glm::vec3 normal1{
                        result.attributes.normals[3 * index.normal_index + 0],
                        result.attributes.normals[3 * index.normal_index + 1],
                        result.attributes.normals[3 * index.normal_index + 2]};

                    index = shape.mesh.indices[index_offset + 2];
                    glm::vec3 normal2{
                        result.attributes.normals[3 * index.normal_index + 0],
                        result.attributes.normals[3 * index.normal_index + 1],
                        result.attributes.normals[3 * index.normal_index + 2]};

                    triangles.push_back(Triangle(pos0, pos1, pos2, normal0, normal1, normal2));
                }
                else
                {
                    triangles.push_back(Triangle(pos0, pos1, pos2));
                }
            }
            index_offset += num_face_vertices;
        }
    }

    if (triangles.empty())
    {
        std::cerr << "Warning: No triangles loaded from " << fileName << std::endl;
    }
    mBVH.build(std::move(triangles));
}

std::optional<HitInfo> Model::intersect(const Ray &ray, float t_min, float t_max) const
{
    return mBVH.intersect(ray, t_min, t_max);
}