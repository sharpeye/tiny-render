#pragma once
#include <boost/gil/gil_all.hpp>
#include "model.h"

namespace sharpeye
{
	namespace gil = boost::gil;

	void render_model( gil::rgb8_view_t const & view, Model const & model );

} // ns sharpeye
