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
* Description:  Implementation of CSIContentHandler.
*
*/



// INCLUDE FILES

#include "CSIContentHandler.h"
#include "PushMtmUtil.h"
#include "PushMtmSettings.h"
#include "PushMtmLog.h"
#include "PushMtmUiDef.h"
#include "StringResourceReader.h"
#include "PushContentHandlerPanic.h"
#include "si_dict.h"
#include "PushContentHandlerUtils.h"
#include <push/CSIPushMsgEntry.h>
#include <msvids.h>
#include <PushMtmUi.rsg>
#include <nw_dom_node.h>
#include <nw_dom_document.h>
#include <nw_dom_element.h>
#include <nw_dom_text.h>
#include <nw_wbxml_dictionary.h>
#include <THttpFields.h>

// CONSTANTS

// si attributes / elements
_LIT8( KSi,          "si" );
_LIT8( KIndication,  "indication" );
_LIT8( KHrefAttrib,  "href" );
_LIT8( KSiIdAttrib,  "si-id" );
_LIT8( KCreatedAttrib,   "created" );
_LIT8( KExpiresAttrib,   "si-expires" );
_LIT8( KActionAttrib,    "action" );

// action attribute literals
_LIT8( KDeleteAction,    "delete" );
_LIT8( KSignalNone,      "signal-none" );
_LIT8( KSignalLow,       "signal-low" );
_LIT8( KSignalMedium,    "signal-medium" );
_LIT8( KSignalHigh,      "signal-high" );

_LIT( KSiTextContentType, "text/vnd.wap.si" );

const TInt KValidMaxEncodedDateTimeSize = 7;
const TInt KValidUTCLength = 20; // YYYY-MM-DDTHH:MM:SSZ
const TInt KValidUTCNumericals = 14;
const TInt KValidUTCYearBlockLength = 4;
const TInt KValidUTCOtherBlockLength = 2;
const TUint8 KAsciiZeroCharCode = 0x30;

const TInt KValidTTimeMonthStart = 4;
const TInt KValidTTimeDayStart = 6;
const TInt KValidTTimeHourStart = 8;
const TInt KValidTTimeMinuteStart = 10;
const TInt KValidTTimeSecondStart = 12;
const TInt KValidTTimeBlockLength = 2;

const TInt KValidTTimeLength = 14; // YYYYMMDDHHMMSS

const TInt KNoOfDictArrays = 1;

_LIT( KCharMinus, "-" );
_LIT( KCharT, "T" );
_LIT( KCharColon, ":" );
_LIT( KCharZ, "Z" );

/// Conversion buffer size.
LOCAL_D const TInt KPushConversionBufferSize = 256;
/// Zero width non-breaking space character.
LOCAL_D const TUint16 KPushZeroWidthNbsp = 0xfeff;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CSIContentHandler::NewL
// ---------------------------------------------------------
//
CSIContentHandler* CSIContentHandler::NewL()
	{
    PUSHLOG_ENTERFN("CSIContentHandler::NewL")

	CSIContentHandler* self = new (ELeave) CSIContentHandler;
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop( self );

    PUSHLOG_LEAVEFN("CSIContentHandler::NewL")
	return self;
	}

// ---------------------------------------------------------
// CSIContentHandler::~CSIContentHandler
// ---------------------------------------------------------
//
CSIContentHandler::~CSIContentHandler()
	{
    PUSHLOG_ENTERFN("CSIContentHandler::~CSIContentHandler")

    Cancel();
	delete iHrefBuf;
	delete iSiIdBuf;
	delete iData;
    delete iCharacterSetConverter;
    iCharacterSetConverter = NULL;
    delete iCharacterSetsAvailable;
    iCharacterSetsAvailable = NULL;

    PUSHLOG_LEAVEFN("CSIContentHandler::~CSIContentHandler")
	}

// ---------------------------------------------------------
// CSIContentHandler::CSIContentHandler
// ---------------------------------------------------------
//
CSIContentHandler::CSIContentHandler()
:   CPushContentHandlerBase(),
    iSavedMsgId( KMsvNullIndexEntryId ),
    iPushMsgAction( KErrNotFound ),
    iExpiresTime( Time::NullTTime() ),
    iCreatedTime( Time::NullTTime() )
	{
	}

// ---------------------------------------------------------
// CSIContentHandler::ConstructL
// ---------------------------------------------------------
//
void CSIContentHandler::ConstructL()
	{
    PUSHLOG_ENTERFN("CSIContentHandler::ConstructL")

    CPushContentHandlerBase::ConstructL();
    // Added to Active Scheduler.

    PUSHLOG_LEAVEFN("CSIContentHandler::ConstructL")
    }

// ---------------------------------------------------------
// CSIContentHandler::CollectGarbageL
// ---------------------------------------------------------
//
void CSIContentHandler::CollectGarbageL()
	{
    PUSHLOG_ENTERFN("CSIContentHandler::CollectGarbageL")

    DoCollectGarbageL();

    //Ready.
    iState = EFilteringAndParsing;
	IdleComplete();

    PUSHLOG_LEAVEFN("CSIContentHandler::CollectGarbageL")
    }

