SHELL=/bin/sh
VPATH=$(CMISS_ROOT)/cmgui/source
TEST_PATH=$(CMISS_ROOT)/cmgui/test_examples

#SGI debug version
unemap32 : force unemap_sgi.make
	if [ -f unemap_sgi.make ]; then \
		make -f unemap_sgi.make; \
	else \
		make -f $(VPATH)/unemap_sgi.make; \
	fi

unemap_sgi.make : unemap.imake unemap.make
	if [ -f unemap.imake ]; then \
		imake -DIRIX -s unemap_sgi.make -T unemap.imake -f $(VPATH)/nothing.imake; \
	else \
		imake -DIRIX -s unemap_sgi.make -T $(VPATH)/unemap.imake -f $(VPATH)/nothing.imake; \
	fi

#SGI rig nodes version
unemap_nodes : force unemap_sginodes.make
	if [ -f unemap_sginodes.make ]; then \
		make -f unemap_sginodes.make; \
	else \
		make -f $(VPATH)/unemap_sginodes.make; \
	fi

unemap_sginodes.make : unemap.imake unemap.make
	if [ -f unemap.imake ]; then \
		imake -DIRIX -DUSE_UNEMAP_NODES -s unemap_sginodes.make -T unemap.imake -f $(VPATH)/nothing.imake; \
	else \
		imake -DIRIX -DUSE_UNEMAP_NODES -s unemap_sginodes.make -T $(VPATH)/unemap.imake -f $(VPATH)/nothing.imake; \
	fi

#SGI optimised version
unemap_optimised : force unemap_sgioptimised.make
	if [ -f unemap_sgioptimised.make ]; then \
		make -f unemap_sgioptimised.make; \
	else \
		make -f $(VPATH)/unemap_sgioptimised.make; \
	fi	

unemap_sgioptimised.make : unemap.imake unemap.make
	if [ -f unemap.imake ]; then \
		imake -DIRIX -DOPTIMISED -s unemap_sgioptimised.make -T unemap.imake -f $(VPATH)/nothing.imake; \
	else \
		imake -DIRIX -DOPTIMISED -s unemap_sgioptimised.make -T $(VPATH)/unemap.imake -f $(VPATH)/nothing.imake; \
	fi

#SGI 64bit version
unemap_64 : force unemap_sgi64.make
	if [ -f unemap_sgi64.make ]; then \
		make -f unemap_sgi64.make; \
	else \
		make -f $(VPATH)/unemap_sgi64.make; \
	fi	

unemap_sgi64.make : unemap.imake unemap.make
	if [ -f unemap.imake ]; then \
		imake -DIRIX -DO64 -DOPTIMISED -s unemap_sgi64.make -T unemap.imake -f $(VPATH)/nothing.imake; \
	else \
		imake -DIRIX -DO64 -DOPTIMISED -s unemap_sgi64.make -T $(VPATH)/unemap.imake -f $(VPATH)/nothing.imake; \
	fi

#Linux version
unemap_linux : force unemap_linux.make
	if [ -f unemap_linux.make ]; then \
		make -f unemap_linux.make; \
	else \
		make -f $(VPATH)/unemap_linux.make; \
	fi

unemap_linux.make : unemap.imake unemap.make
	if [ -f unemap.imake ]; then \
		imake -DLINUX -s unemap_linux.make -T unemap.imake -f $(VPATH)/nothing.imake; \
	else \
		imake -DLINUX -s unemap_linux.make -T $(VPATH)/unemap.imake -f $(VPATH)/nothing.imake; \
	fi

update :
	if ( [ "$(PWD)" -ef "$(VPATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cvs update -d && \
		chgrp -R cmgui_programmers *; \
		make -f unemap.make CMGUI_BASE_DIRECTORY=$(VPATH) unemap32 unemap_nodes unemap_optimised unemap_64; \
		rsh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(VPATH) ; make -f unemap.make unemap_linux' ; \
		chgrp -R cmgui_programmers *; \
	else \
		echo "Must be cmiss and in $(VPATH)"; \
	fi

depend : unemap_sgi.make unemap_sginodes.make unemap_sgioptimised.make unemap_sgi64.make unemap_linux.make
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(VPATH); \
		make -f unemap_sgi.make depend 2> /dev/null ; \
		make -f unemap_sginodes.make depend 2> /dev/null ; \
		make -f unemap_sgioptimised.make depend 2> /dev/null ; \
		make -f unemap_sgi64.make depend 2> /dev/null ; \
		rsh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(VPATH) ; make -f unemap_linux.make depend >& /dev/null' ; \
	else \
		echo "Must be cmiss"; \
	fi

cronjob :
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(VPATH); \
		echo -n > unemap_programmer.mail ; \
		if ! make -f unemap.make depend; then \
			cat dependfail.mail >> unemap_programmer.mail ; \
		fi ; \
		if ! make -f unemap.make update; then \
			cat updatefail.mail >> unemap_programmer.mail ; \
		fi ; \
		if [ -s unemap_programmer.mail ]; then \
			cat unemap_header.mail unemap_programmer.mail | sed "s/DATE/`date`/" | mail cmguiprogrammers@esu1.auckland.ac.nz ; \
#		else \
#			cat unemap_success.mail | sed "s/DATE/`date`/" | mail s.blackett@auckland.ac.nz ; \
		fi; \
	else \
		echo "Must be cmiss"; \
	fi

nothing.imake :
	echo "\n" > nothing.imake

force :
	@echo "\n" > /dev/null
