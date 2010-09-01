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
* Description:  Implementation of CSLContentHandler.
*
*/



// INCLUDE FILES

#include "CSLContentHandler.h"
#include "PushMtmUtil.h"
#include "PushMtmSettings.h"
#include "PushAuthenticationUtilities.h"
#include "PushMtmLog.h"
#include "PushContentHandlerPanic.h"
#include "PushMtmAutoFetchOperation.h"
#include "PushMtmUiDef.h"
#include "StringResourceReader.h"
#include "sl_dict.h"
#include "PushContentHandlerUtils.h"
#include <push/cslpushmsgentry.h>
#include <msvids.h>
#include <apgtask.h>
#include <apgcli.h>
#include <w32std.h>
#include <PushMtmUi.rsg>
#include <nw_dom_node.h>
#include <nw_dom_document.h>
#include <nw_dom_element.h>
#include <nw_dom_text.h>
#include <nw_wbxml_dictionary.h>
#include <nw_string_char.h>
#include "PushMtmPrivateCRKeys.h"
#include <centralrepository.h> 

// CONSTANTS

// sl attributes / elements
_LIT8( KSl,      "sl" );
_LIT8( KHref,    "href" );
_LIT8( KAction,  "action" );

// action attribute literals
_LIT8( KExecHigh,"execute-high" );  
_LIT8( KExecLow, "execute-low" );
_LIT8( KCache,   "cache" );

// Text SL MIME type
_LIT( KSlTextContentType, "text/vnd.wap.sl" );

// Browser command to fetch an URL. See Browser API Specification!
_LIT( KBrowserCmdFetchUrl, "4 " );
const TUid KBrowserAppUid = { 0x10008D39 };

const TInt KNoOfDictArrays = 1;

/// Autofetch time delay in seconds.
const TInt KAutofetchDelayInSec = 5;

// file monitored by browser
_LIT( KPushMtmUrl, "c:\\system\\temp\\PushMtmUrl.txt" );

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSLContentHandler::NewL
// ---------------------------------------------------------
//
CSLContentHandler* CSLContentHandler::NewL()
	{
	CSLContentHandler* self = new (ELeave) CSLContentHandler;
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );
	return self;
	}

// ---------------------------------------------------------
// CSLContentHandler::~CSLContentHandler
// ---------------------------------------------------------
//
CSLContentHandler::~CSLContentHandler()
	{
    PUSHLOG_ENTERFN("CSLContentHandler::~CSLContentHandler")

    Cancel();
    delete iFetchOp;
    delete iHrefBuf;

    PUSHLOG_LEAVEFN("CSLContentHandler::~CSLContentHandler")
    }

// ---------------------------------------------------------
// CSLContentHandler::CSLContentHandler
// ---------------------------------------------------------
//
CSLContentHandler::CSLContentHandler()
:   CPushContentHandlerBase(), 
    iSavedMsgId( KMsvNullIndexEntryId ), 
    iPushMsgAction( KErrNotFound ), 
    iSaveAsRead( EFalse )
	{
	}

// ---------------------------------------------------------
// CSLContentHandler::ConstructL
// ---------------------------------------------------------
//
void CSLContentHandler::ConstructL()
	{
    PUSHLOG_ENTERFN("CSLContentHandler::ConstructL")
    
    CRepository* PushSL = CRepository::NewL( KCRUidPushMtm );
    CleanupStack::PushL( PushSL );
    User::LeaveIfError( PushSL->Get( KPushMtmServiceEnabled , iPushSLEnabled ) );
    CleanupStack::PopAndDestroy( PushSL ); 
    
    CPushContentHandlerBase::ConstructL();
    // Added to Active Scheduler.
    PUSHLOG_LEAVEFN("CSLContentHandler::ConstructL")
    }

// ---------------------------------------------------------
// CSLContentHandler::CollectGarbageL
// ---------------------------------------------------------
//
void CSLContentHandler::CollectGarbageL()
	{
    PUSHLOG_ENTERFN("CSLContentHandler::CollectGarbageL")

    DoCollectGarbageL();

	if(iPushSLEnabled)
	    iState = EFilteringAndParsing;
	else
	    iState = EDone;
	
	IdleComplete();

    PUSHLOG_LEAVEFN("CSLContentHandler::CollectGarbageL")
    }


