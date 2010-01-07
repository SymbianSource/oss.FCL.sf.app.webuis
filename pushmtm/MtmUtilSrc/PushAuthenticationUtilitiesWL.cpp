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
* Description:  Authentication utilities using White List.
*
*/



// INCLUDE FILES

#include "PushAuthenticationUtilities.h"
#include "PushInitiatorList.h"
#include "PushInitiator.h"
#include "PushMtmSettings.h"
#include "PushMtmUtilPanic.h"
#include "PushMtmLog.h"
#include <push/PushMessage.h>
#include <PushEntry.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// TPushAuthenticationUtil::AuthenticateMsgL
// ---------------------------------------------------------
//
EXPORT_C TBool TPushAuthenticationUtil::AuthenticateMsgL
										(
										const CPushMtmSettings& aMtmSettings,
										const CPushMessage& aPushMsg
										)
    {
    PUSHLOG_ENTERFN("TPushAuthenticationUtil::AuthenticateMsgL")

    // In case of white listing by default do not accept messages.
    TBool msgAuthenticated = EFalse;

    // Get server address.
    TPtrC8 srvAddress;
    if ( !aPushMsg.GetServerAddress( srvAddress ) )
        {
	    srvAddress.Set( KNullDesC8 );
        }

    if ( srvAddress.Length() == 0 )
        {
        PUSHLOG_WRITE(" No Server address")
        }
    else
        {
        PUSHLOG_WRITE_FORMAT(" Server address: <%S>",&srvAddress)

        CPushInitiator* sender = new (ELeave) CPushInitiator;
        CleanupStack::PushL( sender );

        // Copy the 8-bit text into a 16-bit one.
        HBufC* from = HBufC::NewMaxLC( srvAddress.Length() );

        from->Des().Copy( srvAddress );
        sender->SetAddressL( *from, CPushInitiator::ETypeAny );

        CleanupStack::PopAndDestroy( from ); // from

        CPushInitiatorList& whiteList = aMtmSettings.PushInitiatorList();
        TInt index( 0 );
        TInt err = whiteList.Find( *sender, index );
        if ( !err )
            {
            // Sender is in White list.
            msgAuthenticated = ETrue;
            PUSHLOG_WRITE(" Is in White List: msg accepted")
            }

        CleanupStack::PopAndDestroy( sender ); // sender
        }

    PUSHLOG_WRITE_FORMAT(" msgAuthenticated <%d>",msgAuthenticated)
    PUSHLOG_LEAVEFN("TPushAuthenticationUtil::AuthenticateMsgL")
    return msgAuthenticated;
    }

// ---------------------------------------------------------
// TPushAuthenticationUtil::AuthenticateMsgL
// ---------------------------------------------------------
//
EXPORT_C TBool TPushAuthenticationUtil::AuthenticateMsgL
										(
										const CPushMtmSettings& aMtmSettings,
										const CPushMsgEntryBase& aPushMsg
										)
    {
    PUSHLOG_ENTERFN("TPushAuthenticationUtil::AuthenticateMsgL 2")

    // In case of white listing by default do not accept messages.
    TBool msgAuthenticated = EFalse;

    // Get server address.
    TPtrC8 srvAddress = aPushMsg.From();

    if ( srvAddress.Length() == 0 )
        {
        PUSHLOG_WRITE(" Empty server address")
        }
    else
        {
        PUSHLOG_WRITE_FORMAT(" Server address: <%S>",&srvAddress)

        CPushInitiator* sender = new (ELeave) CPushInitiator;
        CleanupStack::PushL( sender );

        // Copy the 8-bit text into a 16-bit one.
        HBufC* from = HBufC::NewMaxLC( srvAddress.Length() );

        from->Des().Copy( srvAddress );
        sender->SetAddressL( *from, CPushInitiator::ETypeAny );

        CleanupStack::PopAndDestroy( from ); // from

        CPushInitiatorList& whiteList = aMtmSettings.PushInitiatorList();
        TInt index( 0 );
        TInt err = whiteList.Find( *sender, index );
        if ( !err )
            {
            // Sender is in White list.
            msgAuthenticated = ETrue;
            PUSHLOG_WRITE(" Is in White List: msg accepted")
            }

        CleanupStack::PopAndDestroy( sender ); // sender
        }

    PUSHLOG_WRITE_FORMAT(" msgAuthenticated <%d>",msgAuthenticated)
    PUSHLOG_LEAVEFN("TPushAuthenticationUtil::AuthenticateMsgL 2")
    return msgAuthenticated;
    }

// End of file.
