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
* Description:  Implementation of CUnknownContentHandler.
*
*/



// INCLUDE FILES

#include "CUnknownContentHandler.h"
#include "PushMtmUtil.h"
#include "PushMtmLog.h"
#include "PushContentHandlerPanic.h"
#include "StringResourceReader.h"
#include <msvids.h>
#ifdef __SERIES60_PUSH_SP 
#include <CUnknownPushMsgEntry.h>
#include <BioDB.h>        // BIO Message Database and message query methods
#include <gsmubuf.h>    // CSmsBuffer class
#include <gsmumsg.h>    // CSmsMessage class
#include <smuthdr.h>    // CSmsHeader class
#include <smut.h>        // TSmsUtilities class
#include <txtrich.h>    // CRichText class
#include <biouids.h>    // KUidBioMessageTypeMtm const
#include <apgcli.h>
#include <apmrec.h>
#include <DocumentHandler.h>
#include <Uri16.h>
#include <UriUtils.h>
#include <msvuids.h>    // KUidMsvMessageEntry const
#include <PushMtmUi.rsg>

// SMUT Unbranch
#include <csmsgetdetdescinterface.h>

#endif // __SERIES60_PUSH_SP 

// CONSTANTS


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CUnknownContentHandler::NewL
// ---------------------------------------------------------
//
CUnknownContentHandler* CUnknownContentHandler::NewL()
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::NewL")

    CUnknownContentHandler* self = new (ELeave) CUnknownContentHandler; 
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    PUSHLOG_LEAVEFN("CUnknownContentHandler::NewL")
    return self;
    }

// ---------------------------------------------------------
// CUnknownContentHandler::~CUnknownContentHandler
// ---------------------------------------------------------
//
CUnknownContentHandler::~CUnknownContentHandler()
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::~CUnknownContentHandler")

    Cancel();

    PUSHLOG_LEAVEFN("CUnknownContentHandler::~CUnknownContentHandler")
    }

// ---------------------------------------------------------
// CUnknownContentHandler::CUnknownContentHandler
// ---------------------------------------------------------
//
CUnknownContentHandler::CUnknownContentHandler()
:   CPushContentHandlerBase(), 
    iSavedMsgId( KMsvNullIndexEntryId )
    {
    }

// ---------------------------------------------------------
// CUnknownContentHandler::ConstructL
// ---------------------------------------------------------
//
void CUnknownContentHandler::ConstructL()
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::ConstructL")

    CPushContentHandlerBase::ConstructL();
    // Added to Active Scheduler.

    PUSHLOG_LEAVEFN("CUnknownContentHandler::ConstructL")
    }


// ---------------------------------------------------------
// CUnknownContentHandler::CollectGarbageL
// ---------------------------------------------------------
//
void CUnknownContentHandler::CollectGarbageL()
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::CollectGarbageL")

    DoCollectGarbageL();

#ifdef __SERIES60_PUSH_SP
    iState = EFilteringAndProcessing;
#else // __SERIES60_PUSH_SP
    // Do nothing - message is discarded.
    iState = EDone;
#endif // __SERIES60_PUSH_SP
    IdleComplete();

    PUSHLOG_LEAVEFN("CUnknownContentHandler::CollectGarbageL")
    }

#ifdef __SERIES60_PUSH_SP

// ---------------------------------------------------------
// CUnknownContentHandler::ProcessingPushMsgEntryL
// ---------------------------------------------------------
//
void CUnknownContentHandler::ProcessingPushMsgEntryL()
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::ProcessingPushMsgEntryL")

    const TBool bioMsg( BioMessageTypeL() );
    if ( bioMsg )
        {
        PUSHLOG_WRITE(" BIO message");
        // convert to bio format & save
        SaveBioMessageEntryL();
        }
    else
        {
        PUSHLOG_WRITE(" Not BIO message");

        // Check if the received content is supported.
        RApaLsSession apaLs;
        User::LeaveIfError( apaLs.Connect() );
        CleanupClosePushL<RApaLsSession>( apaLs );

        // Try to find out the data type.
        TPtrC contentTypePtr;
        iMessage->GetContentType( contentTypePtr );
        HBufC8* contentT8 = HBufC8::NewMaxLC( contentTypePtr.Length() );
        contentT8->Des().Copy( contentTypePtr );
        TDataRecognitionResult result;
        User::LeaveIfError( apaLs.RecognizeData( KNullDesC(), 
                                                 *contentT8, 
                                                 result ) );
        CleanupStack::PopAndDestroy( contentT8 ); // contentT8

        CDocumentHandler* docHandler = CDocumentHandler::NewLC( NULL );
        TBool supported = docHandler->CanOpenL( result.iDataType );

        CleanupStack::PopAndDestroy( 2, &apaLs ); // docHandler, apaLs

        // Save only supported msg.
        if ( supported )
            {
                // create unknown push message entry and save
                SaveUnknownPushMsgEntryL();
            }
        }

    iState = EDone;
    IdleComplete();

    PUSHLOG_LEAVEFN("CUnknownContentHandler::ProcessingPushMsgEntryL")
    }

