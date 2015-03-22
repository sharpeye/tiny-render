#pragma once

#include <string>
#include <istream>
#include <tiny-render/model.h>

namespace sharpeye
{
	// load model from Wavefront .obj file
	Model load_obj( std::istream & is );
	Model load_obj( std::string const& filename );

} // ns sharpeye
