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
* Description:  Implementation of CMultiPartMixedContentHandler.
*
*/



// INCLUDE FILES

#include "CMultiPartMixedContentHandler.h"
#include <PushDispatcher.h>
#include <CMultiPartTextIterator.h>
#include <CMultiPartBinIterator.h>

// Constants

_LIT(KReserved, "Reserved");
#if defined(_DEBUG)
_LIT(KErrPushMsgNull,	"NULL CPushMessage");
#endif

/** 
 * Static Factory Construction
 *
 * @param aFs Reference to a file session
 * @param aLibrary Reference to DLL Library Object
 * @param aIndex Index number corresponding to the Unknown App Handler Class 
 *        'EMultiPartMixedContentHandler'
 *
 * @return fully initialized instance of this class
 */
CMultiPartMixedContentHandler* CMultiPartMixedContentHandler::NewL()
	{
	CMultiPartMixedContentHandler* self = 
        new (ELeave) CMultiPartMixedContentHandler;
	CleanupStack::PushL(self);
	self->ConstructL();
	CleanupStack::Pop(self);
	return self;
	}

/** 
 * Destructor
 */
CMultiPartMixedContentHandler::~CMultiPartMixedContentHandler()
	{
    Cancel();
	delete iMultiMessage;
    iContentHandler = NULL; // Not owned.
	}

/**
 * Constructor
 *
 * @param aFs Reference to a file session
 * @param aLibrary Reference to DLL Library Object
 * @param aIndex Index number corresponding to the Unknown App Handler Class 
 *        'EMultiPartMixedContentHandler'
 */ 
CMultiPartMixedContentHandler::CMultiPartMixedContentHandler()
:   CContentHandlerBase()
	{
	}

/**
 *  This will complete initialization of the object
 */
void CMultiPartMixedContentHandler::ConstructL()
	{
	CActiveScheduler::Add(this);
	}

#ifdef __TEST_MULTIPART_MIX_SUPP

/** 
 * Creates a Multipart Binary Iterator that will be used for splitting apart the binary
 * mulitpart.
 *
 * @param aPushMsg A WAP binary multipart.mixed message that will be processed
 */
void CMultiPartMixedContentHandler::LoadMultipartMsgL()
	{
	__ASSERT_DEBUG( iMessage!= 0 , User::Panic(KErrPushMsgNull,0));

	TPtrC contentType;
	iMessage->GetContentType(contentType);
	if (!contentType.FindF(KMultipartText))
		iMultiMessage = CMultipartTextIterator::NewL(*iMessage);
	else if (!contentType.FindF(KMultipartBin))
		iMultiMessage = CMultipartBinIterator::NewL(*iMessage);
	else
		User::Leave(KErrCorrupt);
	
	iMultiMessage->FirstL();
	iState = EHandlePart;
	
	IdleComplete();
	}

/**
 *	Handles the Message Part returned from the Multipart iterator
 *  by creating a new handler based on the Content-Type, then dispatches it
 *  synchronously.
 */
void CMultiPartMixedContentHandler::HandlePartL()
	{
	CPushMessage* msgPart = iMultiMessage->PartL();
	CleanupStack::PushL(msgPart);
	
	TPtrC contentType;
	msgPart->GetContentType(contentType);
	CContentHandlerBase& contentHandler = 
        PushContentTypeDispatcher::GetHandlerL( contentType, *iLog, *iManager );
	iContentHandler = &contentHandler;
	CleanupStack::Pop(msgPart);

	// treat as an acknowledged message always to ensure 
	// sequential creation of handlers
	iContentHandler->HandleMessageL(msgPart, iStatus);
	iState = ENextPart;
	SetActive();
	}

/**
 *	Moves multipart iterator to the next part. If there is none,
 *  State: EDone will be set to complete this handler
 */