// ---------------------------------------------------------
// CUnknownContentHandler::BioMessageTypeL
// ---------------------------------------------------------
//
TBool CUnknownContentHandler::BioMessageTypeL()
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::BioMessageTypeL")

    // need to create local RFs for BIO otherwise raises exception
    RFs localFS;
    CleanupClosePushL( localFS );
    User::LeaveIfError( localFS.Connect() );
    CBIODatabase* bioDB = CBIODatabase::NewLC( localFS );

    TPtrC contentTypePtr;
    iMessage->GetContentType( contentTypePtr );

    iBioMsgUID = KNullUid;
    TBool isBio = EFalse;
    // IsBioMessageL returns KErrNone if found or KErrNotFound if not found 
    if ( bioDB->IsBioMessageL( EBioMsgIdIana, 
                           contentTypePtr, 
                           NULL, 
                           iBioMsgUID ) == KErrNone )
        {
        isBio = ETrue;
        }

    CleanupStack::PopAndDestroy( 2 ); // bioDB, localFS

    PUSHLOG_LEAVEFN("CUnknownContentHandler::BioMessageTypeL")
    return isBio;
    }

// ---------------------------------------------------------
// CUnknownContentHandler::SaveBioMessageEntryL
// ---------------------------------------------------------
//
void CUnknownContentHandler::SaveBioMessageEntryL()
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::SaveBioMessageEntryL")

    // create sms message from CPushMessage and then save message
    RFs fileSession;
    User::LeaveIfError(fileSession.Connect());
    CleanupClosePushL(fileSession);
    CSmsBuffer* smsBuffer = CSmsBuffer::NewL();
    CleanupStack::PushL(smsBuffer);

    CSmsMessage* smsMessage = CSmsMessage::NewL( fileSession, 
                                                 CSmsPDU::ESmsDeliver, 
                                                 smsBuffer );
    CleanupStack::PushL( smsMessage );

    smsMessage->SmsPDU().SetAlphabet( TSmsDataCodingScheme::ESmsAlphabet8Bit );

    TPtrC8 fieldValue;

    if ( iMessage->GetBinaryHeaderField( EHttpFrom, fieldValue ) ||
     iMessage->GetBinaryHeaderField( EHttpXWapInitiatorURI, fieldValue ) ||
     iMessage->GetBinaryHeaderField( EHttpContentLocation, fieldValue ) )
        {
        // Convert 8 bit to 16 bit
        HBufC* tempAddr = HBufC::NewLC( fieldValue.Length() );
        tempAddr->Des().Copy( fieldValue );

        smsMessage->SmsPDU().SetToFromAddressL( *tempAddr );
        CleanupStack::PopAndDestroy( tempAddr );
        }

    if ( iMessage->GetMessageBody( fieldValue ) )
        {
        HBufC* tempBody = HBufC::NewLC( fieldValue.Length() );
        tempBody->Des().Copy( fieldValue );

        smsBuffer->InsertL( 0, *tempBody );
        CleanupStack::PopAndDestroy( tempBody );
        }

    CleanupStack::Pop( 2 ); //smsBuffer, smsMessage

    StoreMsgL( smsMessage ); // destroys CSmsMessage (contains smsBuffer)
    CleanupStack::PopAndDestroy( &fileSession );

    PUSHLOG_LEAVEFN("CUnknownContentHandler::SaveBioMessageEntryL")
    }

