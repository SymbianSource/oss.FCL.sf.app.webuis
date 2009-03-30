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
*      API provider.
*  
*
*/


#ifndef __MULTIPLE_WINDOWS_APIPROVIDER_H__
#define __MULTIPLE_WINDOWS_APIPROVIDER_H__

// INCLUDES FILES
#include "WmlBrowserBuild.h"

// FORWARD DECLARATIONS
class CBrCtlInterface;
//class CBrowserDialogsProvider;
class CBrowserDisplay;
class CBrowserLoadObserver;
class CBrowserSoftkeysObserver;
class CBrowserSpecialLoadObserver;
class CBrowserWindowManager;

// CLASS DECLARATION

/**
*  Interface to query several API implementations via Window class.
*  
*  @lib Browser.app
*  @since Series 60 3.0
*/
class MWindowApiProvider
    {
    public:
#if 0
        virtual CBrCtlInterface&             BrCtlInterface() const = 0;
        // virtual CBrowserDialogsProvider&     DialogsProvider() const = 0;
        virtual CBrowserDisplay&             Display() const = 0;
        virtual CBrowserLoadObserver&        LoadObserver() const = 0;
        virtual CBrowserSoftkeysObserver&    SoftkeysObserver() const = 0;
        virtual CBrowserSpecialLoadObserver& SpecialLoadObserver() const = 0;
        virtual CBrowserWindowManager&       WindowMgr() const = 0;
#endif
        virtual TBool IsWindowActive() const = 0;
        virtual TInt WindowId() const = 0;
        virtual const TDesC* TargetName() const = 0;
    };

#endif  // __MULTIPLE_WINDOWS_APIPROVIDER_H__

// End of File
