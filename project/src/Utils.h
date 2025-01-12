#pragma once
#include <fstream>
#include "Math.h"

namespace dae
{
	namespace Utils
	{
		//Just parses vertices and indices
#pragma warning(push)
#pragma warning(disable : 4505) //Warning unreferenced local function
		static bool ParseOBJ(const std::string& filename, std::vector<Vertex>& vertices, std::vector<uint32_t>& indices, bool flipAxisAndWinding = true)
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
					//Vertex
					float x, y, z;
					file >> x >> y >> z;

					positions.emplace_back(x, y, z);
				}
				else if (sCommand == "vt")
				{
					// Vertex TexCoord
					float u, v;
					file >> u >> v;
					UVs.emplace_back(u, 1 - v);
				}
				else if (sCommand == "vn")
				{
					// Vertex Normal
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
					Vertex vertex{};
					size_t iPosition, iTexCoord, iNormal;

					uint32_t tempIndices[3];
					for (size_t iFace = 0; iFace < 3; iFace++)
					{
						// OBJ format uses 1-based arrays
						file >> iPosition;
						vertex.x = positions[iPosition - 1].x;
						vertex.y = positions[iPosition - 1].y;
						vertex.z = positions[iPosition - 1].z;

						if ('/' == file.peek())//is next in buffer ==  '/' ?
						{
							file.ignore();//read and ignore one element ('/')

							if ('/' != file.peek())
							{
								// Optional texture coordinate
								file >> iTexCoord;
								vertex.u = UVs[iTexCoord - 1].x;
								vertex.v = UVs[iTexCoord - 1].y;
							}

							if ('/' == file.peek())
							{
								file.ignore();

								// Optional vertex normal
								file >> iNormal;
								vertex.nx = normals[iNormal - 1].x;
								vertex.ny = normals[iNormal - 1].y;
								vertex.nz = normals[iNormal - 1].z;
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

				const Vector3 p0 = { vertices[index0].x, vertices[index0].y, vertices[index0].z };
				const Vector3 p1 = { vertices[index1].x, vertices[index1].y, vertices[index1].z };
				const Vector3 p2 = { vertices[index2].x, vertices[index2].y, vertices[index2].z };
				const Vector2 uv0 = { vertices[index0].u, vertices[index0].v };
				const Vector2 uv1 = { vertices[index1].u, vertices[index1].v };
				const Vector2 uv2 = { vertices[index2].u, vertices[index2].v };

				const Vector3 edge0 = p1 - p0;
				const Vector3 edge1 = p2 - p0;
				const Vector2 diffX = Vector2(uv1.x - uv0.x, uv2.x - uv0.x);
				const Vector2 diffY = Vector2(uv1.y - uv0.y, uv2.y - uv0.y);
				float r = 1.f / Vector2::Cross(diffX, diffY);

				Vector3 tangent = (edge0 * diffY.y - edge1 * diffY.x) * r;
				vertices[index0].tx += tangent.x;
				vertices[index0].ty += tangent.y;
				vertices[index0].tz += tangent.z;

				vertices[index1].tx += tangent.x;
				vertices[index1].ty += tangent.y;
				vertices[index1].tz += tangent.z;

				vertices[index2].tx += tangent.x;
				vertices[index2].ty += tangent.y;
				vertices[index2].tz += tangent.z;
			}

			//Create the Tangents (reject)
			for (auto& v : vertices)
			{
				auto const tan = Vector3::Reject({ v.tx, v.ty, v.tz }, { v.nx, v.ny, v.nz }).Normalized();
				v.tx = tan.x;
				v.ty = tan.y;
				v.tz = tan.z;

				if(flipAxisAndWinding)
				{
					v.z *= -1.f;
					v.nz *= -1.f;
					v.tz *= -1.f;
				}
			}

			return true;
		}
#pragma warning(pop)
	}
}