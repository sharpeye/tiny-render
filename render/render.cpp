#include "stdafx.h"
#include <random>
#include <glm/gtc/matrix_transform.hpp>
#include <tiny-render/render.h>

namespace sharpeye
{
	typedef gil::rgb8_view_t::point_t point_t;

	Render::Render( gil::rgb8_view_t const & frame )
		: _frame{ frame }
		, _zbuffer{ frame.width(), frame.height() }
	{
		gil::fill_pixels( gil::view( _zbuffer ), gil::gray32f_pixel_t{ 2.0f } );
	}

	gil::gray32f_view_t Render::zbuffer()
	{
		return gil::view( _zbuffer );
	}

	gil::rgb8_view_t Render::frame()
	{
		return _frame;
	}

	void Render::set_view_matrix( glm::dmat4 const & m )
	{
		_view = m;
	}

	void Render::set_proj_matrix( glm::dmat4 const & m )
	{
		_proj = m;
	}

	static glm::dmat4 calc_viewport( double x, double y, double w, double h )
	{
		glm::dmat4 m{ 1.0 };

		m[ 3 ][ 0 ] = x + w / 2.0;
		m[ 3 ][ 1 ] = y + h / 2.0;
		m[ 3 ][ 2 ] = 0;

		m[ 0 ][ 0 ] = w / 2.0;
		m[ 1 ][ 1 ] = h / 2.0;
		m[ 2 ][ 2 ] = 1;

		return m;
	}

	static glm::dmat4 calc_viewport( gil::rgb8_view_t const & frame )
	{
		auto const w = frame.width();
		auto const h = frame.height();

		return calc_viewport( 0, 0, w, h );
	}

	static glm::dmat4 calc_proj( double k )
	{
		glm::dmat4 mat{ 1.0 };
		mat[ 2 ][ 3 ] = k;
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

	static double calc_intensity( glm::dvec3 const & light, glm::dmat4 const & m, glm::dvec3 const & n )
	{
		return std::max( glm::dot( glm::dvec3{ m * glm::dvec4{ n, 0 } }, light ), 0.0 );
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
		glm::dvec3 const & intensity )
	{
		auto zbuffer = gil::view( _zbuffer );

		point_t lt;
		point_t rb;
		std::tie( lt, rb ) = calc_bbox( _frame, a, b, c );

		for( auto i = lt.x; i <= rb.x; ++i )
		{
			for( auto j = lt.y; j <= rb.y; ++j )
			{
				auto u = barycentric( a, b, c, { i, j } );

				if( u.x < 0 || u.y < 0 || u.z < 0 )
				{
					continue;
				}

				auto p = static_cast< float >( glm::dot( glm::dvec3{ a.z, b.z, c.z }, u ) );

				if( std::abs( p ) > 1 )
				{
					continue;
				}

				auto & z = zbuffer( i, j )[ 0 ];

				if( z > p )
				{
					z = p;

					auto val = std::min( 1.0, std::max( 0.0, glm::dot( intensity, u ) ) );
					auto color = (gil::bits8) ( val * 255 );

					_frame( i, j ) = { color, color, color };
				}
			}
		}
	}

	void Render::draw( Model const & model )
	{
		double const w = _frame.width();
		double const h = _frame.height();

		glm::dmat4 const viewport = calc_viewport( _frame );
		glm::dmat4 const model_view = _view * model.m;

		glm::dvec3 const light = glm::normalize( glm::dvec3{ _proj * model_view * glm::dvec4{
			1, 1, 1, 0  // why not (-1,-1,-1) ?
		} } );

		auto const m = viewport * _proj * model_view;
		auto const normal_mtx = glm::transpose( glm::inverse( _proj * model_view ) );

		for( auto const & face : model.faces )
		{
			auto const & v0 = model.vertices[ face.v[ 0 ] ];
			auto const & v1 = model.vertices[ face.v[ 1 ] ];
			auto const & v2 = model.vertices[ face.v[ 2 ] ];

			auto const & n0 = model.normals[ face.n[ 0 ] ];
			auto const & n1 = model.normals[ face.n[ 1 ] ];
			auto const & n2 = model.normals[ face.n[ 2 ] ];

			glm::dvec3 intensity
			{
				calc_intensity( light, normal_mtx, n0 ),
				calc_intensity( light, normal_mtx, n1 ),
				calc_intensity( light, normal_mtx, n2 )
			};

			auto a = to_view( v0, m );
			auto b = to_view( v1, m );
			auto c = to_view( v2, m );

			fill_triangle( a, b, c, intensity );
		}
	}

} // ns sharpeye
