//******************************************************************************
// FILE : variable_finite_element.cpp
//
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
// Finite element types - element/xi and finite element field.
//==============================================================================

#include <new>
#include <string>
#include <stdio.h>
#include <iostream>

//???DB.  Put in include?
const bool Assert_on=true;

template<class Assertion,class Exception>inline void Assert(
	Assertion assertion,Exception exception)
{
	if (Assert_on&&!(assertion)) throw exception;
}

#include "computed_variable/variable_finite_element.hpp"
#include "computed_variable/variable_vector.hpp"
extern "C"
{
#include "finite_element/finite_element_region.h"
//???DB.  Get rid of debug.h (C->C++)
#include "general/debug.h"
}

// module classes
// ==============

// class Variable_input_element_xi
// -------------------------------

class Variable_input_element_xi : public Variable_input
//******************************************************************************
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_element_xi;
	friend class Variable_finite_element;
	friend class Variable_finite_element_check_derivative_functor;
	public:
		Variable_input_element_xi(
			const Variable_finite_element_handle& variable_finite_element,
			bool element=true,bool xi=true):element(element),xi(xi),indices(0),
			variable_element_xi(),variable_finite_element(variable_finite_element){}
		Variable_input_element_xi(
			const Variable_finite_element_handle& variable_finite_element,
			Variable_size_type index):element(false),xi(true),indices(1),
			variable_element_xi(),variable_finite_element(variable_finite_element)
		{
			indices[0]=index;
		}
		Variable_input_element_xi(
			const Variable_finite_element_handle& variable_finite_element,
			const boost::numeric::ublas::vector<Variable_size_type>& indices):
			element(false),xi(true),indices(indices),variable_element_xi(),
			variable_finite_element(variable_finite_element){}
		Variable_input_element_xi(
			const Variable_element_xi_handle& variable_element_xi,bool element=true,
			bool xi=true):element(element),xi(xi),indices(0),
			variable_element_xi(variable_element_xi),variable_finite_element(){}
		Variable_input_element_xi(
			const Variable_element_xi_handle& variable_element_xi,
			Variable_size_type index):element(false),xi(true),indices(1),
			variable_element_xi(variable_element_xi),variable_finite_element()
		{
			indices[0]=index;
		}
		Variable_input_element_xi(
			const Variable_element_xi_handle& variable_element_xi,
			const boost::numeric::ublas::vector<Variable_size_type>& indices):
			element(false),xi(true),indices(indices),
			variable_element_xi(variable_element_xi),variable_finite_element(){}
		~Variable_input_element_xi(){};
		Variable_size_type size()
		{
			Variable_size_type result;

			result=0;
			if (xi)
			{
				result=indices.size();
				if (0==result)
				{
					if (variable_element_xi)
					{
						result=variable_element_xi->size();
					}
					else if (variable_finite_element)
					{
						result=variable_finite_element->size();
					}
				}
			}

			return (result);
		};
		virtual bool operator==(const Variable_input& input)
		{
			try
			{
				const Variable_input_element_xi& input_element_xi=
					dynamic_cast<const Variable_input_element_xi&>(input);
				bool result;

				result=false;
				if ((input_element_xi.variable_element_xi==variable_element_xi)&&
					(input_element_xi.variable_finite_element==variable_finite_element)&&
					(input_element_xi.element==element)&&(input_element_xi.xi==xi)&&
					(input_element_xi.indices.size()==indices.size()))
				{
					int i=indices.size();

					result=true;
					while (result&&(i>0))
					{
						i--;
						if (!(indices[i]==input_element_xi.indices[i]))
						{
							result=false;
						}
					}
				}

				return (result);
			}
			catch (std::bad_cast)
			{
				return (false);
			};
		};
	private:
		bool element,xi;
		boost::numeric::ublas::vector<Variable_size_type> indices;
		Variable_element_xi_handle variable_element_xi;
		Variable_finite_element_handle variable_finite_element;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_element_xi>
	Variable_input_element_xi_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_element_xi>
	Variable_input_element_xi_handle;
#else
typedef Variable_input_element_xi * Variable_input_element_xi_handle;
#endif

// class Variable_input_nodal_values
// ---------------------------------

struct Count_nodal_values_data
{
	enum FE_nodal_value_type value_type;
	int component_number,number_of_components,number_of_values,version;
	struct FE_field *fe_field;
	int *node_offsets,number_of_node_offsets,*number_of_node_values;
	struct FE_node **offset_nodes;
}; /* struct Count_nodal_values_data */

static int component_count_nodal_values(struct FE_node *node,
	struct FE_field *fe_field,int component_number,
	enum FE_nodal_value_type value_type,int version)
/*******************************************************************************
LAST MODIFIED : 29 April 2003

DESCRIPTION :
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_type,*nodal_value_types;
	int i,number_of_versions,number_of_values;

	ENTER(component_count_nodal_values);
	number_of_values=1+get_FE_node_field_component_number_of_derivatives(node,
		fe_field,component_number);
	if (FE_NODAL_UNKNOWN!=value_type)
	{
		/*???DB.  Could use FE_nodal_value_version_exists instead */
		i=number_of_values;
		number_of_values=0;
		if (nodal_value_types=get_FE_node_field_component_nodal_value_types(node,
			fe_field,component_number))
		{
			nodal_value_type=nodal_value_types;
			while ((i>0)&&(value_type!= *nodal_value_type))
			{
				nodal_value_type++;
				i--;
			}
			if (i>0)
			{
				number_of_values++;
			}
			DEALLOCATE(nodal_value_types);
		}
	}
	number_of_versions=get_FE_node_field_component_number_of_versions(node,
		fe_field,component_number);
	if ((version<0)||(version>number_of_versions))
	{
		number_of_values *= number_of_versions;
	}
	LEAVE;

	return (number_of_values);
} /* component_count_nodal_values */

static int count_nodal_values(struct FE_node *node,
	void *count_nodal_values_data_void)
/*******************************************************************************
LAST MODIFIED : 29 April 2003

DESCRIPTION :
Counts the number of nodal values specified by <count_nodal_values_data_void> at
the <node>.
==============================================================================*/
{
	int component_number,node_number,number_of_values,return_code;
	struct Count_nodal_values_data *count_nodal_values_data;
	struct FE_node **offset_nodes;

	ENTER(count_nodal_values);
	return_code=0;
	/* check arguments */
	if (node&&(count_nodal_values_data=(struct Count_nodal_values_data *)
		count_nodal_values_data_void))
	{
		if (offset_nodes=count_nodal_values_data->offset_nodes)
		{
			node_number=0;
			while ((node_number<count_nodal_values_data->number_of_node_offsets)&&
				(node!=offset_nodes[node_number]))
			{
				node_number++;
			}
			if (node_number<count_nodal_values_data->number_of_node_offsets)
			{
				(count_nodal_values_data->node_offsets)[node_number]=
					count_nodal_values_data->number_of_values;
			}
		}
		if ((0<=(component_number=count_nodal_values_data->component_number))&&
			(component_number<count_nodal_values_data->number_of_components))
		{
			number_of_values=component_count_nodal_values(node,
				count_nodal_values_data->fe_field,component_number,
				count_nodal_values_data->value_type,
				count_nodal_values_data->version);
		}
		else
		{
			number_of_values=0;
			for (component_number=0;component_number<
				count_nodal_values_data->number_of_components;component_number++)
			{
				number_of_values += component_count_nodal_values(node,
					count_nodal_values_data->fe_field,component_number,
					count_nodal_values_data->value_type,
					count_nodal_values_data->version);
			}
		}
		if ((offset_nodes=count_nodal_values_data->offset_nodes)&&
			(node_number<count_nodal_values_data->number_of_node_offsets))
		{
			(count_nodal_values_data->number_of_node_values)[node_number]=
				number_of_values;
		}
		count_nodal_values_data->number_of_values += number_of_values;
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* count_nodal_values */

class Variable_input_nodal_values : public Variable_input
//******************************************************************************
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	friend class Variable_finite_element;
	friend class Variable_finite_element_check_derivative_functor;
	public:
		Variable_input_nodal_values(
			const Variable_finite_element_handle& variable_finite_element):
			value_type(FE_NODAL_UNKNOWN),version(-1),node((struct FE_node *)NULL),
			variable_finite_element(variable_finite_element){};
		Variable_input_nodal_values(
			const Variable_finite_element_handle& variable_finite_element,
			struct FE_node *node,enum FE_nodal_value_type value_type,int version):
			value_type(value_type),version(version),node(node),
			variable_finite_element(variable_finite_element)
		{
			if (node)
			{
				ACCESS(FE_node)(node);
			}
		};
		Variable_input_nodal_values(
			const Variable_finite_element_handle& variable_finite_element,
			struct FE_node *node):value_type(FE_NODAL_UNKNOWN),version(-1),
			node(node),variable_finite_element(variable_finite_element)
		{
			if (node)
			{
				ACCESS(FE_node)(node);
			}
		};
		Variable_input_nodal_values(
			const Variable_finite_element_handle& variable_finite_element,
			enum FE_nodal_value_type value_type):value_type(value_type),version(-1),
			node((struct FE_node *)NULL),
			variable_finite_element(variable_finite_element){};
		Variable_input_nodal_values(
			const Variable_finite_element_handle& variable_finite_element,
			int version):value_type(FE_NODAL_UNKNOWN),version(version),
			node((struct FE_node *)NULL),
			variable_finite_element(variable_finite_element){};
		~Variable_input_nodal_values()
		{
			DEACCESS(FE_node)(&node);
		};
		Variable_size_type size()
		{
			int component_number,number_of_components,return_code;
			struct Count_nodal_values_data count_nodal_values_data;
			struct FE_field *fe_field;
			struct FE_region *fe_region;
			Variable_size_type result;

			result=0;
			if (variable_finite_element&&(fe_field=variable_finite_element->field)&&
				(fe_region=FE_field_get_FE_region(fe_field))&&
				(0<(number_of_components=get_FE_field_number_of_components(fe_field))))
			{
				//???DB.  Have component_number in variable_finite_element?
				component_number= -1;
				count_nodal_values_data.number_of_values=0;
				count_nodal_values_data.value_type=value_type;
				count_nodal_values_data.version=version;
				count_nodal_values_data.fe_field=fe_field;
				count_nodal_values_data.component_number=component_number;
				count_nodal_values_data.number_of_components=number_of_components;
				count_nodal_values_data.number_of_node_offsets=0;
				count_nodal_values_data.offset_nodes=(struct FE_node **)NULL;
				count_nodal_values_data.node_offsets=(int *)NULL;
				count_nodal_values_data.number_of_node_values=(int *)NULL;
				if (node)
				{
					return_code=count_nodal_values(node,(void *)&count_nodal_values_data);
				}
				else
				{
					return_code=FE_region_for_each_FE_node(fe_region,count_nodal_values,
						(void *)&count_nodal_values_data);
				}
				if (return_code)
				{
					result=(Variable_size_type)(count_nodal_values_data.number_of_values);
				}
			}

			return (result);
		};
		virtual bool operator==(const Variable_input& input)
		{
			try
			{
				const Variable_input_nodal_values& input_nodal_values=
					dynamic_cast<const Variable_input_nodal_values&>(input);

				return (
					(input_nodal_values.variable_finite_element==
					variable_finite_element)&&
					(input_nodal_values.value_type==value_type)&&
					(input_nodal_values.version==version)&&
					(input_nodal_values.node==node));
			}
			catch (std::bad_cast)
			{
				return (false);
			};
		};
	private:
		enum FE_nodal_value_type value_type;
		int version;
		struct FE_node *node;
		Variable_finite_element_handle variable_finite_element;
};

#if defined (USE_INTRUSIVE_SMART_POINTER)
typedef boost::intrusive_ptr<Variable_input_nodal_values>
	Variable_input_nodal_values_handle;
#elif defined (USE_SMART_POINTER)
typedef boost::shared_ptr<Variable_input_nodal_values>
	Variable_input_nodal_values_handle;
#else
typedef Variable_input_nodal_values * Variable_input_nodal_values_handle;
#endif


// global classes
// ==============

// class Variable_element_xi
// -------------------------

Variable_element_xi::Variable_element_xi(struct FE_element *element,
	const Vector& xi):Variable(),xi(xi),element(element)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (element)
	{
		if (xi.size()==(Variable_size_type)get_FE_element_dimension(element))
		{
			ACCESS(FE_element)(element);
		}
		else
		{
			throw std::invalid_argument("Variable_element_xi::Variable_element_xi");
		}
	}
}

