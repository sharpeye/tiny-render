#include "stdafx.h"
#include <iostream>
#include <boost/gil/extension/dynamic_image/dynamic_image_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/program_options.hpp>
#include <boost/filesystem/convenience.hpp>
#include <tiny-render/render.h>
#include <tiny-render/obj.h>

using namespace sharpeye;
namespace po = boost::program_options;

int main( int argc, char * argv[] )
{
	std::string in;
	std::string out;
	bool save_zbuffer = false;

	po::options_description desc( "Options" );

	desc.add_options()
		( "help", "produce help message" )
		( "input,i", po::value< std::string >( &in ), "path to model (.obj)" )
		( "zbuffer,z", po::bool_switch( &save_zbuffer ), "save z-buffer to file (.z.png)" )
		( "output,o", po::value< std::string >( &out )->default_value( "output.png" ), "path to output (.png)" )
		;

	try
	{
		po::variables_map vm;
		po::store( po::parse_command_line( argc, argv, desc ), vm );
		po::notify( vm );

		if( vm.count( "help" ) )
		{
			std::cout << desc << std::endl;

			return 1;
		}

		auto const w = 800;
		auto const h = 800;

		gil::rgb8_image_t img( w, h );

		gil::fill_pixels( gil::view( img ), gil::rgb8_pixel_t{ 0, 100, 0 } );

		Render render{ gil::view( img ) };

		render.set_view_matrix( glm::lookAt( 
			glm::dvec3{ 1, 1, 3 }, 
			glm::dvec3{ 0, 0, 0 },
			glm::dvec3{ 0, 1, 0 } ) );

		render.set_proj_matrix( glm::perspective( 45.0, w / (double) h, 1.0, 10.0 ) );

		auto model = load_obj( in );

		render.draw( model );

		gil::png_write_view( out, gil::flipped_up_down_view( gil::const_view( img ) ) );

		if( save_zbuffer )
		{
			auto zbuffer = render.zbuffer();
			gil::gray8_image_t img( zbuffer.width(), zbuffer.height() );

			gil::transform_pixels( zbuffer, gil::view( img ), []( auto px )
			{
				auto v = ( 1 - px[ 0 ] ) * 255.0 / 2.0;

				return gil::gray8_pixel_t{ static_cast< gil::bits8 >( std::min( 255.0, std::max( 0.0, v ) ) ) };
			} );

			auto fn = boost::filesystem::change_extension( out, ".z.png" );

			gil::png_write_view( fn.string(), gil::flipped_up_down_view( gil::const_view( img ) ) );
		}

		return 0;
	}
	catch( std::exception & e )
	{
		std::cerr << "[error] " << e.what() << std::endl;
	}

	return 1;
}
