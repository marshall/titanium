# -*- makefile -*- Time-stamp: <03/10/26 16:04:46 ptr>
# $Id: top.mak 1459 2005-04-18 21:25:32Z ptr $

!ifndef LDFLAGS
LDFLAGS = 
!endif

!include $(RULESBASE)/$(USE_MAKE)/lib/macro.mak
!include $(RULESBASE)/$(USE_MAKE)/lib/$(COMPILER_NAME).mak
!include $(RULESBASE)/$(USE_MAKE)/lib/rules-so.mak
!include $(RULESBASE)/$(USE_MAKE)/lib/rules-a.mak
!include $(RULESBASE)/$(USE_MAKE)/lib/rules-install-so.mak
!include $(RULESBASE)/$(USE_MAKE)/lib/rules-install-a.mak
!include $(RULESBASE)/$(USE_MAKE)/lib/clean.mak