Variable_element_xi::Variable_element_xi(
	const Variable_element_xi& variable_element_xi):Variable(),
	xi(variable_element_xi.xi),element(variable_element_xi.element)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
	if (element)
	{
		if (xi.size()==(Variable_size_type)get_FE_element_dimension(element))
		{
			ACCESS(FE_element)(element);
		}
		else
		{
			throw std::invalid_argument("Variable_element_xi::Variable_element_xi");
		}
	}
}

Variable_element_xi& Variable_element_xi::operator=(
	const Variable_element_xi& variable_element_xi)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	Variable_size_type number_of_xi=variable_element_xi.xi.size();

	if (variable_element_xi.element)
	{
		if ((0==number_of_xi)||
			(number_of_xi==(Variable_size_type)get_FE_element_dimension(
			variable_element_xi.element)))
		{
			if (element)
			{
				DEACCESS(FE_element)(&element);
				element=ACCESS(FE_element)(variable_element_xi.element);
			}
			xi.resize(number_of_xi);
			xi=variable_element_xi.xi;
		}
		else
		{
			throw std::invalid_argument("Variable_element_xi::operator=");
		}
	}
	else
	{
		if (element)
		{
			DEACCESS(FE_element)(&element);
		}
		xi.resize(number_of_xi);
		xi=variable_element_xi.xi;
	}

	return (*this);
}

Variable_element_xi::~Variable_element_xi()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	DEACCESS(FE_element)(&element);
}

Variable_size_type Variable_element_xi::size()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (xi.size());
}

Vector *Variable_element_xi::scalars()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (new Vector(xi));
}

Variable_input_handle Variable_element_xi::input_element_xi()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Returns the element/xi input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_element_xi(
		Variable_element_xi_handle(this))));
}

Variable_input_handle Variable_element_xi::input_element()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Returns the element input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_element_xi(
		Variable_element_xi_handle(this),true,false)));
}

Variable_input_handle Variable_element_xi::input_xi()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_element_xi(
		Variable_element_xi_handle(this),false)));
}

Variable_input_handle Variable_element_xi::input_xi(Variable_size_type index)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_element_xi(
		Variable_element_xi_handle(this),index)));
}

Variable_input_handle Variable_element_xi::input_xi(
	const boost::numeric::ublas::vector<Variable_size_type> indices)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_element_xi(
		Variable_element_xi_handle(this),indices)));
}

Variable_handle Variable_element_xi::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Duplicate <this> so that <this> isn't changed by operations on the result.
//==============================================================================
{
	return (Variable_handle(new Variable_element_xi(*this)));
}

void Variable_element_xi::evaluate_derivative_local(Matrix& matrix,
	std::list<Variable_input_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 12 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_size_type i,index,number_of_input_values,number_of_values;
	Variable_input_element_xi_handle input_element_xi_handle;

	// matrix is zero'd on entry
	if ((1==independent_variables.size())&&(input_element_xi_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_element_xi,Variable_input>(
		independent_variables.front())
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_element_xi *>(independent_variables.front())
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_element_xi_handle->variable_element_xi==Variable_handle(this))&&
		(input_element_xi_handle->xi))
	{
		number_of_values=this->size();
		number_of_input_values=(input_element_xi_handle->indices).size();
		Assert((number_of_values==matrix.size1())&&((0==number_of_input_values)||
			(number_of_input_values==matrix.size2())),std::logic_error(
			"Variable_element_xi::evaluate_derivative_local.  "
			"Incorrect matrix size"));
		if (0==number_of_input_values)
		{
			for (i=0;i<number_of_values;i++)
			{
				matrix(i,i)=1;
			}
		}
		else
		{
			for (i=0;i<number_of_input_values;i++)
			{
				index=input_element_xi_handle->indices[i];
				if ((0<index)&&(index<=number_of_values))
				{
					matrix(index-1,i)=1;
				}
			}
		}
	}
}

Variable_handle Variable_element_xi::get_input_value_local(
	const Variable_input_handle& input)
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_handle value_element_xi;
	Variable_input_element_xi_handle input_element_xi_handle;

	if ((input_element_xi_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_element_xi,Variable_input>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_element_xi *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_element_xi_handle->variable_element_xi==
		Variable_element_xi_handle(this)))
	{
		if (input_element_xi_handle->xi)
		{
			Variable_size_type number_of_input_values=
				input_element_xi_handle->size();

			if (0==(input_element_xi_handle->indices).size())
			{
				if (input_element_xi_handle->element)
				{
					value_element_xi=Variable_element_xi_handle(new Variable_element_xi(
						element,xi));
				}
				else
				{
					value_element_xi=Variable_vector_handle(new Variable_vector(xi));
				}
			}
			else
			{
				Variable_size_type i,index,number_of_values=this->size();
				boost::numeric::ublas::vector<Scalar>
					selected_values(number_of_input_values);

				for (i=0;i<number_of_input_values;i++)
				{
					index=input_element_xi_handle->indices[i];
					if ((0<index)&&(index<=number_of_values))
					{
						selected_values[i]=xi[index-1];
					}
				}
#if defined (OLD_CODE)
				//???DB.  Doesn't make sense to have element and indices
				if (input_element_xi_handle->element)
				{
					value_element_xi=Variable_element_xi_handle(
						new Variable_element_xi(element,selected_values));
				}
				else
				{
#endif // defined (OLD_CODE)
					value_element_xi=Variable_vector_handle(
						new Variable_vector(selected_values));
#if defined (OLD_CODE)
				}
#endif // defined (OLD_CODE)
			}
		}
		else
		{
			if (input_element_xi_handle->element)
			{
				value_element_xi=Variable_element_xi_handle(
					new Variable_element_xi(element,Vector(0)));
			}
			else
			{
				value_element_xi=Variable_vector_handle(new Variable_vector(Vector(0)));
			}
		}
	}
	else
	{
		value_element_xi=Variable_element_xi_handle((Variable_element_xi *)0);
	}

	return (value_element_xi);
}

