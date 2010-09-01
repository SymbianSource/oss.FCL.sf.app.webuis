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
#include <limits.h>
#include "NetworkObserver.h"
#include "SystemInfoPlugin.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CNetworkObserver::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
CNetworkObserver* CNetworkObserver::NewL( CSystemInfoPlugin* aPlugin )
    {
    CNetworkObserver* self = new (ELeave) CNetworkObserver( aPlugin );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CNetworkObserver::CNetworkObserver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CNetworkObserver::CNetworkObserver( CSystemInfoPlugin* aPlugin )
  : iSystemInfoPlugin( aPlugin ),
    iSignalPckg( iSignal ),
    iRegistrationStatusPckg( iRegistrationStatus ),
    iNetworkInfoPckg( iNetworkInfo )
    {
    }

// -----------------------------------------------------------------------------
// CNetworkObserver::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CNetworkObserver::ConstructL()
    {
    iSignalObserver = CSystemInfoObserver::NewL( this, EIdSignalStrength );
    iRegistrationStatusObserver = CSystemInfoObserver::NewL( this, EIdRegistrationStatus );
    iNetworkInfoObserver = CSystemInfoObserver::NewL( this, EIdNetworkInfo );

    iTelephony = CTelephony::NewL();
    iTelephony2 = CTelephony::NewL();
    iTelephony3 = CTelephony::NewL();

    // bootstrap monitoring

    // signal
    iSignal.iSignalStrength = INT_MIN; // default indicates unknown
    iSignal.iBar = CHAR_MIN; // default indicates unknown
    iSignalObserver->RequestNotificationL();

    // registration status
    iRegistrationStatus.iRegStatus = CTelephony::ERegistrationUnknown; // default
    iRegistrationStatusObserver->RequestNotificationL();

    // network info
    iNetworkInfo.iDisplayTag.Copy(_L("")); // default indicates unknown
    iNetworkInfoObserver->RequestNotificationL();
    }

// -----------------------------------------------------------------------------
// CNetworkObserver::~CNetworkObserver()
// Destructor
// -----------------------------------------------------------------------------
CNetworkObserver::~CNetworkObserver()
    {
    delete iSignalObserver;
    delete iRegistrationStatusObserver;
    delete iRegistrationStatusChangedFunction;
    delete iNetworkInfoObserver;
    delete iTelephony;
    delete iTelephony2;
    delete iTelephony3;
    }

// -----------------------------------------------------------------------------
// CNetworkObserver::SignalBars
// -----------------------------------------------------------------------------
TInt CNetworkObserver::SignalBars() const
    {
    if ( !iSignalStarted )
        {
        return INT_MIN; // value indicates unknown
        }
    return iSignal.iBar;
    }

// -----------------------------------------------------------------------------
// CNetworkObserver::RegistrationStatus
// -----------------------------------------------------------------------------
TInt CNetworkObserver::RegistrationStatus() const
    {
    TInt s = iRegistrationStatus.iRegStatus;
    // explicitly map enum values in case CTelephony API changes.
    switch ( s )
        {
    case CTelephony::ERegistrationUnknown:
        s = 0;
        break;
    case CTelephony::ENotRegisteredNoService:
        s = 1;
        break;
    case CTelephony::ENotRegisteredEmergencyOnly:
        s = 2;
        break;
    case CTelephony::ENotRegisteredSearching:
        s = 3;
        break;
    case CTelephony::ERegisteredBusy:
        s = 4;
        break;
    case CTelephony::ERegisteredOnHomeNetwork:
        s = 5;
        break;
    case CTelephony::ERegistrationDenied:
        s = 6;
        break;
    case CTelephony::ERegisteredRoaming:
        s = 7;
        break;
    default:
        s = 0;
        break;
        };

    return s;
    }

// -----------------------------------------------------------------------------
// CNetworkObserver::NetworkName
// -----------------------------------------------------------------------------
const TDesC& CNetworkObserver::NetworkName() const
    {
    return iNetworkInfo.iDisplayTag;
    }

// -----------------------------------------------------------------------------
// CNetworkObserver::RequestNotificationL
// -----------------------------------------------------------------------------
void CNetworkObserver::RequestNotificationL( TIdNetworkItem aId, HBufC8* aFunctionName )
    {
    if ( aId == EIdRegistrationStatus )
        {
        delete iRegistrationStatusChangedFunction;
        iRegistrationStatusChangedFunction = aFunctionName;
        // already monitoring, don't request notification again
        }
    }

// -----------------------------------------------------------------------------
// CNetworkObserver::SubscribeL
// -----------------------------------------------------------------------------
void CNetworkObserver::SubscribeL( TInt aId, TRequestStatus& aStatus )
    {
    if ( aId == EIdSignalStrength )
        {
        if ( iSignalStarted )
            {
            iTelephony->NotifyChange( aStatus,
                                      CTelephony::ESignalStrengthChange,
                                      iSignalPckg );
            }
        else
            {
            iTelephony->GetSignalStrength( aStatus, iSignalPckg );
            }
        }
    else if ( aId == EIdRegistrationStatus )
        {
        if ( iRegistrationStatusStarted )
            {
            iTelephony2->NotifyChange( aStatus,
                                       CTelephony::ENetworkRegistrationStatusChange,
                                       iRegistrationStatusPckg );
            }
        else
            {
            iTelephony2->GetNetworkRegistrationStatus( aStatus, iRegistrationStatusPckg );
            }
        }
    else if ( aId == EIdNetworkInfo )
        {
        if ( iNetworkInfoStarted )
            {
            iTelephony3->NotifyChange( aStatus,
                                       CTelephony::ECurrentNetworkInfoChange,
                                       iNetworkInfoPckg );
            }
        else
            {
            iTelephony3->GetCurrentNetworkInfo( aStatus, iNetworkInfoPckg );
            }
        }
    }

// -----------------------------------------------------------------------------
// CNetworkObserver::UpdateL
// -----------------------------------------------------------------------------
void CNetworkObserver::UpdateL( TInt aId )
    {
    if ( aId == EIdSignalStrength )
        {
        iSignalStarted = 1;
        }
    else if ( aId == EIdRegistrationStatus )
        {
        iRegistrationStatusStarted = 1;
        if ( iRegistrationStatusChangedFunction )
            {
            iSystemInfoPlugin->InvokeCallback( *iRegistrationStatusChangedFunction, NULL, 0 );
            }
        }
    else if ( aId == EIdNetworkInfo )
        {
        iNetworkInfoStarted = 1;
        // FUTURE add callback to script API
        }
    }

// -----------------------------------------------------------------------------
// CNetworkObserver::Cancel
// -----------------------------------------------------------------------------
void CNetworkObserver::Cancel( TInt aId )
    {
    if ( aId == EIdSignalStrength )
        {
        (void) iTelephony->CancelAsync( ( iSignalStarted ?
                                          CTelephony::ESignalStrengthChangeCancel
                                          : CTelephony::EGetSignalStrengthCancel ) );
        }
    else if ( aId == EIdRegistrationStatus )
        {
        (void) iTelephony2->CancelAsync( ( iRegistrationStatusStarted ?
                                           CTelephony::ENetworkRegistrationStatusChangeCancel
                                           : CTelephony::EGetNetworkRegistrationStatusCancel ) );
        }
    else if ( aId == EIdNetworkInfo )
        {
        (void) iTelephony3->CancelAsync( ( iNetworkInfoStarted ?
                                           CTelephony::ECurrentNetworkInfoChangeCancel
                                           : CTelephony::EGetCurrentNetworkInfoCancel ) );
        }
    }

// -----------------------------------------------------------------------------
// CNetworkObserver::Close
// -----------------------------------------------------------------------------
void CNetworkObserver::Close( TInt /*aId*/ )
    {
    }

//  End of File
