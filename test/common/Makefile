# -*- Mode: makefile-gmake -*-

.PHONY: clean cleaner all debug release coverage test_banner test valgrind
.PHONY: slib-debug slib-release slib-coverage

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

# Prefer to use absolute paths, that helps with generating coverage reports
REALPATH := $(shell which realpath)
define realpath
    $(if $(REALPATH), $(shell realpath $(1)), echo $(1))
endef

SRC_DIR := $(realpath .)
LIB_DIR := $(realpath ../..)
COMMON_DIR := $(realpath ../common)
BUILD_DIR = build
DEBUG_BUILD_DIR = $(BUILD_DIR)/debug
RELEASE_BUILD_DIR = $(BUILD_DIR)/release
COVERAGE_BUILD_DIR = $(BUILD_DIR)/coverage

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
DEBUG_FLAGS = -g
RELEASE_FLAGS =
COVERAGE_FLAGS = --coverage

#
# Verbosity
#

ifdef V
SUBMAKE = make V=$(V)
else
SUBMAKE = make --no-print-directory
endif

RUN = $(if $(V),$1,$(if $(2),@echo $2 && $1, @$1))

ifndef KEEP_SYMBOLS
KEEP_SYMBOLS = 0
endif

ifneq ($(KEEP_SYMBOLS),0)
RELEASE_FLAGS += -g
SUBMAKE_OPTS += KEEP_SYMBOLS=1
endif

ifndef GCOV
GCOV = 0
endif

ifneq ($(GCOV),0)
DEBUG_FLAGS += --coverage
RELEASE_FLAGS += --coverage
SUBMAKE_OPTS += GCOV=1
endif

DEBUG_LDFLAGS = $(FULL_LDFLAGS) $(DEBUG_FLAGS)
RELEASE_LDFLAGS = $(FULL_LDFLAGS) $(RELEASE_FLAGS)
COVERAGE_LDFLAGS = $(FULL_LDFLAGS) $(COVERAGE_FLAGS)

DEBUG_CFLAGS = $(FULL_CFLAGS) $(DEBUG_FLAGS) -DDEBUG -DDEBUG_MEM
RELEASE_CFLAGS = $(FULL_CFLAGS) $(RELEASE_FLAGS) -O2
COVERAGE_CFLAGS = $(FULL_CFLAGS) $(COVERAGE_FLAGS)

#
# Files
#

DEBUG_OBJS = \
  $(COMMON_SRC:%.c=$(DEBUG_BUILD_DIR)/%.o) \
  $(SRC:%.c=$(DEBUG_BUILD_DIR)/%.o)
RELEASE_OBJS = \
  $(COMMON_SRC:%.c=$(RELEASE_BUILD_DIR)/%.o) \
  $(SRC:%.c=$(RELEASE_BUILD_DIR)/%.o)
COVERAGE_OBJS = \
  $(COMMON_SRC:%.c=$(COVERAGE_BUILD_DIR)/%.o) \
  $(SRC:%.c=$(COVERAGE_BUILD_DIR)/%.o)

DEBUG_LIB := $(LIB_DIR)/libslavad.a
RELEASE_LIB := $(LIB_DIR)/libslava.a
COVERAGE_LIB := $(LIB_DIR)/build/coverage/libslavac.a

#
# Dependencies
#

DEPS = $(DEBUG_OBJS:%.o=%.d) \
  $(RELEASE_OBJS:%.o=%.d) \
  $(COVERAGE_OBJS:%.o=%.d)

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(DEPS)),)
-include $(DEPS)
endif
endif

$(DEBUG_OBJS): | $(DEBUG_BUILD_DIR)
$(RELEASE_OBJS): | $(RELEASE_BUILD_DIR)
$(COVERAGE_OBJS): | $(COVERAGE_BUILD_DIR)

#
# Rules
#

DEBUG_EXE = $(DEBUG_BUILD_DIR)/$(EXE)
RELEASE_EXE = $(RELEASE_BUILD_DIR)/$(EXE)
COVERAGE_EXE = $(COVERAGE_BUILD_DIR)/$(EXE)

