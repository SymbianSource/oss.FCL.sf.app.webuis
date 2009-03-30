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
* Description: 
*
*/


// INCLUDE FILES
#include "SystemInfoObserver.h"

// ============================ MEMBER FUNCTIONS ===============================


// -----------------------------------------------------------------------------
// CSystemInfoObserver::CSystemInfoObserver
// Two-phased constructor.
// -----------------------------------------------------------------------------
CSystemInfoObserver* CSystemInfoObserver::NewL( MSystemInfoObserverClient* aClient, TInt aId )
    {
    CSystemInfoObserver* self = new ( ELeave ) CSystemInfoObserver( aClient, aId );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CSystemInfoObserver::CSystemInfoObserver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CSystemInfoObserver::CSystemInfoObserver( MSystemInfoObserverClient* aClient, TInt aId )
    : CActive( EPriorityStandard ),
      iClient( aClient ),
      iId( aId )
    {
    }


// -----------------------------------------------------------------------------
// CSystemInfoObserver::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CSystemInfoObserver::ConstructL()
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CSystemInfoObserver::~CSystemInfoObserver()
// Destructor
// -----------------------------------------------------------------------------
CSystemInfoObserver::~CSystemInfoObserver()
    {
    Cancel();
    if ( iClient )
      {
        iClient->Close( iId );
      }
    }

// -----------------------------------------------------------------------------
// CSystemInfoObserver::RequestNotification
// -----------------------------------------------------------------------------
void CSystemInfoObserver::RequestNotificationL()
    {
    if ( !IsActive() )
        {
        iClient->SubscribeL( iId, iStatus );
        SetActive();
        }
    }

// -----------------------------------------------------------------------------
// CSystemInfoObserver::RunL
// -----------------------------------------------------------------------------
void CSystemInfoObserver::RunL()
    {
    if( iStatus == KErrNone )
       {
       iClient->UpdateL( iId );
       iClient->SubscribeL( iId, iStatus );  // FUTURE consider letting client decide if to do this
       SetActive();
       }
    }

// -----------------------------------------------------------------------------
// CSystemInfoObserver::RunError
// -----------------------------------------------------------------------------
TInt CSystemInfoObserver::RunError( TInt /*aError*/ )
    {
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CSystemInfoObserver::DoCancel
// -----------------------------------------------------------------------------
void CSystemInfoObserver::DoCancel()
    {
    iClient->Cancel( iId );
    }

//  End of File
