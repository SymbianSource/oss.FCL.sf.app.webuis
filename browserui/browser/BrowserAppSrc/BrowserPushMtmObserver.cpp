/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  CBrowserPushMtmObserver
*
*/


// INCLUDE FILES
#include "BrowserPushMtmObserver.h"
#include "ApiProvider.h"
#include "BrowserAppUi.h"
#include "CommonConstants.h"
#include "Logger.h"


// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS
// file monitored by browser
_LIT( KPushMtmUrl, "c:\\system\\temp\\PushMtmUrl.txt" );

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBrowserPushMtmObserver::CBrowserPushMtmObserver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBrowserPushMtmObserver::CBrowserPushMtmObserver(MApiProvider* aApiProvider) : CActive( CActive::EPriorityIdle ),
iApiProvider(aApiProvider)
    {
    CActiveScheduler::Add(this);
    }

// -----------------------------------------------------------------------------
// CBrowserPushMtmObserver::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBrowserPushMtmObserver::ConstructL()
    {
    LOG_ENTERFN("CBrowserPushMtmObserver::ConstructL");
    User::LeaveIfError(iFsSession.Connect());
    }

// -----------------------------------------------------------------------------
// CBrowserPushMtmObserver::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBrowserPushMtmObserver* CBrowserPushMtmObserver::NewL( MApiProvider* aApiProvider )
    {
    CBrowserPushMtmObserver* self = new( ELeave ) CBrowserPushMtmObserver(aApiProvider);
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// Destructor
CBrowserPushMtmObserver::~CBrowserPushMtmObserver()
    {
    Cancel();
    iFsSession.Close();
    }

// -----------------------------------------------------------------------------
// CBrowserPushMtmObserver::RunL
//
// -----------------------------------------------------------------------------
//
void CBrowserPushMtmObserver::RunL()
    {
    LOG_ENTERFN("CBrowserPushMtmObserver::RunL");
    HBufC8* url8 = NULL;
    TRAP_IGNORE( url8 = ReadMsgFileL() );
    StartObserver();
    if( url8 != NULL )
        {
        CleanupStack::PushL( url8 );
	    CBrowserAppUi* appUi = STATIC_CAST(CBrowserAppUi*, iApiProvider);
        appUi->ParseAndProcessParametersL( url8->Des() );
        BROWSER_LOG( (_L( "CBrowserPushMtmObserver::RunL appUi->ParseAndProcessParametersL with url8: %S" ), url8 ));
        appUi->SetLastActiveViewId( KUidBrowserContentViewId );
        appUi->SetViewToBeActivatedIfNeededL( appUi->LastActiveViewId() );
        // view activation and bringing the browser to foreground are two 
        // distinct actions.
        appUi->ActivateLocalViewL( KUidBrowserContentViewId );
        appUi->HandleForegroundEventL( ETrue ); 		
        BROWSER_LOG( (_L( "CBrowserPushMtmObserver::RunL appUi->HandleForegroundEventL") ));
        CleanupStack::PopAndDestroy(/*url8*/);
        }
    }

// -----------------------------------------------------------------------------
// CBrowserPushMtmObserver::RunError
//
// -----------------------------------------------------------------------------
//
TInt CBrowserPushMtmObserver::RunError(TInt /*aError*/)
    {
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBrowserPushMtmObserver::DoCancel
//
// -----------------------------------------------------------------------------
//
void CBrowserPushMtmObserver::DoCancel()
    {
    iFsSession.NotifyChangeCancel(iStatus);
    }

// -----------------------------------------------------------------------------
// CBrowserPushMtmObserver::StartObserver
//
// -----------------------------------------------------------------------------
//
void CBrowserPushMtmObserver::StartObserver()
    {
    LOG_ENTERFN("CBrowserPushMtmObserver::StartObserver");
    if (!IsActive())
        {
        iFsSession.NotifyChange( ENotifyWrite, iStatus, KPushMtmUrl() );
        SetActive();
        BROWSER_LOG( (_L( "CBrowserPushMtmObserver::StartObserver iFsSession.NotifyChange") ));
        }
    }


// -----------------------------------------------------------------------------
// CBrowserPushMtmObserver::ReadMsgFileL
//
// -----------------------------------------------------------------------------
//
HBufC8* CBrowserPushMtmObserver::ReadMsgFileL()
    {
    LOG_ENTERFN("CBrowserPushMtmObserver::ReadMsgFileL");

    RFs             rfs;
    RFile           file;
    TInt            size;
    HBufC8*         buffer = NULL;
    TPtr8           bufferPtr(NULL, 0);
    TInt            err = KErrNone;

    // Open the file.
    User::LeaveIfError(rfs.Connect());
    CleanupClosePushL(rfs);

    TInt tryCount = 0;
    for (tryCount = 0; tryCount < 5; tryCount++) 
        {
        err = file.Open( rfs, KPushMtmUrl, EFileRead | EFileShareExclusive );
        if (err == KErrInUse)
            {
            // wait 50 miliseconds and try again
            User::After(50000);
            }
        else
            {
            break;
            }
        }
        
    BROWSER_LOG( (_L( "CBrowserPushMtmObserver::ReadMsgFileL file.Open return: %d" ), err ));
    User::LeaveIfError( err );
    CleanupClosePushL(file);

    // Read file
    err = file.Size(size);
    BROWSER_LOG( (_L( "CBrowserPushMtmObserver::ReadMsgFileL file.Size: %d, err: %d" ), size, err ));
    User::LeaveIfError( err );
    
    buffer = HBufC8::NewLC(size);
    bufferPtr.Set(buffer->Des());

    err = file.Read( bufferPtr, size );
    BROWSER_LOG( (_L( "CBrowserPushMtmObserver::ReadMsgFileL file.Read: %d" ), err ));
    User::LeaveIfError( err );

    // Clean up.
    CleanupStack::Pop(/*buffer*/);
    CleanupStack::PopAndDestroy(/*file*/);

    // don't need to leave due to this error
    err = rfs.Delete( KPushMtmUrl );
    BROWSER_LOG( (_L( "CBrowserPushMtmObserver::ReadMsgFileL rfs.Delete: %d" ), err ));
    CleanupStack::PopAndDestroy(/*rfs*/);

    BROWSER_LOG( (_L( "CBrowserPushMtmObserver::ReadMsgFileL return: %S" ), buffer ));
    return buffer;
    }


//  End of File