int Variable_element_xi::set_input_value_local(
	const Variable_input_handle& input,const Variable_handle& values)
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	int return_code;
	Variable_input_element_xi_handle input_element_xi_handle;

	return_code=0;
	if ((input_element_xi_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_element_xi,Variable_input>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_element_xi *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_element_xi_handle->variable_element_xi==
		Variable_element_xi_handle(this)))
	{
		if (input_element_xi_handle->element)
		{
			Variable_element_xi_handle values_element_xi_handle;

			if (values_element_xi_handle=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_element_xi,Variable>(values)
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_element_xi *>(values)
#endif /* defined (USE_SMART_POINTER) */
				)
			{
				if (input_element_xi_handle->xi)
				{
					Variable_size_type number_of_input_values=
						(input_element_xi_handle->indices).size();

					if (0==number_of_input_values)
					{
						Variable_size_type number_of_xi=
							(values_element_xi_handle->xi).size();

						if (!(values_element_xi_handle->element)||
							(number_of_xi==(Variable_size_type)get_FE_element_dimension(
							values_element_xi_handle->element)))
						{
							DEACCESS(FE_element)(&element);
							if (values_element_xi_handle->element)
							{
								element=ACCESS(FE_element)(values_element_xi_handle->element);
							}
							xi.resize(number_of_xi);
							xi=values_element_xi_handle->xi;
							return_code=1;
						}
					}
				}
			}
		}
		else
		{
			if (input_element_xi_handle->xi)
			{
				Vector *values_vector;

				if (values_vector=values->scalars())
				{
					Variable_size_type number_of_input_values=
						(input_element_xi_handle->indices).size();

					if (0==number_of_input_values)
					{
						if ((this->xi).size()==values_vector->size())
						{
							this->xi= *values_vector;
							return_code=1;
						}
					}
					else
					{
						if (number_of_input_values==values_vector->size())
						{
							Variable_size_type i,index,number_of_values=(this->xi).size();

							for (i=0;i<number_of_input_values;i++)
							{
								index=input_element_xi_handle->indices[i];
								if ((0<index)&&(index<=number_of_values))
								{
									(this->xi)[index-1]=(*values_vector)[i];
								}
							}
							return_code=1;
						}
					}
					delete values_vector;
				}
			}
		}
	}

	return (return_code);
}

string_handle Variable_element_xi::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		if (element)
		{
			out << "element=" << FE_element_get_cm_number(element) << " ";
		}
		out << "xi=" << xi;
		*return_string=out.str();
	}

	return (return_string);
}

// class Variable_finite_element
// -----------------------------

Variable_finite_element::Variable_finite_element(struct FE_field *field):
	Variable(),component_number(-1),time(0),element((struct FE_element *)NULL),
	field(field),node((struct FE_node *)NULL),xi()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (field)
	{
		ACCESS(FE_field)(field);
	}
}

Variable_finite_element::Variable_finite_element(struct FE_field *field,
	int component_number):Variable(),component_number(component_number-1),time(0),
	element((struct FE_element *)NULL),field(field),node((struct FE_node *)NULL),
	xi()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (field)
	{
		ACCESS(FE_field)(field);
	}
}

Variable_finite_element::Variable_finite_element(struct FE_field *field,
	const std::string component_name):Variable(),component_number(-1),time(0),
	element((struct FE_element *)NULL),field(field),node((struct FE_node *)NULL),
	xi()
//******************************************************************************
// LAST MODIFIED : 12 November 2003
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	if (field)
	{
		char *name;
		int i;

		i=get_FE_field_number_of_components(field);
		if (0<i)
		{
			name=(char *)NULL;
			do
			{
				i--;
				if (name)
				{
					DEALLOCATE(name);
				}
				name=get_FE_field_component_name(field,i);
			} while ((i>0)&&(std::string(name)!=component_name));
			if (std::string(name)==component_name)
			{
				component_number=i;
			}
		}
		ACCESS(FE_field)(field);
	}
}

Variable_finite_element::Variable_finite_element(
	const Variable_finite_element& variable_finite_element):
	Variable(),component_number(variable_finite_element.component_number),
	time(variable_finite_element.time),element(variable_finite_element.element),
	field(variable_finite_element.field),node(variable_finite_element.node),
	xi(variable_finite_element.xi)
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Copy constructor.
//==============================================================================
{
}

Variable_finite_element& Variable_finite_element::operator=(
	const Variable_finite_element& variable_finite_element)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// Assignment operator.
//==============================================================================
{
	component_number=variable_finite_element.component_number;
	time=variable_finite_element.time;
	DEACCESS(FE_element)(&element);
	if (variable_finite_element.element)
	{
		element=ACCESS(FE_element)(variable_finite_element.element);
	}
	DEACCESS(FE_field)(&field);
	if (variable_finite_element.field)
	{
		field=ACCESS(FE_field)(variable_finite_element.field);
	}
	DEACCESS(FE_node)(&node);
	if (variable_finite_element.node)
	{
		node=ACCESS(FE_node)(variable_finite_element.node);
	}
	xi.resize(variable_finite_element.xi.size());
	xi=variable_finite_element.xi;

	return (*this);
}

Variable_finite_element::~Variable_finite_element()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
	DEACCESS(FE_element)(&element);
	DEACCESS(FE_field)(&field);
	DEACCESS(FE_node)(&node);
}

Variable_size_type Variable_finite_element::size()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_size_type result;

	result=get_FE_field_number_of_components(field);
	if (-1!=component_number)
	{
		if ((0<=component_number)&&((Variable_size_type)component_number<result))
		{
			result=1;
		}
		else
		{
			result=0;
		}
	}

	return (result);
}

Vector *Variable_finite_element::scalars()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	return (evaluate_local()->scalars());
}

Variable_input_handle Variable_finite_element::input_element_xi()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Returns the element/xi input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_element_xi(
		Variable_finite_element_handle(this))));
}

Variable_input_handle Variable_finite_element::input_element()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Returns the element input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_element_xi(
		Variable_finite_element_handle(this),true,false)));
}

Variable_input_handle Variable_finite_element::input_xi()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_element_xi(
		Variable_finite_element_handle(this),false)));
}

Variable_input_handle Variable_finite_element::input_xi(
	Variable_size_type index)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_element_xi(
		Variable_finite_element_handle(this),index)));
}

Variable_input_handle Variable_finite_element::input_xi(
	const boost::numeric::ublas::vector<Variable_size_type> indices)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
// Returns the xi input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_element_xi(
		Variable_finite_element_handle(this),indices)));
}

Variable_input_handle Variable_finite_element::input_nodal_values()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_nodal_values(
		Variable_finite_element_handle(this))));
}

Variable_input_handle Variable_finite_element::input_nodal_values(
	struct FE_node *node,enum FE_nodal_value_type value_type,int version)
//******************************************************************************
// LAST MODIFIED : 9 November 2003
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_nodal_values(
		Variable_finite_element_handle(this),node,value_type,version)));
}

Variable_input_handle Variable_finite_element::input_nodal_values(
	struct FE_node *node)
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_nodal_values(
		Variable_finite_element_handle(this),node)));
}

Variable_input_handle Variable_finite_element::input_nodal_values(
	enum FE_nodal_value_type value_type)
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_nodal_values(
		Variable_finite_element_handle(this),value_type)));
}

Variable_input_handle Variable_finite_element::input_nodal_values(int version)
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
// Returns the nodal values input.
//==============================================================================
{
	return (Variable_input_handle(new Variable_input_nodal_values(
		Variable_finite_element_handle(this),version)));
}

Variable_handle Variable_finite_element::evaluate_local()
//******************************************************************************
// LAST MODIFIED : 7 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	int element_dimension,number_of_components;
	FE_value *values,*xi_coordinates;
	Variable_handle result;

	values=(FE_value *)NULL;
	xi_coordinates=(FE_value *)NULL;
	if (field&&
		(0<(number_of_components=get_FE_field_number_of_components(field)))&&
		ALLOCATE(values,FE_value,number_of_components)&&((node&&!element)||
		(!node&&element&&(0<(element_dimension=get_FE_element_dimension(element)))&&
		((Variable_size_type)element_dimension==xi.size())&&ALLOCATE(xi_coordinates,
		FE_value,element_dimension))))
	{
		int i;
		Variable_size_type j,number_of_values;

		if (0<=component_number)
		{
			number_of_values=1;
		}
		else
		{
			number_of_values=(Variable_size_type)number_of_components;
		}
		for (i=0;i<element_dimension;i++)
		{
			xi_coordinates[i]=(FE_value)(xi[i]);
		}
		if (calculate_FE_field(field,component_number,node,element,xi_coordinates,
			(FE_value)time,values))
		{
			Vector result_vector(number_of_values);

			for (j=0;j<number_of_values;j++)
			{
				result_vector[j]=values[j];
			}

			result=Variable_handle(new Variable_vector(result_vector));
		}
		else
		{
			result=Variable_handle(new Variable_vector(Vector(0)));
		}
	}
	else
	{
		result=Variable_handle(new Variable_vector(Vector(0)));
	}
	DEALLOCATE(xi_coordinates);
	DEALLOCATE(values);

	return (result);
}

static int extract_component_values(
	struct FE_element_field_values *element_field_values,
	int number_of_components,int component_number,
	int **numbers_of_component_values_address,
	FE_value ***component_values_address)
