#Paths
PRODUCT_PATH=$(CMISS_ROOT)/cmgui
PRODUCT_SOURCE_PATH=$(PRODUCT_PATH)/source
MAILFILE_PATH=mailfiles

#Build defaults
USER_INTERFACE=USER_INTERFACE=MOTIF_USER_INTERFACE
DEBUG=DEBUG=true
ABI=
MEMORYCHECK=
USE_UNEMAP_NODES=
USE_UNEMAP_3D=

ifdef CMISS_ROOT
   CMISS_ROOT_DEFINED = true
else # CMISS_ROOT
   CMISS_ROOT = $(CMGUI_DEV_ROOT)
endif # CMISS_ROOT

MAKEFILE = unemap.make
SUBMAKEFILE := unemap.Makefile
SUBMAKEFILE_FOUND = $(wildcard source/$(SUBMAKEFILE))
ifdef CMISS_ROOT_DEFINED
   ifeq ($(SUBMAKEFILE_FOUND),)
      SUBMAKEFILE := $(CMISS_ROOT)/cmgui/source/$(SUBMAKEFILE)
   endif # $(SUBMAKEFILE_FOUND) ==
endif # CMISS_ROOT_DEFINED

#Developers default
unemap-debug :

unemap unemap-debug unemap-debug-memorycheck unemap-nodes unemap-3d unemap-3d-debug unemap64 unemap64-debug : USER_INTERFACE=USER_INTERFACE=MOTIF_USER_INTERFACE
unemap unemap-nodes unemap-3d unemap64 : DEBUG=DEBUG=false
unemap-debug unemap-3d-debug unemap64-debug : DEBUG=DEBUG=true
unemap64 unemap64-debug : ABI=ABI=64
unemap-debug-memorycheck : MEMORYCHECK=MEMORYCHECK=true
unemap-nodes : USE_UNEMAP_NODES=USE_UNEMAP_NODES=true
unemap-nodes unemap-3d : USE_UNEMAP_3D=USE_UNEMAP_3D=true

utilities: TARGET=utilities

unemap unemap-debug unemap-debug-memorycheck unemap-nodes unemap-3d unemap-3d-debug unemap64 utilities :
	cd source ; \
	$(MAKE) -f $(SUBMAKEFILE) $(TARGET) $(USER_INTERFACE) $(ABI) $(DEBUG) $(MEMORYCHECK) $(USE_UNEMAP_NODES) $(USE_UNEMAP_3D) ;

update_sources :
	if ( [ "$(PWD)" -ef "$(PRODUCT_PATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cvs update && \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers * && \
		cd $(PRODUCT_PATH) && \
		ssh 130.216.191.92 'export CVS_RSH=ssh; export CVS_SERVER=/product/cmiss/bin/cvs ; cd $(CMISS_ROOT)/cmgui ; $(CMISS_ROOT)/bin/cvs update ' ; \
	else \
		echo "Must be cmiss and in $(PRODUCT_PATH)"; \
	fi

ESU_BUILD_LIST = unemap unemap-debug unemap-debug-memorycheck unemap64 unemap64-debug unemap-nodes unemap-3d unemap-3d-debug utilities
ESP_BUILD_LIST = unemap unemap-debug unemap-debug-memorycheck unemap-3d unemap-3d-debug utilities
HPC1_BUILD_LIST = unemap unemap-debug unemap64 unemap64-debug unemap-3d

update : update_sources
	if ( [ "$(PWD)" -ef "$(PRODUCT_PATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers * && \
		cd $(PRODUCT_PATH) && \
		$(MAKE) -f $(MAKEFILE) $(ESU_BUILD_LIST) && \
		ssh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(PRODUCT_PATH) ; $(MAKE) -f $(MAKEFILE) $(ESP_BUILD_LIST)' && \
		ssh 130.216.191.92 'export CMISS_ROOT=/product/cmiss ; export CMGUI_DEV_ROOT=$(PWD) ; cd $(CMISS_ROOT)/cmgui ; gmake -f  $(MAKEFILE) $(HPC1_BUILD_LIST) ;  ' && \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers *; \
	else \
		echo "Must be cmiss and in $(PRODUCT_PATH)"; \
	fi

depend: update_sources
	if [ "$(USER)" = "cmiss" ]; then \
		CMGUI_DEV_ROOT=$(PWD) ; \
		export CMGUI_DEV_ROOT ; \
		$(MAKE) -f $(MAKEFILE) $(ESU_BUILD_LIST) TARGET=depend ; \
		ssh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(PRODUCT_PATH) ; setenv CMGUI_DEV_ROOT $(PWD) ; $(MAKE) -f $(MAKEFILE) $(ESP_BUILD_LIST) TARGET=depend ; ' ; \
		ssh 130.216.191.92 'export CMISS_ROOT=/product/cmiss ; export CMGUI_DEV_ROOT=$(PWD) ; cd $(CMISS_ROOT)/cmgui ; gmake -f $(MAKEFILE) $(HPC1_BUILD_LIST) TARGET=depend ;  ' ; \
	else \
		echo "Must be cmiss"; \
	fi

cronjob:
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(PRODUCT_PATH); \
		cvs update ; \
		echo -n > $(MAILFILE_PATH)/programmer.mail ; \
		if ! $(MAKE) -f $(MAKEFILE) depend; then \
			cat $(MAILFILE_PATH)/dependfail.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
		if ! $(MAKE) -f $(MAKEFILE) update; then \
			cat $(MAILFILE_PATH)/updatefail.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
		if [ -s $(TEST_PATH)/all.mail ]; then \
			cat $(TEST_PATH)/all.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
	else \
		echo "Must be cmiss"; \
	fi
#This mail is added into the example mail.

