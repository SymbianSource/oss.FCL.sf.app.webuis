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
#include "PowerObserver.h"
#include "SystemInfoPlugin.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CPowerObserver::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
CPowerObserver* CPowerObserver::NewL( CSystemInfoPlugin* aPlugin )
    {
    CPowerObserver* self = new (ELeave) CPowerObserver( aPlugin );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CPowerObserver::CPowerObserver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CPowerObserver::CPowerObserver( CSystemInfoPlugin* aPlugin )
    : iSystemInfoPlugin( aPlugin ),
      iBatteryInfoPckg( iBatteryInfo ),
      iIndicatorPckg( iIndicator )
    {
    }

// -----------------------------------------------------------------------------
// CPowerObserver::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CPowerObserver::ConstructL()
    {
    iBatteryInfoObserver = CSystemInfoObserver::NewL( this, EIdBatteryInfo );
    iIndicatorObserver = CSystemInfoObserver::NewL( this, EIdIndicator );

    iTelephony = CTelephony::NewL();
    iTelephony2 = CTelephony::NewL();

    // bootstrap monitoring

    // battery
    iBatteryInfo.iChargeLevel = 0; // default indicates unknown
    iBatteryInfoObserver->RequestNotificationL();

    // charger
    iIndicator.iIndicator = 0;
    iIndicatorObserver->RequestNotificationL();
    }

// -----------------------------------------------------------------------------
// CPowerObserver::~CPowerObserver()
// Destructor
// -----------------------------------------------------------------------------
CPowerObserver::~CPowerObserver()
    {
    delete iIndicatorObserver;
    delete iBatteryInfoObserver;
    delete iBatteryInfoChangedFunction;
    delete iIndicatorChangedFunction;
    delete iTelephony2;
    delete iTelephony;
    }

// -----------------------------------------------------------------------------
// CPowerObserver::ChargeLevel
// -----------------------------------------------------------------------------
TInt CPowerObserver::ChargeLevel() const
    {
    if ( !iBatteryInfoStarted )
        {
        // value indicates unknown
        return INT_MIN;
        }
    return STATIC_CAST( TInt, iBatteryInfo.iChargeLevel );
    }

// -----------------------------------------------------------------------------
// CPowerObserver::ChargerConnected
// -----------------------------------------------------------------------------
TInt CPowerObserver::ChargerConnected() const
    {
    if ( !iIndicatorStarted )
        {
        // value indicates unknown
        return INT_MIN;
        }
    return ( CTelephony::KIndChargerConnected & iIndicator.iIndicator ) ? 1 : 0;
    }

// -----------------------------------------------------------------------------
// CPowerObserver::RequestNotificationL
// -----------------------------------------------------------------------------
void CPowerObserver::RequestNotificationL( TIdPowerItem aId, HBufC8* aFunctionName )
    {
    if ( aId == EIdBatteryInfo )
        {
        delete iBatteryInfoChangedFunction;
        iBatteryInfoChangedFunction = aFunctionName;
        // already monitoring, don't request notification again
        }
    else if ( aId == EIdIndicator )
        {
        delete iIndicatorChangedFunction;
        iIndicatorChangedFunction = aFunctionName;
        // already monitoring, don't request notification again
        }
    }

// -----------------------------------------------------------------------------
// CPowerObserver::SubscribeL
// -----------------------------------------------------------------------------
void CPowerObserver::SubscribeL( TInt aId, TRequestStatus& aStatus )
    {
    if ( aId == EIdBatteryInfo )
        {
        if ( iBatteryInfoStarted )
            {
            iTelephony->NotifyChange( aStatus,
                                      CTelephony::EBatteryInfoChange,
                                      iBatteryInfoPckg );
            }
        else
            {
            iTelephony->GetBatteryInfo( aStatus, iBatteryInfoPckg );
            }
        }
    else if ( aId == EIdIndicator )
        {
        if ( iIndicatorStarted )
            {
            iTelephony->NotifyChange( aStatus,
                                      CTelephony::EIndicatorChange,
                                      iIndicatorPckg );
            }
        else
            {
            iTelephony->GetIndicator( aStatus, iIndicatorPckg );
            }
        }
    }

// -----------------------------------------------------------------------------
// CPowerObserver::UpdateL
// -----------------------------------------------------------------------------
void CPowerObserver::UpdateL( TInt aId )
    {
    if ( aId == EIdBatteryInfo )
        {
        iBatteryInfoStarted = 1;
        if ( iBatteryInfoChangedFunction )
            {
            iSystemInfoPlugin->InvokeCallback( *iBatteryInfoChangedFunction, NULL, 0 );
            }
        }
    else if ( aId == EIdIndicator )
        {
        iIndicatorStarted = 1;
        if ( iIndicatorChangedFunction )
            {
            iSystemInfoPlugin->InvokeCallback( *iIndicatorChangedFunction, NULL, 0 );
            }
        }
    }

// -----------------------------------------------------------------------------
// CPowerObserver::Cancel
// -----------------------------------------------------------------------------
void CPowerObserver::Cancel( TInt aId )
    {
    if ( aId == EIdBatteryInfo )
        {
        (void) iTelephony->CancelAsync( ( iBatteryInfoStarted ?
                                          CTelephony::EBatteryInfoChangeCancel
                                          : CTelephony::EGetBatteryInfoCancel ) );
        }
    else if ( aId == EIdIndicator )
        {
        (void) iTelephony->CancelAsync( ( iIndicatorStarted ?
                                          CTelephony::EIndicatorChangeCancel
                                          : CTelephony::EGetIndicatorCancel ) );
        }
    }

// -----------------------------------------------------------------------------
// CPowerObserver::Close
// -----------------------------------------------------------------------------
void CPowerObserver::Close( TInt /*aId*/ )
    {
    }

//  End of File
