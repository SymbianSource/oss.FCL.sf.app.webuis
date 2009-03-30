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
*      Internal class for storing a window.
*  
*
*/


#ifndef __MULTIPLE_WINDOWS_WINDOWQUEUE_H__
#define __MULTIPLE_WINDOWS_WINDOWQUEUE_H__

// INCLUDES FILES
#include "WmlBrowserBuild.h"
#include <e32base.h>

// FORWARD DECLARATIONS
class CBrowserWindow;

// CLASS DECLARATION

// multiple linked queue for windows
// iPrev and iNext for stepping through all the queue
// iParent points to the window's parent
class CBrowserWindowQue: public CBase
    {
    public:  // default constructor to have a valid window
        CBrowserWindowQue( CBrowserWindow* aWindow ) : 
             iWindow(aWindow), iParent(NULL),
             iPrev(NULL), iNext(NULL), iStatus( 0 ) {}
        ~CBrowserWindowQue();

    public:
        // returns the last item in the queue
        inline CBrowserWindowQue* LastItem() const ;
        // appends an iten at the end of the queue
        void AppendL( CBrowserWindowQue* aItem );
        // points to the 'next' window which is not deleted
        CBrowserWindowQue* Next() const;
        // points to the 'prev' window which is not deleted
        CBrowserWindowQue* Prev() const;

        
    public:
        enum TWindowStatus {  // bit mask
            EWindowNone = 0,
            EWindowDeleted = 1
        };

    public:  // data members
        CBrowserWindow* iWindow;
        CBrowserWindowQue* iParent;
        CBrowserWindowQue* iPrev;
        CBrowserWindowQue* iNext;
        TInt iStatus;  // bitmask
    };


#endif  // __MULTIPLE_WINDOWS_WINDOWQUEUE_H__

// End of File
