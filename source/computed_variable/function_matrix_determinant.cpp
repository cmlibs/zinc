//******************************************************************************
// FILE : function_matrix_determinant.cpp
//
// LAST MODIFIED : 23 May 2005
//
// DESCRIPTION :
//==============================================================================

#if defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_determinant_implementation.cpp"
#else // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)
#include "computed_variable/function_matrix_determinant.hpp"
#endif // defined (ONE_TEMPLATE_DEFINITION_IMPLEMENTED)

#if !defined (AIX)
#include "computed_variable/function_derivative_matrix.hpp"

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#include "computed_variable/function_derivative.hpp"
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (OLD_CODE)
template<>
Function_handle Function_variable_matrix_determinant<Scalar>::
	evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_determinant<Scalar> >
		function_matrix_determinant;
	Function_handle result(0);
	Function_size_type order;

	if ((function_matrix_determinant=boost::dynamic_pointer_cast<
		Function_matrix_determinant<Scalar>,Function>(function()))&&
		(0<(order=independent_variables.size())))
	{
		Function_size_type number_of_dependent_values,number_of_rows;
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative,matrix;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_derivatnew_handle temp_function;
		Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

		if (
#if defined (EVALUATE_RETURNS_VALUE)
			(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_determinant->matrix_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
			(function_matrix_determinant->matrix_private->evaluate)()&&
			(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_determinant->matrix_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
			(matrix->number_of_columns()==(number_of_rows=matrix->number_of_rows()))&&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(function_matrix_determinant->matrix_private->
			evaluate_derivative(independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(temp_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_determinant->matrix_private->derivative(
			independent_variables)))&&(temp_variable=temp_function->output())&&
			(temp_variable->evaluate())&&(temp_variable=temp_function->matrix(
			independent_variables))&&(derivative=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>(temp_variable->get_value()))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(number_of_rows*number_of_rows==
			(number_of_dependent_values=derivative->number_of_rows()))&&
			(0<derivative->number_of_columns()))
		{
			bool valid;
			Function_size_type i,j,k,l,number_of_columns;
			std::vector<Function_size_type> entries(2*order);
			std::vector<Matrix> Df(order);

			number_of_columns=number_of_dependent_values;
			i=0;
			valid=true;
			while (valid&&(i<order))
			{
				Matrix& Df_i=Df[i];

				Df_i.resize(1,number_of_columns);
				if (i<number_of_rows)
				{
					bool zero_derivative;

					for (j=0;j<=2*i+1;++j)
					{
						entries[j]=1;
					}
					j=0;
					while (valid&&(j<number_of_columns))
					{
						// check for repeated rows/columns in derivative
						zero_derivative=false;
						k=0;
						while (!zero_derivative&&(k<=i))
						{
							l=0;
							while (!zero_derivative&&(l<k))
							{
								zero_derivative=((entries[2*l]==entries[2*k])||
									(entries[2*l+1]==entries[2*k+1]));
								++l;
							}
							++k;
						}
						if (zero_derivative)
						{
							Df_i(0,j)=0;
						}
						else
						{
							// doing a copy
							boost::intrusive_ptr< Function_matrix<Scalar> >
								matrix_temp(boost::dynamic_pointer_cast<Function_matrix<Scalar>,
								Function>((matrix->output())->get_value()));
							Function_size_type column,row;

							if (matrix_temp)
							{
								for (k=0;k<=i;++k)
								{
									row=entries[2*k];
									column=entries[2*k+1];
									for (l=1;l<=number_of_rows;++l)
									{
										(*matrix_temp)(row,l)=0;
									}
									for (l=1;l<=number_of_rows;++l)
									{
										(*matrix_temp)(l,column)=0;
									}
									(*matrix_temp)(row,column)=1;
								}
								valid=matrix_temp->determinant(Df_i(0,j));
							}
							else
							{
								valid=false;
							}
						}
						// move to next
						k=2*i+1;
						++entries[k];
						while ((0!=k)&&(number_of_rows<entries[k]))
						{
							entries[k]=1;
							--k;
							++entries[k];
						}
						if ((0==k)&&(number_of_rows<entries[k]))
						{
							entries[k]=1;
						}
						++j;
					}
				}
				else
				{
					Df_i.clear();
				}
				number_of_columns *= number_of_dependent_values;
				++i;
			}
			if (valid)
			{
				Function_derivative_matrix_handle derivative_f(0),derivative_g(0);
				std::list<Function_variable_handle> intermediate_independent_variables(
					order,function_matrix_determinant->matrix_private);
				std::list<Matrix> matrices(0);
				std::list<Matrix>::iterator matrix_iterator,matrix_iterator_end;

				for (i=0;i<order;++i)
				{
					matrices.push_back(Df[0]);
					matrix_iterator_end=matrices.end();
					--matrix_iterator_end;
					matrix_iterator=matrices.begin();
					number_of_columns=number_of_dependent_values;
					j=1;
					while (matrix_iterator!=matrix_iterator_end)
					{
						if (number_of_columns!=(k=matrix_iterator->size2()))
						{
							if (number_of_dependent_values==k)
							{
								j=1;
							}
							else
							{
								if (number_of_columns*number_of_dependent_values==k)
								{
									++j;
									number_of_columns=k;
								}
							}
						}
						matrices.push_back(Df[j]);
						++matrix_iterator;
					}
				}
				try
				{
					derivative_f=new Function_derivative_matrix(this,
						intermediate_independent_variables,matrices);
					derivative_g=new Function_derivative_matrix(
						function_matrix_determinant->matrix_private,independent_variables);
					if (derivative_f&&derivative_g)
					{
						Function_derivative_matrix_handle derivative_matrix=
							Function_derivative_matrix_compose(this,derivative_f,
							derivative_g);

						if (derivative_matrix)
						{
							result=derivative_matrix->matrix(independent_variables);
						}
					}
				}
				catch (Function_derivative_matrix::Construction_exception)
				{
					// do nothing
					//???debug
					std::cout << "Function_variable_matrix_determinant<Scalar>::evaluate_derivative.  Failed" << std::endl;
				}
			}
		}
	}

	return (result);
}
#endif // defined (OLD_CODE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
// class Function_derivatnew_matrix_determinant
// --------------------------------------------

class Function_derivatnew_matrix_determinant : public Function_derivatnew
//******************************************************************************
// LAST MODIFIED : 21 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	public:
		// for construction exception
		class Construction_exception {};
		// constructor
		Function_derivatnew_matrix_determinant(
			const Function_variable_handle& dependent_variable,
			const std::list<Function_variable_handle>& independent_variables);
		// destructor
		~Function_derivatnew_matrix_determinant();
	// inherited
	private:
#if defined (EVALUATE_RETURNS_VALUE)
		virtual Function_handle evaluate(Function_variable_handle atomic_variable);
#else // defined (EVALUATE_RETURNS_VALUE)
		virtual bool evaluate(Function_variable_handle atomic_variable);
#endif // defined (EVALUATE_RETURNS_VALUE)
	private:
		// copy operations are private and undefined to prevent copying
		void operator=(const Function&);
	private:
		Function_derivatnew_handle matrix_derivative;
};

Function_derivatnew_matrix_determinant::Function_derivatnew_matrix_determinant(
	const Function_variable_handle& dependent_variable,
	const std::list<Function_variable_handle>& independent_variables):
	Function_derivatnew(dependent_variable,independent_variables)
//******************************************************************************
// LAST MODIFIED : 21 April 2005
//
// DESCRIPTION :
// Constructor.
//==============================================================================
{
	boost::intrusive_ptr< Function_matrix_determinant<Scalar> >
		function_matrix_determinant;
	boost::intrusive_ptr< Function_variable_matrix_determinant<Scalar> >
		variable_matrix_determinant;

	if ((variable_matrix_determinant=boost::dynamic_pointer_cast<
		Function_variable_matrix_determinant<Scalar>,Function_variable>(
		dependent_variable))&&(function_matrix_determinant=
		boost::dynamic_pointer_cast<Function_matrix_determinant<Scalar>,Function>(
		dependent_variable->function())))
	{
		if (matrix_derivative=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(function_matrix_determinant->matrix_private->derivative(
			independent_variables)))
		{
			matrix_derivative->add_dependent_function(this);
		}
		else
		{
			throw Function_derivatnew_matrix_determinant::Construction_exception();
		}
	}
	else
	{
		throw Function_derivatnew_matrix_determinant::Construction_exception();
	}
}

Function_derivatnew_matrix_determinant::
	~Function_derivatnew_matrix_determinant()
//******************************************************************************
// LAST MODIFIED : 21 April 2005
//
// DESCRIPTION :
// Destructor.
//==============================================================================
{
#if defined (CIRCULAR_SMART_POINTERS)
	// do nothing
#else // defined (CIRCULAR_SMART_POINTERS)
	if (matrix_derivative)
	{
		matrix_derivative->remove_dependent_function(this);
	}
#endif // defined (CIRCULAR_SMART_POINTERS)
}

template<>
Function_handle Function_variable_matrix_determinant<Scalar>::derivative(
	const std::list<Function_variable_handle>& independent_variables)
//******************************************************************************
// LAST MODIFIED : 20 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	return (Function_handle(new Function_derivatnew_matrix_determinant(
		Function_variable_handle(this),independent_variables)));
}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
template<>
Function_handle Function_variable_matrix_determinant<Scalar>::
	evaluate_derivative(
	std::list<Function_variable_handle>& independent_variables)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (EVALUATE_RETURNS_VALUE)