// ---------------------------------------------------------
// CSIContentHandler::ParsePushMsgL
// Note that cXML parser dosn't do any validation!
// ---------------------------------------------------------
//
void CSIContentHandler::ParsePushMsgL()
	{
    PUSHLOG_ENTERFN("CSIContentHandler::ParsePushMsgL")

    TPtrC8 bodyPtr;
    iMessage->GetMessageBody( bodyPtr );
    // If there is no body in the message leave with an error
    if ( bodyPtr.Size() == 0 )
        {
        PUSHLOG_WRITE("CSIContentHandler::ParsePushMsgL: Empty body")
        User::Leave( KErrCorrupt );
        }

    // Get content type. It will tell us wheather the msg body is encoded or
    // textual.
    TPtrC contentType;
	iMessage->GetContentType( contentType );
    PUSHLOG_WRITE_FORMAT("CSIContentHandler::ParsePushMsgL: HTTP header - Content type <%S>",&contentType);

    /*
    TPtrC8 encodingPtr;
    TBool encodingFound = iMessage->GetHeaderField
                          ( EHttpContentEncoding, encodingPtr );
    #ifdef __TEST_LOG__
    TBuf<64> encodingBuf;
    encodingBuf.Copy( encodingPtr );
    PUSHLOG_WRITE_FORMAT(" HTTP header - Content encoding <%S>",&encodingBuf);
    #endif // __TEST_LOG__
    */

    // Add SI dictionary.
    NW_WBXML_Dictionary_t* dictArray[ KNoOfDictArrays ] =
        { (NW_WBXML_Dictionary_t*)&NW_SI_WBXMLDictionary };

    NW_Status_t stat = NW_STAT_SUCCESS;

    RWbxmlDictionary wbxmlDict;
    wbxmlDict.InitializeL( KNoOfDictArrays, dictArray );
    CleanupClosePushL<RWbxmlDictionary>( wbxmlDict );

    NW_TinyDom_Handle_t domHandle;
    NW_Byte* buffer = (NW_Byte*)bodyPtr.Ptr();
    NW_Int32 length = (NW_Int32)bodyPtr.Size();
    // Let's use the content type now.
    NW_Bool encoded = ( contentType.CompareF( KSiTextContentType ) == 0 ) ?
                                                         NW_FALSE : NW_TRUE;
    // SI public identifier.
    NW_Uint32 publicID = NW_SI_PublicId;
    NW_Bool extTNotStringTable = NW_FALSE;
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
		PUSHLOG_WRITE("CSIContentHandler::ParsePushMsgL: domNode is Null")
		}
    User::LeaveIfNull( domNode );
	PUSHLOG_WRITE("CSIContentHandler::ParsePushMsgL: domNode is not Null") // to be deleted
    // Let domNode be on the Cleanup Stack.
    docTreeOwner->SetDocTree( domNode );

    // It must be a document node.
    type = NW_DOM_Node_getNodeType( domNode );
    if ( type != NW_DOM_DOCUMENT_NODE )
        {
        PUSHLOG_WRITE_FORMAT("CSIContentHandler::ParsePushMsgL: Not Document node <%d>",type)
        User::Leave( KErrArgument );
        }

    // Get character encoding (NW_Uint32)
    iCharEncoding = NW_DOM_DocumentNode_getCharacterEncoding( domNode );
    PUSHLOG_WRITE_FORMAT("CSIContentHandler::ParsePushMsgL: Doc char encoding <%x>",iCharEncoding)

    /**********************************
    *   ELEMENT si
    ***********************************/
    // Get the first element of the document that must be an si.
	// first make sure if there any children in the dom tree, otherwise we will PANIC(in NW_DOM_DocumentNode_getDocumentElement) and crash WatcherMainThread.
	TBool domNodeHasChildNodes = EFalse;
	domNodeHasChildNodes = NW_DOM_Node_hasChildNodes( domNode );
	PUSHLOG_WRITE_FORMAT("CSIContentHandler::ParsePushMsgL: check if Dom tree has <SI> node <%d>", domNodeHasChildNodes)
	if (!domNodeHasChildNodes)
        {
        PUSHLOG_WRITE("CSIContentHandler::ParsePushMsgL: No SI element present in the dom tree. Message corrupted.")
        User::Leave( KErrCorrupt );
        }

	PUSHLOG_WRITE("CSIContentHandler::ParsePushMsgL: before calling getDocumentElement")
    NW_DOM_ElementNode_t* siElement =
        NW_DOM_DocumentNode_getDocumentElement( domNode );
	PUSHLOG_WRITE("CSIContentHandler::ParsePushMsgL: after calling getDocumentElement")
	if (!siElement)
		{
		PUSHLOG_WRITE("CSIContentHandler::ParsePushMsgL: siElement is Null")
		}
	PUSHLOG_WRITE("CSIContentHandler::ParsePushMsgL: siElement is not Null, before leaving")
    User::LeaveIfNull( siElement );
	PUSHLOG_WRITE("CSIContentHandler::ParsePushMsgL: siElement is not Null, after leaving if siElement is null")

    type = NW_DOM_Node_getNodeType( siElement );

    CStringOwner* stringOwner = new (ELeave) CStringOwner;
    CleanupStack::PushL( stringOwner );

    NW_String_t* name = NW_String_new();
    User::LeaveIfNull( name );
    // Let name be on the Cleanup Stack.
    stringOwner->SetString( name );
    stat = NW_DOM_Node_getNodeName( siElement, name );
    User::LeaveIfError( NwxStatusToErrCode( stat ) );
    NW_Byte*  nameBuf = NW_String_getStorage( name );
    NW_Uint16 nameLen = NW_String_getCharCount( name, iCharEncoding );
    TPtrC8 namePtr( nameBuf, nameLen );

    // Now comes the validity check.
    if ( type != NW_DOM_ELEMENT_NODE || namePtr.CompareF( KSi ) != 0 )
        {
        PUSHLOG_WRITE_FORMAT("CSIContentHandler::ParsePushMsgL: Not si element node <%d>",type)
        User::Leave( KErrArgument );
        }

    CleanupStack::PopAndDestroy( stringOwner ); // stringOwner

    /**********************************
    *   ELEMENT indication
    ***********************************/
    if ( NW_DOM_Node_hasChildNodes( siElement ) )
        {
        NW_DOM_Node_t* node = NW_DOM_Node_getFirstChild( siElement );
		if (!node)
			{
			PUSHLOG_WRITE("CSIContentHandler::ParsePushMsgL: no si child nodes!")
			}
        User::LeaveIfNull( node );

        // Find the indication element.
        TBool indicationFound = EFalse;
        do {
            type = NW_DOM_Node_getNodeType( node );

            CStringOwner* stringOwner = new (ELeave) CStringOwner;
            CleanupStack::PushL( stringOwner );

            NW_String_t* name = NW_String_new();
            User::LeaveIfNull( name );
            stringOwner->SetString( name );
            stat = NW_DOM_Node_getNodeName( node, name );
			PUSHLOG_WRITE_FORMAT("CSIContentHandler::ParsePushMsgL: getNodeName ErrCode: %d", NwxStatusToErrCode( stat ))
            User::LeaveIfError( NwxStatusToErrCode( stat ) );
            NW_Byte*  nameBuf = NW_String_getStorage( name );
            NW_Uint16 nameLen = NW_String_getCharCount( name,
                                                        iCharEncoding );
            TPtrC8 namePtr( nameBuf, nameLen );

            if ( type == NW_DOM_ELEMENT_NODE &&
                 namePtr.CompareF( KIndication ) == 0 )
                {
                // We found the indication element. Parse it.
                PUSHLOG_WRITE("CSIContentHandler::ParsePushMsgL: indication under si found.")
                indicationFound = ETrue;
                NW_DOM_ElementNode_t* indicationElement =
                    REINTERPRET_CAST( NW_DOM_ElementNode_t*, node );
                ParseIndicationL( *indicationElement );
                }

            CleanupStack::PopAndDestroy( stringOwner ); // stringOwner

            if ( !indicationFound )
                {
                // Iterate next.
                node = NW_DOM_Node_getNextSibling( node );
                if ( !node )
                    {
                    PUSHLOG_WRITE("CSIContentHandler::ParsePushMsgL: No more sibling.")
                    break;
                    }
                }

            } while ( !indicationFound );
        }

    // Cleanup.
    CleanupStack::PopAndDestroy( 2, &wbxmlDict ); // docTreeOwner, wbxmlDict

    if ( !ActionFlag() )
        {
        // default if no action explicitly stated
        iPushMsgAction = CSIPushMsgEntry::ESIPushMsgSignalMedium;
        SetActionFlag( ETrue );
        PUSHLOG_WRITE_FORMAT("CSIContentHandler::ParsePushMsgL: Defaulting to ActionFlag: %d",iPushMsgAction)
        }

    iState = EProcessing;
	IdleComplete();

    PUSHLOG_LEAVEFN("CSIContentHandler::ParsePushMsgL")
	}

// ---------------------------------------------------------
// CSIContentHandler::ParseIndicationL
// ---------------------------------------------------------
//
void CSIContentHandler::ParseIndicationL( NW_DOM_ElementNode_t& aIndication )
	{
    PUSHLOG_ENTERFN("CSIContentHandler::ParseIndicationL")

    NW_Status_t stat = NW_STAT_SUCCESS;
    NW_DOM_NodeType_t type = 0;

    if ( NW_DOM_ElementNode_hasAttributes( &aIndication ) )
        {
        NW_DOM_AttributeListIterator_t attrIter;
        stat = NW_DOM_ElementNode_getAttributeListIterator
                             ( &aIndication, &attrIter );
		PUSHLOG_WRITE_FORMAT("CSIContentHandler::ParseIndicationL: getAttribListIter ErrCode: %d", NwxStatusToErrCode( stat ))
        User::LeaveIfError( NwxStatusToErrCode( stat ) );

        NW_DOM_AttributeHandle_t attrHandle;
        while ( NW_DOM_AttributeListIterator_getNextAttribute
                ( &attrIter, &attrHandle ) == NW_STAT_WBXML_ITERATE_MORE )
            {
            ParseIndAttributeL( attrHandle );
            }
        }

    /**********************************
    *   PCDATA of ELEMENT indication
    ***********************************/
    if ( NW_DOM_Node_hasChildNodes( &aIndication ) )
        {
        NW_DOM_TextNode_t* textNode =
            NW_DOM_Node_getFirstChild( &aIndication );
        User::LeaveIfNull( textNode );

        type = NW_DOM_Node_getNodeType( textNode );
        if ( type != NW_DOM_TEXT_NODE )
            {
            PUSHLOG_WRITE_FORMAT("CSIContentHandler::ParseIndicationL: Not text node <%d>",type)
            User::Leave( KErrArgument );
            }

        ParseTextL( *textNode );
        }

    PUSHLOG_LEAVEFN("CSIContentHandler::ParseIndicationL")
    }

