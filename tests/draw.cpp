#include "stdafx.h"
#include <sstream>
#include <boost/test/unit_test.hpp>
#include <tiny-render/line.h>

using namespace sharpeye;

BOOST_AUTO_TEST_SUITE( Draw )

BOOST_AUTO_TEST_CASE( hline )
{
	int const w = 8;
	int const h = 8;

	gil::rgb8_pixel_t img[ w * h ] = {};

	auto view = gil::interleaved_view( w, h, img, sizeof( gil::rgb8_pixel_t ) * w );

	gil::rgb8_pixel_t const px{ 255, 0, 0 };

	draw_hline( view, 1, w - 2, 4, px );

	gil::rgb8_pixel_t reference[] = 
	{
		{}, {}, {}, {}, {}, {}, {}, {},
		{}, {}, {}, {}, {}, {}, {}, {},
		{}, {}, {}, {}, {}, {}, {}, {},
		{}, {}, {}, {}, {}, {}, {}, {},
		{}, px, px, px, px, px, px, {}, 
		{}, {}, {}, {}, {}, {}, {}, {},
		{}, {}, {}, {}, {}, {}, {}, {},
		{}, {}, {}, {}, {}, {}, {}, {},
	};

	static_assert( sizeof( reference ) == sizeof( img ), "must be same size" );

	BOOST_REQUIRE( memcmp( img, reference, sizeof( reference ) ) == 0 );
}

BOOST_AUTO_TEST_CASE( vline )
{
	int const w = 8;
	int const h = 8;

	gil::rgb8_pixel_t img[ w * h ] = {};

	auto view = gil::interleaved_view( w, h, img, sizeof( gil::rgb8_pixel_t ) * w );

	gil::rgb8_pixel_t const px{ 255, 0, 0 };

	draw_vline( view, 1, h - 2, 4, px );

	gil::rgb8_pixel_t reference[] = 
	{
		{}, {}, {}, {}, {}, {}, {}, {},
		{}, {}, {}, {}, px, {}, {}, {},
		{}, {}, {}, {}, px, {}, {}, {},
		{}, {}, {}, {}, px, {}, {}, {},
		{}, {}, {}, {}, px, {}, {}, {},
		{}, {}, {}, {}, px, {}, {}, {},
		{}, {}, {}, {}, px, {}, {}, {},
		{}, {}, {}, {}, {}, {}, {}, {},
	};

	static_assert( sizeof( reference ) == sizeof( img ), "must be same size" );

	BOOST_REQUIRE( memcmp( img, reference, sizeof( reference ) ) == 0 );
}

BOOST_AUTO_TEST_CASE( bresenham )
{
	int const w = 8;
	int const h = 8;

	gil::rgb8_pixel_t img[ w * h ] = {};

	auto view = gil::interleaved_view( w, h, img, sizeof( gil::rgb8_pixel_t ) * w );

	gil::rgb8_pixel_t const px{ 255, 0, 0 };

	bresenhams_line( view, { 1, 1 }, { w - 2, h - 2 }, px );
	bresenhams_line( view, { w - 2, 1 }, { 1, h - 2 }, px );

	gil::rgb8_pixel_t reference[] = 
	{
		{}, {}, {}, {}, {}, {}, {}, {},
		{}, px, {}, {}, {}, {}, px, {},
		{}, {}, px, {}, {}, px, {}, {},
		{}, {}, {}, px, px, {}, {}, {},
		{}, {}, {}, px, px, {}, {}, {},
		{}, {}, px, {}, {}, px, {}, {},
		{}, px, {}, {}, {}, {}, px, {},
		{}, {}, {}, {}, {}, {}, {}, {},
	};

	static_assert( sizeof( reference ) == sizeof( img ), "must be same size" );

	BOOST_REQUIRE( memcmp( img, reference, sizeof( reference ) ) == 0 );
}

BOOST_AUTO_TEST_CASE( line )
{
	int const w = 8;
	int const h = 8;

	gil::rgb8_pixel_t img[ w * h ] = {};

	auto view = gil::interleaved_view( w, h, img, sizeof( gil::rgb8_pixel_t ) * w );

	gil::rgb8_pixel_t const px{ 255, 0, 0 };

	draw_line( view, { w - 2, h - 2 }, { 1, 1 }, px );
	draw_line( view, { w - 2, 1 }, { 1, h - 2 }, px );

	draw_line( view, { 1, 1 }, { 1, h - 2 }, px );
	draw_line( view, { 1, h - 2 }, { w - 2, h - 2 }, px );
	draw_line( view, { w - 2, 1 }, { w - 2, h - 2 }, px );
	draw_line( view, { 1, 1 }, { w - 2, 1 }, px );

	gil::rgb8_pixel_t reference[] = 
	{
		{}, {}, {}, {}, {}, {}, {}, {},
		{}, px, px, px, px, px, px, {}, 
		{}, px, px, {}, {}, px, px, {},
		{}, px, {}, px, px, {}, px, {},
		{}, px, {}, px, px, {}, px, {},
		{}, px, px, {}, {}, px, px, {},
		{}, px, px, px, px, px, px, {}, 
		{}, {}, {}, {}, {}, {}, {}, {},
	};

	static_assert( sizeof( reference ) == sizeof( img ), "must be same size" );

	BOOST_REQUIRE( memcmp( img, reference, sizeof( reference ) ) == 0 );
}

BOOST_AUTO_TEST_SUITE_END()
