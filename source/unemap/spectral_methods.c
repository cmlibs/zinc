/*******************************************************************************
FILE : spectral_methods.c

LAST MODIFIED : 13 October 1999

DESCRIPTION :
Functions for analysing signals using spectral methods.
==============================================================================*/

#include <stddef.h>
#include <math.h>
#include "general/debug.h"
#include "general/geometry.h"
#include "unemap/rig.h"
#include "unemap/rig_node.h"
#include "unemap/spectral_methods.h"
#include "user_interface/message.h"

/*
Module macros
-------------
*/
#define CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location) \
/* calculate the bit-reversed location */ \
single_bit=number_of_transform_points; \
while ((single_bit>=2)&&(single_bit&bit_reversed_location)) \
{ \
	bit_reversed_location -= single_bit; \
	single_bit /= 2; \
} \
bit_reversed_location += single_bit

#define CALCULATE_BIT_REVERSED_LOCATION_MINUS(bit_reversed_location) \
/* calculate the bit-reversed location */ \
single_bit=number_of_transform_points; \
while ((single_bit>=2)&&!(single_bit&bit_reversed_location)) \
{ \
	bit_reversed_location += single_bit; \
	single_bit /= 2; \
} \
bit_reversed_location -= single_bit

/*
Global functions
----------------
*/
int fourier_transform(enum Fourier_window_type window,
	struct Device *signal_real,struct Device *signal_imaginary,
	struct Device *transform_real,struct Device *transform_imaginary)