// ---------------------------------------------------------
// CSIContentHandler::ParseIndAttributeL
// ---------------------------------------------------------
//
void CSIContentHandler::ParseIndAttributeL( NW_DOM_AttributeHandle_t&
                                            aAttrHandle )
	{
    PUSHLOG_ENTERFN("CSIContentHandler::ParseIndAttributeL")

    NW_Status_t stat = NW_STAT_SUCCESS;

    CStringOwner* stringOwner = new (ELeave) CStringOwner;
    CleanupStack::PushL( stringOwner );

    NW_String_t* attrName = NW_String_new();
    User::LeaveIfNull( attrName );
    stringOwner->SetString( attrName );

    // Get the name of the attribute.
    stat = NW_DOM_AttributeHandle_getName( &aAttrHandle, attrName );
    User::LeaveIfError( NwxStatusToErrCode( stat ) );
    NW_Byte*  attrNameBuf = NW_String_getStorage( attrName );
    NW_Uint16 attrNameLen = NW_String_getCharCount( attrName, iCharEncoding );
    TPtrC8 attrNamePtr( attrNameBuf, attrNameLen );

    if ( attrNamePtr.CompareF( KCreatedAttrib ) == 0 )
        {
        if ( CreatedFlag() )
            {
            PUSHLOG_WRITE(" created redefinition")
            User::Leave( KErrCorrupt );
            }
        else
            {
            TBool gotDate = AttributeToTTimeL( aAttrHandle, iCreatedTime );
            SetCreatedFlag( gotDate );
            PUSHLOG_WRITE_FORMAT(" iCreatedTime set %d",gotDate?1:0)
            }
        }
    else if ( attrNamePtr.CompareF( KHrefAttrib ) == 0 )
        {
        if ( HrefFlag() )
            {
            PUSHLOG_WRITE(" href redefinition")
            User::Leave( KErrCorrupt );
            }
        else
            {
            CStringOwner* stringOwner = new (ELeave) CStringOwner;
            CleanupStack::PushL( stringOwner );

            NW_String_t* val = NW_String_new();
            User::LeaveIfNull( val );
            stringOwner->SetString( val );
            stat = NW_DOM_AttributeHandle_getValue( &aAttrHandle, val );
            if ( stat != NW_STAT_DOM_NO_STRING_RETURNED )
                {
                User::LeaveIfError( NwxStatusToErrCode( stat ) );
                NW_Byte* storage = NW_String_getStorage( val );
                NW_Uint16 length = NW_String_getCharCount( val,
                                                           iCharEncoding );
                if ( length == 0 )
                    {
                    // Zero length href attribute is considered as missing.
                    PUSHLOG_WRITE(" Zero length HrefFlag");
                    }
                else
                    {
                    TPtrC8 hrefPtr( storage, length );
                    HBufC* tempHrefBuf = HBufC::NewMaxL( length );
                    // No leavable after it!!! until...
                    tempHrefBuf->Des().Copy( hrefPtr );
                    iHrefBuf = tempHrefBuf; // ...until here.
                    SetHrefFlag( ETrue );
                    PUSHLOG_WRITE_FORMAT(" HrefFlag set <%S>",iHrefBuf);
                    }
                }

            CleanupStack::PopAndDestroy( stringOwner ); // stringOwner
            }
        }
    else if ( attrNamePtr.CompareF( KExpiresAttrib ) == 0 )
        {
        if ( ExpiresFlag() )
            {
            PUSHLOG_WRITE(" expires redefinition")
            User::Leave( KErrCorrupt );
            }
        else
            {
            TBool gotDate = AttributeToTTimeL( aAttrHandle, iExpiresTime );
            SetExpiresFlag( gotDate );
            PUSHLOG_WRITE_FORMAT(" iExpiresTime set %d",gotDate?1:0)
            }
        }
    else if ( attrNamePtr.CompareF( KSiIdAttrib ) == 0 )
        {
        if ( SiIdFlag() )
            {
            PUSHLOG_WRITE(" si-id redefinition")
            User::Leave( KErrCorrupt );
            }
        else
            {
            // It is expected to be String.
            CStringOwner* stringOwner = new (ELeave) CStringOwner;
            CleanupStack::PushL( stringOwner );

            NW_String_t* val = NW_String_new();
            User::LeaveIfNull( val );
            stringOwner->SetString( val );
            stat = NW_DOM_AttributeHandle_getValue( &aAttrHandle, val );
            User::LeaveIfError( NwxStatusToErrCode( stat ) );
            NW_Byte* storage = NW_String_getStorage( val );
            NW_Uint16 length = NW_String_getCharCount( val, iCharEncoding );
            TPtrC8 siidPtr( storage, length );

            iSiIdBuf = HBufC::NewMaxL( siidPtr.Length() );
            iSiIdBuf->Des().Copy( siidPtr );
            SetSiIdFlag( ETrue );
            PUSHLOG_WRITE_FORMAT(" SiIdFlag set <%S>",iSiIdBuf)

            CleanupStack::PopAndDestroy( stringOwner ); // stringOwner
            }
        }
    else if ( attrNamePtr.CompareF( KActionAttrib ) == 0 )
        {
        if ( ActionFlag() )
            {
            PUSHLOG_WRITE(" action redefinition")
            User::Leave( KErrCorrupt );
            }
        else
            {
            // It is expected to be String.
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

            CleanupStack::PopAndDestroy( stringOwner ); // stringOwner
            }
        }
    else
        {
        __ASSERT_DEBUG( EFalse,
            ContHandPanic( EPushContHandPanUnexpSiToken ) );
        }

    CleanupStack::PopAndDestroy( stringOwner ); // stringOwner

    PUSHLOG_LEAVEFN("CSIContentHandler::ParseIndAttributeL")
    }

// ---------------------------------------------------------
// CSIContentHandler::ParseTextL
// ---------------------------------------------------------
//
void CSIContentHandler::ParseTextL( NW_DOM_TextNode_t& aTextNode )
	{
    PUSHLOG_ENTERFN("CSIContentHandler::ParseTextL")

    if ( DataFlag() )
        {
        PUSHLOG_WRITE(" Data flag already set.")
        }
    else
        {
        CStringOwner* stringOwner = new (ELeave) CStringOwner;
        CleanupStack::PushL( stringOwner );

        NW_String_t* data = NW_String_new();
        User::LeaveIfNull( data );
        stringOwner->SetString( data );
        NW_Status_t stat = NW_STAT_SUCCESS;
        stat = NW_DOM_TextNode_getData( &aTextNode, data );
        User::LeaveIfError( NwxStatusToErrCode( stat ) );

        HBufC16* ucs2buffer = ConvertToUnicodeL( *data, iCharEncoding );
        // Be careful: ucs2buffer is not on the CleanupStack!
        __ASSERT_DEBUG( ucs2buffer != 0, ContHandPanic( EPushContHandPanNullUcs2Buf ) );

        TPtrC16 ucs2ptrC( *ucs2buffer );
        if ( ucs2ptrC.Length() == 0 )
            {
            // Zero length data is considered as nothing.
            PUSHLOG_WRITE(" Zero length Data");
            }
        else
            {
            PUSHLOG_WRITE_FORMAT(" Data: <%S>",&ucs2ptrC);

            #ifdef __TEST_LOG__
            // Write out each unicode character identifier
            TInt length = ucs2ptrC.Length();
            for (TInt logI=0;logI<length;logI++)
                {
                TBuf16<1> currChar;
                currChar.Copy( ucs2ptrC.Mid( logI, /*aLength*/1 ) );
                PUSHLOG_WRITE_FORMAT2(" 0x%x %S",currChar[0],&currChar);
                }
            #endif // __TEST_LOG__

            iData = ucs2buffer; // Ownership transferred.
            ucs2buffer = NULL;
            SetDataFlag( ETrue );
            }

        CleanupStack::PopAndDestroy( stringOwner );
        }

    PUSHLOG_LEAVEFN("CSIContentHandler::ParseTextL")
    }