Function_handle
#else // defined (EVALUATE_RETURNS_VALUE)
bool
#endif // defined (EVALUATE_RETURNS_VALUE)
	Function_derivatnew_matrix_determinant::evaluate(
	Function_variable_handle
#if defined (EVALUATE_RETURNS_VALUE)
	atomic_variable
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
	)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
//******************************************************************************
// LAST MODIFIED : 23 May 2005
//
// DESCRIPTION :
//==============================================================================
{
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE) || defined (EVALUATE_RETURNS_VALUE)
	Function_handle result(0);
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE) || defined (EVALUATE_RETURNS_VALUE)
	bool result(true);
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE) || defined (EVALUATE_RETURNS_VALUE)

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	if (!evaluated())
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
	{
		boost::intrusive_ptr< Function_matrix_determinant<Scalar> >
			function_matrix_determinant;
		Function_size_type order;

#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		result=false;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			boost::dynamic_pointer_cast<Function_variable_matrix_determinant<Scalar>,
			Function_variable>(dependent_variable)&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(function_matrix_determinant=boost::dynamic_pointer_cast<
			Function_matrix_determinant<Scalar>,Function>(
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			dependent_variable->
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			function()))&&(0<(order=independent_variables.size())))
		{
			boost::intrusive_ptr< Function_matrix<Scalar> > derivative,matrix;
			Function_size_type number_of_dependent_values,number_of_rows;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			Function_variable_handle temp_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

			if (
#if defined (EVALUATE_RETURNS_VALUE)
				(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_determinant->matrix_private->evaluate()))&&
#else // defined (EVALUATE_RETURNS_VALUE)
				(function_matrix_determinant->matrix_private->evaluate)()&&
				(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_determinant->matrix_private->get_value()))&&
