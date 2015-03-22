#pragma once
#include <boost/gil/gil_all.hpp>
#include "model.h"

namespace sharpeye
{
	namespace gil = boost::gil;

	class Render
	{
	public:
		explicit Render( gil::rgb8_view_t const & frame );

		void draw( Model const & model );
		gil::rgb8_view_t frame();
		gil::gray32f_view_t zbuffer();

		void set_view_matrix( glm::dmat4 const & m );
		void set_proj_matrix( glm::dmat4 const & m );

	private:
		void fill_triangle(
			glm::dvec3 const & a,
			glm::dvec3 const & b,
			glm::dvec3 const & c,
			glm::dvec3 const & intensity );

	private:
		gil::rgb8_view_t _frame;
		gil::gray32f_image_t _zbuffer;

		glm::dmat4 _proj;
		glm::dmat4 _view;

	}; // Render

} // ns sharpeye
