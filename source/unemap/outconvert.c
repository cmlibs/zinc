#include "myio.h"
#define USER_INTERFACE
#include "user_interface.h"
#include "mymemory.h"
#include "rig.h"

main()
{
	FILE *input,*input2,*output;
	struct Rig *rig;
	int i,j,number_of_signals,number_of_samples,value,*signal_max,*signal_min;
	short short_value;
	float frequency,gain,offset;

	if (input=fopen("default.cnfg","r"))
	{
		/*??? what about the rig type ? */
		if (rig=read_configuration(input,TEXT,SOCK))
		{
			if (output=fopen("test.signal","wb"))
			{
				if (write_configuration(rig,output,BINARY))
				{
					if (input2=fopen("signal.ascii","r"))
					{
						fscanf(input2,"number of signals = %d\n",&number_of_signals);
						if ((MYMALLOC(signal_max,int,number_of_signals))&&
							(MYMALLOC(signal_min,int,number_of_signals)))
						{
							for (i=0;i<number_of_signals;i++)
							{
								signal_max[i]=1;
								signal_min[i]= -1;
							}
							BINARY_FILE_WRITE(&number_of_signals,sizeof(int),1,output);
							fscanf(input2,"number of samples = %d\n",&number_of_samples);
							BINARY_FILE_WRITE(&number_of_samples,sizeof(int),1,output);
							frequency=2000;
							BINARY_FILE_WRITE(&frequency,sizeof(float),1,output);
							for (i=0;i<number_of_samples;i++)
							{
								BINARY_FILE_WRITE(&i,sizeof(int),1,output);
							}
							for (i=0;i<number_of_samples;i++)
							{
    	          for (j=0;j<number_of_signals;j++)
								{
									fscanf(input2,"%d\n",&value);
									if ((value>1000)||(value<-1000))
									{
										printf("line number: %d\n",i*number_of_signals+j+2);
										j=number_of_signals;
                    i=number_of_samples;
									}
/*									if (value>signal_max[j])
									{
										signal_max[j]=value;
									}
									if (value<signal_min[j])
									{
										signal_min[j]=value;
									}*/
									short_value=(short)value;
									BINARY_FILE_WRITE(&short_value,sizeof(short),1,output);
								}
/*								printf("%d\n",i);*/
							}
							offset=0;
							gain=1;
							for (i=0;i<number_of_signals;i++)
							{
								BINARY_FILE_WRITE(&i,sizeof(int),1,output);
								BINARY_FILE_WRITE(&offset,sizeof(float),1,output);
								BINARY_FILE_WRITE(&gain,sizeof(float),1,output);
/*								printf("index=%d, min=%d, max=%d\n",i,signal_min[i],signal_max[i]);*/
							}
						}
						fclose(input2);
					}
				}
				fclose(output);
			}
		}
		fclose(input);
	}
}
