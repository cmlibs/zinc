SHELL=/bin/sh
PRODUCT_PATH=$(CMISS_ROOT)/cmgui
PRODUCT_SOURCE_PATH=$(PRODUCT_PATH)/source
TEST_PATH=$(CMISS_ROOT)/cmgui/test_examples
BIN_PATH=bin
SOURCE_PATH=source

VPATH=$(PRODUCT_PATH)

#The tags for the executables don't actually point at them (they would have to
#have $(BIN_PATH)/unemap32 etc. but this forces them to get made (which is what 
#we want) and shortens the name you have to type.
#SGI debug version
unemap32 : $(SOURCE_PATH)/unemap_sgi.make
	export CMGUI_DEV_ROOT=$(PWD) ; \
	cd $(SOURCE_PATH); \
	if [ -f unemap_sgi.make ]; then \
		make -f unemap_sgi.make ; \
	else \
		make -f $(PRODUCT_SOURCE_PATH)/unemap_sgi.make ; \
	fi

$(SOURCE_PATH)/unemap_sgi.make : $(SOURCE_PATH)/unemap.imake unemap.make
	cd $(SOURCE_PATH); \
	if [ ! -z $${CMISS_ROOT:=""} ] ; then \
		CMISS_ROOT_DEF=-DCMISS_ROOT_DEFINED; \
	fi ; \
	if [ -f unemap.imake ]; then \
		imake -DIRIX $${CMISS_ROOT_DEF} -s unemap_sgi.make -T unemap.imake -f /dev/null; \
	else \
		imake -DIRIX $${CMISS_ROOT_DEF} -s unemap_sgi.make -T $(PRODUCT_SOURCE_PATH)/unemap.imake -f /dev/null; \
	fi

#SGI rig nodes version
unemap_nodes : $(SOURCE_PATH)/unemap_sginodes.make
	export CMGUI_DEV_ROOT=$(PWD) ; \
	cd $(SOURCE_PATH); \
	if [ -f unemap_sginodes.make ]; then \
		make -f unemap_sginodes.make ; \
	else \
		make -f $(PRODUCT_SOURCE_PATH)/unemap_sginodes.make ; \
	fi

$(SOURCE_PATH)/unemap_sginodes.make : $(SOURCE_PATH)/unemap.imake unemap.make
	cd $(SOURCE_PATH); \
	if [ ! -z $${CMISS_ROOT:=""} ] ; then \
		CMISS_ROOT_DEF=-DCMISS_ROOT_DEFINED; \
	fi ; \
	if [ -f unemap.imake ]; then \
		imake -DIRIX -DUSE_UNEMAP_NODES $${CMISS_ROOT_DEF} -s unemap_sginodes.make -T unemap.imake -f /dev/null; \
	else \
		imake -DIRIX -DUSE_UNEMAP_NODES $${CMISS_ROOT_DEF} -s unemap_sginodes.make -T $(PRODUCT_SOURCE_PATH)/unemap.imake -f /dev/null; \
	fi

#SGI optimised version
unemap_optimised : $(SOURCE_PATH)/unemap_sgioptimised.make
	export CMGUI_DEV_ROOT=$(PWD) ; \
	cd $(SOURCE_PATH); \
	if [ -f unemap_sgioptimised.make ]; then \
		make -f unemap_sgioptimised.make ; \
	else \
		make -f $(PRODUCT_SOURCE_PATH)/unemap_sgioptimised.make ; \
	fi	

$(SOURCE_PATH)/unemap_sgioptimised.make : $(SOURCE_PATH)/unemap.imake unemap.make
	cd $(SOURCE_PATH); \
	if [ ! -z $${CMISS_ROOT:=""} ] ; then \
		CMISS_ROOT_DEF=-DCMISS_ROOT_DEFINED; \
	fi ; \
	if [ -f unemap.imake ]; then \
		imake -DIRIX -DOPTIMISED $${CMISS_ROOT_DEF} -s unemap_sgioptimised.make -T unemap.imake -f /dev/null; \
	else \
		imake -DIRIX -DOPTIMISED $${CMISS_ROOT_DEF} -s unemap_sgioptimised.make -T $(PRODUCT_SOURCE_PATH)/unemap.imake -f /dev/null; \
	fi

