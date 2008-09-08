# -*- Makefile -*- Time-stamp: <03/10/12 20:35:49 ptr>
# $Id: gcc.mak 1881 2005-11-27 18:57:23Z complement $

SRCROOT := ../..
COMPILER_NAME := gcc

include Makefile.inc
include ${SRCROOT}/Makefiles/top.mak

INCLUDES += -I$(STLPORT_INCLUDE_DIR)

ifeq ($(OSNAME),linux)
DEFS += -D_GNU_SOURCE
endif
