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
		, _light{ -1, -1, -1, 0 }
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
		_light = glm::dvec4{ v, 0 };
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

	static glm::dvec3 proj( glm::dvec4 const & v )
	{
		return glm::dvec3{ v } / v.w;
	}

	static glm::dvec3 as_vec( gil::rgb8_pixel_t const & p )
	{
		return { p[ 0 ], p[ 1 ], p[ 2 ] };
	}

	static gil::rgb8_pixel_t as_px( glm::dvec3 const & v )
	{
		return 
		{
			(gil::bits8) v.x,
			(gil::bits8) v.y,
			(gil::bits8) v.z
		};
	}

	static glm::dvec3 correct_barycentric( glm::dvec3 const & bc, glm::dvec3 const & w )
	{
		auto wp = 1.0 / ( bc[ 0 ] / w[ 0 ] + bc[ 1 ] / w[ 1 ] + bc[ 2 ] / w[ 2 ] );

		return 
		{
			bc[ 0 ] * wp / w[ 0 ],
			bc[ 1 ] * wp / w[ 1 ],
			bc[ 2 ] * wp / w[ 2 ]
		};
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

				auto p = glm::dot( glm::dvec3{ a.z, b.z, c.z }, bc );

				if( std::abs( p ) > 1 )
				{
					continue;
				}

				auto & z = zbuffer( i, j )[ 0 ];

				if( z > p )
				{
					z = (float) p;

					auto cbc = correct_barycentric( bc, 
						{ 
							vs[ 0 ][ 3 ], 
							vs[ 1 ][ 3 ], 
							vs[ 2 ][ 3 ] 
					} );

					auto val = std::min( 1.0, std::max( 0.0, glm::dot( lum, bc ) ) );

					auto uv = ts[ 0 ] * cbc[ 0 ] + ts[ 1 ] * cbc[ 1 ] + ts[ 2 ] * cbc[ 2 ];

					auto u = (int) ( uv.x * ( _diff.width() - 1 ) );
					auto v = (int) ( ( 1 - uv.y ) * ( _diff.height() - 1 ) );

					auto color = as_px( as_vec( _diff( u, v ) ) * val );

					_frame( i, j ) = color;
				}
			}
		}
	}

	void Render::set_diffuse_map( gil::rgb8_view_t img )
	{
		_diff = img;
	}

	static glm::dvec4 mul( glm::dmat4 const & m, glm::dvec3 const & v )
	{
		return m * glm::dvec4{ v, 1 };
	}

	void Render::draw( Model const & model )
	{
		double const w = _frame.width();
		double const h = _frame.height();

		glm::dmat4 const viewport = calc_viewport( _frame );
		glm::dmat4 const model_view = _view * model.m;
		glm::dvec3 const light = glm::normalize( glm::dvec3{ _proj * model_view * -_light } );

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

			glm::dvec3 lum
			{
				calc_intensity( light, normal_mtx, n0 ),
				calc_intensity( light, normal_mtx, n1 ),
				calc_intensity( light, normal_mtx, n2 )
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
