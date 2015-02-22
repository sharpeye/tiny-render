#pragma once

#include <string>
#include <istream>
#include "model.h"

namespace miskopawel
{
	// load model from Wavefront .obj file
	Model load_obj( std::istream & is );
	Model load_obj( std::string const& filename );

} // ns miskopawel