/*******************************************************************************
LAST MODIFIED : 13 August 1999

DESCRIPTION :
Calculates the Fourier transform of a complex signal.  If <signal_imaginary> is
NULL, only the non-negative half of the transform is calculated (the Fourier
transform of a real signal is even).  The real and imaginary parts of the
transform need to be stored in the same buffer and they need to be the only
signals in the buffer.  Extra memory is allocated for the transform buffer if
required.
???DB.  Data windowing.
==============================================================================*/
{
	double temp_double,theta,w_k_imaginary_double,w_k_real_double,
		w_minus_1_imaginary,w_minus_1_real;
	float *even_half,*even_half_base,even_half_imaginary,even_half_real,
		h_1_imaginary,h_1_real,h_2_imaginary,h_2_real,mean_imaginary,mean_real,
		*odd_half,odd_half_imaginary,odd_half_real,*signal_imaginary_value,
		*signal_imaginary_values,*signal_real_value,*signal_real_values,
		temp_float_imaginary,temp_float_real,*times,*transform_buffer_values,
		*transform_buffer_value,transform_frequency,window_amplitude,
		w_k_imaginary_float,w_k_real_float;
	int bit_reversed_location,i,j,k,number_of_data_points,
		number_of_data_points_imaginary,number_of_sub_transforms,
		number_of_transform_points,return_code,single_bit,*transform_buffer_time,
		two_i,two_i_plus_1;
	struct Signal_buffer *transform_buffer;

	ENTER(fourier_transform);
	return_code=0;
	times=(float *)NULL;
	signal_real_values=(float *)NULL;
	signal_imaginary_values=(float *)NULL;
	/* check the arguments */
	if (signal_real&&extract_signal_information((struct FE_node *)NULL,
		(struct Signal_drawing_package *)NULL,signal_real,1,1,0,(int *)NULL,
		&number_of_data_points,&times,&signal_real_values,
		(enum Event_signal_status **)NULL,(char **)NULL,(int *)NULL,(float *)NULL,
		(float *)NULL)&&(0<number_of_data_points)&&
		(!signal_imaginary||(signal_imaginary&&extract_signal_information(
		(struct FE_node *)NULL,(struct Signal_drawing_package *)NULL,signal_real,1,1,0,
		(int *)NULL,&number_of_data_points_imaginary,(float **)NULL,
		&signal_imaginary_values,(enum Event_signal_status **)NULL,(char **)NULL,
		(int *)NULL,(float *)NULL,(float *)NULL)&&(number_of_data_points==
		number_of_data_points_imaginary)))&&
		transform_real&&(transform_real->signal)&&
		(transform_buffer=transform_real->signal->buffer)&&
		transform_imaginary&&(transform_imaginary->signal)&&
		(transform_buffer==transform_imaginary->signal->buffer)&&
		(transform_real->signal->index!=transform_imaginary->signal->index)&&
		(2==transform_buffer->number_of_signals))
	{
		/* assume that times are equally spaced and that real times are the same as
			imaginary times */
		/* choose the number of transform points to be the smallest power of 2
			greater than the number of data points */
		number_of_transform_points=1;
		while (number_of_transform_points<number_of_data_points)
		{
			number_of_transform_points *= 2;
		}
		transform_frequency=((float)number_of_transform_points)*
			(times[number_of_data_points-1]-times[0])/((float)number_of_data_points);
		if (!signal_imaginary)
		{
			/* for a real signal only calculate the non-negative half of the
				transform */
			number_of_transform_points /= 2;
		}
		/* check storage for the transform */
		/* allocate/modify storage for the transform */
		if ((number_of_transform_points==transform_buffer->number_of_samples)&&
			(FLOAT_VALUE==transform_buffer->value_type))
		{
			transform_buffer_values=transform_buffer->signals.float_values;
			return_code=1;
		}
		else
		{
			if (number_of_transform_points!=transform_buffer->number_of_samples)
			{
				ALLOCATE(transform_buffer_time,int,number_of_transform_points);
			}
			else
			{
				transform_buffer_time=transform_buffer->times;
			}
			if (transform_buffer_time)
			{
				if (ALLOCATE(transform_buffer_values,float,
					2*number_of_transform_points))
				{
					if (transform_buffer_time!=transform_buffer->times)
					{
						DEALLOCATE(transform_buffer->times);
						transform_buffer->times=transform_buffer_time;
						/* assign times */
						i=number_of_transform_points;
						transform_buffer_time += i;
						while (i>0)
						{
							transform_buffer_time--;
							i--;
							*transform_buffer_time=i;
						}
					}
					if (FLOAT_VALUE!=transform_buffer->value_type)
					{
						DEALLOCATE(transform_buffer->signals.short_int_values);
					}
					else
					{
						DEALLOCATE(transform_buffer->signals.float_values);
					}
					transform_buffer->signals.float_values=transform_buffer_values;
					transform_buffer->number_of_signals=2;
					transform_buffer->number_of_samples=number_of_transform_points;
					transform_buffer->value_type=FLOAT_VALUE;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"fourier_transform.  No memory for values");
					if (transform_buffer_time!=transform_buffer->times)
					{
						DEALLOCATE(transform_buffer_time);
					}
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"fourier_transform.  No memory for times");
				return_code=0;
			}
		}
		if (return_code)
		{
			transform_real->signal->index=0;
			transform_real->signal_minimum=1;
			transform_real->signal_maximum=0;
			transform_real->channel->offset=0.;
			transform_imaginary->signal->index=1;
			transform_imaginary->signal_minimum=1;
			transform_imaginary->signal_maximum=0;
			transform_imaginary->channel->offset=0.;
			transform_buffer->frequency=transform_frequency;
			transform_buffer->start=0;
			transform_buffer->end=number_of_transform_points-1;
			/* calculate the Fourier transform of the complex function whose real
				parts are the even values of the real function and whose imaginary parts
				are the odd values of the real function */
			/* perform the bit-reversal ordering with padding */
			bit_reversed_location=0;
			if (signal_imaginary)
			{
				transform_real->channel->gain=1.;
				transform_imaginary->channel->gain=1.;
				i=number_of_data_points;
				mean_real=0.;
				mean_imaginary=0.;
				signal_real_value=signal_real_values;
				signal_imaginary_value=signal_imaginary_values;
				for (i=number_of_data_points;i>0;i--)
				{
					mean_real += *signal_real_value;
					signal_real_value++;
					mean_imaginary += *signal_imaginary_value;
					signal_imaginary_value++;
				}
				mean_real /= (float)number_of_data_points;
				mean_imaginary /= (float)number_of_data_points;
				signal_real_value=signal_real_values;
				signal_imaginary_value=signal_imaginary_values;
				i=number_of_data_points;
				switch (window)
				{
					case HAMMING_WINDOW:
					{
						temp_float_real=2.*PI/(float)(number_of_data_points+1);
						while (i>0)
						{
							window_amplitude=(1.-cos((float)(i)*temp_float_real))/2.;
							/* perform ordering */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=window_amplitude*
								((*signal_real_value)-mean_real);
							signal_real_value++;
							transform_buffer_value++;
							*transform_buffer_value=window_amplitude*
								((*signal_imaginary_value)-mean_imaginary);
							signal_imaginary_value++;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							i--;
						}
					} break;
					case SQUARE_WINDOW:
					{
						while (i>0)
						{
							/* perform ordering */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=((*signal_real_value)-mean_real);
							signal_real_value++;
							transform_buffer_value++;
							*transform_buffer_value=
								((*signal_imaginary_value)-mean_imaginary);
							signal_imaginary_value++;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							i--;
						}
					} break;
					case PARZEN_WINDOW:
					{
						temp_float_real=(float)(number_of_data_points+1)/2.;
						while (i>0)
						{
							window_amplitude=(float)(i)/temp_float_real;
							if (window_amplitude>1.)
							{
								window_amplitude=2.-window_amplitude;
							}
							/* perform ordering */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=window_amplitude*
								((*signal_real_value)-mean_real);
							signal_real_value++;
							transform_buffer_value++;
							*transform_buffer_value=window_amplitude*
								((*signal_imaginary_value)-mean_imaginary);
							signal_imaginary_value++;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							i--;
						}
					} break;
					case WELCH_WINDOW:
					{
						temp_float_real=(float)(number_of_data_points+1)/2.;
						while (i>0)
						{
							window_amplitude=(temp_float_real-(float)i)/temp_float_real;
							window_amplitude *= -window_amplitude;
							window_amplitude += 1.;
							/* perform ordering */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=window_amplitude*
								((*signal_real_value)-mean_real);
							signal_real_value++;
							transform_buffer_value++;
							*transform_buffer_value=window_amplitude*
								((*signal_imaginary_value)-mean_imaginary);
							signal_imaginary_value++;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							i--;
						}
					} break;
				}
				j=number_of_transform_points-number_of_data_points;
			}
			else
			{
				transform_real->channel->gain=1;
				transform_imaginary->channel->gain=1;
				j=number_of_transform_points;
				signal_real_value=signal_real_values;
				mean_real=0.;
				for (i=number_of_data_points;i>0;i--)
				{
					mean_real += *signal_real_value;
					signal_real_value++;
				}
				mean_real /= (float)number_of_data_points;
				signal_real_value=signal_real_values;
				i=number_of_data_points;
				switch (window)
				{
					case HAMMING_WINDOW:
					{
						temp_float_real=2.*PI/(float)(number_of_data_points-1);
						while (i>1)
						{
							/* perform ordering */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=((*signal_real_value)-mean_real)*
								(1.-cos((float)(i)*temp_float_real))/2.;
							signal_real_value++;
							i--;
							transform_buffer_value++;
							*transform_buffer_value=((*signal_real_value)-mean_real)*
								(1.-cos((float)(i)*temp_float_real))/2.;
							signal_real_value++;
							i--;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							j--;
						}
						if (1==i)
						{
							/* odd number of data values */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=0;
							transform_buffer_value++;
							*transform_buffer_value=0;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							j--;
						}
					} break;
					case SQUARE_WINDOW:
					{
						while (i>1)
						{
							/* perform ordering */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=(*signal_real_value)-mean_real;
							signal_real_value++;
							transform_buffer_value++;
							*transform_buffer_value=(*signal_real_value)-mean_real;
							signal_real_value++;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							i -= 2;
							j--;
						}
						if (1==i)
						{
							/* odd number of data values */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=(*signal_real_value)-mean_real;
							transform_buffer_value++;
							*transform_buffer_value=0;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							j--;
						}
					} break;
					case PARZEN_WINDOW:
					{
						temp_float_real=(float)(number_of_data_points+1)/2.;
						while (i>1)
						{
							/* perform ordering */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							window_amplitude=(float)(i)/temp_float_real;
							if (window_amplitude>1.)
							{
								window_amplitude=2.-window_amplitude;
							}
							*transform_buffer_value=window_amplitude*
								((*signal_real_value)-mean_real);
							signal_real_value++;
							transform_buffer_value++;
							i--;
							window_amplitude=(float)(i)/temp_float_real;
							if (window_amplitude>1.)
							{
								window_amplitude=2.-window_amplitude;
							}
							*transform_buffer_value=window_amplitude*
								((*signal_real_value)-mean_real);
							signal_real_value++;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							i--;
							j--;
						}
						if (1==i)
						{
							/* odd number of data values */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=((*signal_real_value)-mean_real)/
								temp_float_real;
							transform_buffer_value++;
							*transform_buffer_value=0;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							j--;
						}
					} break;
					case WELCH_WINDOW:
					{
						temp_float_real=(float)(number_of_data_points+1)/2.;
						while (i>1)
						{
							/* perform ordering */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							window_amplitude=
								(temp_float_real-(float)(i))/temp_float_real;
							window_amplitude *= -window_amplitude;
							window_amplitude += 1.;
							*transform_buffer_value=window_amplitude*
								((*signal_real_value)-mean_real);
							signal_real_value++;
							transform_buffer_value++;
							i--;
							window_amplitude=
								(temp_float_real-(float)(i))/temp_float_real;
							window_amplitude *= -window_amplitude;
							window_amplitude += 1.;
							*transform_buffer_value=window_amplitude*
								((*signal_real_value)-mean_real);
							signal_real_value++;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							i--;
							j--;
						}
						if (1==i)
						{
							/* odd number of data values */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							window_amplitude=(float)(number_of_data_points)
								/(temp_float_real*temp_float_real);
							*transform_buffer_value=window_amplitude*
								((*signal_real_value)-mean_real);
							transform_buffer_value++;
							*transform_buffer_value=0;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							j--;
						}
					} break;
				}
			}
			while (j>0)
			{
				/* perform ordering */
				transform_buffer_value=transform_buffer_values+bit_reversed_location;
				*transform_buffer_value=0;
				transform_buffer_value++;
				*transform_buffer_value=0;
				CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
				j--;
			}
			/* perform the transformation */
			two_i=1;
			two_i_plus_1=2;
			number_of_sub_transforms=number_of_transform_points;
			while (two_i<number_of_transform_points)
			{
				number_of_sub_transforms /= 2;
				/* calculate w-1 */
				theta=(double)PI/(double)two_i;
				w_minus_1_real=sin(0.5*theta);
				w_minus_1_real *= -2*w_minus_1_real;
				w_minus_1_imaginary=sin(theta);
				/* w^0 */
				w_k_real_double=1;
				w_k_imaginary_double=0;
				/* loop over data */
				even_half_base=transform_buffer_values;
				for (k=two_i;k>0;k--)
				{
					even_half=even_half_base;
					odd_half=even_half_base+two_i_plus_1;
					for (j=number_of_sub_transforms;j>0;j--)
					{
						/* use Danielson-Lanczos lemma */
						odd_half_real= *odd_half;
						odd_half_imaginary=odd_half[1];
						even_half_real= *even_half;
						even_half_imaginary=even_half[1];
						/* (w^k)*odd_half */
						w_k_real_float=(float)w_k_real_double;
						w_k_imaginary_float=(float)w_k_imaginary_double;
						temp_float_real=w_k_real_float*odd_half_real-
							w_k_imaginary_float*odd_half_imaginary;
						temp_float_imaginary=w_k_real_float*odd_half_imaginary+
							w_k_imaginary_float*odd_half_real;
						/* odd_half = even_half - (w^k)*odd_half */
						*odd_half=even_half_real-temp_float_real;
						odd_half[1]=even_half_imaginary-temp_float_imaginary;
						/* even_half = even_half + (w^k)*odd_half */
						*even_half=even_half_real+temp_float_real;
						even_half[1]=even_half_imaginary+temp_float_imaginary;
						even_half=odd_half+two_i_plus_1;
						odd_half=even_half+two_i_plus_1;
					}
					/* w^(k+1)=(w^k)*w=(w^k)+(w^k)*(w-1) */
					temp_double=w_k_real_double+w_k_real_double*w_minus_1_real-
						w_k_imaginary_double*w_minus_1_imaginary;
					w_k_imaginary_double=w_k_imaginary_double+
						w_k_imaginary_double*w_minus_1_real+
						w_k_real_double*w_minus_1_imaginary;
					w_k_real_double=temp_double;
					even_half_base += 2;
				}
				two_i=two_i_plus_1;
				two_i_plus_1 *= 2;
			}
			if (signal_imaginary)
			{
				/* allow for the signal offsets */
				transform_buffer_values[0] += 2*mean_real;
				transform_buffer_values[1] += 2*mean_imaginary;
				transform_buffer_values[0] /= 2.;
				transform_buffer_values[1] /= 2.;
			}
			else
			{
				/* calculate the transformation of the original real function */
				/* calculate w-1 */
				theta=(double)PI/(double)number_of_transform_points;
				w_minus_1_real=sin(0.5*theta);
				w_minus_1_real *= -2*w_minus_1_real;
				w_minus_1_imaginary=sin(theta);
				/* w^0 */
				w_k_real_double=1;
				w_k_imaginary_double=0;
				/* loop over data */
				even_half=transform_buffer_values;
				odd_half=transform_buffer_values+2*number_of_transform_points;
				/* calculate first complex value separately */
				temp_float_real= *even_half;
				temp_float_imaginary=even_half[1];
				*even_half=temp_float_real+temp_float_imaginary;
				even_half++;
				*even_half=temp_float_real-temp_float_imaginary;
				even_half++;
				for (i=number_of_transform_points/2-1;i>0;i--)
				{
					/* w^(k+1)=(w^k)*w=(w^k)+(w^k)*(w-1) */
					temp_double=w_k_real_double+w_k_real_double*w_minus_1_real-
						w_k_imaginary_double*w_minus_1_imaginary;
					w_k_imaginary_double=w_k_imaginary_double+
						w_k_imaginary_double*w_minus_1_real+
						w_k_real_double*w_minus_1_imaginary;
					w_k_real_double=temp_double;
					even_half_real= *even_half;
					even_half_imaginary=even_half[1];
					odd_half--;
					odd_half_imaginary= *odd_half;
					odd_half--;
					odd_half_real= *odd_half;
					/* h_1=(even_half+conj(odd_half))/2 */
					h_1_real=(even_half_real+odd_half_real)/2;
					h_1_imaginary=(even_half_imaginary-odd_half_imaginary)/2;
					/* h_2= -i*(even_half-conj(odd_half))/2 */
					h_2_real=(even_half_imaginary+odd_half_imaginary)/2;
					h_2_imaginary=(odd_half_real-even_half_real)/2;
					/* (w^k)*h_2 */
					w_k_real_float=(float)w_k_real_double;
					w_k_imaginary_float=(float)w_k_imaginary_double;
					temp_float_real=w_k_real_float*h_2_real-
						w_k_imaginary_float*h_2_imaginary;
					temp_float_imaginary=w_k_real_float*h_2_imaginary+
						w_k_imaginary_float*h_2_real;
					/* even_half=h_1+(w^k)*h_2 */
					*even_half=h_1_real+temp_float_real;
					even_half++;
					*even_half=h_1_imaginary+temp_float_imaginary;
					even_half++;
					/* odd_half=conj(h_1-(w^k)*h_2) */
					*odd_half=h_1_real-temp_float_real;
					odd_half[1]=temp_float_imaginary-h_1_imaginary;
				}
				/* allow for the signal offset */
				transform_buffer_values[0] +=
					2.*mean_real*(float)(number_of_transform_points);
				transform_buffer_values[0] /= 2.;
				transform_buffer_values[1] /= 2.;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"fourier_transform.  Transform buffer too small");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"fourier_transform.  Invalid argument(s)");
		return_code=0;
	}
	DEALLOCATE(times);
	DEALLOCATE(signal_real_values);
	DEALLOCATE(signal_imaginary_values);
	LEAVE;

	return (return_code);
} /* fourier_transform */
#if defined (NEW_CODE)
#endif /* defined (NEW_CODE) */

