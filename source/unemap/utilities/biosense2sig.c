/*******************************************************************************
FILE : biosense2sig.c

LAST MODIFIED : 21 January 2000

DESCRIPTION :
Converts a a Biosense location and activation time file to a signal file.
==============================================================================*/
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
	/*???DB.  Contains definition of __BYTE_ORDER for Linux */
#include "general/debug.h"
#include "general/myio.h"
#include "unemap/analysis_work_area.h"
#include "user_interface/message.h"

/*#define FIT_SPHERE*/

static int analysis_write_signal_file(char *file_name,void *analysis_work_area)
/*******************************************************************************
LAST MODIFIED : 17 January 2000

DESCRIPTION :
This function writes the rig configuration and interval of signal data to the
named file.
???DB.  Modified version of the one in unemap/analysis_work_area.c
==============================================================================*/
{
	FILE *output_file;
	float signal_maximum,signal_minimum;
	int buffer_end,buffer_start,event_number,event_time,i,new_datum,
		new_end_search_interval,new_potential_time,new_start_search_interval,
		number_of_events,return_code,temp_int;
	struct Analysis_work_area *analysis;
	struct Device **device;
	struct Event *event,*start_event;
	struct Rig *rig;
	struct Signal_buffer *buffer;

	ENTER(analysis_write_signal_file);
	/* check the arguments */
	if ((analysis=(struct Analysis_work_area *)analysis_work_area)&&
		(rig=analysis->rig))
	{
		/* open the output file */
		if (output_file=fopen(file_name,"wb"))
		{
			if (return_code=write_signal_file(output_file,rig))
			{
				if ((device=rig->devices)&&(*device)&&((i=rig->number_of_devices)>0)&&
					(buffer=get_Device_signal_buffer(*device)))
				{
					buffer_end=buffer->end;
					buffer_start=buffer->start;
					/* write the event detection settings */
					new_datum=(analysis->datum)-buffer_start;
					new_potential_time=(analysis->potential_time)-buffer_start;
					new_start_search_interval=
						(analysis->start_search_interval)-buffer_start;
					new_end_search_interval=(analysis->end_search_interval)-buffer_start;
					if ((1==BINARY_FILE_WRITE((char *)&(new_datum),sizeof(int),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->calculate_events),
						sizeof(char),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->detection),
						sizeof(enum Event_detection_algorithm),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->event_number),sizeof(int),
						1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->number_of_events),
						sizeof(int),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(new_potential_time),sizeof(int),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->minimum_separation),
						sizeof(int),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->threshold),sizeof(int),1,
						output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->datum_type),
						sizeof(enum Datum_type),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->edit_order),
						sizeof(enum Edit_order),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(analysis->signal_order),
						sizeof(enum Signal_order),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(new_start_search_interval),
						sizeof(int),1,output_file))&&
						(1==BINARY_FILE_WRITE((char *)&(new_end_search_interval),
						sizeof(int),1,output_file)))
					{
						if (EDA_LEVEL==analysis->detection)
						{
							/*???DB.  In case need to change the format later */
							temp_int=1;
							if (!((1==BINARY_FILE_WRITE((char *)&temp_int,sizeof(int),1,
								output_file))&&(1==BINARY_FILE_WRITE((char *)&(analysis->level),
								sizeof(float),1,output_file))&&(1==BINARY_FILE_WRITE(
								(char *)&(analysis->level_width),sizeof(int),1,output_file))))
							{
								return_code=0;
								display_message(ERROR_MESSAGE,
							"analysis_write_signal_file.  Error writing EDA_LEVEL settings");
							}
						}
						/* for each signal write the status, range and events */
						while (return_code&&(i>0))
						{
							/* write the status and range */
							/*???DB.  Originally the unscaled maximum and minimum were
								stored.  This has to be maintained for backward compatability */
							signal_minimum=(*device)->signal_minimum;
							signal_maximum=(*device)->signal_maximum;
							if (0!=((*device)->channel)->gain)
							{
								signal_minimum=(((*device)->channel)->offset)+
									signal_minimum/(((*device)->channel)->gain);
								signal_maximum=(((*device)->channel)->offset)+
									signal_maximum/(((*device)->channel)->gain);
							}
							if ((1==BINARY_FILE_WRITE((char *)&((*device)->signal->status),
								sizeof(enum Event_signal_status),1,output_file))&&
								(1==BINARY_FILE_WRITE((char *)&signal_minimum,
								sizeof(float),1,output_file))&&
								(1==BINARY_FILE_WRITE((char *)&signal_maximum,
								sizeof(float),1,output_file)))
							{
								/* write the events */
								start_event=(*device)->signal->first_event;
								while (start_event&&(start_event->time<buffer_start))
								{
									start_event=start_event->next;
								}
								event=start_event;
								number_of_events=0;
								while (event&&(event->time<=buffer_end))
								{
									number_of_events++;
									event=event->next;
								}
								if (1==BINARY_FILE_WRITE((char *)&number_of_events,sizeof(int),
									1,output_file))
								{
									event=start_event;
									while (return_code&&event&&(event->time<=buffer_end))
									{
										event_number=(event->number)-(start_event->number)+1;
										event_time=(event->time)-buffer_start;
										if ((1==BINARY_FILE_WRITE((char *)&(event_time),sizeof(int),
											1,output_file))&&
											(1==BINARY_FILE_WRITE((char *)&(event_number),sizeof(int),
											1,output_file))&&
											(1==BINARY_FILE_WRITE((char *)&(event->status),
											sizeof(enum Event_signal_status),1,output_file)))
										{
											event=event->next;
										}
										else
										{
											return_code=0;
											display_message(ERROR_MESSAGE,
												"analysis_write_signal_file.  Error writing event");
										}
									}
								}
								else
								{
									return_code=0;
									display_message(ERROR_MESSAGE,
								"analysis_write_signal_file.  Error writing number of events");
								}
							}
							else
							{
								return_code=0;
								display_message(ERROR_MESSAGE,
									"analysis_write_signal_file.  Error writing signal range");
							}
							device++;
							i--;
						}
					}
					else
					{
						return_code=0;
						display_message(ERROR_MESSAGE,
							"analysis_write_signal_file.  Error writing analysis settings");
					}
				}
			}
			fclose(output_file);
		}
		else
		{
			display_message(ERROR_MESSAGE,
				"analysis_write_signal_file.  Invalid file: %s",file_name);
			return_code=0;
		}
	}
	else
	{
		display_message(ERROR_MESSAGE,
			"analysis_write_signal_file.  Missing analysis_work_area");
		return_code=0;
	}
	LEAVE;

	return (return_code);
} /* analysis_write_signal_file */

