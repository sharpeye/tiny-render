#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace miskopawel
{
	struct Model
	{
		typedef glm::uvec3 Face;
		typedef glm::dvec3 Vertex;

		std::vector< Face > faces;
		std::vector< Vertex > vertices;

	}; // Model

} // ns miskopawel
