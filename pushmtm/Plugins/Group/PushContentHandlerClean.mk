#
# Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of the License "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description: 
#
#
# Remove Symbian Push Content Handler DLL and registry resource files.
# These push content handlers are replaced by the content handlers delivered 
# in the Push MTM.
#

LANGUAGE=sc

#
# Files to be removed: ECOM DLLs and registry files.
#

# The old-named push content handler DLL left by PushMtm or Symbian:
TARGETDLL1=pushcontenthandler.dll
# ECOM resource left by Symbian
TARGETRESOURCE1=pushcontenthandler.r$(LANGUAGE)
# SI & SL content handlers left by Symbian
TARGETDLL2=sislcontenthandlers.dll
# ECOM resource left by Symbian (for SI & SL content handlers)
TARGETRESOURCE2=sislcontenthandlers.r$(LANGUAGE)
# Just in case: remove the ECOM registry - the system will create a new
ECOMREGISTRY=$(EPOCROOT)epoc32\$(PLATFORM)\c\system\data\plugins\ecom.rom.dat
# Remove Symbian push mtm
TARGETMTMDLL=wappushmtmclientserver.dll

#
# Target directories
#

ifeq (wins,$(findstring wins, $(PLATFORM)))
	TARGETDIRRESOURCE=$(EPOCROOT)epoc32\release\$(PLATFORM)\$(CFG)\z\resource\plugins
else
endif
TARGETDIRDLL=$(EPOCROOT)epoc32\release\$(PLATFORM)\$(CFG)
GENERICTARGETDIRRESOURCE=$(EPOCROOT)epoc32\data\z\resource\plugins
TARGETMTMDIRDLL=$(EPOCROOT)epoc32\release\$(PLATFORM)\$(CFG)

#
# Remove the files
#

do_tidy:
	if exist "$(TARGETDIRDLL)\$(TARGETDLL1)" del "$(TARGETDIRDLL)\$(TARGETDLL1)"
ifeq (wins,$(findstring wins, $(PLATFORM)))
	if exist "$(TARGETDIRRESOURCE)\$(TARGETRESOURCE1)" \
	del "$(TARGETDIRRESOURCE)\$(TARGETRESOURCE1)"
endif
	if exist "$(GENERICTARGETDIRRESOURCE)\$(TARGETRESOURCE1)" \
	del "$(GENERICTARGETDIRRESOURCE)\$(TARGETRESOURCE1)"
	if exist "$(TARGETDIRDLL)\$(TARGETDLL2)" del "$(TARGETDIRDLL)\$(TARGETDLL2)"
ifeq (wins,$(findstring wins, $(PLATFORM)))
	if exist "$(TARGETDIRRESOURCE)\$(TARGETRESOURCE2)" \
	del "$(TARGETDIRRESOURCE)\$(TARGETRESOURCE2)"
endif
	if exist "$(GENERICTARGETDIRRESOURCE)\$(TARGETRESOURCE2)" \
	del "$(GENERICTARGETDIRRESOURCE)\$(TARGETRESOURCE2)"
ifeq (wins,$(findstring wins, $(PLATFORM)))
	if exist "$(ECOMREGISTRY)" del "$(ECOMREGISTRY)"
endif
	if exist "$(TARGETMTMDIRDLL)\$(TARGETMTMDLL)" del "$(TARGETMTMDIRDLL)\$(TARGETMTMDLL)"

do_nothing:

#
# The targets invoked by abld 
#

MAKMAKE : do_tidy

RESOURCE : do_nothing

SAVESPACE : do_nothing

BLD : do_nothing

FREEZE : do_nothing

LIB : do_nothing

CLEANLIB : do_nothing

FINAL : do_nothing

CLEAN : do_tidy

RELEASABLES : do_nothing

