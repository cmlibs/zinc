#ifndef OCCPARTFACTORY_H
#define OCCPARTFACTORY_H

#include <TopoDS_Shape.hxx>
// #include <Handle_AIS_Shape.hxx>

/**
	@author user <hsorby@eggzachary>
*/
class OCCPartFactory
{
public:
    OCCPartFactory();

    ~OCCPartFactory();

    static TopoDS_Shape makeBottle( const Standard_Real width,
                                    const Standard_Real height,
                                    const Standard_Real depth);
    
    static TopoDS_Shape makeCube( const Standard_Real width,
                                  const Standard_Real height,
                                  const Standard_Real depth);
    
    static TopoDS_Shape makeCylinder( const Standard_Real radius,
                                      const Standard_Real height);

};

#endif
