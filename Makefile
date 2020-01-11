# -*- Mode: makefile-gmake -*-
#
# $Id: Makefile,v 1.87 2020/01/11 21:42:07 slava Exp $
#
# Makefile for libslava.a
#
# Copyright (C) 2000-2020 by Slava Monich
#
# Redistribution and use in source and binary forms, with or without 
# modification, are permitted provided that the following conditions 
# are met: 
#
#   1. Redistributions of source code must retain the above copyright 
#      notice, this list of conditions and the following disclaimer. 
#   2. Redistributions in binary form must reproduce the above copyright 
#      notice, this list of conditions and the following disclaimer 
#      in the documentation and/or other materials provided with the 
#      distribution. 
#   3. Neither the names of the copyright holders nor the names of its
#      contributors may be used to endorse or promote products derived
#      from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED ``AS IS'' AND ANY EXPRESSED OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. 
# IN NO EVENT SHALL THE CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, 
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) ARISING 
# IN ANY WAY OUT OF THE USE OR INABILITY TO USE THIS SOFTWARE, EVEN 
# IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
#
# The views and conclusions contained in the software and documentation 
# are those of the authors and should not be interpreted as representing 
# any official policies, either expressed or implied.
#

.PHONY: clean distclean all debug release profile coverage pkgconfig check

DEBUG_NAME = libslavad
RELEASE_NAME = libslava
PROFILE_NAME = libslavap
COVERAGE_NAME = libslavac

CC = $(CROSS_COMPILE)gcc
DEBUG_FLAGS = -g
RELEASE_FLAGS = -O2
PROFILE_FLAGS = $(RELEASE_FLAGS) -pg
COVERAGE_FLAGS = $(RELEASE_FLAGS) --coverage

INCLUDES = -I./include -I./src -I../curl/include -I../expat/lib
DEFINES = -D_REENTRANT -D_HAVE_CURL -D_HAVE_EXPAT
DEBUG_DEFINES = -DDEBUG=1 -DDEBUG_MEM

WARNINGS = -Wall -Wstrict-prototypes -Wshadow -Wwrite-strings \
 -Waggregate-return -Wnested-externs

CPPFLAGS = $(INCLUDES) $(DEFINES)
CFLAGS = $(WARNINGS) -fno-strict-aliasing -fPIC -MMD -MP
ARFLAGS = rc

DEBUG_CFLAGS = $(DEBUG_FLAGS) $(CFLAGS) $(CPPFLAGS) $(DEBUG_DEFINES)
RELEASE_CFLAGS = $(RELEASE_FLAGS) $(CFLAGS) $(CPPFLAGS)
PROFILE_CFLAGS = $(PROFILE_FLAGS) $(CFLAGS) $(CPPFLAGS)
COVERAGE_CFLAGS = $(COVERAGE_FLAGS) $(CFLAGS) $(CPPFLAGS)

#
# Macros to quietly run commands
#

RUN = $(if $(V),$1,$(if $(2),@echo $2 && $1, @$1))

RUN_MKDIR = $(call RUN,mkdir -p $1,"  MKDIR $1")
RUN_AR = $(call RUN,$(AR) $(ARFLAGS) $1 $2,"  AR    $1")
RUN_CC = $(call RUN,$(call CC_CMD,$1,$2,$3),"  CC    $3")
CC_CMD = $(CC) -c $1 -MT$3 -MF$(3:%.o=%.d) $2 -o $3
CC_DEBUG = $(call RUN_CC,$(DEBUG_CFLAGS),$1,$2)
CC_RELEASE = $(call RUN_CC,$(RELEASE_CFLAGS),$1,$2)
CC_PROFILE = $(call RUN_CC,$(PROFILE_CFLAGS),$1,$2)
CC_COVERAGE = $(call RUN_CC,$(COVERAGE_CFLAGS),$1,$2)

ifdef V
SUBMAKE = make V=$(V)
else
SUBMAKE = make --no-print-directory
endif

#
# Library version
#

VERSION=$(shell scripts/version)

#
# Default target
#

all: debug release pkgconfig

#
# OS specific stuff
#

OS = $(shell uname -s)

