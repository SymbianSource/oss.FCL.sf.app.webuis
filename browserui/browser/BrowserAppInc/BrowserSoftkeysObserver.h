/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Handle softkeys updates
*
*/



#ifndef BROWSERSOFTKEYSOBSERVER_H
#define BROWSERSOFTKEYSOBSERVER_H

//  INCLUDES
#include <brctlsoftkeysobserver.h>
#include "WmlBrowserBuild.h"

class CBrowserWindow;

// CLASS DECLARATION

/**
*  This observer is notified when the browser requests softkeys changes
*
*  @lib BrowserEngine.lib
*  @since 2.8
*/
class CBrowserSoftkeysObserver : public CBase, public MBrCtlSoftkeysObserver
    {
    public: // Constructors and destructors

        /**
        * Constructor.
        * @since 2.8
        * @param aView Owner AppUi.
        */
        static CBrowserSoftkeysObserver* NewL(
            CBrowserWindow& aWindow );
            
        /**
        * Destructor.
        */
        virtual ~CBrowserSoftkeysObserver();

    public: // From MBrCtlSoftkeysObserver
        
        /**
        * Browser Control requests to update a softkey
        * @since 2.8
        * @param aKeySoftkey Update the left softkey or the right softkey
        * @param aLabel The label associated with the softkey update
        * @param aCommandId The command to use if the softkey is selected by the user
        * @param aBrCtlSoftkeyChangeReason The reason for the softkey change
        * @return void
        */
        virtual void UpdateSoftkeyL( TBrCtlKeySoftkey aKeySoftkey,
                                     const TDesC& aLabel,
                                     TUint32 aCommandId,
                                     TBrCtlSoftkeyChangeReason aReason );

    protected: // new functions

        /**
        * Default C++ constructor.
        */
        CBrowserSoftkeysObserver(
            CBrowserWindow& aWindow  );

        /**
        * 2nd phase constructor. Leaves on failure.
        */
        void ConstructL();

    private: // data members

        CBrowserWindow* iWindow;     // not owned

    };

#endif      // BROWSERSOFTKEYSOBSERVER_H

// End of File