// ---------------------------------------------------------
// CSIContentHandler::ConvertToUnicodeL
// ---------------------------------------------------------
//
HBufC16* CSIContentHandler::ConvertToUnicodeL( const TDesC8& aSrc, TUint aCharSetId )
	{
    PUSHLOG_ENTERFN("CSIContentHandler::ConvertToUnicodeL");

    __ASSERT_DEBUG( aCharSetId != 0, ContHandPanic( EPushContHandPanNullCharSetId ) );

    InitialiseCharacterSetConverterL();

    HBufC16* ucs2buffer = NULL; // The return value.
    TBool resultOnStack = EFalse;

    #ifdef __TEST_LOG__
    // Write out the original 8-bit buffer
    TInt origiLength = aSrc.Length();
    for (TInt origiLogI=0;origiLogI<origiLength;origiLogI++)
        {
        TBuf16<1> currChar; // Only 16-bit buffer can be written out.
        currChar.Copy( aSrc.Mid( origiLogI, /*aLength*/1 ) );
        PUSHLOG_WRITE_FORMAT2(" 0x%x %S",currChar[0],&currChar);
        }
    #endif // __TEST_LOG__

    // Result
    HBufC16* buffer = HBufC16::NewLC( KPushConversionBufferSize );
    PUSHLOG_WRITE(" buffer allocated");
    TPtr16 ptr( buffer->Des() );

    // Prepare conversion for the given charset ID.
    RFs& fs = iMsvSession->FileSession();
    iCharacterSetConverter->PrepareToConvertToOrFromL
        ( aCharSetId, *iCharacterSetsAvailable, fs );
    PUSHLOG_WRITE(" PrepareToConvertToOrFromL OK");

    TInt state = 0;
    TInt remaining = iCharacterSetConverter->ConvertToUnicode( ptr, aSrc, state );
    PUSHLOG_WRITE_FORMAT(" remaining: %d",remaining);
    while ( remaining >= 0 )
        {
        if ( ucs2buffer == NULL )
            {
            ucs2buffer = HBufC::NewLC( ptr.Length() );
            resultOnStack = ETrue;
            }
        else
            {
            __ASSERT_DEBUG( resultOnStack,
                ContHandPanic( EPushContHandPanSiResNotOnStack ) );
            // This may change the address of ucs2buffer so we need to put
            // it on the cleanup stack again!!
            ucs2buffer = ucs2buffer->ReAllocL
                ( ucs2buffer->Length() + ptr.Length() );
            CleanupStack::Pop();    // old ucs2buffer
            CleanupStack::PushL( ucs2buffer );  // possibly new copy
            PUSHLOG_WRITE(" ucs2buffer reallocated");
            }
        TPtr16 ucs2ptr( ucs2buffer->Des() );
        ucs2ptr.Append( ptr );
        if ( remaining > 0 )
            {
            // Try to convert all remaining characters.
            ptr.Zero();
            remaining = iCharacterSetConverter->ConvertToUnicode
                ( ptr, aSrc.Right( remaining ), state );
            PUSHLOG_WRITE_FORMAT(" remaining: %d",remaining);
            }
        else
            {
            PUSHLOG_WRITE(" break");
            break;
            }
        }

    if ( resultOnStack )
        {
        CleanupStack::Pop();    // ucs2buffer
        resultOnStack = EFalse;
        }

    // ucs2buffer is not on the CleanupStack!

    CleanupStack::PopAndDestroy( buffer ); // buffer

    if ( ucs2buffer == NULL )
        {
        PUSHLOG_WRITE(" NULL ucs2buffer - allocating an empty buf");
        ucs2buffer = KNullDesC().AllocL();
        }
    else
        {
        // Check if first character is a Zero-width nbsp.
        TPtrC16 ucs2ptrC( *ucs2buffer );
        if ( ucs2ptrC.Length() >= 1 && ucs2ptrC[0] == KPushZeroWidthNbsp )
            {
            // First character is a Zero-width NBSP. This character is used as
            // BOM in some encodings and should not be present at this point.
            // But we are tolerant and remove it.
            // (Not expecting big-endianness here.)
            HBufC16* temp = ucs2buffer;
            CleanupStack::PushL( temp );
            ucs2buffer = ucs2ptrC.Mid( 1 ).AllocL();
            CleanupStack::PopAndDestroy( temp ); // temp
            PUSHLOG_WRITE(" BOM removed");
            }
        else
            {
            PUSHLOG_WRITE(" No BOM");
            }
        }


    PUSHLOG_LEAVEFN("CSIContentHandler::ConvertToUnicodeL");
    return ucs2buffer;
    }

// ---------------------------------------------------------
// CSIContentHandler::ConvertToUnicodeL
// ---------------------------------------------------------
//
HBufC16* CSIContentHandler::ConvertToUnicodeL
    ( NW_String_t& aString, NW_Uint32 aCharEncoding )
	{
    PUSHLOG_ENTERFN("CSIContentHandler::ConvertToUnicodeL");

    /* As cXmlLibrary does, we support only the following charsets:
    #define HTTP_iso_10646_ucs_2        0x03E8
    #define HTTP_iso_8859_1             0x04
    #define HTTP_us_ascii               0x03
    #define HTTP_utf_8                  0x6A
    #define HTTP_utf_16                 1015
    */
    TUint id = 0;
    if ( aCharEncoding == HTTP_iso_10646_ucs_2 )
        {
        id = KCharacterSetIdentifierUcs2;
        PUSHLOG_WRITE(" KCharacterSetIdentifierUcs2")
        }
    else if ( aCharEncoding == HTTP_iso_8859_1 )
        {
        id = KCharacterSetIdentifierIso88591;
        PUSHLOG_WRITE(" KCharacterSetIdentifierIso88591")
        }
    else if ( aCharEncoding == HTTP_us_ascii )
        {
        id = KCharacterSetIdentifierAscii;
        PUSHLOG_WRITE(" KCharacterSetIdentifierAscii")
        }
    else if ( aCharEncoding == HTTP_utf_8 )
        {
        id = KCharacterSetIdentifierUtf8;
        PUSHLOG_WRITE(" KCharacterSetIdentifierUtf8")
        }
    else if ( aCharEncoding == HTTP_utf_16 ) // No such in CharConv.h
        {
        id = KCharacterSetIdentifierUcs2;
        PUSHLOG_WRITE(" KCharacterSetIdentifierUcs2")
        }
    else
        {
        id = KCharacterSetIdentifierUtf8; // Defaulting to UTF-8
        PUSHLOG_WRITE(" DEFAULTING to KCharacterSetIdentifierUtf8");
        }

    PUSHLOG_WRITE_FORMAT(" id: 0x%x",id);
    __ASSERT_DEBUG( id != 0, ContHandPanic( EPushContHandPanNullCharSetId ) );

    // Source
    PUSHLOG_WRITE_FORMAT(" Storage: 0x%x",NW_String_getStorage(&aString));
    PUSHLOG_WRITE_FORMAT(" Byte count: %d",NW_String_getByteCount(&aString)-1);

    // We will use NW_String_getByteCount(&aString)-1 as size, because
    // NW_String_getByteCount(&aString) includes NULL terminator.
    const TPtrC8 src( NW_String_getStorage(&aString),
                      NW_String_getByteCount(&aString)-1 );
    HBufC16* ucs2buffer = ConvertToUnicodeL( src, id );

    PUSHLOG_LEAVEFN("CSIContentHandler::ConvertToUnicodeL");
    return ucs2buffer;
    }

// ---------------------------------------------------------
// CSIContentHandler::InitialiseCharacterSetConverterL
// ---------------------------------------------------------
//
void CSIContentHandler::InitialiseCharacterSetConverterL()
	{
    PUSHLOG_ENTERFN("CSIContentHandler::InitialiseCharacterSetConverterL")

    iCharacterSetConverter = CCnvCharacterSetConverter::NewL();

    RFs& fs = iMsvSession->FileSession();
    iCharacterSetsAvailable =
        CCnvCharacterSetConverter::CreateArrayOfCharacterSetsAvailableL( fs );

    PUSHLOG_LEAVEFN("CSIContentHandler::InitialiseCharacterSetConverterL")
    }