/*******************************************************************************
LAST MODIFIED : 9 November 2003

DESCRIPTION :
Extracts the values for the component(s) (<number_of_components> and
<component_number>) from <element_field_values> and puts in
<*component_values_address> (after allocating storage).

If <*numbers_of_component_values_address> is NULL then storage is allocated for
it and the number of values extracted for each component entered.  Otherwise,
checks that the number of values extracted for each component agrees with the
entries.
==============================================================================*/
{
	FE_value **component_values;
	int extract_numbers_of_component_values,i,number_of_component_values,
		*numbers_of_component_values,return_code;

	ENTER(extract_component_values);
	return_code=0;
	Assert(element_field_values&&
		(((1==number_of_components)&&(0<=component_number))||
		((0<number_of_components)&&(-1==component_number)))&&
		numbers_of_component_values_address&&component_values_address,
		std::logic_error("extract_component_values.  Invalid argument(s)"));
	extract_numbers_of_component_values=
		!(*numbers_of_component_values_address);
	/* allocate storage for components */
	if (extract_numbers_of_component_values)
	{
		ALLOCATE(numbers_of_component_values,int,number_of_components);
	}
	else
	{
		numbers_of_component_values= *numbers_of_component_values_address;
	}
	ALLOCATE(component_values,FE_value *,number_of_components);
	if (component_values&&numbers_of_component_values)
	{
		if (-1==component_number)
		{
			i=0;
			return_code=1;
			while (return_code&&(i<number_of_components))
			{
				if (return_code=FE_element_field_values_get_component_values(
					element_field_values,i,&number_of_component_values,
					component_values+i))
				{
					if (extract_numbers_of_component_values)
					{
						numbers_of_component_values[i]=number_of_component_values;
					}
					else
					{
						if (numbers_of_component_values[i]!=number_of_component_values)
						{
							DEALLOCATE(component_values[i]);
							return_code=0;
						}
					}
					i++;
				}
			}
			if (!return_code)
			{
				while (i>0)
				{
					i--;
					DEALLOCATE(component_values[i]);
				}
			}
		}
		else
		{
			if (return_code=FE_element_field_values_get_component_values(
				element_field_values,component_number,&number_of_component_values,
				component_values))
			{
				if (extract_numbers_of_component_values)
				{
					*numbers_of_component_values=number_of_component_values;
				}
				else
				{
					if (*numbers_of_component_values!=number_of_component_values)
					{
						DEALLOCATE(*component_values);
						return_code=0;
					}
				}
			}
		}
		if (return_code)
		{
			*component_values_address=component_values;
			*numbers_of_component_values_address=numbers_of_component_values;
		}
	}
	if (!return_code)
	{
		DEALLOCATE(component_values);
		if (extract_numbers_of_component_values)
		{
			DEALLOCATE(numbers_of_component_values);
		}
	}
	LEAVE;

	return (return_code);
} /* extract_component_values */

static int extract_component_monomial_info(
	struct FE_element_field_values *element_field_values,
	int number_of_components,int component_number,int number_of_xi,
	int **numbers_of_component_values_address,
	int ***component_monomial_info_address,FE_value **monomial_values_address)
/*******************************************************************************
LAST MODIFIED : 9 November 2003

DESCRIPTION :
Extracts the component monomial info (<number_of_components> and
<component_number>) from <element_field_values> and puts in
<*component_monomial_info_address> (after allocating storage).

If <*numbers_of_component_values_address> is NULL then storage is allocated for
it and the number of values for each component calculated and entered.
Otherwise, checks that the number of values extracted for each component agrees
with the entries.

If <monomial_values_address> is not NULL and <*monomial_values_address> is NULL,
then storage for the maximum number of component values is allocated and
assigned to <*monomial_values_address>.
==============================================================================*/
{
	FE_value *monomial_values;
	int **component_monomial_info,extract_numbers_of_component_values,i,j,
		maximum_number_of_component_values,number_of_component_values,
		*numbers_of_component_values,return_code;

	ENTER(extract_component_monomial_info);
	return_code=0;
	Assert(element_field_values&&
		(((1==number_of_components)&&(0<=component_number))||
		((0<number_of_components)&&(-1==component_number)))&&(0<number_of_xi)&&
		numbers_of_component_values_address&&component_monomial_info_address,
		std::logic_error("extract_component_monomial_info.  Invalid argument(s)"));
	/* allocate storage for components */
	extract_numbers_of_component_values=
		!(*numbers_of_component_values_address);
	/* allocate storage for components */
	if (extract_numbers_of_component_values)
	{
		ALLOCATE(numbers_of_component_values,int,number_of_components);
	}
	else
	{
		numbers_of_component_values= *numbers_of_component_values_address;
	}
	ALLOCATE(component_monomial_info,int *,number_of_components);
	if (numbers_of_component_values&&component_monomial_info)
	{
		i=0;
		return_code=1;
		maximum_number_of_component_values=0;
		while (return_code&&(i<number_of_components))
		{
			if (ALLOCATE(component_monomial_info[i],int,number_of_xi+1))
			{
				if (-1==component_number)
				{
					return_code=FE_element_field_values_get_monomial_component_info(
						element_field_values,i,component_monomial_info[i]);
				}
				else
				{
					return_code=FE_element_field_values_get_monomial_component_info(
						element_field_values,component_number,component_monomial_info[i]);
				}
				if (return_code)
				{
					number_of_component_values=1;
					for (j=1;j<=component_monomial_info[i][0];j++)
					{
						number_of_component_values *= 1+component_monomial_info[i][j];
					}
					if (number_of_component_values>maximum_number_of_component_values)
					{
						maximum_number_of_component_values=number_of_component_values;
					}
					if (extract_numbers_of_component_values)
					{
						numbers_of_component_values[i]=number_of_component_values;
					}
					else
					{
						if (numbers_of_component_values[i]!=number_of_component_values)
						{
							return_code=0;
						}
					}
				}
				i++;
			}
		}
		if (return_code)
		{
			if (monomial_values_address&&!(*monomial_values_address))
			{
				if ((0<maximum_number_of_component_values)&&ALLOCATE(monomial_values,
					FE_value,maximum_number_of_component_values))
				{
					*monomial_values_address=monomial_values;
				}
				else
				{
					return_code=0;
				}
			}
			if (return_code)
			{
				*numbers_of_component_values_address=numbers_of_component_values;
				*component_monomial_info_address=component_monomial_info;
			}
		}
		if (!return_code)
		{
			while (i>0)
			{
				i--;
				DEALLOCATE(component_monomial_info[i]);
			}
		}
	}
	if (!return_code)
	{
		DEALLOCATE(component_monomial_info);
		if (extract_numbers_of_component_values)
		{
			DEALLOCATE(numbers_of_component_values);
		}
	}
	LEAVE;

	return (return_code);
} /* extract_component_monomial_info */

static int nodal_value_calculate_component_values(
	Variable_finite_element_handle fe_variable,struct FE_field *fe_field,
	int component_number,FE_value fe_time,struct FE_element *element,
	struct FE_node *node,enum FE_nodal_value_type nodal_value_type,int version,
	int *number_of_element_field_nodes_address,
	struct FE_node ***element_field_nodes_address,
	struct FE_element_field_values *element_field_values,
	int *number_of_nodal_values_address,int **numbers_of_component_values_address,
	FE_value ****component_values_address)
