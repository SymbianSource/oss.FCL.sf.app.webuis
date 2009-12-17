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



// INCLUDE FILES
#include "browserlauncher.h"
#include "BrowserLauncherExtension.h"
#include "logger.h"
#include "downloadedcontenthandler.h"
#include <favouriteslimits.h> // Context Id:s

#include <eikenv.h>
#include <AiwGenericParam.h>
#include <s32mem.h>
#include <coecntrl.h>
#include <eikappui.h>




NONSHARABLE_CLASS( CLauncherCommandAbsorbingControl ) : public CCoeControl
    {
    public:
        /**
        * Two-phased constructor.
        */
        static CLauncherCommandAbsorbingControl* NewL( CActiveSchedulerWait& aWaiter, TInt& aExitCode );

        /**
        * Destructor.
        */
        ~CLauncherCommandAbsorbingControl();
        
        /**
        * Set enabled/disabled status
        */
        inline void SetEnabled( TBool aEnabled ) { iEnabled = aEnabled; }

    private:
        /**
        * From CCoeControl
        */
        virtual TKeyResponse OfferKeyEventL( const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/ );

    private:
        /**
        * C++ default constructor.
        */
        CLauncherCommandAbsorbingControl( CActiveSchedulerWait& aWaiter, TInt& aExitCode );

        /**
        * Symbian OS constructor.
        */
        void ConstructL();

    private: // Data
        CEikAppUi* iAppUi;
        CActiveSchedulerWait& iWaiter;
        TBool iEnabled;
        TInt& iExitCode;
    };


// ============================ MEMBER FUNCTIONS ===============================


// ---------------------------------------------------------
// CLauncherCommandAbsorbingControl::CLauncherCommandAbsorbingControl
//
// ---------------------------------------------------------
//
CLauncherCommandAbsorbingControl::CLauncherCommandAbsorbingControl( CActiveSchedulerWait& aWaiter, TInt& aExitCode )
:   iWaiter( aWaiter ), iExitCode( aExitCode )
    {
    }


// ---------------------------------------------------------
// CLauncherCommandAbsorbingControl::~CLauncherCommandAbsorbingControl
//
// ---------------------------------------------------------
//
CLauncherCommandAbsorbingControl::~CLauncherCommandAbsorbingControl()
    {
    if ( iCoeEnv && iAppUi )
        {
        iAppUi->RemoveFromStack( this );
        }
    }


// ---------------------------------------------------------
// CLauncherCommandAbsorbingControl::NewLC
//
// ---------------------------------------------------------
//
CLauncherCommandAbsorbingControl* CLauncherCommandAbsorbingControl::NewL( CActiveSchedulerWait& aWaiter, TInt& aExitCode )
    {
    CLauncherCommandAbsorbingControl* self = new( ELeave ) CLauncherCommandAbsorbingControl( aWaiter, aExitCode );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }



// ---------------------------------------------------------
// CLauncherCommandAbsorbingControl::ConstructL
//
// ---------------------------------------------------------
//
void CLauncherCommandAbsorbingControl::ConstructL()
    {
    iAppUi = iEikonEnv->EikAppUi();
    //
    CreateWindowL();
    SetExtent( TPoint( 0,0 ), TSize( 0,0 ) );
    ActivateL();
    SetPointerCapture( ETrue );
    ClaimPointerGrab( ETrue );
    //
    iAppUi->AddToStackL( this, ECoeStackPriorityDialog );
    }


// ---------------------------------------------------------
// CLauncherCommandAbsorbingControl::OfferKeyEventL
//
// ---------------------------------------------------------
//
TKeyResponse CLauncherCommandAbsorbingControl::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    TKeyResponse response = EKeyWasNotConsumed;
    //  
    if ( iEnabled )
        {
        if ( aKeyEvent.iCode == EKeyEscape && aType == EEventKey )
            {
            if ( iWaiter.IsStarted() )
                {
                iExitCode = KErrCancel;
                iWaiter.AsyncStop();
                }
            }

        response = EKeyWasConsumed;
        }
    //
    return response;
    }

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBrowserLauncherExtension::CBrowserLauncherExtension
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBrowserLauncherExtension::CBrowserLauncherExtension
    ( CBrowserLauncher& aLauncher )
