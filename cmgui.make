#Paths
PRODUCT_PATH=$(CMISS_ROOT)/cmgui
PRODUCT_SOURCE_PATH=$(PRODUCT_PATH)/source
TEST_PATH=$(CMISS_ROOT)/cmgui/test_examples
MAILFILE_PATH=mailfiles

#Build defaults
USER_INTERFACE=MOTIF_USER_INTERFACE
STATIC_LINK=false
DEBUG=true
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

#Separating these rules allow the command line options to propogate and
#variables that are not defined not to propogate.
cmgui cmgui-debug cmgui-static cmgui-static-debug cmgui64 cmgui64-debug : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui cmgui-debug cmgui-static cmgui-static-debug cmgui64 cmgui64-debug : USER_INTERFACE=MOTIF_USER_INTERFACE
cmgui-static cmgui-static-debug : STATIC_LINK_OPTION=STATIC_LINK=$(STATIC_LINK)
cmgui-static cmgui-static-debug : STATIC_LINK=true
cmgui cmgui-static cmgui64 cmgui-console cmgui-gtk : DEBUG_OPTION=DEBUG=$(DEBUG)
cmgui cmgui-static cmgui64 cmgui-console cmgui-gtk : DEBUG=false
cmgui-debug cmgui-debug-memorycheck cmgui-static-debug cmgui64-debug : DEBUG_OPTION=DEBUG=$(DEBUG)
cmgui-debug cmgui-debug-memorycheck cmgui-static-debug cmgui64-debug : DEBUG=true
cmgui64 cmgui64-debug utilities64 : ABI_OPTION=ABI=$(ABI)
cmgui64 cmgui64-debug utilities64 : ABI=64
cmgui-debug-memorycheck : MEMORYCHECK_OPTION=MEMORYCHECK=$(MEMORYCHECK)
cmgui-debug-memorycheck : MEMORYCHECK=true
cmgui-console : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui-console : USER_INTERFACE=CONSOLE_USER_INTERFACE
cmgui-gtk : USER_INTERFACE_OPTION=USER_INTERFACE=$(USER_INTERFACE)
cmgui-gtk : USER_INTERFACE=GTK_USER_INTERFACE

utilities utilities64 : TARGET_OPTION=utilities
utilities utilities64 : force

ifdef TARGET
   TARGET_OPTION = $(TARGET)
endif
ifdef USER_INTERFACE
   USER_INTERFACE_OPTION = USER_INTERFACE=$(USER_INTERFACE)
endif
ifdef STATIC_LINK
   STATIC_LINK_OPTION = STATIC_LINK=$(STATIC_LINK)
endif
ifdef DEBUG
   DEBUG_OPTION = DEBUG=$(DEBUG)
endif
ifdef ABI
   ABI_OPTION = ABI=$(ABI)
endif
ifdef MEMORYCHECK
   MEMORYCHECK_OPTION = MEMORYCHECK=$(MEMORYCHECK)
endif
OPTIONS = $(TARGET_OPTION) $(USER_INTERFACE_OPTION) $(STATIC_LINK_OPTION) $(DEBUG_OPTION) $(ABI_OPTION) $(MEMORYCHECK_OPTION)

cmgui cmgui-debug cmgui-debug-memorycheck cmgui-static cmgui-static-debug cmgui64 cmgui64-debug cmgui-console cmgui-gtk utilities :
	cd source ; \
	$(MAKE) -f $(SUBMAKEFILE) $(OPTIONS) ;

ESU_BUILD_LIST = cmgui cmgui-debug cmgui64 cmgui-console cmgui-debug-memorycheck
ESU_BUILD_PATH = '$${CMISS_ROOT}/cmgui'
ESU_BUILD_MACHINE = 130.216.208.35 #esu35
ESP_BUILD_LIST = cmgui cmgui-debug cmgui-debug-memorycheck cmgui-static cmgui-static-debug cmgui-console
ESP_BUILD_PATH = '$${CMISS_ROOT}/cmgui'
ESP_BUILD_MACHINE = 130.216.208.156 #esp56
HPC1_BUILD_LIST = cmgui cmgui-debug cmgui64 cmgui64-debug
HPC1_BUILD_PATH = '$${CMISS_ROOT}/cmgui'
HPC1_BUILD_MACHINE = 130.216.191.92 #hpc1


update_sources :
	ssh cmiss@$(ESU_BUILD_MACHINE) 'cd $(ESU_BUILD_PATH)/source ; cvs update' && \
	ssh cmiss@$(HPC1_BUILD_MACHINE) 'cd $(HPC1_BUILD_PATH)/source ; cvs update' ;

#If not already cmiss become cmiss first and then propogate so that the user
#only has to authenticate as cmiss once at the start.
update :
	if [ "$(USER)" = "cmiss" ] ; then \
		ssh cmiss@$(ESU_BUILD_MACHINE) 'cd $(ESU_BUILD_PATH) ; $(MAKE) -f $(MAKEFILE) $(ESU_BUILD_LIST)' && \
		ssh cmiss@$(ESP_BUILD_MACHINE) 'cd $(ESP_BUILD_PATH) ; $(MAKE) -f $(MAKEFILE) $(ESP_BUILD_LIST)' && \
		ssh cmiss@$(HPC1_BUILD_MACHINE) 'cd $(HPC1_BUILD_PATH) ; $(MAKE) -f $(MAKEFILE) $(HPC1_BUILD_LIST)' && \
		ssh cmiss@130.216.209.167 'setenv CMISS_ROOT /product/cmiss ; cd $(PRODUCT_PATH) ; $(MAKE) -f $(MAKEFILE) cmgui-gtk && /home/blackett/bin/cross-make -f $(MAKEFILE) cmgui-gtk' ; \
	else \
		ssh cmiss@$(ESU_BUILD_MACHINE) 'cd $(ESU_BUILD_PATH) ; $(MAKE) -f $(MAKEFILE) update' ; \
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

cronjob: update_sources
	if [ "$(USER)" = "cmiss" ]; then \
		cd $(PRODUCT_PATH); \
		echo -n > $(MAILFILE_PATH)/programmer.mail ; \
		if ! $(MAKE) -f $(MAKEFILE) update; then \
			cat $(MAILFILE_PATH)/updatefail.mail >> $(MAILFILE_PATH)/programmer.mail ; \
		fi ; \
	else \
		echo "Must be cmiss"; \
	fi
#This mail is added into the example mail.

force :
	@echo "\n" > /dev/null
