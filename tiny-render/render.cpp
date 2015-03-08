#include "stdafx.h"
#include <random>
#include "render.h"
#include "line.h"

namespace sharpeye
{
	typedef gil::rgb8_view_t::point_t point_t;

	Render::Render( gil::rgb8_view_t const & frame )
		: _frame{ frame }
		, _zbuffer{ frame.width(), frame.height() }
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

	static glm::dvec3 barycentric( 
		glm::dvec3 const & a, 
		glm::dvec3 const & b, 
		glm::dvec3 const & c,
		point_t const & p )
	{
		auto u = glm::cross(
			glm::dvec3( c.x - a.x, b.x - a.x, a.x - p.x ), 
			glm::dvec3( c.y - a.y, b.y - a.y, a.y - p.y ) );

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

	static glm::dmat4 calc_viewport( gil::rgb8_view_t const & frame )
	{
		auto const w = frame.width();
		auto const h = frame.height();

		auto x = w / 3.0;
		auto dx = w / 2.0;

		auto y = h / 3.0;
		auto dy = h / 2.0;

		glm::dmat4 m{ 1.0 };

		m[ 0 ][ 0 ] = x;
		m[ 1 ][ 1 ] = y;

		m[ 3 ][ 0 ] = dx;
		m[ 3 ][ 1 ] = dy;

		return m;
	}

	static glm::dmat4 calc_proj( glm::dvec3 const & camera )
	{
		glm::dmat4 mat{ 1.0 };
		mat[ 2 ][ 3 ] = -1 / camera.z;
		return mat;
	}

	static glm::dvec3 to_view( glm::dvec3 const & p, glm::dmat4 const & m )
	{
		auto v = m * glm::dvec4{ p, 1 };
		v.x /= v.w;
		v.y /= v.w;
		v.z /= v.w;

		return glm::dvec3{ v };
	}

	void Render::draw( Model const & model )
	{
		glm::dvec3 const light{ 0, 0, -1 };
		glm::dvec3 const camera{ 0, 0, 10 };
		glm::dmat4 const proj = calc_proj( camera );
		glm::dmat4 const viewport = calc_viewport( _frame );

		auto const w = _frame.width();
		auto const h = _frame.height();

		auto const m = viewport * proj;

		for( auto const & face : model.faces )
		{
			auto const & v0 = model.vertices[ face.v[ 0 ] ];
			auto const & v1 = model.vertices[ face.v[ 1 ] ];
			auto const & v2 = model.vertices[ face.v[ 2 ] ];

			auto a = to_view( v0, m );
			auto b = to_view( v1, m );
			auto c = to_view( v2, m );

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