#endif // defined (EVALUATE_RETURNS_VALUE)
				(matrix->number_of_columns()==
				(number_of_rows=matrix->number_of_rows()))&&
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(derivative=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(function_matrix_determinant->matrix_private->
				evaluate_derivative(independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(temp_variable=matrix_derivative->output())&&
				(temp_variable->evaluate())&&
				(temp_variable=matrix_derivative->matrix(independent_variables))&&
				(derivative=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
				Function>(temp_variable->get_value()))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				(number_of_rows*number_of_rows==
				(number_of_dependent_values=derivative->number_of_rows()))&&
				(0<derivative->number_of_columns()))
			{
				bool valid;
				Function_size_type i,j,k,l,number_of_columns;
				std::vector<Function_size_type> entries(2*order);
				std::vector<Matrix> Df(order);

				number_of_columns=number_of_dependent_values;
				i=0;
				valid=true;
				while (valid&&(i<order))
				{
					Matrix& Df_i=Df[i];

					Df_i.resize(1,number_of_columns);
					if (i<number_of_rows)
					{
						bool zero_derivative;

						for (j=0;j<=2*i+1;++j)
						{
							entries[j]=1;
						}
						j=0;
						while (valid&&(j<number_of_columns))
						{
							// check for repeated rows/columns in derivative
							zero_derivative=false;
							k=0;
							while (!zero_derivative&&(k<=i))
							{
								l=0;
								while (!zero_derivative&&(l<k))
								{
									zero_derivative=((entries[2*l]==entries[2*k])||
										(entries[2*l+1]==entries[2*k+1]));
									++l;
								}
								++k;
							}
							if (zero_derivative)
							{
								Df_i(0,j)=0;
							}
							else
							{
								// doing a copy
								boost::intrusive_ptr< Function_matrix<Scalar> > matrix_temp(
									boost::dynamic_pointer_cast<Function_matrix<Scalar>,
									Function>((matrix->output())->get_value()));
								Function_size_type column,row;

								if (matrix_temp)
								{
									for (k=0;k<=i;++k)
									{
										row=entries[2*k];
										column=entries[2*k+1];
										for (l=1;l<=number_of_rows;++l)
										{
											(*matrix_temp)(row,l)=0;
										}
										for (l=1;l<=number_of_rows;++l)
										{
											(*matrix_temp)(l,column)=0;
										}
										(*matrix_temp)(row,column)=1;
									}
									valid=matrix_temp->determinant(Df_i(0,j));
								}
								else
								{
									valid=false;
								}
							}
							// move to next
							k=2*i+1;
							++entries[k];
							while ((0!=k)&&(number_of_rows<entries[k]))
							{
								entries[k]=1;
								--k;
								++entries[k];
							}
							if ((0==k)&&(number_of_rows<entries[k]))
							{
								entries[k]=1;
							}
							++j;
						}
					}
					else
					{
						Df_i.clear();
					}
					number_of_columns *= number_of_dependent_values;
					++i;
				}
				if (valid)
				{
					std::list<Function_variable_handle>
						intermediate_independent_variables(order,
						function_matrix_determinant->matrix_private);
					std::list<Matrix> matrices(0);
					std::list<Matrix>::iterator matrix_iterator,matrix_iterator_end;

					for (i=0;i<order;++i)
					{
						matrices.push_back(Df[0]);
						matrix_iterator_end=matrices.end();
						--matrix_iterator_end;
						matrix_iterator=matrices.begin();
						number_of_columns=number_of_dependent_values;
						j=1;
						while (matrix_iterator!=matrix_iterator_end)
						{
							if (number_of_columns!=(k=matrix_iterator->size2()))
							{
								if (number_of_dependent_values==k)
								{
									j=1;
								}
								else
								{
									if (number_of_columns*number_of_dependent_values==k)
									{
										++j;
										number_of_columns=k;
									}
								}
							}
							matrices.push_back(Df[j]);
							++matrix_iterator;
						}
					}
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					try
					{
						Function_derivative_matrix_handle derivative_f=
							new Function_derivative_matrix(this,
							intermediate_independent_variables,matrices),
							derivative_g=new Function_derivative_matrix(
							function_matrix_determinant->matrix_private,
							independent_variables);

						if (derivative_f&&derivative_g)
						{
							Function_derivative_matrix_handle derivative_matrix=
								Function_derivative_matrix_compose(this,derivative_f,
								derivative_g);

							if (derivative_matrix)
							{
								result=derivative_matrix->matrix(independent_variables);
							}
						}
					}
					catch (Function_derivative_matrix::Construction_exception)
					{
						// do nothing
						//???debug
						std::cout << "Function_variable_matrix_determinant<Scalar>::evaluate_derivative.  Failed" << std::endl;
					}
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
					try
					{
						Derivative_matrix temp_derivative_matrix(matrices);

						derivative_matrix=
							temp_derivative_matrix*(matrix_derivative->derivative_matrix);
						set_evaluated();
#if defined (EVALUATE_RETURNS_VALUE)
#else // defined (EVALUATE_RETURNS_VALUE)
						result=true;
#endif // defined (EVALUATE_RETURNS_VALUE)
					}
					catch (Derivative_matrix::Construction_exception)
					{
						// do nothing
						//???debug
						std::cout << "Function_derivatnew_matrix_determinant>::evaluate.  Failed" << std::endl;
					}
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
				}
			}
		}
	}
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#if defined (EVALUATE_RETURNS_VALUE)
	if (evaluated())
	{
		result=get_value(atomic_variable);
	}
