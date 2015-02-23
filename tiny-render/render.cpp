#include "stdafx.h"
#include "render.h"

namespace sharpeye
{
	void render_model( gil::rgb8_view_t const & view, Model const & model )
	{
		std::clog 
			<< model.vertices.size() << "\n"
			<< model.faces.size() 
			<< std::endl;
	}

} // ns sharpeye
