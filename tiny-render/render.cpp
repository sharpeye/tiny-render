#include "stdafx.h"
#include "render.h"
#include "line.h"

namespace sharpeye
{
	static gil::rgb8_view_t::point_t to_view( 
		gil::rgb8_view_t::x_coord_t w, 
		gil::rgb8_view_t::y_coord_t h, 
		glm::dvec3 const & v )
	{
		auto x = static_cast< std::ptrdiff_t >( ( v.x + 1.0 ) * w / 2.0 );
        auto y = static_cast< std::ptrdiff_t >( ( v.y + 1.0 ) * h / 2.0 );

		return { x, y };
	}

	static bool in_view( 
		gil::rgb8_view_t::x_coord_t w, 
		gil::rgb8_view_t::y_coord_t h, 
		gil::rgb8_view_t::point_t const & p )
	{
		return p.x < w && p.x >= 0 && p.y < h && p.y >= 0;
	}

	template< typename T >
	static T clamp( T v, T dim )
	{
		return (std::max)( T{}, (std::min)( v, dim - 1 ) );
	}

	static decltype( auto ) line_x( 
		gil::rgb8_view_t::point_t::value_type y, 
		gil::rgb8_view_t::point_t & v0, 
		gil::rgb8_view_t::point_t & v1 )
	{
		auto const dx = v1.x - v0.x;
		auto const dy = v1.y - v0.y;

		return static_cast< decltype( y ) >( v0.x + ( y - v0.y ) * dx / (double) dy );
	}

	static decltype( auto ) line_y( 
		gil::rgb8_view_t::point_t::value_type x, 
		gil::rgb8_view_t::point_t & v0, 
		gil::rgb8_view_t::point_t & v1 )
	{
		auto const dx = v1.x - v0.x;
		auto const dy = v1.y - v0.y;

		return static_cast< decltype( x ) >( v0.y + ( x - v0.x ) * dy / (double) dx );
	}

	static void clip_line( 
		gil::rgb8_view_t::x_coord_t w, 
		gil::rgb8_view_t::y_coord_t h, 
		gil::rgb8_view_t::point_t & v0, 
		gil::rgb8_view_t::point_t & v1 )
	{
		if( !in_view( w, h, v0 ) ||
			!in_view( w, h, v1 ) )
		{
			if( !in_view( w, h, v1 ) )
			{
				boost::swap( v0, v1 );
			}

			auto x0 = v0.x;
			auto y0 = v0.y;

			if( x0 < 0 || x0 >= w )
			{
				x0 = clamp( x0, w );
				y0 = line_y( x0, v0, v1 );
			}

			if( y0 < 0 || y0 >= h )
			{
				y0 = clamp( y0, h );
				x0 = line_x( y0, v0, v1 );
			}

			v0 = { x0, y0 };
		}
	}

	static void draw_wireframe( gil::rgb8_view_t const & view, Model const & model, gil::rgb8_pixel_t const & color )
	{
		auto const w = view.width();
		auto const h = view.height();

		for( auto const & face : model.faces )
		{
			for( size_t i = 0; i != 3; i++ )
			{
				auto a = model.vertices[ face.v[ i ] ];
				auto b = model.vertices[ face.v[ ( i + 1 ) % 3 ] ];

				auto v0 = to_view( w, h, a );
				auto v1 = to_view( w, h, b );

				if( !in_view( w, h, v0 ) &&
					!in_view( w, h, v1 ) )
				{
					continue;
				}

				clip_line( w, h, v0, v1 );

				draw_line( view, v0, v1, color );
			}
		}
	}

	void render_model( gil::rgb8_view_t const & view, Model const & model )
	{
		draw_wireframe( view, model, { 255, 255, 255 } );

		/*
		std::clog 
			<< model.vertices.size() << "\n"
			<< model.faces.size() 
			<< std::endl;*/
	}

} // ns sharpeye