#if defined (OLD_CODE)
int fourier_transform(enum Fourier_window_type window,
	struct Device *signal_real,struct Device *signal_imaginary,
	struct Device *transform_real,struct Device *transform_imaginary)
/*******************************************************************************
LAST MODIFIED : 13 August 1999

DESCRIPTION :
Calculates the Fourier transform of a complex signal.  If <signal_imaginary> is
NULL, only the non-negative half of the transform is calculated (the Fourier
transform of a real signal is even).  The real and imaginary parts of the
transform need to be stored in the same buffer and they need to be the only
signals in the buffer.  Extra memory is allocated for the transform buffer if
required.
???DB.  Data windowing.
==============================================================================*/
{
	double temp_double,theta,w_k_imaginary_double,w_k_real_double,
		w_minus_1_imaginary,w_minus_1_real;
	float *even_half,*even_half_base,even_half_imaginary,even_half_real,
		gain_imaginary,gain_real,h_1_imaginary,h_1_real,h_2_imaginary,h_2_real,
		mean_imaginary,mean_real,*odd_half,odd_half_imaginary,odd_half_real,
		*signal_imaginary_float,*signal_real_float,temp_float_imaginary,
		temp_float_real,*transform_buffer_values,*transform_buffer_value,
		transform_frequency,window_amplitude,w_k_imaginary_float,w_k_real_float;
	int bit_reversed_location,buffer_offset,i,j,k,number_of_data_points,
		number_of_sub_transforms,number_of_transform_points,return_code,single_bit,
		*transform_buffer_time,two_i,two_i_plus_1;
	short int *signal_imaginary_short_int,*signal_real_short_int;
	struct Signal_buffer *signal_buffer,*transform_buffer;

	ENTER(fourier_transform);
	/* check the arguments */
	if (signal_real&&(signal_real->signal)&&
		(signal_buffer=signal_real->signal->buffer)&&
		(!signal_imaginary||(signal_imaginary&&(signal_imaginary->signal)&&
		(signal_buffer==signal_imaginary->signal->buffer)))&&
		transform_real&&(transform_real->signal)&&
		(transform_buffer=transform_real->signal->buffer)&&
		transform_imaginary&&(transform_imaginary->signal)&&
		(transform_buffer==transform_imaginary->signal->buffer)&&
		(transform_real->signal->index!=transform_imaginary->signal->index)&&
		(2==transform_buffer->number_of_signals))
	{
		number_of_data_points=(signal_buffer->end)-(signal_buffer->start)+1;
		/* choose the number of transform points to be the smallest power of 2
			greater than the number of data points */
		number_of_transform_points=1;
		while (number_of_transform_points<number_of_data_points)
		{
			number_of_transform_points *= 2;
		}
		transform_frequency=((float)number_of_transform_points)/
			(signal_buffer->frequency);
		if (!signal_imaginary)
		{
			/* for a real signal only calculate the non-negative half of the
				transform */
			number_of_transform_points /= 2;
		}
		/* check storage for the transform */
		/* allocate/modify storage for the transform */
		if ((number_of_transform_points==transform_buffer->number_of_samples)&&
			(FLOAT_VALUE==transform_buffer->value_type))
		{
			transform_buffer_values=transform_buffer->signals.float_values;
			return_code=1;
		}
		else
		{
			if (number_of_transform_points!=transform_buffer->number_of_samples)
			{
				ALLOCATE(transform_buffer_time,int,number_of_transform_points);
			}
			else
			{
				transform_buffer_time=transform_buffer->times;
			}
			if (transform_buffer_time)
			{
				if (ALLOCATE(transform_buffer_values,float,
					2*number_of_transform_points))
				{
					if (transform_buffer_time!=transform_buffer->times)
					{
						DEALLOCATE(transform_buffer->times);
						transform_buffer->times=transform_buffer_time;
						/* assign times */
						i=number_of_transform_points;
						transform_buffer_time += i;
						while (i>0)
						{
							transform_buffer_time--;
							i--;
							*transform_buffer_time=i;
						}
					}
					if (FLOAT_VALUE!=transform_buffer->value_type)
					{
						DEALLOCATE(transform_buffer->signals.short_int_values);
					}
					else
					{
						DEALLOCATE(transform_buffer->signals.float_values);
					}
					transform_buffer->signals.float_values=transform_buffer_values;
					transform_buffer->number_of_signals=2;
					transform_buffer->number_of_samples=number_of_transform_points;
					transform_buffer->value_type=FLOAT_VALUE;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"fourier_transform.  No memory for values");
					if (transform_buffer_time!=transform_buffer->times)
					{
						DEALLOCATE(transform_buffer_time);
					}
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"fourier_transform.  No memory for times");
				return_code=0;
			}
		}
		if (return_code)
		{
			transform_real->signal->index=0;
			transform_real->signal_minimum=1;
			transform_real->signal_maximum=0;
			transform_real->channel->offset=0.;
			transform_imaginary->signal->index=1;
			transform_imaginary->signal_minimum=1;
			transform_imaginary->signal_maximum=0;
			transform_imaginary->channel->offset=0.;
			transform_buffer->frequency=transform_frequency;
			transform_buffer->start=0;
			transform_buffer->end=number_of_transform_points-1;
			/* calculate the Fourier transform of the complex function whose real
				parts are the even values of the real function and whose imaginary parts
				are the odd values of the real function */
			/* perform the bit-reversal ordering with padding */
			bit_reversed_location=0;
			buffer_offset=signal_buffer->number_of_signals;
			gain_real=(signal_real->channel->gain)/(float)number_of_transform_points;
			if (signal_imaginary)
			{
				transform_real->channel->gain=1.;
				transform_imaginary->channel->gain=1.;
				gain_imaginary=(signal_imaginary->channel->gain)/
					(float)number_of_transform_points;
				i=number_of_data_points;
				switch (signal_buffer->value_type)
				{
					case SHORT_INT_VALUE:
					{
						signal_real_short_int=(signal_buffer->signals.short_int_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_real->signal->index));
						signal_imaginary_short_int=
							(signal_buffer->signals.short_int_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_imaginary->signal->index));
						mean_real=0.;
						mean_imaginary=0.;
						for (i=number_of_data_points;i>0;i--)
						{
							mean_real += (float)(*signal_real_short_int);
							signal_real_short_int += buffer_offset;
							mean_imaginary += (float)(*signal_imaginary_short_int);
							signal_imaginary_short_int += buffer_offset;
						}
						mean_real /= (float)number_of_data_points;
						mean_imaginary /= (float)number_of_data_points;
						signal_real_short_int=(signal_buffer->signals.short_int_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_real->signal->index));
						signal_imaginary_short_int=
							(signal_buffer->signals.short_int_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_imaginary->signal->index));
						i=number_of_data_points;
						switch (window)
						{
							case HAMMING_WINDOW:
							{
								temp_float_real=2.*PI/(float)(number_of_data_points+1);
								while (i>0)
								{
									window_amplitude=(1.-cos((float)(i)*temp_float_real))/2.;
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=window_amplitude*gain_real*
										((float)(*signal_real_short_int)-mean_real);
									signal_real_short_int += buffer_offset;
									transform_buffer_value++;
									*transform_buffer_value=window_amplitude*gain_imaginary*
										((float)(*signal_imaginary_short_int)-mean_imaginary);
									signal_imaginary_short_int += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i--;
								}
							} break;
							case SQUARE_WINDOW:
							{
								while (i>0)
								{
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=gain_real*
										((float)(*signal_real_short_int)-mean_real);
									signal_real_short_int += buffer_offset;
									transform_buffer_value++;
									*transform_buffer_value=gain_imaginary*
										((float)(*signal_imaginary_short_int)-mean_imaginary);
									signal_imaginary_short_int += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i--;
								}
							} break;
							case PARZEN_WINDOW:
							{
								temp_float_real=(float)(number_of_data_points+1)/2.;
								while (i>0)
								{
									window_amplitude=(float)(i)/temp_float_real;
									if (window_amplitude>1.)
									{
										window_amplitude=2.-window_amplitude;
									}
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=window_amplitude*gain_real*
										((float)(*signal_real_short_int)-mean_real);
									signal_real_short_int += buffer_offset;
									transform_buffer_value++;
									*transform_buffer_value=window_amplitude*gain_imaginary*
										((float)(*signal_imaginary_short_int)-mean_imaginary);
									signal_imaginary_short_int += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i--;
								}
							} break;
							case WELCH_WINDOW:
							{
								temp_float_real=(float)(number_of_data_points+1)/2.;
								while (i>0)
								{
									window_amplitude=(temp_float_real-(float)i)/temp_float_real;
									window_amplitude *= -window_amplitude;
									window_amplitude += 1.;
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=window_amplitude*gain_real*
										((float)(*signal_real_short_int)-mean_real);
									signal_real_short_int += buffer_offset;
									transform_buffer_value++;
									*transform_buffer_value=window_amplitude*gain_imaginary*
										((float)(*signal_imaginary_short_int)-mean_imaginary);
									signal_imaginary_short_int += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i--;
								}
							} break;
						}
						j=number_of_transform_points-number_of_data_points;
					} break;
					case FLOAT_VALUE:
					{
						signal_real_float=(signal_buffer->signals.float_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_real->signal->index));
						signal_imaginary_float=(signal_buffer->signals.float_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_imaginary->signal->index));
						mean_real=0.;
						mean_imaginary=0.;
						for (i=number_of_data_points;i>0;i--)
						{
							mean_real += *signal_real_float;
							signal_real_float += buffer_offset;
							mean_imaginary += *signal_imaginary_float;
							signal_imaginary_float += buffer_offset;
						}
						mean_real /= (float)number_of_data_points;
						mean_imaginary /= (float)number_of_data_points;
						signal_real_float=(signal_buffer->signals.float_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_real->signal->index));
						signal_imaginary_float=(signal_buffer->signals.float_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_imaginary->signal->index));
						i=number_of_data_points;
						switch (window)
						{
							case HAMMING_WINDOW:
							{
								temp_float_real=2.*PI/(float)(number_of_data_points+1);
								while (i>0)
								{
									window_amplitude=(1.-cos((float)(i)*temp_float_real))/2.;
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=window_amplitude*gain_real*
										((*signal_real_float)-mean_real);
									signal_real_float += buffer_offset;
									transform_buffer_value++;
									*transform_buffer_value=window_amplitude*gain_imaginary*
										((*signal_imaginary_float)-mean_imaginary);
									signal_imaginary_float += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i--;
								}
							} break;
							case SQUARE_WINDOW:
							{
								while (i>0)
								{
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=gain_real*
										((*signal_real_float)-mean_real);
									signal_real_float += buffer_offset;
									transform_buffer_value++;
									*transform_buffer_value=gain_imaginary*
										((*signal_imaginary_float)-mean_imaginary);
									signal_imaginary_float += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i--;
								}
							} break;
							case PARZEN_WINDOW:
							{
								temp_float_real=(float)(number_of_data_points+1)/2.;
								while (i>0)
								{
									window_amplitude=(float)(i)/temp_float_real;
									if (window_amplitude>1.)
									{
										window_amplitude=2.-window_amplitude;
									}
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=window_amplitude*gain_real*
										((*signal_real_float)-mean_real);
									signal_real_float += buffer_offset;
									transform_buffer_value++;
									*transform_buffer_value=window_amplitude*gain_imaginary*
										((*signal_imaginary_float)-mean_imaginary);
									signal_imaginary_float += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i--;
								}
							} break;
							case WELCH_WINDOW:
							{
								temp_float_real=(float)(number_of_data_points+1)/2.;
								while (i>0)
								{
									window_amplitude=(temp_float_real-(float)i)/temp_float_real;
									window_amplitude *= -window_amplitude;
									window_amplitude += 1.;
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=window_amplitude*gain_real*
										((*signal_real_float)-mean_real);
									signal_real_float += buffer_offset;
									transform_buffer_value++;
									*transform_buffer_value=window_amplitude*gain_imaginary*
										((*signal_imaginary_float)-mean_imaginary);
									signal_imaginary_float += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i--;
								}
							} break;
						}
						j=number_of_transform_points-number_of_data_points;
					} break;
				}
			}
			else
			{
/*				gain_real /= 2;*/
				transform_real->channel->gain=gain_real;
				transform_imaginary->channel->gain=gain_real;
				j=number_of_transform_points;
				switch (signal_buffer->value_type)
				{
					case SHORT_INT_VALUE:
					{
						signal_real_short_int=(signal_buffer->signals.short_int_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_real->signal->index));
						mean_real=0.;
						for (i=number_of_data_points;i>0;i--)
						{
							mean_real += (float)(*signal_real_short_int);
							signal_real_short_int += buffer_offset;
						}
						mean_real /= (float)number_of_data_points;
						signal_real_short_int=(signal_buffer->signals.short_int_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_real->signal->index));
						i=number_of_data_points;
						switch (window)
						{
							case HAMMING_WINDOW:
							{
								temp_float_real=2.*PI/(float)(number_of_data_points+1);
								while (i>1)
								{
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									window_amplitude=(1.-cos((float)(i)*temp_float_real))/2.;
									*transform_buffer_value=window_amplitude*
										((float)(*signal_real_short_int)-mean_real);
									signal_real_short_int += buffer_offset;
									i--;
									transform_buffer_value++;
									window_amplitude=(1.-cos((float)(i)*temp_float_real))/2.;
									*transform_buffer_value=window_amplitude*
										((float)(*signal_real_short_int)-mean_real);
									signal_real_short_int += buffer_offset;
									i--;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									j--;
								}
								if (1==i)
								{
									/* odd number of data values */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=0;
									transform_buffer_value++;
									*transform_buffer_value=0;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									j--;
								}
							} break;
							case SQUARE_WINDOW:
							{
								while (i>1)
								{
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=(float)(*signal_real_short_int)-
										mean_real;
									signal_real_short_int += buffer_offset;
									transform_buffer_value++;
									*transform_buffer_value=(float)(*signal_real_short_int)-
										mean_real;
									signal_real_short_int += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i -= 2;
									j--;
								}
								if (1==i)
								{
									/* odd number of data values */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=
										((float)(*signal_real_short_int)-mean_real);
									transform_buffer_value++;
									*transform_buffer_value=0;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									j--;
								}
							} break;
							case PARZEN_WINDOW:
							{
								temp_float_real=(float)(number_of_data_points+1)/2.;
								while (i>1)
								{
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									window_amplitude=(float)(i)/temp_float_real;
									if (window_amplitude>1.)
									{
										window_amplitude=2.-window_amplitude;
									}
									*transform_buffer_value=window_amplitude*
										((float)(*signal_real_short_int)-mean_real);
									signal_real_short_int += buffer_offset;
									transform_buffer_value++;
									i--;
									window_amplitude=(float)(i)/temp_float_real;
									if (window_amplitude>1.)
									{
										window_amplitude=2.-window_amplitude;
									}
									*transform_buffer_value=window_amplitude*
										((float)(*signal_real_short_int)-mean_real);
									signal_real_short_int += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i--;
									j--;
								}
								if (1==i)
								{
									/* odd number of data values */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=((float)(*signal_real_short_int)-
										mean_real)/temp_float_real;
									transform_buffer_value++;
									*transform_buffer_value=0;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									j--;
								}
							} break;
							case WELCH_WINDOW:
							{
								temp_float_real=(float)(number_of_data_points+1)/2.;
								while (i>1)
								{
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									window_amplitude=
										(temp_float_real-(float)(i))/temp_float_real;
									window_amplitude *= -window_amplitude;
									window_amplitude += 1.;
									*transform_buffer_value=window_amplitude*
										((float)(*signal_real_short_int)-mean_real);
									signal_real_short_int += buffer_offset;
									transform_buffer_value++;
									i--;
									window_amplitude=(temp_float_real-(float)(i))/temp_float_real;
									window_amplitude *= -window_amplitude;
									window_amplitude += 1.;
									*transform_buffer_value=window_amplitude*
										((float)(*signal_real_short_int)-mean_real);
									signal_real_short_int += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i--;
									j--;
								}
								if (1==i)
								{
									/* odd number of data values */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									window_amplitude=(float)(number_of_data_points)
										/(temp_float_real*temp_float_real);
									*transform_buffer_value=window_amplitude*
										((float)(*signal_real_short_int)-mean_real);
									transform_buffer_value++;
									*transform_buffer_value=0;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									j--;
								}
							} break;
						}
					} break;
					case FLOAT_VALUE:
					{
						signal_real_float=(signal_buffer->signals.float_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_real->signal->index));
						mean_real=0.;
						for (i=number_of_data_points;i>0;i--)
						{
							mean_real += *signal_real_float;
							signal_real_float += buffer_offset;
						}
						mean_real /= (float)number_of_data_points;
						signal_real_float=(signal_buffer->signals.float_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_real->signal->index));
						i=number_of_data_points;
						switch (window)
						{
							case HAMMING_WINDOW:
							{
								temp_float_real=2.*PI/(float)(number_of_data_points-1);
								while (i>1)
								{
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=((*signal_real_float)-mean_real)*
										(1.-cos((float)(i)*temp_float_real))/2.;
									signal_real_float += buffer_offset;
									i--;
									transform_buffer_value++;
									*transform_buffer_value=((*signal_real_float)-mean_real)*
										(1.-cos((float)(i)*temp_float_real))/2.;
									signal_real_float += buffer_offset;
									i--;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									j--;
								}
								if (1==i)
								{
									/* odd number of data values */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=0;
									transform_buffer_value++;
									*transform_buffer_value=0;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									j--;
								}
							} break;
							case SQUARE_WINDOW:
							{
								while (i>1)
								{
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=(*signal_real_float)-mean_real;
									signal_real_float += buffer_offset;
									transform_buffer_value++;
									*transform_buffer_value=(*signal_real_float)-mean_real;
									signal_real_float += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i -= 2;
									j--;
								}
								if (1==i)
								{
									/* odd number of data values */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=(*signal_real_float)-mean_real;
									transform_buffer_value++;
									*transform_buffer_value=0;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									j--;
								}
							} break;
							case PARZEN_WINDOW:
							{
								temp_float_real=(float)(number_of_data_points+1)/2.;
								while (i>1)
								{
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									window_amplitude=(float)(i)/temp_float_real;
									if (window_amplitude>1.)
									{
										window_amplitude=2.-window_amplitude;
									}
									*transform_buffer_value=window_amplitude*
										((*signal_real_float)-mean_real);
									signal_real_float += buffer_offset;
									transform_buffer_value++;
									i--;
									window_amplitude=(float)(i)/temp_float_real;
									if (window_amplitude>1.)
									{
										window_amplitude=2.-window_amplitude;
									}
									*transform_buffer_value=window_amplitude*
										((*signal_real_float)-mean_real);
									signal_real_float += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i--;
									j--;
								}
								if (1==i)
								{
									/* odd number of data values */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									*transform_buffer_value=((*signal_real_float)-mean_real)/
										temp_float_real;
									transform_buffer_value++;
									*transform_buffer_value=0;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									j--;
								}
							} break;
							case WELCH_WINDOW:
							{
								temp_float_real=(float)(number_of_data_points+1)/2.;
								while (i>1)
								{
									/* perform ordering */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									window_amplitude=
										(temp_float_real-(float)(i))/temp_float_real;
									window_amplitude *= -window_amplitude;
									window_amplitude += 1.;
									*transform_buffer_value=window_amplitude*
										((*signal_real_float)-mean_real);
									signal_real_float += buffer_offset;
									transform_buffer_value++;
									i--;
									window_amplitude=
										(temp_float_real-(float)(i))/temp_float_real;
									window_amplitude *= -window_amplitude;
									window_amplitude += 1.;
									*transform_buffer_value=window_amplitude*
										((*signal_real_float)-mean_real);
									signal_real_float += buffer_offset;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									i--;
									j--;
								}
								if (1==i)
								{
									/* odd number of data values */
									transform_buffer_value=
										transform_buffer_values+bit_reversed_location;
									window_amplitude=(float)(number_of_data_points)
										/(temp_float_real*temp_float_real);
									*transform_buffer_value=window_amplitude*
										((*signal_real_float)-mean_real);
									transform_buffer_value++;
									*transform_buffer_value=0;
									CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
									j--;
								}
							} break;
						}
					} break;
				}
			}
			while (j>0)
			{
				/* perform ordering */
				transform_buffer_value=transform_buffer_values+bit_reversed_location;
				*transform_buffer_value=0;
				transform_buffer_value++;
				*transform_buffer_value=0;
				CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
				j--;
			}
			/* perform the transformation */
			two_i=1;
			two_i_plus_1=2;
			number_of_sub_transforms=number_of_transform_points;
			while (two_i<number_of_transform_points)
			{
				number_of_sub_transforms /= 2;
				/* calculate w-1 */
				theta=(double)PI/(double)two_i;
				w_minus_1_real=sin(0.5*theta);
				w_minus_1_real *= -2*w_minus_1_real;
				w_minus_1_imaginary=sin(theta);
				/* w^0 */
				w_k_real_double=1;
				w_k_imaginary_double=0;
				/* loop over data */
				even_half_base=transform_buffer_values;
				for (k=two_i;k>0;k--)
				{
					even_half=even_half_base;
					odd_half=even_half_base+two_i_plus_1;
					for (j=number_of_sub_transforms;j>0;j--)
					{
						/* use Danielson-Lanczos lemma */
						odd_half_real= *odd_half;
						odd_half_imaginary=odd_half[1];
						even_half_real= *even_half;
						even_half_imaginary=even_half[1];
						/* (w^k)*odd_half */
						w_k_real_float=(float)w_k_real_double;
						w_k_imaginary_float=(float)w_k_imaginary_double;
						temp_float_real=w_k_real_float*odd_half_real-
							w_k_imaginary_float*odd_half_imaginary;
						temp_float_imaginary=w_k_real_float*odd_half_imaginary+
							w_k_imaginary_float*odd_half_real;
						/* odd_half = even_half - (w^k)*odd_half */
						*odd_half=even_half_real-temp_float_real;
						odd_half[1]=even_half_imaginary-temp_float_imaginary;
						/* even_half = even_half + (w^k)*odd_half */
						*even_half=even_half_real+temp_float_real;
						even_half[1]=even_half_imaginary+temp_float_imaginary;
						even_half=odd_half+two_i_plus_1;
						odd_half=even_half+two_i_plus_1;
					}
					/* w^(k+1)=(w^k)*w=(w^k)+(w^k)*(w-1) */
					temp_double=w_k_real_double+w_k_real_double*w_minus_1_real-
						w_k_imaginary_double*w_minus_1_imaginary;
					w_k_imaginary_double=w_k_imaginary_double+
						w_k_imaginary_double*w_minus_1_real+
						w_k_real_double*w_minus_1_imaginary;
					w_k_real_double=temp_double;
					even_half_base += 2;
				}
				two_i=two_i_plus_1;
				two_i_plus_1 *= 2;
			}
			if (signal_imaginary)
			{
				/* allow for the signal offsets */
				transform_buffer_values[0] -= 2.*(signal_real->channel->gain)*
					((signal_real->channel->offset)-mean_real);
				transform_buffer_values[1] -= 2.*(signal_imaginary->channel->gain)*
					((signal_imaginary->channel->offset)-mean_imaginary);
				transform_buffer_values[0] /= 2.;
				transform_buffer_values[1] /= 2.;
			}
			else
			{
				/* calculate the transformation of the original real function */
				/* calculate w-1 */
				theta=(double)PI/(double)number_of_transform_points;
				w_minus_1_real=sin(0.5*theta);
				w_minus_1_real *= -2*w_minus_1_real;
				w_minus_1_imaginary=sin(theta);
				/* w^0 */
				w_k_real_double=1;
				w_k_imaginary_double=0;
				/* loop over data */
				even_half=transform_buffer_values;
				odd_half=transform_buffer_values+2*number_of_transform_points;
				/* calculate first complex value separately */
				temp_float_real= *even_half;
				temp_float_imaginary=even_half[1];
				*even_half=temp_float_real+temp_float_imaginary;
				even_half++;
				*even_half=temp_float_real-temp_float_imaginary;
				even_half++;
				for (i=number_of_transform_points/2-1;i>0;i--)
				{
					/* w^(k+1)=(w^k)*w=(w^k)+(w^k)*(w-1) */
					temp_double=w_k_real_double+w_k_real_double*w_minus_1_real-
						w_k_imaginary_double*w_minus_1_imaginary;
					w_k_imaginary_double=w_k_imaginary_double+
						w_k_imaginary_double*w_minus_1_real+
						w_k_real_double*w_minus_1_imaginary;
					w_k_real_double=temp_double;
					even_half_real= *even_half;
					even_half_imaginary=even_half[1];
					odd_half--;
					odd_half_imaginary= *odd_half;
					odd_half--;
					odd_half_real= *odd_half;
					/* h_1=(even_half+conj(odd_half))/2 */
					h_1_real=(even_half_real+odd_half_real)/2;
					h_1_imaginary=(even_half_imaginary-odd_half_imaginary)/2;
					/* h_2= -i*(even_half-conj(odd_half))/2 */
					h_2_real=(even_half_imaginary+odd_half_imaginary)/2;
					h_2_imaginary=(odd_half_real-even_half_real)/2;
					/* (w^k)*h_2 */
					w_k_real_float=(float)w_k_real_double;
					w_k_imaginary_float=(float)w_k_imaginary_double;
					temp_float_real=w_k_real_float*h_2_real-
						w_k_imaginary_float*h_2_imaginary;
					temp_float_imaginary=w_k_real_float*h_2_imaginary+
						w_k_imaginary_float*h_2_real;
					/* even_half=h_1+(w^k)*h_2 */
					*even_half=h_1_real+temp_float_real;
					even_half++;
					*even_half=h_1_imaginary+temp_float_imaginary;
					even_half++;
					/* odd_half=conj(h_1-(w^k)*h_2) */
					*odd_half=h_1_real-temp_float_real;
					odd_half[1]=temp_float_imaginary-h_1_imaginary;
				}
				/* allow for the signal offset */
				transform_buffer_values[0] -= 2.*((signal_real->channel->offset)-
					mean_real)*(float)(number_of_transform_points);
				transform_buffer_values[0] /= 2.;
				transform_buffer_values[1] /= 2.;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"fourier_transform.  Transform buffer too small");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,"fourier_transform.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* fourier_transform */
