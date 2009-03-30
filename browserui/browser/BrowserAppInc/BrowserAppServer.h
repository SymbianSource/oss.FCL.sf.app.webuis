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
* Description:  Header file for CBrowserAppServer
*
*
*/


#ifndef BrowserAppServer_H
#define BrowserAppServer_H

// INCLUDE FILES
#include <e32base.h>
#include <AknServerApp.h>

// CLASS DECLARATION
class CBrowserAppServer : public CAknAppServer
{
public: 
	/**
	* This is a factory function that takes a UID indicating 
	* the type of service that is to be created. The server 
	* application should implement this function to create and 
	* return service implementations for service UIDs that it 
	* recognises, and base call for all other UIDs.
    * @param aServiceType The service object to create
	* @return Object for all service implementations
	*/
	CApaAppServiceBase* CreateServiceL(TUid aServiceType) const; // from CAknAppServer
	
	/**
	* Second-phase constructor
    * @param The server name. ( called by the framework )
	*/	
    void ConstructL(const TDesC& aFixedServerName);
protected:

	/**
	* ( not much info about this method... )
	* Check the capabilities of a client.
    * @param aMsg Object which encapsulates a client request.
    * @param aAction 
    * @param aMissing
	* @return
	*/
	TCustomResult CustomSecurityCheckL(const RMessage2& aMsg, TInt& aAction, TSecurityInfo& aMissing );
	
	/**
	* ( not much info about this method... )	
	* Check the capabilities of a client.
    * @param aMsg Object which encapsulates a client request.
    * @param aAction 
    * @param aMissing
	* @return
	*/	
	TCustomResult CustomFailureActionL(const RMessage2& /*aMsg*/, TInt /*aAction*/, const TSecurityInfo& /*aMissing*/);
	
private:
	/**
	* Document capabilities.
	*/
	TCapability iClientReqs;
};

#endif	// BrowserAppServer_H
// End Of File