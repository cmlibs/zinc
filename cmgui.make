#Paths
PRODUCT_PATH=$(CMISS_ROOT)/cmgui
PRODUCT_SOURCE_PATH=$(PRODUCT_PATH)/source
TEST_PATH=$(CMISS_ROOT)/cmgui/test_examples
MAILFILE_PATH=mailfiles

#Build defaults
USER_INTERFACE=USER_INTERFACE=MOTIF_USER_INTERFACE
DYNAMIC_GL_LINUX=DYNAMIC_GL_LINUX=false
DEBUG=DEBUG=true
ABI=
MEMORYCHECK=

ifdef CMISS_ROOT
   CMISS_ROOT_DEFINED = true
else # CMISS_ROOT
   CMISS_ROOT = $(CMGUI_DEV_ROOT)
endif # CMISS_ROOT

MAKEFILE = cmgui.make
SUBMAKEFILE := cmgui.Makefile
SUBMAKEFILE_FOUND = $(wildcard source/$(SUBMAKEFILE))
ifdef CMISS_ROOT_DEFINED
   ifeq ($(SUBMAKEFILE_FOUND),)
      SUBMAKEFILE := $(CMISS_ROOT)/cmgui/source/$(SUBMAKEFILE)
   endif # $(SUBMAKEFILE_FOUND) ==
endif # CMISS_ROOT_DEFINED

#Developers default
cmgui-debug :

cmgui cmgui-debug cmgui-dynamicgl cmgui-dynamicgl-debug cmgui64 cmgui64-debug : USER_INTERFACE=USER_INTERFACE=MOTIF_USER_INTERFACE
cmgui-dynamicgl cmgui-dynamicgl-debug : DYNAMIC_GL_LINUX=DYNAMIC_GL_LINUX=true
cmgui cmgui-dynamicgl cmgui64 cmgui-console cmgui-gtk : DEBUG=DEBUG=false
cmgui-debug cmgui-debug-memorycheck cmgui-dynamicgl-debug cmgui64-debug : DEBUG=DEBUG=true
cmgui64 cmgui64-debug : ABI=ABI=64
cmgui-debug-memorycheck : MEMORYCHECK=MEMORYCHECK=true
cmgui-console : USER_INTERFACE=USER_INTERFACE=CONSOLE_USER_INTERFACE
cmgui-gtk : USER_INTERFACE=USER_INTERFACE=GTK_USER_INTERFACE

utilities: TARGET=utilities
utilities: force

cmgui cmgui-debug cmgui-debug-memorycheck cmgui-dynamicgl cmgui-dynamicgl-debug cmgui64 cmgui64-debug cmgui-console cmgui-gtk utilities :
	cd source ; \
	$(MAKE) -f $(SUBMAKEFILE) $(TARGET) $(USER_INTERFACE) $(ABI) $(DYNAMIC_GL_LINUX) $(DEBUG) $(MEMORYCHECK) ;

update_sources :
	if ( [ "$(PWD)" -ef "$(PRODUCT_PATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cvs update && \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers * && \
		cd $(PRODUCT_PATH) && \
		ssh 130.216.191.92 'export CVS_RSH=ssh; cd $(CMISS_ROOT)/cmgui ; $(CMISS_ROOT)/bin/cvs update ' ; \
	else \
		echo "Must be cmiss and in $(PRODUCT_PATH)"; \
	fi

ESU_BUILD_LIST = cmgui cmgui-debug cmgui64 cmgui-console cmgui-debug-memorycheck
ESP_BUILD_LIST = cmgui cmgui-debug cmgui-debug-memorycheck cmgui-dynamicgl cmgui-dynamicgl-debug cmgui-console
HPC1_BUILD_LIST = cmgui cmgui-debug cmgui64 cmgui64-debug

update : update_sources
	if ( [ "$(PWD)" -ef "$(PRODUCT_PATH)" ] && [ "$(USER)" = "cmiss" ] ); then \
		cd $(PRODUCT_SOURCE_PATH) && \
		chgrp -R cmgui_programmers * && \
		cd $(PRODUCT_PATH) && \
		$(MAKE) -f $(MAKEFILE) $(ESU_BUILD_LIST) && \
		ssh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(PRODUCT_PATH) ; $(MAKE) -f $(MAKEFILE) $(ESP_BUILD_LIST)' && \
		ssh 130.216.191.92 'export CMISS_ROOT=/product/cmiss ; export CMGUI_DEV_ROOT=$(PWD) ; cd $(CMISS_ROOT)/cmgui ; gmake -f  $(MAKEFILE) $(HPC1_BUILD_LIST) ;  ' && \
		ssh 130.216.209.167 'setenv CMISS_ROOT /product/cmiss ; cd $(PRODUCT_PATH) ; $(MAKE) -f $(MAKEFILE) cmgui-gtk && /home/blackett/bin/cross-make -f $(MAKEFILE) cmgui-gtk' && \
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
		ssh 130.216.209.167 'setenv CMISS_ROOT /product/cmiss ; setenv CMGUI_DEV_ROOT $(PWD) ; cd $(PRODUCT_PATH) ; $(MAKE) -f $(MAKEFILE) cmgui-gtk TARGET=depend && /home/blackett/bin/cross-make -f $(MAKEFILE) cmgui-gtk TARGET=depend' ; \
	else \
		echo "Must be cmiss"; \
	fi

run_tests:
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(TEST_PATH); \
		$(MAKE) ; \
		ssh 130.216.208.156 'setenv CMISS_ROOT /product/cmiss ; cd $(TEST_PATH) ; make cmgui-linux-test' ; \
		cat all.mail ; \
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
		if ! $(MAKE) -f $(MAKEFILE) run_tests; then \
			cat $(MAILFILE_PATH)/testfail.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
		if [ -s $(TEST_PATH)/all.mail ]; then \
			cat $(TEST_PATH)/all.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
	else \
		echo "Must be cmiss"; \
	fi
#This mail is added into the example mail.

force :
	@echo "\n" > /dev/null
