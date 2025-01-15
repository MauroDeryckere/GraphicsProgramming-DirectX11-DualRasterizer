#pragma once
#include <fstream>
#include "Math.h"
#include "Mesh.h"

namespace dae
{
	namespace Utils
	{

		[[nodiscard]] inline float CalculateObservedArea(const Vector3& normal, const Vector3& lightDirection) noexcept
		{
			float const observedArea{ Vector3::Dot(normal, -lightDirection) };
			if (observedArea < 0.f)
				return 0.f;

			return observedArea;
		}

		[[nodiscard]] inline bool IsPixelInTriangle(Vector2 pixel, std::vector<Vector2> const& triangle) noexcept
		{
			for (uint32_t i{ 0 }; i < triangle.size(); ++i)
			{
				auto const e{ triangle[(i + 1) % triangle.size()] - triangle[i] };
				auto const p = pixel - triangle[i];

				if (Vector2::Cross(e, p) < 0.f)
				{
					return false;
				}
			}
			return true;
		}

		// Checks if at least one part of triangle is outside frustum
		[[nodiscard]] inline bool IsTriangleOutsideFrustum(Mesh const* mesh, uint32_t idx1, uint32_t idx2, uint32_t idx3) noexcept
		{
			const auto& v1 = mesh->GetVertices_Out()[idx1].position;
			const auto& v2 = mesh->GetVertices_Out()[idx2].position;
			const auto& v3 = mesh->GetVertices_Out()[idx3].position;

			// Check all vertices against each frustum plane
			if (v1.z < 0 && v2.z < 0 && v3.z < 0)
			{
				return true;
			}
			if (v1.z > 1 && v2.z > 1 && v3.z > 1)
			{
				return true;
			}

			for (int i = 0; i < 2; ++i)
			{
				if ((v1[i] > 1.f || v1[i] < -1.f) || (v2[i] > 1.f || v2[i] < -1.f) || (v3[i] > 1.f || v3[i] < -1.f))
					return true;
			}

			return false;
		}

		[[nodiscard]] constexpr float DepthRemap(float v, float min, float max) noexcept
		{
			float const normalizedValue{ (v - min) / (max - min) };
			if (normalizedValue < 0.0f)
			{
				return 0.f;
			}
			return normalizedValue;
		}


		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vertex_In>& vertices, std::vector<uint32_t>& indices, bool flipAxisAndWinding = true)
		{
			std::ifstream file(filename);
			if (!file)
				return false;

			std::vector<Vector3> positions{};
			std::vector<Vector3> normals{};
			std::vector<Vector2> UVs{};

			vertices.clear();
			indices.clear();

			std::string sCommand;
			// start a while iteration ending when the end of file is reached (ios::eof)
			while (!file.eof())
			{
				//read the first word of the string, use the >> operator (istream::operator>>) 
				file >> sCommand;
				//use conditional statements to process the different commands	
				if (sCommand == "#")
				{
					// Ignore Comment
				}
				else if (sCommand == "v")
				{
					//Vertex_In
					float x, y, z;
					file >> x >> y >> z;

					positions.emplace_back(x, y, z);
				}
				else if (sCommand == "vt")
				{
					// Vertex_In TexCoord
					float u, v;
					file >> u >> v;
					UVs.emplace_back(u, 1 - v);
				}
				else if (sCommand == "vn")
				{
					// Vertex_In Normal
					float x, y, z;
					file >> x >> y >> z;

					normals.emplace_back(x, y, z);
				}
				else if (sCommand == "f")
				{
					//if a face is read:
					//construct the 3 vertices, add them to the vertex array
					//add three indices to the index array
					//add the material index as attibute to the attribute array
					//
					// Faces or triangles
					Vertex_In vertex{};
					size_t iPosition, iTexCoord, iNormal;

					uint32_t tempIndices[3];
					for (size_t iFace = 0; iFace < 3; iFace++)
					{
						// OBJ format uses 1-based arrays
						file >> iPosition;
						vertex.position = positions[iPosition - 1];

						if ('/' == file.peek())//is next in buffer ==  '/' ?
						{
							file.ignore();//read and ignore one element ('/')

							if ('/' != file.peek())
							{
								// Optional texture coordinate
								file >> iTexCoord;
								vertex.texcoord = UVs[iTexCoord - 1];
							}

							if ('/' == file.peek())
							{
								file.ignore();

								// Optional vertex normal
								file >> iNormal;
								vertex.normal = normals[iNormal - 1];
							}
						}

						vertices.push_back(vertex);
						tempIndices[iFace] = uint32_t(vertices.size()) - 1;
						//indices.push_back(uint32_t(vertices.size()) - 1);
					}

					indices.push_back(tempIndices[0]);
					if (flipAxisAndWinding)
					{
						indices.push_back(tempIndices[2]);
						indices.push_back(tempIndices[1]);
					}
					else
					{
						indices.push_back(tempIndices[1]);
						indices.push_back(tempIndices[2]);
					}
				}
				//read till end of line and ignore all remaining chars
				file.ignore(1000, '\n');
			}

			//Cheap Tangent Calculations
			for (uint32_t i = 0; i < indices.size(); i += 3)
			{
				uint32_t index0 = indices[i];
				uint32_t index1 = indices[size_t(i) + 1];
				uint32_t index2 = indices[size_t(i) + 2];

				const Vector3& p0 = vertices[index0].position;
				const Vector3& p1 = vertices[index1].position;
				const Vector3& p2 = vertices[index2].position;
				const Vector2& uv0 = vertices[index0].texcoord;
				const Vector2& uv1 = vertices[index1].texcoord;
				const Vector2& uv2 = vertices[index2].texcoord;

				const Vector3 edge0 = p1 - p0;
				const Vector3 edge1 = p2 - p0;
				const Vector2 diffX = Vector2(uv1.x - uv0.x, uv2.x - uv0.x);
				const Vector2 diffY = Vector2(uv1.y - uv0.y, uv2.y - uv0.y);
				float r = 1.f / Vector2::Cross(diffX, diffY);

				Vector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
				vertices[index0].tangent += tangent;
				vertices[index1].tangent += tangent;
				vertices[index2].tangent += tangent;
			}

			//Create the Tangents (reject)
			for (auto& v : vertices)
			{
				v.tangent = Vector3::Reject(v.tangent, v.normal).Normalized();

				if (flipAxisAndWinding)
				{
					v.position.z *= -1.f;
					v.normal.z *= -1.f;
					v.tangent.z *= -1.f;
				}

			}

			return true;
		}
#pragma warning(pop)
	}
}
