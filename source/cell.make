SHELL=/bin/sh
VPATH=$(CMISS_ROOT)/cmgui/source
TEST_PATH=$(CMISS_ROOT)/cmgui/test_examples

#SGI debug version
cell : force cell_sgi.make
	if [ -f cell_sgi.make ]; then \
		make -f cell_sgi.make; \
	else \
		make -f $(VPATH)/cell_sgi.make; \
	fi

cell_sgi.make : cell.imake cell.make
	if [ -f cell.imake ]; then \
		imake -DIRIX -s cell_sgi.make -T cell.imake -f $(VPATH)/nothing.imake; \
	else \
		imake -DIRIX -s cell_sgi.make -T $(VPATH)/cell.imake -f $(VPATH)/nothing.imake; \
	fi

#SGI optimised version
cell_optimised : force cell_sgioptimised.make
	if [ -f cell_sgioptimised.make ]; then \
		make -f cell_sgioptimised.make; \
	else \
		make -f $(VPATH)/cell_sgioptimised.make; \
	fi	

cell_sgioptimised.make : cell.imake cell.make
	if [ -f cell.imake ]; then \
		imake -DIRIX -DOPTIMISED -s cell_sgioptimised.make -T cell.imake -f $(VPATH)/nothing.imake; \
	else \
		imake -DIRIX -DOPTIMISED -s cell_sgioptimised.make -T $(VPATH)/cell.imake -f $(VPATH)/nothing.imake; \
	fi

#SGI 64bit version
cell64 : force cell_sgi64.make
	if [ -f cell_sgi64.make ]; then \
		make -f cell_sgi64.make; \
	else \
		make -f $(VPATH)/cell_sgi64.make; \
	fi	

cell_sgi64.make : cell.imake cell.make
	if [ -f cell.imake ]; then \
		imake -DIRIX -DO64 -DOPTIMISED -s cell_sgi64.make -T cell.imake -f $(VPATH)/nothing.imake; \
	else \
		imake -DIRIX -DO64 -DOPTIMISED -s cell_sgi64.make -T $(VPATH)/cell.imake -f $(VPATH)/nothing.imake; \
	fi

#Linux version
cell_linux : force cell_linux.make
	if [ -f cell_linux.make ]; then \
		make -f cell_linux.make; \
	else \
		make -f $(VPATH)/cell_linux.make; \
	fi

cell_linux.make : cell.imake cell.make
	if [ -f cell.imake ]; then \
		imake -DLINUX -s cell_linux.make -T cell.imake -f $(VPATH)/nothing.imake; \
	else \
		imake -DLINUX -s cell_linux.make -T $(VPATH)/cell.imake -f $(VPATH)/nothing.imake; \
	fi

update :
	if ( [ "$(PWD)" -ef "$(VPATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cvs update && \
		chgrp -R cmgui_programmers *; \
		make -f cell.make CMGUI_BASE_DIRECTORY=$(VPATH) cell cell_optimised cell64; \
		rsh 130.216.5.156 'setenv CMISS_ROOT /product/cmiss ; cd $(VPATH) ; make -f cell.make cell_linux' ; \
		chgrp -R cmgui_programmers *; \
	else \
		echo "Must be cmiss and in $(VPATH)"; \
	fi

depend : cell_sgi.make cell_sgioptimised.make cell_sgi64.make cell_linux.make
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(VPATH); \
		make -f cell_sgi.make depend ; \
		make -f cell_sgioptimised.make depend ; \
		make -f cell_sgi64.make depend ; \
		rsh 130.216.5.156 'setenv CMISS_ROOT /product/cmiss ; cd $(VPATH) ; make -f cell_linux.make depend ' ; \
	else \
		echo "Must be cmiss"; \
	fi
	

run_tests :
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(TEST_PATH); \
		make -u; \
		rsh 130.216.5.156 'setenv CMISS_ROOT /product/cmiss ; cd $(TEST_PATH) ; make -W cell_linux.exe cell_linux_test' ; \
		cat all.mail ; \
	else \
		echo "Must be cmiss"; \
	fi

cronjob :
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(VPATH); \
		echo -n > programmer.mail ; \
		if ! make -f cell.make depend; then \
			cat dependfail.mail >> programmer.mail ; \
		fi ; \
		if ! make -f cell.make update; then \
			cat updatefail.mail >> programmer.mail ; \
		fi ; \
		if ! make -f cell.make run_tests; then \
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