ifeq ($(OS),Darwin)
OS_RELEASE = $(shell uname -r | cut -f1 -d.)
ifneq ($(MACOSX_DEPLOYMENT_TARGET),)
CFLAGS += -mmacosx-version-min=$(MACOSX_DEPLOYMENT_TARGET)
endif
endif # Darwin

ifeq ($(ARCH),native)
override ARCH =
endif

ifneq ($(ARCH),)

BUILD_DIR = build/$(ARCH)
DEBUG_LIB = $(DEBUG_NAME)-$(ARCH).a
RELEASE_LIB = $(RELEASE_NAME)-$(ARCH).a
PROFILE_LIB = $(PROFILE_NAME)-$(ARCH).a
COVERAGE_LIB = $(COVERAGE_NAME)-$(ARCH).a

ifeq ($(OS),Darwin)
CFLAGS += -arch $(ARCH)
endif # Darwin

else # ARCH

BUILD_DIR = build
DEBUG_LIB = $(DEBUG_NAME).a
RELEASE_LIB = $(RELEASE_NAME).a
PROFILE_LIB = $(PROFILE_NAME).a
COVERAGE_LIB = $(COVERAGE_NAME).a

# Support for Mac OS X universal binaries
ifeq ($(OS),Darwin)
LIPO = lipo
ifeq ($(shell [ $(OS_RELEASE) -ge 8 ] ; echo $$?),0)
# Tiger of later
CFLAGS  += -arch i386
DEBUG_LIB_I386 = $(DEBUG_NAME)-i386.a
RELEASE_LIB_I386 = $(RELEASE_NAME)-i386.a
PROFILE_LIB_I386 = $(PROFILE_NAME)-i386.a
COVERAGE_LIB_I386 = $(COVERAGE_NAME)-i386.a
DEBUG_LIBS += $(DEBUG_LIB_I386)
RELEASE_LIBS += $(RELEASE_LIB_I386)
PROFILE_LIBS += $(PROFILE_LIB_I386)
COVERAGE_LIBS += $(COVERAGE_LIB_I386)
$(DEBUG_LIB_I386): debug_lib_i386
$(RELEASE_LIB_I386): release_lib_i386
$(PROFILE_LIB_I386): profile_lib_i386
$(COVERAGE_LIB_I386): coverage_lib_i386
ifeq ($(shell [ $(OS_RELEASE) -le 10 ] ; echo $$?),0)
# Support for PowerPC was dropped in Lion
CFLAGS  += -arch ppc
DEBUG_LIB_PPC = $(DEBUG_NAME)-ppc.a
RELEASE_LIB_PPC = $(RELEASE_NAME)-ppc.a
PROFILE_LIB_PPC = $(PROFILE_NAME)-ppc.a
COVERAGE_LIB_PPC = $(COVERAGE_NAME)-ppc.a
DEBUG_LIBS += $(DEBUG_LIB_PPC)
RELEASE_LIBS += $(RELEASE_LIB_PPC)
PROFILE_LIBS += $(PROFILE_LIB_PPC)
COVERAGE_LIBS += $(COVERAGE_LIB_PPC)
$(DEBUG_LIB_PPC): debug_lib_ppc
$(RELEASE_LIB_PPC): release_lib_ppc
$(PROFILE_LIB_PPC): profile_lib_ppc
$(COVERAGE_LIB_PPC): coverage_lib_ppc
endif
ifeq ($(shell [ $(OS_RELEASE) -ge 10 ] ; echo $$?),0)
# Snow Leopard or later
CFLAGS  += -arch x86_64
DEBUG_LIB_X86_64 = $(DEBUG_NAME)-x86_64.a
RELEASE_LIB_X86_64 = $(RELEASE_NAME)-x86_64.a
PROFILE_LIB_X86_64 = $(PROFILE_NAME)-x86_64.a
COVERAGE_LIB_X86_64 = $(COVERAGE_NAME)-x86_64.a
DEBUG_LIBS += $(DEBUG_LIB_X86_64)
RELEASE_LIBS += $(RELEASE_LIB_X86_64)
PROFILE_LIBS += $(PROFILE_LIB_X86_64)
COVERAGE_LIBS += $(COVERAGE_LIB_X86_64)
$(DEBUG_LIB_X86_64): debug_lib_x86_64
$(RELEASE_LIB_X86_64): release_lib_x86_64
$(PROFILE_LIB_X86_64): profile_lib_x86_64
$(COVERAGE_LIB_X86_64): coverage_lib_x86_64
endif # Darwin 10.x
endif # Darwin 8.x

