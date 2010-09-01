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
* Description:  Client Mtm class definition
*
*/



// INCLUDE FILES

#include "PushMtmClient.h"
#include "PushMtmCommands.hrh"
#include "PushMtmProgress.h"
#include "PushMtmCliSrvPanic.h"
#include "PushMtmGcOperation.h"
#include <msvids.h>
#include <msvapi.h>
#include <MTMStore.h>
#include <mtudcbas.h>

// LOCAL CONSTANTS AND MACROS


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMtmClient::NewL
// ---------------------------------------------------------
//
EXPORT_C CPushMtmClient* CPushMtmClient::NewL
                         ( CRegisteredMtmDll& aRegisteredMtmDll, 
                           CMsvSession& aMsvSession )
    {
    CPushMtmClient* client =
        new (ELeave) CPushMtmClient( aRegisteredMtmDll, aMsvSession );
    CleanupStack::PushL( client );
    client->ConstructL();
    CleanupStack::Pop();    // client
    return client;
    }

// ---------------------------------------------------------
// CPushMtmClient::~CPushMtmClient
// ---------------------------------------------------------
//
CPushMtmClient::~CPushMtmClient()
    {
    }

// ---------------------------------------------------------
// CPushMtmClient::CPushMtmClient
// ---------------------------------------------------------
//
CPushMtmClient::CPushMtmClient( CRegisteredMtmDll& aRegisteredMtmDll, 
                                CMsvSession& aMsvSession ) 
:   CBaseMtm( aRegisteredMtmDll, aMsvSession )
    {
    }

// ---------------------------------------------------------
// CPushMtmClient::ConstructL
// ---------------------------------------------------------
//
void CPushMtmClient::ConstructL()
    {
    SwitchCurrentEntryL( KMsvRootIndexEntryId );
    }

// ---------------------------------------------------------
// CPushMtmClient::SaveMessageL
// ---------------------------------------------------------
//
void CPushMtmClient::SaveMessageL()
    {
    User::Leave( KErrNotSupported );
    }

// ---------------------------------------------------------
// CPushMtmClient::LoadMessageL
// ---------------------------------------------------------
//
void CPushMtmClient::LoadMessageL()
    {
    User::Leave( KErrNotSupported );
    }

// ---------------------------------------------------------
// CPushMtmClient::ReplyL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmClient::ReplyL
        (
        TMsvId /*aReplyEntryId*/,
        TMsvPartList /*aPartlist*/,
        TRequestStatus& /*aCompletionStatus*/
        )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmClient::ForwardL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmClient::ForwardL
        (
        TMsvId /*aForwardEntryId*/,
        TMsvPartList /*aPartList*/,
        TRequestStatus& /*aCompletionStatus*/
        )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmClient::AddAddresseeL
// ---------------------------------------------------------
//
void CPushMtmClient::AddAddresseeL( const TDesC& /*aRealAddress*/ )
    {
    User::Leave( KErrNotSupported );
    }

// ---------------------------------------------------------
// CPushMtmClient::AddAddresseeL
// ---------------------------------------------------------
//
void CPushMtmClient::AddAddresseeL( const TDesC& /*aRealAddress*/, 
                                    const TDesC& /*aAlias*/ )
    {
    User::Leave( KErrNotSupported );
    }

// ---------------------------------------------------------
// CPushMtmClient::RemoveAddressee
// ---------------------------------------------------------
//
void CPushMtmClient::RemoveAddressee( TInt /*aIndex*/ )
    {
    // Not suppored.
    }

// ---------------------------------------------------------
// CPushMtmClient::ValidateMessage
// ---------------------------------------------------------
//
TMsvPartList CPushMtmClient::ValidateMessage( TMsvPartList /*aPartList*/ )
    {
	// Not supported. KErrNone means that the current context is valid.
    return KErrNone;
    }

// ---------------------------------------------------------
// CPushMtmClient::Find
// ---------------------------------------------------------
//
TMsvPartList CPushMtmClient::Find( const TDesC& /*aTextToFind*/, 
                                   TMsvPartList /*aPartList*/ )
    {
	// Not supported.
    return 0;
    }

// ---------------------------------------------------------
// CPushMtmClient::QueryCapability
// ---------------------------------------------------------
//
TInt CPushMtmClient::QueryCapability( TUid /*aCapability*/, TInt& /*aResponse*/ )
    {
    return KErrNotSupported; // No supported capabilities here.
    }

// ---------------------------------------------------------
// CPushMtmClient::InvokeSyncFunctionL
// ---------------------------------------------------------
//
void CPushMtmClient::InvokeSyncFunctionL( 
        TInt aFunctionId, 
        const CMsvEntrySelection& aSelection, 
        TDes8& /*aParameter*/ )
	{
    if ( aFunctionId == EPushMtmCmdCollectGarbage )
        {
        TRequestStatus dummyStat; // We call the sync op.
        CPushMtmGcOperation* op = CPushMtmGcOperation::NewL( 
                                            Session(), 
                                            aSelection.Count() ? 
                                                aSelection.At( 0 ) : 
                                                KMsvNullIndexEntryId, 
                                            dummyStat );
        CleanupStack::PushL( op );
        op->DoSyncL();
        CleanupStack::PopAndDestroy( op ); // op
        }
    else
        {
        User::Leave( KErrNotSupported );
        }
	}

// ---------------------------------------------------------
// CPushMtmClient::InvokeAsyncFunctionL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmClient::InvokeAsyncFunctionL(
        TInt aFunctionId,
        const CMsvEntrySelection& aSelection,
        TDes8& /*aParameter*/,
        TRequestStatus& aCompletionStatus )
	{
	CMsvOperation* operation = NULL;

	switch ( aFunctionId )
		{
        case EPushMtmCmdCollectGarbage:
            {
            CPushMtmGcOperation* 
                op = CPushMtmGcOperation::NewL( Session(), 
                                                aSelection.Count() ? 
                                                    aSelection.At( 0 ) : 
                                                    KMsvNullIndexEntryId, 
                                                aCompletionStatus );
            CleanupStack::PushL( op );
            op->StartL();
            operation = op;
            CleanupStack::Pop( op ); // op
            break;
            }

		default:
            {
            User::Leave( KErrNotSupported );
			break;
            }
		}

	return operation;
	}

// ---------------------------------------------------------
// CPushMtmClient::ContextEntrySwitched
// ---------------------------------------------------------
//
void CPushMtmClient::ContextEntrySwitched() 
	{
    // Does nothing 
	}

// End of file.