#endif /* defined (OLD_CODE) */

int inverse_fourier_transform(struct Device *signal_real,
	struct Device *signal_imaginary,struct Device *transform_real,
	struct Device *transform_imaginary)
/*******************************************************************************
LAST MODIFIED : 13 October 1999

DESCRIPTION :
Calculates the inverse Fourier transform of a complex signal.  If
<transform_imaginary> is NULL, then the complex signal is assumed to be even
so that the inverse transform is real.  Extra memory is allocated for the
transform buffer if required.
==============================================================================*/
{
	double temp_double,theta,w_k_imaginary_double,w_k_real_double,
		w_minus_1_imaginary,w_minus_1_real;
	float *even_half,*even_half_base,even_half_imaginary,even_half_real,
		gain_imaginary,gain_real,h_1_imaginary,h_1_real,h_2_imaginary,h_2_real,
		*odd_half,odd_half_imaginary,odd_half_real,*signal_imaginary_float,
		*signal_real_float,temp_float_imaginary,temp_float_real,
		*transform_buffer_values,*transform_buffer_value,transform_frequency,
		w_k_imaginary_float,w_k_real_float;
	int bit_reversed_location,bit_reversed_location_odd,buffer_offset,i,j,k,
		number_of_data_points,number_of_sub_transforms,number_of_transform_points,
		number_of_transform_signals,return_code,single_bit,*transform_buffer_time,
		two_i,two_i_plus_1;
	short int *signal_imaginary_short_int,*signal_real_short_int;
	struct Signal_buffer *signal_buffer,*transform_buffer;

	ENTER(inverse_fourier_transform);
	/* check the arguments */
	if (signal_real&&(signal_real->signal)&&
		(signal_buffer=signal_real->signal->buffer)&&
		signal_imaginary&&(signal_imaginary->signal)&&
		(signal_buffer==signal_imaginary->signal->buffer)&&
		transform_real&&(transform_real->signal)&&
		(transform_buffer=transform_real->signal->buffer)&&
		((!transform_imaginary&&(1==transform_buffer->number_of_signals))||
		(transform_imaginary&&(2==transform_buffer->number_of_signals)&&
		(transform_real->signal->index!=transform_imaginary->signal->index)&&
		(transform_imaginary->signal)&&
		(transform_buffer==transform_imaginary->signal->buffer))))
	{
		number_of_data_points=(signal_buffer->end)-(signal_buffer->start)+1;
		/* choose the number of transform points to be the smallest power of 2
			greater than the number of data points */
		number_of_transform_points=1;
		while (number_of_transform_points<number_of_data_points)
		{
			number_of_transform_points *= 2;
		}
		if (transform_imaginary)
		{
			number_of_transform_signals=2;
			transform_frequency=((float)number_of_transform_points)/
				(signal_buffer->frequency);
		}
		else
		{
			number_of_transform_signals=1;
			transform_frequency=((float)(2*number_of_transform_points))/
				(signal_buffer->frequency);
		}
		/* check storage for the transform */
		/* allocate/modify storage for the transform */
		if (((2*number_of_transform_points)/number_of_transform_signals==
			transform_buffer->number_of_samples)&&
			(FLOAT_VALUE==transform_buffer->value_type))
		{
			transform_buffer_values=transform_buffer->signals.float_values;
			return_code=1;
		}
		else
		{
			if ((2*number_of_transform_points)/number_of_transform_signals!=
				transform_buffer->number_of_samples)
			{
				ALLOCATE(transform_buffer_time,int,
					(2*number_of_transform_points)/number_of_transform_signals);
			}
			else
			{
				transform_buffer_time=transform_buffer->times;
			}
			if (transform_buffer_time)
			{
				if (ALLOCATE(transform_buffer_values,float,
					2*number_of_transform_points))
				{
					if (transform_buffer_time!=transform_buffer->times)
					{
						DEALLOCATE(transform_buffer->times);
						transform_buffer->times=transform_buffer_time;
					}
					if (FLOAT_VALUE!=transform_buffer->value_type)
					{
						DEALLOCATE(transform_buffer->signals.short_int_values);
					}
					else
					{
						DEALLOCATE(transform_buffer->signals.float_values);
					}
					transform_buffer->signals.float_values=transform_buffer_values;
					transform_buffer->number_of_signals=number_of_transform_signals;
					transform_buffer->number_of_samples=
						(2*number_of_transform_points)/number_of_transform_signals;
					transform_buffer->value_type=FLOAT_VALUE;
					return_code=1;
				}
				else
				{
					display_message(ERROR_MESSAGE,
						"inverse_fourier_transform.  No memory for values");
					if (transform_buffer_time!=transform_buffer->times)
					{
						DEALLOCATE(transform_buffer_time);
					}
					return_code=0;
				}
			}
			else
			{
				display_message(ERROR_MESSAGE,
					"inverse_fourier_transform.  No memory for times");
				return_code=0;
			}
		}
		if (return_code)
		{
			/* assign times */
			i=transform_buffer->number_of_samples;
			transform_buffer_time=(transform_buffer->times)+i;
			while (i>0)
			{
				transform_buffer_time--;
				i--;
				*transform_buffer_time=i;
			}
			transform_real->signal->index=0;
			transform_real->signal_minimum=1;
			transform_real->signal_maximum=0;
			transform_real->channel->offset=0.;
			/* use channel offset and gain */
			transform_real->channel->gain=1./(float)number_of_transform_points;
/*			transform_real->channel->gain=1.;*/
			transform_buffer->frequency=transform_frequency;
			transform_buffer->start=0;
			buffer_offset=signal_buffer->number_of_signals;
			i=number_of_data_points;
			j=number_of_transform_points;
			/* calculate the inverse Fourier transform of the complex function */
			/* perform the bit-reversal ordering and padding */
			/* use channel offset and gain */
			gain_real=signal_real->channel->gain;
/*			gain_real=(signal_real->channel->gain)/(float)number_of_transform_points;*/
			/* use channel offset and gain */
			gain_imaginary=signal_imaginary->channel->gain;
/*			gain_imaginary=(signal_imaginary->channel->gain)/
				(float)number_of_transform_points;*/
			bit_reversed_location=0;
			bit_reversed_location_odd=2*(number_of_transform_points-1);
			if (transform_imaginary)
			{
				transform_buffer->end=number_of_transform_points-1;
				transform_imaginary->signal->index=1;
				transform_imaginary->signal_minimum=1;
				transform_imaginary->signal_maximum=0;
				transform_imaginary->channel->offset=0.;
				/* use channel offset and gain */
/*				transform_imaginary->channel->gain=1.;*/
				transform_imaginary->channel->gain=1./(float)number_of_transform_points;
				switch (signal_buffer->value_type)
				{
					case SHORT_INT_VALUE:
					{
						signal_real_short_int=(signal_buffer->signals.short_int_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_real->signal->index));
						signal_imaginary_short_int=
							(signal_buffer->signals.short_int_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_imaginary->signal->index));
						/* use channel offset and gain */
/*						*transform_buffer_values=
							2.*gain_real*(float)(*signal_real_short_int);*/
						transform_real->channel->offset=
							-2.*gain_real*(float)(*signal_real_short_int);
						*transform_buffer_values=0.;
						signal_real_short_int += buffer_offset;
						/* use channel offset and gain */
						transform_imaginary->channel->offset=
							-2.*gain_imaginary*(float)(*signal_imaginary_short_int);
/*						transform_buffer_values[1]=
							2.*gain_imaginary*(float)(*signal_imaginary_short_int);*/
						transform_buffer_values[1]=0.;
						signal_imaginary_short_int += buffer_offset;
						i--;
						j--;
						CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
						while (i>0)
						{
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=gain_real*(float)(*signal_real_short_int);
							signal_real_short_int += buffer_offset;
							transform_buffer_value[1]=gain_imaginary*
								(float)(*signal_imaginary_short_int);
							signal_imaginary_short_int += buffer_offset;
							i -= 2;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location_odd;
							*transform_buffer_value=gain_real*
								(float)(signal_real_short_int[i*buffer_offset]);
							transform_buffer_value[1]=gain_imaginary*
								(float)(signal_imaginary_short_int[i*buffer_offset]);
							CALCULATE_BIT_REVERSED_LOCATION_MINUS(bit_reversed_location_odd);
							j -= 2;
						}
						while (j>0)
						{
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=0.;
							transform_buffer_value[1]=0.;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location_odd;
							*transform_buffer_value=0.;
							transform_buffer_value[1]=0.;
							CALCULATE_BIT_REVERSED_LOCATION_MINUS(bit_reversed_location_odd);
							j -= 2;
						}
					} break;
					case FLOAT_VALUE:
					{
						signal_real_float=(signal_buffer->signals.float_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_real->signal->index));
						signal_imaginary_float=
							(signal_buffer->signals.float_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_imaginary->signal->index));
						/* use channel offset and gain */
						transform_real->channel->offset=2.*gain_real*(*signal_real_float);
/*						*transform_buffer_values=2.*gain_real*(*signal_real_float);*/
						*transform_buffer_values=0.;
						signal_real_float += buffer_offset;
						/* use channel offset and gain */
						transform_imaginary->channel->offset=
							2.*gain_imaginary*(*signal_imaginary_float);
/*						transform_buffer_values[1]=
							2.*gain_imaginary*(*signal_imaginary_float);*/
						transform_buffer_values[1]=0.;
						signal_imaginary_float += buffer_offset;
						i--;
						j--;
						CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
						while (i>0)
						{
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=gain_real*(*signal_real_float);
							signal_real_float += buffer_offset;
							transform_buffer_value[1]=gain_imaginary*
								(*signal_imaginary_float);
							signal_imaginary_float += buffer_offset;
							i -= 2;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location_odd;
							*transform_buffer_value=gain_real*
								signal_real_float[i*buffer_offset];
							transform_buffer_value[1]=gain_imaginary*
								signal_imaginary_float[i*buffer_offset];
							CALCULATE_BIT_REVERSED_LOCATION_MINUS(bit_reversed_location_odd);
							j -= 2;
						}
						while (j>0)
						{
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=0.;
							transform_buffer_value[1]=0.;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location_odd;
							*transform_buffer_value=0.;
							transform_buffer_value[1]=0.;
							CALCULATE_BIT_REVERSED_LOCATION_MINUS(bit_reversed_location_odd);
							j -= 2;
						}
					} break;
				}
			}
			else
			{
				transform_buffer->end=2*number_of_transform_points-1;
				/* calculate the transformation of the original real function */
				/* calculate w-1 */
				theta= -(double)PI/(double)number_of_transform_points;
				w_minus_1_real=sin(0.5*theta);
				w_minus_1_real *= -2*w_minus_1_real;
				w_minus_1_imaginary=sin(theta);
				/* w^0 */
				w_k_real_double=1;
				w_k_imaginary_double=0;
				/* loop over data */
				switch (signal_buffer->value_type)
				{
					case SHORT_INT_VALUE:
					{
						signal_real_short_int=(signal_buffer->signals.short_int_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_real->signal->index));
						signal_imaginary_short_int=
							(signal_buffer->signals.short_int_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_imaginary->signal->index));
						/* calculate first complex value separately */
						/* use channel offset and gain */
						transform_real->channel->offset=
							-gain_real*(float)(*signal_real_short_int);
/*						temp_float_real=2.*gain_real*(float)(*signal_real_short_int);*/
						temp_float_real=0.;
						signal_real_short_int += buffer_offset;
						temp_float_imaginary=
							2.*gain_imaginary*(float)(*signal_imaginary_short_int);
						signal_imaginary_short_int += buffer_offset;
						*transform_buffer_values=(temp_float_real+temp_float_imaginary)/2.;
						transform_buffer_values[1]=
							(temp_float_real-temp_float_imaginary)/2.;
						i--;
						CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
						while (i>0)
						{
							/* w^(k+1)=(w^k)*w=(w^k)+(w^k)*(w-1) */
							temp_double=w_k_real_double+w_k_real_double*w_minus_1_real-
								w_k_imaginary_double*w_minus_1_imaginary;
							w_k_imaginary_double=w_k_imaginary_double+
								w_k_imaginary_double*w_minus_1_real+
								w_k_real_double*w_minus_1_imaginary;
							w_k_real_double=temp_double;
							even_half_real=gain_real*(float)(*signal_real_short_int);
							signal_real_short_int += buffer_offset;
							even_half_imaginary=gain_imaginary*
								(float)(*signal_imaginary_short_int);
							signal_imaginary_short_int += buffer_offset;
							i -= 2;
							odd_half_real=gain_real*
								(float)(signal_real_short_int[i*buffer_offset]);
							odd_half_imaginary=gain_imaginary*
								(float)(signal_imaginary_short_int[i*buffer_offset]);
							/* h_1=(even_half+conj(odd_half))/2 */
							h_1_real=(even_half_real+odd_half_real)/2;
							h_1_imaginary=(even_half_imaginary-odd_half_imaginary)/2;
							/* h_2= i*(even_half-conj(odd_half))/2 */
							h_2_real= -(even_half_imaginary+odd_half_imaginary)/2;
							h_2_imaginary=(even_half_real-odd_half_real)/2;
							/* (w^k)*h_2 */
							w_k_real_float=(float)w_k_real_double;
							w_k_imaginary_float=(float)w_k_imaginary_double;
							temp_float_real=w_k_real_float*h_2_real-
								w_k_imaginary_float*h_2_imaginary;
							temp_float_imaginary=w_k_real_float*h_2_imaginary+
								w_k_imaginary_float*h_2_real;
							/* even_half=h_1+(w^k)*h_2 */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=h_1_real+temp_float_real;
							transform_buffer_value[1]=h_1_imaginary+temp_float_imaginary;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							/* odd_half=conj(h_1-(w^k)*h_2) */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location_odd;
							*transform_buffer_value=h_1_real-temp_float_real;
							transform_buffer_value[1]=temp_float_imaginary-h_1_imaginary;
							CALCULATE_BIT_REVERSED_LOCATION_MINUS(bit_reversed_location_odd);
							j -= 2;
						}
						while (j>0)
						{
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=0.;
							transform_buffer_value[1]=0.;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location_odd;
							*transform_buffer_value=0.;
							transform_buffer_value[1]=0.;
							CALCULATE_BIT_REVERSED_LOCATION_MINUS(bit_reversed_location_odd);
							j -= 2;
						}
					} break;
					case FLOAT_VALUE:
					{
						signal_real_float=(signal_buffer->signals.float_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_real->signal->index));
						signal_imaginary_float=
							(signal_buffer->signals.float_values)+
							((signal_buffer->start)*buffer_offset+
							(signal_imaginary->signal->index));
						/* calculate first complex value separately */
						/* use channel offset and gain */
						transform_real->channel->offset=
							-gain_real*(*signal_real_float);