// ---------------------------------------------------------
// CSIContentHandler::ConvertActionString
// ---------------------------------------------------------
//
TUint CSIContentHandler::ConvertActionString
                         ( const TDesC8& aActionString ) const
	{
	const TInt KMatchFound = 0;

	// set to default signal value (to rid ourselves of build warning)
	TUint actionValue = CSIPushMsgEntry::ESIPushMsgSignalMedium;

	if ( aActionString.Compare( KDeleteAction ) == KMatchFound )
        {
		actionValue = CSIPushMsgEntry::ESIPushMsgDelete;
        }
	else if ( aActionString.Compare( KSignalNone ) == KMatchFound )
        {
		actionValue = CSIPushMsgEntry::ESIPushMsgSignalNone;
        }
	else if ( aActionString.Compare( KSignalLow ) == KMatchFound )
        {
		actionValue = CSIPushMsgEntry::ESIPushMsgSignalLow;
        }
	else if ( aActionString.Compare( KSignalMedium ) == KMatchFound )
        {
		actionValue = CSIPushMsgEntry::ESIPushMsgSignalMedium;
        }
	else if ( aActionString.Compare( KSignalHigh ) == KMatchFound )
        {
		actionValue = CSIPushMsgEntry::ESIPushMsgSignalHigh;
        }

	return actionValue;
	}

// ---------------------------------------------------------
// CSIContentHandler::SetSIPushMsgEntryFieldsL
// ---------------------------------------------------------
//
void CSIContentHandler::SetSIPushMsgEntryFieldsL( CSIPushMsgEntry&
                                                  aSIPushMsgEntry )
	{
    PUSHLOG_ENTERFN("CSIContentHandler::SetSIPushMsgEntryFieldsL")

	if ( SiIdFlag() || HrefFlag() )
		{
		if ( SiIdFlag() && ( HrefFlag() == EFalse ) )
			{
            // Message has only si-id but no href.
            aSIPushMsgEntry.SetIdL( *iSiIdBuf );
			}
		else if ( HrefFlag() && ( SiIdFlag() == EFalse ) )
			{
            // If message has no si-id but does have a href, use href as si-id.
            aSIPushMsgEntry.SetIdL( *iHrefBuf );
            aSIPushMsgEntry.SetUrlL( *iHrefBuf );
			}
		else
            {
            // Use si-id and href as is.
            aSIPushMsgEntry.SetIdL( *iSiIdBuf );
            aSIPushMsgEntry.SetUrlL( *iHrefBuf );
            }
		}

    __ASSERT_DEBUG( ActionFlag(),
                    ContHandPanic( EPushContHandPanUnspecSiAction ) );
	if ( ActionFlag() )
        {
		aSIPushMsgEntry.SetAction( iPushMsgAction );
        }
	else // default if no action explicitly stated
        {
		aSIPushMsgEntry.SetAction( CSIPushMsgEntry::ESIPushMsgSignalMedium );
        }

	// uses default null time value if no explicit date set in message
	aSIPushMsgEntry.SetCreated( iCreatedTime );
	aSIPushMsgEntry.SetExpires( iExpiresTime );

	// PCDATA (text) from message
	if ( DataFlag() )
        {
		aSIPushMsgEntry.SetTextL( *iData );
        }

	TPtrC8 msgHeaderPtr;
	iMessage->GetHeader( msgHeaderPtr );
	aSIPushMsgEntry.SetHeaderL( msgHeaderPtr );

    // Get server address.
    TPtrC8 srvAddress;
    if ( iMessage->GetServerAddress( srvAddress ) )
        {
	    aSIPushMsgEntry.SetFromL( srvAddress );
        }

    // First line in Inbox: TMsvEntry::iDetails.
    if ( srvAddress.Length() == 0 )
        {
        // Read from resource.
        HBufC* details =
            iStrRscReader->AllocReadResourceLC( R_PUSHMISC_UNK_SENDER );
        aSIPushMsgEntry.SetMsgDetailsL( *details );
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
        aSIPushMsgEntry.SetMsgDetailsL( *convertedFrom );
        //
        CleanupStack::PopAndDestroy( 2, details ); // convertedFrom, details
        }

    // Second line in Inbox: TMsvEntry::iDescription.
	if ( DataFlag() )
        {
        // Display SI message.
		aSIPushMsgEntry.SetMsgDescriptionL( *iData );
        }
    else
        {
        // Display URL.
        __ASSERT_DEBUG( HrefFlag(),
                        ContHandPanic( EPushContHandPanUnspecSiHref ) );
        const TPtrC url = aSIPushMsgEntry.Url();
        HBufC* convertedUrl = CPushMtmUtil::ConvertUriToDisplayFormL( url );
        CleanupStack::PushL( convertedUrl );
        //
        aSIPushMsgEntry.SetMsgDescriptionL( *convertedUrl );
        //
        CleanupStack::PopAndDestroy( convertedUrl ); // convertedUrl
        }

    // ******** Push MTM specific processing *********

    /*
    * Unfortunately in CPushMsgEntryBase there is no such functionality
    * with which we can reach TMsvEntry as non-const, but we have to
    * modify the entry's iMtmData2 member somehow. We can do it
    * with either casting or with modifying and saving the entry
    * manually after it has been saved by CSIPushMsgEntry. The latter
    * solution is more expensive so we choose the first.
    */
    TMsvEntry& tEntry = CONST_CAST( TMsvEntry&, aSIPushMsgEntry.Entry() );
    if ( HrefFlag() )
        {
        CPushMtmUtil::SetAttrs( tEntry, EPushMtmAttrHasHref );
        }
    else
        {
        CPushMtmUtil::ResetAttrs( tEntry, EPushMtmAttrHasHref );
        }

    // *** Set the entry to unread and new state. ***

    tEntry.SetNew( ETrue );
    tEntry.SetUnread( ETrue );

    PUSHLOG_LEAVEFN("CSIContentHandler::SetSIPushMsgEntryFieldsL")
	}

// ---------------------------------------------------------
// CSIContentHandler::ProcessingPushMsgEntryL
// ---------------------------------------------------------
//
void CSIContentHandler::ProcessingPushMsgEntryL()
	{
    PUSHLOG_ENTERFN("CSIContentHandler::ProcessingPushMsgEntryL")

	TBool deletePushMsg( EFalse );

    __ASSERT_DEBUG( ActionFlag(),
                    ContHandPanic( EPushContHandPanUnspecSiAction ) );

    // S60 requirement: if both the href and the message is empty then
    // delete the msg.
    if ( HrefFlag() == EFalse && DataFlag() == EFalse )
        {
        deletePushMsg = ETrue;
        }

    // Expiration.
    if ( !deletePushMsg && ExpiresFlag() )
        {
	    TTime today;
	    today.UniversalTime();
#ifdef __TEST_LOG__
        _LIT( KDateFormat, "%E%D%X%N%Y %1 %2 %3" );
        _LIT( KTimeFormat, "%-B%:0%J%:1%T%:2%S%:3%+B" );
        TBuf<32> dateHolder;
        TBuf<32> timeHolder;
        today.FormatL( dateHolder, KDateFormat );
        today.FormatL( timeHolder, KTimeFormat );
        PUSHLOG_WRITE_FORMAT(" now date: <%S>",&dateHolder)
        PUSHLOG_WRITE_FORMAT(" now time: <%S>",&timeHolder)
        iExpiresTime.FormatL( dateHolder, KDateFormat );
        iExpiresTime.FormatL( timeHolder, KTimeFormat );
        PUSHLOG_WRITE_FORMAT(" exp date: <%S>",&dateHolder)
        PUSHLOG_WRITE_FORMAT(" exp time: <%S>",&timeHolder)
#endif // __TEST_LOG__
	    // check if message has expiry date before today's date
	    if ( iExpiresTime < today )
		    {
            PUSHLOG_WRITE("CSIContentHandler already expired")
		    deletePushMsg = ETrue;
		    }
        }

	// An SI with the action attribute set to “delete” MUST have an
    // explicitly assigned value for si-id.
	if ( !deletePushMsg && ActionFlag() )
		{
		if ( iPushMsgAction == CSIPushMsgEntry::ESIPushMsgDelete )
            {
            if ( !SiIdFlag() || ( SiIdFlag() && iSiIdBuf->Length() == 0 ) )
                {
                deletePushMsg = ETrue;
                }
            }
        }

    // Handling out of order delivery & Replacement.
    TMsvId matchingEntryId = KMsvNullIndexEntryId;

    if ( !deletePushMsg && ( SiIdFlag() || HrefFlag() ) && CreatedFlag() )
        {
        deletePushMsg = HandleMsgOrderReceptionL( matchingEntryId );
        }

    if ( !deletePushMsg && ActionFlag() )
        {
        // SI with action=signal-none must not be presented to the end-user.
        // Note. In S60 signal-none behaves the same as delete: the
        // message is discarded after processing it!
        if ( iPushMsgAction == CSIPushMsgEntry::ESIPushMsgSignalNone )
            {
            deletePushMsg = ETrue;
            }
        // SI with action=delete must also be discarded.
        else if ( iPushMsgAction == CSIPushMsgEntry::ESIPushMsgDelete )
            {
            deletePushMsg = ETrue;
            }
        }

	// Store message if not marked for deletion.
	if ( !deletePushMsg )
        {
		StoreSIMessageL( matchingEntryId );
        }
    else
        {
        // The new entry must be discarded.
        // Delete the corresponding matching entry, too.
        if ( matchingEntryId != KMsvNullIndexEntryId )
            {
            iWapPushUtils->DeleteEntryL( matchingEntryId );
            }
        }

	iState = EDone;
	IdleComplete();

    PUSHLOG_LEAVEFN("CSIContentHandler::ProcessingPushMsgEntryL")
	}

