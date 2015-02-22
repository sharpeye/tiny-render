#include "stdafx.h"
#include <fstream>
#include <iterator>
#include <boost/spirit/include/qi.hpp>
#include <boost/phoenix.hpp>
#include "obj.h"

BOOST_FUSION_ADAPT_STRUCT( miskopawel::Model::Vertex, (double, x) (double, y) (double, z) )
BOOST_FUSION_ADAPT_STRUCT( miskopawel::Model::Face, (unsigned, x) (unsigned, y) (unsigned, z) )

namespace miskopawel
{
	using namespace boost::spirit;

	template < typename Iterator >
	struct obj_parser : qi::grammar< Iterator, void(), ascii::blank_type >
	{
		obj_parser()
			: obj_parser::base_type( start )
		{
			using qi::double_;
			using qi::uint_;
			using qi::lit;
			using qi::eol;
			using boost::phoenix::at_c;
			using boost::phoenix::push_back;
			using boost::phoenix::placeholders::_1;

			comment = lit( '#' ) >> *(qi::char_ - eol);

			vertex = lit( 'v' ) >> ( double_ >> double_ >> double_ );
			face = lit( 'f' ) >> ( uint_ >> uint_ >> uint_ );

			line = comment 
				| vertex[ push_back( boost::ref( model.vertices ), at_c< 0 >( _1 ) ) ]
				| face[ push_back( boost::ref( model.faces ), at_c< 0 >( _1 ) ) ]
				;

			start = *(line >> +eol);
		}

		qi::rule< Iterator, void(), ascii::blank_type > start;
		qi::rule< Iterator, void(), ascii::blank_type > line;
		qi::rule< Iterator, Model::Vertex (), ascii::blank_type > vertex;
		qi::rule< Iterator, Model::Face (), ascii::blank_type > face;
		qi::rule< Iterator, void() > comment;

		Model model;

	}; // obj_parser

	Model load_obj( std::istream & is )
	{
		is.unsetf( std::ios::skipws );

		boost::spirit::istream_iterator it{ is }, end;

		obj_parser< decltype( it ) > parser;

		bool r = qi::phrase_parse( it, end, parser, ascii::blank );

		if( !r || it != end )
		{
			/*
			std::cerr<< "error: '";
			std::copy( it, end, std::ostream_iterator< char >( std::cerr ) );
			std::cerr << "'\n";*/

			return {};
		}

		return std::move( parser.model );
	}

	Model load_obj( std::string const& filename )
	{
		std::ifstream file( filename );

		return load_obj( file );
	}

} // ns miskopawel