// ---------------------------------------------------------
// CUnknownContentHandler::StoreMsgL
// ---------------------------------------------------------
//
void CUnknownContentHandler::StoreMsgL( CSmsMessage* aSmsMsg )
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::StoreMsgL")

    CleanupStack::PushL( aSmsMsg );

    // Create a CSmsHeader based on this message. smsHdr takes ownership of aSmsMsg
    CSmsHeader* smsHdr = CSmsHeader::NewL( aSmsMsg );
    CleanupStack::Pop( aSmsMsg );
    CleanupStack::PushL( smsHdr );

    // get root entry
    CMsvEntry* msvEntry = iMsvSession->GetEntryL( KMsvRootIndexEntryId );
    CleanupStack::PushL( msvEntry );

    // create an invisible blank entry 
    TMsvEntry entry;
    entry.iType = KUidMsvMessageEntry;
    entry.SetVisible( EFalse );
    entry.SetInPreparation( ETrue );
    entry.SetReadOnly( EFalse );
    entry.SetUnread( ETrue );

    TMsvId serviceId = SetBioServiceIdL();
    TBuf<KSmsDescriptionLength> description;    

// SMUT Unbranch
    CSmsGetDetDescInterface* smsPlugin = CSmsGetDetDescInterface::NewL();
    CleanupStack::PushL( smsPlugin );

    if ( iBioMsgUID != KNullUid )
        {
        // sets entry with values passed in
        TSmsUtilities::PopulateMsgEntry( entry, 
                                         *aSmsMsg, 
                                         serviceId, 
                                         KUidBIOMessageTypeMtm );
        entry.iBioType = iBioMsgUID.iUid;

        // Look up and set the description
        TInt index;
        CBIODatabase* bioDB = CBIODatabase::NewLC( msvEntry->Session().FileSession() );
        bioDB->GetBioIndexWithMsgIDL( iBioMsgUID, index );
        description.Copy( bioDB->BifReader(index).Description() );
        entry.iDescription.Set( description );
        CleanupStack::PopAndDestroy(); // bioDB
        }
    else
        {
        // sets entry with values passed in
        TSmsUtilities::PopulateMsgEntry( entry, *aSmsMsg, serviceId );

        // Look up and set the description
// SMUT Unbranch
        smsPlugin->GetDescription( *aSmsMsg, description );
        entry.iDescription.Set( description );
        }
    
    // Set the details
    TBuf<KSmsDetailsLength> details;    
    TInt err = smsPlugin->GetDetails( msvEntry->Session().FileSession(), 
                                          *aSmsMsg, 
                                          details );
    if ( !err )
        {
        entry.iDetails.Set( details );
        }

// SMUT Unbranch
    CleanupStack::PopAndDestroy( smsPlugin );

    // Store entry in inbox
    msvEntry->SetEntryL( KMsvGlobalInBoxIndexEntryId );
    msvEntry->CreateL( entry );
    msvEntry->Session().CleanupEntryPushL( entry.Id() );
    msvEntry->SetEntryL( entry.Id() );

    // Save the message
    CMsvStore* store = msvEntry->EditStoreL();
    CleanupStack::PushL( store );

    // Save off the CSmsHdr
    smsHdr->StoreL( *store );

    // Save the body
    
    // Create and fill a CRichText object to save to store..
    CParaFormatLayer* paraFormatLayer = CParaFormatLayer::NewL();
    CleanupStack::PushL( paraFormatLayer );
    CCharFormatLayer* charFormatLayer = CCharFormatLayer::NewL();
    CleanupStack::PushL( charFormatLayer );
    CRichText* richText = CRichText::NewL( paraFormatLayer, charFormatLayer );
    CleanupStack::PushL( richText );
    HBufC* bufBody=HBufC::NewLC( aSmsMsg->Buffer().Length() );
    TPtr bufBodyPtr = bufBody->Des();
    aSmsMsg->Buffer().Extract( bufBodyPtr, 0, aSmsMsg->Buffer().Length() );
    richText->InsertL( 0, bufBodyPtr ); 
    store->StoreBodyTextL( *richText );
    CleanupStack::PopAndDestroy( 4 ); //bufBody, richText, charFormatLayer, paraFormatLayer
    
    store->CommitL();
    entry.SetReadOnly( ETrue );
    entry.SetVisible( ETrue );
    entry.SetInPreparation( EFalse );
    msvEntry->ChangeL( entry );

    CleanupStack::PopAndDestroy(); //store
    msvEntry->Session().CleanupEntryPop(); //entry
    CleanupStack::PopAndDestroy( 2 ); //msvEntry, smsHdr

    PUSHLOG_LEAVEFN("CUnknownContentHandler::StoreMsgL")
    }