#else // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (EVALUATE_RETURNS_VALUE)
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

	return (result);
}

template<>
bool Function_matrix_determinant<Scalar>::evaluate_derivative(
	Scalar& derivative,Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 20 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_determinant<Scalar> >
		atomic_variable_matrix_determinant;

	result=false;
	if ((atomic_variable_matrix_determinant=boost::dynamic_pointer_cast<
		Function_variable_matrix_determinant<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_determinant->function())&&
		(1==atomic_variable_matrix_determinant->row())&&
		(1==atomic_variable_matrix_determinant->column())&&
		(0<atomic_independent_variables.size()))
	{
		boost::intrusive_ptr< Function_matrix<Scalar> > derivative_value;
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
		Function_derivatnew_handle derivative_function;
		Function_variable_handle derivative_variable;
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)

		if (
#if defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_value=boost::dynamic_pointer_cast<
			Function_matrix<Scalar>,Function>(atomic_variable_matrix_determinant->
			evaluate_derivative(atomic_independent_variables)))&&
#else // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(derivative_function=boost::dynamic_pointer_cast<Function_derivatnew,
			Function>(atomic_variable_matrix_determinant->derivative(
			atomic_independent_variables)))&&(derivative_variable=
			derivative_function->output())&&(derivative_variable->evaluate())&&
			(derivative_variable=derivative_function->matrix(
			atomic_independent_variables))&&
			(derivative_value=boost::dynamic_pointer_cast<Function_matrix<Scalar>,
			Function>(derivative_variable->get_value()))&&
