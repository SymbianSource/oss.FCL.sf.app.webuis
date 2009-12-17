/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of the License "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*     Common constants
*     
*
*/


#ifndef COMMONCONSTANTS_H
#define COMMONCONSTANTS_H

// INCLUDE FILES
#include <aknconsts.h>

// CONSTANTS

const TUid KUidBrowserContentViewId = { 75 };
const TUid KUidBrowserBookmarksViewId = { 79 };
const TUid KUidBrowserSettingsViewId = { 78 };
const TUid KUidBrowserStartupViewId = { 81 };
const TUid KUidBrowserWindowSelectionViewId = { 83 };
const TUid KUidBrowserInitialViewId = { 84 };
const TUid KUidBrowserFeedsFolderViewId = { 85 };
const TUid KUidBrowserFeedsTopicViewId = { 86 };
const TUid KUidBrowserFeedsFeedViewId = { 87 };

const TUid KUidBrowserNullViewId = { 0 };           ///< null view id used in startup

const TUid KUidCustomMsgDownloadsList = { 0x01 };   ///< jump to download list
const TUid KUidSettingsGotoToolbarGroup = { 0x02 };   ///< jump to toolbar section of settings view
const TUid KUidSettingsGotoShortcutsGroup = { 0x03 };  ///< jump to shortcuts section of settings view

// Granualities for dynamic arrays
const TInt KGranularityLow = 10;        ///< minimum granularity
const TInt KGranularityMedium = 5;
const TInt KGranularityHigh = 2;        ///< maximum gr.

_LIT( KHttpString, "http://" );
_LIT( KFileString, "file://" );
_LIT( KWWWString,  "http://www." );


#define KBrsrMalformedUrl                       -26018
#define KBrsrUnknownScheme                      -26026
#define KBrsrWmlbrowserCardNotInDeck            -26144

_LIT( KWmlValueHttpScheme, "http" );

#define KAvkonMbmPath KAvkonBitmapFile

_LIT( KBrowserDirAndFile,"z:browser.mbm");

_LIT( KWmlValueFile, "file" );
_LIT( KWmlValueFileSlashSlashSlashStr, "file:///" );
// _LIT( KWmlDefaultLocation, "c:" );
// _LIT( KWmlDefaultLocationSlash, "c:/" );
_LIT( KWmlBackSlash, "\\" );

const TInt KWmlEmptyResourceId      = 0;

const TInt KMaxUrlLenghtInOTA       = 255;
const TInt KMaxHomePgUrlLength      = 1000;
const TInt KMaxSearchPgUrlLength    = 1000;
const TInt KAhlePrimaryStorage      = 100; //AHLE primary storage, also used 
                                           //as adaptive bookmarks max list size

// Encodings (character set ids) - no header file is provided with character set
const TUint KCharacterSetIdentifierWindows1250 = 0x100059D6;
const TUint KCharacterSetIdentifierWindows1251 = 0x100059D7;
const TUint KCharacterSetIdentifierWindows1253 = 0x100059D8;
const TUint KCharacterSetIdentifierWindows1254 = 0x100059D9;
const TUint KCharacterSetIdentifierWindows1255 = 0x101F8547;
const TUint KCharacterSetIdentifierWindows1256 = 0x101F8548;
const TUint KCharacterSetIdentifierWindows1257 = 0x100059DA;
const TUint KCharacterSetIdentifierWindows1258 = 0x102073B8;
const TUint KCharacterSetIdentifierTis620      = 0x101f8549;
const TUint KCharacterSetIdentifierWindows874  = 0x101F854A;
const TUint KCharacterSetIdentifierKoi8_r      = 0x101f8778;     // Russian
const TUint KCharacterSetIdentifierKoi8_u      = 0x101f8761;     // Ukrainian
const TUint KCharacterSetIdentifierIscii_temp	   = 0x1027508E;

const TUint KCharacterSetIdentifierEucKr       = 0x2000E526;     // Korean
const TUint KCharacterSetIdentifierKsc5601_temp     = 0x200113CD;     // Korean
const TUint KCharacterSetIdentifierAutomatic   = 0x00;
const TUint KCharacterSetIdentifierUCS2        = 0xffffffff;     // dummy value this id is never checked

#endif	// COMMONCONSTANTS_H
