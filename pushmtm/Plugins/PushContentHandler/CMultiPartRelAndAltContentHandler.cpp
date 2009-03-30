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
* Description:  Implementation of CMultiPartRelAndAltContentHandler.
*
*/



// INCLUDE FILES

#include "CMultiPartRelAndAltContentHandler.h"
#include <CMultiPartIteratorBase.h>
#include <msvids.h>

// Constants

_LIT(KReserved, "Reserved");

/**
 * Static Factory Construction
 *
 * version of NewLC which leaves nothing
 * on the cleanup stack
 */
CMultiPartRelAndAltContentHandler* CMultiPartRelAndAltContentHandler::NewL()
	{
	CMultiPartRelAndAltContentHandler* self = new (ELeave) CMultiPartRelAndAltContentHandler;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/**
 * Default d'tor
 */
CMultiPartRelAndAltContentHandler::~CMultiPartRelAndAltContentHandler()
	{
    Cancel();
	delete iMultiMessage;
    delete iMsvSession;
	}

/**
 * The Multipart Related Content handler 
 * Index number : ESLContentHandlerIndex 
 */ 
CMultiPartRelAndAltContentHandler::CMultiPartRelAndAltContentHandler()
:   CContentHandlerBase(), 
    iSavedMsgId( KMsvNullIndexEntryId )
	{
	}

/**
 *  This will complete initialization of the object
 */
void CMultiPartRelAndAltContentHandler::ConstructL()
	{
    iMsvSession = CMsvSession::OpenSyncL( *this );
	CActiveScheduler::Add(this);
	}

#ifdef __TEST_MULTIPART_REL_SUPP

/**
* Setup the multipart message.
*/
void CMultiPartRelAndAltContentHandler::LoadMultiPartMsgL()
	{
#ifdef _DEBUG
	_LIT(KNullMsg,"NULL CPushMessage");
	__ASSERT_DEBUG( iMessage != 0 , User::Panic(KNullMsg,0));
#endif
	TPtrC contentType;
	iMessage->GetContentType(contentType);
	if (!contentType.FindF(KMultipartText))
		iMultiMessage = CMultipartTextIterator::NewL(*iMessage);
	else if (!contentType.FindF(KMultipartBin))
		iMultiMessage = CMultipartBinIterator::NewL(*iMessage);
	else
		User::Leave(KErrCorrupt);
	iMultiMessage->FirstL();
	IdleComplete();
	}

/**
 *	Saves message part to messaging server
 */
void CMultiPartRelAndAltContentHandler::HandlePartL()
	{
	CPushMessage* msgPart = iMultiMessage->PartL();
	CleanupStack::PushL(msgPart);

	// Create a new Unknown Push Entry to hold the header and body data
	CUnknownPushMsgEntry* msgEntry=CUnknownPushMsgEntry::NewL();
	CleanupStack::PushL(msgEntry);
	// Get the header
	TPtrC8 header;
	msgPart->GetHeader(header);
	//Get the From field
	TPtrC8 from;
	if (!msgPart->GetBinaryHeaderField(EHttpFrom,from) &&
		!msgPart->GetBinaryHeaderField(EHttpXWapInitiatorURI,from) &&
		!msgPart->GetBinaryHeaderField(EHttpContentLocation,from) )
		{
		from.Set(KNullDesC8);
		}
	// Get the body
	TPtrC8 body;
	msgPart->GetMessageBody(body);
	TPtrC content;
	msgPart->GetContentType(content);

	//Set fields of the Unknown Entry
	msgEntry->SetHeaderL(header);
	msgEntry->SetMessageDataL(body);	
	msgEntry->SetFromL(from);
	//Need this next  bit so UI knows what to do with the data in the Push Entry
	msgEntry->SetContentTypeL( content );

	msgEntry->SaveL( *iMsvSession, iSavedMsgId );
	CleanupStack::PopAndDestroy( 2 ); //msgEntry, msgPart
	
	iState = ENextPart;
	IdleComplete();

	}

/**
 *	Move multipart iterator to the next part
 */
void CMultiPartRelAndAltContentHandler::NextPartL()
	{
	if (iMultiMessage->NextL())
		{
		iState = EHandlePart;
		}
	else 
		{
		iState = EDone;
		}
	IdleComplete();
	}

/**
 * 
 */
void CMultiPartRelAndAltContentHandler::SaveMsgRootL()
	{
	TPtrC8 msgHeaderPtr;
	iMessage->GetHeader(msgHeaderPtr);
	CMultiPartPushMsgEntry* msgEntry=CMultiPartPushMsgEntry::NewL();
	CleanupStack::PushL(msgEntry);
	msgEntry->SetHeaderL(msgHeaderPtr); //Top level entry
	TPtrC contentType;
	iMessage->GetContentType(contentType);
	msgEntry->SetContentTypeL(contentType);

	iSavedMsgId = msgEntry->SaveL(*iMsvSession, 
                                  KMsvGlobalInBoxIndexEntryId);
	CleanupStack::PopAndDestroy();//msgEntry
	}

/**
 * Loads multipart data and creates multipart head entry in the message 
 * server index.
 * @throw KErrCorrupt - message data is corrupt 
 * @throw KErrNoMemory - insufficient free memory to complete operations
 */

void CMultiPartRelAndAltContentHandler::LoadMsgDataL()
	{
	LoadMultiPartMsgL();
	SaveMsgRootL();
	iState = EHandlePart;
	}

void CMultiPartRelAndAltContentHandler::SetMsgCorruptFlagL()
	{
	CMsvEntry* msvEntry = iMsvSession->GetEntryL( iSavedMsgId );
	CleanupStack::PushL( msvEntry );
	TMsvEntry entry = msvEntry->Entry();
	// get the iMtmData1 value & zero bits 0-3 then add new status
	TInt32 mtmdata1 = ( entry.iMtmData1 & 0xFFFFFFF0 );
	mtmdata1 += CPushMsgEntryBase::EPushMsgStatusCorrupt;
	// Now set values for TMsvEntry and update the server entry
	entry.iMtmData1 = mtmdata1;
	msvEntry->ChangeL( entry );

	CleanupStack::PopAndDestroy( msvEntry );
	}

#endif // __TEST_MULTIPART_REL_SUPP

/**
 * Async. Version
 */
void CMultiPartRelAndAltContentHandler::HandleMessageL( CPushMessage* aPushMsg, 
                                                        TRequestStatus& aStatus )
	{
#ifdef __TEST_MULTIPART_REL_SUPP
	iAcknowledge=ETrue;
	SetConfirmationStatus(aStatus);
	iMessage = aPushMsg;
	iState = ELoadMsgData;
	IdleComplete();
#else // __TEST_MULTIPART_REL_SUPP
	iAcknowledge=ETrue;
	SetConfirmationStatus(aStatus);
	iMessage = aPushMsg;
    // Drop all multipart messages.
    iState = EDone;
	IdleComplete();
#endif // __TEST_MULTIPART_REL_SUPP
	}

/**
 * Sync. Version
 */
void CMultiPartRelAndAltContentHandler::HandleMessageL( CPushMessage* aPushMsg )
	{
#ifdef __TEST_MULTIPART_REL_SUPP
	iAcknowledge=EFalse;
	iMessage = aPushMsg;
	iState = ELoadMsgData;
	IdleComplete();
#else // __TEST_MULTIPART_REL_SUPP
	iAcknowledge=EFalse;
	iMessage = aPushMsg;
    // Drop all multipart messages.
    iState = EDone;
	IdleComplete();
#endif // __TEST_MULTIPART_REL_SUPP
	}

void CMultiPartRelAndAltContentHandler::CancelHandleMessage()
	{
    Cancel();
	}

void CMultiPartRelAndAltContentHandler::CPushHandlerBase_Reserved1()
	{
	User::Panic(KReserved, KErrNotSupported);
	}

void CMultiPartRelAndAltContentHandler::CPushHandlerBase_Reserved2()
	{
	User::Panic(KReserved, KErrNotSupported);
	}

void CMultiPartRelAndAltContentHandler::DoCancel()
	{
	Complete( KErrCancel );
	}

/**
*RunL handles each part separately
*/
void CMultiPartRelAndAltContentHandler::RunL()
	{
	switch(iState)
		{

#ifdef __TEST_MULTIPART_REL_SUPP

        case ELoadMsgData:
            {
			LoadMsgDataL();
			break;
            }
		case EHandlePart:
            {
			HandlePartL();
			break;
            }
		case ENextPart:
            {
			NextPartL();
			break;
            }

#endif // __TEST_MULTIPART_REL_SUPP

        case EDone:
            {
			Complete(KErrNone);
			break;
            }
		default:
            {
			break;
            }
		}
	}

/** 
 * Clean up
 */
TInt CMultiPartRelAndAltContentHandler::RunError(TInt aError)
	{
	iState=EDone;

#ifdef __TEST_MULTIPART_REL_SUPP

    if (iSavedMsgId != KMsvNullIndexEntryId)
		{
		TRAPD(error, SetMsgCorruptFlagL());
		}

#endif // __TEST_MULTIPART_REL_SUPP

	Complete(aError);
	return KErrNone;
	}

// ---------------------------------------------------------
// CMultiPartRelAndAltContentHandler::HandleSessionEventL
// ---------------------------------------------------------
//
void CMultiPartRelAndAltContentHandler::HandleSessionEventL( 
                                             TMsvSessionEvent /*aEvent*/, 
                                             TAny* /*aArg1*/, 
                                             TAny* /*aArg2*/, 
                                             TAny* /*aArg3*/ )
    {}

