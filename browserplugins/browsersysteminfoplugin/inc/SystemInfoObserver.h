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


#ifndef SYSTEMOBSERVER_H
#define SYSTEMOBSERVER_H

// INCLUDES
#include <e32base.h>
#include <npscript.h>

// FORWARD DECLARATIONS

// CONSTANTS

// State enum and member variable

// CLASS DECLARATIONS

/**
 * MSystemInfoObserverClient is an interface to whatever data is being
 * observed (RProperty, int, string, etc).
 * @lib npSystemInfoPlugin.lib
 * @since S60 v3.2
 */
class MSystemInfoObserverClient
    {
public:
    virtual void SubscribeL( TInt aId, TRequestStatus& aStatus ) = 0;
    virtual void UpdateL( TInt aId ) = 0;
    virtual void Cancel( TInt aId ) = 0;
    virtual void Close( TInt aId ) = 0;
/*
 * Outline of use for a publish and subscribe variable
 * #include <e32property.h>
 * #include <hwrmpowerstatesdkpskeys.h>
 * RProperty iProperty;
 * iProperty.Attach( KPSUidHWRMPowerState, KHWRMBatteryLevel );
 * SubscribeL => iProperty.Subscribe( status );
 * UpdateL => iProperty.Get( value );
 * Cancel => iProperty.Cancel();
 * Close => iProperty.Close();
 * The ID parameter is there so that a class can contain
 * multiple observers and differentiate them in these methods by ID.
 * You can also use it for state in a state machine.
 */
    };

/**
*  CSystemInfoObserver.
*  Active class for monitoring variables.
*  @lib npSystemInfoPlugin.lib
*  @since S60 v3.2
*/
NONSHARABLE_CLASS(CSystemInfoObserver) : public CActive
    {
public:
    /**
     * Two-phased constructor.
     */
    static CSystemInfoObserver* NewL( MSystemInfoObserverClient* aClient, TInt aId );

    /**
     * Destructor.
     */
    virtual ~CSystemInfoObserver();

private:
    /**
     * C++ default constructor.
     */
    CSystemInfoObserver( MSystemInfoObserverClient* aClient, TInt aId );

    /**
     * By default Symbian 2nd phase constructor is private.
     * @param none
     * @return void
     */
    void ConstructL();

public:
    /**
     * Request observed status change notification
     * @since 3.2
     * @param none
     * @return void
     */
    void RequestNotificationL();

protected:    // CActive
    /**
     * Implements cancellation of an outstanding request.
     * @param none
     * @return void
     */
    void DoCancel();

    /**
     * Handles an active object’s request completion event.
     * @param none
     * @return void
     */
    void RunL();

    /**
     * Handles an active object’s error
     * @param error
     * @return void
     */
    TInt RunError(TInt aError);

private:
    MSystemInfoObserverClient* iClient;
    TInt iId;
    };

#endif      // SYSTEMOBSERVER_H

// End of File