RM_RUN = $(call RUN,rm -f *~,)

debug: slib-debug $(DEBUG_EXE)

release: slib-release $(RELEASE_EXE)

coverage: slib-coverage $(COVERAGE_EXE)

clean:
	$(call RUN,rm -f *~,)
	$(call RUN,rm -fr $(BUILD_DIR),)

cleaner: clean
	$(call RUN,$(SUBMAKE) -C $(LIB_DIR) clean,)

test_banner:
	@echo "===========" $(EXE) "=========== "

test: test_banner debug
	@$(DEBUG_EXE)

valgrind: test_banner debug
	$(call RUN,G_DEBUG=gc-friendly G_SLICE=always-malloc valgrind --tool=memcheck --leak-check=full --show-possibly-lost=yes $(DEBUG_EXE),)

$(DEBUG_BUILD_DIR):
	$(call RUN,mkdir -p $@,"  MKDIR $@")

$(RELEASE_BUILD_DIR):
	$(call RUN,mkdir -p $@,"  MKDIR $@")

$(COVERAGE_BUILD_DIR):
	$(call RUN,mkdir -p $@,"  MKDIR $@")

CC_RUN = $(call RUN,$(call CC_CMD,$1,$2,$3),"  CC    $3")
LD_RUN = $(call RUN,$(LD) $1 $2 $3 $(LIBS) -o $4,"  LD    $@")
CC_CMD = $(CC) -c $1 -MT$3 -MF$(3:%.o=%.d) $2 -o $3
CC_DEBUG = $(call CC_RUN,$(DEBUG_CFLAGS),$1,$2)
CC_RELEASE = $(call CC_RUN,$(RELEASE_CFLAGS),$1,$2)
CC_COVERAGE = $(call CC_RUN,$(COVERAGE_CFLAGS),$1,$2)

$(DEBUG_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(call CC_DEBUG,$<,$@)

$(RELEASE_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(call CC_RELEASE,$<,$@)

$(COVERAGE_BUILD_DIR)/%.o : $(SRC_DIR)/%.c
	$(call CC_COVERAGE,$<,$@)

$(DEBUG_BUILD_DIR)/%.o : $(COMMON_DIR)/%.c
	$(call CC_DEBUG,$<,$@)

$(RELEASE_BUILD_DIR)/%.o : $(COMMON_DIR)/%.c
	$(call CC_RELEASE,$<,$@)

$(COVERAGE_BUILD_DIR)/%.o : $(COMMON_DIR)/%.c
	$(call CC_COVERAGE,$<,$@)

$(DEBUG_EXE): $(DEBUG_LIB) $(DEBUG_BUILD_DIR) $(DEBUG_OBJS)
	$(call LD_RUN,$(DEBUG_LDFLAGS),$(DEBUG_OBJS),$<,$@)

$(RELEASE_EXE): $(RELEASE_LIB) $(RELEASE_BUILD_DIR) $(RELEASE_OBJS)
	$(call LD_RUN,$(RELEASE_LDFLAGS),$(RELEASE_OBJS),$<,$@)
ifeq ($(KEEP_SYMBOLS),0)
	$(call RUN,strip $@,"  STRIP $@")
endif

$(COVERAGE_EXE): $(COVERAGE_LIB) $(COVERAGE_BUILD_DIR) $(COVERAGE_OBJS)
	$(call LD_RUN,$(COVERAGE_LDFLAGS),$(COVERAGE_OBJS),$<,$@)

slib-debug:
	$(call RUN,$(SUBMAKE) $(SUBMAKE_OPTS) -C $(LIB_DIR) debug,)

slib-release:
	$(call RUN,$(SUBMAKE) $(SUBMAKE_OPTS) -C $(LIB_DIR) release)

slib-coverage:
	$(call RUN,$(SUBMAKE) $(SUBMAKE_OPTS) -C $(LIB_DIR) coverage)
