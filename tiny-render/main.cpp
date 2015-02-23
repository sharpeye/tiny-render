#include "stdafx.h"
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <boost/gil/extension/io/png_io.hpp>
#include <glm/glm.hpp>
#include "line.h"
#include "model.h"
#include "render.h"
#include "obj.h"

using namespace sharpeye;

int wmain( int argc, wchar_t * argv[] )
{
	gil::rgb8_image_t img( 800, 800 );

	gil::fill_pixels( gil::view( img ), gil::rgb8_pixel_t{ 255, 255, 255 } );

	render_model( gil::view( img ), load_obj( "D:\\tmp\\african_head.obj" ) );

	gil::png_write_view( "D:\\tmp\\img.png", gil::const_view( img ) );

	return 0;
}
