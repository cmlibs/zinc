/*******************************************************************************
FILE : ratio_signals.c

LAST MODIFIED : 15 May 2002

DESCRIPTION :
Read in a signal file and a list of pairs of electrodes and write out a signal
file containing the ratios of the pairs.

CODE SWITCHS :
TESTING - try and reproduce Darren's ratio executable - not general code.
==============================================================================*/
#include <stddef.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "general/debug.h"
#include "general/mystring.h"
#include "unemap/analysis.h"
#include "unemap/rig.h"
#include "unemap/rig_node.h"
#include "user_interface/user_interface.h"

#define TESTING

/*
Main program
------------
*/
int main(int argc,char *argv[])
/*******************************************************************************
LAST MODIFIED : 15 May 2002

DESCRIPTION :
==============================================================================*/
{
#if defined (TESTING)
	char *name;
	FILE *offsets_file,*signal_file;
	float a,alpha_aa,alpha_ab,alpha_ac,alpha_ba,alpha_bb,alpha_bc,alpha_ca,
		alpha_cb,alpha_cc,b,background_offset,*background_offsets,beta_a,beta_b,
		beta_c,c,*denominator_value,*denominator_values,dyda,dydb,dydc,error,
		error_last,lambda,laser_average,laser_rms,mean_x,mean_y,moving_average,
		*numerator_value,*numerator_values,sampling_frequency,*saved_value,
		*saved_values,scale,sign_a,signal_rms,sum_x,sum_xx,sum_xy,sum_y,sum_yy,
		temp_a,temp_b,temp_c,temp_float,tolerance,*value,x,y;
	int count,device_index,i,number_of_devices,number_of_offsets,
		number_of_samples,number_of_signals,number_in_moving_average,return_code,
		signal_index,*time;
	struct Device **denominator,**device,**numerator,**ratio_device;
	struct Rig *ratio_rig,*signal_rig;
	struct Signal_buffer *signal_buffer;
#else /* defined (TESTING) */
	char *name,*numerator_name,*denominator_name;
	FILE *ratio_file,*signal_file;
	float *denominator_value,*denominator_values,*numerator_value,
		*numerator_values,signal_offset,*value;
	int i,index,number_of_pairs,number_of_samples,number_of_signals,return_code,
		*time;
	struct Device **device,**numerator,**denominator;
	struct Rig *ratio_rig,*signal_rig;
	struct Signal_buffer *signal_buffer;
#endif /* defined (TESTING) */

	/* check arguments */
	return_code=0;
#if defined (TESTING)
	if (5==argc)
	{
		/* read the signal file */
		signal_rig=(struct Rig *)NULL;
		if ((signal_file=fopen(argv[1],"rb"))&&read_signal_file(signal_file,
			&signal_rig))
		{
			fclose(signal_file);
			/* open the background offsets file */
			if (offsets_file=fopen(argv[2],"r"))
			{
				/* read in the offsets */
				number_of_offsets=0;
				return_code=1;
				background_offsets=(float *)NULL;
				while (return_code&&(1==fscanf(offsets_file,"%f",&background_offset)))
				{
					if (REALLOCATE(value,background_offsets,float,number_of_offsets+1))
					{
						background_offsets=value;
						background_offsets[number_of_offsets]=background_offset;
						number_of_offsets++;
					}
					else
					{
						printf("ERROR.  Could not reallocate background_offsets\n");
						return_code=0;
					}
				}
				if (return_code&&(16==signal_rig->number_of_devices)&&
					(12==number_of_offsets))
				{
					if ((1==sscanf(argv[3],"%d",&number_in_moving_average))&&
						(0<number_in_moving_average)&&ALLOCATE(saved_values,float,
						number_in_moving_average+1))
					{
						number_in_moving_average /= 2;
						number_in_moving_average *= 2;
						number_in_moving_average++;
						/* create ratios signal buffer */
						number_of_devices=2*(signal_rig->number_of_devices)+
							number_of_offsets+6;
						number_of_signals=3*(signal_rig->number_of_devices)+
							2*number_of_offsets+6;
						number_of_samples=
							(*(signal_rig->devices))->signal->buffer->number_of_samples;
						sampling_frequency=(*(signal_rig->devices))->signal->buffer->
							frequency;
						if (signal_buffer=create_Signal_buffer(FLOAT_VALUE,
							number_of_signals,number_of_samples,sampling_frequency))
						{
							time=signal_buffer->times;
							for (i=0;i<number_of_samples;i++)
							{
								*time=i;
								time++;
							}
							/* create ratio rig */
							if (ratio_rig=create_standard_Rig("ratios",PATCH,MONITORING_OFF,
								EXPERIMENT_OFF,1,&number_of_devices,1,0,(float)1))
							{
								/* fill in the devices and signals */
								ratio_device=ratio_rig->devices;
								device_index=0;
								signal_index=0;
								/* put in existing signals */
								device=signal_rig->devices;
								while (return_code&&
									(device_index<(signal_rig->number_of_devices)))
								{
									/* change name */
									if (REALLOCATE(name,(*ratio_device)->description->name,char,
										strlen((*device)->description->name)+1))
									{
										(*ratio_device)->description->name=name;
										strcpy(name,(*device)->description->name);
										/* create the signal */
										if ((*ratio_device)->signal=create_Signal(signal_index,
											signal_buffer,REJECTED,0))
										{
											/* fill in the values */
											numerator_values=(float *)NULL;
											if (extract_signal_information((struct FE_node *)NULL,
												(struct Signal_drawing_package *)NULL,*device,1,
												1,0,(int *)NULL,(int *)NULL,(float **)NULL,
												&numerator_values,(enum Event_signal_status **)NULL,
												(char **)NULL,(int *)NULL,(float *)NULL,
												(float *)NULL))
											{
												numerator_value=numerator_values;
												value=(signal_buffer->signals).float_values+
													signal_index;
												for (i=number_of_samples;i>0;i--)
												{
													*value= *numerator_value;
													value += number_of_signals;
													numerator_value++;
												}
												DEALLOCATE(numerator_values);
												device++;
												ratio_device++;
												device_index++;
												signal_index++;
											}
											else
											{
												printf("ERROR.  Could not extract signal values\n");
												return_code=0;
											}
										}
										else
										{
											printf("ERROR.  Could not create signal\n");
											return_code=0;
										}
									}
									else
									{
										printf("ERROR.  Could not reallocate device name\n");
										return_code=0;
									}
								}
								/* add background subtracted signals */
								device=ratio_rig->devices;
								if (return_code&&
									extract_signal_information((struct FE_node *)NULL,
									(struct Signal_drawing_package *)NULL,device[3],1,
									1,0,(int *)NULL,(int *)NULL,(float **)NULL,
									&denominator_values,(enum Event_signal_status **)NULL,
									(char **)NULL,(int *)NULL,(float *)NULL,
									(float *)NULL))
								{
									saved_value=saved_values;
									temp_float=denominator_values[0];
									moving_average=(float)(number_in_moving_average/2)*temp_float;
									for (i=number_in_moving_average/2;i>0;i--)
									{
										*saved_value=temp_float;
										saved_value++;
									}
									denominator_value=denominator_values;
									for (i=number_in_moving_average/2+1;i>0;i--)
									{
										*saved_value= *denominator_value;
										moving_average += *denominator_value;
										denominator_value++;
										saved_value++;
									}
									saved_value=saved_values;
									laser_average=0;
									denominator_value=denominator_values;
									laser_rms=0;
									for (i=number_of_samples-(number_in_moving_average/2)-1;i>0;
										i--)
									{
										temp_float=(*denominator_value)-moving_average/
											(float)number_in_moving_average;
										laser_rms += temp_float*temp_float;
										laser_average += *denominator_value;
										moving_average -= *saved_value;
										*saved_value=denominator_value[number_in_moving_average/2];
										moving_average += *saved_value;
										saved_value++;
										if (number_in_moving_average<=saved_value-saved_values)
										{
											saved_value=saved_values;
										}
										denominator_value++;
									}
									for (i=(number_in_moving_average/2)+1;i>0;i--)
									{
										temp_float=(*denominator_value)-moving_average/
											(float)number_in_moving_average;
										laser_rms += temp_float*temp_float;
										laser_average += *denominator_value;
										moving_average -= *saved_value;
										*saved_value=denominator_values[number_of_samples-1];
										moving_average += *saved_value;
										saved_value++;
										if (number_in_moving_average<=saved_value-saved_values)
										{
											saved_value=saved_values;
										}
										denominator_value++;
									}
									laser_average /= (float)number_of_samples;
									laser_rms=(float)sqrt((double)laser_rms/
										(double)number_in_moving_average);
									denominator_value=denominator_values;
									for (i=number_of_samples;i>0;i--)
									{
										*denominator_value -= laser_average;
										denominator_value++;
									}
									while (return_code&&
										(device_index<2*(signal_rig->number_of_devices)))
									{
										/* change name */
										if (REALLOCATE(name,(*ratio_device)->description->name,char,
											strlen((*device)->description->name)+12))
										{
											(*ratio_device)->description->name=name;
											strcpy(name,(*device)->description->name);
											strcat(name,"_b");
											/* create the signals */
											if (((*device)->signal->next=create_Signal(signal_index,
												signal_buffer,REJECTED,1))&&
												((*ratio_device)->signal=create_Signal(signal_index+
												(signal_rig->number_of_devices),signal_buffer,REJECTED,
												0)))
											{
												/* fill in the values */
												numerator_values=(float *)NULL;
												if (extract_signal_information((struct FE_node *)NULL,
													(struct Signal_drawing_package *)NULL,*device,1,
													1,0,(int *)NULL,(int *)NULL,(float **)NULL,
													&numerator_values,(enum Event_signal_status **)NULL,
													(char **)NULL,(int *)NULL,(float *)NULL,
													(float *)NULL))
												{
													saved_value=saved_values;
													temp_float=numerator_values[0];
													moving_average=
														(float)(number_in_moving_average/2)*temp_float;
													for (i=number_in_moving_average/2;i>0;i--)
													{
														*saved_value=temp_float;
														saved_value++;
													}
													numerator_value=numerator_values;
													for (i=number_in_moving_average/2+1;i>0;i--)
													{
														*saved_value= *numerator_value;
														moving_average += *numerator_value;
														numerator_value++;
														saved_value++;
													}
													saved_value=saved_values;
													numerator_value=numerator_values;
													signal_rms=0;
													value=(signal_buffer->signals).float_values+
														signal_index;
													for (
														i=number_of_samples-(number_in_moving_average/2)-1;
														i>0;i--)
													{
														*value=moving_average/
															(float)number_in_moving_average;
														temp_float=(*numerator_value)-(*value);
														value += number_of_signals;
														signal_rms += temp_float*temp_float;
														moving_average -= *saved_value;
														*saved_value=
															numerator_value[number_in_moving_average/2];
														moving_average += *saved_value;
														saved_value++;
														if (number_in_moving_average<=
															saved_value-saved_values)
														{
															saved_value=saved_values;
														}
														numerator_value++;
													}
													for (i=(number_in_moving_average/2)+1;i>0;i--)
													{
														*value=moving_average/
															(float)number_in_moving_average;
														temp_float=(*numerator_value)-(*value);
														value += number_of_signals;
														signal_rms += temp_float*temp_float;
														moving_average -= *saved_value;
														*saved_value=numerator_values[number_of_samples-1];
														moving_average += *saved_value;
														saved_value++;
														if (number_in_moving_average<=
															saved_value-saved_values)
														{
															saved_value=saved_values;
														}
														numerator_value++;
													}
													signal_rms=(float)sqrt((double)signal_rms/
														(double)number_in_moving_average);
													numerator_value=numerator_values;
													denominator_value=denominator_values;
													value=(signal_buffer->signals).float_values+
														(signal_index+(signal_rig->number_of_devices));
													scale=signal_rms/laser_rms;
													background_offset=
														background_offsets[device_index-number_of_offsets];
													for (i=number_of_samples;i>0;i--)
													{
														*value=(*numerator_value)-background_offset-
															(*denominator_value)*scale;
														value += number_of_signals;
														numerator_value++;
														denominator_value++;
													}
													DEALLOCATE(numerator_values);
													device++;
													ratio_device++;
													device_index++;
													signal_index++;
												}
												else
												{
													printf(
							"ERROR.  Could not extract numerator and denominator values\n");
													return_code=0;
												}
											}
											else
											{
												printf("ERROR.  Could not create signal\n");
												return_code=0;
											}
										}
										else
										{
											printf("ERROR.  Could not reallocate device name\n");
											return_code=0;
										}
									}
									DEALLOCATE(denominator_values);
								}
								else
								{
									if (return_code)
									{
										printf("ERROR.  Could not extract laser signal values\n");
										return_code=0;
									}
								}
								signal_index += (signal_rig->number_of_devices);
								/* add exponential decay approximations */
								device=ratio_device-number_of_offsets;
								tolerance=.01;
								while (return_code&&(device_index<
									2*(signal_rig->number_of_devices)+number_of_offsets))
								{
									/* change name */
									if (REALLOCATE(name,(*ratio_device)->description->name,char,
										strlen((*device)->description->name)+2))
									{
										(*ratio_device)->description->name=name;
										strcpy(name,(*device)->description->name);
										strcat(name,"e");
										/* create the signal */
										if (((*device)->signal->next=create_Signal(signal_index,
											signal_buffer,REJECTED,1))&&
											((*ratio_device)->signal=create_Signal(signal_index+
											number_of_offsets,signal_buffer,REJECTED,1)))
										{
											if (extract_signal_information((struct FE_node *)NULL,
												(struct Signal_drawing_package *)NULL,*device,1,
												1,0,(int *)NULL,(int *)NULL,(float **)NULL,
												&numerator_values,(enum Event_signal_status **)NULL,
												(char **)NULL,(int *)NULL,(float *)NULL,
												(float *)NULL))
											{
												/* fit a*exp(c*t[i])+b to v[i] */
												/* initial estimate for b */
												if (numerator_values[0]>
													numerator_values[number_of_samples-1])
												{
													sign_a=1;
													b=numerator_values[number_of_samples-1]-
														(numerator_values[0]-
														numerator_values[number_of_samples-1])/10.;
												}
												else
												{
													sign_a= -1;
													b=numerator_values[number_of_samples-1]+
														(numerator_values[0]-
														numerator_values[number_of_samples-1])/10.;
												}
												/* do linear regression of c*t[i]+log(a) against
													log(v[i]-b) to get initial estimates of c and a */
													numerator_value=numerator_values;
												sum_x=(float)0;
												sum_y=(float)0;
												count=0;
												for (i=0;i<number_of_samples;i++)
												{
													x=(float)i/sampling_frequency;
													y=(*numerator_value)-b;
													if (y<0)
													{
														y= -y;
													}
													if (y>(float)0)
													{
														count++;
														y=(float)log((double)y);
														sum_x += x;
														sum_y += y;
													}
													numerator_value++;
												}
												if (0<count)
												{
													mean_x=sum_x/(float)count;
													mean_y=sum_y/(float)count;
													sum_xx=(float)0;
													sum_yy=(float)0;
													sum_xy=(float)0;
													c=(float)0;
													numerator_value=numerator_values;
													for (i=0;i<number_of_samples;i++)
													{
														x=(float)i/sampling_frequency;
														y=(*numerator_value)-b;
														if (y<0)
														{
															y= -y;
														}
														if (y>(float)0)
														{
															y=(float)log((double)y);
															x -= mean_x;
															c += x*y;
															y -= mean_y;
															sum_xx += x*x;
															sum_yy += y*y;
															sum_xy += x*y;
														}
														numerator_value++;
													}
													c /= sum_xx;
													a=(mean_y-c*mean_x);
													a=sign_a*(float)exp((double)a);
												}
												else
												{
													printf(
														"ERROR.  No valid values for exponential fit\n");
													return_code=0;
												}
												if (return_code)
												{
													/*???debug */
													printf("Levenberg-Marquardt\n");
													/* do Levenberg-Marquardt to improve */
													lambda=1000.;
													lambda=0.001;
													error=0;
													numerator_value=numerator_values;
													for (i=0;i<number_of_samples;i++)
													{
														x=(float)i/sampling_frequency;
														y=(*numerator_value)-
															(a*(float)exp((double)(c*x))+b);
														error += y*y;
														numerator_value++;
													}
													/*???debug */
													printf("  a=%g, b=%g, c=%g, lambda=%g, error=%g\n",a,
														b,c,lambda,error);
													do
													{
	/*???debug */
/*	float alpha_aa_save,alpha_ab_save,alpha_ac_save,alpha_ba_save,alpha_bb_save,
		alpha_bc_save,alpha_ca_save,alpha_cb_save,alpha_cc_save,beta_a_save,
		beta_b_save,beta_c_save;*/
														beta_a=0;
														beta_b=0;
														beta_c=0;
														alpha_aa=0;
														alpha_ab=0;
														alpha_ac=0;
														alpha_ba=0;
														alpha_bb=0;
														alpha_bc=0;
														alpha_ca=0;
														alpha_cb=0;
														alpha_cc=0;
														numerator_value=numerator_values;
														for (i=0;i<number_of_samples;i++)
														{
															x=(float)i/sampling_frequency;
															dyda=(float)exp((double)(c*x));
															dydb=1;
															dydc=a*x*dyda;
															y=(*numerator_value)-(a*dyda+b);
															beta_a += y*dyda;
															beta_b += y*dydb;
															beta_c += y*dydc;
															alpha_aa += dyda*dyda;
															alpha_ab += dyda*dydb;
															alpha_ac += dyda*dydc;
															alpha_ba += dydb*dyda;
															alpha_bb += dydb*dydb;
															alpha_bc += dydb*dydc;
															alpha_ca += dydc*dyda;
															alpha_cb += dydc*dydb;
															alpha_cc += dydc*dydc;
															numerator_value++;
														}
														alpha_aa *= 1+lambda;
														alpha_bb *= 1+lambda;
														alpha_cc *= 1+lambda;
	/*???debug */
	/*alpha_aa_save=alpha_aa;
	alpha_ab_save=alpha_ab;
	alpha_ac_save=alpha_ac;
	alpha_ba_save=alpha_ba;
	alpha_bb_save=alpha_bb;
	alpha_bc_save=alpha_bc;
	alpha_ca_save=alpha_ca;
	alpha_cb_save=alpha_cb;
	alpha_cc_save=alpha_cc;
	beta_a_save=beta_a;
	beta_b_save=beta_b;
	beta_c_save=beta_c;*/
														/* solve the linear equations alpha*delta=beta for
															delta */
														temp_a=fabs(alpha_aa);
														temp_b=fabs(alpha_ba);
														temp_c=fabs(alpha_ca);
														if (temp_a>=temp_b)
														{
															if (temp_c>temp_a)
															{
																temp_float=alpha_aa;
																alpha_aa=alpha_ca;
																alpha_ca=temp_float;
																temp_float=alpha_ab;
																alpha_ab=alpha_cb;
																alpha_cb=temp_float;
																temp_float=alpha_ac;
																alpha_ac=alpha_cc;
																alpha_cc=temp_float;
																temp_float=beta_a;
																beta_a=beta_c;
																beta_c=temp_float;
															}
														}
														else
														{
															if (temp_b>temp_c)
															{
																temp_float=alpha_aa;
																alpha_aa=alpha_ba;
																alpha_ba=temp_float;
																temp_float=alpha_ab;
																alpha_ab=alpha_bb;
																alpha_bb=temp_float;
																temp_float=alpha_ac;
																alpha_ac=alpha_bc;
																alpha_bc=temp_float;
																temp_float=beta_a;
																beta_a=beta_b;
																beta_b=temp_float;
															}
															else
															{
																temp_float=alpha_aa;
																alpha_aa=alpha_ca;
																alpha_ca=temp_float;
																temp_float=alpha_ab;
																alpha_ab=alpha_cb;
																alpha_cb=temp_float;
																temp_float=alpha_ac;
																alpha_ac=alpha_cc;
																alpha_cc=temp_float;
																temp_float=beta_a;
																beta_a=beta_c;
																beta_c=temp_float;
															}
														}
														alpha_ab /= alpha_aa;
														alpha_ac /= alpha_aa;
														beta_a /= alpha_aa;
														alpha_bb -= alpha_ba*alpha_ab;
														alpha_bc -= alpha_ba*alpha_ac;
														beta_b -= alpha_ba*beta_a;
														alpha_cb -= alpha_ca*alpha_ab;
														alpha_cc -= alpha_ca*alpha_ac;
														beta_c -= alpha_ca*beta_a;
														if (fabs(alpha_cb)>fabs(alpha_bb))
														{
															temp_float=alpha_bb;
															alpha_bb=alpha_cb;
															alpha_cb=temp_float;
															temp_float=alpha_bc;
															alpha_bc=alpha_cc;
															alpha_cc=temp_float;
															temp_float=beta_b;
															beta_b=beta_c;
															beta_c=temp_float;
														}
														alpha_bc /= alpha_bb;
														beta_b /= alpha_bb;
														alpha_cc -= alpha_cb*alpha_bc;
														beta_c -= alpha_cb*beta_b;
	/*???debug */
	/*printf("%g %g %g   %g\n",alpha_aa_save,alpha_ab_save,alpha_ac_save,beta_a_save);
	printf("%g %g %g   %g\n",alpha_ba_save,alpha_bb_save,alpha_bc_save,beta_b_save);
	printf("%g %g %g   %g\n",alpha_ca_save,alpha_cb_save,alpha_cc_save,beta_c_save);
	printf("\n");*/
														beta_c /= alpha_cc;
														beta_b=beta_b-alpha_bc*beta_c;
														beta_a=beta_a-alpha_ab*beta_b-alpha_ac*beta_c;
														error_last=error;
	/*???debug */
	/*printf("%g %g %g\n",beta_a,beta_b,beta_c);
	printf("%g %g %g\n",
		beta_a_save-alpha_aa_save*beta_a-alpha_ab_save*beta_b-alpha_ac_save*beta_c,
		beta_b_save-alpha_ba_save*beta_a-alpha_bb_save*beta_b-alpha_bc_save*beta_c,
		beta_c_save-alpha_ca_save*beta_a-alpha_cb_save*beta_b-alpha_cc_save*beta_c);
	printf("\n");*/
														a += beta_a;
														b += beta_b;
														c += beta_c;
														error=0;
														numerator_value=numerator_values;
														for (i=0;i<number_of_samples;i++)
														{
															x=(float)i/sampling_frequency;
															y=(*numerator_value)-
																(a*(float)exp((double)(c*x))+b);
															error += y*y;
															numerator_value++;
														}
														if (error>=error_last)
														{
															lambda *= 10;
														}
														else
														{
															lambda /= 10;
														}
														/*???debug */
														printf("  a=%g, b=%g, c=%g, lambda=%g, error=%g\n",
															a,b,c,lambda,error);
													} while ((lambda<=1.e6)&&((error>=error_last)||
														((error_last-error)/(1+error_last)>tolerance)));
													value=(signal_buffer->signals).float_values+
														signal_index;
													saved_value=(signal_buffer->signals).float_values+
														(signal_index+number_of_offsets);
													numerator_value=numerator_values;
													for (i=0;i<number_of_samples;i++)
													{
														x=(float)i/sampling_frequency;
														y=a*(float)exp((double)(c*x));
														*value=y+b;
														*saved_value=((*numerator_value)-b)/y;
														value += number_of_signals;
														saved_value += number_of_signals;
														numerator_value++;
													}
													device++;
													ratio_device++;
													device_index++;
													signal_index++;
												}
												DEALLOCATE(numerator_values);
											}
											else
											{
												printf("ERROR.  Could not extract signal values\n");
												return_code=0;
											}
										}
										else
										{
											printf("ERROR.  Could not create signal\n");
											return_code=0;
										}
									}
									else
									{
										printf("ERROR.  Could not reallocate device name\n");
										return_code=0;
									}
								}
								signal_index += number_of_offsets;
								/* add ratio signals */
								numerator=(ratio_rig->devices)+
									(2*(signal_rig->number_of_devices));
								denominator=numerator+1;
								while (return_code&&(signal_index<number_of_signals))
								{
									/* change name */
									if (REALLOCATE(name,(*ratio_device)->description->name,char,
										strlen((*numerator)->description->name)+
										strlen((*denominator)->description->name)+2))
									{
										(*ratio_device)->description->name=name;
										strcpy(name,(*numerator)->description->name);
										strcat(name,"/");
										strcat(name,(*denominator)->description->name);
										/* create the signal */
										if ((*ratio_device)->signal=create_Signal(signal_index,
											signal_buffer,REJECTED,0))
										{
											/* fill in the values */
											numerator_values=(float *)NULL;
											denominator_values=(float *)NULL;
											if (extract_signal_information((struct FE_node *)NULL,
												(struct Signal_drawing_package *)NULL,*numerator,1,
												1,0,(int *)NULL,(int *)NULL,(float **)NULL,
												&numerator_values,(enum Event_signal_status **)NULL,
												(char **)NULL,(int *)NULL,(float *)NULL,
												(float *)NULL)&&
												extract_signal_information((struct FE_node *)NULL,
												(struct Signal_drawing_package *)NULL,*denominator,1,
												1,0,(int *)NULL,(int *)NULL,(float **)NULL,
												&denominator_values,(enum Event_signal_status **)NULL,
												(char **)NULL,(int *)NULL,(float *)NULL,
												(float *)NULL))
											{
												numerator_value=numerator_values;
												denominator_value=denominator_values;
												value=(signal_buffer->signals).float_values+
													signal_index;
												for (i=number_of_samples;i>0;i--)
												{
													if (0!=(*denominator_value))
													{
														*value=(*numerator_value)/(*denominator_value);
													}
													else
													{
														*value=(float)0;
													}
													value += number_of_signals;
													numerator_value++;
													denominator_value++;
												}
												DEALLOCATE(numerator_values);
												DEALLOCATE(denominator_values);
												ratio_device++;
												numerator += 2;
												denominator += 2;
												device_index++;
												signal_index++;
											}
											else
											{
												printf("ERROR.  Could not extract signal values\n");
												return_code=0;
											}
										}
										else
										{
											printf("ERROR.  Could not create signal\n");
											return_code=0;
										}
									}
									else
									{
										printf("ERROR.  Could not reallocate device name\n");
										return_code=0;
									}
								}
								if (return_code)
								{
									if (analysis_write_signal_file(argv[4],ratio_rig,
										number_of_samples/3,(2*number_of_samples)/3,0,
										number_of_samples-1,(char)0,EDA_INTERVAL,1,1,20,80,
										FIXED_DATUM,DEVICE_ORDER,CHANNEL_ORDER,1,
										number_in_moving_average))
									{
										printf("Created signal file: %s\n",argv[4]);
									}
									else
									{
										printf("ERROR.  Writing signal file %s\n",argv[4]);
										return_code=0;
									}
#if defined (OLD_CODE)
									if (signal_file=fopen(argv[4],"wb"))
									{
										if (write_signal_file(signal_file,ratio_rig))
										{
											printf("Created signal file: %s\n",argv[4]);
										}
										else
										{
											printf("ERROR.  Writing signal file\n");
											return_code=0;
										}
										fclose(signal_file);
									}
									else
									{
										printf("ERROR.  Could not open new signal file %s\n",
											argv[4]);
										return_code=0;
									}
#endif /* defined (OLD_CODE) */
								}
							}
							else
							{
								printf("ERROR.  Could not create ratios rig\n");
								return_code=0;
							}
						}
						else
						{
							printf("ERROR.  Could not create ratios signal buffer\n");
							printf("  number_of_signals=%d\n",number_of_offsets);
							printf("  number_of_samples=%d\n",number_of_samples);
							return_code=0;
						}
						DEALLOCATE(saved_values);
					}
					else
					{
						printf("ERROR.  Invalid number_in_moving_average or "
							"could not allocate saved_values\n");
						printf("  number_in_moving_average=%s\n",argv[3]);
						return_code=0;
					}
				}
				else
				{
					if (return_code)
					{
						printf("ERROR.  Invalid background offsets or rig.  %d %d\n",
							number_of_offsets,signal_rig->number_of_devices);
						return_code=0;
					}
				}
				fclose(offsets_file);
			}
			else
			{
				printf("ERROR.  Could not open background offsets file.  %s\n",argv[2]);
				return_code=0;
			}
		}
		else
		{
			printf("ERROR.  Could not read signal file.  %s\n",argv[1]);
			return_code=0;
		}
	}
	else
	{
		printf("usage: ratio_signals in_signal_file background_offsets_file number_in_moving_average out_signal_file\n");
		printf("  in_signal_file is the name of the signal file to be ratioed\n");
		printf("  background_offsets_file is a list of space separated offsets, one for each electrode\n");
		printf("  number_in_moving_average is the number of values to be used for the moving averages\n");
		printf("  out_signal_file is the name for the signal file that contains the ratios\n");
		return_code=0;
	}
#else /* defined (TESTING) */
	if (5==argc)
	{
		/* read the signal file */
		signal_rig=(struct Rig *)NULL;
		if ((signal_file=fopen(argv[1],"rb"))&&read_signal_file(signal_file,
			&signal_rig))
		{
			fclose(signal_file);
			/* open the ratio pairs file */
			if (ratio_file=fopen(argv[2],"r"))
			{
				/* get the signal offset */
				if (1==sscanf(argv[3],"%f",&signal_offset))
				{
					/* count the number of pairs and check the devices exist */
					number_of_pairs=0;
					return_code=1;
					numerator_name=(char *)NULL;
					denominator_name=(char *)NULL;
					while (return_code&&read_string(ratio_file,"s",&numerator_name)&&
						read_string(ratio_file,"s",&denominator_name)&&!feof(ratio_file))
					{
						i=signal_rig->number_of_devices;
						numerator=signal_rig->devices;
						while ((i>0)&&(*numerator)&&((*numerator)->description)&&
							strcmp(numerator_name,(*numerator)->description->name))
						{
							numerator++;
							i--;
						}
						if ((i>0)&&(*numerator)&&((*numerator)->description)&&
							!strcmp(numerator_name,(*numerator)->description->name))
						{
							i=signal_rig->number_of_devices;
							denominator=signal_rig->devices;
							while ((i>0)&&(*denominator)&&((*denominator)->description)&&
								strcmp(denominator_name,(*denominator)->description->name))
							{
								denominator++;
								i--;
							}
							if ((i>0)&&(*denominator)&&((*denominator)->description)&&
								!strcmp(denominator_name,(*denominator)->description->name))
							{
								number_of_pairs++;
							}
							else
							{
								printf("ERROR.  Unknown denominator.  %s\n",denominator_name);
								return_code=0;
							}
						}
						else
						{
							printf("ERROR.  Unknown numerator.  %s\n",numerator_name);
							return_code=0;
						}
						DEALLOCATE(numerator_name);
						DEALLOCATE(denominator_name);
					}
					DEALLOCATE(numerator_name);
					DEALLOCATE(denominator_name);
					if (return_code&&(0<number_of_pairs))
					{
						/* create ratios signal buffer */
						number_of_signals=number_of_pairs;
						number_of_samples=
							(*(signal_rig->devices))->signal->buffer->number_of_samples;
						if (signal_buffer=create_Signal_buffer(FLOAT_VALUE,number_of_pairs,
							number_of_samples,
							(*(signal_rig->devices))->signal->buffer->frequency))
						{
							time=signal_buffer->times;
							for (i=0;i<number_of_samples;i++)
							{
								*time=i;
								time++;
							}
							/* create ratio rig */
							if (ratio_rig=create_standard_Rig("ratios",PATCH,MONITORING_OFF,
								EXPERIMENT_OFF,1,&number_of_pairs,1,0,(float)1))
							{
								/* fill in the devices and signals */
								rewind(ratio_file);
								number_of_pairs=0;
								numerator_name=(char *)NULL;
								denominator_name=(char *)NULL;
								device=ratio_rig->devices;
								index=0;
								while (return_code&&
									read_string(ratio_file,"s",&numerator_name)&&
									read_string(ratio_file,"s",&denominator_name)&&
									!feof(ratio_file))
								{
									i=signal_rig->number_of_devices;
									numerator=signal_rig->devices;
									while ((i>0)&&(*numerator)&&((*numerator)->description)&&
										strcmp(numerator_name,(*numerator)->description->name))
									{
										numerator++;
										i--;
									}
									if ((i>0)&&(*numerator)&&((*numerator)->description)&&
										!strcmp(numerator_name,(*numerator)->description->name))
									{
										i=signal_rig->number_of_devices;
										denominator=signal_rig->devices;
										while ((i>0)&&(*denominator)&&
											((*denominator)->description)&&strcmp(denominator_name,
											(*denominator)->description->name))
										{
											denominator++;
											i--;
										}
										if ((i>0)&&(*denominator)&&((*denominator)->description)&&
											!strcmp(denominator_name,
											(*denominator)->description->name))
										{
											/* change name */
											if (REALLOCATE(name,(*device)->description->name,char,
												strlen(numerator_name)+strlen(denominator_name)+2))
											{
												(*device)->description->name=name;
												strcpy(name,numerator_name);
												strcat(name,"/");
												strcat(name,denominator_name);
												/* create the signal */
												if ((*device)->signal=create_Signal(index,signal_buffer,
													UNDECIDED,0))
												{
													/* fill in the values */
													numerator_values=(float *)NULL;
													denominator_values=(float *)NULL;
													if (extract_signal_information((struct FE_node *)NULL,
														(struct Signal_drawing_package *)NULL,*numerator,1,
														1,0,(int *)NULL,(int *)NULL,(float **)NULL,
														&numerator_values,(enum Event_signal_status **)NULL,
														(char **)NULL,(int *)NULL,(float *)NULL,
														(float *)NULL)&&
														extract_signal_information((struct FE_node *)NULL,
														(struct Signal_drawing_package *)NULL,*denominator,
														1,1,0,(int *)NULL,(int *)NULL,(float **)NULL,
														&denominator_values,
														(enum Event_signal_status **)NULL,(char **)NULL,
														(int *)NULL,(float *)NULL,(float *)NULL))
													{
														numerator_value=numerator_values;
														denominator_value=denominator_values;
														value=(signal_buffer->signals).float_values+index;
														for (i=number_of_samples;i>0;i--)
														{
															if (0!=(*denominator_value)+signal_offset)
															{
																*value=((*numerator_value)+signal_offset)/
																	((*denominator_value)+signal_offset);
															}
															else
															{
																*value=(float)0;
															}
															value += number_of_signals;
															numerator_value++;
															denominator_value++;
														}
														DEALLOCATE(numerator_values);
														DEALLOCATE(denominator_values);
														number_of_pairs++;
														device++;
													}
													else
													{
														printf(
								"ERROR.  Could not extract numerator and denominator values\n");
														return_code=0;
													}
												}
												else
												{
													printf("ERROR.  Could not create signal\n");
													return_code=0;
												}
											}
											else
											{
												printf("ERROR.  Could not reallocate device name\n");
												return_code=0;
											}
										}
										else
										{
											printf("ERROR.  Unknown denominator.  %s\n",
												denominator_name);
											return_code=0;
										}
									}
									else
									{
										printf("ERROR.  Unknown numerator.  %s\n",numerator_name);
										return_code=0;
									}
									DEALLOCATE(numerator_name);
									DEALLOCATE(denominator_name);
									index++;
								}
								DEALLOCATE(numerator_name);
								DEALLOCATE(denominator_name);
								if (return_code)
								{
									if (signal_file=fopen(argv[4],"wb"))
									{
										if (write_signal_file(signal_file,ratio_rig))
										{
											printf("Created signal file: %s\n",argv[4]);
										}
										else
										{
											printf("ERROR.  Writing signal file\n");
											return_code=0;
										}
										fclose(signal_file);
									}
									else
									{
										printf("ERROR.  Could not open new signal file %s\n",
											argv[4]);
										return_code=0;
									}
								}
							}
							else
							{
								printf("ERROR.  Could not create ratios rig\n");
								return_code=0;
							}
						}
						else
						{
							printf("ERROR.  Could not create ratios signal buffer\n");
							printf("  number_of_signals=%d\n",number_of_pairs);
							printf("  number_of_samples=%d\n",number_of_samples);
							return_code=0;
						}
					}
					else
					{
						printf("ERROR.  Invalid or empty ratio pairs file.  %s\n",argv[2]);
						return_code=0;
					}
					fclose(ratio_file);
				}
				else
				{
					printf("ERROR.  Could not read signal offset\n");
					return_code=0;
				}
			}
			else
			{
				printf("ERROR.  Could not open ratio pairs file.  %s\n",argv[2]);
				return_code=0;
			}
		}
		else
		{
			printf("ERROR.  Could not read signal file.  %s\n",argv[1]);
			return_code=0;
		}
	}
	else
	{
		printf("usage: ratio_signals in_signal_file ratio_pairs_file signal_offset out_signal_file\n");
		printf("  in_signal_file is the name of the signal file to be ratioed\n");
		printf("  ratio_pairs_file is a list of space separated pairs electrodes to be ratioed.  One pair to a line.  Numerator first in each pair\n");
		printf("  signal_offset is an offset which is added to every signal before the ratios are calculated\n");
		printf("  out_signal_file is the name for the signal file that contains the ratios\n");
		return_code=0;
	}
#endif /* defined (TESTING) */

	return (return_code);
} /* main */
