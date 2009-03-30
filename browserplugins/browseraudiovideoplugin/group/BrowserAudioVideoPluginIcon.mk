#
# Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
# Description:  Generating Browser Audio and Video plugin animation
#		           and scalable icons.
#

ifeq (WINS,$(findstring WINS, $(PLATFORM)))
ZDIR=\epoc32\release\$(PLATFORM)\$(CFG)\Z
else
ZDIR=\epoc32\data\z
endif

TARGETDIR=$(ZDIR)\RESOURCE\APPS
HEADERDIR=\epoc32\include
ICONTARGETFILENAME=$(TARGETDIR)\BrowserAudioVideoPlugin_aif.mif
HEADERFILENAME=$(HEADERDIR)\BrowserAudioVideoPlugin.mbg

do_nothing :
	@rem do_nothing

MAKMAKE : do_nothing

BLD : do_nothing

CLEAN : do_nothing

LIB : do_nothing

CLEANLIB : do_nothing

# ----------------------------------------------------------------------------
#
# NOTE 1: DO NOT DEFINE MASK FILE NAMES! They are included automatically by
# MifConv if the mask depth is defined.
#
# NOTE 2: Usually, source paths should not be included in the bitmap
# definitions. MifConv searches for the icons in all icon directories in a
# predefined order, which is currently \s60\icons, \s60\bitmaps2.
# The directory \s60\icons is included in the search only if the feature flag
# __SCALABLE_ICONS is defined.
# ----------------------------------------------------------------------------

