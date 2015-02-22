#include "stdafx.h"
#include <sstream>
#include <boost/test/unit_test.hpp>
#include "../tiny-render/obj.h"

BOOST_AUTO_TEST_SUITE( Obj )

BOOST_AUTO_TEST_CASE( obj_loader )
{
	std::istringstream iss{ "v 1 2 3\n# comment\n\nf 5 7 9\n\n" };
	auto model = miskopawel::load_obj( iss );

	BOOST_REQUIRE( model.vertices.size() == 1 );
	BOOST_REQUIRE( (model.vertices[ 0 ] == glm::dvec3{ 1, 2, 3 }) );

	BOOST_REQUIRE( model.faces.size() == 1 );
	BOOST_REQUIRE( (model.faces[ 0 ] == glm::uvec3{ 5, 7, 9 }) );
}

BOOST_AUTO_TEST_CASE( empty )
{
	std::istringstream iss{ "" };
	auto model = miskopawel::load_obj( iss );

	BOOST_REQUIRE( model.vertices.size() == 0 );
	BOOST_REQUIRE( model.faces.size() == 0 );
}

BOOST_AUTO_TEST_CASE( bad_format )
{
	std::istringstream iss{ "v 1 \n2 3\n" };
	auto model = miskopawel::load_obj( iss );

	BOOST_REQUIRE( model.vertices.size() == 0 );
	BOOST_REQUIRE( model.faces.size() == 0 );
}

BOOST_AUTO_TEST_SUITE_END()