// ---------------------------------------------------------
// CSLContentHandler::ParsePushMsgL
// Note that cXML parser dosn't do any validation!
// ---------------------------------------------------------
//
void CSLContentHandler::ParsePushMsgL()
	{
    PUSHLOG_ENTERFN("CSLContentHandler::ParsePushMsgL")

    TPtrC8 bodyPtr;
    iMessage->GetMessageBody( bodyPtr );
    // If there is no body in the message leave with an error
    if ( bodyPtr.Size() == 0 )
        {
        PUSHLOG_WRITE("CSLContentHandler::ParsePushMsgL: Empty body")
        User::Leave( KErrCorrupt );
        }

    TPtrC contentType;
    iMessage->GetContentType( contentType );
    PUSHLOG_WRITE_FORMAT(" Content type <%S>",&contentType)

    // Add SL dictionary.
    NW_WBXML_Dictionary_t* dictArray[ KNoOfDictArrays ] = 
        { (NW_WBXML_Dictionary_t*)&NW_SL_WBXMLDictionary };

    NW_Status_t stat = NW_STAT_SUCCESS;

    RWbxmlDictionary wbxmlDict;
    wbxmlDict.InitializeL( KNoOfDictArrays, dictArray );
    CleanupClosePushL<RWbxmlDictionary>( wbxmlDict );

    NW_TinyDom_Handle_t domHandle;
    NW_Byte* buffer = (NW_Byte*)bodyPtr.Ptr();
    NW_Int32 length = (NW_Int32)bodyPtr.Size();
    NW_Bool encoded = ( contentType.CompareF( KSlTextContentType ) == 0 ) ? 
                                                         NW_FALSE : NW_TRUE;
    NW_Uint32 publicID = NW_SL_PublicId;
    NW_Bool extTNotStringTable = NW_FALSE; // What is this?
    NW_DOM_NodeType_t type = 0;
    /**********************************
    *   Root of DOM
    ***********************************/
    CDocumentTreeOwner* docTreeOwner = new (ELeave) CDocumentTreeOwner;
    CleanupStack::PushL( docTreeOwner );
    NW_DOM_DocumentNode_t* domNode = NW_DOM_DocumentNode_BuildTree
        ( 
                            &domHandle, 
                            buffer, 
                            length, 
                            encoded, 
                            publicID, 
                            extTNotStringTable 
        );
	if (!domNode)
		{
		PUSHLOG_WRITE("CSLContentHandler::ParsePushMsgL: domNode is Null")
		}
    User::LeaveIfNull( domNode );
    docTreeOwner->SetDocTree( domNode );

    type = NW_DOM_Node_getNodeType( domNode );
    if ( type != NW_DOM_DOCUMENT_NODE )
        {
        PUSHLOG_WRITE_FORMAT("CSLContentHandler::ParsePushMsgL: Not Document node <%d>",type)
        User::Leave( KErrArgument );
        }

    iCharEncoding = NW_DOM_DocumentNode_getCharacterEncoding( domNode );
    PUSHLOG_WRITE_FORMAT("CSLContentHandler::ParsePushMsgL: Doc char encoding <%x>",iCharEncoding)

    /**********************************
    *   ELEMENT sl
    ***********************************/
	// first make sure if there any children in the dom tree, otherwise we will PANIC(in NW_DOM_DocumentNode_getDocumentElement) and crash WatcherMainThread.
	TBool domNodeHasChildNodes = EFalse;
	domNodeHasChildNodes = NW_DOM_Node_hasChildNodes( domNode );
	PUSHLOG_WRITE_FORMAT("CSLContentHandler::ParsePushMsgL: check if Dom tree has <SI> node <%d>", domNodeHasChildNodes)
	if (!domNodeHasChildNodes)
        {
        PUSHLOG_WRITE("CSLContentHandler::ParsePushMsgL: No SL element present in the dom tree. Message corrupted.")
        User::Leave( KErrCorrupt );
        }

	PUSHLOG_WRITE("CSLContentHandler::ParsePushMsgL: before calling getDocumentElement")
    NW_DOM_ElementNode_t* slElement = 
        NW_DOM_DocumentNode_getDocumentElement( domNode );
	if (!slElement)
		{
		PUSHLOG_WRITE("CSLContentHandler::ParsePushMsgL: slElement is Null")
		}
    User::LeaveIfNull( slElement );

    type = NW_DOM_Node_getNodeType( slElement );

    CStringOwner* stringOwner = new (ELeave) CStringOwner;
    CleanupStack::PushL( stringOwner );

    NW_String_t* name = NW_String_new();
    User::LeaveIfNull( name );
    stringOwner->SetString( name );
    stat = NW_DOM_Node_getNodeName( slElement, name );
	PUSHLOG_WRITE_FORMAT("CSLContentHandler::ParsePushMsgL: getNodeName ErrCode: %d", NwxStatusToErrCode( stat ))
    User::LeaveIfError( NwxStatusToErrCode( stat ) );
    NW_Byte*  nameBuf = NW_String_getStorage( name );
    NW_Uint16 nameLen = NW_String_getCharCount( name, iCharEncoding );
    TPtrC8 namePtr( nameBuf, nameLen );

    if ( type != NW_DOM_ELEMENT_NODE || namePtr.CompareF( KSl ) != 0 )
        {
        PUSHLOG_WRITE_FORMAT("CSLContentHandler::ParsePushMsgL: Not sl element node <%d>",type)
        User::Leave( KErrArgument );
        }

    CleanupStack::PopAndDestroy( stringOwner ); // stringOwner

    /**********************************
    *   Attributes of ELEMENT sl
    ***********************************/
    if ( NW_DOM_ElementNode_hasAttributes( slElement ) )
        {
        NW_DOM_AttributeListIterator_t attrIter;
        stat = NW_DOM_ElementNode_getAttributeListIterator
                             ( slElement, &attrIter );
		PUSHLOG_WRITE_FORMAT("CSLContentHandler::ParsePushMsgL: getAttribListIter ErrCode: %d", NwxStatusToErrCode( stat ))
        User::LeaveIfError( NwxStatusToErrCode( stat ) );

        NW_DOM_AttributeHandle_t attrHandle;
        while( NW_DOM_AttributeListIterator_getNextAttribute
               ( &attrIter, &attrHandle ) == NW_STAT_WBXML_ITERATE_MORE )
            {
            ParseSlAttributeL( attrHandle );
            }
        }

    // Cleanup.
    CleanupStack::PopAndDestroy( 2, &wbxmlDict ); // docTreeOwner, wbxmlDict

    // if 'action' attribute not specified, the value 'execute-low' is used.
	if ( !ActionFlag() )
        {
		iPushMsgAction = CSLPushMsgEntry::ESLPushMsgExecuteLow;
        SetActionFlag( ETrue );
        PUSHLOG_WRITE_FORMAT(" Defaulting to ActionFlag: %d",iPushMsgAction)
        }

    iState = EProcessing;
    IdleComplete();

    PUSHLOG_LEAVEFN("CSLContentHandler::ParsePushMsgL")
	}