void CMultiPartMixedContentHandler::NextPartL()
	{
	if (iMultiMessage->NextL())
		iState = EHandlePart;
	else 
		iState = EDone;
	
	IdleComplete();
	}

#endif // __TEST_MULTIPART_MIX_SUPP

/** 
 * The Asynchronous entry point for this plug-in to handle the CPushMessage.
 * 
 * Since we really didn't start an async event, make it look as though our
 * asynchronous request has been completed. 
 *
 * @param aPushMsg A WAP binary multipart.mixed message that will be processed
 * @param aStatus The TRequestStatus of the caller indicating that this will 
 *        be used aysnchronously
 */
void CMultiPartMixedContentHandler::HandleMessageL( CPushMessage* aPushMsg, 
                                                    TRequestStatus& aStatus )
	{
	__ASSERT_DEBUG( aPushMsg != NULL, User::Panic(KErrPushMsgNull, KErrNone));

#ifdef __TEST_MULTIPART_MIX_SUPP
	iAcknowledge = ETrue;
	iMessage = aPushMsg;
	SetConfirmationStatus(aStatus);
	iState = ELoadMsgData;
	IdleComplete();
#else // __TEST_MULTIPART_MIX_SUPP
	iAcknowledge = ETrue;
	iMessage = aPushMsg;
	SetConfirmationStatus(aStatus);
    // Drop all multipart/mixed messages.
    iState = EDone;
	IdleComplete();
#endif // __TEST_MULTIPART_MIX_SUPP
	}


/** 
 * The Synchronous entry point for this plug-in to handle the CPushMessage.
 *
 * @param aPushMsg A WAP binary multipart.mixed message that will be processed
 * @param aStatus The TRequestStatus of the caller indicating that this will be 
 *        used aysnchronously
 */
void CMultiPartMixedContentHandler::HandleMessageL( CPushMessage* aPushMsg )
	{
	__ASSERT_DEBUG( aPushMsg != NULL, User::Panic(KErrPushMsgNull, KErrNone));

#ifdef __TEST_MULTIPART_MIX_SUPP
	iAcknowledge = EFalse;
	iMessage = aPushMsg;
	iState = ELoadMsgData;
	IdleComplete();
#else // __TEST_MULTIPART_MIX_SUPP
	iAcknowledge = EFalse;
	iMessage = aPushMsg;
    // Drop all multipart messages.
    iState = EDone;
	IdleComplete();
#endif // __TEST_MULTIPART_MIX_SUPP
	}

/** 
 * Cancels Asynronous requests on called handler.
 */
void CMultiPartMixedContentHandler::CancelHandleMessage()
	{
    Cancel();
	}

void CMultiPartMixedContentHandler::CPushHandlerBase_Reserved1()
	{
	User::Panic(KReserved, KErrNotSupported);
	}

void CMultiPartMixedContentHandler::CPushHandlerBase_Reserved2()
	{
	User::Panic(KReserved, KErrNotSupported);
	}

/** 
 * Cancels Asynronous requests on caller and completes self.
 */
void CMultiPartMixedContentHandler::DoCancel()
	{
	if ( iContentHandler )
        {
		iContentHandler->CancelHandleMessage();
        }
	Complete( KErrCancel );
	}

/** 
 * State machine to step through the multipart message until it is done
 *
 * State EHandlePart: Starts the processing of each part of the multipart
 * State ENextPart: Moves the interator to point to the next part of the multipart
 * when there are no more parts, EDone will be the next state
 * State EDone: Clean up and complete
 */
void CMultiPartMixedContentHandler::RunL()
	{
	switch(iState)
		{

#ifdef __TEST_MULTIPART_MIX_SUPP

        case ELoadMsgData:
            {
			LoadMultipartMsgL();
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

#endif // __TEST_MULTIPART_MIX_SUPP

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
TInt CMultiPartMixedContentHandler::RunError(TInt aError)
	{
	iState=EDone;
	Complete(aError);
	return KErrNone;
	}

