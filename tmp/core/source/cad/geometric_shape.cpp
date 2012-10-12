
#include "geometricshape.h"

GeometricShape::GeometricShape()
{
}

GeometricShape::~GeometricShape()
{
	while ( m_points.size() > 0 )
	{
		delete m_points.back();
		m_points.pop_back();
	}

	while ( m_curves.size() > 0 )
	{
		delete m_curves.back();
		m_curves.pop_back();
	}

	while ( m_surfaces.size() > 0 )
	{
		delete m_surfaces.back();
		m_surfaces.pop_back();
	}
}

