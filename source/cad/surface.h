#ifndef SURFACE_H
#define SURFACE_H

#include "entity.h"

#include <vector>

#include <TopoDS_Face.hxx>
#include <Geom_Surface.hxx>
#include <GeomAPI_ProjectPointOnSurf.hxx>
#include <Poly_Triangle.hxx>
#include <Quantity_Color.hxx>
#include <Handle_Poly_Triangulation.hxx>
#include <Handle_TColgp_HArray1OfVec.hxx>
#include <TColgp_Array1OfDir.hxx>
/**
	@author user <hsorby@eggzachary>
*/
class Surface;
typedef const Surface *surface_id;

class Surface : public Entity
{
public:
    Surface( Entity* parent = 0 );
    Surface( TopoDS_Face face, Entity* parent = 0 );
    ~Surface();

    surface_id id() const {return this;}
    int uvPoints(int point_index, double &u, double &v) const;
    int surfacePoint(double u, double v, double *point, double *uDerivative = 0, double *vDerivative = 0) const;

    inline bool isEmpty() const { return m_surfaceUVRep.empty(); }
	inline int pointCount() const { return m_surfaceUVRep.size() / 2; }

	void tessellate();

private:
	Standard_Boolean triangleIsValid(const gp_Pnt& P1, const gp_Pnt& P2, const gp_Pnt& P3);

private:
    TopoDS_Face m_face;
	Handle_Geom_Surface m_surface;
	GeomAPI_ProjectPointOnSurf m_project;
	Quantity_Color m_colour;

    std::vector<double> m_surfaceUVRep;
};

#endif