/*******************************************************************************
LAST MODIFIED : 26 November 2003

DESCRIPTION :
Calculate the component values for the derivatives of the specified components
(<component_number>) of <fe_field> in the <element> with respect to the
specified nodal values (<node>, <nodal_value_type> and <version>.

<*number_of_nodal_values_address> is calculated and an array with an entry for
each of the specified nodal values (<*number_of_nodal_values_address> long> is
allocated, NULLed and assigned to <*component_values_address>.  For each nodal
value, if the derivative is not identically zero then a 2 dimensional array
(first index is component number and second index is component value number) is
allocated, filled-in and assigned to the corresponding entry in
<*component_values_address>.

The interpolation function for the <fe_field> component(s) is assumed to be a
linear combination of basis functions polynomial in the element coordinates.
The coefficients for the linear combination are the nodal values.  So, the
derivative with respect to a nodal value is the corresponding basis function.
The component values are for the monomials.  To calculate these, all the nodal
values for the <element> are set to zero, the nodal value of interest is set to
one and <calculate_FE_element_field_values> is used.

<*number_of_element_field_nodes_address> and <*element_field_nodes_address> are
for the <fe_field> and <element> and can be passed in or computed in here.
<element_field_values> is working storage that is passed in.

???DB.  Can get all from <fe_variable>.  Do multiple times?
==============================================================================*/
{
	FE_value ***component_values,***nodal_value_component_values;
	int *element_field_nodal_value_offsets,*element_field_number_of_nodal_values,
		*element_field_number_of_specified_nodal_values,i,j,k,number_of_components,
		number_of_element_field_nodes,number_of_nodal_values,
		number_of_saved_element_field_nodes,number_of_values,number_of_versions,
		return_code;
	struct Count_nodal_values_data count_nodal_values_data;
	struct FE_region *fe_region;
	struct FE_node **element_field_nodes;

	ENTER(nodal_value_calculate_component_values);
	return_code=0;
	/* check arguments */
	fe_region=FE_field_get_FE_region(fe_field);
	number_of_components=get_FE_field_number_of_components(fe_field);
	Assert(fe_variable&&fe_field&&fe_region&&(0<number_of_components)&&
		((-1==component_number)||
		((0<=component_number)&&(component_number<number_of_components)))&&
		element&&element_field_values&&number_of_nodal_values_address&&
		component_values_address,std::logic_error(
		"nodal_value_calculate_component_values.  Invalid argument(s)"));
	{
		if (-1!=component_number)
		{
			number_of_components=1;
		}
		if (number_of_element_field_nodes_address&&element_field_nodes_address)
		{
			number_of_element_field_nodes= *number_of_element_field_nodes_address;
			element_field_nodes= *element_field_nodes_address;
		}
		else
		{
			number_of_element_field_nodes=0;
			element_field_nodes=(struct FE_node **)NULL;
		}
		if ((number_of_element_field_nodes>0)&&element_field_nodes)
		{
			return_code=0;
		}
		else
		{
			return_code=calculate_FE_element_field_nodes(element,
				fe_field,&number_of_element_field_nodes,
				&element_field_nodes,(struct FE_element *)NULL);
		}
		if (return_code)
		{
			//???DB.  To be done/Needs finishing
			/* set up working storage */
			Variable_handle *element_field_saved_nodal_values;

			ALLOCATE(element_field_nodal_value_offsets,int,
				number_of_element_field_nodes);
			ALLOCATE(element_field_number_of_specified_nodal_values,int,
				number_of_element_field_nodes);
			ALLOCATE(element_field_number_of_nodal_values,int,
				number_of_element_field_nodes);
			element_field_saved_nodal_values=
				new Variable_handle[number_of_element_field_nodes];
			if (element_field_nodal_value_offsets&&
				element_field_number_of_specified_nodal_values&&
				element_field_number_of_nodal_values&&element_field_saved_nodal_values)
			{
				/* NULL working storage and calculate total number of nodal values for
					each <element_field_node> */
				for (i=0;i<number_of_element_field_nodes;i++)
				{
					element_field_saved_nodal_values[i]=Variable_handle(0);
					element_field_nodal_value_offsets[i]= -1;
					element_field_number_of_specified_nodal_values[i]=0;
					element_field_number_of_nodal_values[i]=0;
					if (-1==component_number)
					{
						for (j=0;j<number_of_components;j++)
						{
							number_of_values=
								(1+get_FE_node_field_component_number_of_derivatives(
								element_field_nodes[i],fe_field,j));
							number_of_versions=
								get_FE_node_field_component_number_of_versions(
								element_field_nodes[i],fe_field,j);
							if ((version<0)||(version>number_of_versions))
							{
								element_field_number_of_nodal_values[i] +=
									number_of_values*number_of_versions;
							}
							else
							{
								element_field_number_of_nodal_values[i] += number_of_values;
							}
						}
					}
					else
					{
						number_of_values=
							(1+get_FE_node_field_component_number_of_derivatives(
							element_field_nodes[i],fe_field,component_number));
						number_of_versions=
							get_FE_node_field_component_number_of_versions(
							element_field_nodes[i],fe_field,component_number);
						if ((version<0)||(version>number_of_versions))
						{
							element_field_number_of_nodal_values[i] +=
								number_of_values*number_of_versions;
						}
						else
						{
							element_field_number_of_nodal_values[i] += number_of_values;
						}
					}
				}
				/* calculate total number of specified nodal values (<number_of_values>)
					and the number of specified nodal values for the
					<element_field_nodes> */
				count_nodal_values_data.number_of_values=0;
				count_nodal_values_data.value_type=nodal_value_type;
				count_nodal_values_data.version=version;
				count_nodal_values_data.fe_field=fe_field;
				count_nodal_values_data.component_number=component_number;
				count_nodal_values_data.number_of_components=number_of_components;
				count_nodal_values_data.number_of_node_offsets=
					number_of_element_field_nodes;
				count_nodal_values_data.offset_nodes=element_field_nodes;
				count_nodal_values_data.node_offsets=element_field_nodal_value_offsets;
				count_nodal_values_data.number_of_node_values=
					element_field_number_of_specified_nodal_values;
				if (node)
				{
					return_code=count_nodal_values(node,(void *)&count_nodal_values_data);
				}
				else
				{
					return_code=FE_region_for_each_FE_node(fe_region,
						count_nodal_values,(void *)&count_nodal_values_data);
				}
				number_of_nodal_values=count_nodal_values_data.number_of_values;
				if (return_code&&(0<number_of_nodal_values))
				{
					/* allocate storage for the component_values */
					if (return_code&&ALLOCATE(component_values,FE_value **,
						number_of_nodal_values))
					{
						/* NULL the component values array */
						nodal_value_component_values=component_values;
						for (i=number_of_nodal_values;i>0;i--)
						{
							*nodal_value_component_values=(FE_value **)NULL;
							nodal_value_component_values++;
						}
						/* save and zero all the nodal values for the <fe_field> on the
							element */
						number_of_saved_element_field_nodes=0;
						while (return_code&&(number_of_saved_element_field_nodes<
							number_of_element_field_nodes))
						{
							Variable_input_nodal_values_handle 
								input(new Variable_input_nodal_values(fe_variable,
								element_field_nodes[number_of_saved_element_field_nodes]));

							if (element_field_saved_nodal_values[
								number_of_saved_element_field_nodes]=
								(fe_variable->get_input_value)(input))
							{
								Variable_size_type i,number_of_values=
									(element_field_saved_nodal_values[
									number_of_saved_element_field_nodes])->size();
								Vector zero_vector(number_of_values);

								number_of_saved_element_field_nodes++;
								for (i=0;i<number_of_values;i++)
								{
									zero_vector[i]=(Scalar)0;
								}
								return_code=(fe_variable->set_input_value)(input,
									Variable_vector_handle(new Variable_vector(zero_vector)));
							}
							else
							{
								return_code=0;
							}
						}
						/* calculate component values for nodal value derivatives */
						i=0;
						while (return_code&&(i<number_of_element_field_nodes))
						{
							if ((element_field_nodal_value_offsets[i]>=0)&&
								(element_field_number_of_specified_nodal_values[i]>0))
							{
								Variable_size_type j,number_of_values=(Variable_size_type)
									(element_field_number_of_specified_nodal_values[i]);
								Variable_vector_handle unit_vector(new Variable_vector(
									Vector(number_of_values)));
								Variable_input_nodal_values_handle 
									input(new Variable_input_nodal_values(fe_variable,
									element_field_nodes[i],nodal_value_type,version));

								for (j=0;j<number_of_values;j++)
								{
									(*unit_vector)[j]=(Scalar)0;
								}
								nodal_value_component_values=component_values+
									element_field_nodal_value_offsets[i];
								Assert((int)number_of_values==
									element_field_number_of_specified_nodal_values[i],
									std::logic_error("nodal_value_calculate_component_values.  "
									"Incorrect number of nodal values"));
								j=0;
								while (return_code&&(j<number_of_values))
								{
									/* set nodal values */
									(*unit_vector)[j]=(Scalar)1;
									if ((fe_variable->set_input_value)(input,unit_vector)&&
										clear_FE_element_field_values(element_field_values)&&
										FE_element_field_values_set_no_modify(
										element_field_values)&&
										calculate_FE_element_field_values(element,fe_field,
										fe_time,(char)0,element_field_values,
										(struct FE_element *)NULL))
									{
										return_code=extract_component_values(
											element_field_values,number_of_components,
											component_number,numbers_of_component_values_address,
											nodal_value_component_values);
									}
									else
									{
										return_code=0;
									}
									/* set nodal values */
									(*unit_vector)[j]=(Scalar)0;
									nodal_value_component_values++;
									j++;
								}
								if (return_code)
								{
									// zero the last 1 from the above loop
									return_code=(fe_variable->set_input_value)(input,unit_vector);
								}
								if (!return_code)
								{
									while (j>0)
									{
										j--;
										nodal_value_component_values--;
										if (*nodal_value_component_values)
										{
											for (k=0;k<number_of_components;k++)
											{
												DEALLOCATE((*nodal_value_component_values)[k]);
											}
											DEALLOCATE(*nodal_value_component_values);
										}
									}
								}
							}
							i++;
						}
						if (return_code)
						{
							*number_of_nodal_values_address=number_of_nodal_values;
							*component_values_address=component_values;
						}
						else
						{
							while (i>0)
							{
								i--;
								if ((element_field_nodal_value_offsets[i]>=0)&&
									(element_field_number_of_specified_nodal_values[i]>0))
								{
									nodal_value_component_values=component_values+
										element_field_nodal_value_offsets[i];
									for (j=0;
										j<element_field_number_of_specified_nodal_values[i];j++)
									{
										if (*nodal_value_component_values)
										{
											for (k=0;k<number_of_components;k++)
											{
												DEALLOCATE((*nodal_value_component_values)[k]);
											}
										}
										DEALLOCATE(*nodal_value_component_values);
										nodal_value_component_values++;
									}
								}
							}
							DEALLOCATE(component_values);
						}
						/* reset all the nodal values for the <fe_field> on the element */
						i=0;
						while (return_code&&(i<number_of_saved_element_field_nodes))
						{
							Variable_input_nodal_values_handle 
								input(new Variable_input_nodal_values(fe_variable,
								element_field_nodes[i]));

							return_code=(fe_variable->set_input_value)(input,
								element_field_saved_nodal_values[i]);
							i++;
						}
					}
					else
					{
						return_code=0;
					}
				}
				else
				{
					return_code=0;
				}
				/* destroy working computed values for saving current values and
					setting temporary values */
				for (i=0;i<number_of_element_field_nodes;i++)
				{
					element_field_saved_nodal_values[i]=Variable_vector_handle(0);
				}
			}
			else
			{
				return_code=0;
			}
			/* get rid of working storage */
			delete [] element_field_saved_nodal_values;
			DEALLOCATE(element_field_number_of_nodal_values);
			DEALLOCATE(element_field_number_of_specified_nodal_values);
			DEALLOCATE(element_field_nodal_value_offsets);
		}
		if (return_code&&number_of_element_field_nodes_address&&
			element_field_nodes_address)
		{
			*number_of_element_field_nodes_address=number_of_element_field_nodes;
			*element_field_nodes_address=element_field_nodes;
		}
	}
	LEAVE;

	return (return_code);
} /* nodal_value_calculate_component_values */

