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

using namespace miskopawel;

int wmain( int argc, wchar_t * argv[] )
{
	/*
	std::vector< int > v;

	std::cout << v.size() << std::endl;
	boost::phoenix::push_back( boost::ref( v ), boost::phoenix::placeholders::_1 )( 1 );
	std::cout << v.size() << std::endl;*/
	

	/*
	gil::rgb8_image_t img( 800, 800 );

	gil::fill_pixels( gil::view( img ), gil::rgb8_pixel_t{ 255, 255, 255 } );

	render_model( gil::view( img ), load_modle( "D:\\tmp\\african_head.obj" ) ); */

	std::istringstream is{ "# comment\nv 0.0 1.0 2.0\n# comment \n#f 7 3 5\n" };
	//std::istringstream is{ "v 0.0 1.0 2.0\nv 3.0 5.0 7.0\n" };
	//std::istringstream is{ "v 0.0 1.0 2.0\n" };
	load_obj( is );

	//load_obj( "D:\\tmp\\african_head.obj" );

	/*
	auto x = img.width() - 2;
	auto y = img.height() - 2;
	auto view = gil::view( img );

	// box
	draw_line( view, { 1, 1 }, { 1, y }, { 0, 0, 255 } );
	draw_line( view, { x, y }, { x, 1 }, { 0, 0, 255 } );
	draw_line( view, { x, 1 }, { 1, 1 }, { 0, 0, 255 } );
	draw_line( view, { x, y }, { 1, y }, { 0, 0, 255 } );
	// cross
	draw_line( view, { x, y }, { 1, 1 }, { 0, 0, 255 } );
	draw_line( view, { 1, y }, { x, 1 }, { 0, 0, 255 } );*/

//	gil::png_write_view( "D:\\tmp\\img.png", gil::const_view( img ) );

	return 0;
}
