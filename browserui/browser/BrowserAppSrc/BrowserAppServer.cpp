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
* Description: 
*      Implementation of CBrowserAppServer
*
*
*/


// INCLUDES
#include <e32std.h>
#include "BrowserAppServer.h"
#include "BrowserLauncherService.h"

// -----------------------------------------------------------------------------
// CBrowserAppServer::CreateServiceL()
// -----------------------------------------------------------------------------
//
CApaAppServiceBase* CBrowserAppServer::CreateServiceL( TUid aServiceType ) const
    {

	// Create the requested service object.
	if ( aServiceType == TUid::Uid( 0x01010101 ) )
		{
		CBrowserLauncherService* service = new( ELeave ) CBrowserLauncherService;
		CleanupStack::PushL( service );
		service->ConstructL( );
		CleanupStack::Pop( service );
		return service;
		}
	else
		{
		return CAknAppServer::CreateServiceL( aServiceType );
		}

    }

// -----------------------------------------------------------------------------
// CBrowserAppServer::ConstructL()
// -----------------------------------------------------------------------------
//
void CBrowserAppServer::ConstructL( const TDesC& aFixedServerName )
    {
    CAknAppServer::ConstructL( aFixedServerName );
    }

// -----------------------------------------------------------------------------
// CBrowserAppServer::CustomSecurityCheckL()
// -----------------------------------------------------------------------------
//
CPolicyServer::TCustomResult CBrowserAppServer::CustomSecurityCheckL(const RMessage2& aMsg, TInt& aAction, TSecurityInfo& aMissing )
    {/*
    switch (aMsg.Function())
        {
        case RLauncherClientService::EMessageId:
            return aMsg.HasCapability(iClientReqs) ? CPolicyServer::EPass : CPolicyServer::EFail;
        break;

        default:
            return CApaAppServer::CustomSecurityCheckL (aMsg, aAction, aMissing);
        break;
        }*/
    return CApaAppServer::CustomSecurityCheckL (aMsg, aAction, aMissing);
    }

// -----------------------------------------------------------------------------
// CBrowserAppServer::CustomFailureActionL()
// -----------------------------------------------------------------------------
//
CPolicyServer::TCustomResult CBrowserAppServer::CustomFailureActionL(const RMessage2& aMsg, TInt /*aAction*/, const TSecurityInfo& /*aMissing*/)
    {/*
    switch (aMsg.Function())
        {
        case RLauncherClient::EMessageId:
            return aMsg.HasCapability(iClientReqs) ? CPolicyServer::EPass : CPolicyServer::EFail;
        break;

        default:
            return CApaAppServiceBase::CustomFailureActionL (aMsg, aAction, aMissing);
        break;
        }*/
    //Dummy return, because CustomFailureActionL is not implemented.
    return aMsg.HasCapability(iClientReqs) ? CPolicyServer::EPass : CPolicyServer::EFail;
    }