// ---------------------------------------------------------
// CSLContentHandler::ParseSlAttributeL
// ---------------------------------------------------------
//
void CSLContentHandler::ParseSlAttributeL
                        ( NW_DOM_AttributeHandle_t& aAttrHandle )
	{
    PUSHLOG_ENTERFN("CSLContentHandler::ParseSlAttributeL")

    NW_Status_t stat = NW_STAT_SUCCESS;

    CStringOwner* attrNameOwner = new (ELeave) CStringOwner;
    CleanupStack::PushL( attrNameOwner );

    NW_String_t* attrName = NW_String_new();
    User::LeaveIfNull( attrName );
    attrNameOwner->SetString( attrName );

    // Get the name of the attribute.
    stat = NW_DOM_AttributeHandle_getName( &aAttrHandle, attrName );
    User::LeaveIfError( NwxStatusToErrCode( stat ) );
    NW_Byte*  attrNameBuf = NW_String_getStorage( attrName );
    NW_Uint16 attrNameLen = NW_String_getCharCount( attrName, iCharEncoding );
    TPtrC8 attrNamePtr( attrNameBuf, attrNameLen );

    if ( attrNamePtr.CompareF( KHref ) == 0 )
        {
        if ( !HrefFlag() )
            {
            HBufC* tempHrefBuf = NULL;

            CStringOwner* valOwner = new (ELeave) CStringOwner;
            CleanupStack::PushL( valOwner );

            NW_String_t* val = NW_String_new();
            User::LeaveIfNull( val );
            valOwner->SetString( val );
            stat = NW_DOM_AttributeHandle_getValue( &aAttrHandle, val );
            if ( stat != NW_STAT_DOM_NO_STRING_RETURNED )
                {
                User::LeaveIfError( NwxStatusToErrCode( stat ) );
                NW_Byte* storage = NW_String_getStorage( val );
                NW_Uint16 length = NW_String_getCharCount( val, iCharEncoding );
                TPtrC8 prefixPtr( storage, length );
                tempHrefBuf = HBufC::NewMaxL( length );
                // No leavable after it!!! until...
                tempHrefBuf->Des().Copy( prefixPtr );
                }

            CleanupStack::PopAndDestroy( valOwner ); // valOwner

            if ( tempHrefBuf )
                {
                if ( tempHrefBuf->Length() == 0 )
                    {
                    // Zero length Href is considered as nothing.
                    PUSHLOG_WRITE(" Zero length HrefFlag");
                    }
                else
                    {
                    iHrefBuf = tempHrefBuf; // ...until here.
                    SetHrefFlag( ETrue );
                    PUSHLOG_WRITE_FORMAT(" HrefFlag set <%S>",iHrefBuf);
                    }
                }
            }
        }
    else if ( attrNamePtr.CompareF( KAction ) == 0 )
        {
        if ( !ActionFlag() )
            {
            CStringOwner* stringOwner = new (ELeave) CStringOwner;
            CleanupStack::PushL( stringOwner );

            NW_String_t* val = NW_String_new();
            User::LeaveIfNull( val );
            stringOwner->SetString( val );
            stat = NW_DOM_AttributeHandle_getValue( &aAttrHandle, val ); 
            User::LeaveIfError( NwxStatusToErrCode( stat ) );
            NW_Byte* storage = NW_String_getStorage( val );
            NW_Uint16 length = NW_String_getCharCount( val, iCharEncoding );
            TPtrC8 actionPtr( storage, length );

            iPushMsgAction = ConvertActionString( actionPtr );
            SetActionFlag( ETrue );
            PUSHLOG_WRITE_FORMAT(" ActionFlag: %d",iPushMsgAction)

            CleanupStack::PopAndDestroy( stringOwner );
            }
        }
    else
        {
        __ASSERT_DEBUG( EFalse, 
            ContHandPanic( EPushContHandPanUnexpSlToken ) );
        }

    CleanupStack::PopAndDestroy( attrNameOwner ); // attrNameOwner

    PUSHLOG_LEAVEFN("CSLContentHandler::ParseSlAttributeL")
    }

// ---------------------------------------------------------
// CSLContentHandler::ConvertActionString
// ---------------------------------------------------------
//
TUint CSLContentHandler::ConvertActionString
                         ( const TDesC8& aActionString ) const
	{
	const TInt KMatchFound = 0;

	// if 'action' attribute not specified, the value 'execute-low' is used.
	TUint actionValue = CSLPushMsgEntry::ESLPushMsgExecuteLow;

	if ( aActionString.Compare( KExecHigh ) == KMatchFound )
        {
		actionValue = CSLPushMsgEntry::ESLPushMsgExecuteHigh;
        }
	else if ( aActionString.Compare( KExecLow ) == KMatchFound )
        {
		actionValue = CSLPushMsgEntry::ESLPushMsgExecuteLow;
        }
	else if ( aActionString.Compare( KCache ) == KMatchFound )
        {
		actionValue = CSLPushMsgEntry::ESLPushMsgExecuteCache;
        }

	return actionValue;
	}

