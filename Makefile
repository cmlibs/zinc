SHELL=/bin/sh
PRODUCT_PATH=$(CMISS_ROOT)/cmgui
PRODUCT_SOURCE_PATH=$(PRODUCT_PATH)/source
TEST_PATH=$(CMISS_ROOT)/cmgui/test_examples
BIN_PATH=bin
SOURCE_PATH=source
MAILFILE_PATH=mailfiles
#By overriding the TARGET on the command line you can specify
#a sub object to be compiled i.e. make cmgui_linux TARGET=command/cmiss.o
TARGET= 

VPATH=$(PRODUCT_PATH)

COMMON_IMAKE_RULE= \
	cd $(SOURCE_PATH); \
	if [ ! -z $${CMISS_ROOT:=""} ] ; then \
		CMISS_ROOT_DEF=-DCMISS_ROOT_DEFINED; \
		if [ -f cmgui.imake ]; then \
			CMGUI_IMAKE_FILE="-T cmgui.imake" ; \
		else \
			CMGUI_IMAKE_FILE="-T $(PRODUCT_SOURCE_PATH)/cmgui.imake" ; \
		fi ; \
		if [ -f common.imake ]; then \
			COMMON_IMAKE_FILE="-f common.imake" ; \
		else \
			COMMON_IMAKE_FILE="-f $(PRODUCT_SOURCE_PATH)/common.imake" ; \
		fi ; \
	else \
		CMISS_ROOT_DEF= ;\
		CMGUI_IMAKE_FILE="-T cmgui.imake" ; \
		COMMON_IMAKE_FILE="-f common.imake" ; \
	fi ;

COMMON_MAKE_RULE= \
	CMGUI_DEV_ROOT=$(PWD) ; \
	export CMGUI_DEV_ROOT ; \
	cd $(SOURCE_PATH);	