debug_lib_%:
	@$(SUBMAKE) ARCH=$* debug
release_lib_%:
	@$(SUBMAKE) ARCH=$* release
profile_lib_%:
	@$(SUBMAKE) ARCH=$* profile
coverage_lib_%:
	@$(SUBMAKE) ARCH=$* coverage

endif # Darwin

endif # ARCH

#
# (More or less) platform independent sources
#

SRC = s_base32.c s_base64.c s_bitset.c s_buf.c s_cs.c s_dom.c \
  s_event.c s_file.c s_fio.c s_fmem.c s_fnull.c s_fsock.c s_fsplit.c \
  s_fsub.c s_futil.c s_fwrap.c s_fzio.c s_fzip.c s_hash.c s_hist.c \
  s_init.c s_itr.c s_itra.c s_itrc.c s_itrf.c s_itrs.c s_lib.c s_lock.c \
  s_math.c s_md.c s_md5.c s_mem.c s_mfp.c s_mpm.c s_mutex.c s_net.c s_opt.c \
  s_parse.c s_prop.c s_propx.c s_ring.c s_rwlock.c s_queue.c s_random.c \
  s_sha1.c s_stack.c s_str.c s_strbuf.c s_thread.c s_time.c s_trace.c \
  s_utf8.c s_util.c s_vector.c s_wkq.c s_xml.c \
  s_xmlp.c

#
# Platform specific sources
#

SRC1 = u_event.c u_furl.c u_futil.c u_mutex.c u_thread.c u_trace.c

#
# Directories
#

SRC_DIR = ./src
SRC1_DIR = ./src/unix
DEBUG_BUILD_DIR = $(BUILD_DIR)/debug
RELEASE_BUILD_DIR = $(BUILD_DIR)/release
PROFILE_BUILD_DIR = $(RELEASE_BUILD_DIR)/profile
COVERAGE_BUILD_DIR = $(BUILD_DIR)/coverage
LIBDIR ?= /usr/lib

#
# Files
#
PKGCONFIG = \
  $(BUILD_DIR)/slib.pc \
  $(BUILD_DIR)/slibd.pc 
SRC_FILES = \
  $(SRC:%=$(SRC_DIR)/%) \
  $(SRC1:%=$(SRC1_DIR)/%)
DEBUG_OBJS = \
  $(SRC:%.c=$(DEBUG_BUILD_DIR)/%.o) \
  $(SRC1:%.c=$(DEBUG_BUILD_DIR)/%.o)
RELEASE_OBJS = \
  $(SRC:%.c=$(RELEASE_BUILD_DIR)/%.o) \
  $(SRC1:%.c=$(RELEASE_BUILD_DIR)/%.o)
PROFILE_OBJS = \
  $(SRC:%.c=$(PROFILE_BUILD_DIR)/%.o) \
  $(SRC1:%.c=$(PROFILE_BUILD_DIR)/%.o)
COVERAGE_OBJS = \
  $(SRC:%.c=$(COVERAGE_BUILD_DIR)/%.o) \
  $(SRC1:%.c=$(COVERAGE_BUILD_DIR)/%.o)

#
# Dependencies
#

DEPS = \
  $(DEBUG_OBJS:%.o=%.d) \
  $(RELEASE_OBJS:%.o=%.d) \
  $(PROFILE_OBJS:%.o=%.d) \
  $(COVERAGE_OBJS:%.o=%.d)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
endif

ifeq (3.81,$(firstword $(sort $(MAKE_VERSION) 3.81)))
$(DEBUG_OBJS):    | $(DEBUG_BUILD_DIR)
$(RELEASE_OBJS):  | $(RELEASE_BUILD_DIR)
$(PROFILE_OBJS):  | $(PROFILE_BUILD_DIR)
$(COVERAGE_OBJS): | $(COVERAGE_BUILD_DIR)
endif

