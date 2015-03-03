#include "stdafx.h"
#include <iostream>
#include <boost/gil/extension/dynamic_image/dynamic_image_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>
#include <boost/program_options.hpp>
#include "render.h"
#include "obj.h"

using namespace sharpeye;
namespace po = boost::program_options;

int main( int argc, char * argv[] )
{
	std::string in;
	std::string out;

	po::options_description desc( "Options" );

	desc.add_options()
		( "help", "produce help message" )
		( "input,i", po::value< std::string >( &in ), "path to model (.obj)" )
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

		gil::rgb8_image_t img( 820, 820 );

		gil::fill_pixels( gil::view( img ), gil::rgb8_pixel_t{ 0, 0, 0 } );

		render_model( 
			gil::subimage_view( gil::view( img ), 10, 10, 800, 800 ), 
			load_obj( in ) );

		gil::png_write_view( out, gil::flipped_up_down_view( gil::const_view( img ) ) );

		return 0;
	}
	catch( std::exception & e )
	{
		std::cerr << "[error] " << e.what() << std::endl;
	}

	return 1;
}