// ---------------------------------------------------------
// CSLContentHandler::SetSlPushMsgEntryFieldsL
// ---------------------------------------------------------
//
void CSLContentHandler::SetSlPushMsgEntryFieldsL( CSLPushMsgEntry& 
                                                  aSlPushMsgEntry ) const
	{
    PUSHLOG_ENTERFN("CSLContentHandler::SetSlPushMsgEntryFieldsL")

	// Set URL and Action fields.
    if ( HrefFlag() )
        {
	    aSlPushMsgEntry.SetUrlL( *iHrefBuf );
        }

    __ASSERT_DEBUG( ActionFlag(), 
                    ContHandPanic( EPushContHandPanUnspecSlAction ) );
    if ( ActionFlag() )
        {
	    aSlPushMsgEntry.SetAction( iPushMsgAction );
        }
    else // if not specified, the value 'execute-low' is used.
        {
        aSlPushMsgEntry.SetAction( CSLPushMsgEntry::ESLPushMsgExecuteLow );
        }

	// Set all the relevant header fields.
	TPtrC8 msgHeaderPtr;
	iMessage->GetHeader( msgHeaderPtr );
	aSlPushMsgEntry.SetHeaderL( msgHeaderPtr );

    // Get server address.
    TPtrC8 srvAddress;
    if ( iMessage->GetServerAddress( srvAddress ) )
        {
	    aSlPushMsgEntry.SetFromL( srvAddress );
        }

    // First line in Inbox: TMsvEntry::iDetails.
    if ( srvAddress.Length() == 0 )
        {
        // Read from resource.
        HBufC* details = 
            iStrRscReader->AllocReadResourceLC( R_PUSHMISC_UNK_SENDER );
        aSlPushMsgEntry.SetMsgDetailsL( *details );
        CleanupStack::PopAndDestroy( details );
        }
    else
        {
        // Convert the "From" information to the format required by the UI 
        // spec and then decode it.
        HBufC* details = iWapPushUtils->ConvertDetailsL( srvAddress );
        CleanupStack::PushL( details );
        HBufC* convertedFrom = 
            CPushMtmUtil::ConvertUriToDisplayFormL( *details );
        CleanupStack::PushL( convertedFrom );
        //
        aSlPushMsgEntry.SetMsgDetailsL( *convertedFrom );
        //
        CleanupStack::PopAndDestroy( 2, details ); // convertedFrom, details
        }

    // Second line in Inbox: TMsvEntry::iDescription.
    // Read from resource.
    HBufC* description = 
        iStrRscReader->AllocReadResourceLC( R_PUSHMISC_INBOX_SERV_MSG );
    aSlPushMsgEntry.SetMsgDescriptionL( *description );
    CleanupStack::PopAndDestroy( description );

    // ******** Push MTM specific processing *********

    /*
    * Unfortunately in CPushMsgEntryBase there is no such functionality 
    * with which we can reach TMsvEntry as non-const, but we have to 
    * modify the entry's iMtmData2 member somehow. We can do it 
    * with either casting or with modifying and saving the entry 
    * manually after it has been saved by CSLPushMsgEntry. The latter 
    * solution is more expensive so we choose the first.
    */
    TMsvEntry& tEntry = CONST_CAST( TMsvEntry&, aSlPushMsgEntry.Entry() );
    if ( HrefFlag() )
        {
        CPushMtmUtil::SetAttrs( tEntry, EPushMtmAttrHasHref );
        }
    else
        {
        CPushMtmUtil::ResetAttrs( tEntry, EPushMtmAttrHasHref );
        }

    // Indication is required if the entry is saved as 'read' (delete & 
    // replacement notification). It can happen only in case of SL message.
    // Otherwise the flag has to be cleared!
    if ( !iSaveAsRead )
        {
        // Saving as unread & new.
        tEntry.SetNew( ETrue );
        tEntry.SetUnread( ETrue );
        CPushMtmUtil::ResetAttrs( tEntry, EPushMtmReadButContentChanged );
        }
    else
        {
        // Saving as read.
        tEntry.SetNew( EFalse );
        tEntry.SetUnread( EFalse );
        CPushMtmUtil::SetAttrs( tEntry, EPushMtmReadButContentChanged );
        }

    PUSHLOG_LEAVEFN("CSLContentHandler::SetSlPushMsgEntryFieldsL")
    }

