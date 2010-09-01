/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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


#ifndef NETWORKOBSERVER_H
#define NETWORKOBSERVER_H

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
*  CNetworkObserver.
*  Interface to information about the radio network connection.
*  @lib npSystemInfoPlugin.lib
*  @since S60 v3.2
*/
NONSHARABLE_CLASS(CNetworkObserver) : public CBase, public MSystemInfoObserverClient
    {
public: // Constructors and destructor

    /**
     * Two-phased constructor.
     */
    static CNetworkObserver* NewL( CSystemInfoPlugin* aPlugin );

    /**
     * Destructor.
     */
    virtual ~CNetworkObserver();

public:
    enum TIdNetworkItem
        {
        EIdSignalStrength,
        EIdRegistrationStatus,
        EIdNetworkInfo
        };

    /**
     * Request network values change notification.
     * @since 3.2
     * @param name of the callback, transfer ownership
     * @return void
     */
    void RequestNotificationL( TIdNetworkItem aId, HBufC8* aFunctionName );

public:

    /**
     * Called to get current network signal bars.
     * @since 3.2
     * @param void
     * @return signal bars
     */
    TInt SignalBars() const;

    /**
     * Called to get current network registration status.
     * @since 3.2
     * @param void
     * @return network registration status
     */
    TInt RegistrationStatus() const;

    /**
     * Called to get current network name
     * @since 3.2
     * @param void
     * @return network name
     */
    const TDesC& NetworkName() const;

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
    CNetworkObserver( CSystemInfoPlugin* aPlugin );

    /**
     * By default Symbian 2nd phase constructor is private.
     * @param none
     * @return void
     */
    void ConstructL();

protected:
    CSystemInfoPlugin* iSystemInfoPlugin; // not owned
    CTelephony* iTelephony;  // owned
    CTelephony* iTelephony2; // owned
    CTelephony* iTelephony3; // owned

    // signal
    CTelephony::TSignalStrengthV1Pckg iSignalPckg;
    CTelephony::TSignalStrengthV1 iSignal;
    CSystemInfoObserver* iSignalObserver; // owned
    TInt iSignalStarted;

    // registration status
    CTelephony::TNetworkRegistrationV1Pckg iRegistrationStatusPckg;
    CTelephony::TNetworkRegistrationV1 iRegistrationStatus;
    CSystemInfoObserver* iRegistrationStatusObserver; // owned
    HBufC8* iRegistrationStatusChangedFunction; // owned
    TInt iRegistrationStatusStarted;

    // network info (includes name)
    CTelephony::TNetworkInfoV2Pckg iNetworkInfoPckg;
    CTelephony::TNetworkInfoV2 iNetworkInfo;
    CSystemInfoObserver* iNetworkInfoObserver; // owned
    TInt iNetworkInfoStarted;
    };

#endif // NETWORKOBSERVER_H

// End of File