int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 19 January 2000

DESCRIPTION :
==============================================================================*/
{
	char *device_name,flag;
	enum Event_signal_status *status_array,*status_array_temp;
	FILE *biosense_file;
	float sampling_frequency,x,*x_array,*x_array_temp,y,*y_array,*y_array_temp,z,
		*z_array,*z_array_temp;
	int device_number,i,id,*id_array,*id_array_temp,max_time,min_time,
		number_of_devices,number_of_samples,return_code,time,*time_array,
		*time_array_temp;
	short int *value;
	struct Analysis_work_area analysis_work_area;
	struct Channel *channel;
	struct Device **device,**devices;
	struct Device_description *description;
	struct Region *region;
	struct Region_list_item *region_list;
	struct Signal *signal;
	struct Signal_buffer *signal_buffer;
#if defined (FIT_SPHERE)
	float dfdx,dfdy,dfdz,drdx,drdy,drdz,d2fdxdy,d2fdxdz,d2fdx2,d2fdydx,d2fdydz,
		d2fdy2,d2fdzdx,d2fdzdy,d2fdz2,f,f_prev,phi,pivmax,pivot1,pivot2,pivot3,r,ri,
		ri2,swap,theta,tolerance,vv1,vv2,vv3;
	int converged,inddd1,inddd2,max_steps,step_number;
#endif /* defined (FIT_SPHERE) */

	return_code=0;
	/* check arguments */
	if (3==argc)
	{
		if (biosense_file=fopen(argv[1],"r"))
		{
			/* count the number of devices and determine the minimum and maximum
				activation times */
			return_code=1;
			number_of_devices=0;
			status_array=(enum Event_signal_status *)NULL;
			x_array=(float *)NULL;
			y_array=(float *)NULL;
			z_array=(float *)NULL;
			time_array=(int *)NULL;
			id_array=(int *)NULL;
			max_time=0;
			min_time=1;
			while (return_code&&(4==fscanf(biosense_file," %d %g %g %g",&id,&x,&y,
				&z)))
			{
				number_of_devices++;
				if (
					REALLOCATE(status_array_temp,status_array,enum Event_signal_status,
						number_of_devices)&&
					REALLOCATE(x_array_temp,x_array,float,number_of_devices)&&
					REALLOCATE(y_array_temp,y_array,float,number_of_devices)&&
					REALLOCATE(z_array_temp,z_array,float,number_of_devices)&&
					REALLOCATE(id_array_temp,id_array,int,number_of_devices)&&
					REALLOCATE(time_array_temp,time_array,int,number_of_devices))
				{
					status_array=status_array_temp;
					x_array=x_array_temp;
					y_array=y_array_temp;
					z_array=z_array_temp;
					time_array=time_array_temp;
					id_array=id_array_temp;
					x_array[number_of_devices-1]=x;
					y_array[number_of_devices-1]=y;
					z_array[number_of_devices-1]=z;
					id_array[number_of_devices-1]=id;
					if (1==fscanf(biosense_file," %d",&time))
					{
						time_array[number_of_devices-1]=time;
						if (max_time<min_time)
						{
							min_time=time;
							max_time=time;
						}
						else
						{
							if (time<min_time)
							{
								min_time=time;
							}
							else
							{
								if (max_time<time)
								{
									max_time=time;
								}
							}
						}
						status_array[number_of_devices-1]=ACCEPTED;
					}
					else
					{
						status_array[number_of_devices-1]=REJECTED;
					}
					if (1!=fscanf(biosense_file," %c",&flag))
					{
						printf("Could not read flag %d\n",number_of_devices);
						return_code=0;
					}
				}
				else
				{
					printf("Could not reallocate storage information %d\n",
						number_of_devices);
					return_code=0;
				}
			}
			if (return_code)
			{
				if (feof(biosense_file))
				{
					if (number_of_devices<=0)
					{
						printf("No data\n");
						return_code=0;
					}
				}
				else
				{
					printf("Error reading location %d\n",number_of_devices);
					return_code=0;
				}
			}
			if (return_code)
			{
				/*???debug */
				printf("number_of_devices=%d  min_time=%d, max_time=%d\n",
					number_of_devices,min_time,max_time);
				/* use the centre of mass as the initial estimate */
				x=0;
				y=0;
				z=0;
				x_array_temp=x_array;
				y_array_temp=y_array;
				z_array_temp=z_array;
				for (i=number_of_devices;i>0;i--)
				{
					x += *x_array_temp;
					y += *y_array_temp;
					z += *z_array_temp;
					x_array_temp++;
					y_array_temp++;
					z_array_temp++;
				}
				x /= (float)number_of_devices;
				y /= (float)number_of_devices;
				z /= (float)number_of_devices;
#if defined (FIT_SPHERE)
				/* fit a sphere to the data locations */
				tolerance=1e-6;
				max_steps=10;
				step_number=0;
				converged=0;
				do
				{
					r=0;
					x_array_temp=x_array;
					y_array_temp=y_array;
					z_array_temp=z_array;
					for (i=number_of_devices;i>0;i--)
					{
						ri=sqrt((x-(*x_array_temp))*(x-(*x_array_temp))+
							(y-(*y_array_temp))*(y-(*y_array_temp))+
							(z-(*z_array_temp))*(z-(*z_array_temp)));
						r += ri;
						x_array_temp++;
						y_array_temp++;
						z_array_temp++;
					}
					r /= (float)number_of_devices;
					f_prev=f;
					f=0;
					dfdx=0;
					dfdy=0;
					dfdz=0;
					drdx=0;
					drdy=0;
					drdz=0;
					x_array_temp=x_array;
					y_array_temp=y_array;
					z_array_temp=z_array;
					for (i=number_of_devices;i>0;i--)
					{
						ri=sqrt((x-(*x_array_temp))*(x-(*x_array_temp))+
							(y-(*y_array_temp))*(y-(*y_array_temp))+
							(z-(*z_array_temp))*(z-(*z_array_temp)));
						f += (r-ri)*(r-ri);
						if (0<ri)
						{
							dfdx += (x-(*x_array_temp))*(ri-r)/ri;
							dfdy += (y-(*y_array_temp))*(ri-r)/ri;
							dfdz += (z-(*z_array_temp))*(ri-r)/ri;
							drdx += (x-(*x_array_temp))/ri;
							drdy += (y-(*y_array_temp))/ri;
							drdz += (z-(*z_array_temp))/ri;
						}
						x_array_temp++;
						y_array_temp++;
						z_array_temp++;
					}
					drdx /= (float)number_of_devices;
					drdy /= (float)number_of_devices;
					drdz /= (float)number_of_devices;
					d2fdx2=0;
					d2fdxdy=0;
					d2fdxdz=0;
					d2fdydx=0;
					d2fdy2=0;
					d2fdydz=0;
					d2fdzdx=0;
					d2fdzdy=0;
					d2fdz2=0;
					x_array_temp=x_array;
					y_array_temp=y_array;
					z_array_temp=z_array;
					for (i=number_of_devices;i>0;i--)
					{
						ri2=(x-(*x_array_temp))*(x-(*x_array_temp))+
							(y-(*y_array_temp))*(y-(*y_array_temp))+
							(z-(*z_array_temp))*(z-(*z_array_temp));
						ri=sqrt(ri2);
						if (0<ri)
						{
							d2fdx2 += (ri-r)/ri+(x-(*x_array_temp))*
								(r*(x-(*x_array_temp))/ri-ri*drdx)/ri2;
							d2fdxdy += (x-(*x_array_temp))*
								(r*(y-(*y_array_temp))/ri-ri*drdy)/ri2;
							d2fdxdz += (x-(*x_array_temp))*
								(r*(z-(*z_array_temp))/ri-ri*drdz)/ri2;
							d2fdydx += (y-(*y_array_temp))*
								(r*(x-(*x_array_temp))/ri-ri*drdx)/ri2;
							d2fdy2 += (ri-r)/ri+(y-(*y_array_temp))*
								(r*(y-(*y_array_temp))/ri-ri*drdy)/ri2;
							d2fdydz += (y-(*y_array_temp))*
								(r*(z-(*z_array_temp))/ri-ri*drdz)/ri2;
							d2fdzdx += (z-(*z_array_temp))*
								(r*(x-(*x_array_temp))/ri-ri*drdx)/ri2;
							d2fdzdy += (z-(*z_array_temp))*
								(r*(y-(*y_array_temp))/ri-ri*drdy)/ri2;
							d2fdz2 += (ri-r)/ri+(z-(*z_array_temp))*
								(r*(z-(*z_array_temp))/ri-ri*drdz)/ri2;
						}
						x_array_temp++;
						y_array_temp++;
						z_array_temp++;
					}
					if (0<step_number)
					{
						/* check for convergence */
						if (fabs(f-f_prev)/(f+f_prev)<tolerance)
						{
							converged=1;
						}
					}
					/*???debug */
					printf("%d  %g  %g %g %g %g\n",step_number,f,r,x,y,z);
#if defined (DEBUG)
					printf("df=\n");
					printf("  %g %g %g\n",dfdx,dfdy,dfdz);
					printf("d2f=\n");
					printf("  %g %g %g\n",d2fdx2,d2fdxdy,d2fdxdz);
					printf("  %g %g %g\n",d2fdydx,d2fdy2,d2fdydz);
					printf("  %g %g %g\n",d2fdzdx,d2fdzdy,d2fdz2);
#endif /* defined (DEBUG) */
					if (!converged)
					{
						vv1=fabs(d2fdx2);
						if (fabs(d2fdxdy)>vv1)
						{
							vv1=fabs(d2fdxdy);
						}
						if (fabs(d2fdxdz)>vv1)
						{
							vv1=fabs(d2fdxdz);
						}
						vv2=fabs(d2fdydx);
						if (fabs(d2fdy2)>vv2)
						{
							vv2=fabs(d2fdy2);
						}
						if (fabs(d2fdydz)>vv2)
						{
							vv2=fabs(d2fdydz);
						}
						vv3=fabs(d2fdzdx);
						if (fabs(d2fdzdy)>vv3)
						{
							vv3=fabs(d2fdzdy);
						}
						if (fabs(d2fdz2)>vv3)
						{
							vv3=fabs(d2fdz2);
						}
						pivot1=vv1*fabs(d2fdx2);
						pivot2=vv2*fabs(d2fdydx);
						pivot3=vv3*fabs(d2fdzdx);
						pivmax=pivot1;
						if (pivot2>pivmax)
						{
							pivmax=pivot2;
						}
						if (pivot3>pivmax)
						{
							pivmax=pivot3;
						}
						inddd1=1;
						if (pivot2==pivmax)
						{
							swap=d2fdx2;
							d2fdx2=d2fdydx;
							d2fdydx=swap;
							swap=d2fdxdy;
							d2fdxdy=d2fdy2;
							d2fdy2=swap;
							swap=d2fdxdz;
							d2fdxdz=d2fdydz;
							d2fdydz=swap;
							inddd1=2;
							vv2=vv1;
						}
						else
						{
							if (pivot3==pivmax)
							{
								swap=d2fdx2;
								d2fdx2=d2fdzdx;
								d2fdzdx=swap;
								swap=d2fdxdy;
								d2fdxdy=d2fdzdy;
								d2fdzdy=swap;
								swap=d2fdxdz;
								d2fdxdz=d2fdz2;
								d2fdz2=swap;
								inddd1=3;
								vv3=vv1;
							}
						}
						d2fdydx=d2fdydx/d2fdx2;
						d2fdzdx=d2fdzdx/d2fdx2;
						d2fdy2=d2fdy2-d2fdydx*d2fdxdy;
						d2fdzdy=d2fdzdy-d2fdzdx*d2fdxdy;
						pivot2=vv2*fabs(d2fdy2);
						pivot3=vv3*fabs(d2fdzdy);
						pivmax=pivot2;
						if (pivot3>pivmax)
						{
							pivmax=pivot3;
						}
						inddd2=2;
						if (pivot3==pivmax)
						{
							swap=d2fdydx;
							d2fdydx=d2fdzdx;
							d2fdzdx=swap;
							swap=d2fdy2;
							d2fdy2=d2fdzdy;
							d2fdzdy=swap;
							swap=d2fdydz;
							d2fdydz=d2fdz2;
							d2fdz2=swap;
							inddd2=3;
							vv3=vv2;
						}
						d2fdzdy=d2fdzdy/d2fdy2;
						d2fdydz=d2fdydz-d2fdydx*d2fdxdz;
						d2fdz2=d2fdz2-d2fdzdx*d2fdxdz-d2fdzdy*d2fdydz;
						if (inddd1==2)
						{
							swap=dfdx;
							dfdx=dfdy;
							dfdy=swap;
						}
						else
						{
							if (inddd1==3)
							{
								swap=dfdx;
								dfdx=dfdz;
								dfdz=swap;
							}
						}
						if (inddd2==3)
						{
							swap=dfdy;
							dfdy=dfdz;
							dfdz=swap;
						}
						dfdy=dfdy-d2fdydx*dfdx;
						dfdz=dfdz-d2fdzdx*dfdx-d2fdzdy*dfdy;
						dfdz=dfdz/d2fdz2;
						dfdy=dfdy-d2fdydz*dfdz;
						dfdy=dfdy/d2fdy2;
						dfdx=dfdx-d2fdxdy*dfdy-d2fdxdz*dfdz;
						dfdx=dfdx/d2fdx2;
						x -= dfdx;
						y -= dfdy;
						z -= dfdz;
					}
					step_number++;
				}
				while (return_code&&!converged&&(step_number<max_steps));
				if (return_code&&converged)
				{
					/*???debug */
					printf("x=%g, y=%g, z=%g, r=%g\n",x,y,z,r);
#endif /* defined (FIT_SPHERE) */
					number_of_samples=max_time-min_time+1;
					sampling_frequency=(float)1000;
					if (signal_buffer=create_Signal_buffer(SHORT_INT_VALUE,
						number_of_devices,number_of_samples,sampling_frequency))
					{
						value=(signal_buffer->signals).short_int_values;
						for (i=number_of_devices*number_of_samples;i>0;i--)
						{
							*value=0;
							value++;
						}
						for (i=0;i<number_of_samples;i++)
						{
							(signal_buffer->times)[i]=min_time+i;
						}
						ALLOCATE(devices,struct Device *,number_of_devices);
						ALLOCATE(device_name,char,
							2+(int)log10((double)number_of_devices));
						if (devices&&device_name)
						{
							/* create the region */
							if ((region=create_Region("region",SOCK,0,number_of_devices))&&
								(region_list=create_Region_list_item(region,
								(struct Region_list_item *)NULL)))
							{
								(region->properties).sock.focus=(float)1;
								/* create the devices */
								device=devices;
								device_number=0;
								x_array_temp=x_array;
								y_array_temp=y_array;
								z_array_temp=z_array;
								status_array_temp=status_array;
								id_array_temp=id_array;
								time_array_temp=time_array;
#if defined (DEBUG)
/*???debug */
{
	float max_phi=0,min_phi=1,max_r=0,min_r=1,max_theta=0,
		min_theta=1,max_x=0,min_x=1,max_y=0,min_y=1,max_z=0,min_z=1;

#endif /* defined (DEBUG) */
								while ((device_number<number_of_devices)&&return_code)
								{
									sprintf(device_name,"%d",*id_array_temp);
									if ((description=create_Device_description(device_name,
										ELECTRODE,region))&&(channel=create_Channel(
										device_number+1,(float)0,(float)1))&&
										(signal=create_Signal(device_number,signal_buffer,
										*status_array_temp,0))&&(*device=create_Device(
										device_number,description,channel,signal)))
									{
#if defined (OLD_CODE)
#if defined (DEBUG)
										/*???debug */
										printf("%d.  %g %g %g\n",device_number,(*x_array_temp)-x,
											(*y_array_temp)-y,(*z_array_temp)-z);
#endif /* defined (DEBUG) */
#if defined (DEBUG)
	/*???debug */
	if (max_x<min_x)
	{
		min_x=(*x_array_temp)-x;
		max_x=min_x;
	}
	else
	{
		if ((*x_array_temp)-x>max_x)
		{
			max_x=(*x_array_temp)-x;
		}
		else
		{
			if ((*x_array_temp)-x<min_x)
			{
				min_x=(*x_array_temp)-x;
			}
		}
	}
	if (max_y<min_y)
	{
		min_y=(*y_array_temp)-y;
		max_y=min_y;
	}
	else
	{
		if ((*y_array_temp)-y>max_y)
		{
			max_y=(*y_array_temp)-y;
		}
		else
		{
			if ((*y_array_temp)-y<min_y)
			{
				min_y=(*y_array_temp)-y;
			}
		}
	}
	if (max_z<min_z)
	{
		min_z=(*z_array_temp)-z;
		max_z=min_z;
	}
	else
	{
		if ((*z_array_temp)-z>max_z)
		{
			max_z=(*z_array_temp)-z;
		}
		else
		{
			if ((*z_array_temp)-z<min_z)
			{
				min_z=(*z_array_temp)-z;
			}
		}
	}
#endif /* defined (DEBUG) */
										cartesian_to_spherical_polar((*x_array_temp)-x,
											(*y_array_temp)-y,(*z_array_temp)-z,&ri,&theta,&phi,
											(float *)NULL);
#if defined (DEBUG)
	/*???debug */
	if (max_r<min_r)
	{
		min_r=ri;
		max_r=min_r;
	}
	else
	{
		if (ri>max_r)
		{
			max_r=ri;
		}
		else
		{
			if (ri<min_r)
			{
				min_r=ri;
			}
		}
	}
	if (max_theta<min_theta)
	{
		min_theta=theta;
		max_theta=min_theta;
	}
	else
	{
		if (theta>max_theta)
		{
			max_theta=theta;
		}
		else
		{
			if (theta<min_theta)
			{
				min_theta=theta;
			}
		}
	}
	if (max_phi<min_phi)
	{
		min_phi=phi;
		max_phi=min_phi;
	}
	else
	{
		if (phi>max_phi)
		{
			max_phi=phi;
		}
		else
		{
			if (phi<min_phi)
			{
				min_phi=phi;
			}
		}
	}
#endif /* defined (DEBUG) */
										cartesian_to_spherical_polar((*x_array_temp)-x,
											(*y_array_temp)-y,(*z_array_temp)-z,&ri,&theta,&phi,
											(float *)NULL);
#if defined (DEBUG)
										/*???debug */
										printf("  %g %g %g\n",ri,theta,phi);
#endif /* defined (DEBUG) */
										prolate_spheroidal_to_cartesian(1,phi-PI/2,theta,
											(region->properties).sock.focus,
											&(description->properties.electrode.position.x),
											&(description->properties.electrode.position.y),
											&(description->properties.electrode.position.z),
											(float *)NULL);
#if defined (DEBUG)
										/*???debug */
										printf("  %g %g %g\n",
											description->properties.electrode.position.x,
											description->properties.electrode.position.y,
											description->properties.electrode.position.z);
#endif /* defined (DEBUG) */
#endif /* defined (OLD_CODE) */
										/* rotate about y because x is the principal axis for
											prolate (assuming z is up and down the body for biosense*/
										description->properties.electrode.position.x=
											z-(*z_array_temp);
										description->properties.electrode.position.y=
											(*y_array_temp)-y;
										description->properties.electrode.position.z=
											(*x_array_temp)-x;
										if (ACCEPTED== *status_array_temp)
										{
											if (!((*device)->signal->first_event=create_Event(
												(*time_array_temp)-min_time,1,ACCEPTED,
												(struct Event *)NULL,(struct Event *)NULL)))
											{
												printf("ERROR.  Could not create event %d\n",
													device_number);
												return_code=0;
											}
										}
										device_number++;
										device++;
										x_array_temp++;
										y_array_temp++;
										z_array_temp++;
										status_array_temp++;
										id_array_temp++;
										time_array_temp++;
									}
									else
									{
										printf("ERROR.  Could not allocate memory for device %d\n",
											device_number);
										return_code=0;
									}
								}
#if defined (DEBUG)
	/*???debug */
	printf("x %g %g\n",min_x,max_x);
	printf("y %g %g\n",min_y,max_y);
	printf("z %g %g\n",min_z,max_z);
	printf("r %g %g\n",min_r,max_r);
	printf("theta %g %g\n",min_theta,max_theta);
	printf("phi %g %g\n",min_phi,max_phi);
}
#endif /* defined (DEBUG) */
								if (return_code)
								{
									/* set up the analysis work area */
									analysis_work_area.datum=min_time;
									analysis_work_area.potential_time=(max_time-min_time)/2;
									analysis_work_area.start_search_interval=0;
									analysis_work_area.end_search_interval=max_time-min_time;
									analysis_work_area.calculate_events=1;
									analysis_work_area.detection=EDA_INTERVAL;
									analysis_work_area.objective=ABSOLUTE_SLOPE;
									analysis_work_area.event_number=1;
									analysis_work_area.number_of_events=1;
									analysis_work_area.minimum_separation=100;
									analysis_work_area.threshold=90;
									analysis_work_area.datum_type=AUTOMATIC_DATUM;
									analysis_work_area.edit_order=DEVICE_ORDER;
									analysis_work_area.signal_order=CHANNEL_ORDER;
									/* create the rig */
									if (analysis_work_area.rig=create_Rig("biosense",
										MONITORING_OFF,EXPERIMENT_OFF,number_of_devices,devices,
										(struct Page_list_item *)NULL,1,region_list,
										(struct Region *)NULL))
									{
										/* add the activation times */
										if (analysis_write_signal_file(argv[2],
											&analysis_work_area))
										{
											printf("Created signal file: %s\n",argv[2]);
										}
										else
										{
											printf("ERROR.  Writing signal file\n");
											return_code=0;
										}
									}
									else
									{
										printf("ERROR.  Could not create rig\n");
										return_code=0;
									}
								}
							}
							else
							{
								printf("ERROR.  Could not allocate memory for region\n");
								return_code=0;
							}
						}
						else
						{
							printf(
								"ERROR.  Could not allocate memory for device list\n");
							return_code=0;
						}
					}
					else
					{
						printf("ERROR.  Could not create combined signal buffer\n");
						printf("  number_of_devices=%d\n",number_of_devices);
						printf("  number_of_samples=%d\n",number_of_samples);
						return_code=0;
					}
#if defined (FIT_SPHERE)
				}
				else
				{
					printf("ERROR.  Failed to converge\n");
				}
#endif /* defined (FIT_SPHERE) */
			}
			fclose(biosense_file);
		}
		else
		{
			printf("Could not open Biosense file: %s\n",argv[1]);
			return_code=0;
		}
	}
	else
	{
		printf("usage: biosense2sig biosense_file signal_file\n");
		printf("  biosense_file is the name of the Biosense file (provided)\n");
		printf("  signal_file is the name of the unemap signal file (created)\n");
		return_code=0;
	}

	return (return_code);
} /* main */