// ---------------------------------------------------------
// CSLContentHandler::ProcessingPushMsgEntryL
// ---------------------------------------------------------
//
void CSLContentHandler::ProcessingPushMsgEntryL()
	{
    PUSHLOG_ENTERFN("CSLContentHandler::ProcessingPushMsgEntryL")

    TBool discardPushMsg( EFalse );

    __ASSERT_DEBUG( ActionFlag(), 
                    ContHandPanic( EPushContHandPanUnspecSlAction ) );

    // S60 requirement: if the href is empty then delete (discard) the msg.
    if ( HrefFlag() == EFalse )
        {
        PUSHLOG_WRITE(" No SL Href.")
        discardPushMsg = ETrue;
        }
    else
        {
        __ASSERT_DEBUG( HrefFlag() && iHrefBuf, 
                        ContHandPanic( EPushContHandPanUnspecSlHref ) );

        // The message will not be discarded
        discardPushMsg = EFalse;

        CMsvEntrySelection* matchingUrlList = iWapPushUtils->FindUrlLC
                                              ( *iHrefBuf, KUidWapPushMsgSL );
        TInt matchingListCount = matchingUrlList->Count();
        PUSHLOG_WRITE_FORMAT(" matchingListCount: %d",matchingListCount)

        // Only one SL is allowed with the same Url, so leave the first and 
        // delete the others.
        if ( 1 < matchingListCount )
            {
            for ( TInt count = 1; count < matchingListCount; ++count )
                {
                iWapPushUtils->DeleteEntryL( matchingUrlList->At(count) );
                }
            matchingListCount = 1; // Only one remains.
            }

	    if ( 0 < matchingListCount )
		    {
		    // Find msg of the same href and discard it if it has a lower or 
            // the same action value.
            CSLPushMsgEntry* matchingSl = CSLPushMsgEntry::NewL();
	        CleanupStack::PushL( matchingSl );

            const TMsvId matchingId = matchingUrlList->At(0);
            matchingSl->RetrieveL( *iMsvSession, matchingId );

            if ( iPushMsgAction <= matchingSl->Action() ) 
			    {
                PUSHLOG_WRITE(" SL: not higher action")
                discardPushMsg = ETrue;
                }

            CleanupStack::PopAndDestroy( matchingSl ); // matchingSl, 
            }

	    CleanupStack::PopAndDestroy( matchingUrlList ); // matchingUrlList
        }

    if ( discardPushMsg )
        {
        // Nothing to do.
        PUSHLOG_WRITE(" SL discarded.")
        iState = EDone;
        IdleComplete();
        }
    else
        {
        iState = HandleServiceInvocationL();
        IdleComplete();
        }

    __ASSERT_DEBUG( iSavedMsgId == KMsvNullIndexEntryId, 
                    ContHandPanic( EPushContHandPanSlMsgIdSet ) );

    PUSHLOG_LEAVEFN("CSLContentHandler::ProcessingPushMsgEntryL")
	}

// ---------------------------------------------------------
// CSLContentHandler::HandleServiceInvocationL
// ---------------------------------------------------------
//
TInt CSLContentHandler::HandleServiceInvocationL() const
    {
    PUSHLOG_ENTERFN("CSLContentHandler::HandleServiceInvocationL")

    TInt nextState = ESavePushMsgEntry;

    if ( iPushMsgAction == CSLPushMsgEntry::ESLPushMsgExecuteCache )
        {
        PUSHLOG_WRITE(" SL cache");
        TBool isAuthenticated = TPushAuthenticationUtil::
            AuthenticateMsgL( *iMtmSettings, *iMessage );
		if ( !isAuthenticated )
            {
            PUSHLOG_WRITE(" Not authenticated");
            // The message is placed to Inbox.
            nextState = ESavePushMsgEntry;
            }
        else
            {
            // Authenticated. Fetch SL-cache.
            nextState = EFetching;
            }
        }

    else if ( iPushMsgAction == CSLPushMsgEntry::ESLPushMsgExecuteLow )
        {
        PUSHLOG_WRITE(" SL execute-low")
        // It is independent from Automatic/Manual setting and WL 
        // authentication is not applied. The message is placed to Inbox 
        // for manual downloading.
        nextState = ESavePushMsgEntry;
        }

    else // ESLPushMsgExecuteHigh
        {
        PUSHLOG_WRITE(" SL execute-high");
        // If the settings is Manual or it does not pass the WL authentication 
		// then it is placed to Inbox for manual downloading.
        // If the setting is Automatic and it passes the WL authentication, 
        // the Browser is started standalone to download the URL without any 
		// user interaction.
        if ( iMtmSettings->ServiceLoadingType() == 
			               CPushMtmSettings::EManual )
            {
            PUSHLOG_WRITE(" Manual setting")
            // The message is placed to Inbox.
            nextState = ESavePushMsgEntry;
            }
        else // Automatic loading
            {
            PUSHLOG_WRITE(" Automatic setting");
            // Authenticate the message.
            TBool isAuthenticated = TPushAuthenticationUtil::
                AuthenticateMsgL( *iMtmSettings, *iMessage );
            if ( !isAuthenticated )
	            {
                PUSHLOG_WRITE(" Not authenticated");
	            // The message is placed to Inbox.
	            nextState = ESavePushMsgEntry;
	            }
            else
	            {
	            // Authenticated - start downloading.
	            nextState = EFetching;
	            }
            }
        }

    PUSHLOG_LEAVEFN("CSLContentHandler::HandleServiceInvocationL")
    return nextState;
    }

// ---------------------------------------------------------
// CSLContentHandler::FetchPushMsgEntryL
// ---------------------------------------------------------
//
void CSLContentHandler::FetchPushMsgEntryL()
	{
    PUSHLOG_ENTERFN("CSLContentHandler::FetchPushMsgEntryL")

    __ASSERT_DEBUG( iSavedMsgId == KMsvNullIndexEntryId, 
                    ContHandPanic( EPushContHandPanAlreadyInitialized ) );
    __ASSERT_DEBUG( HrefFlag() && iHrefBuf, 
                    ContHandPanic( EPushContHandPanUnspecSlHref ) );

    /* 
    * In case of execute-high use the Browser to download the service.
    * In case of cache use the fetch operation to download the service 
    * silently. 
    */

    if ( iPushMsgAction == CSLPushMsgEntry::ESLPushMsgExecuteHigh )
        {
        PUSHLOG_WRITE(" Start Browser")
        // Launch the Browser with the URI, then save the message.
        // Trap errors. If Browser's launching fails, then save the 
        // message as 'unread'. In case of an error, it is not forwarded.
        TRAPD( err, StartBrowserL() );
        iState = ESavePushMsgEntry;
        // Mark it 'read' after succesfull Browser startup.
        iSaveAsRead = err ? EFalse : ETrue;
        IdleComplete();
        }
    else if ( iPushMsgAction == CSLPushMsgEntry::ESLPushMsgExecuteCache )
        {
        // Fetch the service inside the content handler.
        iStatus = KRequestPending;
        SetActive();
        __ASSERT_DEBUG( !iFetchOp, 
                        ContHandPanic( EPushContHandPanFetchAlreadyInit ) );

        iFetchOp = CPushMtmAutoFetchOperation::NewL( *iHrefBuf, 
                                                     KAutofetchDelayInSec, 
                                                     iStatus );
        iFetchOp->StartL();
        PUSHLOG_WRITE(" Fetch op started")
        iState = EFetchCompleted; // Next state.
        // Fetch completes it.
        }
    else
        {
        __ASSERT_DEBUG( EFalse, 
                        ContHandPanic( EPushContHandPanBadActionValue ) );
        User::Leave( KErrNotSupported );
        }

    PUSHLOG_LEAVEFN("CSLContentHandler::FetchPushMsgEntryL")
    }

