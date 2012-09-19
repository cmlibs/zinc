
extern "C"
{
#include "computed_field/computed_field.h"
#include "general/debug.h"
}
#include "cad/element_identifier.h"
#include "cad/computed_field_cad_topology.h"


Cmiss_cad_identifier::Cmiss_cad_identifier(Cmiss_field_cad_topology_id cad_topology, Cad_primitive_identifier cad_identifier)
	: cad_topology(cad_topology)
	, identifier(cad_identifier)
{
	Cmiss_field_cad_topology_access(this->cad_topology);
	//DEBUG_PRINT("Constructor %p %d\n", this, Cmiss_field_get_access_count(reinterpret_cast<Cmiss_field_id>(this->cad_topology)));
}

Cmiss_cad_identifier::~Cmiss_cad_identifier()
{
	//DEBUG_PRINT("Destructor %p %d\n", this, Cmiss_field_get_access_count(reinterpret_cast<Cmiss_field_id>(this->cad_topology))-1);
	Cmiss_field_cad_topology_destroy(&(this->cad_topology));
}

Cmiss_cad_identifier::Cmiss_cad_identifier(const Cmiss_cad_identifier& cad_identifier)
{
	this->cad_topology = cad_identifier.cad_topology;
	Cmiss_field_cad_topology_access(this->cad_topology);
	//DEBUG_PRINT("Copy constructor %d\n", Cmiss_field_get_access_count(reinterpret_cast<Cmiss_field_id>(this->cad_topology)));
	this->identifier = cad_identifier.identifier;
}

Cmiss_cad_identifier& Cmiss_cad_identifier::operator=(const Cmiss_cad_identifier& source)
{
	//DEBUG_PRINT("Assignment operator\n");
	Cmiss_cad_identifier_id result = new Cmiss_cad_identifier(source);
	return (*result);
}

bool Cmiss_cad_identifier::operator==(const Cmiss_cad_identifier& other) const
{
	bool result = false;
	//DEBUG_PRINT("Comparison operator\n");
	if (this->cad_topology == other.cad_topology &&
		this->identifier.number == other.identifier.number &&
		this->identifier.type == other.identifier.type)
		result = true;

	return result;
}

bool Cmiss_cad_identifier::operator!=(const Cmiss_cad_identifier& other) const
{
	return !(*this == other);
}




