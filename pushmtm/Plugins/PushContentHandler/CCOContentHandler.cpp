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
* Description:  Implementation of CCOContentHandler.
*
*/



// INCLUDE FILES

#include "CCOContentHandler.h"
//#include "PushMtmUtil.h"
//#include "PushMtmSettings.h"
//#include <msvids.h>
//#include <XmlElemt.h>

// CONSTANTS

#if defined(_DEBUG)
_LIT( KErrPushMsgNull, "NULL CPushMessage" );
#endif

_LIT( KReserved, "Reserved" );

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CCOContentHandler::NewL
// ---------------------------------------------------------
//
CCOContentHandler* CCOContentHandler::NewL()
	{
	CCOContentHandler* self = new(ELeave) CCOContentHandler;  
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	return self;
	}

// ---------------------------------------------------------
// CCOContentHandler::~CCOContentHandler
// ---------------------------------------------------------
//
CCOContentHandler::~CCOContentHandler()
	{
    Cancel();
	}

// ---------------------------------------------------------
// CCOContentHandler::CCOContentHandler
// ---------------------------------------------------------
//
CCOContentHandler::CCOContentHandler()
:   CContentHandlerBase()
	{
	}

// ---------------------------------------------------------
// CCOContentHandler::ConstructL
// ---------------------------------------------------------
//
void CCOContentHandler::ConstructL()
	{
	CActiveScheduler::Add(this);
	}

// ---------------------------------------------------------
// CCOContentHandler::HandleMessageL
// ---------------------------------------------------------
//
void CCOContentHandler::HandleMessageL( CPushMessage* aPushMsg, 
                                        TRequestStatus& aStatus )
	{
	__ASSERT_DEBUG( aPushMsg != NULL, User::Panic(KErrPushMsgNull, KErrNone));

	iMessage = aPushMsg;
	iAcknowledge = ETrue;
	SetConfirmationStatus( aStatus );

    iState = EDone;
	IdleComplete();
	}

// ---------------------------------------------------------
// CCOContentHandler::HandleMessageL
// ---------------------------------------------------------
//
void CCOContentHandler::HandleMessageL( CPushMessage* aPushMsg )
	{
	__ASSERT_DEBUG( aPushMsg != NULL, User::Panic(KErrPushMsgNull, KErrNone));
	
	iAcknowledge = EFalse;
	iMessage = aPushMsg;

    iState = EDone;
	IdleComplete();
	}

// ---------------------------------------------------------
// CCOContentHandler::CPushHandlerBase_Reserved1
// ---------------------------------------------------------
//
void CCOContentHandler::CPushHandlerBase_Reserved1()
	{
	User::Panic(KReserved, KErrNotSupported);
	}

// ---------------------------------------------------------
// CCOContentHandler::CPushHandlerBase_Reserved1
// ---------------------------------------------------------
//
void CCOContentHandler::CPushHandlerBase_Reserved2()
	{
	User::Panic(KReserved, KErrNotSupported);
	}

// ---------------------------------------------------------
// CCOContentHandler::CancelHandleMessage
// ---------------------------------------------------------
//
void CCOContentHandler::CancelHandleMessage()
	{
    Cancel();
	}

// ---------------------------------------------------------
// CCOContentHandler::DoCancel
// ---------------------------------------------------------
//
void CCOContentHandler::DoCancel()
	{
	Complete( KErrCancel );
	}

// ---------------------------------------------------------
// CCOContentHandler::RunL
// ---------------------------------------------------------
//
void CCOContentHandler::RunL()
	{
	switch( iState )
		{
        case EDone:
            {
			Complete( KErrNone );
			break;
            }
		default:
			break;
		}
	}

// ---------------------------------------------------------
// CCOContentHandler::RunError
// ---------------------------------------------------------
//
TInt CCOContentHandler::RunError( TInt aError )
	{
	iState = EDone;
	Complete( aError );
	return KErrNone;
	}

