#Paths
PRODUCT_PATH=$(CMISS_ROOT)/cmgui
PRODUCT_SOURCE_PATH=$(PRODUCT_PATH)/source
MAILFILE_PATH=mailfiles

#Build defaults
USER_INTERFACE=USER_INTERFACE=MOTIF_USER_INTERFACE
DEBUG=DEBUG=true
ABI=
MEMORYCHECK=

ifdef CMISS_ROOT
   CMISS_ROOT_DEFINED = true
else # CMISS_ROOT
   CMISS_ROOT = $(CMGUI_DEV_ROOT)
endif # CMISS_ROOT

MAKEFILE = cell.make
SUBMAKEFILE := cell.Makefile
SUBMAKEFILE_FOUND = $(wildcard source/$(SUBMAKEFILE))
ifdef CMISS_ROOT_DEFINED
   ifeq ($(SUBMAKEFILE_FOUND),)
      SUBMAKEFILE := $(CMISS_ROOT)/cmgui/source/$(SUBMAKEFILE)
   endif # $(SUBMAKEFILE_FOUND) ==
endif # CMISS_ROOT_DEFINED

#Developers default
cell-debug :

cell cell-debug cell-debug-memorycheck cell64 cell64-debug : USER_INTERFACE=USER_INTERFACE=MOTIF_USER_INTERFACE
cell cell64 : DEBUG=DEBUG=false
cell-debug cell-debug-memorycheck cell64-debug : DEBUG=DEBUG=true
cell64 cell64-debug : ABI=ABI=64
cell-debug-memorycheck : MEMORYCHECK=MEMORYCHECK=true

utilities: TARGET=utilities
utilities: force

cell cell-debug cell-debug-memorycheck cell64 cell64-debug utilities :
	cd source ; \
	$(MAKE) -f $(SUBMAKEFILE) $(TARGET) $(USER_INTERFACE) $(ABI) $(DEBUG) $(MEMORYCHECK) ;

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

ESU_BUILD_LIST = cell cell-debug cell-debug-memorycheck cell64 utilities
ESP_BUILD_LIST = cell cell-debug cell-debug-memorycheck utilities
HPC1_BUILD_LIST = cell cell-debug cell64 

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
		echo -n > $(MAILFILE_PATH)/cell_programmer.mail ; \
		if ! $(MAKE) -f $(MAKEFILE) depend; then \
			cat $(MAILFILE_PATH)/dependfail.mail >> $(MAILFILE_PATH)/cell_programmer.mail ; \
		fi ; \
		if ! $(MAKE) -f $(MAKEFILE) update; then \
			cat $(MAILFILE_PATH)/updatefail.mail >> $(MAILFILE_PATH)/cell_programmer.mail ; \
		fi ; \
	else \
		echo "Must be cmiss"; \
	fi
#This mail is added into the example mail.

force :
	@echo "\n" > /dev/null