#
# Rules
#

debug: $(DEBUG_BUILD_DIR) $(DEBUG_LIB)

release: $(RELEASE_BUILD_DIR) $(RELEASE_LIB)

profile: $(PROFILE_BUILD_DIR) $(PROFILE_LIB)

coverage: $(COVERAGE_BUILD_DIR) $(COVERAGE_BUILD_DIR)/$(COVERAGE_LIB)

check:
	@$(SUBMAKE) -C test test

clean:
	@$(SUBMAKE) -C test clean
	$(call RUN,rm -fr core *~ */*~ libslava*.a $(BUILD_DIR))
	$(call RUN,rm -fr RPMS installroot documentation.list)
	$(call RUN,rm -fr debian/tmp debian/libslava-dev)
	$(call RUN,rm -f debian/files debian/*.substvars)
	$(call RUN,rm -f debian/*.debhelper.log debian/*.debhelper debian/*~)

distclean: clean
	rm -fr test/coverage/*.gcov test/coverage/report

pkgconfig: $(PKGCONFIG)

$(DEBUG_BUILD_DIR):
	$(call RUN_MKDIR,$@)

$(RELEASE_BUILD_DIR):
	$(call RUN_MKDIR,$@)

$(PROFILE_BUILD_DIR):
	$(call RUN_MKDIR,$@)

$(COVERAGE_BUILD_DIR):
	$(call RUN_MKDIR,$@)

ifeq ($(DEBUG_LIBS),)
$(DEBUG_LIB): $(DEBUG_OBJS)
	$(call RUN_AR,$@,$?)
	$(call RUN,ranlib  $@)
else
$(DEBUG_LIB): $(DEBUG_LIBS)
	$(call RUN,$(LIPO) $^ -create -output $@,"  LIPO  $@")
endif

ifeq ($(RELEASE_LIBS),)
$(RELEASE_LIB): $(RELEASE_OBJS)
	$(call RUN_AR,$@,$?)
	$(call RUN,ranlib  $@)
else
$(RELEASE_LIB): $(RELEASE_LIBS)
	$(call RUN,$(LIPO) $^ -create -output $@,"  LIPO  $@")
endif

ifeq ($(PROFILE_LIBS),)
$(PROFILE_LIB): $(PROFILE_OBJS)
	$(call RUN_AR,$@,$?)
	$(call RUN,ranlib  $@)
else
$(PROFILE_LIB): $(PROFILE_LIBS)
	$(call RUN,$(AR) $^ -create -output $@,"  AR    $@")
endif

ifeq ($(COVERAGE_LIBS),)
$(COVERAGE_BUILD_DIR)/$(COVERAGE_LIB): $(COVERAGE_OBJS)
	$(call RUN_AR,$@,$?)
	$(call RUN,ranlib  $@)
else
$(COVERAGE_BUILD_DIR)/$(COVERAGE_LIB): $(COVERAGE_LIBS)
	$(call RUN,$(AR) $^ -create -output $@,"  AR    $@")
endif

#
# Pattern Rules
#

$(BUILD_DIR)/%.pc: %.pc.in
	$(call RUN,sed -e 's/\[version\]/'$(VERSION)/g -e 's/\[libdir\]/'`echo $(LIBDIR) | sed 's/\\//\\\\\\//g'`/g $< > $@)

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(call CC_DEBUG,$<,$@)

$(DEBUG_BUILD_DIR)/%.o : $(SRC1_DIR)/%.c
	$(call CC_DEBUG,$<,$@)

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(call CC_RELEASE,$<,$@)

$(RELEASE_BUILD_DIR)/%.o : $(SRC1_DIR)/%.c
	$(call CC_RELEASE,$<,$@)

$(PROFILE_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(call CC_PROFILE,$<,$@)

$(PROFILE_BUILD_DIR)/%.o : $(SRC1_DIR)/%.c
	$(call CC_PROFILE,$<,$@)

$(COVERAGE_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(call CC_COVERAGE,$<,$@)

$(COVERAGE_BUILD_DIR)/%.o : $(SRC1_DIR)/%.c
	$(call CC_COVERAGE,$<,$@)

#
# Install
#

INSTALL_PERM  = 644
INSTALL_OWNER = $(shell id -u)
INSTALL_GROUP = $(shell id -g)

INSTALL = install
INSTALL_DIRS = $(INSTALL) -d
INSTALL_FILES = $(INSTALL) -m $(INSTALL_PERM)

INSTALL_LIB_DIR = $(DESTDIR)$(LIBDIR)
INSTALL_INCLUDE_DIR = $(DESTDIR)/usr/include/slib
INSTALL_PKGCONFIG_DIR = $(DESTDIR)/$(LIBDIR)/pkgconfig

install: $(PKGCONFIG) $(INSTALL_LIB_DIR) $(INSTALL_INCLUDE_DIR) $(INSTALL_PKGCONFIG_DIR)
	$(INSTALL_FILES) $(DEBUG_LIB) $(RELEASE_LIB) $(INSTALL_LIB_DIR)
	$(INSTALL_FILES) include/*.h $(INSTALL_INCLUDE_DIR)
	rm $(INSTALL_INCLUDE_DIR)/s_nt*.h
	$(INSTALL_FILES) $(PKGCONFIG) $(INSTALL_PKGCONFIG_DIR)

$(INSTALL_LIB_DIR):
	$(INSTALL_DIRS) $@

$(INSTALL_INCLUDE_DIR):
	$(INSTALL_DIRS) $@

$(INSTALL_PKGCONFIG_DIR):
	$(INSTALL_DIRS) $@

#
# $Log: Makefile,v $
# Revision 1.87  2020/01/11 21:42:07  slava
# o removed unnecessary GCOV stuff
#
# Revision 1.86  2020/01/11 19:46:03  slava
# o rpm packaging
#
# Revision 1.85  2020/01/11 17:08:27  slava
# o added check target
#
# Revision 1.84  2018/07/28 15:39:24  slava
# o added separate coverage target. It's not very convenient when the
#   same object files can be built with different flags. Better keep
#   each build flavor in a directory of its own, helps to avoid mixups.
#
# Revision 1.83  2017/10/28 22:44:17  slava
# o replaced veryclean target with distclean which actually does something
#   useful. Also, remove coverage reports on distclean, to reduce the size
#   of the source tarballs produced by dpkg-buildpackage.
#
# Revision 1.82  2016/11/09 21:28:39  slava
# Install debug library too (dpkg)
#
# Revision 1.81  2015/12/05 22:26:31  slava
# o fixed the problem with ranlib not being invoked after the build
#
# Revision 1.80  2015/12/05 22:16:30  slava
# o housekeeping
#
# Revision 1.79  2015/12/05 19:22:56  slava
# o compatibility with older make (before 3.81)
#
# Revision 1.78  2015/11/29 11:51:16  slava
# o run more commands quietly
#
# Revision 1.77  2015/11/27 11:59:29  slava
# o added support for code coverage
#
# Revision 1.76  2015/11/23 19:18:12  slava
# o made macosx-version-min configurable on Mac OS X
#
# Revision 1.75  2015/11/23 17:58:05  slava
# o fixed library dependencies on OS X
#
# Revision 1.74  2015/11/21 16:48:44  slava
# o added debian packaging files
#
# Revision 1.73  2014/03/27 15:37:58  slava
# o make build directories order-only prerequisites of the object files
#
# Revision 1.72  2013/12/26 14:58:44  slava
# o made dependency generation work with gcc 4.1.x
#
# Revision 1.71  2013/10/26 13:34:43  slava
# o changed dependency generation mechanism
#
# Revision 1.70  2013/09/25 08:59:54  slava
# o don't rebuild dependencies on clean
#
# Revision 1.69  2012/12/13 17:42:26  slava
# o fixed point math
#
# Revision 1.68  2011/09/06 06:02:33  slava
# Apple dropped support for PowerPC in Lion
#
# Revision 1.67  2010/10/23 21:41:05  slava
# o quiet compilation
#
# Revision 1.66  2010/10/23 21:15:56  slava
# o Mac OS X compilation issues
#
# Revision 1.65  2010/10/07 18:11:33  slava
# o fixed Mac OS X 10.4 compatibility issue
#
# Revision 1.64  2010/07/03 09:42:42  slava
# o added UTF-8 utilities
#
# Revision 1.63  2009/11/17 00:14:24  slava
# o introducing generic synchronization API. This is a pretty destructive
#   checkin, it's not 100% backward compatible. The Lock type is now a
#   generic lock. What used to be called Lock is now RWLock. Sorry.
#
# Revision 1.62  2009/09/26 19:51:08  slava
# o added support for Mac OS X universal binaries
#
# Revision 1.61  2009/06/30 05:55:33  slava
# o added -fPIC to gcc flags. It's necessary in order to link slib into shared
#   objects in 64-bit environment. This flag is set by default on Darwin/MacOSX
#   but not on x86_64 Linux
#
# Revision 1.60  2009/05/23 09:14:38  slava
# o a few tweaks for x86_64 build
#
# Revision 1.59  2008/09/03 09:24:35  slava
# o added ring buffer object
#
# Revision 1.58  2008/03/02 09:44:07  slava
# o moved SLIB_InitModules() and SLIB_Shutdown() to the separate file
#   s_init.c to break curcular dependency between source files. Almost
#   every xxx_InitModule function was dependant on all other xxx_InitModule
#   functions via SLIB_Abort which happened to be in the same file (and
#   therefore, the same object module) as SLIB_InitModules.
#
# Revision 1.57  2007/06/05 18:09:03  slava
# o updated clean target to delete CPU-specific results (needed on Mac OS X)
#
# Revision 1.56  2007/02/08 17:53:33  slava
# o added Unix implementation of URL stream using libcurl. Not particularly
#   efficient. Each FILE_OpenURL call creates its own thread.
#
# Revision 1.55  2006/10/13 23:35:37  slava
# o added "sub-stream" I/O which reads no more than specified number of bytes
#   from another stream. It can be created by FILE_SubStream() function.
#
# Revision 1.54  2006/10/12 18:30:39  slava
# o splitting s_util.c into multiple files, because it's getting huge.
#   Moved parsing utilities to s_parse.c
#
# Revision 1.53  2006/03/20 06:50:24  slava
# o added singleton iterator to complete the picture
#
# Revision 1.52  2006/03/20 02:49:47  slava
# o moved individual iterators into separate files so that they don't
#   always get linked into each and every executable. Now they should
#   only be linked if they are actually being used.
#
# Revision 1.51  2006/03/12 15:12:24  slava
# o added u_trace.c
#
# Revision 1.50  2005/08/23 23:10:52  slava
# o added NULL I/O object
#
# Revision 1.49  2005/02/21 02:13:05  slava
# o cleaning up some mess with dependencies
#
# Revision 1.48  2005/02/19 01:50:54  slava
# o fixed dependencies so that build directories always get created before
#   compilation starts
#
# Revision 1.47  2005/02/19 01:12:09  slava
# o separated Unix specific code from platform independent code
#
# Revision 1.46  2004/08/18 02:52:01  slava
# o added support for BASE32 encoding
#
# Revision 1.45  2004/07/31 07:27:03  slava
# o implemented MD-5 digest (RFC 1321)
#
# Revision 1.44  2004/07/29 17:09:37  slava
# o message digest framework
#
# Revision 1.43  2004/07/19 22:55:11  slava
# o moved BASE64 encoding functions from s_util to s_base64 module
#
# Revision 1.42  2003/12/04 04:47:18  slava
# o changed default target to compile the kernel library if _LINUX_KERNEL_
#   macro is defined
#
# Revision 1.41  2003/12/04 04:30:53  slava
# o event support for Linux kernel
#
# Revision 1.40  2003/12/01 02:54:08  slava
# o made KERNEL_DIR configurable from the command line.
#
# Revision 1.39  2003/11/30 11:48:35  slava
# o Linux kernel compilation issues
#
# Revision 1.38  2003/11/30 10:08:21  slava
# o changed compilation options
#
# Revision 1.37  2003/11/30 02:49:57  slava
# o port to Linux kernel mode environment
#
# Revision 1.36  2003/11/08 20:47:43  slava
# o added iterators
#
# Revision 1.35  2003/07/31 15:15:45  slava
# o multiple precision arithmetic
#
# Revision 1.34  2003/05/21 02:21:25  slava
# o slightly changed the format of the dependency file
#
# Revision 1.33  2003/03/13 01:13:25  slava
# o fixed the "Updating dependencies" message to display proper name of the
#   make utility
#
# Revision 1.32  2003/01/28 20:13:09  slava
# o added s_dom.c to the build
#
# Revision 1.31  2003/01/05 17:23:39  slava
# o added s_propx.c and s_xmlp.c to the build
#
# Revision 1.30  2002/09/23 02:56:08  slava
# o integration with Expat 1.19.5 on Unix
#
# Revision 1.29  2002/09/15 19:43:17  slava
# o use -O2 optimization in release build, because -O3 produces weird
#   compilation warnings and sometimes strange code
#
# Revision 1.28  2002/07/15 18:03:45  slava
# o print a message when updating dependencies
#
# Revision 1.27  2002/06/22 14:14:28  slava
# o don't need to define _GNU_SOURCE
#
# Revision 1.26  2002/05/29 06:55:43  slava
# o added s_opt.c to the build
#
# Revision 1.25  2002/05/22 14:12:23  slava
# o added s_fmem.c to the build
#
# Revision 1.24  2002/05/22 05:05:09  slava
# o added s_strbuf (former s_buf) to the build
#
# Revision 1.23  2002/02/13 08:16:20  slava
# o changed the sed script that generates dependency file
#
# Revision 1.22  2002/02/11 08:24:54  slava
# o added 'profile' target
#
# Revision 1.21  2002/02/11 06:38:10  slava
# o updated clean target
#
# Revision 1.20  2002/01/25 04:17:43  slava
# o added s_fzio.c to the build
#
# Revision 1.19  2002/01/08 16:16:39  slava
# o enclose sed parameters in quotes
#
# Revision 1.18  2001/12/28 02:48:56  slava
# o added s_bitset.c to the build
#
# Revision 1.17  2001/12/23 22:03:39  slava
# o another major code restructuring. slib sources have been moved to src
#   subdirectory (history preserved, tags deleted) and header moved to
#   include subdirectory. src directiry also contains library private
#   headers (currently only one)
#
# Revision 1.16  2001/12/22 03:42:58  slava
# o added s_net.c to the build
#
# Revision 1.15  2001/12/21 01:50:33  slava
# o build both release and debug libraries
#
# Revision 1.14  2001/11/27 06:55:55  slava
# o added s_wkq module
#
# Revision 1.13  2001/11/26 08:02:30  slava
# o added s_math module
#
# Revision 1.12  2001/11/25 01:48:17  slava
# o added s_lib.c to the build
#
# Revision 1.11  2001/10/09 21:41:27  slava
# o defined _HAVE_EXPAT when compiling this stuff on Unix
#
# Revision 1.10  2001/10/08 08:39:24  slava
# o added s_xml module
#
# Revision 1.9  2001/10/08 05:53:28  slava
# o minor cleanup
#
# Revision 1.8  2001/10/08 05:14:50  slava
# o enable more gcc warnings
#
# Revision 1.7  2001/05/30 09:13:54  slava
# o added s_time.c
#
# Revision 1.6  2001/05/26 21:59:53  slava
# o updated Unix makefiles (added s_trace.c)
#
# Revision 1.5  2001/01/12 06:52:53  slava
# o support for histogramming
#
# Revision 1.4  2001/01/03 09:18:23  slava
# o new file I/O support for transparent access to plain or compressed
#   files, sockets, etc.
#
# Revision 1.3  2000/12/31 02:55:47  oleg
# o added Stack to slib
#
# Revision 1.2  2000/10/15 14:26:41  slava
# o added "Mode: makefile" comment
#
# Revision 1.1  2000/08/19 04:48:58  slava
# o initial checkin
#
