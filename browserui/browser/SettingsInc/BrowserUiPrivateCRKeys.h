/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/
#ifndef BROWSERUIPRIVATECRKEYS_H
#define BROWSERUIPRIVATECRKEYS_H

#include <BrowserUiSDKCRKeys.h>


// Indicates whether to show connection dialogs or not.
// Valid values: 0 = off, 1 = on
const TUint32 KBrowserConnectionDialogs =   0x00000006;

// Boolean value which determines whether a confirmation 
// query will be shown when exiting Browser.
// Valid values: 0 = off, 1 = on
const TUint32 KBrowserQueryOnExit =         0x0000000A;

// Flags for variating Browser application for common code
// Valid values: integer 0-255
const TUint32 KBrowserUiCommonFlags =       0x00000002;

// Flags for variating Browser application for CDMA branch
// Valid values: integer 0-255
const TUint32 KBrowserUiCdmaFlags =         0x00000003;

// Enabling of the Auto Form Fill feature
// Valid values: 0 = off, 1 = on
const TUint32 KBrowserFormDataSaving =   0x00000022;

// BrowserNG Private Keys

// Browser Adaptive Bookmarks mode setting.
// Valid values: 0 = on, 1 = hide folder, 2 = off
const TUint32 KBrowserNGAdaptiveBookmarks =   0x00000028;

// Determines whether the browser is in Full Screen or 
// Softkeys Only mode.
// Valid values: 0 = normal, 1 = soft keys, 2 = full screen
const TUint32 KBrowserNGFullScreen =          0x00000029;

// Search Page URL setting. A string value. Can be up 
// to 1000 characters in length.
// Valid Valies: up to 1000 characters in length.
const TUint32 KBrowserNGSearchPageURL =       0x0000002A;

// Determines whether pop-up blocking in Multiple Windows is enabled or not.
// Valid values: 0 = off, 1 = on (default = 1)
const TUint32 KBrowserNGPopupBlocking =       0x0000002F;

// Browser's user-defined homepage URL setting.
// Valid values: URL string, up to 1000 characters
const TUint32 KBrowserNGHomepageURL =         0x00000030;

//DEPRECATED/OBSOLETE: Determine if the platform toolbar should be visible next time Browser starts
const TUint32 KBrowserNGShowPlatformToolbar = 0x00000048;

//DEPRECATED/OBSOLETE:  Int value which determines which of the toolbars was last visible. 0 - basic toolbar, 1 - address toolbar
const TUint32 KBrowserNGLastVisibleToolbar = 0x00000049;

// Int value which determines if the Access keys feature is enabled or disabled. 0 - disabled, 1 - enabled
const TUint32 KBrowserNGAccessKeys = 0x00000051; 

#endif      // BROWSERUIPRIVATECRKEYS_H
