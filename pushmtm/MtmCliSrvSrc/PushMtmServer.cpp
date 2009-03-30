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
* Description:  Server Mtm class definition
*
*/



// INCLUDE FILES

#include "PushMtmServer.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMtmServer::NewL
// ---------------------------------------------------------
//
EXPORT_C CPushMtmServer* CPushMtmServer::NewL
    ( CRegisteredMtmDll& aRegisteredMtmDll, CMsvServerEntry* aInitialEntry )
	{
	CPushMtmServer* server =
        new (ELeave) CPushMtmServer( aRegisteredMtmDll, aInitialEntry );
	CleanupStack::PushL( server );
	server->ConstructL();
	CleanupStack::Pop();    // server
	return server;
	}

// ---------------------------------------------------------
// CPushMtmServer::~CPushMtmServer
// ---------------------------------------------------------
//
CPushMtmServer::~CPushMtmServer()
	{
	Cancel();
	}

// ---------------------------------------------------------
// CPushMtmServer::CPushMtmServer
// ---------------------------------------------------------
//
CPushMtmServer::CPushMtmServer
( CRegisteredMtmDll& aRegisteredMtmDll, CMsvServerEntry* aInitialEntry )
	: 	CBaseServerMtm( aRegisteredMtmDll, aInitialEntry ),
		iProgressPckg( iProgress )
	{
	}
		
// ---------------------------------------------------------
// CPushMtmServer::ConstructL
// ---------------------------------------------------------
//
void CPushMtmServer::ConstructL()
	{
	CActiveScheduler::Add( this );
	}

// ---------------------------------------------------------
// CPushMtmServer::CopyToLocalL
// ---------------------------------------------------------
//
void CPushMtmServer::CopyToLocalL( const CMsvEntrySelection& /*aSelection*/,
                            TMsvId /*aDestination*/, TRequestStatus& /*aStatus*/ )
	{
    User::Leave( KErrNotSupported );
	}

// ---------------------------------------------------------
// CPushMtmServer::CopyFromLocalL
// ---------------------------------------------------------
//
void CPushMtmServer::CopyFromLocalL( const CMsvEntrySelection& /*aSelection*/,
                            TMsvId /*aDestination*/, TRequestStatus& /*aStatus*/ )
	{
    User::Leave( KErrNotSupported );
	}

// ---------------------------------------------------------
// CPushMtmServer::CopyWithinServiceL
// ---------------------------------------------------------
//
void CPushMtmServer::CopyWithinServiceL( const CMsvEntrySelection& /*aSelection*/,
                            TMsvId /*aDestination*/, TRequestStatus& /*aStatus*/ )
	{
    User::Leave( KErrNotSupported );
	}

// ---------------------------------------------------------
// CPushMtmServer::MoveToLocalL
// ---------------------------------------------------------
//
void CPushMtmServer::MoveToLocalL( const CMsvEntrySelection& /*aSelection*/,
                            TMsvId /*aDestination*/, TRequestStatus& /*aStatus*/ )
	{
    User::Leave( KErrNotSupported );
	}

// ---------------------------------------------------------
// CPushMtmServer::MoveFromLocalL
// ---------------------------------------------------------
//
void CPushMtmServer::MoveFromLocalL( const CMsvEntrySelection& /*aSelection*/,
                            TMsvId /*aDestination*/, TRequestStatus& /*aStatus*/ )
	{
    User::Leave( KErrNotSupported );
	}

// ---------------------------------------------------------
// CPushMtmServer::MoveWithinServiceL
// ---------------------------------------------------------
//
void CPushMtmServer::MoveWithinServiceL( const CMsvEntrySelection& /*aSelection*/,
                            TMsvId /*aDestination*/, TRequestStatus& /*aStatus*/ )
	{
    User::Leave( KErrNotSupported );
	}

// ---------------------------------------------------------
// CPushMtmServer::DeleteAllL
// ---------------------------------------------------------
//
void CPushMtmServer::DeleteAllL
( const CMsvEntrySelection& /*aSelection*/, TRequestStatus& /*aStatus*/ )
	{
    User::Leave( KErrNotSupported );
	}

// ---------------------------------------------------------
// CPushMtmServer::CreateL
// ---------------------------------------------------------
//
void CPushMtmServer::CreateL
( TMsvEntry /*aNewEntry*/, TRequestStatus& /*aStatus*/ )
    {
    User::Leave( KErrNotSupported );
    }

// ---------------------------------------------------------
// CPushMtmServer::ChangeL
// ---------------------------------------------------------
//
void CPushMtmServer::ChangeL
( TMsvEntry /*aNewEntry*/, TRequestStatus& /*aStatus*/ )
    {
    User::Leave( KErrNotSupported );
    }

// ---------------------------------------------------------
// CPushMtmServer::StartCommandL
// ---------------------------------------------------------
//
void CPushMtmServer::StartCommandL( CMsvEntrySelection& /*aSelection*/,
        TInt /*aCommand*/, const TDesC8& /*aParameter*/, TRequestStatus& /*aStatus*/ )
    {
    User::Leave( KErrNotSupported );
    }

// ---------------------------------------------------------
// CPushMtmServer::CommandExpected
// ---------------------------------------------------------
//
TBool CPushMtmServer::CommandExpected()
    {
    return EFalse;
    }

// ---------------------------------------------------------
// CPushMtmServer::Progress
// ---------------------------------------------------------
//
const TDesC8& CPushMtmServer::Progress()
    {
    return iProgressPckg;
    }

// ---------------------------------------------------------
// CPushMtmServer::DoComplete
// ---------------------------------------------------------
//
void CPushMtmServer::DoComplete( TInt /*aError*/ )
    {
    }

// ---------------------------------------------------------
// CPushMtmServer::DoRunL
// ---------------------------------------------------------
//
void CPushMtmServer::DoRunL()
	{
    }

// ---------------------------------------------------------
// CPushMtmServer::DoCancel
// ---------------------------------------------------------
//
void CPushMtmServer::DoCancel()
    {
    }

//  End of File 
