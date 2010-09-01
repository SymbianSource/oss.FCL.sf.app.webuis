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
* Description:  ?Description
*
*/


#ifndef __BROWSERDOWNLOADINDICATORTIMEREVENT_H
#define __BROWSERDOWNLOADINDICATORTIMEREVENT_H

// INCLUDES
//#include <e32std.h>

// CLASS DECLARATION

/**
*  ?one_line_short_description.
*  ?other_description_lines
*
*  @lib Browser.app
*  @since 1.2
*/
class MBrowserDownloadIndicatorTimerEvent 
    {
    public:     // New functions
        
        /**
         * Set initial download animation off.
         * @since 1.2
         */
        virtual void SetBrowserDownloadIndicatorStateOff() = 0;

    };
#endif //__BROWSERDOWNLOADINDICATORTIMEREVENT_H
