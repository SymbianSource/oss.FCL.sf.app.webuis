/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BrowserLauncherExtension
*
*/



#ifndef BROWSERLAUNCHEREXTENSION_H
#define BROWSERLAUNCHEREXTENSION_H

//  INCLUDES
#include <e32std.h>
#include <e32base.h>
#include <AknServerApp.h>
#include "LauncherClientService.h"

// FORWARD DECLARATIONS
class CBrowserLauncher;
class CLauncherCommandAbsorbingControl;
//class MLaunchedServerObserver;

// CLASS DECLARATION

/**
*  Browser Launcher extension class.
*/
class CBrowserLauncherExtension : public CActive, 
                                  public MLaunchedServerObserver
    {
    public: // Constructors and destructor

        static CBrowserLauncherExtension* NewL( CBrowserLauncher& aLauncher );
        virtual ~CBrowserLauncherExtension();

    public: // New functions

        /**
        * Wait for the Browser to finish the asynchronous execution.
        * This method uses CActiveSchedulerWait!
        * @return Exit code.
        */
        TInt WaitBrowserToFinish();

        /**
        * Set the packed laucher data. Ownership transferred.
        */
        void SetPackedLauncherData( HBufC8* aBuf );

    private: // Functions from CActive

	    virtual void DoCancel();
	    virtual void RunL();
	    virtual TInt RunError( TInt aError );

    private: // From MLaunchedServerObserver

        virtual void HandleServerAppExit( TInt aReason );
        virtual void HandleReceivedDataL( TRequestStatus aStatus );

    private: // Constructors

        CBrowserLauncherExtension( CBrowserLauncher& aLauncher );
        void ConstructL();

    public: // Data members

        CBrowserLauncher& iLauncher; ///< Reference to the Launcher.
        MAknServerAppExitObserver* iBrowserLauncherClientExitObserver; // uses-a
        CActiveSchedulerWait iWait;
        TInt iExitCode;
        MDownloadedContentHandler *iDownloadedContentHandler; // uses-a
        // Buffer to hold the data, that the server sends.
        // The buffer should be allocated by the client, on receiving
        // server HandleReceivedDataL() events.
        HBufC8* iReceiveBuffer; 
        // The size of the buffer, which the client should allocate, to
        // create iReceiveBuffer. This value is set on handling 
        // HandleReceivedDataL() event.
        TInt iBufferSize;
        // The prev member packed into a ptr
        TPckg<TInt> iBufferSizeP;
        RLauncherClientService iLauncherClientService;
        TFileName iDownloadedFileName;
        HBufC8* iPackedLauncherData; // Owned.
        
    private:
        CLauncherCommandAbsorbingControl* iCommandAbsorber;
    };

#endif // BROWSERLAUNCHEREXTENSION_H