// ---------------------------------------------------------
// CSIContentHandler::StoreSIMessageL
// ---------------------------------------------------------
//
void CSIContentHandler::StoreSIMessageL( TMsvId aMatchingEntryId )
	{
    PUSHLOG_ENTERFN("CSIContentHandler::StoreSIMessageL")

	CSIPushMsgEntry* siEntry = CSIPushMsgEntry::NewL();
	CleanupStack::PushL( siEntry );

    if ( aMatchingEntryId != KMsvNullIndexEntryId )
    {
       PUSHLOG_WRITE("Matching SI found");
       //Delete this old entry
       iWapPushUtils->DeleteEntryL( aMatchingEntryId );
    }

    SetSIPushMsgEntryFieldsL( *siEntry );
    iSavedMsgId = siEntry->SaveL( *iMsvSession, KMsvGlobalInBoxIndexEntryId );

#ifdef __TEST_LOG__
        _LIT( KDateFormat, "%E%D%X%N%Y %1 %2 %3" );
        _LIT( KTimeFormat, "%-B%:0%J%:1%T%:2%S%:3%+B" );
        TBuf<32> dateHolder;
        TBuf<32> timeHolder;
        TTime recDateTime = siEntry->ReceivedDate();
        recDateTime.FormatL( dateHolder, KDateFormat );
        recDateTime.FormatL( timeHolder, KTimeFormat );
        PUSHLOG_WRITE_FORMAT(" rec date: <%S>",&dateHolder)
        PUSHLOG_WRITE_FORMAT(" rec time: <%S>",&timeHolder)
#endif // __TEST_LOG__

	CleanupStack::PopAndDestroy( siEntry ); // siEntry

    PUSHLOG_LEAVEFN("CSIContentHandler::StoreSIMessageL")
	}

// ---------------------------------------------------------
// CSIContentHandler::HandleMsgOrderReceptionL
// ---------------------------------------------------------
//
TBool CSIContentHandler::HandleMsgOrderReceptionL( TMsvId& aMatchingEntryId )
	{
    PUSHLOG_ENTERFN("CSIContentHandler::HandleMsgOrderReceptionL")

    __ASSERT_DEBUG( ( SiIdFlag() || HrefFlag() ),
            ContHandPanic( EPushContHandPanNoSiIdOrHrefAttr ) );
    __ASSERT_DEBUG( CreatedFlag(),
            ContHandPanic( EPushContHandPanNoCreatedAttr ) );

    CMsvEntrySelection* matchingIdList = NULL;
	TBool discardPushMsg( EFalse );

	// Get list of matching stored SI messages.
	if ( SiIdFlag() && iSiIdBuf->Length() != 0 )
        {
		matchingIdList = iWapPushUtils->FindSiIdLC( *iSiIdBuf );
        }
	else // HrefFlag()
        {
        // Use href as si-id.
		matchingIdList = iWapPushUtils->FindSiIdLC( *iHrefBuf );
        }
    const TInt matchingListCount( matchingIdList->Count() );
    // Note that the count can be greater than 1.

    PUSHLOG_WRITE_FORMAT("CSIContentHandler Count: %d",matchingListCount)

	if ( 0 < matchingListCount && CreatedFlag() )
		{
		CSIPushMsgEntry* siEntry = CSIPushMsgEntry::NewL();
		CleanupStack::PushL( siEntry );

		// Delete older stored messages and/or mark current message for
        // deletion if same date or older than stored messages
        TBool foundOneToBeReplaced = EFalse;
		for ( TInt count = 0; count < matchingListCount; ++count )
			{
			TMsvId matchingSiMsgEntryId( matchingIdList->At(count) );

            siEntry->RetrieveL( *iMsvSession, matchingSiMsgEntryId );

			// Skip date comparisons if creation date not valid -
            // SI without created attribute never gets replaced.
			TTime existingSiCreatedTime( siEntry->Created() );

			if ( existingSiCreatedTime == Time::NullTTime() )
                {
				// continue;
                }
            else
                {
                __ASSERT_DEBUG( !foundOneToBeReplaced,
                                ContHandPanic( EPushContHandPanTooManySi ) );
                if ( foundOneToBeReplaced )
                    {
                    PUSHLOG_WRITE(" Already found one")
                    // Only one SI has to be found.
                    // If the program runs into it, then make a
                    // garbage collection to ensure consistency and
                    // remove other messages found.
                    iWapPushUtils->DeleteEntryL( matchingSiMsgEntryId );
                    // After the 'for' only one SI is allowed that has created
                    // attribute.
                    }
                else
                    {
                    foundOneToBeReplaced = ETrue; // A match was found.
                    // Check if received SI is newer than existing stored Si
                    // (out of order).
                    if ( iCreatedTime > existingSiCreatedTime )
	                    {
                        PUSHLOG_WRITE(" Replacing...")
                        // The new SI replaces the existing.
                        aMatchingEntryId = matchingSiMsgEntryId;
                        discardPushMsg = EFalse;
	                    }
                    else if ( iCreatedTime <= existingSiCreatedTime )
                        {
                        PUSHLOG_WRITE(" Discarding...")
                        // Received SI is older than existing stored Si.
                        discardPushMsg = ETrue;
                        }
                    }
                }
			}

		CleanupStack::PopAndDestroy( siEntry ); // siEntry
		}

	CleanupStack::PopAndDestroy( matchingIdList ); // matchingIdList

    PUSHLOG_LEAVEFN("CSIContentHandler::HandleMsgOrderReceptionL")
    return discardPushMsg;
	}

// ---------------------------------------------------------
// CSIContentHandler::ConvertDateTimeL
// ---------------------------------------------------------
//
TBool CSIContentHandler::ConvertDateTimeL( const TDesC& aDateTime,
                                           TTime& aConvertedDate ) const
	{
    PUSHLOG_ENTERFN("CSIContentHandler::ConvertDateTimeL")

	TTime convertedTime = Time::NullTTime();
	TBool convertedOK = EFalse;

    // check supplied descriptor is the correct length
	if ( aDateTime.Length() != KValidUTCLength )
        {
        PUSHLOG_WRITE_FORMAT(" invalid UTC length <%d>",aDateTime.Length())
        User::Leave( KErrCorrupt );
        }
    else
		{
		TBuf<KValidUTCLength> str = aDateTime;
        PUSHLOG_WRITE_FORMAT(" datetime str: <%S>",&str)
		if ( !IsValidUTCTime( str ) )
            {
            // The UTC time is invalid.
            PUSHLOG_WRITE(" invalid UTC time")
            User::Leave( KErrCorrupt );
            }
        else
			{
            // Now 'str' is in format YYYYMMDD:HHMMSS
			// Adjust UTC time to zero offset TTime. Only month and day
            // is effected.
			const TInt KFirstMonthChar = KValidTTimeMonthStart;
			const TInt KSecondMonthChar = KFirstMonthChar + 1;
			const TInt KFirstDayChar = KValidTTimeDayStart;
			const TInt KSecondDayChar = KFirstDayChar + 1;
            // Month.
			// check for special case of month = 10 which becomes 09
			if ( str[KFirstMonthChar] == '1' && str[KSecondMonthChar] == '0' )
				{
				str[KFirstMonthChar] = '0';
				str[KSecondMonthChar] = '9';
				}
			else
                {
				// month value is either 11, 12 or less than 10, ie 1 - 9.
				// reduce day by one, eg 11 becomes 10, 12 becomes 11, 09 becomes 08
				str[KSecondMonthChar]--;
                }

            // Day.
			// check for special cases 10, 20, 30
			if ( str[KSecondDayChar] == '0' )
				{
				// reduce day by 1, ie 10 becomes 09, 20 becomes 19 ...
				str[KSecondDayChar] = '9';
				str[KFirstDayChar]--;
				}
			else
                {
				// day value is between 1 and 9 so reduce day by one
				// eg 29 becomes 28, 11 bcomes 10, 31 becomes 30
				str[KSecondDayChar]--;
                }

			// string is now syntaxically correct, but Set() will return an
            // error if it's semantically incorrect.
            User::LeaveIfError( convertedTime.Set( str ) );
			convertedOK = ETrue;
			}
		}

    PUSHLOG_LEAVEFN("CSIContentHandler::ConvertDateTimeL")
	aConvertedDate = convertedTime;
	return convertedOK;
	}

