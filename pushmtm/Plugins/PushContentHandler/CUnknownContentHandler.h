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
* Description:  Declaration of CUnknownContentHandler.
*
*/



#ifndef __CUNKNOWNCONTENTHANDLER_H__
#define __CUNKNOWNCONTENTHANDLER_H__


// INCLUDE FILES

#include "CPushContentHandlerBase.h"
#include "PushContentHandlerDef.hrh"
#include <E32Base.h>
#include <bldvariant.hrh>

// CONSTANTS

const TUid KUidPushUnknownContentHandler = { EUidPushUnknownContentHandler };
_LIT( KUnknownContentHandlerData, "*" );

// FORWARD DECLARATIONS

class CSmsMessage;
class CUnknownPushMsgEntry;

// CLASS DECLARATION

/**
* CUnknownContentHandler provides the default plugin when the 
* specific plugin for a data type cannot be matched.
*
* If the feature flag __SERIES60_PUSH_SP is defined, then all this 
* kind of messages are discarded on reception.
*/ 
class CUnknownContentHandler : public CPushContentHandlerBase
	{
    public: // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure.
        * @return The constructed object.
        */
	    static CUnknownContentHandler* NewL();

        /**
        * Destructor.
        */
	    virtual ~CUnknownContentHandler();

    private: // Constructors

        /**
        * Constructor.
        */
	    CUnknownContentHandler();

        /**
        * Symbian OS constructor.
        */
	    void ConstructL();

    private: // New functions

        /**
        * Do Garbage Collection synchronously.
        * @return None.
        */
	    void  CollectGarbageL();

#ifdef __SERIES60_PUSH_SP

        /**
        * Create/Saving Push Msg Entry in the appropriate message store.
        * First query the type of push message to check if it is a known BIO
        * message type: 
        *   If it is then 
        *		save Push Message as appropriate BIO message
        *	else
        *		save as UnknownPushMsgEntry.
        * @return None.
        */
	    void  ProcessingPushMsgEntryL();

        /**
        * Check if the given Push Message, which is now owned by the handler,
        * is a BIO Messsage (setting the member variable iBioMsgUID if it is)
        * and returning a boolean to indicate success (ETrue) or EFalse otherwise.
        * @return TBool
        *  ETrue - push message is a BIO message
        *  EFalse - push message is not BIO message
        */
	    TBool BioMessageTypeL();

        /**
        * Save CPushMessage as the relevant BIO message type.
        * Firstly create SMS message and use CPushMessage as
        * source to populate SMS. Then save SMS message.
        * @return None.
        */
	    void  SaveBioMessageEntryL();

        /**
        * Create CMsvEntry for sms messsage, stores the entry and 
        * then delete entry once done.
        * @param aMessage CSmsMessage which embodies details of CPushMessage.
        */
	    void  StoreMsgL( CSmsMessage* aSmsMsg );

        /**
        * Set the service id for Bio messaging
        * @return TMsvId The service id for the message store.
        */
	    TMsvId SetBioServiceIdL();

        /** 
        * Save CPushMessage as a CUnknownPushMsgEntry
        * @return None.
        */
	    void  SaveUnknownPushMsgEntryL();

        /**
        * Sets the Unknown Push Message Entry Fields from the data in
        * the received CPushMessage:
        * @param aUnknownPushMsgEntry CUnknownPushMsgEntry to populate with 
        *        CPushMessage details.
        */
	    void  SetUnknownPushMsgEntryFieldsL( CUnknownPushMsgEntry& 
                                             aUnknownPushMsgEntry );

        /**
        * Extract the file name from the URI.
        * @param aUri The URI.
        * @return The file name part or an empty string.
        */
	    HBufC* FileNameFromUriL( const TDesC& aUri ) const;

#endif // __SERIES60_PUSH_SP

    private: // Methods from CPushHandlerBase

        /**
        * HandleMessage Async. Version. Takes ownership of Push Message and 
        * sets self active to continue processing message.
        * @param aPushMsg CPushMessage to process.
        * @param aStatus Request status variable for use in asynchronous 
        *        operations.
        * @return None.
        */
	    void HandleMessageL( CPushMessage* aPushMsg, TRequestStatus& aStatus );

        /**
        * HandleMessage Sync. Version. Takes ownership of Push Message and 
        * sets self active to continue processing message.
        * Initial State: Set data members then go to the next state 
        * @param aPushMsg CPushMessage to process.
        */
	    void HandleMessageL( CPushMessage* aPushMsg );

        /**
        * Same functionality as DoCancel()
        */
	    void  CancelHandleMessage();

        /**
        * Reserved function.
        */
	    void CPushHandlerBase_Reserved1();

        /**
        * Reserved function.
        */
	    void CPushHandlerBase_Reserved2();
	    
    private: // Methods from CActive

        /**
        * Cancels the handling of the message and revokes the active status
        * of the handler 
        */
	    void DoCancel();

        /**
        * Handler States:
        *    EFilteringAndProcessing - Filter new msg according to Service 
        *                  message settings and Convert CPushMessage to a 
        *                  format to save and save the message (either as Bio 
        *                  msg or UnknownPushMsgEntry)
        *	 EDone		- Clean up, complete observer
        */
	    void RunL();

        /**
        * This is invoked when RunL Leaves with an error.
        * Cleans up and returns.
        * @param aError Error passed into this function
        */
	    TInt RunError( TInt aError );

    private: // Data

	    enum TState
            {
            EGarbageCollecting, 
            EFilteringAndProcessing,
            EDone
            };

        TMsvId              iSavedMsgId; ///< ID of the saved message.
	    TUid				iBioMsgUID; ///< BIO msg UID.
    };

#endif	// __CUNKNOWNCONTENTHANDLER_H__

