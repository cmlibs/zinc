
#if !defined CMGUI_CAD_GEOMETRICSHAPE_H
#define CMGUI_CAD_GEOMETRICSHAPE_H

#include <vector>

#include <Quantity_Color.hxx>

#include "point.h"
#include "curve.h"
#include "surface.h"

class GeometricShape
{
	public:
		GeometricShape();
		~GeometricShape();

		void surfaceColour( const Quantity_Color& c ) { m_surfaceColour = c; }
		void curveColour( const Quantity_Color& c ) { m_curveColour = c; }

		const Quantity_Color& surfaceColour() const { return m_surfaceColour; }
		const Quantity_Color& curveColour() const { return m_curveColour; }

		int pointCount() const { return m_points.size(); }
		int curveCount() const { return m_curves.size(); }
		int surfaceCount() const { return m_surfaces.size(); }

		const Point* point( int index ) { return m_points.at( index ); }
		const Curve* curve( int index ) { return m_curves.at( index ); }
		const Surface* surface( int index ) { return m_surfaces.at( index ); }

		void append( Point* pt ) { m_points.push_back( pt ); }
		void append( Curve* c ) { m_curves.push_back( c ); }
		void append( Surface* s ) { m_surfaces.push_back( s ); }

	private:
		std::vector<Point*> m_points;
		std::vector<Curve*> m_curves;
		std::vector<Surface*> m_surfaces;

		Quantity_Color m_surfaceColour;
		Quantity_Color m_curveColour;
};


#endif