// ---------------------------------------------------------
// CSLContentHandler::StartBrowserL
// ---------------------------------------------------------
//
void CSLContentHandler::StartBrowserL()
    {
    PUSHLOG_ENTERFN("CSLContentHandler::StartBrowserL")

    // Parameters are separated by space
    // 1st parameter: type of the further parameters
    // 2nd parameter: URL
    //
    HBufC* param = HBufC::NewLC( KBrowserCmdFetchUrl().Length() + 
                                 iHrefBuf->Length() );
    TPtr paramPtr = param->Des();
    paramPtr.Copy( KBrowserCmdFetchUrl );
    paramPtr.Append( *iHrefBuf );

    RWsSession wsSession;
    User::LeaveIfError( wsSession.Connect() );
    CleanupClosePushL<RWsSession>( wsSession );
    TApaTaskList taskList( wsSession );
    TApaTask task = taskList.FindApp( KBrowserAppUid );

    if ( task.Exists() )
        {
        PUSHLOG_WRITE("CSLContentHandler Browser::SendMessage")

        RFs             rfs;
        RFile           file;
        TPtrC8          param8Ptr;
        // 8-bit buffer is required.
        HBufC8* param8 = HBufC8::NewLC( param->Length() );
        param8->Des().Copy( *param );
        param8Ptr.Set(param8->Des());

        // Open the file.
        User::LeaveIfError(rfs.Connect());
        CleanupClosePushL(rfs);

        // Replace file if exists or Create file if not exist yet
        User::LeaveIfError( file.Replace( rfs, KPushMtmUrl, EFileWrite | EFileShareExclusive ) );
        CleanupClosePushL(file);

        // Write to file      
        User::LeaveIfError( file.Write( param8Ptr ) );
        
        // Clean up.
        CleanupStack::PopAndDestroy(/*file*/);
        CleanupStack::PopAndDestroy(/*rfs*/);
        CleanupStack::PopAndDestroy( /*param8*/ );
        }
    else 
        {
        PUSHLOG_WRITE("CSLContentHandler Browser::StartDocument")
        RApaLsSession appArcSession;
        User::LeaveIfError( appArcSession.Connect() );
        CleanupClosePushL<RApaLsSession>( appArcSession );
        TThreadId id;
        User::LeaveIfError
            (
                appArcSession.StartDocument( *param, KBrowserAppUid, id )
            );
        CleanupStack::PopAndDestroy( &appArcSession );
        }

    CleanupStack::PopAndDestroy( &wsSession );
    CleanupStack::PopAndDestroy( param );

    PUSHLOG_LEAVEFN("CSLContentHandler::StartBrowserL")
    }

// ---------------------------------------------------------
// CSLContentHandler::FetchCompletedL
// ---------------------------------------------------------
//
void CSLContentHandler::FetchCompletedL()
	{
    PUSHLOG_ENTERFN("CSLContentHandler::FetchCompletedL")

    __ASSERT_DEBUG( iPushMsgAction == CSLPushMsgEntry::ESLPushMsgExecuteCache, 
                    ContHandPanic( EPushContHandPanBadActionValue ) );
    __ASSERT_DEBUG( iSavedMsgId == KMsvNullIndexEntryId, 
                    ContHandPanic( EPushContHandPanAlreadyInitialized ) );
    __ASSERT_DEBUG( iFetchOp, ContHandPanic( EPushContHandPanNoFetchOp ) );

    const TInt fetchRes = iStatus.Int();
    PUSHLOG_WRITE_FORMAT(" fetchRes <%d>",fetchRes)

    if ( fetchRes != KErrNone )
        {
        // Downloading failed. Save the message.
        iState = ESavePushMsgEntry;
        }
    else
        {
        // Silent fetching has completed successfully.
        // The message should not be saved.
        iState = EDone;
        }

    // Next state set. Complete.
    IdleComplete();

    PUSHLOG_LEAVEFN("CSLContentHandler::FetchCompletedL")
    }

