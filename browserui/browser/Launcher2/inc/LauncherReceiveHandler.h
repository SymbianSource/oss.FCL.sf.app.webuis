/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Header file for CBrowserAppUi
*
*
*/


#ifndef __LAUNCHERRECEIVEHANDLER_H__
#define __LAUNCHERRECEIVEHANDLER_H__

// INCLUDE FILES 
#include <e32base.h>
#include "Launcherclientservice.h"

// CLASS DECLARATION
class CLauncherReceiveHandler : public CActive
{
public:
	/**
	* Default constructor.
    * @param aHandler Notify events of server exit, and server asyncronous data handling.
    * @param aSession Session of the RLauncherClientService
	*/
    CLauncherReceiveHandler( MLaunchedServerObserver* aHandler, RLauncherClientService& aSession );

	/**
	* Destroy the object and release all memory objects.
	*/
    ~CLauncherReceiveHandler();

	/**
	* Create a CLauncherReceiveHandler object.
    * @param aHandler Notify events of server exit, and server asyncronous data handling.
    * @param aSession Session of the RLauncherClientService
	* @return A pointer to the created instance of CLauncherReceiveHandler
	*/
    static CLauncherReceiveHandler* NewL( MLaunchedServerObserver* aHandler, RLauncherClientService& aSession );

	/**
	* Create a CLauncherReceiveHandler object
    * @param aHandler Notify events of server exit, and server asyncronous data handling.
    * @param aSession Session of the RLauncherClientService
	* @return A pointer to the created instance of CLauncherReceiveHandler
	*/
    static CLauncherReceiveHandler* NewLC( MLaunchedServerObserver* aHandler, RLauncherClientService& aSession );

	/**
	* Second phase constructor.
	*/
    void ConstructL();

	/**
	* Starts listening of the server appliacion, so the client can be notified
	* when the server application sends data.
	*/
    void StartListen();
            
protected: // from CActive
	/**
	* Cancel any outstanding requests
	*/
    void DoCancel();

	/**
	* Respond to an event
	*/
    void RunL();

private:
	/**
	* Observes server application events.
	*/
	MLaunchedServerObserver* iServerObserver; // "uses-a"
	
	/**
	* Reference to the current session.
	*/	
	RLauncherClientService& iSession;         // "uses-a"
};

#endif // __LAUNCHERReceiveHANDLER_H__

// End of File