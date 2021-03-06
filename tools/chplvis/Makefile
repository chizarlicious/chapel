# Copyright 2015 Cray Inc.
# Other additional copyright holders may be indicated within.
# 
# The entirety of this work is licensed under the Apache License,
# Version 2.0 (the "License"); you may not use this file except
# in compliance with the License.
# 
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# chplvis -- a tool to visualize communication and tasks across locales.

# Chapel config

ifndef CHPL_MAKE_HOME
export CHPL_MAKE_HOME=$(shell pwd)/../..
endif

CHPL_MAKE_HOST_TARGET = --target
include $(CHPL_MAKE_HOME)/make/Makefile.base

# FLTK config section

FLTK_CONFIG=$(FLTK_INSTALL_DIR)/bin/fltk-config
FLTK_FLUID=$(FLTK_INSTALL_DIR)/bin/fluid

CHPL_HOME= $(shell printenv CHPL_HOME)
CHPL_HOST_PLATFORM= $(shell printenv CHPL_HOST_PLATFORM)

CXXFLAGS=  -Wall -I. -g

# Suffix rule for compiling .cxx files
.SUFFIXES: .o .h .cxx
.cxx.o:
	$$($(FLTK_CONFIG) --cxx) -c $$($(FLTK_CONFIG) --cxxflags) $(CXXFLAGS) $<


SRCS= ViewField.cxx DataModel.cxx Event.cxx InfoBar.cxx LocaleWin.cxx  CommWin.cxx
HFILES= ViewField.h DataModel.h Event.h InfoBar.h LocaleWin.h CommWin.h

GENSRCS= chplvis.cxx
GENH= chplvis.h

OFILES= $(SRCS:.cxx=.o) $(GENSRCS:.cxx=.o)

all: chplvis

chplvis: $(GENSRCS) $(OFILES) 
	$$($(FLTK_CONFIG) --cxx)  -o chplvis $(OFILES) \
	        $$($(FLTK_CONFIG) --ldflags) $$($(FLTK_CONFIG) --libs)

chplvis.cxx chplvis.h: chplvis.fl
	$(FLTK_FLUID) -c chplvis.fl

clean:
	rm -f $(GENSRCS) $(GENH) $(OFILES) *~

clobber: clean
	rm -f chplvis

install: chplvis
	cp chplvis $(CHPL_HOME)/bin/$(CHPL_HOST_PLATFORM)

# Dependencies
chplvis.o: chplvis.h ViewField.h DataModel.h Event.h InfoBar.h LocaleWin.h CommWin.h
ViewField.o: ViewField.h DataModel.h Event.h
DataModel.o: DataModel.h Event.h
InfoBar.o: InfoBar.h ViewField.h
LocaleWin.o: LocaleWin.h
CommWin.o: CommWin.h
Event.o: Event.h