// ---------------------------------------------------------
// CSLContentHandler::SavePushMsgEntryL
// ---------------------------------------------------------
//
void CSLContentHandler::SavePushMsgEntryL()
    {
    PUSHLOG_ENTERFN("CSLContentHandler::SavePushMsgEntryL")

    __ASSERT_DEBUG( ActionFlag(), 
                    ContHandPanic( EPushContHandPanUnspecSlAction ) );
    __ASSERT_DEBUG( HrefFlag() && iHrefBuf, 
                    ContHandPanic( EPushContHandPanUnspecSlHref ) );
    __ASSERT_DEBUG( iSavedMsgId == KMsvNullIndexEntryId, 
                    ContHandPanic( EPushContHandPanAlreadyInitialized ) );

    CMsvEntrySelection* matchingUrlList = iWapPushUtils->FindUrlLC
                                          ( *iHrefBuf, KUidWapPushMsgSL );
    TInt matchingListCount = matchingUrlList->Count();
    PUSHLOG_WRITE_FORMAT(" matchingListCount: %d",matchingListCount)

    // Only one SL is allowed with the same Url, so leave the first and 
    // delete the others.
    __ASSERT_DEBUG( matchingListCount <= 1, 
                    ContHandPanic( EPushContHandPanTooManySl ) );
    if ( 1 < matchingListCount )
        {
        for ( TInt count = 1; count < matchingListCount; ++count )
            {
            iWapPushUtils->DeleteEntryL( matchingUrlList->At(count) );
            }
        matchingListCount = 1; // Only one remains.
        }

    TBool saveNewMsg = ETrue; // Save by default.
    TMsvId matchingEntryId = KMsvNullIndexEntryId;

    // Apply reception rules.
    if ( matchingListCount == 0 )
        {
        // Nothing to do.
        saveNewMsg = ETrue;
        }
    else
        {
        CSLPushMsgEntry* matchingSl = CSLPushMsgEntry::NewL();
        CleanupStack::PushL( matchingSl );

        matchingEntryId = matchingUrlList->At(0);
        matchingSl->RetrieveL( *iMsvSession, matchingEntryId );

        if ( iPushMsgAction <= matchingSl->Action() ) 
            {
            // Discard the new SL: it does not have higher 
            // action value as the existing.
            PUSHLOG_WRITE(" SL not higher action - discarded")
            saveNewMsg = EFalse;
            }
        else
            {
            // The new has greater action attribute. 
            // Update the old SL with the new data.
            saveNewMsg = ETrue;
            }

        CleanupStack::PopAndDestroy( matchingSl ); // matchingSl
        }

    CleanupStack::PopAndDestroy( matchingUrlList ); // matchingUrlList

    // Store message if not marked for deletion.
    if ( saveNewMsg )
        {
		StoreSLMessageL( matchingEntryId );
		}

    iState = EDone;
    IdleComplete();

    PUSHLOG_LEAVEFN("CSLContentHandler::SavePushMsgEntryL")
    }

// ---------------------------------------------------------
// CSLContentHandler::StoreSLMessageL
// ---------------------------------------------------------
//
void CSLContentHandler::StoreSLMessageL( TMsvId aMatchingEntryId )
	{
    PUSHLOG_ENTERFN("CSLContentHandler::StoreSLMessageL")

	CSLPushMsgEntry* slEntry = CSLPushMsgEntry::NewL();
	CleanupStack::PushL( slEntry );

    if ( aMatchingEntryId == KMsvNullIndexEntryId )
        {
        PUSHLOG_WRITE(" No matching SL")
        // Save new to Inbox.
        SetSlPushMsgEntryFieldsL( *slEntry );
	    iSavedMsgId = 
            slEntry->SaveL( *iMsvSession, KMsvGlobalInBoxIndexEntryId );
        // Set the entry to read and *not* new state depending on iSaveAsRead.
        if ( !iSaveAsRead )
            {
            // Do nothing SaveL saves it as unread.
            }
        else
            {
            // SaveL owerrides the read settings (iEntry.SetUnread(ETrue);) 
            // that we set in SetSlPushMsgEntryFieldsL, thus the read status 
            // has to be reset manually here:
            iWapPushUtils->MarkServiceUnreadL( iSavedMsgId, EFalse );
            }
        }
    else
        {
        PUSHLOG_WRITE(" Matching SL")
        slEntry->RetrieveL( *iMsvSession, aMatchingEntryId );
        SetSlPushMsgEntryFieldsL( *slEntry );

        slEntry->UpdateL( *iMsvSession );
        iSavedMsgId = aMatchingEntryId;
        // Note that UpdateL does not change the read/unread status.

        // Move the updated msg back to Inbox.
        TMsvId parentId = slEntry->Entry().Parent();
        if ( parentId != KMsvGlobalInBoxIndexEntryId )
	        {
            PUSHLOG_WRITE(" Moving back to Inbox")
            CMsvEntry* cParent = iMsvSession->GetEntryL( parentId );
            CleanupStack::PushL( cParent );
	        cParent->MoveL( iSavedMsgId, KMsvGlobalInBoxIndexEntryId );
            CleanupStack::PopAndDestroy( cParent ); // cParent
	        }
        }

#ifdef __TEST_LOG__
        _LIT( KDateFormat, "%E%D%X%N%Y %1 %2 %3" );
        _LIT( KTimeFormat, "%-B%:0%J%:1%T%:2%S%:3%+B" );
        TBuf<32> dateHolder;
        TBuf<32> timeHolder;
        TTime recDateTime = slEntry->ReceivedDate();
        recDateTime.FormatL( dateHolder, KDateFormat );
        recDateTime.FormatL( timeHolder, KTimeFormat );
        PUSHLOG_WRITE_FORMAT(" rec date: <%S>",&dateHolder)
        PUSHLOG_WRITE_FORMAT(" rec time: <%S>",&timeHolder)
#endif // __TEST_LOG__

	CleanupStack::PopAndDestroy( slEntry ); // slEntry

    PUSHLOG_LEAVEFN("CSLContentHandler::StoreSLMessageL")
	}