#SGI 64bit version
unemap_64 : force $(SOURCE_PATH)/unemap_sgi64.make
	export CMGUI_DEV_ROOT=$(PWD) ; \
	cd $(SOURCE_PATH); \
	if [ -f unemap_sgi64.make ]; then \
		make -f unemap_sgi64.make ; \
	else \
		make -f $(PRODUCT_SOURCE_PATH)/unemap_sgi64.make ; \
	fi	

$(SOURCE_PATH)/unemap_sgi64.make : $(SOURCE_PATH)/unemap.imake unemap.make
	cd $(SOURCE_PATH); \
	if [ ! -z $${CMISS_ROOT:=""} ] ; then \
		CMISS_ROOT_DEF=-DCMISS_ROOT_DEFINED; \
	fi ; \
	if [ -f unemap.imake ]; then \
		imake -DIRIX -DO64 -DOPTIMISED $${CMISS_ROOT_DEF} -s unemap_sgi64.make -T unemap.imake -f /dev/null; \
	else \
		imake -DIRIX -DO64 -DOPTIMISED $${CMISS_ROOT_DEF} -s unemap_sgi64.make -T $(PRODUCT_SOURCE_PATH)/unemap.imake -f /dev/null; \
	fi

#Linux version
unemap_linux : force $(SOURCE_PATH)/unemap_linux.make
	export CMGUI_DEV_ROOT=$(PWD) ; \
	cd $(SOURCE_PATH); \
	if [ -f unemap_linux.make ]; then \
		make -f unemap_linux.make ; \
	else \
		make -f $(PRODUCT_SOURCE_PATH)/unemap_linux.make ; \
	fi

$(SOURCE_PATH)/unemap_linux.make : $(SOURCE_PATH)/unemap.imake unemap.make
	cd $(SOURCE_PATH); \
	if [ ! -z $${CMISS_ROOT:=""} ] ; then \
		CMISS_ROOT_DEF=-DCMISS_ROOT_DEFINED; \
	fi ; \
	if [ -f unemap.imake ]; then \
		imake -DLINUX $${CMISS_ROOT_DEF} -s unemap_linux.make -T unemap.imake -f /dev/null; \
	else \
		imake -DLINUX $${CMISS_ROOT_DEF} -s unemap_linux.make -T $(PRODUCT_SOURCE_PATH)/unemap.imake -f /dev/null; \
	fi

update :
	if ( [ "$(PWD)" -ef "$(PRODUCT_PATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cvs update && \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers * && \
		cd $(PRODUCT_PATH) && \
		make -f unemap.make unemap32 unemap_nodes unemap_optimised unemap_64; \
		rsh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(PRODUCT_PATH) ; make -f unemap.make unemap_linux' ; \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers *; \
	else \
		echo "Must be cmiss and in $(PRODUCT_PATH)"; \
	fi

depend : $(SOURCE_PATH)/unemap_sgi.make $(SOURCE_PATH)/unemap_sginodes.make $(SOURCE_PATH)/unemap_sgioptimised.make $(SOURCE_PATH)/unemap_sgi64.make $(SOURCE_PATH)/unemap_linux.make
	if [ "$(USER)" = "cmiss" ]; then \
		export CMGUI_DEV_ROOT=$(PWD) ; \
		cd $(PRODUCT_SOURCE_PATH); \
		make -f unemap_sgi.make depend  ; \
		make -f unemap_sginodes.make depend  ; \
		make -f unemap_sgioptimised.make depend  ; \
		make -f unemap_sgi64.make depend  ; \
		rsh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; setenv CMGUI_DEV_ROOT $(PWD) ; cd $(PRODUCT_SOURCE_PATH) ; make -f unemap_linux.make depend ' ; \
	else \
		echo "Must be cmiss"; \
	fi

cronjob :
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(PRODUCT_PATH); \
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
