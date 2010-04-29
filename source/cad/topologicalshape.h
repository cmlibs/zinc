
#if !defined CMGUI_CAD_TOPOLOGICAL_SHAPE_H
#define CMGUI_CAD_TOPOLOGICAL_SHAPE_H

#include <list>
#include <string>

#include <TopoDS_Shape.hxx>
#include <Quantity_Color.hxx>
#include <TopTools_IndexedMapOfShape.hxx>

class GeometricShape;

class TopologicalShape
{
	public:
		TopologicalShape( const TopoDS_Shape& s );
		~TopologicalShape();

		void shape( const TopoDS_Shape& s ) { m_shape = s; }
		void surfaceColour( const Quantity_Color& c ) { m_surfaceColour = c; }
		void curveColour( const Quantity_Color& c ) { m_curveColour = c; }
		void label( const char* name ) { m_label = std::string( name ); }

		TopoDS_Shape shape() const { return m_shape; }
		Quantity_Color surfaceColour() const { return m_surfaceColour; }
		int surfaceColour(double *colour) const;
		Quantity_Color curveColour() const { return m_curveColour; }
		const char*  label() const { return m_label.c_str(); }

		void tessellate( GeometricShape* geoShape );

	private:
		void clearMaps();
		void buildMaps();
		void extractPoints( GeometricShape* geoShape );
		void extractCurvesAndSurfaces( GeometricShape* geoShape );

	private:
		std::string m_label;
		TopoDS_Shape m_shape;
		Quantity_Color m_surfaceColour;
		Quantity_Color m_curveColour;

		TopTools_IndexedMapOfShape m_vmap, m_emap, m_fmap;
};


#endif
