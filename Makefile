SHELL=/bin/sh
PRODUCT_PATH=$(CMISS_ROOT)/cmgui
PRODUCT_SOURCE_PATH=$(PRODUCT_PATH)/source
TEST_PATH=$(CMISS_ROOT)/cmgui/test_examples
BIN_PATH=bin
SOURCE_PATH=source
MAILFILE_PATH=mailfiles

VPATH=$(PRODUCT_PATH)

#The tags for the executables don't actually point at them (they would have to
#have $(BIN_PATH)/cmgui etc. but this forces them to get made (which is what 
#we want) and shortens the name you have to type.
#SGI debug version
cmgui : force $(SOURCE_PATH)/cmgui_sgi.make
	export CMGUI_DEV_ROOT=$(PWD) ; \
	cd $(SOURCE_PATH); \
	if [ -f cmgui_sgi.make ]; then \
		make -f cmgui_sgi.make ; \
	else \
		make -f $(PRODUCT_SOURCE_PATH)/cmgui_sgi.make ; \
	fi

$(SOURCE_PATH)/cmgui_sgi.make : $(SOURCE_PATH)/cmgui.imake cmgui.make
	cd $(SOURCE_PATH); \
	if [ ! -z $${CMISS_ROOT:=""} ] ; then \
		CMISS_ROOT_DEF=-DCMISS_ROOT_DEFINED; \
	fi ; \
	if [ -f cmgui.imake ]; then \
		imake -DIRIX $${CMISS_ROOT_DEF} -s cmgui_sgi.make -T cmgui.imake -f /dev/null; \
	else \
		imake -DIRIX $${CMISS_ROOT_DEF} -s cmgui_sgi.make -T $(PRODUCT_SOURCE_PATH)/cmgui.imake -f /dev/null; \
	fi

#SGI optimised version
cmgui_optimised : force $(SOURCE_PATH)/cmgui_sgioptimised.make
	export CMGUI_DEV_ROOT=$(PWD) ; \
	cd $(SOURCE_PATH); \
	if [ -f cmgui_sgioptimised.make ]; then \
		make -f cmgui_sgioptimised.make ; \
	else \
		make -f $(PRODUCT_SOURCE_PATH)/cmgui_sgioptimised.make ; \
	fi	

$(SOURCE_PATH)/cmgui_sgioptimised.make : $(SOURCE_PATH)/cmgui.imake cmgui.make
	cd $(SOURCE_PATH); \
	if [ ! -z $${CMISS_ROOT:=""} ] ; then \
		CMISS_ROOT_DEF=-DCMISS_ROOT_DEFINED; \
	fi ; \
	if [ -f cmgui.imake ]; then \
		imake -DIRIX -DOPTIMISED $${CMISS_ROOT_DEF} -s cmgui_sgioptimised.make -T cmgui.imake -f /dev/null; \
	else \
		imake -DIRIX -DOPTIMISED $${CMISS_ROOT_DEF} -s cmgui_sgioptimised.make -T $(PRODUCT_SOURCE_PATH)/cmgui.imake -f /dev/null; \
	fi

#SGI optimised lite version
cmgui_lite : force $(SOURCE_PATH)/cmgui_sgilite.make
	export CMGUI_DEV_ROOT=$(PWD) ; \
	cd $(SOURCE_PATH); \
	if [ -f cmgui_sgilite.make ]; then \
		make -f cmgui_sgilite.make ; \
	else \
		make -f $(PRODUCT_SOURCE_PATH)/cmgui_sgilite.make ; \
	fi	

$(SOURCE_PATH)/cmgui_sgilite.make : $(SOURCE_PATH)/cmgui.imake cmgui.make
	cd $(SOURCE_PATH); \
	if [ ! -z $${CMISS_ROOT:=""} ] ; then \
		CMISS_ROOT_DEF=-DCMISS_ROOT_DEFINED; \
	fi ; \
	if [ -f cmgui.imake ]; then \
		imake -DIRIX -DOPTIMISED -DLITEWEIGHT $${CMISS_ROOT_DEF} -s cmgui_sgilite.make -T cmgui.imake -f /dev/null; \
	else \
		imake -DIRIX -DOPTIMISED -DLITEWEIGHT $${CMISS_ROOT_DEF} -s cmgui_sgilite.make -T $(PRODUCT_SOURCE_PATH)/cmgui.imake -f /dev/null; \
	fi

#SGI debug memory check version
cmgui_memorycheck : force $(SOURCE_PATH)/cmgui_sgi_memorycheck.make
	export CMGUI_DEV_ROOT=$(PWD) ; \
	cd $(SOURCE_PATH); \
	if [ -f cmgui_sgi_memorycheck.make ]; then \
		make -f cmgui_sgi_memorycheck.make ; \
	else \
		make -f $(PRODUCT_SOURCE_PATH)/cmgui_sgi_memorycheck.make ; \
	fi

$(SOURCE_PATH)/cmgui_sgi_memorycheck.make : $(SOURCE_PATH)/cmgui.imake cmgui.make
	cd $(SOURCE_PATH); \
	if [ -f cmgui.imake ]; then \
		imake -DIRIX -DMEMORY_CHECK $${CMISS_ROOT_DEF} -s cmgui_sgi_memorycheck.make -T cmgui.imake -f /dev/null; \
	else \
		imake -DIRIX -DMEMORY_CHECK $${CMISS_ROOT_DEF} -s cmgui_sgi_memorycheck.make -T $(PRODUCT_SOURCE_PATH)/cmgui.imake -f /dev/null; \
	fi

