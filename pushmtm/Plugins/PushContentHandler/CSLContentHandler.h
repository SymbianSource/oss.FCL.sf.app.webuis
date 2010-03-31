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
* Description:  Declaration of CSLContentHandler.
*
*/



#ifndef __CSLCONTENTHANDLER_H__
#define __CSLCONTENTHANDLER_H__

// INCLUDE FILES

#include "CPushContentHandlerBase.h"
#include "PushMtmDef.hrh"
#include "PushContentHandlerDef.hrh"
#include <E32Base.h>
#include <bldvariant.hrh>
#include <nw_dom_attribute.h>

// CONSTANTS

const TUid KUidPushSLContentHandler	= { EUidPushSLContentHandler };
_LIT( KSLContentHandlerData, "text/vnd.wap.sl||application/vnd.wap.slc" );

// FORWARD DECLARATIONS

class CSLPushMsgEntry;
class CPushMtmAutoFetchOperation;

// CLASS DECLARATION

/** 
* CSLContentHandler Handles Service Loading Content  
*
* This handler takes ownership of the SL message and processes it
* according to the Reception rules 
* [WAP Service Loading version 31-July-2001]
* and either stores the message or deletes it.
* Also S60 specific requirements are applied.
*
* NB. If __SERIES60_PUSH_SL is not defined, then all SL push messages are 
*     discarded!
*/
class CSLContentHandler : public CPushContentHandlerBase
	{
    public: // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure.
        * @return The constructed object.
        */
	    static CSLContentHandler* NewL();

        /**
        * D'tor.
        */
	    virtual ~CSLContentHandler();

    private: // Constructors

        /**
        * Constructor.
        */
	    CSLContentHandler();

        /**
        * Symbian OS constructor.
        */
	    void ConstructL();

    private: // New functions

        /**
        * Do Garbage Collection synchronously when a new message arrives.
        * @return None.
        */
	    void  CollectGarbageL();


        /**
        * Parse the Push SL message using XML parser.
        * If Push Message is an SLC then convert it first to text using 
        * CWbxmlConverterUtil class.
        * @return None.
        */
	    void ParsePushMsgL();

        /**
        * Parse an attribute of the sl element.
        * @param aAttrHandle The attribute to be parsed.
        * @return None.
        */
	    void  ParseSlAttributeL( NW_DOM_AttributeHandle_t& aAttrHandle );

        /**
        * Convert the action string to a representative numeric value to 
        * facilitate storing the message.
        * @param aActionString The attribute value indicating the action level 
        *        eg 'execute-high'
        * @return TUint: a value representing the action type.
        */
	    TUint ConvertActionString( const TDesC8& aActionString ) const;

        /**
        * Set SL entry fields prior to storing message.
        * @param aSlPushMsgEntry Entry represents message format to use when 
        *        storing it.
        * @return None.
        */
	    void SetSlPushMsgEntryFieldsL( CSLPushMsgEntry& aSlPushMsgEntry ) const;

        /**
        * Check if the message has to be discarded due to empty Href or 
        * there is a message with higher acton value.
        * @return None.
        */
	    void ProcessingPushMsgEntryL();

        /**
        * Handle Service invocation: decide what to do next according to 
        * action value and push settings.
        * @return Next state.
        */
	    TInt HandleServiceInvocationL() const;

        /**
        * Download the indicated content.
        * In case of execute-high use the Browser to download the service.
        * In case of cache use the fetch operation to download the service 
        * silently.
        * @return None.
        */
	    void FetchPushMsgEntryL();

        /**
        * Start the Browser and instruct it to download the content 
        * indicated by the Sl's URI.
        * @return None.
        */
	    void StartBrowserL();

        /**
        * Check the result of the SL-cache downloading. If it fails, save the 
        * message. If it succeeds, discard the message.
        * @return None.
        */
	    void FetchCompletedL();

        /**
        * Apply reception rules and save the push message. 
        * If a message already exists with the same URL, 
        * it is saved only if it has a higher action value.
        * @return None.
        */
	    void SavePushMsgEntryL();

        /**
        * Store the new message to Messaging.
        * @param aMatchingEntryId ID of the matching SI entry.
        * @return None.
        */
	    void StoreSLMessageL( TMsvId aMatchingEntryId );

	    // Attribute flags

        /**
        * Flag whether message has an action attribute
        * @param aAction boolean to set flag to - 
        *                ETrue for action attrib present
        *                EFalse for no action attrib.
        */ 
	    inline void SetActionFlag( TBool aAction );

        /**
        * Check status of action flag to see if message has an action 
        * attribute.
        * @return TBool ETrue - action attribute present in message
        *               EFalse - no action attribute.
        */
	    inline TBool ActionFlag() const;

        /**
        * Flag whether message has a Href attribute
        * @param aHref boolean to set flag to - 
        *              ETrue for Href attrib present
        *              EFalse for no href attrib
        */ 
	    inline void SetHrefFlag( TBool aHref );

        /**
        * Check status of href flag to see if message has an href attribute.
        * @return TBool
        *         ETrue - href attribute present in message
        *         EFalse - no href attribute
        */
	    inline TBool HrefFlag() const;


    private: // Methods from CPushHandlerBase
	    
        /**
        * HandleMessage Async. Version. Takes ownership of Push Message and 
        * sets self active to continue processing message.
        * @param aPushMsg CPushMessage to process.
        * @param aStatus Request status variable for use in asynchronous 
        *        operations.
        * @return None.
        */
	    void HandleMessageL(CPushMessage* aPushMsg, TRequestStatus& aStatus);

        /**
        * HandleMessage Sync. Version. Takes ownership of Push Message and 
        * sets self active to continue processing message.
        * Initial State: Set data members then go to the next state 
        * @param aPushMsg CPushMessage to process.
        */
	    void HandleMessageL(CPushMessage* aPushMsg);

        /** 
        * Same functionality as DoCancel()
        */
	    void CancelHandleMessage();

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
        * Terminates any activity.
        * @return None.
        */
	    void DoCancel();

        /**
        * Step through the various representative states for handling a 
        * message.
        * @return None.
        */
	    void RunL();

        /**
        * This is invoked when RunL Leaves with an error so clean up and 
        * return.
        * @return Error code to scheduler.
        */
	    TInt RunError( TInt aError );

    private: // Data

	    enum
            {
            EAction = 0x01, 
            EHref = 0x02
            };

	    enum TState                 ///< States of this state machine.
            {
            EGarbageCollecting, 
            EFilteringAndParsing,
            EProcessing, 
            EFetching, 
            EFetchCompleted, 
            ESavePushMsgEntry, 
            EDone
            };

        TMsvId          iSavedMsgId; ///< ID of the saved message.
        TUint32         iAttributes; ///< Attribute indication (set or not).
	    TInt            iPushMsgAction; ///< SL action.
	    HBufC*          iHrefBuf; ///< Href attribute. Has.

        /// Asynchronous fetch operation that downloads the SL-cache. Owned.
        CPushMtmAutoFetchOperation* iFetchOp;

        NW_Uint32       iCharEncoding; ///< Char encoding of the current msg.
        TBool           iSaveAsRead; ///< Mark the message 'read' after saving.
        TInt            iPushSLEnabled;
    };

#include "CSLContentHandler.inl"

#endif	// __CSLCONTENTHANDLER_H__

