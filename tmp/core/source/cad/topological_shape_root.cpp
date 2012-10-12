
#include "topologicalshaperoot.h"

TopologicalShapeRoot::TopologicalShapeRoot()
{
}

TopologicalShapeRoot::~TopologicalShapeRoot()
{
	while ( m_children.size() > 0 )
	{
		delete m_children.back();
		m_children.pop_back();
	}
}

