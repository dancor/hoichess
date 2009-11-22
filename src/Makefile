# $Id: Makefile 1462 2007-12-18 20:49:56Z holger $

PLATFORM = unix

CXX = g++
# CXX = /usr/local/bin/g++-snapshot
CPP = $(CXX) -E
INSTALL = /usr/bin/install

LIBPATH =


CXXFLAGS = -W -Wall
LDFLAGS  = 


###############################################################################

-include Makefile.local

#PLATFORM = unix

BUILDDIR         = .build-$(PLATFORM)
BUILDDIR_CHESS   = $(BUILDDIR)/.hoichess
BUILDDIR_XIANGQI = $(BUILDDIR)/.hoixiangqi

SOURCES_COMMON  = $(wildcard *.cc)
SOURCES_CHESS   = $(wildcard chess/*.cc)
SOURCES_XIANGQI = $(wildcard xiangqi/*.cc)

ALL_SOURCES = $(SOURCES_COMMON) $(SOURCES_CHESS) $(SOURCES_XIANGQI)
ALL_HEADERS = $(wildcard *.h chess/*.h xiangqi/*.h)
ALL_OBJS    = $(patsubst %.cc,%.o,$(ALL_SOURCES))

OBJS_COMMON  = $(patsubst %.cc,%.o,$(SOURCES_COMMON))
OBJS_CHESS   = $(patsubst %.cc,%.o,$(SOURCES_CHESS))
OBJS_XIANGQI = $(patsubst %.cc,%.o,$(SOURCES_XIANGQI))

BUILDDIR_OBJS_CHESS   = $(foreach f,$(OBJS_COMMON) $(OBJS_CHESS),$(BUILDDIR_CHESS)/$(f))
BUILDDIR_OBJS_XIANGQI = $(foreach f,$(OBJS_COMMON) $(OBJS_XIANGQI),$(BUILDDIR_XIANGQI)/$(f))


INCLUDE = -I.


#
# Platform-specific stuff
#

ifeq ($(PLATFORM),unix)
override CXXFLAGS += -DHAVE_PTHREAD
LIBS += -lpthread
BIN_CHESS   = $(BUILDDIR)/hoichess
BIN_XIANGQI = $(BUILDDIR)/hoixiangqi
endif

ifeq ($(PLATFORM),mingw32)
override INCLUDE += -Iwin32 -Ilib
SOURCES_COMMON += $(wildcard lib/*.cc win32/*.cc)
ALL_HEADERS += $(wildcard lib/*.h win32/*.h)
BIN_CHESS   = $(BUILDDIR)/hoichess.exe
BIN_XIANGQI = $(BUILDDIR)/hoixiangqi.exe
endif

#
# Feature configurations
#

ifdef HAVE_READLINE
override CXXFLAGS += -DHAVE_READLINE
LIBS += -lreadline
endif

#
# OS-specific stuff
# 

ifeq ($(shell uname -s),SunOS)
override INCLUDE += -Ilib
SOURCES_COMMON += lib/my_getopt.cc
endif

###############################################################################

.PHONY: all
all:
	$(MAKE) hoichess
	$(MAKE) hoixiangqi

.PHONY: hoichess
hoichess: $(BIN_CHESS)

.PHONY: hoixiangqi
hoixiangqi: $(BIN_XIANGQI)


compile.h: $(subst compile.h,,$(ALL_SOURCES) $(ALL_HEADERS))
	touch $@

debug_printconfig.h: config.h build/mk_debug_config_h.sh
	build/mk_debug_config_h.sh > $@


$(BIN_CHESS): INCLUDE += -Ichess
$(BIN_CHESS): CXXFLAGS += -DHOICHESS
$(BIN_CHESS): $(BUILDDIR_OBJS_CHESS)

$(BIN_XIANGQI): INCLUDE += -Ixiangqi
$(BIN_XIANGQI):	CXXFLAGS += -DHOIXIANGQI
$(BIN_XIANGQI): $(BUILDDIR_OBJS_XIANGQI)


# link object files
$(BIN_CHESS) $(BIN_XIANGQI):
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) $(LIBPATH) $(LIBS) -o $@ $^

# compile source files
$(BUILDDIR_CHESS)/%.o $(BUILDDIR_XIANGQI)/%.o: %.cc Makefile Makefile.local
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -MT $@ -MD -MF $(patsubst %.o,%.d,$@) -c -o $@ $<


.PHONY: install
install: all
	$(INSTALL) -m 755 -D $(BIN_CHESS) $(DESTDIR)$(bindir)/$(notdir $(BIN_CHESS))
	$(INSTALL) -m 755 -D $(BIN_XIANGQI) $(DESTDIR)$(bindir)/$(notdir $(BIN_XIANGQI))


.PHONY: clean
clean:
	rm -rf $(BUILDDIR)

.PHONY: maintainer-clean
maintainer-clean: clean
	rm -f debug_printconfig.h
	


# Include dependencies
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),maintainer-clean)
-include $(patsubst %.o,%.d,$(BUILDDIR_OBJS_CHESS))
-include $(patsubst %.o,%.d,$(BUILDDIR_OBJS_XIANGQI))
endif
endif
