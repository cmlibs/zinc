CMGUI_DEV_SOURCE=$(PWD)/..
PERL_INSTALL=$(CMISS_ROOT)/cmiss_perl
VPATH=$(CMGUI_DEV_SOURCE)
BUILDER="Shane Blackett <s.blackett@auckland.ac.nz>"
#perl Makefile.PL PREFIX=/home/blackett/perl OPTIMIZE="-g -W -Wall -Wno-parentheses -Wno-switch -Wno-format -Wno-unused -Werror"

ifndef SYSNAME
  SYSNAME := $(shell uname)
  ifeq ($(SYSNAME),)
    $(error error with shell command uname)
  endif
endif

ifndef NODENAME
  NODENAME := $(shell uname -n)
  ifeq ($(NODENAME),)
    $(error error with shell command uname -n)
  endif
endif

ifndef MACHNAME
  MACHNAME := $(shell uname -m)
  ifeq ($(MACHNAME),)
    $(error error with shell command uname -m)
  endif
endif

ifeq ($(DEBUG),true)
   OPT_FLAG := OPTIMIZE="-g -W -Wall -Wno-parentheses -Wno-switch -Wno-format -Wno-unused -Werror"
else
   OPT_FLAG := 
endif

ifneq ($(origin CMISS$(ABI_ENV)_PERL),undefined)
  PERL := $(CMISS$(ABI_ENV)_PERL)
else
  ifdef CMISS_PERL
    PERL := $(CMISS_PERL)
  else
    # defaults first
    PERL = perl# first perl in path
  endif
endif

define InstallPerlArchModules
	cd Cmiss && \
	make clean ; \
	export CMGUI_DEV_SOURCE=$(CMGUI_DEV_SOURCE) && \
	$(1) Makefile.PL PREFIX=$(PERL_INSTALL) $(OPT_FLAG) && \
	make install;

endef

main :
	$(call InstallPerlArchModules, $(PERL))

ifeq ($(SYSNAME),Linux)
   PERL_EXECUTABLES = $(wildcard ${CMISS_ROOT}/perl/bin-5.?.?-i386-linux*/perl)
   PERL_EXECUTABLES += $(wildcard ${CMISS_ROOT}/perl/bin-5.?.?-i686-linux*/perl)	
endif
ifeq ($(filter-out IRIX%,$(SYSNAME)),)# SGI
   PERL_EXECUTABLES += $(wildcard ${CMISS_ROOT}/perl/bin-5.?.?-irix*/perl)
endif
ifeq ($(SYSNAME),AIX)
   PERL_EXECUTABLES += $(wildcard ${CMISS_ROOT}/perl/bin-5.?.?-rs6000*/perl)
endif

.NOTPARALLEL:

all :
	$(foreach perl, $(PERL_EXECUTABLES), $(call InstallPerlArchModules, $(perl)))
