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
		void set_light_dir( glm::dvec3 const & v );
		void set_diffuse_map( gil::rgb8_view_t img );

	private:
		void fill_triangle(
			glm::dmat3x4 const & vs,
			glm::dmat3 const & ts,
			glm::dvec3 const & lum );

	private:
		gil::rgb8_view_t _frame;
		gil::gray32f_image_t _zbuffer;

		glm::dmat4 _proj;
		glm::dmat4 _view;
		glm::dvec4 _light;

		gil::rgb8_view_t _diff;

	}; // Render

} // ns sharpeye