#SGI 64bit version
cmgui64 : force $(SOURCE_PATH)/cmgui_sgi64.make
	export CMGUI_DEV_ROOT=$(PWD) ; \
	cd $(SOURCE_PATH); \
	if [ -f cmgui_sgi64.make ]; then \
		make -f cmgui_sgi64.make ; \
	else \
		make -f $(PRODUCT_SOURCE_PATH)/cmgui_sgi64.make ; \
	fi	

$(SOURCE_PATH)/cmgui_sgi64.make : $(SOURCE_PATH)/cmgui.imake cmgui.make
	cd $(SOURCE_PATH); \
	if [ ! -z $${CMISS_ROOT:=""} ] ; then \
		CMISS_ROOT_DEF=-DCMISS_ROOT_DEFINED; \
	fi ; \
	if [ -f cmgui.imake ]; then \
		imake -DIRIX -DO64 -DOPTIMISED $${CMISS_ROOT_DEF} -s cmgui_sgi64.make -T cmgui.imake -f /dev/null; \
	else \
		imake -DIRIX -DO64 -DOPTIMISED $${CMISS_ROOT_DEF} -s cmgui_sgi64.make -T $(PRODUCT_SOURCE_PATH)/cmgui.imake -f /dev/null; \
	fi

#Linux version
cmgui_linux : force $(SOURCE_PATH)/cmgui_linux.make
	export CMGUI_DEV_ROOT=$(PWD) ; \
	cd $(SOURCE_PATH); \
	if [ -f cmgui_linux.make ]; then \
		make -f cmgui_linux.make ; \
	else \
		make -f $(PRODUCT_SOURCE_PATH)/cmgui_linux.make ; \
	fi

$(SOURCE_PATH)/cmgui_linux.make : $(SOURCE_PATH)/cmgui.imake cmgui.make
	cd $(SOURCE_PATH); \
	if [ ! -z $${CMISS_ROOT:=""} ] ; then \
		CMISS_ROOT_DEF=-DCMISS_ROOT_DEFINED; \
	fi ; \
	if [ -f cmgui.imake ]; then \
		imake -DLINUX $${CMISS_ROOT_DEF} -s cmgui_linux.make -T cmgui.imake -f /dev/null; \
	else \
		imake -DLINUX $${CMISS_ROOT_DEF} -s cmgui_linux.make -T $(PRODUCT_SOURCE_PATH)/cmgui.imake -f /dev/null; \
	fi

update :
	if ( [ "$(PWD)" -ef "$(PRODUCT_PATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cvs update && \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers * && \
		cd $(PRODUCT_PATH) && \
		make -f cmgui.make cmgui cmgui_optimised cmgui64 cmgui_lite cmgui_memorycheck && \
		rsh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(PRODUCT_PATH) ; make -f cmgui.make cmgui_linux' && \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers *; \
	else \
		echo "Must be cmiss and in $(PRODUCT_PATH)"; \
	fi

depend : $(SOURCE_PATH)/cmgui_sgi.make $(SOURCE_PATH)/cmgui_sgioptimised.make $(SOURCE_PATH)/cmgui_sgi64.make $(SOURCE_PATH)/cmgui_linux.make $(SOURCE_PATH)/cmgui_sgi_memorycheck.make
	if [ "$(USER)" = "cmiss" ]; then \
		export CMGUI_DEV_ROOT=$(PWD) ; \
		cd $(PRODUCT_SOURCE_PATH); \
		make -f cmgui_sgi.make depend ; \
		make -f cmgui_sgioptimised.make depend ; \
		make -f cmgui_sgi_memorycheck.make depend ; \
		make -f cmgui_sgi64.make depend ; \
		rsh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; setenv CMGUI_DEV_ROOT $(PWD) ; cd $(PRODUCT_SOURCE_PATH) ; make -f cmgui_linux.make depend ' ; \
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
		cd $(PRODUCT_PATH); \
		echo -n > $(MAILFILE_PATH)/programmer.mail ; \
		if ! make -f cmgui.make depend; then \
			cat $(MAILFILE_PATH)/dependfail.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
		if ! make -f cmgui.make update; then \
			cat $(MAILFILE_PATH)/updatefail.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
		if ! make -f cmgui.make run_tests; then \
			cat $(MAILFILE_PATH)/testfail.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
		if [ -s $(TEST_PATH)/all.mail ]; then \
			cat $(TEST_PATH)/all.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
		if [ -s $(MAILFILE_PATH)/programmer.mail ]; then \
			cat $(MAILFILE_PATH)/header.mail $(MAILFILE_PATH)/programmer.mail | sed "s/DATE/`date`/" | mail cmguiprogrammers@esu1.auckland.ac.nz ; \
		else \
			cat $(MAILFILE_PATH)/success.mail | sed "s/DATE/`date`/" | mail s.blackett@auckland.ac.nz ; \
		fi; \
	else \
		echo "Must be cmiss"; \
	fi

nothing.imake :
	echo "\n" > nothing.imake

force :
	@echo "\n" > /dev/null