static int calculate_monomial_derivative_values(int number_of_xi_coordinates,
	int *xi_orders,int *xi_derivative_orders,FE_value *xi_coordinates,
	FE_value *monomial_derivative_values)
/*******************************************************************************
LAST MODIFIED : 9 November 2003

DESCRIPTION :
For the specified monomials (<number_of_xi_coordinates> and order<=<xi_orders>
for each xi), calculates there derivatives (<xi_derivative_orders>) at
<xi_coordinates> and stores in <monomial_derivative_values>.  Expects the memory
to already be allocated for the <monomial_derivative_values>.
NB.  xi_1 is varying slowest (xi_n fastest)
==============================================================================*/
{
	FE_value *temp_value,*value,xi,*xi_coordinate,xi_power;
	int derivative_order,i,j,k,order,number_of_values,return_code,
		*xi_derivative_order,*xi_order,zero_derivative;

	ENTER(calculate_monomial_derivative_values);
	return_code=0;
	Assert((0<number_of_xi_coordinates)&&(xi_order=xi_orders)&&
		(xi_derivative_order=xi_derivative_orders)&&(xi_coordinate=xi_coordinates)&&
		monomial_derivative_values,std::logic_error(
		"calculate_monomial_derivative_values.  Invalid argument(s)"));
	{
		/* check for zero derivative */
		zero_derivative=0;
		number_of_values=1;
		for (i=number_of_xi_coordinates;i>0;i--)
		{
			order= *xi_order;
			if (*xi_derivative_order>order)
			{
				zero_derivative=1;
			}
			number_of_values *= order+1;
			xi_derivative_order++;
			xi_order++;
		}
		value=monomial_derivative_values;
		if (zero_derivative)
		{
			/* zero derivative */
			for (k=number_of_values;k>0;k--)
			{
				*value=(FE_value)0;
				value++;
			}
		}
		else
		{
			xi_order=xi_orders;
			xi_derivative_order=xi_derivative_orders;
			*value=1;
			number_of_values=1;
			for (i=number_of_xi_coordinates;i>0;i--)
			{
				xi= *xi_coordinate;
				xi_coordinate++;
				derivative_order= *xi_derivative_order;
				xi_derivative_order++;
				order= *xi_order;
				xi_order++;
				if (0<derivative_order)
				{
					xi_power=1;
					for (j=derivative_order;j>1;j--)
					{
						xi_power *= (FE_value)j;
					}
					value += number_of_values*(derivative_order-1);
					for (j=derivative_order;j<=order;j++)
					{
						temp_value=monomial_derivative_values;
						for (k=number_of_values;k>0;k--)
						{
							value++;
							*value=(*temp_value)*xi_power;
							temp_value++;
						}
						xi_power *= xi*(float)(j+1)/(float)(j-derivative_order+1);
					}
					temp_value=monomial_derivative_values;
					for (k=derivative_order*number_of_values;k>0;k--)
					{
						*temp_value=0;
						temp_value++;
					}
				}
				else
				{
					xi_power=xi;
					for (j=order;j>0;j--)
					{
						temp_value=monomial_derivative_values;
						for (k=number_of_values;k>0;k--)
						{
							value++;
							*value=(*temp_value)*xi_power;
							temp_value++;
						}
						xi_power *= xi;
					}
				}
				number_of_values *= (order+1);
			}
		}
		return_code=1;
	}
	LEAVE;

	return (return_code);
} /* calculate_monomial_derivative_values */

class Variable_finite_element_check_derivative_functor
//******************************************************************************
// LAST MODIFIED : 12 November 2003
//
// DESCRIPTION :
//
// NOTES :
// 1. Composite inputs are already handled by evaluate_derivative.
//
// ???DB.  Only valid for monomial standard basis and nodal value based.  This
//   is enforced by <FE_element_field_values_get_monomial_component_info> and
//   <FE_element_field_values_get_component_values>.
//==============================================================================
{
	public:
		Variable_finite_element_check_derivative_functor(
			Variable_finite_element_handle variable_finite_element,
			bool& zero_derivative,
			Variable_input_nodal_values_handle& nodal_values_input,
			boost::numeric::ublas::vector<Variable_input_element_xi_handle>::iterator
			element_xi_input_iterator,Variable_size_type& number_of_columns):
			first(true),zero_derivative(zero_derivative),
			variable_finite_element(variable_finite_element),
			nodal_values_input(nodal_values_input),
			element_xi_input_iterator(element_xi_input_iterator),
			number_of_columns(number_of_columns){};
		~Variable_finite_element_check_derivative_functor(){};
		int operator() (Variable_input_handle& input)
		{
			if (first)
			{
				number_of_columns=input->size();
			}
			else
			{
				number_of_columns *= input->size();
			}
			*element_xi_input_iterator=Variable_input_element_xi_handle(0);
			if (!zero_derivative)
			{
				Variable_input_element_xi_handle input_element_xi_handle;
				Variable_input_nodal_values_handle input_nodal_values_handle;

				if ((input_element_xi_handle=
#if defined (USE_SMART_POINTER)
					boost::dynamic_pointer_cast<Variable_input_element_xi,Variable_input>(
					input)
#else /* defined (USE_SMART_POINTER) */
					dynamic_cast<Variable_input_element_xi *>(input)
#endif /* defined (USE_SMART_POINTER) */
					)&&(variable_finite_element==
					input_element_xi_handle->variable_finite_element)&&
					(input_element_xi_handle->xi))
				{
					// not checking order of derivative against order of polynomial
					*element_xi_input_iterator=input_element_xi_handle;
				}
				else if ((input_nodal_values_handle=
#if defined (USE_SMART_POINTER)
					boost::dynamic_pointer_cast<Variable_input_nodal_values,
					Variable_input>(input)
#else /* defined (USE_SMART_POINTER) */
					dynamic_cast<Variable_input_nodal_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
					)&&(variable_finite_element==
					input_nodal_values_handle->variable_finite_element))
				{
					if (nodal_values_input)
					{
						// because the nodal values are the coefficients for a linear
						//   combination
						zero_derivative=true;
					}
					else
					{
						nodal_values_input=input_nodal_values_handle;
					}
				}
				else
				{
					zero_derivative=true;
				}
				first=false;
			}
			element_xi_input_iterator++;

			return (1);
		};
	private:
		bool first;
		bool& zero_derivative;
		Variable_finite_element_handle variable_finite_element;
		Variable_input_nodal_values_handle& nodal_values_input;
		boost::numeric::ublas::vector<Variable_input_element_xi_handle>::iterator
			element_xi_input_iterator;
		Variable_size_type& number_of_columns;
};