#The tags for the executables don't actually point at them (they would have to
#have $(BIN_PATH)/cmgui etc. but this forces them to get made (which is what 
#we want) and shortens the name you have to type.
#SGI debug version
cmgui : force $(SOURCE_PATH)/cmgui_sgi.make
	$(COMMON_MAKE_RULE) \
	if [ -f cmgui_sgi.make ]; then \
		$(MAKE) -f cmgui_sgi.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cmgui_sgi.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/cmgui_sgi.make : $(SOURCE_PATH)/cmgui.imake $(SOURCE_PATH)/common.imake cmgui.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX $${CMISS_ROOT_DEF} -s cmgui_sgi.make $${CMGUI_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#SGI optimised version
cmgui_optimised : force $(SOURCE_PATH)/cmgui_sgioptimised.make
	$(COMMON_MAKE_RULE) \
	if [ -f cmgui_sgioptimised.make ]; then \
		$(MAKE) -f cmgui_sgioptimised.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cmgui_sgioptimised.make $(TARGET) ; \
	fi	

$(SOURCE_PATH)/cmgui_sgioptimised.make : $(SOURCE_PATH)/cmgui.imake $(SOURCE_PATH)/common.imake cmgui.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX -DOPTIMISED $${CMISS_ROOT_DEF} -s cmgui_sgioptimised.make $${CMGUI_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#SGI optimised lite version
cmgui_lite : force $(SOURCE_PATH)/cmgui_sgilite.make
	$(COMMON_MAKE_RULE) \
	if [ -f cmgui_sgilite.make ]; then \
		$(MAKE) -f cmgui_sgilite.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cmgui_sgilite.make $(TARGET) ; \
	fi	

$(SOURCE_PATH)/cmgui_sgilite.make : $(SOURCE_PATH)/cmgui.imake $(SOURCE_PATH)/common.imake cmgui.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX -DOPTIMISED -DLITEWEIGHT $${CMISS_ROOT_DEF} -s cmgui_sgilite.make $${CMGUI_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#SGI debug memory check version
cmgui_memorycheck : force $(SOURCE_PATH)/cmgui_sgi_memorycheck.make
	$(COMMON_MAKE_RULE) \
	if [ -f cmgui_sgi_memorycheck.make ]; then \
		$(MAKE) -f cmgui_sgi_memorycheck.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cmgui_sgi_memorycheck.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/cmgui_sgi_memorycheck.make : $(SOURCE_PATH)/cmgui.imake $(SOURCE_PATH)/common.imake cmgui.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX -DMEMORY_CHECK $${CMISS_ROOT_DEF} -s cmgui_sgi_memorycheck.make $${CMGUI_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#SGI 64bit version
cmgui64 : force $(SOURCE_PATH)/cmgui_sgi64.make
	$(COMMON_MAKE_RULE) \
	if [ -f cmgui_sgi64.make ]; then \
		$(MAKE) -f cmgui_sgi64.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cmgui_sgi64.make $(TARGET) ; \
	fi	

$(SOURCE_PATH)/cmgui_sgi64.make : $(SOURCE_PATH)/cmgui.imake $(SOURCE_PATH)/common.imake cmgui.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX -DO64 -DOPTIMISED $${CMISS_ROOT_DEF} -s cmgui_sgi64.make $${CMGUI_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#Linux version
cmgui_linux : force $(SOURCE_PATH)/cmgui_linux.make
	$(COMMON_MAKE_RULE) \
	if [ -f cmgui_linux.make ]; then \
		$(MAKE) -f cmgui_linux.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cmgui_linux.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/cmgui_linux.make : $(SOURCE_PATH)/cmgui.imake $(SOURCE_PATH)/common.imake cmgui.make
	$(COMMON_IMAKE_RULE) \
	imake -DLINUX $${CMISS_ROOT_DEF} -s cmgui_linux.make $${CMGUI_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#Linux debug memory check version
cmgui_linux_memorycheck : force $(SOURCE_PATH)/cmgui_linux_memorycheck.make
	$(COMMON_MAKE_RULE) \
	if [ -f cmgui_linux_memorycheck.make ]; then \
		$(MAKE) -f cmgui_linux_memorycheck.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cmgui_linux_memorycheck.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/cmgui_linux_memorycheck.make : $(SOURCE_PATH)/cmgui.imake $(SOURCE_PATH)/common.imake cmgui.make
	$(COMMON_IMAKE_RULE) \
	imake -DLINUX -DMEMORY_CHECK $${CMISS_ROOT_DEF} -s cmgui_linux_memorycheck.make $${CMGUI_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#Linux optimised version
cmgui_linux_optimised : force $(SOURCE_PATH)/cmgui_linux_optimised.make
	$(COMMON_MAKE_RULE) \
	if [ -f cmgui_linux_optimised.make ]; then \
		$(MAKE) -f cmgui_linux_optimised.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cmgui_linux_optimised.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/cmgui_linux_optimised.make : $(SOURCE_PATH)/cmgui.imake $(SOURCE_PATH)/common.imake cmgui.make
	$(COMMON_IMAKE_RULE) \
	imake -DLINUX -DOPTIMISED $${CMISS_ROOT_DEF} -s cmgui_linux_optimised.make $${CMGUI_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#Linux dynamic OpenGL version
cmgui_linux_dynamic : force $(SOURCE_PATH)/cmgui_linux_dynamic.make
	$(COMMON_MAKE_RULE) \
	if [ -f cmgui_linux_dynamic.make ]; then \
		$(MAKE) -f cmgui_linux_dynamic.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cmgui_linux_dynamic.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/cmgui_linux_dynamic.make : $(SOURCE_PATH)/cmgui.imake $(SOURCE_PATH)/common.imake cmgui.make
	$(COMMON_IMAKE_RULE) \
	imake -DLINUX -DDYNAMIC_GL_LINUX $${CMISS_ROOT_DEF} -s cmgui_linux_dynamic.make $${CMGUI_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#Linux optimised dynamic OpenGL version
cmgui_linux_optimised_dynamic : force $(SOURCE_PATH)/cmgui_linux_optimised_dynamic.make
	$(COMMON_MAKE_RULE) \
	if [ -f cmgui_linux_optimised_dynamic.make ]; then \
		$(MAKE) -f cmgui_linux_optimised_dynamic.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cmgui_linux_optimised_dynamic.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/cmgui_linux_optimised_dynamic.make : $(SOURCE_PATH)/cmgui.imake $(SOURCE_PATH)/common.imake cmgui.make
	$(COMMON_IMAKE_RULE) \
	imake -DLINUX -DOPTIMISED -DDYNAMIC_GL_LINUX $${CMISS_ROOT_DEF} -s cmgui_linux_optimised_dynamic.make $${CMGUI_IMAKE_FILE} $${COMMON_IMAKE_FILE};

update :
	if ( [ "$(PWD)" -ef "$(PRODUCT_PATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers * && \
		cd $(PRODUCT_PATH) && \
		$(MAKE) -f cmgui.make cmgui cmgui_optimised cmgui64 cmgui_lite cmgui_memorycheck && \
		ssh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(PRODUCT_PATH) ; $(MAKE) -f cmgui.make cmgui_linux cmgui_linux_memorycheck cmgui_linux_dynamic cmgui_linux_optimised cmgui_linux_optimised_dynamic' && \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers *; \
	else \
		echo "Must be cmiss and in $(PRODUCT_PATH)"; \
	fi

depend : $(SOURCE_PATH)/cmgui_sgi.make $(SOURCE_PATH)/cmgui_sgioptimised.make $(SOURCE_PATH)/cmgui_sgi64.make $(SOURCE_PATH)/cmgui_linux.make $(SOURCE_PATH)/cmgui_linux_memorycheck.make $(SOURCE_PATH)/cmgui_linux_dynamic.make $(SOURCE_PATH)/cmgui_sgi_memorycheck.make $(SOURCE_PATH)/cmgui_linux_optimised.make 
	if [ "$(USER)" = "cmiss" ]; then \
		CMGUI_DEV_ROOT=$(PWD) ; \
		export CMGUI_DEV_ROOT ; \
		cd $(PRODUCT_SOURCE_PATH); \
		rm -f *.depend ; \
		$(MAKE) -f cmgui_sgi.make depend ; \
		$(MAKE) -f cmgui_sgioptimised.make depend ; \
		$(MAKE) -f cmgui_sgi_memorycheck.make depend ; \
		$(MAKE) -f cmgui_sgilite.make depend ; \
		$(MAKE) -f cmgui_sgi64.make depend ; \
		ssh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; setenv CMGUI_DEV_ROOT $(PWD) ; cd $(PRODUCT_SOURCE_PATH) ; $(MAKE) -f cmgui_linux.make depend ; $(MAKE) -f cmgui_linux_memorycheck.make depend; $(MAKE) -f cmgui_linux_dynamic.make depend ; $(MAKE) -f cmgui_linux_optimised.make depend ; $(MAKE) -f cmgui_linux_optimised_dynamic.make depend ' ; \
	else \
		echo "Must be cmiss"; \
	fi

run_tests :
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(TEST_PATH); \
		$(MAKE) -u; \
		ssh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(TEST_PATH) ; make -W cmgui_linux.exe cmgui_linux_test' ; \
		cat all.mail ; \
	else \
		echo "Must be cmiss"; \
	fi

cronjob :
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(PRODUCT_PATH); \
		cvs update ; \
		echo -n > $(MAILFILE_PATH)/programmer.mail ; \
		if ! $(MAKE) -f cmgui.make depend; then \
			cat $(MAILFILE_PATH)/dependfail.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
		if ! $(MAKE) -f cmgui.make update; then \
			cat $(MAILFILE_PATH)/updatefail.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
		if ! $(MAKE) -f cmgui.make run_tests; then \
			cat $(MAILFILE_PATH)/testfail.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
		if [ -s $(TEST_PATH)/all.mail ]; then \
			cat $(TEST_PATH)/all.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
	else \
		echo "Must be cmiss"; \
	fi
#This mail is added into the example mail.

utilities : $(SOURCE_PATH)/cmgui_sgi.make force
	$(COMMON_MAKE_RULE) \
	if [ -f cmgui_sgi.make ]; then \
		$(MAKE) -f cmgui_sgi.make utilities; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cmgui_sgi.make utilities; \
	fi

force :
	@echo "\n" > /dev/null
