#include "point.h"

Point::Point( Entity* parent )
    : Entity( parent )
{
}

Point::Point( const gp_Pnt& pnt, Entity* parent )
    : Entity( parent )
    , m_pnt( pnt )
{
}

Point::~Point()
{
}