// ---------------------------------------------------------
// CSLContentHandler::HandleMessageL
// ---------------------------------------------------------
//
void CSLContentHandler::HandleMessageL( CPushMessage* aPushMsg, 
                                        TRequestStatus& aStatus )
	{
    PUSHLOG_ENTERFN("CSLContentHandler::HandleMessageL 2")

    __ASSERT_DEBUG( aPushMsg != NULL, 
                    ContHandPanic( EPushContHandPanMsgNull ) );

#ifdef __TEST_LOG__
    TPtrC8 bodyPtr;
    aPushMsg->GetMessageBody( bodyPtr );
    PUSHLOG_HEXDUMP( bodyPtr )
#endif // __TEST_LOG__

	iMessage = aPushMsg;
	iAcknowledge = ETrue;
	SetConfirmationStatus( aStatus );

	iState = EGarbageCollecting;
	IdleComplete();

    PUSHLOG_LEAVEFN("CSLContentHandler::HandleMessageL 2")
    }

// ---------------------------------------------------------
// CSLContentHandler::HandleMessageL
// ---------------------------------------------------------
//
void CSLContentHandler::HandleMessageL( CPushMessage* aPushMsg )
	{
    PUSHLOG_ENTERFN("CSLContentHandler::HandleMessageL 1")

    __ASSERT_DEBUG( aPushMsg != NULL, 
                    ContHandPanic( EPushContHandPanMsgNull ) );
	
#ifdef __TEST_LOG__
    TPtrC8 bodyPtr;
    aPushMsg->GetMessageBody( bodyPtr );
    PUSHLOG_HEXDUMP( bodyPtr )
#endif // __TEST_LOG__

	iAcknowledge = EFalse;
	iMessage = aPushMsg;

	iState = EGarbageCollecting;
	IdleComplete();

    PUSHLOG_LEAVEFN("CSLContentHandler::HandleMessageL 1")
    }

// ---------------------------------------------------------
// CSLContentHandler::CancelHandleMessage
// ---------------------------------------------------------
//
void CSLContentHandler::CancelHandleMessage()
	{
    Cancel();
	}

// ---------------------------------------------------------
// CSLContentHandler::CPushHandlerBase_Reserved1
// ---------------------------------------------------------
//
void CSLContentHandler::CPushHandlerBase_Reserved1()
	{
	}

// ---------------------------------------------------------
// CSLContentHandler::CPushHandlerBase_Reserved1
// ---------------------------------------------------------
//
void CSLContentHandler::CPushHandlerBase_Reserved2()
	{
	}

// ---------------------------------------------------------
// CSLContentHandler::DoCancel
// ---------------------------------------------------------
//
void CSLContentHandler::DoCancel()
	{
    PUSHLOG_ENTERFN("CSLContentHandler::DoCancel")
    // TODO Cancel outstanding requests!
	Complete( KErrCancel );
    PUSHLOG_LEAVEFN("CSLContentHandler::DoCancel")
	}

// ---------------------------------------------------------
// CSLContentHandler::RunL
// ---------------------------------------------------------
//
void CSLContentHandler::RunL()
	{
    PUSHLOG_ENTERFN("CSLContentHandler::RunL")

    // Handle errors in RunError().
    PUSHLOG_WRITE_FORMAT(" iStatus.Int(): %d",iStatus.Int())

	switch ( iState )
		{
	    case EGarbageCollecting:
            {
		    CollectGarbageL();
		    break;
            }


        case EFilteringAndParsing:
            {
            if(iPushSLEnabled)
                {
                if ( !FilterPushMsgL() )
                    {
                    // It did not pass the filter. Done.
                    iState = EDone;
                    IdleComplete();
                    }
                else
                    {
                    // Continue.
                    TInt ret = KErrNone;
                    PUSHLOG_WRITE("CSLContentHandler::RunL : before trapping parsing.")
                    TRAP(ret, ParsePushMsgL());
                    PUSHLOG_WRITE_FORMAT("CSLContentHandler::RunL : after trapping parsing. ret = %d", ret)
                    if ( ret != KErrNone)
                        {
                        PUSHLOG_WRITE("CSLContentHandler::RunL : Parsing failed. discarding message.")
                        iState = EDone;
                        IdleComplete();
                        }
                    }
                }
			break;
            }

        case EProcessing:
            {
            if(iPushSLEnabled)
			ProcessingPushMsgEntryL();
			break;
            }

		case EFetching:
            {
            if(iPushSLEnabled)
			FetchPushMsgEntryL();
			break;
            }

		case EFetchCompleted:
            {
            if(iPushSLEnabled)
			FetchCompletedL();
			break;
            }

		case ESavePushMsgEntry:
            {
            if(iPushSLEnabled)
			SavePushMsgEntryL();
			break;
            }


        case EDone:
            {
            PUSHLOG_WRITE("CSLContentHandler EDone")
			Complete( KErrNone );
			break;
            }
		default:
            {
            // JIC.
            PUSHLOG_WRITE("CSLContentHandler default EDone")
			Complete( KErrNone );
			break;
            }
		}

    PUSHLOG_LEAVEFN("CSLContentHandler::RunL")
    }

// ---------------------------------------------------------
// CSLContentHandler::RunError
// ---------------------------------------------------------
//
TInt CSLContentHandler::RunError( TInt aError )
	{
    PUSHLOG_WRITE_FORMAT("CSLContentHandler::RunError: %d",aError)

	iState = EDone;
	Complete( aError );
	return KErrNone;
	}

