#include "stdafx.h"
#include <random>
#include "render.h"
#include "line.h"

namespace sharpeye
{
	typedef gil::rgb8_view_t::point_t point_t;
	typedef gil::rgb8_view_t::x_coord_t x_coord_t;
	typedef gil::rgb8_view_t::y_coord_t y_coord_t;

	static point_t to_view( x_coord_t w, y_coord_t h, glm::dvec3 const & v )
	{
		auto x = static_cast< x_coord_t >( ( v.x + 1.0 ) * w / 2.0 );
        auto y = static_cast< y_coord_t >( ( v.y + 1.0 ) * h / 2.0 );

		return { x, y };
	}

	static bool in_view( x_coord_t w, y_coord_t h, point_t const & p )
	{
		return p.x < w && p.x >= 0 && p.y < h && p.y >= 0;
	}

	template< typename T >
	static T clamp( T v, T dim )
	{
		return (std::max)( T{}, (std::min)( v, dim - 1 ) );
	}

	static x_coord_t line_x( y_coord_t y, point_t & v0, point_t & v1 )
	{
		auto const dx = v1.x - v0.x;
		auto const dy = v1.y - v0.y;

		return static_cast< x_coord_t >( v0.x + ( y - v0.y ) * dx / (double) dy );
	}

	static y_coord_t line_y( x_coord_t x, point_t & v0, point_t & v1 )
	{
		auto const dx = v1.x - v0.x;
		auto const dy = v1.y - v0.y;

		return static_cast< y_coord_t >( v0.y + ( x - v0.x ) * dy / (double) dx );
	}

	static void clip_line( x_coord_t w, y_coord_t h, point_t & v0, point_t & v1 )
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

	void draw_wireframe( gil::rgb8_view_t const & view, Model const & model, gil::rgb8_pixel_t const & color )
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

	static glm::dvec3 barycentric( 
		point_t const & a, 
		point_t const & b, 
		point_t const & c,
		point_t const & p )
	{
		auto u = glm::cross(
			glm::dvec3( c.x - a.x, b.x - a.x, a.x - p.x ), 
			glm::dvec3( c.y - a.y, b.y - a.y, a.y - p.y ) );

		if( std::abs( u.z ) < 1 )
		{
			return { -1, -1, -1 };
		}
    
		return { 1. - ( u.x + u.y ) / u.z, u.y / u.z, u.x / u.z };
	}

	static void fill_triangle(
		gil::rgb8_view_t const & view,
		point_t const & a, 
		point_t const & b, 
		point_t const & c,
		gil::rgb8_pixel_t const & color )
	{
		point_t lt
		{
			std::max( 0, std::min( { a.x, b.x, c.x } ) ),
			std::max( 0, std::min( { a.y, b.y, c.y } ) )
		};

		point_t rb
		{
			std::min( view.width() - 1, std::max( { a.x, b.x, c.x } ) ),
			std::min( view.height() - 1, std::max( { a.y, b.y, c.y } ) )
		};

		for( auto i = lt.x; i != rb.x; ++i )
		{
			for( auto j = lt.y; j != rb.y; ++j )
			{
				point_t p{ i, j };
				auto u = barycentric( a, b, c, p );

				if( u.x < 0 || u.y < 0 || u.z < 0 )
				{
					continue;
				}

				view( p ) = color;
			}
		}
	}

	void render_model( gil::rgb8_view_t const & view, Model const & model )
	{
		glm::dvec3 const light{ 0, 0, -1 };

		auto const w = view.width();
		auto const h = view.height();

		for( auto const & face : model.faces )
		{
			auto const & v0 = model.vertices[ face.v[ 0 ] ];
			auto const & v1 = model.vertices[ face.v[ 1 ] ];
			auto const & v2 = model.vertices[ face.v[ 2 ] ];

			auto a = to_view( w, h, v0 );
			auto b = to_view( w, h, v1 );
			auto c = to_view( w, h, v2 );

			auto n = glm::normalize( glm::cross( v2 - v0, v1 - v0 ) );
			auto intensity = glm::dot( n, light );

			if( intensity > 0 )
			{
				auto color = static_cast< unsigned char >( intensity * 255 );

				fill_triangle( view, a, b, c, { color, color, color } );
			}
		}
	}

} // ns sharpeye
