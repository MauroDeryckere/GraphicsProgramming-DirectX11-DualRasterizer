#pragma	once

#include "pch.h"

namespace dae
{
	struct Vertex_In
	{
		Vector3 position{};
		Vector2 texcoord{};
		Vector3 normal{};
		Vector3 tangent{};
	};

	struct Vertex_Out
	{
		Vector4 position{};
		Vector2 texcoord{};
		Vector3 normal{};
		Vector3 tangent{};
	};
}

