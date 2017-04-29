# -*- Mode: makefile-gmake -*-

.PHONY: clean all debug release slib-release slib-debug

#
# Real test makefile defines EXE (and possibly SRC) and includes this one.
#

ifndef EXE
${error EXE not defined}
endif

ifndef SRC
SRC = $(EXE).c
endif

#
# Default target
#

all: debug release

#
# Directories
#

SRC_DIR = .
LIB_DIR = ../..
COMMON_DIR = ../common
BUILD_DIR = build
DEBUG_BUILD_DIR = $(BUILD_DIR)/debug
RELEASE_BUILD_DIR = $(BUILD_DIR)/release

#
# Code coverage
#

ifndef GCOV
GCOV = 0
endif

ifneq ($(GCOV),0)
CFLAGS += --coverage
LDFLAGS += --coverage
SUBMAKE_OPTS += GCOV=1
endif

#
# Tools and flags
#

CC = $(CROSS_COMPILE)gcc
LD = $(CC)
WARNINGS = -Wall
INCLUDES = -I$(LIB_DIR)/include -I$(LIB_DIR)/src -I$(COMMON_DIR)
BASE_FLAGS = -fPIC
BASE_LDFLAGS = $(BASE_FLAGS) $(LDFLAGS)
BASE_CFLAGS = $(BASE_FLAGS) $(CFLAGS)
FULL_CFLAGS = $(BASE_CFLAGS) $(DEFINES) $(WARNINGS) $(INCLUDES) -MMD -MP
FULL_LDFLAGS = $(BASE_LDFLAGS)
LIBS = -lpthread -lm
QUIET_MAKE = make --no-print-directory
DEBUG_FLAGS = -g
RELEASE_FLAGS =

ifndef KEEP_SYMBOLS
KEEP_SYMBOLS = 0
endif

ifneq ($(KEEP_SYMBOLS),0)
RELEASE_FLAGS += -g
SUBMAKE_OPTS += KEEP_SYMBOLS=1
endif

DEBUG_LDFLAGS = $(FULL_LDFLAGS) $(DEBUG_FLAGS)
RELEASE_LDFLAGS = $(FULL_LDFLAGS) $(RELEASE_FLAGS)
DEBUG_CFLAGS = $(FULL_CFLAGS) $(DEBUG_FLAGS) -DDEBUG -DDEBUG_MEM
RELEASE_CFLAGS = $(FULL_CFLAGS) $(RELEASE_FLAGS) -O2

RUN = $(if $(V),$1,$(if $(2),@echo $2 && $1, @$1))

#
# Files
#

DEBUG_OBJS = \
  $(COMMON_SRC:%.c=$(DEBUG_BUILD_DIR)/%.o) \
  $(SRC:%.c=$(DEBUG_BUILD_DIR)/%.o)
RELEASE_OBJS = \
  $(COMMON_SRC:%.c=$(RELEASE_BUILD_DIR)/%.o) \
  $(SRC:%.c=$(RELEASE_BUILD_DIR)/%.o)

DEBUG_LIB := $(LIB_DIR)/libslavad.a
RELEASE_LIB := $(LIB_DIR)/libslava.a

#
# Dependencies
#

DEPS = $(DEBUG_OBJS:%.o=%.d) $(RELEASE_OBJS:%.o=%.d)
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
endif

$(DEBUG_OBJS): | $(DEBUG_BUILD_DIR)
$(RELEASE_OBJS): | $(RELEASE_BUILD_DIR)

#
# Rules
#

DEBUG_EXE = $(DEBUG_BUILD_DIR)/$(EXE)
RELEASE_EXE = $(RELEASE_BUILD_DIR)/$(EXE)

debug: slib-debug $(DEBUG_EXE)

release: slib-release $(RELEASE_EXE)

clean:
	rm -f *~
	rm -fr $(BUILD_DIR)

cleaner: clean
	@make -C $(LIB_DIR) clean

test_banner:
	@echo "===========" $(EXE) "=========== "

test: test_banner debug
	@$(DEBUG_EXE)

valgrind: test_banner debug
	$(call RUN,G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --tool=memcheck --leak-check=full --show-possibly-lost=yes $(DEBUG_EXE),)

$(DEBUG_BUILD_DIR):
	mkdir -p $@

$(RELEASE_BUILD_DIR):
	mkdir -p $@

CC_RUN = $(call RUN,$(call CC_CMD,$1,$2,$3),"  CC    $3")
LD_RUN = $(call RUN,$(LD) $1 $2 $3 $(LIBS) -o $4,"  LD    $@")
CC_CMD = $(CC) -c $1 -MT$3 -MF$(3:%.o=%.d) $2 -o $3
CC_DEBUG = $(call CC_RUN,$(DEBUG_CFLAGS),$1,$2)
CC_RELEASE = $(call CC_RUN,$(RELEASE_CFLAGS),$1,$2)

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(call CC_DEBUG,$<,$@)

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(call CC_RELEASE,$<,$@)

$(DEBUG_BUILD_DIR)/%.o : $(COMMON_DIR)/%.c
	$(call CC_DEBUG,$<,$@)

$(RELEASE_BUILD_DIR)/%.o : $(COMMON_DIR)/%.c
	$(call CC_RELEASE,$<,$@)

$(DEBUG_EXE): $(DEBUG_LIB) $(DEBUG_BUILD_DIR) $(DEBUG_OBJS)
	$(call LD_RUN,$(DEBUG_LDFLAGS),$(DEBUG_OBJS),$<,$@)

$(RELEASE_EXE): $(RELEASE_LIB) $(RELEASE_BUILD_DIR) $(RELEASE_OBJS)
	$(call LD_RUN,$(RELEASE_LDFLAGS),$(RELEASE_OBJS),$<,$@)
ifeq ($(KEEP_SYMBOLS),0)
	$(call RUN,strip $@,"  STRIP $@")
endif

slib-debug:
	@$(QUIET_MAKE) $(SUBMAKE_OPTS) -C $(LIB_DIR) debug

slib-release:
	@$(QUIET_MAKE) $(SUBMAKE_OPTS) -C $(LIB_DIR) release