#endif // defined (USE_FUNCTION_VARIABLE__EVALUATE_DERIVATIVE)
			(1==derivative_value->number_of_rows())&&
			(1==derivative_value->number_of_columns()))
		{
			derivative=(*derivative_value)(1,1);
			result=true;
		}
	}

	return (result);
}

#if defined (OLD_CODE)
template<>
bool Function_matrix_determinant<Scalar>::evaluate_derivative(
	Scalar& derivative,Function_variable_handle atomic_variable,
	std::list<Function_variable_handle>& atomic_independent_variables)
//******************************************************************************
// LAST MODIFIED : 7 April 2005
//
// DESCRIPTION :
//==============================================================================
{
	bool result;
	boost::intrusive_ptr< Function_variable_matrix_determinant<Scalar> >
		atomic_variable_matrix_determinant;
	Function_derivative_matrix_handle derivative_g(0);
	Function_handle matrix_current(0);
	boost::intrusive_ptr< Function_matrix<Scalar> > matrix(0);
	Function_size_type number_of_rows,order;

	result=false;
	if ((atomic_variable_matrix_determinant=boost::dynamic_pointer_cast<
		Function_variable_matrix_determinant<Scalar>,Function_variable>(
		atomic_variable))&&equivalent(Function_handle(this),
		atomic_variable_matrix_determinant->function())&&
		(1==atomic_variable_matrix_determinant->row())&&
		(1==atomic_variable_matrix_determinant->column())&&
		(0<(order=atomic_independent_variables.size()))&&matrix_private&&
		(matrix_current=matrix_private->get_value())&&
		(matrix=boost::dynamic_pointer_cast<Function_matrix<Scalar>,Function>(
		matrix_private->evaluate()))&&
		(0<(number_of_rows=matrix->number_of_rows()))&&
		(derivative_g=new Function_derivative_matrix(matrix_private,
		atomic_independent_variables)))
	{
		bool valid;
		Function_size_type i,j,k,l,number_of_columns,size;
		std::vector<Function_size_type> entries(2*order);
		std::vector<Matrix> Df(order);

		size=number_of_rows*number_of_rows;
		number_of_columns=size;
		i=0;
		valid=true;
		while (valid&&(i<order))
		{
			Matrix& Df_i=Df[i];

			Df_i.resize(1,number_of_columns);
			if (i<number_of_rows)
			{
				bool zero_derivative;

				for (j=0;j<=2*i+1;++j)
				{
					entries[j]=1;
				}
				j=0;
				while (valid&&(j<number_of_columns))
				{
					// check for repeated rows/columns in derivative
					zero_derivative=false;
					k=0;
					while (!zero_derivative&&(k<=i))
					{
						l=0;
						while (!zero_derivative&&(l<k))
						{
							zero_derivative=((entries[2*l]==entries[2*k])||
								(entries[2*l+1]==entries[2*k+1]));
							++l;
						}
						++k;
					}
					if (zero_derivative)
					{
						Df_i(0,j)=0;
					}
					else
					{
						// doing a copy
						boost::intrusive_ptr< Function_matrix<Scalar> >
							matrix_temp(boost::dynamic_pointer_cast<Function_matrix<Scalar>,
							Function>((matrix->output())->get_value()));
						Function_size_type column,row;

						if (matrix_temp)
						{
							for (k=0;k<=i;++k)
							{
								row=entries[2*k];
								column=entries[2*k+1];
								for (l=1;l<=number_of_rows;++l)
								{
									(*matrix_temp)(row,l)=0;
								}
								for (l=1;l<=number_of_rows;++l)
								{
									(*matrix_temp)(l,column)=0;
								}
								(*matrix_temp)(row,column)=1;
							}
							valid=matrix_temp->determinant(Df_i(0,j));
						}
						else
						{
							valid=false;
						}
					}
					// move to next
					k=2*i+1;
					++entries[k];
					while ((0!=k)&&(number_of_rows<entries[k]))
					{
						entries[k]=1;
						--k;
						++entries[k];
					}
					if ((0==k)&&(number_of_rows<entries[k]))
					{
						entries[k]=1;
					}
					++j;
				}
			}
			else
			{
				Df_i.clear();
			}
			number_of_columns *= size;
			++i;
		}
		if (valid)
		{
			Function_derivative_matrix_handle derivative_f(0);
			std::list<Function_variable_handle> intermediate_independent_variables(
				order,matrix_private);
			std::list<Matrix> matrices(0);
			std::list<Matrix>::iterator matrix_iterator,matrix_iterator_end;

			for (i=0;i<order;++i)
			{
				matrices.push_back(Df[0]);
				matrix_iterator_end=matrices.end();
				--matrix_iterator_end;
				matrix_iterator=matrices.begin();
				number_of_columns=size;
				j=1;
				while (matrix_iterator!=matrix_iterator_end)
				{
					if (number_of_columns!=(k=matrix_iterator->size2()))
					{
						if (size==k)
						{
							j=1;
						}
						else
						{
							if (number_of_columns*size==k)
							{
								++j;
								number_of_columns=k;
							}
						}
					}
					matrices.push_back(Df[j]);
					++matrix_iterator;
				}
			}
			derivative_f=new Function_derivative_matrix(atomic_variable,
				intermediate_independent_variables,matrices);
			if (derivative_f)
			{
				Function_derivative_matrix_handle derivative_matrix=
					Function_derivative_matrix_compose(atomic_variable,derivative_f,
					derivative_g);
				boost::intrusive_ptr< Function_matrix<Scalar> > function_matrix;

				if (derivative_matrix&&(function_matrix=boost::dynamic_pointer_cast<
					Function_matrix<Scalar>,Function>(derivative_matrix->matrix(
					atomic_independent_variables))))
				{
					derivative=(*function_matrix)(1,1);
					result=true;
				}
			}
		}
	}
	if (matrix_current)
	{
		matrix_private->rset_value(matrix_current);
	}

	return (result);
}
#endif // defined (OLD_CODE)

#endif // !defined (AIX)