void Variable_finite_element::evaluate_derivative_local(Matrix& matrix,
	std::list<Variable_input_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 26 November 2003
//
// DESCRIPTION :
// ???DB.  Throw an exception for failure?
//==============================================================================
{
	bool zero_derivative;
	Variable_size_type derivative_order=independent_variables.size(),
		number_of_columns,number_of_components;
	Variable_input_nodal_values_handle nodal_values_input(0);
	boost::numeric::ublas::vector<Variable_input_element_xi_handle>
		element_xi_inputs(derivative_order);

	// matrix is zero'd on entry
	// check independent variables for a zero matrix
	number_of_components=this->size();
	zero_derivative=false;
	number_of_columns=0;
	std::for_each(independent_variables.begin(),independent_variables.end(),
		Variable_finite_element_check_derivative_functor(
		Variable_finite_element_handle(this),zero_derivative,
		nodal_values_input,element_xi_inputs.begin(),number_of_columns));
	Assert((number_of_components==matrix.size1())&&
		(number_of_columns==matrix.size2()),
		std::logic_error("Variable_finite_element::evaluate_derivative_local.  "
		"Incorrect matrix size"));
	if (!zero_derivative)
	{
		FE_value **component_values=(FE_value **)NULL,
			*monomial_derivative_values=(FE_value *)NULL,
			***nodal_value_component_values=(FE_value ***)NULL;
		int **component_monomial_info=(int **)NULL,
			*numbers_of_component_values=(int *)NULL,number_of_element_field_nodes=0,
			number_of_nodal_values=0,return_code=0;
		struct FE_element_field_values *element_field_values;
		struct FE_node **element_field_nodes=(struct FE_node **)NULL;
		Variable_size_type number_of_xi=xi.size();

		// set up temporary storage
		element_field_values=CREATE(FE_element_field_values)();
		if (element_field_values)
		{
			if (nodal_values_input)
			{
				if (nodal_value_calculate_component_values(
					Variable_finite_element_handle(this),field,component_number,
					(FE_value)time,element,nodal_values_input->node,
					nodal_values_input->value_type,nodal_values_input->version,
					&number_of_element_field_nodes,&element_field_nodes,
					element_field_values,&number_of_nodal_values,
					&numbers_of_component_values,&nodal_value_component_values)&&
					extract_component_monomial_info(element_field_values,
					number_of_components,component_number,number_of_xi,
					&numbers_of_component_values,&component_monomial_info,
					&monomial_derivative_values))
				{
					return_code=1;
				}
			}
			else
			{
				if (clear_FE_element_field_values(element_field_values)&&
					calculate_FE_element_field_values(element,field,(FE_value)time,
					(char)0,element_field_values,(struct FE_element *)NULL)&&
					extract_component_values(element_field_values,number_of_components,
					component_number,&numbers_of_component_values,&component_values)&&
					extract_component_monomial_info(element_field_values,
					number_of_components,component_number,number_of_xi,
					&numbers_of_component_values,&component_monomial_info,
					&monomial_derivative_values))
				{
					return_code=1;
				}
			}
			if (return_code)
			{
				FE_value *xi_array;
				int *xi_derivative_orders;

				xi_array=new FE_value[number_of_xi];
				xi_derivative_orders=new int[number_of_xi];
				if (xi_array&&xi_derivative_orders)
				{
					bool carry;
					FE_value **derivative_component_values=(FE_value **)NULL,
						*value_address_1,*value_address_2;
					Variable_size_type column_number,i;
					boost::numeric::ublas::vector<Variable_size_type>
						derivative_independent_values(derivative_order+1);
					int j;
					Scalar component_value;

					for (i=0;i<=derivative_order;i++)
					{
						derivative_independent_values[i]=0;
					}
					for (i=0;i<number_of_xi;i++)
					{
						xi_array[i]=(FE_value)(xi[i]);
					}
					/* chose the component values to use for calculating the
						values */
					if (nodal_values_input)
					{
						derivative_component_values=nodal_value_component_values[0];
					}
					else
					{
						derivative_component_values=component_values;
					}
					/* loop over the values within the derivative block */
					column_number=0;
					while (return_code&&
						(0==derivative_independent_values[derivative_order]))
					{
						if (derivative_component_values)
						{
							for (i=0;i<number_of_xi;i++)
							{
								xi_derivative_orders[i]=0;
							}
							for (i=0;i<derivative_order;i++)
							{
								if (element_xi_inputs[i])
								{
									// element/xi derivative
									if (0<element_xi_inputs[i]->indices.size())
									{
										xi_derivative_orders[(element_xi_inputs[i]->indices)[
											derivative_independent_values[i]]-1]++;
									}
									else
									{
										xi_derivative_orders[
											derivative_independent_values[i]]++;
									}
								}
							}
							/* loop over components */
							i=0;
							while (return_code&&(i<number_of_components))
							{
								if (return_code=calculate_monomial_derivative_values(
									number_of_xi,component_monomial_info[i]+1,
									xi_derivative_orders,xi_array,monomial_derivative_values))
								{
									/* calculate the derivative */
									component_value=(Scalar)0;
									value_address_1=monomial_derivative_values;
									value_address_2=derivative_component_values[i];
									for (j=numbers_of_component_values[i];j>0;j--)
									{
										component_value +=
											(double)(*value_address_1)*
											(double)(*value_address_2);
										value_address_1++;
										value_address_2++;
									}
									matrix(i,column_number)=component_value;
								}
								i++;
							}
						}
						/* step to next value within derivative block */
						column_number++;
						i=0;
						carry=true;
						do
						{
							derivative_independent_values[i]++;
							if (element_xi_inputs[i])
							{
								Variable_size_type
									number_of_values=(element_xi_inputs[i]->indices).size();

								if (0<number_of_values)
								{
									if (derivative_independent_values[i]>=number_of_values)
									{
										derivative_independent_values[i]=0;
									}
									else
									{
										carry=false;
									}
								}
								else
								{
									if (derivative_independent_values[i]>=number_of_xi)
									{
										derivative_independent_values[i]=0;
									}
									else
									{
										carry=false;
									}
								}
							}
							else
							{
								if (derivative_independent_values[i]>=
									(Variable_size_type)number_of_nodal_values)
								{
									derivative_independent_values[i]=0;
								}
								else
								{
									carry=false;
								}
								// update the component values to use for calculating the values
								derivative_component_values=
									nodal_value_component_values[
									derivative_independent_values[i]];
							}
							i++;
						} while ((i<derivative_order)&&carry);
						if (carry)
						{
							derivative_independent_values[derivative_order]++;
						}
					}
				}
				delete [] xi_array;
				delete [] xi_derivative_orders;
			}
		}
		// remove temporary storage
		if (element_field_values)
		{
			DESTROY(FE_element_field_values)(&element_field_values);
		}
	}
}

enum Swap_nodal_values_type
{
	SWAP_NODAL_VALUES_GET,
	SWAP_NODAL_VALUES_SET,
	SWAP_NODAL_VALUES_SWAP
}; /* enum Swap_nodal_values_type */

struct Swap_nodal_values_data
{
	enum FE_nodal_value_type value_type;
	enum Swap_nodal_values_type swap_type;
	FE_value *values;
	int component_number,number_of_components,number_of_values,value_number,
		version;
	struct FE_field *fe_field;
}; /* struct Swap_nodal_values_data */

static int swap_nodal_values(struct FE_node *node,
	void *swap_nodal_values_data_void)
/*******************************************************************************
LAST MODIFIED : 30 April 2003

DESCRIPTION :
Swaps the nodal values specified by <swap_nodal_values_data_void> at the <node>.
==============================================================================*/
{
	enum FE_nodal_value_type *nodal_value_type,*nodal_value_types,value_type;
	enum Swap_nodal_values_type swap_type;
	FE_value swap_value,*swap_value_1,*swap_value_2,*value,*values;
	int all_components,component_number,i,j,number_of_swap_values,
		number_of_values,number_of_values_per_version,number_of_versions,
		return_code,version;
	struct FE_field *fe_field;
	struct Swap_nodal_values_data *swap_nodal_values_data;

	ENTER(swap_nodal_values);
	return_code=0;
	/* check arguments */
	if (node&&(swap_nodal_values_data=(struct Swap_nodal_values_data *)
		swap_nodal_values_data_void))
	{
		fe_field=swap_nodal_values_data->fe_field;
		swap_type=swap_nodal_values_data->swap_type;
		value_type=swap_nodal_values_data->value_type;
		if (get_FE_nodal_field_FE_value_values(fe_field,node,&number_of_values,
			&values))
		{
			value=values;
			return_code=1;
			component_number=0;
			all_components=(swap_nodal_values_data->component_number<0)||
				(swap_nodal_values_data->component_number>=
				swap_nodal_values_data->number_of_components);
			while (return_code&&(component_number<
				swap_nodal_values_data->number_of_components))
			{
				number_of_values_per_version=
					1+get_FE_node_field_component_number_of_derivatives(node,fe_field,
					component_number);
				number_of_versions=get_FE_node_field_component_number_of_versions(node,
					fe_field,component_number);
				if (all_components||
					(component_number==swap_nodal_values_data->component_number))
				{
					swap_value_1=(swap_nodal_values_data->values)+
						(swap_nodal_values_data->value_number);
					if (FE_NODAL_UNKNOWN==value_type)
					{
						if ((0<=(version=swap_nodal_values_data->version))&&(version<
							number_of_versions))
						{
							swap_value_2=value+(version*number_of_values_per_version);
							number_of_swap_values=number_of_values_per_version;
						}
						else
						{
							swap_value_2=value;
							number_of_swap_values=
								number_of_versions*number_of_values_per_version;
						}
						switch (swap_type)
						{
							case SWAP_NODAL_VALUES_GET:
							{
								memcpy(swap_value_1,swap_value_2,
									number_of_swap_values*sizeof(FE_value));
							} break;
							case SWAP_NODAL_VALUES_SET:
							{
								memcpy(swap_value_2,swap_value_1,
									number_of_swap_values*sizeof(FE_value));
							} break;
							case SWAP_NODAL_VALUES_SWAP:
							{
								for (i=number_of_swap_values;i>0;i--)
								{
									swap_value= *swap_value_1;
									*swap_value_1= *swap_value_2;
									*swap_value_2=swap_value;
									swap_value_1++;
									swap_value_2++;
								}
							} break;
						}
						swap_nodal_values_data->value_number += number_of_swap_values;
					}
					else
					{
						if (nodal_value_types=get_FE_node_field_component_nodal_value_types(
							node,fe_field,component_number))
						{
							nodal_value_type=nodal_value_types;
							i=0;
							while ((i<number_of_values_per_version)&&
								(value_type!= *nodal_value_type))
							{
								nodal_value_type++;
								i++;
							}
							if (i<number_of_values_per_version)
							{
								if ((0<=(version=swap_nodal_values_data->version))&&(version<
									number_of_versions))
								{
									i += version*number_of_values_per_version;
									swap_value_1=(swap_nodal_values_data->values)+
										(swap_nodal_values_data->value_number);
									swap_value_2=value+i;
									switch (swap_type)
									{
										case SWAP_NODAL_VALUES_GET:
										{
											*swap_value_1= *swap_value_2;
										} break;
										case SWAP_NODAL_VALUES_SET:
										{
											*swap_value_2= *swap_value_1;
										} break;
										case SWAP_NODAL_VALUES_SWAP:
										{
											swap_value= *swap_value_1;
											*swap_value_1= *swap_value_2;
											*swap_value_2=swap_value;
										} break;
									}
									(swap_nodal_values_data->value_number)++;
								}
								else
								{
									swap_value_1=(swap_nodal_values_data->values)+
										(swap_nodal_values_data->value_number);
									swap_value_2=value+i;
									switch (swap_type)
									{
										case SWAP_NODAL_VALUES_GET:
										{
											for (j=number_of_versions;j>0;j--)
											{
												*swap_value_1= *swap_value_2;
												swap_value_1++;
												swap_value_2 += number_of_values_per_version;
											}
										} break;
										case SWAP_NODAL_VALUES_SET:
										{
											for (j=number_of_versions;j>0;j--)
											{
												*swap_value_2= *swap_value_1;
												swap_value_1++;
												swap_value_2 += number_of_values_per_version;
											}
										} break;
										case SWAP_NODAL_VALUES_SWAP:
										{
											for (j=number_of_versions;j>0;j--)
											{
												swap_value= *swap_value_1;
												*swap_value_1= *swap_value_2;
												*swap_value_2=swap_value;
												swap_value_1++;
												swap_value_2 += number_of_values_per_version;
											}
										} break;
									}
									swap_nodal_values_data->value_number += number_of_versions;
								}
							}
							DEALLOCATE(nodal_value_types);
						}
						else
						{
							return_code=0;
						}
					}
				}
				value += number_of_values_per_version*number_of_versions;
				component_number++;
			}
			if (return_code&&((SWAP_NODAL_VALUES_SET==swap_type)||
				(SWAP_NODAL_VALUES_SWAP==swap_type)))
			{
				return_code=set_FE_nodal_field_FE_value_values(fe_field,node,values,
					&number_of_values);
			}
			DEALLOCATE(values);
		}
	}
	LEAVE;

	return (return_code);
} /* swap_nodal_values */

Variable_handle Variable_finite_element::get_input_value_local(
	const Variable_input_handle& input)
//******************************************************************************
// LAST MODIFIED : 13 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	Variable_handle result;
	Variable_input_element_xi_handle input_element_xi_handle;
	Variable_input_nodal_values_handle input_nodal_values_handle;

	result=Variable_vector_handle((Variable_vector *)0);
	if ((input_element_xi_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_element_xi,Variable_input>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_element_xi *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_element_xi_handle->variable_finite_element==
		Variable_finite_element_handle(this)))
	{
		if (input_element_xi_handle->element)
		{
			// no indices if have element
			result=Variable_handle(new Variable_element_xi(element,xi));
		}
		else
		{
			Variable_size_type
				number_of_values=(input_element_xi_handle->indices).size();

			if (0<number_of_values)
			{
				Variable_size_type i,index,number_of_xi=xi.size();
				Vector values_vector(number_of_values);

				for (i=0;i<number_of_values;i++)
				{
					index=(input_element_xi_handle->indices)[i];
					if (index<number_of_xi)
					{
						values_vector[i]=xi[index];
					}
					else
					{
						values_vector[i]=(Scalar)0;
					}
				}
				result=Variable_vector_handle(new Variable_vector(values_vector));
			}
			else
			{
				result=Variable_vector_handle(new Variable_vector(xi));
			}
		}
	}
	else if ((input_nodal_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_nodal_values,Variable_input>(
		input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_nodal_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_nodal_values_handle->variable_finite_element==
		Variable_finite_element_handle(this)))
	{
		int return_code;
		Variable_size_type number_of_values=input_nodal_values_handle->size();
		struct FE_region *region;
		struct Swap_nodal_values_data swap_nodal_values_data;

		if (field&&(region=FE_field_get_FE_region(field))&&(0<number_of_values))
		{
			swap_nodal_values_data.swap_type=SWAP_NODAL_VALUES_GET;
			swap_nodal_values_data.number_of_values=0;
			swap_nodal_values_data.value_type=input_nodal_values_handle->value_type;
			swap_nodal_values_data.version=input_nodal_values_handle->version;
			swap_nodal_values_data.fe_field=field;
			swap_nodal_values_data.component_number=component_number;
			swap_nodal_values_data.number_of_components=
				get_FE_field_number_of_components(field);
			swap_nodal_values_data.value_number=0;
			if (ALLOCATE(swap_nodal_values_data.values,FE_value,number_of_values))
			{
				if (input_nodal_values_handle->node)
				{
					return_code=swap_nodal_values(input_nodal_values_handle->node,
						(void *)&swap_nodal_values_data);
				}
				else
				{
					return_code=FE_region_for_each_FE_node(region,swap_nodal_values,
						(void *)&swap_nodal_values_data);
				}
				if (return_code)
				{
					Variable_vector_handle values_vector;

					values_vector=Variable_vector_handle(new Variable_vector(Vector(
						number_of_values)));
					if (values_vector)
					{
						Variable_size_type i;

						for (i=0;i<number_of_values;i++)
						{
							(*values_vector)[i]=(swap_nodal_values_data.values)[i];
						}
						result=values_vector;
					}
				}
				DEALLOCATE(swap_nodal_values_data.values);
			}
		}
	}

	return (result);
}

int Variable_finite_element::set_input_value_local(
	const Variable_input_handle& input,const Variable_handle& values)
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	int return_code;
	Variable_input_element_xi_handle input_element_xi_handle;
	Variable_input_nodal_values_handle input_nodal_values_handle;

	return_code=0;
	if ((input_element_xi_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_element_xi,Variable_input>(input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_element_xi *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_element_xi_handle->variable_finite_element==
		Variable_finite_element_handle(this)))
	{
		if (input_element_xi_handle->element)
		{
			// no indices if have element
			Variable_element_xi_handle values_element_xi;

			if (values_element_xi=
#if defined (USE_SMART_POINTER)
				boost::dynamic_pointer_cast<Variable_element_xi,Variable>(values)
#else /* defined (USE_SMART_POINTER) */
				dynamic_cast<Variable_element_xi *>(values)
#endif /* defined (USE_SMART_POINTER) */
				)
			{
				if (input_element_xi_handle->xi)
				{
					xi.resize((values_element_xi->xi).size());
					xi=values_element_xi->xi;
					return_code=1;
				}
				else
				{
					return_code=1;
				}
				if (return_code)
				{
					if (element)
					{
						DEACCESS(FE_element)(&element);
					}
					if (values_element_xi->element)
					{
						element=ACCESS(FE_element)(values_element_xi->element);
					}
				}
			}
		}
		else
		{
			Vector *values_vector;

			if (values_vector=values->scalars())
			{
				Variable_size_type
					number_of_values=input_element_xi_handle->indices.size(),
					number_of_xi=xi.size();

				if (0<number_of_values)
				{
					Variable_size_type i,index;

					for (i=0;i<number_of_values;i++)
					{
						index=input_element_xi_handle->indices[i];
						if (index<number_of_xi)
						{
							xi[index]=(*values_vector)[i];
						}
					}
					return_code=1;
				}
				else
				{
					if (number_of_xi==values_vector->size())
					{
						xi=(*values_vector);
						return_code=1;
					}
				}
				delete values_vector;
			}
		}
	}
	else if ((input_nodal_values_handle=
#if defined (USE_SMART_POINTER)
		boost::dynamic_pointer_cast<Variable_input_nodal_values,Variable_input>(
		input)
#else /* defined (USE_SMART_POINTER) */
		dynamic_cast<Variable_input_nodal_values *>(input)
#endif /* defined (USE_SMART_POINTER) */
		)&&(input_nodal_values_handle->variable_finite_element==
		Variable_finite_element_handle(this)))
	{
		Variable_size_type i,number_of_values=input_nodal_values_handle->size();
		struct FE_region *region;
		struct Swap_nodal_values_data swap_nodal_values_data;
		Vector *values_vector;

		if (field&&(region=FE_field_get_FE_region(field))&&(0<number_of_values)&&
			(number_of_values==values->size())&&(values_vector=values->scalars()))
		{
			swap_nodal_values_data.swap_type=SWAP_NODAL_VALUES_SET;
			swap_nodal_values_data.number_of_values=0;
			swap_nodal_values_data.value_type=input_nodal_values_handle->value_type;
			swap_nodal_values_data.version=input_nodal_values_handle->version;
			swap_nodal_values_data.fe_field=field;
			swap_nodal_values_data.component_number=component_number;
			swap_nodal_values_data.number_of_components=
				get_FE_field_number_of_components(field);
			swap_nodal_values_data.value_number=0;
			if (ALLOCATE(swap_nodal_values_data.values,FE_value,number_of_values))
			{
				for (i=0;i<number_of_values;i++)
				{
					(swap_nodal_values_data.values)[i]=(FE_value)((*values_vector)[i]);
				}
				if (input_nodal_values_handle->node)
				{
					return_code=swap_nodal_values(input_nodal_values_handle->node,
						(void *)&swap_nodal_values_data);
				}
				else
				{
					return_code=FE_region_for_each_FE_node(region,swap_nodal_values,
						(void *)&swap_nodal_values_data);
				}
				DEALLOCATE(swap_nodal_values_data.values);
			}
			delete values_vector;
		}
	}

	return (return_code);
}

string_handle Variable_finite_element::get_string_representation_local()
//******************************************************************************
// LAST MODIFIED : 10 November 2003
//
// DESCRIPTION :
//==============================================================================
{
	string_handle return_string;
	std::ostringstream out;

	if (return_string=new std::string)
	{
		if (field)
		{
			out << get_FE_field_name(field);
		}
		if (-1!=component_number)
		{
			out << "(" << component_number << ")";
		}
		*return_string=out.str();
	}

	return (return_string);
}
