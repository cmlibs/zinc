/*******************************************************************************
FILE : unload_extension.c

LAST MODIFIED : 4 September 1995

DESCRIPTION :
==============================================================================*/
#include <stddef.h>
#include "myio.h"
#include <errno.h>
/* header file for sysconfig syscall.  Used for loading and unloading kernel
	extensions */
#include <sys/sysconfig.h>
#include <sys/device.h>

main()
{
	struct cfg_load kernel_extension;
	struct cfg_kmod configure;

	/* check if the extension is loaded */
	kernel_extension.path="./acquisition_interrupt";
	kernel_extension.libpath=NULL;
	kernel_extension.kmid=0;
	if (sysconfig(SYS_QUERYLOAD,&kernel_extension,sizeof(struct cfg_load))==0)
	{
		if (kernel_extension.kmid!=0)
		{
			/* unconfigure the kernel extension */
			configure.kmid=kernel_extension.kmid;
			configure.cmd=CFG_TERM;
			configure.mdiptr=NULL;
			configure.mdilen=0;
			if (sysconfig(SYS_CFGKMOD,&configure,sizeof(struct cfg_kmod))==0)
			{
				printf("unconfiguration successful\n");
			}
			else
			{
				printf("unconfiguration unsuccessful.  error number=%d\n",errno);
			}
			/* unload the acquisition interrupt kernel extension */
			if (sysconfig(SYS_KULOAD,&kernel_extension,sizeof(struct cfg_load))==0)
			{
				printf("unload successful\n");
			}
			else
			{
				printf("unload unsuccessful.  error number=%d\n",errno);
			}
		}
		else
		{
			printf("extension is not loaded\n");
		}
	}
	else
	{
		printf("load query failed.  error number=%d\n",errno);
	}
} /* main */
