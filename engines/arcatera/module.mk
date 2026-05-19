MODULE := engines/arcatera

MODULE_OBJS = \
	arcatera.o \
	brgparser.o \
	console.o \
	events.o \
	fmvintroview.o \
	messages.o \
	metaengine.o \
	sceneparser.o \
	view.o

# This module can be built as a plugin
ifeq ($(ENABLE_ARCATERA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