// ---------------------------------------------------------
// CSIContentHandler::ConvertOpaqueToUtcL
// ---------------------------------------------------------
//
HBufC* CSIContentHandler::ConvertOpaqueToUtcL( const TDesC8& aOpaque ) const
	{
    PUSHLOG_ENTERFN("CSIContentHandler::ConvertOpaqueToUtcL")

    const TInt opaqueSize = aOpaque.Size();
    if ( KValidMaxEncodedDateTimeSize < opaqueSize )
        {
        PUSHLOG_WRITE_FORMAT(" Bad OPAQUE size: <%d>",opaqueSize)
        User::Leave( KErrCorrupt );
        }

    HBufC* converted = HBufC::NewMaxLC( KValidUTCLength );
    TPtr convertedPtr = converted->Des();
    convertedPtr.SetLength( 0 ); // Reset.

    // Split up each opaque byte to two bytes.
    TUint8 byte( 0x00 );
    TUint8 high( 0x00 );
    TUint8 low( 0x00 );
    TInt i = 0;
    for ( i = 0; i < opaqueSize; ++i )
        {
        byte = aOpaque[i];
        high = (TUint8)( (byte & 0xF0) >> 4 );
        low  = (TUint8)(  byte & 0x0F );
        // Check high and low if they are in the range [0-9].
        if ( 9 < high || 9 < low )
            {
            PUSHLOG_WRITE_FORMAT2(" Overflow: <%d, %d>",high,low)
            User::Leave( KErrOverflow );
            }
        convertedPtr.Append( TChar(KAsciiZeroCharCode + high) );
        convertedPtr.Append( TChar(KAsciiZeroCharCode + low) );
        }

    // A valid UTC %Datetime contains 14 numerical characters and 6
    // non-numerical: “1999-04-30T06:40:00Z”.
    // So fill the remaining bytes with zeros.
    for ( i = convertedPtr.Length(); i < KValidUTCNumericals; ++i )
        {
        convertedPtr.Append( TChar('0') );
        }

    // Insert the necessary non-numerical boundary characters.
    i = 0;
    // Skip year and insert '-'. (Don't forget to increase i with 1 each time!)
    i += KValidUTCYearBlockLength;
    convertedPtr.Insert( i++, KCharMinus );
    // Skip month and insert '-'.
    i += KValidUTCOtherBlockLength;
    convertedPtr.Insert( i++, KCharMinus );
    // Skip day and insert 'T'.
    i += KValidUTCOtherBlockLength;
    convertedPtr.Insert( i++, KCharT );
    // Skip hour and insert ':'.
    i += KValidUTCOtherBlockLength;
    convertedPtr.Insert( i++, KCharColon );
    // Skip minute and insert ':'.
    i += KValidUTCOtherBlockLength;
    convertedPtr.Insert( i++, KCharColon );
    // Skip second and insert 'Z'.
    i += KValidUTCOtherBlockLength;
    convertedPtr.Insert( i++, KCharZ );

    CleanupStack::Pop( converted ); // converted
    PUSHLOG_LEAVEFN("CSIContentHandler::ConvertOpaqueToUtcL")
	return converted;
	}

// ---------------------------------------------------------
// CSIContentHandler::IsValidUTCTime
// ---------------------------------------------------------
//
TBool CSIContentHandler::IsValidUTCTime( TDes& aDateTime ) const
	{
    PUSHLOG_ENTERFN("CSIContentHandler::IsValidUTCTime")

    TBool isValid( ETrue ); // Return value.

    // Now aDateTime has to be in format YYYY-MM-DDTHH:MM:SSZ

    // check supplied descriptor is the correct length
	if ( aDateTime.Length() != KValidUTCLength )
        {
        PUSHLOG_WRITE_FORMAT(" invalid UTC length <%d>",aDateTime.Length())
        isValid = EFalse;
        }
    else
        {
	    // strip out formatting characters
	    TInt formatCharPos = 4;
	    aDateTime.Delete( formatCharPos, 1 );
	    // now move through two characters at a time and remove other chars
	    // to just leave digits
	    const TInt KRemainingFormatChars = 5;
        TInt i( 0 );
	    for ( i = 0; i < KRemainingFormatChars; ++i )
		    {
		    formatCharPos += 2;
		    aDateTime.Delete( formatCharPos, 1 );
		    }

        // Now aDateTime has to be in format YYYYMMDDHHMMSS

        __ASSERT_DEBUG( aDateTime.Length() == KValidTTimeLength,
                        ContHandPanic( EPushContHandPanBadTTimeLength ) );

        // now have UTC string stripped of format characters - check remaining
        // characters are all digits - YYYYMMDDHHMMSS
        TChar ch;
        for ( i = 0; i < KValidTTimeLength; ++i )
		    {
		    ch = aDateTime[i];
		    if ( ch.IsDigit() == EFalse )
                {
                PUSHLOG_WRITE_FORMAT(" not digit <%d>",i)
                isValid = EFalse;
                }
		    }

        if ( isValid )
            {
            /*
            In YYYYMMDDHHMMSS
            YYYY = 4 digit year ("0000" ... "9999")
            MM = 2 digit month ("01"=January, "02"=February ... "12"=December)
            DD = 2 digit day ("01", "02" ... "31")
            HH = 2 digit hour, 24-hour timekeeping system ("00" ... "23")
            MM = 2 digit minute ("00" ... "59")
            SS = 2 digit second ("00" ... "59")
            */
            TInt err;
            TUint val;
            // Do not check year. There are no restrictions.
            // Check month.
            TLex parser( aDateTime.Mid( KValidTTimeMonthStart,
                                        KValidTTimeBlockLength ) );
            err = parser.Val( val, EDecimal );
            if ( err )
                {
                isValid = EFalse;
                PUSHLOG_WRITE_FORMAT(" parser err: <%d>",err)
                }
            else
                {
                PUSHLOG_WRITE_FORMAT(" month: <%d>",val)
                if ( val < 1 || 12 < val )
                    {
                    isValid = EFalse;
                    }
                }
            // Check day.
            if ( isValid )
                {
                parser = aDateTime.Mid( KValidTTimeDayStart,
                                        KValidTTimeBlockLength );
                err = parser.Val( val, EDecimal );
                if ( err )
                    {
                    isValid = EFalse;
                    PUSHLOG_WRITE_FORMAT(" parser err: <%d>",err)
                    }
                else
                    {
                    PUSHLOG_WRITE_FORMAT(" day: <%d>",val)
                    if ( val < 1 || 31 < val )
                        {
                        isValid = EFalse;
                        }
                    }
                }
            // Check hour.
            if ( isValid )
                {
                parser = aDateTime.Mid( KValidTTimeHourStart,
                                        KValidTTimeBlockLength );
                err = parser.Val( val, EDecimal );
                if ( err )
                    {
                    isValid = EFalse;
                    PUSHLOG_WRITE_FORMAT(" parser err: <%d>",err)
                    }
                else
                    {
                    PUSHLOG_WRITE_FORMAT(" hour: <%d>",val)
                    if ( 23 < val )
                        {
                        isValid = EFalse;
                        }
                    }
                }
            // Check minute.
            if ( isValid )
                {
                parser = aDateTime.Mid( KValidTTimeMinuteStart,
                                        KValidTTimeBlockLength );
                err = parser.Val( val, EDecimal );
                if ( err )
                    {
                    isValid = EFalse;
                    PUSHLOG_WRITE_FORMAT(" parser err: <%d>",err)
                    }
                else
                    {
                    PUSHLOG_WRITE_FORMAT(" min: <%d>",val)
                    if ( 59 < val )
                        {
                        isValid = EFalse;
                        }
                    }
                }
            // Check second.
            if ( isValid )
                {
                parser = aDateTime.Mid( KValidTTimeSecondStart,
                                        KValidTTimeBlockLength );
                err = parser.Val( val, EDecimal );
                if ( err )
                    {
                    isValid = EFalse;
                    PUSHLOG_WRITE_FORMAT(" parser err: <%d>",err)
                    }
                else
                    {
                    PUSHLOG_WRITE_FORMAT(" sec: <%d>",val)
                    if ( 59 < val )
                        {
                        isValid = EFalse;
                        }
                    }
                }

	        // insert colon seperating date from time
	        const TInt KColonPosition = 8;
	        aDateTime.Insert( KColonPosition, KCharColon );

            // Now aDateTime has to be in format YYYYMMDD:HHMMSS
            }
        }

    PUSHLOG_LEAVEFN("CSIContentHandler::IsValidUTCTime")
	return isValid; // aDateTime contains a modified buffer.
	}

