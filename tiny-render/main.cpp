#include "stdafx.h"
#include <iostream>
#include <boost/gil/extension/dynamic_image/dynamic_image_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>
#include "render.h"
#include "obj.h"

using namespace sharpeye;

int main( int argc, char * argv[] )
{
	gil::rgb8_image_t img( 820, 820 );

	gil::fill_pixels( gil::view( img ), gil::rgb8_pixel_t{ 0, 0, 0 } );

	render_model( gil::subimage_view( gil::view( img ), 10, 10, 800, 800 ), load_obj( "D:\\tmp\\african_head.obj" ) );

	gil::png_write_view( argc > 1 ? argv[ 1 ] : "output.png", gil::rotated180_view( gil::const_view( img ) ) );

	return 0;
}
