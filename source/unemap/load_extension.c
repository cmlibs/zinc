/*******************************************************************************
FILE : load_extension.c

LAST MODIFIED : 4 Septembr 1995

DESCRIPTION :
AIX kernel extension loader.  An adaptation of the program in section A.9.1
(p.217) of
'IBM AIX Version 3.1 RISC System/6000 as a Real-Time System', Document Number
  GG24-3633-0, March 1991, International Technical Support Center Austin, Texas.

COMPILATION :
cc load_extension.c -o load_extension

USAGE :
load_extension <kernel extension>
==============================================================================*/
#include <stddef.h>
#include "myio.h"
/* header file for error numbers */
#include <errno.h>
/* header file for string routines */
#include <string.h>
/* header file for AIX types */
#include <sys/types.h>
/* header file for stat routine */
#include <sys/stat.h>
/* header file for sysconfig syscall */
#include <sys/sysconfig.h>
/* defines CFG_INIT and CFG_TERM */
#include <sys/device.h>

main(int argc,char *argv[])
{
	struct cfg_kmod configure;
	struct cfg_load kernel_extension;
	struct stat stat_buffer;
	char load_command,load_file_name[256];
	int return_code;

	/* check the user ID of the process and exit if not superuser */
	if (getuid()==0)
	{
		/* fetch arguments passed to the program */
		if (argc==2)
		{
			strncpy(load_file_name,argv[1],255);
			load_file_name[255]='\0';
			/* check if the file exists */
			if (stat(load_file_name,&stat_buffer)==0)
			{
				kernel_extension.path=load_file_name;
				kernel_extension.libpath=NULL;
				kernel_extension.kmid=0;
				do
				{
					do
					{
						printf("Enter choice: (u)nload, (l)oad, (q)uery or (e)nd\n");
						load_command=getchar();
					}
					while ((load_command!='u')&&(load_command!='l')&&(load_command!='q')&&
						(load_command!='e'));
					/* do a sysconfig query to find out if the extension is already
						loaded */
					if (sysconfig(SYS_QUERYLOAD,&kernel_extension,
						sizeof(kernel_extension)))
					{
						perror("sysconfig(SYS_QUERYLOAD)");
					}
					switch (load_command)
					{
						case 'l':
						{
							if (sysconfig(SYS_KLOAD,&kernel_extension,
								sizeof(kernel_extension)))
							{
								perror("sysconfig(SYS_KLOAD)");
							}
							else
							{
								printf("Extension loaded - kmid is %d\n",kernel_extension.kmid);
								/* configure the kernel extension */
								configure.kmid=kernel_extension.kmid;
								configure.cmd=CFG_INIT;
								configure.mdiptr=NULL;
								configure.mdilen=0;
								if (sysconfig(SYS_CFGKMOD,&configure,sizeof(configure)))
								{
									perror("sysconfig(SYS_CFGKMOD)");
								}
							}
						} break;
						case 'u':
						{
							/* check if the kernel extension is loaded */
							if (kernel_extension.kmid!=0)
							{
								/* unconfigure the kernel extension */
								configure.kmid=kernel_extension.kmid;
								configure.cmd=CFG_TERM;
								configure.mdiptr=NULL;
								configure.mdilen=0;
								if (sysconfig(SYS_CFGKMOD,&configure,sizeof(configure)))
								{
									perror("sysconfig(SYS_CFGKMOD)");
								}
								else
								{
									if (sysconfig(SYS_KULOAD,&kernel_extension,
										sizeof(kernel_extension)))
									{
										perror("sysconfig(SYS_KULOAD)");
									}
									else
									{
										printf("Unload OK\n");
									}
								}
							}
							else
							{
								printf("Extension is not loaded\n");
							}
						} break;
						case 'q':
						{
							if (kernel_extension.kmid==0)
							{
								printf("Extension is not loaded\n");
							}
							else
							{
								printf("Extension loaded - kmid is %d\n",kernel_extension.kmid);
							}
						} break;
					}
				}
				while (load_command!='e');
			}
			else
			{
				printf("%s does not exist\n",load_file_name);
				return_code=errno;
			}
		}
		else
		{
			printf("Usage: %s <kernel extension>\n",argv[0]);
			return_code=EINVAL;
		}
	}
	else
	{
		printf("load_extension: Permission denied\n");
		return_code=EACCES;
	}

	return (return_code);
} /* main */
