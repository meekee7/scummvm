MODULE := engines/eba

MODULE_OBJS = \
	eba.o \
	console.o \
	events.o \
	messages.o \
	metaengine.o \
	view.o \
	view1.o \
	view\demo.o \
	view\dragscript.o \
	view\ednabase.o \
	view\ednagirl.o \
	view\ednastd.o \
	view\harvey.o \
	view\intro.o \
	view\mainmenu.o \
	view\scriptonclick.o \
	view\startmenu.o \
	view\zen.o

# This module can be built as a plugin
ifeq ($(ENABLE_EBA), DYNAMIC_PLUGIN)
PLUGIN := 1
endif

# Include common rules
include $(srcdir)/rules.mk

# Detection objects
DETECT_OBJS += $(MODULE)/detection.o
