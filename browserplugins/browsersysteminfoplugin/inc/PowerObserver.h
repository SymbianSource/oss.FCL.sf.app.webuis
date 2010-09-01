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


#ifndef POWEROBSERVER_H
#define POWEROBSERVER_H

// INCLUDES
#include <e32base.h>
#include <etel3rdparty.h>
#include <npscript.h>
#include "SystemInfoObserver.h"

// FORWARD DECLARATIONS
class CSystemInfoPlugin;

// CONSTANTS

// State enum and member variable

// CLASS DECLARATIONS
/**
*  CPowerObserver.
*  Interface to information about power and battery.
*  @lib npSystemInfoPlugin.lib
*  @since S60 v3.2
*/
NONSHARABLE_CLASS(CPowerObserver) : public CBase, public MSystemInfoObserverClient
    {
public:
    /**
     * Two-phased constructor.
     */
    static CPowerObserver* NewL( CSystemInfoPlugin* aPlugin );

    /**
     * Destructor.
     */
    virtual ~CPowerObserver();

public:
    enum TIdPowerItem
        {
        EIdBatteryInfo,
        EIdIndicator  // charger connected indicator
        };

    /**
     * Request power values change notification.
     * @since 3.2
     * @param name of the callback
     * @return void
     */
    void RequestNotificationL( TIdPowerItem aId, HBufC8* aFunctionName );

public:
    /**
     * Called to get battery charge level.
     * @since 3.2
     * @param void
     * @return charge level
     */
    TInt ChargeLevel() const;

    /**
     * Called to get charger connected flag.
     * @since 3.2
     * @param void
     * @return 1 if charger connected, 0 if not connected
     */
    TInt ChargerConnected() const;

public:
    // from MSystemInfoObserverClient
    virtual void SubscribeL( TInt aId, TRequestStatus& aStatus );
    virtual void UpdateL( TInt aId );
    virtual void Cancel( TInt aId );
    virtual void Close( TInt aId );

private:
    /**
     * C++ default constructor.
     */
    CPowerObserver( CSystemInfoPlugin* aPlugin );

    /**
     * By default Symbian 2nd phase constructor is private.
     * @param none
     * @return void
     */
    void ConstructL();

public:
    CSystemInfoPlugin* iSystemInfoPlugin;
    CTelephony* iTelephony; // owned
    CTelephony* iTelephony2; // owned

    // battery
    CTelephony::TBatteryInfoV1Pckg iBatteryInfoPckg;
    CTelephony::TBatteryInfoV1 iBatteryInfo;
    CSystemInfoObserver* iBatteryInfoObserver; // owned
    HBufC8* iBatteryInfoChangedFunction;
    TInt iBatteryInfoStarted;

    // charger
    CTelephony::TIndicatorV1Pckg iIndicatorPckg;
    CTelephony::TIndicatorV1 iIndicator;
    CSystemInfoObserver* iIndicatorObserver; // owned
    HBufC8* iIndicatorChangedFunction;
    TInt iIndicatorStarted;
   };

#endif // POWEROBSERVER_H

// End of File
