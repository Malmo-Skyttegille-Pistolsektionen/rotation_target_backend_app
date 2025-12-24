APPDIR = $(APPROOTDIR)/app
LIBSDIR = $(APPROOTDIR)/libs
GENERICDIR = $(APPROOTDIR)/generic

include $(APPDIR)/app.makefile
include $(LIBSDIR)/libs.makefile
include $(GENERICDIR)/generic.makefile

CSRC += $(APPROOTDIR)/main.c

INCLUDES += $(APPROOTDIR)/target
