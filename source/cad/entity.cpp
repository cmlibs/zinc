#include "entity.h"

Entity::Entity( Entity* parent )
    : m_parent( parent )
    , m_children()
{
}


Entity::~Entity()
{
}


