#pragma once
#include <vector>
#include <glm/glm.hpp>

namespace sharpeye
{
	struct Model
	{
		typedef glm::dvec3 Vertex;

		struct Face
		{
			glm::uvec3 v;
			glm::uvec3 t;
			glm::uvec3 n;

		}; // Face

		std::vector< Face > faces;
		std::vector< Vertex > vertices;

	}; // Model

	bool operator == ( Model::Face const & lhs, Model::Face const & rhs );

} // ns sharpeye
