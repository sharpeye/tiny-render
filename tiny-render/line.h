#pragma once
#include <algorithm>
#include <boost/utility/swap.hpp>
#include <boost/gil/gil_all.hpp>

namespace sharpeye
{
	namespace gil = boost::gil;

	template< typename View > inline
	void draw_hline( View const & view,
		typename View::point_t::value_type a,
		typename View::point_t::value_type b,
		typename View::point_t::value_type y,
		typename View::value_type const & color )	
	{
		if( a > b )
		{
			boost::swap( a, b );
		}

		std::fill_n( view.row_begin( y ) + a, b - a + 1, color );
	}

	template< typename View > inline
	void draw_vline( View const & view, 
		typename View::point_t::value_type a, 
		typename View::point_t::value_type b, 
		typename View::point_t::value_type x,
		typename View::value_type const & color )
	{
		if( a > b )
		{
			boost::swap( a, b );
		}

		std::fill_n( view.col_begin( x ) + a, b - a + 1, color );
	}

	template< typename View, typename F > inline
	void bresenhams_line_impl(
		typename View::point_t a, 
		typename View::point_t b,
		typename View::value_type const & color, F && f )
	{
		if( a.x > b.x )
		{
			boost::swap( a, b );
		}

		for( auto x = a.x; x <= b.x; ++x )
		{
			auto t = ( x - a.x ) / (double) ( b.x - a.x );
			auto y = static_cast< decltype( x ) >( a.y * ( 1. - t ) + b.y * t );

			f( x, y );
		}
	}

	template< typename View > inline
	void bresenhams_line( View const & view, 
		typename View::point_t a, 
		typename View::point_t b,
		typename View::value_type const & color )
	{
		auto dx = std::abs( b.x - a.x );
		auto dy = std::abs( b.y - a.y );

		if( dx < dy )
		{
			boost::swap( a.x, a.y );
			boost::swap( b.x, b.y );

			return bresenhams_line_impl< View >( a, b, color, 
				[&]( int x, int y )
				{ 
					view( y, x ) = color;
				} );
		}

		bresenhams_line_impl< View >( a, b, color, 
			[&]( int x, int y )
			{ 
				view( x, y ) = color;
			} );
	}

	template< typename View > inline
	void draw_line( View const & view, 
		typename View::point_t a, 
		typename View::point_t b,
		typename View::value_type const & color )
	{
		if( a.x == b.x )
		{
			return draw_vline( view, a.y, b.y, a.x, color );
		}

		if( a.y == b.y )
		{
			return draw_hline( view, a.x, b.x, a.y, color );
		}

		bresenhams_line( view, a, b, color );
	}

} // ns sharpeye
