#include "stdafx.h"
#include <fstream>
#include <iterator>
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>
#include <tiny-render/obj.h>

BOOST_FUSION_ADAPT_STRUCT( glm::dvec3, (double, x) (double, y) (double, z) )
BOOST_FUSION_ADAPT_STRUCT( glm::uvec3, (unsigned, x) (unsigned, y) (unsigned, z) )
BOOST_FUSION_ADAPT_STRUCT( sharpeye::Model::Face, (glm::uvec3, v) (glm::uvec3, t) (glm::uvec3, n) )
BOOST_FUSION_ADAPT_STRUCT( sharpeye::Model, 
	(std::vector< sharpeye::Model::Face >, faces) 
	(std::vector< sharpeye::Model::Vertex >, vertices)
	(std::vector< sharpeye::Model::Vertex >, normals)
)

namespace sharpeye
{
	using namespace boost::spirit;

	template < typename Iterator >
	struct obj_parser : qi::grammar< Iterator, Model (), ascii::blank_type >
	{
		obj_parser()
			: obj_parser::base_type( start )
		{
			using qi::double_;
			using qi::uint_;
			using qi::lit;
			using qi::eol;
			using qi::_val;
			using qi::_1;
			using qi::_a;
			using qi::_b;
			using qi::_c;
			using qi::eps;
			using boost::phoenix::at_c;
			using boost::phoenix::push_back;
			using boost::phoenix::bind;

			vertex = lit( 'v' ) >> double_ >> double_ >> double_;
			normal = lit( "vn" ) >> double_ >> double_ >> double_;

			index = uint_[ _val = _1 - 1 ];

			auto normalize = []( glm::dvec3 const & v )
			{
				return glm::normalize( v );
			};

			// _a - vertex, _b - texture, _c - normal
			face = lit( 'f' ) >> (
					( 
						index[ at_c< 0 >( _a ) = _1 ] >> index[ at_c< 1 >( _a ) = _1 ] >> index[ at_c< 2 >( _a ) = _1 ] 
					) |
					( 
						index[ at_c< 0 >( _a ) = _1 ] >> '/' >> index[ at_c< 0 >( _b ) = _1 ] >> 
						index[ at_c< 1 >( _a ) = _1 ] >> '/' >> index[ at_c< 1 >( _b ) = _1 ] >> 
						index[ at_c< 2 >( _a ) = _1 ] >> '/' >> index[ at_c< 2 >( _b ) = _1 ]
					) |
					(
						index[ at_c< 0 >( _a ) = _1 ] >> "//" >> index[ at_c< 0 >( _c ) = _1 ] >> 
						index[ at_c< 1 >( _a ) = _1 ] >> "//" >> index[ at_c< 1 >( _c ) = _1 ] >> 
						index[ at_c< 2 >( _a ) = _1 ] >> "//" >> index[ at_c< 2 >( _c ) = _1 ]
					) |
					(
						index[ at_c< 0 >( _a ) = _1 ] >> '/' >> index[ at_c< 0 >( _b ) = _1 ] >> '/' >> index[ at_c< 0 >( _c ) = _1 ] >> 
						index[ at_c< 1 >( _a ) = _1 ] >> '/' >> index[ at_c< 1 >( _b ) = _1 ] >> '/' >> index[ at_c< 1 >( _c ) = _1 ] >> 
						index[ at_c< 2 >( _a ) = _1 ] >> '/' >> index[ at_c< 2 >( _b ) = _1 ] >> '/' >> index[ at_c< 2 >( _c ) = _1 ]
					) )[
						at_c< 0 >( _val ) = _a,
						at_c< 1 >( _val ) = _b,
						at_c< 2 >( _val ) = _c
					];

			start = (
					face[ push_back( at_c< 0 >( _val ), _1 ) ] |
					vertex[ push_back( at_c< 1 >( _val ), _1 ) ] |
					normal[ push_back( at_c< 2 >( _val ), bind( normalize, _1 ) ) ] |
					*( qi::char_ - eol ) ) % eol
				| eol;
		}

		qi::rule< Iterator, Model (), ascii::blank_type > start;
		qi::rule< Iterator, void(), ascii::blank_type > line;
		qi::rule< Iterator, unsigned() > index;
		qi::rule< Iterator, Model::Vertex (), ascii::blank_type > vertex;
		qi::rule< Iterator, Model::Vertex (), ascii::blank_type > normal;
		qi::rule< Iterator, Model::Face (), ascii::blank_type, qi::locals< glm::uvec3, glm::uvec3, glm::uvec3 > > face;

	}; // obj_parser

	Model load_obj( std::istream & is )
	{
		is.unsetf( std::ios::skipws );

		boost::spirit::istream_iterator it{ is }, end;

		obj_parser< decltype( it ) > parser;

		Model model;

		bool r = qi::phrase_parse( it, end, parser, ascii::blank, model );

		if( !r || it != end )
		{
			/*
			std::cerr<< "error: '";
			std::copy( it, end, std::ostream_iterator< char >( std::cerr ) );
			std::cerr << "'\n";*/

			return {};
		}

		return std::move( model );
	}

	Model load_obj( std::string const& filename )
	{
		std::ifstream file( filename );

		return load_obj( file );
	}

} // ns sharpeye