RESOURCE :
	mifconv $(ICONTARGETFILENAME) /h$(HEADERFILENAME) \
		/c8    qgn_graf_video_plugin_anim_buffering00.svg \
		/c8    qgn_graf_video_plugin_anim_buffering01.svg \
		/c8    qgn_graf_video_plugin_anim_buffering02.svg \
		/c8    qgn_graf_video_plugin_anim_buffering03.svg \
		/c8    qgn_graf_video_plugin_anim_buffering04.svg \
		/c8    qgn_graf_video_plugin_anim_buffering05.svg \
		/c8    qgn_graf_video_plugin_anim_buffering06.svg \
		/c8    qgn_graf_video_plugin_anim_buffering07.svg \
		/c8    qgn_graf_video_plugin_anim_buffering08.svg \
		/c8    qgn_graf_video_plugin_anim_buffering09.svg \
		/c8    qgn_graf_video_plugin_anim_buffering10.svg \
		/c8    qgn_graf_video_plugin_anim_buffering11.svg \
		/c8    qgn_graf_video_plugin_anim_audio_playing00.svg \
		/c8    qgn_graf_video_plugin_anim_audio_playing01.svg \
		/c8    qgn_graf_video_plugin_anim_audio_playing02.svg \
		/c8    qgn_graf_video_plugin_anim_audio_playing03.svg \
		/c8    qgn_graf_video_plugin_anim_audio_playing04.svg \
		/c8    qgn_graf_video_plugin_anim_audio_playing05.svg \
		/c8    qgn_graf_video_plugin_anim_audio_playing06.svg \
		/c8    qgn_graf_video_plugin_anim_audio_playing07.svg \
		/c8    qgn_graf_video_plugin_anim_audio_playing08.svg \
		/c8    qgn_graf_video_plugin_anim_audio_playing09.svg \
		/c8    qgn_graf_video_plugin_anim_audio_playing10.svg \
		/c8    qgn_graf_video_plugin_anim_audio_playing11.svg \
		/c8    qgn_graf_video_plugin_anim_video_stopped00.svg \
		/c8    qgn_graf_video_plugin_anim_video_stopped01.svg \
		/c8    qgn_graf_video_plugin_anim_video_stopped02.svg \
		/c8    qgn_graf_video_plugin_anim_video_stopped03.svg \
		/c8    qgn_graf_video_plugin_anim_video_stopped04.svg \
		/c8    qgn_graf_video_plugin_anim_video_stopped05.svg \
		/c8    qgn_graf_video_plugin_anim_video_stopped06.svg \
		/c8    qgn_graf_video_plugin_anim_video_stopped07.svg \
		/c8    qgn_graf_video_plugin_anim_video_stopped08.svg \
		/c8    qgn_graf_video_plugin_anim_video_stopped09.svg \
		/c8    qgn_graf_video_plugin_anim_video_stopped10.svg \
		/c8    qgn_graf_video_plugin_anim_video_stopped11.svg \
		/c8    qgn_graf_video_plugin_anim_video_paused00.svg \
		/c8    qgn_graf_video_plugin_anim_video_paused01.svg \
		/c8    qgn_graf_video_plugin_anim_video_paused02.svg \
		/c8    qgn_graf_video_plugin_anim_video_paused03.svg \
		/c8    qgn_graf_video_plugin_anim_video_paused04.svg \
		/c8    qgn_graf_video_plugin_anim_video_paused05.svg \
		/c8    qgn_graf_video_plugin_anim_video_paused06.svg \
		/c8    qgn_graf_video_plugin_anim_video_paused07.svg \
		/c8    qgn_graf_video_plugin_anim_video_paused08.svg \
		/c8    qgn_graf_video_plugin_anim_video_paused09.svg \
		/c8    qgn_graf_video_plugin_anim_video_paused10.svg \
		/c8    qgn_graf_video_plugin_anim_video_paused11.svg \
		/c8    qgn_graf_video_plugin_anim_audio_stopped00.svg \
		/c8    qgn_graf_video_plugin_anim_audio_stopped01.svg \
		/c8    qgn_graf_video_plugin_anim_audio_stopped02.svg \
		/c8    qgn_graf_video_plugin_anim_audio_stopped03.svg \
		/c8    qgn_graf_video_plugin_anim_audio_stopped04.svg \
		/c8    qgn_graf_video_plugin_anim_audio_stopped05.svg \
		/c8    qgn_graf_video_plugin_anim_audio_stopped06.svg \
		/c8    qgn_graf_video_plugin_anim_audio_stopped07.svg \
		/c8    qgn_graf_video_plugin_anim_audio_stopped08.svg \
		/c8    qgn_graf_video_plugin_anim_audio_stopped09.svg \
		/c8    qgn_graf_video_plugin_anim_audio_stopped10.svg \
		/c8    qgn_graf_video_plugin_anim_audio_stopped11.svg \
		/c8    qgn_graf_video_plugin_anim_audio_paused00.svg \
		/c8    qgn_graf_video_plugin_anim_audio_paused01.svg \
		/c8    qgn_graf_video_plugin_anim_audio_paused02.svg \
		/c8    qgn_graf_video_plugin_anim_audio_paused03.svg \
		/c8    qgn_graf_video_plugin_anim_audio_paused04.svg \
		/c8    qgn_graf_video_plugin_anim_audio_paused05.svg \
		/c8    qgn_graf_video_plugin_anim_audio_paused06.svg \
		/c8    qgn_graf_video_plugin_anim_audio_paused07.svg \
		/c8    qgn_graf_video_plugin_anim_audio_paused08.svg \
		/c8    qgn_graf_video_plugin_anim_audio_paused09.svg \
		/c8    qgn_graf_video_plugin_anim_audio_paused10.svg \
		/c8    qgn_graf_video_plugin_anim_audio_paused11.svg \
		/c8    qgn_graf_video_plugin_anim_no_video00.svg \
		/c8    qgn_graf_video_plugin_anim_no_video01.svg \
		/c8    qgn_graf_video_plugin_anim_no_video02.svg \
		/c8    qgn_graf_video_plugin_anim_no_video03.svg \
		/c8    qgn_graf_video_plugin_anim_no_video04.svg \
		/c8    qgn_graf_video_plugin_anim_no_video05.svg \
		/c8    qgn_graf_video_plugin_anim_no_video06.svg \
		/c8    qgn_graf_video_plugin_anim_no_video07.svg \
		/c8    qgn_graf_video_plugin_anim_no_video08.svg \
		/c8    qgn_graf_video_plugin_anim_no_video09.svg \
		/c8    qgn_graf_video_plugin_anim_no_video10.svg \
		/c8    qgn_graf_video_plugin_anim_no_video11.svg \
		/c8    qgn_graf_video_plugin_anim_no_buffering_broken00.svg \
		/c8    qgn_graf_video_plugin_anim_no_buffering_broken01.svg \
		/c8    qgn_graf_video_plugin_anim_no_buffering_broken02.svg \
		/c8    qgn_graf_video_plugin_anim_no_buffering_broken03.svg \
		/c8    qgn_graf_video_plugin_anim_no_buffering_broken04.svg \
		/c8    qgn_graf_video_plugin_anim_no_buffering_broken05.svg \
		/c8    qgn_graf_video_plugin_anim_no_buffering_broken06.svg \
		/c8    qgn_graf_video_plugin_anim_no_buffering_broken07.svg \
		/c8    qgn_graf_video_plugin_anim_no_buffering_broken08.svg \
		/c8    qgn_graf_video_plugin_anim_no_buffering_broken09.svg \
		/c8    qgn_graf_video_plugin_anim_no_buffering_broken10.svg \
		/c8    qgn_graf_video_plugin_anim_no_buffering_broken11.svg
	
FREEZE : do_nothing

SAVESPACE : do_nothing

RELEASABLES :
	@echo $(HEADERFILENAME)&& \
	@echo $(ICONTARGETFILENAME)

FINAL : do_nothing
