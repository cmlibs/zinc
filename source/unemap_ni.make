SHELL=/bin/sh
VPATH=$(CMISS_ROOT)/cmgui/source
TEST_PATH=$(CMISS_ROOT)/cmgui/test_examples

#SGI debug version
unemap32 : force unemap_ni_sgi.make
	if [ -f unemap_ni_sgi.make ]; then \
		make -f unemap_ni_sgi.make; \
	else \
		make -f $(VPATH)/unemap_ni_sgi.make; \
	fi

unemap_ni_sgi.make : unemap_ni.imake unemap_ni.make nothing.imake
	if [ -f unemap_ni.imake ]; then \
		imake -DIRIX -s unemap_ni_sgi.make -T unemap_ni.imake -f nothing.imake; \
	else \
		imake -DIRIX -s unemap_ni_sgi.make -T $(VPATH)/unemap_ni.imake -f nothing.imake; \
	fi

#SGI optimised version
unemap_optimised : force unemap_ni_sgioptimised.make
	if [ -f unemap_ni_sgioptimised.make ]; then \
		make -f unemap_ni_sgioptimised.make; \
	else \
		make -f $(VPATH)/unemap_ni_sgioptimised.make; \
	fi	

unemap_ni_sgioptimised.make : unemap_ni.imake unemap_ni.make nothing.imake
	if [ -f unemap_ni.imake ]; then \
		imake -DIRIX -DOPTIMISED -s unemap_ni_sgioptimised.make -T unemap_ni.imake -f nothing.imake; \
	else \
		imake -DIRIX -DOPTIMISED -s unemap_ni_sgioptimised.make -T $(VPATH)/unemap_ni.imake -f nothing.imake; \
	fi

#SGI 64bit version
unemap_64 : force unemap_ni_sgi64.make
	if [ -f unemap_ni_sgi64.make ]; then \
		make -f unemap_ni_sgi64.make; \
	else \
		make -f $(VPATH)/unemap_ni_sgi64.make; \
	fi	

unemap_ni_sgi64.make : unemap_ni.imake unemap_ni.make nothing.imake
	if [ -f unemap_ni.imake ]; then \
		imake -DIRIX -DO64 -DOPTIMISED -s unemap_ni_sgi64.make -T unemap_ni.imake -f nothing.imake; \
	else \
		imake -DIRIX -DO64 -DOPTIMISED -s unemap_ni_sgi64.make -T $(VPATH)/unemap_ni.imake -f nothing.imake; \
	fi

#Linux version
unemap_linux : force unemap_ni_linux.make
	if [ -f unemap_ni_linux.make ]; then \
		make -f unemap_ni_linux.make; \
	else \
		make -f $(VPATH)/unemap_ni_linux.make; \
	fi

unemap_ni_linux.make : unemap_ni.imake unemap_ni.make nothing.imake
	if [ -f unemap_ni.imake ]; then \
		imake -DLINUX -s unemap_ni_linux.make -T unemap_ni.imake -f nothing.imake; \
	else \
		imake -DLINUX -s unemap_ni_linux.make -T $(VPATH)/unemap_ni.imake -f nothing.imake; \
	fi

update :
	if ( [ "$(PWD)" -ef "$(VPATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cvs update -d && \
		chgrp -R cmgui_programmers *; \
		make -f unemap_ni.make CMGUI_BASE_DIRECTORY=$(VPATH) unemap32 unemap_optimised unemap_64; \
		rsh 130.216.5.155 'setenv CMISS_ROOT /product/cmiss ; cd $(VPATH) ; make -f unemap_ni.make unemap_linux' ; \
		chgrp -R cmgui_programmers *; \
	else \
		echo "Must be cmiss and in $(VPATH)"; \
	fi

depend : unemap_ni_sgi.make unemap_ni_sgioptimised.make unemap_ni_sgi64.make unemap_ni_linux.make
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(VPATH); \
		make -f unemap_ni_sgi.make depend 2> /dev/null ; \
		make -f unemap_ni_sgioptimised.make depend 2> /dev/null ; \
		make -f unemap_ni_sgi64.make depend 2> /dev/null ; \
		rsh 130.216.5.155 'setenv CMISS_ROOT /product/cmiss ; cd $(VPATH) ; make -f unemap_ni_linux.make depend >& /dev/null' ; \
	else \
		echo "Must be cmiss"; \
	fi

cronjob :
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(VPATH); \
		echo -n > unemap_programmer.mail ; \
		if ! make -f unemap_ni.make depend; then \
			cat dependfail.mail >> unemap_programmer.mail ; \
		fi ; \
		if ! make -f unemap_ni.make update; then \
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