/*						temp_float_real=2.*gain_real*(*signal_real_float);*/
						temp_float_real=0.;
						signal_real_float += buffer_offset;
						temp_float_imaginary=2.*gain_imaginary*(*signal_imaginary_float);
						signal_imaginary_float += buffer_offset;
						*transform_buffer_values=(temp_float_real+temp_float_imaginary)/2.;
						transform_buffer_values[1]=
							(temp_float_real-temp_float_imaginary)/2.;
						i--;
						CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
						while (i>0)
						{
							/* w^(k+1)=(w^k)*w=(w^k)+(w^k)*(w-1) */
							temp_double=w_k_real_double+w_k_real_double*w_minus_1_real-
								w_k_imaginary_double*w_minus_1_imaginary;
							w_k_imaginary_double=w_k_imaginary_double+
								w_k_imaginary_double*w_minus_1_real+
								w_k_real_double*w_minus_1_imaginary;
							w_k_real_double=temp_double;
							even_half_real=gain_real*(*signal_real_float);
							signal_real_float += buffer_offset;
							even_half_imaginary=gain_imaginary*(*signal_imaginary_float);
							signal_imaginary_float += buffer_offset;
							i -= 2;
							odd_half_real=gain_real*signal_real_float[i*buffer_offset];
							odd_half_imaginary=gain_imaginary*
								signal_imaginary_float[i*buffer_offset];
							/* h_1=(even_half+conj(odd_half))/2 */
							h_1_real=(even_half_real+odd_half_real)/2;
							h_1_imaginary=(even_half_imaginary-odd_half_imaginary)/2;
							/* h_2= i*(even_half-conj(odd_half))/2 */
							h_2_real= -(even_half_imaginary+odd_half_imaginary)/2;
							h_2_imaginary=(even_half_real-odd_half_real)/2;
							/* (w^k)*h_2 */
							w_k_real_float=(float)w_k_real_double;
							w_k_imaginary_float=(float)w_k_imaginary_double;
							temp_float_real=w_k_real_float*h_2_real-
								w_k_imaginary_float*h_2_imaginary;
							temp_float_imaginary=w_k_real_float*h_2_imaginary+
								w_k_imaginary_float*h_2_real;
							/* even_half=h_1+(w^k)*h_2 */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=h_1_real+temp_float_real;
							transform_buffer_value[1]=h_1_imaginary+temp_float_imaginary;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							/* odd_half=conj(h_1-(w^k)*h_2) */
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location_odd;
							*transform_buffer_value=h_1_real-temp_float_real;
							transform_buffer_value[1]=temp_float_imaginary-h_1_imaginary;
							CALCULATE_BIT_REVERSED_LOCATION_MINUS(bit_reversed_location_odd);
							j -= 2;
						}
						while (j>0)
						{
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location;
							*transform_buffer_value=0.;
							transform_buffer_value[1]=0.;
							CALCULATE_BIT_REVERSED_LOCATION_PLUS(bit_reversed_location);
							transform_buffer_value=
								transform_buffer_values+bit_reversed_location_odd;
							*transform_buffer_value=0.;
							transform_buffer_value[1]=0.;
							CALCULATE_BIT_REVERSED_LOCATION_MINUS(bit_reversed_location_odd);
							j -= 2;
						}
					} break;
				}
			}
			/* perform the transformation */
			two_i=1;
			two_i_plus_1=2;
			number_of_sub_transforms=number_of_transform_points;
			while (two_i<number_of_transform_points)
			{
				number_of_sub_transforms /= 2;
				/* calculate w-1 */
				theta= -(double)PI/(double)two_i;
				w_minus_1_real=sin(0.5*theta);
				w_minus_1_real *= -2*w_minus_1_real;
				w_minus_1_imaginary=sin(theta);
				/* w^0 */
				w_k_real_double=1;
				w_k_imaginary_double=0;
				/* loop over data */
				even_half_base=transform_buffer_values;
				for (k=two_i;k>0;k--)
				{
					even_half=even_half_base;
					odd_half=even_half_base+two_i_plus_1;
					for (j=number_of_sub_transforms;j>0;j--)
					{
						/* use Danielson-Lanczos lemma */
						odd_half_real= *odd_half;
						odd_half_imaginary=odd_half[1];
						even_half_real= *even_half;
						even_half_imaginary=even_half[1];
						/* (w^k)*odd_half */
						w_k_real_float=(float)w_k_real_double;
						w_k_imaginary_float=(float)w_k_imaginary_double;
						temp_float_real=w_k_real_float*odd_half_real-
							w_k_imaginary_float*odd_half_imaginary;
						temp_float_imaginary=w_k_real_float*odd_half_imaginary+
							w_k_imaginary_float*odd_half_real;
						/* odd_half = even_half - (w^k)*odd_half */
						*odd_half=even_half_real-temp_float_real;
						odd_half[1]=even_half_imaginary-temp_float_imaginary;
						/* even_half = even_half + (w^k)*odd_half */
						*even_half=even_half_real+temp_float_real;
						even_half[1]=even_half_imaginary+temp_float_imaginary;
						even_half=odd_half+two_i_plus_1;
						odd_half=even_half+two_i_plus_1;
					}
					/* w^(k+1)=(w^k)*w=(w^k)+(w^k)*(w-1) */
					temp_double=w_k_real_double+w_k_real_double*w_minus_1_real-
						w_k_imaginary_double*w_minus_1_imaginary;
					w_k_imaginary_double=w_k_imaginary_double+
						w_k_imaginary_double*w_minus_1_real+
						w_k_real_double*w_minus_1_imaginary;
					w_k_real_double=temp_double;
					even_half_base += 2;
				}
				two_i=two_i_plus_1;
				two_i_plus_1 *= 2;
			}
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"inverse_fourier_transform.  Transform buffer too small");
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"inverse_fourier_transform.  Invalid argument(s)");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* inverse_fourier_transform */
