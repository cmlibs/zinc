SHELL=/bin/sh
PRODUCT_PATH=$(CMISS_ROOT)/cmgui
#PRODUCT_PATH=$(HOME)/new_cell
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
		if [ -f cell.imake ]; then \
			CELL_IMAKE_FILE="-T cell.imake" ; \
		else \
			CELL_IMAKE_FILE="-T $(PRODUCT_SOURCE_PATH)/cell.imake" ; \
		fi ; \
		if [ -f common.imake ]; then \
			COMMON_IMAKE_FILE="-f common.imake" ; \
		else \
			COMMON_IMAKE_FILE="-f $(PRODUCT_SOURCE_PATH)/common.imake" ; \
		fi ; \
	else \
		CMISS_ROOT_DEF= ;\
		CELL_IMAKE_FILE="-T cell.imake" ; \
		COMMON_IMAKE_FILE="-f common.imake" ; \
	fi ;

COMMON_MAKE_RULE= \
	CMGUI_DEV_ROOT=$(PWD) ; \
	export CMGUI_DEV_ROOT ; \
	cd $(SOURCE_PATH);	

#The tags for the executables don't actually point at them (they would have to
#have $(BIN_PATH)/cell32 etc. but this forces them to get made (which is what 
#we want) and shortens the name you have to type.
#SGI debug version
cell32 : $(SOURCE_PATH)/cell_sgi.make
	$(COMMON_MAKE_RULE) \
	if [ -f cell_sgi.make ]; then \
		$(MAKE) -f cell_sgi.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cell_sgi.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/cell_sgi.make : $(SOURCE_PATH)/cell.imake $(SOURCE_PATH)/common.imake cell.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX $${CMISS_ROOT_DEF} -s cell_sgi.make $${CELL_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#SGI optimised version
cell_optimised : $(SOURCE_PATH)/cell_sgioptimised.make
	$(COMMON_MAKE_RULE) \
	if [ -f cell_sgioptimised.make ]; then \
		$(MAKE) -f cell_sgioptimised.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cell_sgioptimised.make $(TARGET) ; \
	fi	

$(SOURCE_PATH)/cell_sgioptimised.make : $(SOURCE_PATH)/cell.imake $(SOURCE_PATH)/common.imake cell.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX -DOPTIMISED $${CMISS_ROOT_DEF} -s cell_sgioptimised.make $${CELL_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#SGI debug memory check version
cell_memorycheck : $(SOURCE_PATH)/cell_sgi_memorycheck.make
	$(COMMON_MAKE_RULE) \
	if [ -f cell_sgi_memorycheck.make ]; then \
		$(MAKE) -f cell_sgi_memorycheck.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cell_sgi_memorycheck.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/cell_sgi_memorycheck.make : $(SOURCE_PATH)/cell.imake $(SOURCE_PATH)/common.imake cell.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX -DMEMORY_CHECK $${CMISS_ROOT_DEF} -s cell_sgi_memorycheck.make $${CELL_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#SGI 64bit version
cell_64 : force $(SOURCE_PATH)/cell_sgi64.make
	$(COMMON_MAKE_RULE) \
	if [ -f cell_sgi64.make ]; then \
		$(MAKE) -f cell_sgi64.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cell_sgi64.make $(TARGET) ; \
	fi	

$(SOURCE_PATH)/cell_sgi64.make : $(SOURCE_PATH)/cell.imake $(SOURCE_PATH)/common.imake cell.make
	$(COMMON_IMAKE_RULE) \
	imake -DIRIX -DO64 -DOPTIMISED $${CMISS_ROOT_DEF} -s cell_sgi64.make $${CELL_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#Linux debug version
cell_linux : force $(SOURCE_PATH)/cell_linux.make
	$(COMMON_MAKE_RULE) \
	if [ -f cell_linux.make ]; then \
		$(MAKE) -f cell_linux.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cell_linux.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/cell_linux.make : $(SOURCE_PATH)/cell.imake $(SOURCE_PATH)/common.imake cell.make
	$(COMMON_IMAKE_RULE) \
	imake -DLINUX $${CMISS_ROOT_DEF} -s cell_linux.make $${CELL_IMAKE_FILE} $${COMMON_IMAKE_FILE};

#Linux optimised version
cell_linux_optimised : force $(SOURCE_PATH)/cell_linux_optimised.make
	$(COMMON_MAKE_RULE) \
	if [ -f cell_linux_optimised.make ]; then \
		$(MAKE) -f cell_linux_optimised.make $(TARGET) ; \
	else \
		$(MAKE) -f $(PRODUCT_SOURCE_PATH)/cell_linux_optimised.make $(TARGET) ; \
	fi

$(SOURCE_PATH)/cell_linux_optimised.make : $(SOURCE_PATH)/cell.imake $(SOURCE_PATH)/common.imake cell.make
	$(COMMON_IMAKE_RULE) \
	imake -DLINUX -DOPTIMISED $${CMISS_ROOT_DEF} -s cell_linux_optimised.make $${CELL_IMAKE_FILE} $${COMMON_IMAKE_FILE};

update :
	if ( [ "$(PWD)" -ef "$(PRODUCT_PATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers * && \
		cd $(PRODUCT_PATH) && \
		$(MAKE) -f cell.make cell32 cell_optimised cell_memorycheck cell_64; \
		ssh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(PRODUCT_PATH) ; $(MAKE) -f cell.make cell_linux cell_linux_optimised' ; \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers *; \
	else \
		echo "Must be cmiss and in $(PRODUCT_PATH)"; \
	fi

depend : $(SOURCE_PATH)/cell_sgi.make $(SOURCE_PATH)/cell_sgioptimised.make $(SOURCE_PATH)/cell_sgi_memorycheck.make $(SOURCE_PATH)/cell_sgi64.make $(SOURCE_PATH)/cell_linux.make
	if [ "$(USER)" = "cmiss" ]; then \
		CMGUI_DEV_ROOT=$(PWD) ; \
		export CMGUI_DEV_ROOT ; \
		cd $(PRODUCT_SOURCE_PATH); \
		$(MAKE) -f cell_sgi.make depend  ; \
		$(MAKE) -f cell_sgioptimised.make depend  ; \
		$(MAKE) -f cell_sgi_memorycheck.make depend  ; \
		$(MAKE) -f cell_sgi64.make depend  ; \
		ssh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; setenv CMGUI_DEV_ROOT $(PWD) ; cd $(PRODUCT_SOURCE_PATH) ; $(MAKE) -f cell_linux.make depend ' ; \
	else \
		echo "Must be cmiss"; \
	fi

cronjob :
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(PRODUCT_PATH); \
		echo -n > $(MAILFILE_PATH)/cell_programmer.mail ; \
		if ! $(MAKE) -f cell.make depend; then \
			cat $(MAILFILE_PATH)/dependfail.mail >> $(MAILFILE_PATH)/cell_programmer.mail ; \
		fi ; \
		if ! $(MAKE) -f cell.make update; then \
			cat $(MAILFILE_PATH)/updatefail.mail >> $(MAILFILE_PATH)/cell_programmer.mail ; \
		fi ; \
	else \
		echo "Must be cmiss"; \
	fi
#Mail is sent attached to the example mail.

force :
	@echo "\n" > /dev/null
