#include "stdafx.h"
#include <random>
#include <glm/gtc/matrix_transform.hpp>
#include <tiny-render/render.h>

namespace sharpeye
{
	using point_t = gil::rgb8_view_t::point_t;
	using x_coord_t = gil::rgb8_view_t::x_coord_t;
	using y_coord_t = gil::rgb8_view_t::y_coord_t;

	static double clip( double v )
	{
		return std::min( 1.0, std::max( 0.0, v ) );
	}

	Render::Render( gil::rgb8_view_t const & frame )
		: _frame{ frame }
		, _zbuffer{ frame.width(), frame.height() }
		, _light{ 1, 1, 1 }
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

	void Render::set_light_dir( glm::dvec3 const & v )
	{
		_light = -v;
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

	static glm::dvec3 mul_v( glm::dmat4 const & m, glm::dvec3 const & v )
	{
		return glm::dvec3{ m * glm::dvec4{ v, 0 } };
	}

	static glm::dvec4 mul( glm::dmat4 const & m, glm::dvec3 const & v )
	{
		return m * glm::dvec4{ v, 1 };
	}

	static glm::dvec3 proj( glm::dvec4 const & v )
	{
		return glm::dvec3{ v } / v.w;
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

	static glm::dvec3 correct_barycentric( glm::dvec3 const & bc, double w1, double w2, double w3 )
	{
		auto wp = 1.0 / ( bc[ 0 ] / w1 + bc[ 1 ] / w2 + bc[ 2 ] / w3 );

		return 
		{
			bc[ 0 ] * wp / w1,
			bc[ 1 ] * wp / w2,
			bc[ 2 ] * wp / w3
		};
	}

	decltype(auto) calc_bbox( gil::rgb8_view_t const & view,
		glm::dvec3 const & a, 
		glm::dvec3 const & b, 
		glm::dvec3 const & c )
	{
		point_t lt
		{
			std::max( (x_coord_t) std::min( { a.x, b.x, c.x } ), 0 ),
			std::max( (y_coord_t) std::min( { a.y, b.y, c.y } ), 0 )
		};

		point_t rb
		{
			std::min( (x_coord_t) std::max( { a.x, b.x, c.x } ), view.width() - 1 ),
			std::min( (y_coord_t) std::max( { a.y, b.y, c.y } ), view.height() - 1 )
		};

		return std::make_pair( lt, rb );
	}

	static gil::rgb8_pixel_t calc_color( gil::rgb8_pixel_t const & px, double lum )
	{
		return 
		{
			(gil::bits8) ( px[ 0 ] * lum ),
			(gil::bits8) ( px[ 1 ] * lum ),
			(gil::bits8) ( px[ 2 ] * lum )
		};
	}

	static gil::rgb8_pixel_t pixel( gil::rgb8_view_t const & view, glm::dvec3 const & uv )
	{
		point_t location
		{
			(x_coord_t) ( uv.x * ( view.width() - 1 ) ),
			(y_coord_t) ( ( 1 - uv.y ) * ( view.height() - 1 ) )
		};

		return view( location );
	}

	static double interpolate( glm::dvec3 const & bc, glm::dvec3 const & v )
	{
		return glm::dot( v, bc );
	}

	static glm::dvec3 interpolate( glm::dvec3 const & bc, glm::dmat3 const & m )
	{
		return m[ 0 ] * bc[ 0 ] + m[ 1 ] * bc[ 1 ] + m[ 2 ] * bc[ 2 ];
	}

	void Render::fill_triangle(
		glm::dmat3x4 const & vs,
		glm::dmat3 const & ts,
		glm::dvec3 const & lum )
	{
		auto a = proj( vs[ 0 ] );
		auto b = proj( vs[ 1 ] );
		auto c = proj( vs[ 2 ] );

		auto zbuffer = gil::view( _zbuffer );

		point_t lt;
		point_t rb;
		std::tie( lt, rb ) = calc_bbox( _frame, a, b, c );

		for( auto i = lt.x; i <= rb.x; ++i )
		{
			for( auto j = lt.y; j <= rb.y; ++j )
			{
				auto bc = barycentric( a, b, c, { i, j } );

				if( bc.x < 0 || bc.y < 0 || bc.z < 0 )
				{
					continue;
				}

				auto p = interpolate( bc, { a.z, b.z, c.z } );

				if( std::abs( p ) > 1 )
				{
					continue;
				}

				auto & z = zbuffer( i, j )[ 0 ];

				if( z > p )
				{
					z = (float) p;

					auto cbc = correct_barycentric( bc, 
							vs[ 0 ][ 3 ], 
							vs[ 1 ][ 3 ], 
							vs[ 2 ][ 3 ] 
						);

					auto uv = interpolate( cbc, ts );
					auto intensity = interpolate( cbc, lum );

					_frame( i, j ) = calc_color( pixel( _diff, uv ), intensity );
				}
			}
		}
	}

	void Render::set_diffuse_map( gil::rgb8_view_t img )
	{
		_diff = img;
	}

	void Render::draw( Model const & model )
	{
		double const w = _frame.width();
		double const h = _frame.height();

		glm::dmat4 const viewport = calc_viewport( _frame );
		glm::dmat4 const model_view = _view * model.m;

		auto const m = viewport * _proj * model_view;
		//auto const normal_mtx = glm::transpose( glm::inverse( _proj * model_view ) );

		for( auto const & face : model.faces )
		{
			auto const & v0 = model.vertices[ face.v[ 0 ] ];
			auto const & v1 = model.vertices[ face.v[ 1 ] ];
			auto const & v2 = model.vertices[ face.v[ 2 ] ];

			auto const & n0 = model.normals[ face.n[ 0 ] ];
			auto const & n1 = model.normals[ face.n[ 1 ] ];
			auto const & n2 = model.normals[ face.n[ 2 ] ];

			glm::dvec3 lum
			{
				clip( glm::dot( _light, mul_v( model.m, n0 ) ) ),
				clip( glm::dot( _light, mul_v( model.m, n1 ) ) ),
				clip( glm::dot( _light, mul_v( model.m, n2 ) ) )
			};

			glm::dmat3x4 vs
			{ 
				mul( m, v0 ), 
				mul( m, v1 ), 
				mul( m, v2 )
			};

			glm::dmat3 ts
			{
				model.uv[ face.t[ 0 ] ],
				model.uv[ face.t[ 1 ] ],
				model.uv[ face.t[ 2 ] ]
			};

			fill_triangle( vs, ts, lum );
		}
	}

} // ns sharpeye
