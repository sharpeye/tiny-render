#pragma once
#include <boost/gil/gil_all.hpp>
#include "model.h"

namespace miskopawel
{
	namespace gil = boost::gil;

	void render_model( gil::argb8_view_t const & view, Model const & model );

} // ns miskopawel
