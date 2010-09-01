/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Blocks dialog requests to the BrowserDialogsProvider if a
*               window is in the background. When the window comes to the 
*               foreground, it will be displayed.
*
*
*/

#ifndef __BROWSERDIALOGSPROVIDERBLOCKER_H
#define __BROWSERDIALOGSPROVIDERBLOCKER_H

// INCLUDES
#include "BrowserDialogsProviderProxy.h"
#include <e32base.h>

// FORWARD DECLARATIONS
class CBrowserWindowFocusNotifier;

// CLASS DEFINITION
class CBrowserDialogsProviderBlocker : public CActive
    {
    public:  // constructors

        /**
        * Create a CBrowserDialogsProviderBlocker object. Leaves on failure.
        * @param aDlg . Not owned.
        * @return A reference to the created instance of 
        * CBrowserDialogsProviderBlocker
        */
        static CBrowserDialogsProviderBlocker* NewLC(
                            CBrowserWindowFocusNotifier& aWinFocusNotifier );

    private:  // default c++, and 2nd phase constructor

        /**
        * Constructs this object
        * @param aDlg. Not owned.
        */
        CBrowserDialogsProviderBlocker( 
                            CBrowserWindowFocusNotifier& aWinFocusNotifier );

        /**
        * Performs second phase construction of this object
        */
        void ConstructL();

    public:

        /**
        * Destroy the object and release all memory objects
        */
        ~CBrowserDialogsProviderBlocker();

        /**
        * Start an asynchronous request.
        */
        void Start();

    protected: // from CActive

        /**
        * Cancel any outstanding requests
        */
        void DoCancel();

        /**
        * Handles object`s request completion event.
        */
        void RunL();

    private:

        /**
        * Indicates the completion status of a request made to a 
        * service provider.
        */
        //TRequestStatus iStatus;
        
        CActiveSchedulerWait            iWait;              // Asynchronous wait
        
        CBrowserWindowFocusNotifier&    iWinFocusNotifier;  // not owned
    };

#endif  // __BROWSERDIALOGSPROVIDERBLOCKER_H

// End of file
