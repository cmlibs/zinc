SHELL=/bin/sh
VPATH=$(CMISS_ROOT)/cmgui/source
TEST_PATH=$(CMISS_ROOT)/cmgui/test_examples

#SGI debug version
cmgui : force cmgui_sgi.make
	if [ -f cmgui_sgi.make ]; then \
		make -f cmgui_sgi.make; \
	else \
		make -f $(VPATH)/cmgui_sgi.make; \
	fi

cmgui_sgi.make : cmgui.imake cmgui.make
	if [ -f cmgui.imake ]; then \
		imake -DIRIX -s cmgui_sgi.make -T cmgui.imake -f /dev/null; \
	else \
		imake -DIRIX -s cmgui_sgi.make -T $(VPATH)/cmgui.imake -f /dev/null; \
	fi

#SGI optimised version
cmgui_optimised : force cmgui_sgioptimised.make
	if [ -f cmgui_sgioptimised.make ]; then \
		make -f cmgui_sgioptimised.make; \
	else \
		make -f $(VPATH)/cmgui_sgioptimised.make; \
	fi	

cmgui_sgioptimised.make : cmgui.imake cmgui.make
	if [ -f cmgui.imake ]; then \
		imake -DIRIX -DOPTIMISED -s cmgui_sgioptimised.make -T cmgui.imake -f /dev/null; \
	else \
		imake -DIRIX -DOPTIMISED -s cmgui_sgioptimised.make -T $(VPATH)/cmgui.imake -f /dev/null; \
	fi

#SGI optimised lite version
cmgui_lite : force cmgui_sgilite.make
	if [ -f cmgui_sgilite.make ]; then \
		make -f cmgui_sgilite.make; \
	else \
		make -f $(VPATH)/cmgui_sgilite.make; \
	fi	

cmgui_sgilite.make : cmgui.imake cmgui.make
	if [ -f cmgui.imake ]; then \
		imake -DIRIX -DOPTIMISED -DLITEWEIGHT -s cmgui_sgilite.make -T cmgui.imake -f /dev/null; \
	else \
		imake -DIRIX -DOPTIMISED -DLITEWEIGHT -s cmgui_sgilite.make -T $(VPATH)/cmgui.imake -f /dev/null; \
	fi

#SGI debug memory check version
cmgui_memorycheck : force cmgui_sgi_memorycheck.make
	if [ -f cmgui_sgi_memorycheck.make ]; then \
		make -f cmgui_sgi_memorycheck.make; \
	else \
		make -f $(VPATH)/cmgui_sgi_memorycheck.make; \
	fi

cmgui_sgi_memorycheck.make : cmgui.imake cmgui.make
	if [ -f cmgui.imake ]; then \
		imake -DIRIX -DMEMORY_CHECK -s cmgui_sgi_memorycheck.make -T cmgui.imake -f /dev/null; \
	else \
		imake -DIRIX -DMEMORY_CHECK -s cmgui_sgi_memorycheck.make -T $(VPATH)/cmgui.imake -f /dev/null; \
	fi

#SGI 64bit version
cmgui64 : force cmgui_sgi64.make
	if [ -f cmgui_sgi64.make ]; then \
		make -f cmgui_sgi64.make; \
	else \
		make -f $(VPATH)/cmgui_sgi64.make; \
	fi	

cmgui_sgi64.make : cmgui.imake cmgui.make
	if [ -f cmgui.imake ]; then \
		imake -DIRIX -DO64 -DOPTIMISED -s cmgui_sgi64.make -T cmgui.imake -f /dev/null; \
	else \
		imake -DIRIX -DO64 -DOPTIMISED -s cmgui_sgi64.make -T $(VPATH)/cmgui.imake -f /dev/null; \
	fi

#Linux version
cmgui_linux : force cmgui_linux.make
	if [ -f cmgui_linux.make ]; then \
		make -f cmgui_linux.make; \
	else \
		make -f $(VPATH)/cmgui_linux.make; \
	fi

cmgui_linux.make : cmgui.imake cmgui.make
	if [ -f cmgui.imake ]; then \
		imake -DLINUX -s cmgui_linux.make -T cmgui.imake -f /dev/null; \
	else \
		imake -DLINUX -s cmgui_linux.make -T $(VPATH)/cmgui.imake -f /dev/null; \
	fi

update :
	if ( [ "$(PWD)" -ef "$(VPATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cvs update && \
		chgrp -R cmgui_programmers * && \
		make -f cmgui.make CMGUI_BASE_DIRECTORY=$(VPATH) cmgui cmgui_optimised cmgui64 cmgui_lite cmgui_memorycheck && \
		rsh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(VPATH) ; make -f cmgui.make cmgui_linux' && \
		chgrp -R cmgui_programmers *; \
	else \
		echo "Must be cmiss and in $(VPATH)"; \
	fi

depend : cmgui_sgi.make cmgui_sgioptimised.make cmgui_sgi64.make cmgui_linux.make cmgui_sgi_memorycheck.make
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(VPATH); \
		make -f cmgui_sgi.make depend ; \
		make -f cmgui_sgioptimised.make depend ; \
		make -f cmgui_sgi_memorycheck.make depend ; \
		make -f cmgui_sgi64.make depend ; \
		rsh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(VPATH) ; make -f cmgui_linux.make depend ' ; \
	else \
		echo "Must be cmiss"; \
	fi
	

run_tests :
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(TEST_PATH); \
		make -u; \
		rsh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(TEST_PATH) ; make -W cmgui_linux.exe cmgui_linux_test' ; \
		cat all.mail ; \
	else \
		echo "Must be cmiss"; \
	fi

cronjob :
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(VPATH); \
		echo -n > programmer.mail ; \
		if ! make -f cmgui.make depend; then \
			cat dependfail.mail >> programmer.mail ; \
		fi ; \
		if ! make -f cmgui.make update; then \
			cat updatefail.mail >> programmer.mail ; \
		fi ; \
		if ! make -f cmgui.make run_tests; then \
			cat testfail.mail >> programmer.mail ; \
		fi ; \
		if [ -s $(TEST_PATH)/all.mail ]; then \
			cat $(TEST_PATH)/all.mail >> programmer.mail ; \
		fi ; \
		if [ -s programmer.mail ]; then \
			cat header.mail programmer.mail | sed "s/DATE/`date`/" | mail cmguiprogrammers@esu1.auckland.ac.nz ; \
		else \
			cat success.mail | sed "s/DATE/`date`/" | mail s.blackett@auckland.ac.nz ; \
		fi; \
	else \
		echo "Must be cmiss"; \
	fi

nothing.imake :
	echo "\n" > nothing.imake

force :
	@echo "\n" > /dev/null
