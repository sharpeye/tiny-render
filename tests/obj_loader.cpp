#include "stdafx.h"
#include <algorithm>
#include <sstream>
#include <boost/range/algorithm/equal.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/format.hpp>
#include "../tiny-render/obj.h"

using namespace sharpeye;

BOOST_AUTO_TEST_SUITE( Obj )

BOOST_AUTO_TEST_CASE( obj_loader )
{
	std::istringstream iss{ "v 1 2 3\n# comment\n\nf 5 7 9\n\n" };
	auto model = load_obj( iss );

	BOOST_REQUIRE( model.vertices.size() == 1 );
	BOOST_REQUIRE( (model.vertices[ 0 ] == glm::dvec3{ 1, 2, 3 }) );

	BOOST_REQUIRE( model.faces.size() == 1 );
	BOOST_REQUIRE( (model.faces[ 0 ].v == glm::uvec3{ 4, 6, 8 }) );
	BOOST_REQUIRE( (model.faces[ 0 ].t == glm::uvec3{}) );
	BOOST_REQUIRE( (model.faces[ 0 ].n == glm::uvec3{}) );
}

BOOST_AUTO_TEST_CASE( empty )
{
	std::istringstream iss{ "" };
	auto model = load_obj( iss );

	BOOST_REQUIRE( model.vertices.size() == 0 );
	BOOST_REQUIRE( model.faces.size() == 0 );
}

BOOST_AUTO_TEST_CASE( bad_format )
{
	std::istringstream iss{ "v 1 \n2 3\n" };
	auto model = load_obj( iss );

	BOOST_REQUIRE( model.vertices.size() == 0 );
	BOOST_REQUIRE( model.faces.size() == 0 );
}

BOOST_AUTO_TEST_CASE( faces_1 )
{
	Model::Face fs[] = { 
		Model::Face{ { 1, 2, 3 } },
		Model::Face{ { 4, 5, 6 } },
		Model::Face{ { 7, 8, 9 } }
	};

	std::string s;

	for( auto f : fs  )
	{
		s += str( boost::format( "f %d %d %d\n" ) 
			% ( f.v.x + 1 ) 
			% ( f.v.y + 1 ) 
			% ( f.v.z + 1 ) 
			);
	}

	std::istringstream iss{ s };

	auto model = load_obj( iss );

	BOOST_REQUIRE( model.vertices.size() == 0 );
	BOOST_REQUIRE( model.faces.size() == _countof( fs ) );
	BOOST_REQUIRE( boost::equal( model.faces, fs ) );
}


BOOST_AUTO_TEST_CASE( faces_2 )
{
	Model::Face fs[] = { 
		Model::Face{ { 1, 2, 3 }, { 3, 2, 1 } },
		Model::Face{ { 4, 5, 6 }, { 6, 5, 4 } },
		Model::Face{ { 7, 8, 9 }, { 9, 8, 7 } }
	};

	std::string s;

	for( auto f : fs  )
	{
		s += str( boost::format( "f %d/%d %d/%d %d/%d\n" ) 
			% ( f.v.x + 1 ) % ( f.t.x + 1 )
			% ( f.v.y + 1 ) % ( f.t.y + 1 )
			% ( f.v.z + 1 ) % ( f.t.z + 1 )
			);
	}

	std::istringstream iss{ s };

	auto model = load_obj( iss );

	BOOST_REQUIRE( model.vertices.size() == 0 );
	BOOST_REQUIRE( model.faces.size() == _countof( fs ) );
	BOOST_REQUIRE( boost::equal( model.faces, fs ) );
}

BOOST_AUTO_TEST_CASE( faces_3 )
{
	Model::Face fs[] = { 
		Model::Face{ { 10, 2, 3 }, { 3, 2, 1 }, { 9, 8, 7 } },
		Model::Face{ { 40, 5, 6 }, { 6, 5, 4 }, { 6, 5, 4 } },
		Model::Face{ { 70, 8, 9 }, { 9, 8, 7 }, { 3, 2, 1 } }
	};

	std::string s;

	for( auto f : fs  )
	{
		s += str( boost::format( "f %d/%d/%d %d/%d/%d %d/%d/%d\n" ) 
			% ( f.v.x + 1 ) % ( f.t.x + 1 ) % ( f.n.x + 1 )
			% ( f.v.y + 1 ) % ( f.t.y + 1 ) % ( f.n.y + 1 )
			% ( f.v.z + 1 ) % ( f.t.z + 1 ) % ( f.n.z + 1 )
			);
	}

	std::istringstream iss{ s };

	auto model = load_obj( iss );

	BOOST_REQUIRE( model.vertices.size() == 0 );
	BOOST_REQUIRE( model.faces.size() == _countof( fs ) );
	BOOST_REQUIRE( boost::equal( model.faces, fs ) );
}

BOOST_AUTO_TEST_CASE( faces_4 )
{
	Model::Face fs[] = { 
		Model::Face{ { 6, 10, 30 }, {}, { 35, 24, 17 } },
		Model::Face{ { 4, 5, 6 }, {}, { 6, 5, 4 } },
		Model::Face{ { 7, 8, 9 }, {}, { 9, 8, 7 } }
	};

	std::string s;

	for( auto f : fs  )
	{
		s += str( boost::format( "f %d//%d %d//%d %d//%d\n" ) 
			% ( f.v.x + 1 ) % ( f.n.x + 1 )
			% ( f.v.y + 1 ) % ( f.n.y + 1 )
			% ( f.v.z + 1 ) % ( f.n.z + 1 )
			);
	}

	std::istringstream iss{ s };

	auto model = load_obj( iss );

	BOOST_REQUIRE( model.vertices.size() == 0 );
	BOOST_REQUIRE( model.faces.size() == _countof( fs ) );
	BOOST_REQUIRE( boost::equal( model.faces, fs ) );
}

BOOST_AUTO_TEST_SUITE_END()