// ---------------------------------------------------------
// CUnknownContentHandler::SetBioServiceIdL
// ---------------------------------------------------------
//
TMsvId CUnknownContentHandler::SetBioServiceIdL()
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::SetBioServiceIdL")

    CMsvEntry* msvEntry = iMsvSession->GetEntryL( KMsvRootIndexEntryId );

    CleanupStack::PushL( msvEntry );
    
    TMsvId serviceId = KMsvNullIndexEntryId; 
    TRAPD( err, TSmsUtilities::ServiceIdL( *msvEntry, 
                                           serviceId, 
                                           KUidBIOMessageTypeMtm ) );
    if ( err == KErrNotFound )
        {
        serviceId = KMsvLocalServiceIndexEntryId;
        err = KErrNone;
        }
    
    CleanupStack::PopAndDestroy(); //msvEntry
    User::LeaveIfError( err );

    PUSHLOG_LEAVEFN("CUnknownContentHandler::SetBioServiceIdL")
    return serviceId;
    }

// ---------------------------------------------------------
// CUnknownContentHandler::SaveUnknownPushMsgEntryL
// ---------------------------------------------------------
//
void CUnknownContentHandler::SaveUnknownPushMsgEntryL()
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::SaveUnknownPushMsgEntryL")

    CUnknownPushMsgEntry* unknownPushMsgEntry = 
                          CUnknownPushMsgEntry::NewL();
    CleanupStack::PushL( unknownPushMsgEntry );

    SetUnknownPushMsgEntryFieldsL( *unknownPushMsgEntry );

    iSavedMsgId = unknownPushMsgEntry->SaveL( *iMsvSession, 
                                              KMsvGlobalInBoxIndexEntryId );

    CleanupStack::PopAndDestroy( unknownPushMsgEntry ); // unknownPushMsgEntry

    PUSHLOG_LEAVEFN("CUnknownContentHandler::SaveUnknownPushMsgEntryL")
    }

// ---------------------------------------------------------
// CUnknownContentHandler::SetUnknownPushMsgEntryFieldsL
// ---------------------------------------------------------
//
void CUnknownContentHandler::SetUnknownPushMsgEntryFieldsL
                             ( CUnknownPushMsgEntry& aUnknownPushMsgEntry )
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::SetUnknownPushMsgEntryFieldsL")

    // Set all the relevant fields

    // Get server address.
    TPtrC8 srvAddress;
    if ( iMessage->GetServerAddress( srvAddress ) )
        {
        aUnknownPushMsgEntry.SetFromL( srvAddress );
        }

    TPtrC contentTypePtr;
    iMessage->GetContentType( contentTypePtr );
    aUnknownPushMsgEntry.SetContentTypeL( contentTypePtr );

    TPtrC8 msgBodyPtr;
    iMessage->GetMessageBody( msgBodyPtr );
    aUnknownPushMsgEntry.SetMessageDataL( msgBodyPtr );

    // First line in Inbox: TMsvEntry::iDetails.
    if ( srvAddress.Length() == 0 )
        {
        // Read from resource.
        HBufC* details = 
            iStrRscReader->AllocReadResourceLC( R_PUSHMISC_UNK_SENDER );
        aUnknownPushMsgEntry.SetMsgDetailsL( *details );
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
        aUnknownPushMsgEntry.SetMsgDetailsL( *convertedFrom );
        //
        CleanupStack::PopAndDestroy( 2, details ); // convertedFrom, details
        }

    // Second line in Inbox: TMsvEntry::iDescription.
    TPtrC8 contentUriPtr;
    iMessage->GetBinaryHeaderField( EHttpXWapContentURI, contentUriPtr );
    // Copy the 8-bit descriptor into a unicode one.
    HBufC* contentUri = HBufC::NewMaxLC( contentUriPtr.Length() );
    contentUri->Des().Copy( contentUriPtr );
    // Extract file name from X-wap-content-uri.
    HBufC* fileName = FileNameFromUriL( *contentUri );
    CleanupStack::PopAndDestroy( contentUri ); // contentUri
    CleanupStack::PushL( fileName );

    if ( fileName->Length() == 0 )
        {
        // TODO: Read from resource.
        //"Unknown content type" qtn_wap_push_msg_content_not_supported
        }
    else
        {
        aUnknownPushMsgEntry.SetMsgDescriptionL( *fileName );
        }

    CleanupStack::PopAndDestroy( fileName ); // fileName

    PUSHLOG_LEAVEFN("CUnknownContentHandler::SetUnknownPushMsgEntryFieldsL")
    }

