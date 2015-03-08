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

	private:
		void fill_triangle(
			glm::dvec3 const & a,
			glm::dvec3 const & b,
			glm::dvec3 const & c,
			gil::rgb8_pixel_t const & color );

	private:
		gil::rgb8_view_t _frame;
		gil::gray32f_image_t _zbuffer;

	}; // Render

} // ns sharpeye