:   CActive( CActive::EPriorityStandard ), iLauncher( aLauncher ), iBufferSizeP( iBufferSize )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherExtension::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBrowserLauncherExtension::ConstructL()
    {
    
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherExtension::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBrowserLauncherExtension* CBrowserLauncherExtension::NewL( CBrowserLauncher& aLauncher )
    {
    LOG_ENTERFN( "CBrowserLauncherExtension::NewL" );
    CBrowserLauncherExtension* self = new (ELeave) CBrowserLauncherExtension( aLauncher );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }

// Destructor
CBrowserLauncherExtension::~CBrowserLauncherExtension()
    {
    LOG_ENTERFN( "CBrowserLauncherExtension::~CBrowserLauncherExtension" );
    Cancel();
    iLauncherClientService.Close();
    delete iReceiveBuffer;
    delete iPackedLauncherData;
    delete iCommandAbsorber;
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherExtension::WaitBrowserToFinish
// -----------------------------------------------------------------------------
//
TInt CBrowserLauncherExtension::WaitBrowserToFinish()
    {
    LOG_ENTERFN( "CBrowserLauncherExtension::WaitBrowserToFinish" );
	if( !iCommandAbsorber )
	  {
	  iCommandAbsorber = CLauncherCommandAbsorbingControl::NewL( iWait, iExitCode );
	  }
    iCommandAbsorber->SetEnabled( ETrue );
    iWait.Start();
    iCommandAbsorber->SetEnabled( EFalse );

    // This class will complete us as an MAknServerAppExitObserver.
    return iExitCode;
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherExtension::SetPackedLauncherData
// -----------------------------------------------------------------------------
//
void CBrowserLauncherExtension::SetPackedLauncherData( HBufC8* aBuf )
    {
    delete iPackedLauncherData;
    iPackedLauncherData = aBuf;
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherExtension::DoCancel
// -----------------------------------------------------------------------------
//
void CBrowserLauncherExtension::DoCancel()
    {
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherExtension::RunL
// -----------------------------------------------------------------------------
//
void CBrowserLauncherExtension::RunL()
    {
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherExtension::RunError
// -----------------------------------------------------------------------------
//
TInt CBrowserLauncherExtension::RunError( TInt aError )
    {
    return aError;
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherExtension::HandleServerAppExit
// -----------------------------------------------------------------------------
//
void CBrowserLauncherExtension::HandleServerAppExit( TInt aReason )
    {
    LOG_ENTERFN( "CBrowserLauncherExtension::HandleServerAppExit" );
    if ( iWait.IsStarted() )
        {
        /* If the Launcher was started synchronously, then 
           iBrowserLauncherClientExitObserver cannot be set => 
           no need to call iBrowserLauncherClientExitObserver->HandleServerAppExit() */

        // Note down the exit reason
        iExitCode = aReason;
        iWait.AsyncStop();
        iCommandAbsorber->SetEnabled( EFalse );
        }
    else if ( iBrowserLauncherClientExitObserver )
        {
        // Forward the exit event
        iBrowserLauncherClientExitObserver->HandleServerAppExit( aReason );
        }
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherExtension::HandleReceivedDataL
// -----------------------------------------------------------------------------
//
void CBrowserLauncherExtension::HandleReceivedDataL( TRequestStatus aStatus )
    {
    LOG_ENTERFN( "CBrowserLauncherExtension::HandleReceivedDataL" );
    
    LOG_WRITE_FORMAT(" aStatus.Int(): %d", aStatus.Int());
    User::LeaveIfError( aStatus.Int() );
    
    //--------------------create buffer for server-----
    // Create buffer to hold data, sent by Browser application.
    // iBufferSize was defined by the server by now...
    delete iReceiveBuffer;
    iReceiveBuffer = 0;
    iReceiveBuffer = HBufC8::NewL( iBufferSize );
    TPtr8 tempRecBuf ( iReceiveBuffer->Des() );
    // Send the allocated descriptor to server, who fills it.
    User::LeaveIfError( iLauncherClientService.SendSyncBuffer( tempRecBuf ) );
    // OK, buffer has been filled.

    //--------------------internalize stream-----------
    // Create Stream object.
    RDesReadStream readStream( tempRecBuf );
    CleanupClosePushL( readStream );
    // Create param list object
    CAiwGenericParamList* paramList = CAiwGenericParamList::NewL( readStream );
    LOG_WRITE(" paramList OK");
    CleanupStack::PopAndDestroy( &readStream );
    CleanupStack::PushL( paramList );
    //--------------------eof internalize stream-------

    TBool clientWantsToContinue( EFalse );
    TBool wasContentHandled = EFalse;

    // If the embedding application handles the downloaded content.
    if ( iDownloadedContentHandler )
        {
        LOG_WRITE_FORMAT(" iDownloadedContentHandler: 0x%x", iDownloadedContentHandler);
        // Extract the file name from the generic paam list
        TInt index(0);
        const TAiwGenericParam* paramFileName = 
            paramList->FindFirst( index, EGenericParamFile, EVariantTypeDesC );
        TPtrC fileName;
        if ( index == KErrNotFound )
            {
            fileName.Set( KNullDesC );
            }
        else
            {
            fileName.Set( paramFileName->Value().AsDes() );
            }

        // Call the embedding app with these parameters, to handle the downloaded
        // content
        TRAPD( handlerLeave, wasContentHandled = 
                                            iDownloadedContentHandler->HandleContentL( 
                                                                        fileName, 
                                                                        *paramList, 
                                                                        clientWantsToContinue ) );
        ++handlerLeave; // To avoid warnings. Used for logging.
        LOG_WRITE_FORMAT(" handlerLeave: %d", handlerLeave);
        }
        
    // Tell the server app the output parameters
    User::LeaveIfError( iLauncherClientService.SendSyncBools
                                 ( clientWantsToContinue, wasContentHandled ) );
    
    // This async request completed, reinitialize it
    iLauncherClientService.SendAsync(iBufferSizeP);
    
    CleanupStack::PopAndDestroy( paramList ); // paramList
    }

// End of file