// ---------------------------------------------------------
// CUnknownContentHandler::FileNameFromUriL
// ---------------------------------------------------------
//
HBufC* CUnknownContentHandler::FileNameFromUriL( const TDesC& aUri ) const
    {
    TUriParser pars;
    User::LeaveIfError( pars.Parse( aUri ) );

    HBufC* res = NULL;
    if ( pars.IsPresent( EUriPath ) )
        {
        TPtrC path = pars.Extract( EUriPath );
        // Extract the remaining text after the last slash character.
        // If the path contains a file name, then it has to contain 
        // a slash also, so if there is no slash then it's sure that there 
        // is no file name in it.
        TInt index = path.LocateReverse( TChar('/') );
        if ( index < 0 )
            {
            // No file name.
            }
        else if ( ++index < path.Length() ) // If not at the end...
            {
            // Last slash found. Extract the text after the slash.
            res = path.Mid( index ).AllocL();
            }
        }

    if ( !res )
        {
        res = KNullDesC().AllocL();
        }

    return res;
    }

#endif // __SERIES60_PUSH_SP


// ---------------------------------------------------------
// CUnknownContentHandler::HandleMessageL
// ---------------------------------------------------------
//
void CUnknownContentHandler::HandleMessageL( CPushMessage* aPushMsg, 
                                             TRequestStatus& aStatus )
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::HandleMessageL")

    __ASSERT_DEBUG( aPushMsg != NULL, 
                    ContHandPanic( EPushContHandPanMsgNull ) );

    iMessage = aPushMsg;
    iAcknowledge = ETrue;
    SetConfirmationStatus( aStatus );
    iState = EGarbageCollecting;
    IdleComplete();
    }

// ---------------------------------------------------------
// CUnknownContentHandler::HandleMessageL
// ---------------------------------------------------------
//
void CUnknownContentHandler::HandleMessageL( CPushMessage* aPushMsg )
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::HandleMessageL")

    __ASSERT_DEBUG( aPushMsg != NULL, 
                    ContHandPanic( EPushContHandPanMsgNull ) );

    iAcknowledge = EFalse;
    iMessage = aPushMsg;
    iState = EGarbageCollecting;
    IdleComplete();

    PUSHLOG_LEAVEFN("CUnknownContentHandler::HandleMessageL")
    }

// ---------------------------------------------------------
// CUnknownContentHandler::CancelHandleMessage
// ---------------------------------------------------------
//
void CUnknownContentHandler::CancelHandleMessage()
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::CancelHandleMessage")
    Cancel();
    PUSHLOG_LEAVEFN("CUnknownContentHandler::CancelHandleMessage")
    }

// ---------------------------------------------------------
// CUnknownContentHandler::CPushHandlerBase_Reserved1
// ---------------------------------------------------------
//
void CUnknownContentHandler::CPushHandlerBase_Reserved1()
    {
    }

// ---------------------------------------------------------
// CUnknownContentHandler::CPushHandlerBase_Reserved2
// ---------------------------------------------------------
//
void CUnknownContentHandler::CPushHandlerBase_Reserved2()
    {
    }

// ---------------------------------------------------------
// CUnknownContentHandler::DoCancel
// ---------------------------------------------------------
//
void CUnknownContentHandler::DoCancel()
    {
    PUSHLOG_ENTERFN("CUnknownContentHandler::DoCancel")
    Complete( KErrCancel );
    PUSHLOG_LEAVEFN("CUnknownContentHandler::DoCancel")
    }

// ---------------------------------------------------------
// CUnknownContentHandler::RunL
// ---------------------------------------------------------
//
void CUnknownContentHandler::RunL()
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

#ifdef __SERIES60_PUSH_SP

        case EFilteringAndProcessing:
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
                ProcessingPushMsgEntryL();
                }
            break;
            }
        
#endif // __SERIES60_PUSH_SP

        default:
            {
            PUSHLOG_WRITE("CUnknownContentHandler default EDone")
            Complete( KErrNone );
            break;
            }
        }
    }

// ---------------------------------------------------------
// CUnknownContentHandler::RunError
// ---------------------------------------------------------
//
TInt CUnknownContentHandler::RunError( TInt aError )
    {
    PUSHLOG_WRITE_FORMAT("CUnknownContentHandler::RunError: %d",aError)

    iState = EDone;
    Complete( aError );
    return KErrNone;
    }

