#Paths
PRODUCT_PATH=$(CMISS_ROOT)/cmgui
PRODUCT_SOURCE_PATH=$(PRODUCT_PATH)/source
MAILFILE_PATH=mailfiles

#Build defaults
USER_INTERFACE=USER_INTERFACE=MOTIF_USER_INTERFACE
DYNAMIC_GL_LINUX=false
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

cell cell-debug cell-debug-memorycheck cell64 cell64-debug : USER_INTERFACE_OPTION=$(USER_INTERFACE)
cell cell-debug cell-debug-memorycheck cell64 cell64-debug : USER_INTERFACE=MOTIF_USER_INTERFACE
cell cell64 : DEBUG_OPTION=$(DEBUG)
cell cell64 : DEBUG=false
cell-debug cell-debug-memorycheck cell64-debug : DEBUG_OPTION=$(DEBUG)
cell-debug cell-debug-memorycheck cell64-debug : DEBUG=true
cell64 cell64-debug  utilities64 : ABI_OPTION=$(ABI)
cell64 cell64-debug  utilities64 : ABI=64
cell-debug-memorycheck : MEMORYCHECK_OPTION=$(MEMORYCHECK)
cell-debug-memorycheck : MEMORYCHECK=true

utilities utilities64 : TARGET_OPTION=utilities
utilities utilities64 : force

ifdef TARGET
   TARGET_OPTION = $(TARGET)
endif
ifdef USER_INTERFACE
   USER_INTERFACE_OPTION = USER_INTERFACE=$(USER_INTERFACE)
endif
ifdef DYNAMIC_GL_LINUX
   DYNAMIC_GL_OPTION = DYNAMIC_GL_LINUX=$(DYNAMIC_GL_LINUX)
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
OPTIONS = $(TARGET_OPTION) $(USER_INTERFACE_OPTION) $(DYNAMIC_GL_LINUX_OPTION) $(DEBUG_OPTION) $(ABI_OPTION) $(MEMORYCHECK_OPTION)

cell cell-debug cell-debug-memorycheck cell64 cell64-debug utilities :
	cd source ; \
	$(MAKE) -f $(SUBMAKEFILE) $(OPTIONS) ;

ESU_BUILD_LIST = cell cell-debug cell-debug-memorycheck cell64 utilities
ESU_BUILD_PATH = '$${CMISS_ROOT}/cmgui'
ESU_BUILD_MACHINE = 130.216.208.35 #esu35
ESP_BUILD_LIST = cell cell-debug cell-debug-memorycheck utilities
ESP_BUILD_PATH = '$${CMISS_ROOT}/cmgui'
ESP_BUILD_MACHINE = 130.216.208.156 #esp56

update_sources :
	ssh cmiss@$(ESU_BUILD_MACHINE) 'cd $(ESU_BUILD_PATH)/source ; cvs update'

update :
	ssh cmiss@$(ESU_BUILD_MACHINE) 'cd $(ESU_BUILD_PATH) ; $(MAKE) -f $(MAKEFILE) $(ESU_BUILD_LIST)' && \
	ssh cmiss@$(ESP_BUILD_MACHINE) 'cd $(ESP_BUILD_PATH) ; $(MAKE) -f $(MAKEFILE) $(ESP_BUILD_LIST)'

depend:
	ssh cmiss@$(ESU_BUILD_MACHINE) 'cd $(ESU_BUILD_PATH) ; $(MAKE) -f $(MAKEFILE) $(ESU_BUILD_LIST) TARGET=depend' && \
	ssh cmiss@$(ESP_BUILD_MACHINE) 'cd $(ESP_BUILD_PATH) ; $(MAKE) -f $(MAKEFILE) $(ESP_BUILD_LIST) TARGET=depend'

cronjob: update_sources
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
