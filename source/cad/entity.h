#ifndef ENTITY_H
#define ENTITY_H

#include <string>
#include <list>
/**
    @author user <hsorby@eggzachary>
*/
class Entity
{
public:
    // all known entity types
    enum GeomType
    {
        Unknown,
        Pointt,
        BoundaryLayerPoint,
        Line,
        Circle,
        Ellipse,
        Conic,
        Parabola,
        Hyperbola,
        TrimmedCurve,
        OffsetCurve,
        BSpline,
        Bezier,
        ParametricCurve,
        BoundaryLayerCurve,
        DiscreteCurve,
        Plane,
        Nurb,
        Cylinder,
        Sphere,
        Cone,
        Torus,
        RuledSurface,
        ParametricSurface,
        ProjectionFace,
        BSplineSurface,
        BezierSurface,
        SurfaceOfRevolution,
        BoundaryLayerSurface,
        DiscreteSurface,
        CompoundSurface,
        Volume,
        DiscreteVolume
    };

    // return a string describing the entity type
    std::string geomTypeString()
    {
        const char *name[] =
        {
            "Unknown",
            "Point",
            "Boundary layer point",
            "Line",
            "Circle",
            "Ellipse",
            "Conic",
            "Parabola",
            "Hyperbola",
            "TrimmedCurve",
            "OffsetCurve",
            "BSpline",
            "Bezier",
            "Parametric curve",
            "Boundary layer curve",
            "Discrete curve",
            "Plane",
            "Nurb",
            "Cylinder",
            "Sphere",
            "Cone",
            "Torus",
            "Ruled surface",
            "Parametric surface",
            "Projection surface",
            "BSpline surface",
            "Bezier surface",
            "Surface of Revolution",
            "Boundary layer surface",
            "Discrete surface",
            "Compound surface",
            "Volume",
            "Discrete volume"
        };
        unsigned int type = (unsigned int)geomType();
        if ( type >= ( sizeof(name) / sizeof(name[0]) ) )
            return std::string( "Undefined" );
        else
            return std::string( name[type] );
    }

    virtual GeomType geomType() const { return Unknown; }
    
    Entity( Entity* parent = 0 );
    virtual ~Entity();

private:
    Entity* m_parent;
    std::list<Entity*> m_children;

};

#endif
