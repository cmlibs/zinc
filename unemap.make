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
		if [ -f unemap.imake ]; then \
			UNEMAP_IMAKE_FILE="-T unemap.imake" ; \
		else \
			UNEMAP_IMAKE_FILE="-T $(PRODUCT_SOURCE_PATH)/unemap.imake" ; \
		fi ; \
		if [ -f common.imake ]; then \
			COMMON_IMAKE_FILE="-f common.imake" ; \
		else \
			COMMON_IMAKE_FILE="-f $(PRODUCT_SOURCE_PATH)/common.imake" ; \
		fi ; \
	else \
		CMISS_ROOT_DEF= ;\
		UNEMAP_IMAKE_FILE="-T unemap.imake" ; \
		COMMON_IMAKE_FILE="-f common.imake" ; \
	fi ;

COMMON_MAKE_RULE= \
	CMGUI_DEV_ROOT=$(PWD) ; \
	export CMGUI_DEV_ROOT ; \
	cd $(SOURCE_PATH);	

#The tags for the executables don't actually point at them (they would have to
#have $(BIN_PATH)/unemap32 etc. but this forces them to get made (which is what 
#we want) and shortens the name you have to type.
#SGI debug version
unemap32 : $(SOURCE_PATH)/unemap_sgi.make
	$(COMMON_MAKE_RULE) \
	if [ -f unemap_sgi.make ]; then \
		$(MAKE) -f unemap_sgi.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/unemap_sgi.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/unemap_sgi.make : $(SOURCE_PATH)/unemap.imake $(SOURCE_PATH)/common.imake unemap.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX $${CMISS_ROOT_DEF} -s unemap_sgi.make $${UNEMAP_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#SGI rig nodes version
unemap_nodes : $(SOURCE_PATH)/unemap_sginodes.make
	$(COMMON_MAKE_RULE) \
	if [ -f unemap_sginodes.make ]; then \
		$(MAKE) -f unemap_sginodes.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/unemap_sginodes.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/unemap_sginodes.make : $(SOURCE_PATH)/unemap.imake unemap.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX -DUSE_UNEMAP_NODES $${CMISS_ROOT_DEF} -s unemap_sginodes.make $${UNEMAP_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#SGI optimised version
unemap_optimised : $(SOURCE_PATH)/unemap_sgioptimised.make
	$(COMMON_MAKE_RULE) \
	if [ -f unemap_sgioptimised.make ]; then \
		$(MAKE) -f unemap_sgioptimised.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/unemap_sgioptimised.make $(TARGET) ; \
	fi	

$(SOURCE_PATH)/unemap_sgioptimised.make : $(SOURCE_PATH)/unemap.imake unemap.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX -DOPTIMISED $${CMISS_ROOT_DEF} -s unemap_sgioptimised.make $${UNEMAP_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#SGI debug memory check version
unemap_memorycheck : $(SOURCE_PATH)/unemap_sgi_memorycheck.make
	$(COMMON_MAKE_RULE) \
	if [ -f unemap_sgi_memorycheck.make ]; then \
		$(MAKE) -f unemap_sgi_memorycheck.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/unemap_sgi_memorycheck.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/unemap_sgi_memorycheck.make : $(SOURCE_PATH)/unemap.imake $(SOURCE_PATH)/common.imake unemap.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX -DMEMORY_CHECK $${CMISS_ROOT_DEF} -s unemap_sgi_memorycheck.make $${UNEMAP_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#SGI 64bit version
unemap_64 : force $(SOURCE_PATH)/unemap_sgi64.make
	$(COMMON_MAKE_RULE) \
	if [ -f unemap_sgi64.make ]; then \
		$(MAKE) -f unemap_sgi64.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/unemap_sgi64.make $(TARGET) ; \
	fi	

$(SOURCE_PATH)/unemap_sgi64.make : $(SOURCE_PATH)/unemap.imake unemap.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX -DO64 -DOPTIMISED $${CMISS_ROOT_DEF} -s unemap_sgi64.make $${UNEMAP_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#Linux version
unemap_linux : force $(SOURCE_PATH)/unemap_linux.make
	$(COMMON_MAKE_RULE) \
	if [ -f unemap_linux.make ]; then \
		$(MAKE) -f unemap_linux.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/unemap_linux.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/unemap_linux.make : $(SOURCE_PATH)/unemap.imake unemap.make
	$(COMMON_IMAKE_RULE) \
	imake -DLINUX $${CMISS_ROOT_DEF} -s unemap_linux.make $${UNEMAP_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#Linux nodes version
unemap_linux_nodes : force $(SOURCE_PATH)/unemap_linux_nodes.make
	$(COMMON_MAKE_RULE) \
	if [ -f unemap_linux_nodes.make ]; then \
		$(MAKE) -f unemap_linux_nodes.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/unemap_linux_nodes.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/unemap_linux_nodes.make : $(SOURCE_PATH)/unemap.imake unemap.make
	$(COMMON_IMAKE_RULE) \
	imake -DLINUX  -DUSE_UNEMAP_NODES $${CMISS_ROOT_DEF} -s unemap_linux_nodes.make $${UNEMAP_IMAKE_FILE} $${COMMON_IMAKE_FILE};

update :
	if ( [ "$(PWD)" -ef "$(PRODUCT_PATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cvs update && \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers * && \
		cd $(PRODUCT_PATH) && \
		$(MAKE) -f unemap.make unemap32 unemap_nodes unemap_optimised unemap_64 unemap_memorycheck utilities; \
		ssh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(PRODUCT_PATH) ; $(MAKE) -f unemap.make unemap_linux unemap_linux_nodes utilities_linux' ; \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers *; \
	else \
		echo "Must be cmiss and in $(PRODUCT_PATH)"; \
	fi

depend : $(SOURCE_PATH)/unemap_sgi.make $(SOURCE_PATH)/unemap_sginodes.make $(SOURCE_PATH)/unemap_sgioptimised.make $(SOURCE_PATH)/unemap_sgi64.make $(SOURCE_PATH)/unemap_linux.make $(SOURCE_PATH)/unemap_linux_nodes.make
	if [ "$(USER)" = "cmiss" ]; then \
		CMGUI_DEV_ROOT=$(PWD) ; \
		export CMGUI_DEV_ROOT ; \
		cd $(PRODUCT_SOURCE_PATH); \
		$(MAKE) -f unemap_sgi.make depend  ; \
		$(MAKE) -f unemap_sginodes.make depend  ; \
		$(MAKE) -f unemap_sgioptimised.make depend  ; \
		$(MAKE) -f unemap_sgi64.make depend  ; \
		ssh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; setenv CMGUI_DEV_ROOT $(PWD) ; cd $(PRODUCT_SOURCE_PATH) ; $(MAKE) -f unemap_linux.make depend ; $(MAKE) -f unemap_linux_nodes.make depend ' ; \
	else \
		echo "Must be cmiss"; \
	fi

cronjob :
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(PRODUCT_PATH); \
		echo -n > $(MAILFILE_PATH)/unemap_programmer.mail ; \
		if ! $(MAKE) -f unemap.make depend; then \
			cat $(MAILFILE_PATH)/dependfail.mail >> $(MAILFILE_PATH)/unemap_programmer.mail ; \
		fi ; \
		if ! $(MAKE) -f unemap.make update; then \
			cat $(MAILFILE_PATH)/updatefail.mail >> $(MAILFILE_PATH)/unemap_programmer.mail ; \
		fi ; \
		if [ -s $(MAILFILE_PATH)/unemap_programmer.mail ]; then \
			cat $(MAILFILE_PATH)/unemap_header.mail $(MAILFILE_PATH)/unemap_programmer.mail | sed "s/DATE/`date`/" | mail cmguiprogrammers@esu1.auckland.ac.nz ; \
		else \
			cat $(MAILFILE_PATH)/unemap_success.mail | sed "s/DATE/`date`/" | mail s.blackett@auckland.ac.nz ; \
		fi; \
	else \
		echo "Must be cmiss"; \
	fi

utilities : $(SOURCE_PATH)/unemap_sgi.make force
	$(COMMON_MAKE_RULE) \
	if [ -f unemap_sgi.make ]; then \
		$(MAKE) -f unemap_sgi.make utilities; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/unemap_sgi.make utilities; \
	fi

utilities_linux : $(SOURCE_PATH)/unemap_linux.make force
	$(COMMON_MAKE_RULE) \
	if [ -f unemap_linux.make ]; then \
		$(MAKE) -f unemap_linux.make utilities; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/unemap_linux.make utilities; \
	fi

force :
	@echo "\n" > /dev/null
