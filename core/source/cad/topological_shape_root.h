
#if !defined CMGUI_CAD_TOPOLOGICALSHAPEROOT_H
#define CMGUI_CAD_TOPOLOGICALSHAPEROOT_H

#include <list>

#include "topologicalshape.h"

class TopologicalShapeRoot
{
	public:
		TopologicalShapeRoot();
		~TopologicalShapeRoot();

		void child( TopologicalShape* s ) { m_children.push_back( s ); }
		TopologicalShape* lastChild() const { return m_children.back(); }
		std::list<TopologicalShape*> children() const { return m_children; }

	private:
		std::list<TopologicalShape*> m_children;
};


#endif
