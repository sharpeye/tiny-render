#include "stdafx.h"
#include <random>
#include "render.h"
#include "line.h"

namespace sharpeye
{
	typedef gil::rgb8_view_t::point_t point_t;
	typedef gil::rgb8_view_t::x_coord_t x_coord_t;
	typedef gil::rgb8_view_t::y_coord_t y_coord_t;

	Render::Render( gil::rgb8_view_t const & frame )
		: _frame( frame )
		, _zbuffer( frame.width(), frame.height() )
	{
		gil::fill_pixels( gil::view( _zbuffer ), gil::gray32f_pixel_t{ std::numeric_limits< float >::lowest() } );
	}

	gil::gray32f_view_t Render::zbuffer()
	{
		return gil::view( _zbuffer );
	}

	gil::rgb8_view_t Render::frame()
	{
		return _frame;
	}

	static glm::dvec3 to_view( x_coord_t w, y_coord_t h, glm::dvec3 const & v )
	{
		auto x = ( v.x + 1.0 ) * w / 2.0;
		auto y = ( v.y + 1.0 ) * h / 2.0;

		return { x, y, v.z };
	}

	static glm::dvec3 barycentric( 
		glm::dvec3 const & a, 
		glm::dvec3 const & b, 
		glm::dvec3 const & c,
		point_t const & p )
	{
		auto u = glm::cross(
			glm::dvec3( c.x - a.x, b.x - a.x, a.x - p.x ), 
			glm::dvec3( c.y - a.y, b.y - a.y, a.y - p.y ) );

		/*
		if( std::abs( u.z ) < 1 )
		{
			return { -1, -1, -1 };
		}//*/

		return { 1. - ( u.x + u.y ) / u.z, u.y / u.z, u.x / u.z };
	}

	auto calc_bbox( gil::rgb8_view_t const & view,
		glm::dvec3 const & a, 
		glm::dvec3 const & b, 
		glm::dvec3 const & c )
	{
		point_t lt
		{
			std::max( (std::ptrdiff_t) std::min( { a.x, b.x, c.x } ), 0 ),
			std::max( (std::ptrdiff_t) std::min( { a.y, b.y, c.y } ), 0 )
		};

		point_t rb
		{
			std::min( (std::ptrdiff_t) std::max( { a.x, b.x, c.x } ), view.width() - 1 ),
			std::min( (std::ptrdiff_t) std::max( { a.y, b.y, c.y } ), view.height() - 1 )
		};

		return std::make_pair( lt, rb );
	}

	void Render::fill_triangle(
		glm::dvec3 const & a, 
		glm::dvec3 const & b, 
		glm::dvec3 const & c,
		gil::rgb8_pixel_t const & color )
	{
		auto zbuffer = gil::view( _zbuffer );
		auto bbox = calc_bbox( _frame, a, b, c );

		for( auto i = bbox.first.x; i <= bbox.second.x; ++i )
		{
			for( auto j = bbox.first.y; j <= bbox.second.y; ++j )
			{
				auto u = barycentric( a, b, c, { i, j } );

				if( u.x < 0 || u.y < 0 || u.z < 0 )
				{
					continue;
				}

				auto p = a + u.y * ( b - a ) + u.z * ( c - a );

				if( zbuffer( i, j )[ 0 ] < p.z )
				{
					zbuffer( i, j )[ 0 ] = (float) p.z;
					_frame( i, j ) = color;
				}
			}
		}
	}

	void Render::draw( Model const & model )
	{
		std::vector< glm::dvec3 > vs;

		glm::dvec3 const light{ 0, 0, -1 };

		auto const w = _frame.width();
		auto const h = _frame.height();

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
				auto color = static_cast< gil::bits8 >( intensity * 255 );

				fill_triangle( a, b, c, { color, color, color } );
			}
		}
	}

} // ns sharpeye