// ---------------------------------------------------------
// CSIContentHandler::AttributeToTTimeL
// ---------------------------------------------------------
//
TBool CSIContentHandler::AttributeToTTimeL
                        ( NW_DOM_AttributeHandle_t& aAttrHandle,
                          TTime& aConvertedDate ) const
    {
    PUSHLOG_ENTERFN("CSIContentHandler::AttributeToTTimeL")

    TBool gotDate = EFalse;
    NW_Status_t stat = NW_STAT_SUCCESS;
    NW_DOM_AttrVal_t attrVal;

    // It is expected to be String or Opaque.
    // It may be Opaque, to which we will need a NW_DOM_AttrVal_t structure.
    stat = NW_DOM_AttributeHandle_getNextVal( &aAttrHandle, &attrVal );

    if ( stat != NW_STAT_WBXML_ITERATE_MORE )
        {
        User::LeaveIfError( NwxStatusToErrCode( stat ) );
        }
    else
        {
        NW_Uint16 valType = NW_DOM_AttrVal_getType( &attrVal );

        if ( valType == NW_DOM_ATTR_VAL_STRING )
            {
            CStringOwner* stringOwner = new (ELeave) CStringOwner;
            CleanupStack::PushL( stringOwner );

            NW_String_t* val = NW_String_new();
            User::LeaveIfNull( val );
            stringOwner->SetString( val );
            //
            stat = NW_DOM_AttrVal_toString( &attrVal, val, iCharEncoding );
            User::LeaveIfError( NwxStatusToErrCode( stat ) );
            NW_Byte* storage = NW_String_getStorage( val );
            NW_Uint16 length = NW_String_getCharCount( val, iCharEncoding );
            TPtrC8 dataPtr( storage, length );
            HBufC* dataBuf = HBufC::NewMaxLC( dataPtr.Length() );
            dataBuf->Des().Copy( dataPtr );
            gotDate = ConvertDateTimeL( *dataBuf, aConvertedDate );

            CleanupStack::PopAndDestroy( 2, stringOwner ); // dataBuf,
                                                           // stringOwner
            }
        else if ( valType == NW_DOM_ATTR_VAL_OPAQUE )
            {
            NW_Uint32 len = 0;
            NW_Byte* data = NW_DOM_AttrVal_getOpaque( &attrVal, &len );
            User::LeaveIfNull( data );
            TPtrC8 dataPtr( data, len );

            HBufC* dateTime = ConvertOpaqueToUtcL( dataPtr );
            CleanupStack::PushL( dateTime );
            gotDate = ConvertDateTimeL( *dateTime, aConvertedDate );
            CleanupStack::PopAndDestroy( dateTime ); // dateTime
            }
        else
            {
            User::Leave( KErrNotSupported );
            }
        }

    PUSHLOG_LEAVEFN("CSIContentHandler::AttributeToTTimeL")
    return gotDate; // aConvertedDate contains the result.
    }

// ---------------------------------------------------------
// CSIContentHandler::HandleMessageL
// ---------------------------------------------------------
//
void CSIContentHandler::HandleMessageL( CPushMessage* aPushMsg,
                                        TRequestStatus& aStatus )
	{
    PUSHLOG_ENTERFN("CSIContentHandler::HandleMessageL")

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

    PUSHLOG_LEAVEFN("CSIContentHandler::HandleMessageL")
	}

// ---------------------------------------------------------
// CSIContentHandler::HandleMessageL
// ---------------------------------------------------------
//
void CSIContentHandler::HandleMessageL( CPushMessage* aPushMsg )
	{
    PUSHLOG_ENTERFN("CSIContentHandler::HandleMessageL")

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

    PUSHLOG_LEAVEFN("CSIContentHandler::HandleMessageL")
	}

// ---------------------------------------------------------
// CSIContentHandler::CancelHandleMessage
// ---------------------------------------------------------
//
void CSIContentHandler::CancelHandleMessage()
	{
    PUSHLOG_ENTERFN("CSIContentHandler::CancelHandleMessage")
    Cancel();
    PUSHLOG_LEAVEFN("CSIContentHandler::CancelHandleMessage")
	}

// ---------------------------------------------------------
// CSIContentHandler::CPushHandlerBase_Reserved1
// ---------------------------------------------------------
//
void CSIContentHandler::CPushHandlerBase_Reserved1()
    {
    }

// ---------------------------------------------------------
// CSIContentHandler::CPushHandlerBase_Reserved2
// ---------------------------------------------------------
//
void CSIContentHandler::CPushHandlerBase_Reserved2()
    {
    }

// ---------------------------------------------------------
// CSIContentHandler::DoCancel
// ---------------------------------------------------------
//
void CSIContentHandler::DoCancel()
	{
    PUSHLOG_ENTERFN("CSIContentHandler::DoCancel")
	Complete( KErrCancel );
    PUSHLOG_LEAVEFN("CSIContentHandler::DoCancel")
	}

// ---------------------------------------------------------
// CSIContentHandler::RunL
// ---------------------------------------------------------
//
void CSIContentHandler::RunL()
	{
    // Handle errors in RunError().
    PUSHLOG_WRITE_FORMAT("iStatus.Int(): %d",iStatus.Int())
    User::LeaveIfError( iStatus.Int() );

	// use active state machine routine to manage activites:
	switch ( iState )
		{
	    case EGarbageCollecting:
            {
		    CollectGarbageL();
		    break;
            }
	    case EFilteringAndParsing:
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
				PUSHLOG_WRITE("CSIContentHandler::RunL : before trapping parsing.")
				TRAP(ret, ParsePushMsgL());
				PUSHLOG_WRITE_FORMAT("CSIContentHandler::RunL : after trapping parsing. ret = %d", ret)
				if ( ret != KErrNone)
					{
					PUSHLOG_WRITE("CSIContentHandler::RunL : Parsing failed. discarding message.")
					iState = EDone;
					IdleComplete();
					}
                }
		    break;
            }
	    case EProcessing:
            {
		    ProcessingPushMsgEntryL();
		    break;
            }
	    case EDone:
            {
            PUSHLOG_WRITE("CSIContentHandler EDone")
		    Complete( KErrNone );
		    break;
            }
	    default:
            {
            // JIC.
            PUSHLOG_WRITE("CSIContentHandler default Done")
		    Complete( KErrNone );
		    break;
            }
		}
	}

// ---------------------------------------------------------
// CSIContentHandler::RunError
// ---------------------------------------------------------
//
TInt CSIContentHandler::RunError( TInt aError )
	{
    PUSHLOG_WRITE_FORMAT("CSIContentHandler::RunError: %d",aError)

	iState = EDone;
	Complete( aError );
	return KErrNone;
	}

