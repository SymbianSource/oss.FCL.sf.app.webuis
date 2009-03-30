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
* Description:  Declaration of CSIContentHandler.
*
*/



#ifndef __CSICONTENTHANDLER_H__
#define __CSICONTENTHANDLER_H__


// INCLUDE FILES

#include "CPushContentHandlerBase.h"
#include "PushContentHandlerDef.hrh"
#include <E32Base.h>
#include <msvstd.h>
#include <nw_dom_attribute.h>
#include <CharConv.h>

// CONSTANTS

const TUid KUidPushSIContentHandler	= { EUidPushSIContentHandler };
_LIT(KSIContentHandlerData,"text/vnd.wap.si||application/vnd.wap.sic");

// FORWARD DECLARATIONS

class CSIPushMsgEntry;

// CLASS DECLARATION

/** 
* CSIContentHandler handles Service Indication content  
*
* This handler takes ownership of the SI message and processes it
* according to the Reception rules 
* [WAP Service Indication version 31-July-2001]
* and either stores the message or deletes it.
* Also S60 specific requirements are applied.
*
* OVERVIEW:
*	The main body of this class and its functionality lies within
*	HandleMessage (asynchonous). This :
*		1. takes ownership of the push message
*		2. validates the message and generates a parsed document tree
*		3. extracts the message attribute values from document tree 
*		4. processes the received message
*		5. finishes
*
* NB: A received message with no creation date will be stored (unless 
* its action is set to delete or the expiry condition is met). This 
* means that the si-id in the message store will not be unique.
*/
class CSIContentHandler : public CPushContentHandlerBase
	{
    public: // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure.
        * @return The constructed object.
        */
	    static CSIContentHandler* NewL();

        /**
        * Destructor.
        */
	    virtual ~CSIContentHandler();

    private: // Constructors

        /**
        * Constructor.
        */
	    CSIContentHandler();

        /**
        * Symbian OS constructor.
        */
	    void  ConstructL();

    private: // New functions

        /**
        * Do Garbage Collection synchronously.
        * @return None.
        */
	    void  CollectGarbageL();

        /**
        * Parse the current message. If the SI is wbxml encoded then it is 
        * decoded first to XML format. Then the message details (attributes) 
        * are earned from the XML tree.
        * @return None.
        */
	    void  ParsePushMsgL();

        /**
        * Parse the indication element.
        * @param aIndication The element to be parsed.
        * @return None.
        */
	    void  ParseIndicationL( NW_DOM_ElementNode_t& aIndication );

        /**
        * Parse an attribute of the indication element.
        * @param aAttrHandle The attribute to be parsed.
        * @return None.
        */
	    void  ParseIndAttributeL( NW_DOM_AttributeHandle_t& aAttrHandle );

        /**
        * Parse the text of the indication element.
        * @param aTextNode The text node to be parsed.
        * @return None.
        */
	    void  ParseTextL( NW_DOM_TextNode_t& aTextNode );


        /**
        * Converts the given buffer to unicode (UCS-2).
        * @param aSrc Source text.
        * @param aCharSetId Character set ID.
        * @return The UCS-2 unicode buffer.
        */
	    HBufC16*  ConvertToUnicodeL( const TDesC8& aSrc, TUint aCharSetId );

        /**
        * Converts the given buffer to unicode (UCS-2).
        * @param aString Source text.
        * @param aCharEncoding Character encoding.
        * @return The UCS-2 unicode buffer.
        */
	    HBufC16*  ConvertToUnicodeL( NW_String_t& aString, NW_Uint32 aCharEncoding );

        /**
        * Initialise the converter. 
        * @return None.
        */
        void InitialiseCharacterSetConverterL();


        /**
        * Convert the action string to a representative numeric value to 
        * facilitate storing the message.
        * @param aActionString The attribute value indicating the action level 
        *        eg 'signal-high'
        * @return TUint: a value representing the action type.
        */
	    TUint ConvertActionString( const TDesC8& aActionString ) const;

        /**
        * The entry fields are set to the correct values.
        * @return None.
        */
	    void  SetSIPushMsgEntryFieldsL( CSIPushMsgEntry& aSIPushMsgEntry );

        /**
        * Do the final processing of the message according to the OMA and S60 
        * requirements.
        * @return None.
        */
	    void  ProcessingPushMsgEntryL();

        /**
        * Store the new message to Messaging.
        * @param aMatchingEntryId ID of the matching SI entry.
        * @return None.
        */
	    void StoreSIMessageL( TMsvId aMatchingEntryId );

        /**
        * Handle message order reception.
        * NB: A received message with no creation date will be stored (unless 
        * its action is set to delete or the expiry condition is met). This 
        * means that the si-id in the message store will not be unique.
        * @param aMatchingEntryId ID of the matching SI entry.
        *        It is changed only if the function returns EFalse (replace)!
        * @return ETrue if the entry has to be discarded.
        */
	    TBool HandleMsgOrderReceptionL( TMsvId& aMatchingEntryId );

	    // conversion utilities

        /** 
        * Convert UTC date time into native TTime format. Method can leave.
        * @param aDateTime The UTC string representing the date and time.
        * @param aConvertedDate Member date variable passed in to accept 
        *        converted UTC date as TTime varaible.
        * @return TBool indicates if conversion successful (ETrue) or 
        *         conversion failed (EFalse)
        */
	    TBool ConvertDateTimeL( const TDesC& aDateTime, 
                                TTime& aConvertedDate ) const;

        /** 
        * Convert OPAQUE data to UTC date time as specified in SI spec.
        * @param aOpaque OPAQUE data.
        * @return The UTC time.
        */
	    HBufC* ConvertOpaqueToUtcL( const TDesC8& aOpaque ) const;

        /**
        * Check given UTC time string conforms to expected format:
        * YYYY-MM-DDTHH:MM:SSZ and strip out formatting characters
        * then validate the remaining characters are all digits.
        * If validated then add TTime formating character so final
        * string returned will be of the format YYYYMMDD:HHMMSS.
        * @param aDateTime UTC string which is validated and returned.
        * @return boolean: indicates if given date is valid (ETrue) 
        *         or not (EFalse)
        */
	    TBool IsValidUTCTime( TDes& aDateTime ) const;

        /** 
        * Convert OPAQUE or STRING attribute to TTime.
        * @param aAttrHandle Handle to the attribute.
        * @param aConvertedDate The resulted TTime.
        * @return ETrue if the conversion succeeded.
        */
	    TBool AttributeToTTimeL( NW_DOM_AttributeHandle_t& aAttrHandle, 
                                 TTime& aConvertedDate ) const;

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
        * Flag whether message has a created date attribute.
        * @param aCreated boolean to set flag to - 
        *                 ETrue for si-created attrib present
        *                 EFalse for no si-created attrib
        */ 
	    inline void SetCreatedFlag( TBool aCreated );

        /**
        * Check status of created flag to see if message has a si-created 
        * attribute.
        * @return TBool ETrue - si-created attribute present in message
        *               EFalse - no si-created attribute
        */
	    inline TBool CreatedFlag() const;

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

        /**
        * Flag whether message has an expiry date attribute (si-expires).
        * @param aExpires boolean to set flag to - 
        *                 ETrue for si-expires present
        *                 EFalse for no si-expires date
        */ 
	    inline void SetExpiresFlag( TBool aExpires );

        /**
        * Check status of flag to see if message has an expiry date attribute.
        * @return TBool
        *         ETrue - si-expires attribute present in message
        *         EFalse - no si-expires attribute
        */
	    inline TBool ExpiresFlag() const;
	    
        /**
        * Flag whether message has a si-id attribute
        * @param aSiId boolean to set flag to - 
        *              ETrue for si-id attrib present
        *              EFalse for no si-id attrib
        */ 
	    inline void SetSiIdFlag( TBool aSiId );

        /**
        * Check status of flag to see if message has an si-id attribute.
        * @return TBool
        *         ETrue - si-id attribute present in message
        *         EFalse - no si-id attribute
        */
	    inline TBool SiIdFlag() const;

        /**
        * Flag whether message has a Data attribute (text)
        * @param aData
        *        boolean to set flag to - ETrue for data attrib present
        *                                 EFalse for no data attrib
        */ 
	    inline void SetDataFlag( TBool aData );

        /**
        * Check status of data flag to see if message has any data.
        * @return TBool
        *         ETrue - data attribute present in message
        *         EFalse - no data attribute
        */
	    inline TBool DataFlag() const;

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
        * Terminates any activity
        * @return None.
        */
	    void DoCancel();

        /**
        * Step through the various representative states for handling a message 
        * States:
        *  EGarbageCollecting - Deleting expired SI msgs from messaging
        *  EFilteringAndParsing - Filter push msg according to Service message 
        *                settings and Parses SI push message (retrieving the 
        *                attributes)
        *  EProcessing - Processing the current entry. OMA and S60 rules are 
        *                applied, then save or update SI msg in the Msg store
        *  EDone - Clean up, complete observer
        * @return None.
        */
	    void RunL();

        /**
        * This is invoked when RunL Leaves with an error so clean up and 
        * return.
        * @return Error code to scheduler.
        */
	    TInt RunError(TInt aError);

    private: // Data

	    enum
            {
            EAction = 0x01, 
            ECreated = 0x02, 
            EHref = 0x04, 
            EExpires = 0x08, 
            ESiId = 0x10, 
            EData = 0x20
            };

	    enum TState
            {
            EGarbageCollecting, 
            EFilteringAndParsing, 
            EProcessing, 
            EDone
            };

        TMsvId          iSavedMsgId; ///< ID of the message saved.
	    TUint32         iAttributes; ///< Attribute indication (set or not).
	    TInt            iPushMsgAction;	///< Action attribute
	    HBufC*          iHrefBuf; ///< Href attribute. Has.
	    HBufC*          iSiIdBuf; ///< Si-id attribute. Has.
	    TTime           iExpiresTime; ///< Expiration attribute.
	    TTime           iCreatedTime; ///< Created attribute.
	    HBufC*          iData; ///< Message text. Has.

        NW_Uint32       iCharEncoding; ///< Char encoding of the current msg.

        //
        CCnvCharacterSetConverter* iCharacterSetConverter; ///< Owned.
        CArrayFix<CCnvCharacterSetConverter::SCharacterSet>* iCharacterSetsAvailable; ///< Owned.
    };

#include "CSIContentHandler.inl"

#endif	// __CSICONTENTHANDLER_H__

