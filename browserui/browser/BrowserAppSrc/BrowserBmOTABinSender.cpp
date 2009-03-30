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
* Description: 
*
*
*/


// INCLUDE FILES
        // System includes
#include <eikdef.h>
#include <eikenv.h>
#include <sendui.h>
#include <CMessageData.h>
#include <TSendingCapabilities.h>
#include <SenduiMtmUids.h>
#include <msgbiouids.h>
#include <txtrich.h>
#include <smut.h>

	// User includes
#include "BrowserAppUi.h"
#include "BrowserUtil.h"
#include "BrowserDialogs.h"
#include "BrowserBmOTABinSender.h"


// MACROS

#ifndef _BOOKMARK_SENT_ASCII
// Calculates the byte size of 16bits (a)
#define TUINT16_LEN(a)  ( sizeof(a) / sizeof(TUint16) )
#endif

// ================= LOCAL CONSTANTS =======================

/// Granularity of the bookmark list.
LOCAL_C const TInt KGranularity = 1;

/// Attachment file name.
_LIT( KAttachmentFilename, "c:\\system\\temp\\Bookmark.txt");

#ifndef  _BOOKMARK_SENT_ASCII
// Binary encoded BM data. See OTA spec
static const TUint16 Kch_lstart[] = { 0x00, 0x01, 0x00, 0x45 };
static const TUint16 Kch_lend[] = { 0x01 };
static const TUint16 Kch_start[] = { 0xC6,0x7F,0x01,0x87,0x15,0x11,0x03 };
static const TUint16 Kch_mid[] = { 0x00,0x01,0x87,0x17,0x11,0x03 };
static const TUint16 Kch_end[] = { 0x00,0x01,0x01 };
#endif  // _BOOKMARK_SENT_ASCII


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------------------------
// CBookmarkOTAItem::CBookmarkOTAItem
// ---------------------------------------------------------------------------
//
CBookmarkOTAItem::CBookmarkOTAItem()
    {
    }

// ---------------------------------------------------------------------------
// CBookmarkOTAItem::~CBookmarkOTAItem
// ---------------------------------------------------------------------------
//
CBookmarkOTAItem::~CBookmarkOTAItem()
    {
    delete iBookmark;
    }

// ---------------------------------------------------------------------------
// CBookmarkOTAItem::NewL
// ---------------------------------------------------------------------------
//
CBookmarkOTAItem* CBookmarkOTAItem::NewL( const TDesC& aUrl, const TDesC& aTitle )
    {
    CBookmarkOTAItem *item = new (ELeave) CBookmarkOTAItem();
    CleanupStack::PushL( item );

    item->ConstructL( aUrl, aTitle );

    CleanupStack::Pop(); // item

    return item;
    }

// ---------------------------------------------------------------------------
// CBookmarkOTAItem::ConstructL
// ---------------------------------------------------------------------------
//
void CBookmarkOTAItem::ConstructL( const TDesC& aUrl, 
#ifdef  _BOOKMARK_SENT_ASCII
                                   const TDesC& /*aTitle*/ )
#else   // _BOOKMARK_SENT_ASCII
                                   const TDesC& aTitle )
#endif  // _BOOKMARK_SENT_ASCII

    {
#ifdef  _BOOKMARK_SENT_ASCII
    iBookmark = aUrl.AllocL();
#else   // _BOOKMARK_SENT_ASCII
    TInt    length  = TUINT16_LEN( Kch_start ) + 
                      TUINT16_LEN( Kch_mid ) + 
                      TUINT16_LEN( Kch_end );

    // calculate full length of BOOKMARK
    length += aTitle.Length() + aUrl.Length();
    
    iBookmark = HBufC::NewL( length );

    TPtrC  start( Kch_start, TUINT16_LEN(Kch_start) );
    TPtrC  mid( Kch_mid, TUINT16_LEN(Kch_mid) );
    TPtrC  end( Kch_end, TUINT16_LEN(Kch_end) );

    iBookmark->Des().Append( start );
    iBookmark->Des().Append( aTitle );
    iBookmark->Des().Append( mid );
    iBookmark->Des().Append( aUrl );
    iBookmark->Des().Append( end );

#endif  // _BOOKMARK_SENT_ASCII
    }

// ---------------------------------------------------------------------------
// CWmlBrowserBmOTABinSender::NewL
// ---------------------------------------------------------------------------
//
CWmlBrowserBmOTABinSender* CWmlBrowserBmOTABinSender::NewL()
    {
    CWmlBrowserBmOTABinSender* sender = new (ELeave) 
                                            CWmlBrowserBmOTABinSender();
    CleanupStack::PushL( sender );

    sender->ConstructL();

    CleanupStack::Pop();    // sender

    return sender;
    }

// ---------------------------------------------------------------------------
// CWmlBrowserBmOTABinSender::~CWmlBrowserBmOTABinSender
// ---------------------------------------------------------------------------
//
CWmlBrowserBmOTABinSender::~CWmlBrowserBmOTABinSender()
    {
    ResetAndDestroy();
    delete iSendUi;
    }

// ---------------------------------------------------------------------------
// CWmlBrowserBmOTABinSender::ResetAndDestroy
// ---------------------------------------------------------------------------
//
void CWmlBrowserBmOTABinSender::ResetAndDestroy()
    {
    iBookmarks.ResetAndDestroy();
    }

// ---------------------------------------------------------------------------
// CWmlBrowserBmOTABinSender::AppendL
// ---------------------------------------------------------------------------
//
void CWmlBrowserBmOTABinSender::AppendL( const TText *aUrl, 
                                         const TText *aTitle )
    {
	__ASSERT_DEBUG( (aUrl != NULL), Util::Panic( Util::EUninitializedData ) );
    TPtrC tempUrl( aUrl );
    CBookmarkOTAItem *item = NULL;

    if( !aTitle )
        {
        _LIT( KEmptyText, "" );

        item = CBookmarkOTAItem::NewL( tempUrl, (TDesC&)KEmptyText );
        }
    else
        {
        TPtrC tempTitle( aTitle );

        item = CBookmarkOTAItem::NewL( tempUrl, tempTitle );
        }

    CleanupStack::PushL( item );

    iBookmarks.AppendL( item );

    CleanupStack::Pop(); // item
    }

// ---------------------------------------------------------------------------
// CWmlBrowserBmOTABinSender::AppendL
// ---------------------------------------------------------------------------
//
void CWmlBrowserBmOTABinSender::AppendL( const TDesC& aUrl, const TDesC& aTitle )
    {
    CBookmarkOTAItem *item = CBookmarkOTAItem::NewL( aUrl, aTitle );
    CleanupStack::PushL( item );

    iBookmarks.AppendL( item );

    CleanupStack::Pop(); // item
    }

// ---------------------------------------------------------------------------
// CWmlBrowserBmOTABinSender::SendAddressL
// ---------------------------------------------------------------------------
//
void CWmlBrowserBmOTABinSender::SendAddressL( )
    {
    CMessageData* msgData = CMessageData::NewLC();//Cl St: 1
    __ASSERT_DEBUG( iBookmarks.Length(), 
                     Util::Panic( Util::EUninitializedData ) );

    // sending error indicator
    // used with CreateAndSendMessageL()
    // to free up allocated memory
    // and than leave at the and of 
    // this function
    TInt error = KErrNone;

    // calculate full length of OTA message
    TInt    length = 0;

    TSendingCapabilities capa;
    CArrayFixFlat<TUid>* servicesToDim = new(ELeave) CArrayFixFlat<TUid>( 2 );
    CleanupStack::PushL( servicesToDim );
    servicesToDim->AppendL( KSenduiMtmAudioMessageUid );
	// service: audio message to dim 
	servicesToDim->AppendL( KSenduiMtmPostcardUid );    
	// service: postcard to dim						  
    TUid service = iSendUi->ShowSendQueryL(NULL, capa, servicesToDim, KNullDesC);
	iSendUi->ServiceCapabilitiesL( service, capa );
	
	CleanupStack::PopAndDestroy(); // destroy serviceToDim 

#ifndef _BOOKMARK_SENT_ASCII
    length += TUINT16_LEN( Kch_lstart ) + TUINT16_LEN( Kch_lend );
#endif  // _BOOKMARK_SENT_ASCII

    for( TInt index = 0; index < iBookmarks.Count(); ++index )
        {
        length += iBookmarks[index]->iBookmark->Length();
        }

#ifdef _BOOKMARK_SENT_ASCII
    // +1 '\n' per bookmarks
    // no need '\n' after the last one
    length += iBookmarks.Count() - 1;
#endif  // _BOOKMARK_SENT_ASCII

    // body contains the binary sequence
    HBufC* body = HBufC::NewLC( length );

#ifndef _BOOKMARK_SENT_ASCII
    TPtrC  lstart( Kch_lstart, TUINT16_LEN( Kch_lstart ) );
    TPtrC  lend( Kch_lend, TUINT16_LEN( Kch_lend ) );
    // body starts with CHARACTERISTIC-LIST
    body->Des().Append( lstart );
#endif  // _BOOKMARK_SENT_ASCII

    for( TInt index2 = 0; index2 < iBookmarks.Count(); ++index2 )
        {
        // Append CHARACTERISTIC to CHARACTERISTIC-LIST
        body->Des().Append( iBookmarks[index2]->iBookmark->Des().Ptr(), 
                            iBookmarks[index2]->iBookmark->Length() );

#ifdef _BOOKMARK_SENT_ASCII
        // append '\n' after ever bookmark except the last one
        if( index2 < iBookmarks.Count() - 1 )
            {
            _LIT( KNewLine, "\n" );
            body->Des().Append( KNewLine );
            }
#endif  // _BOOKMARK_SENT_ASCII
        }

#ifndef _BOOKMARK_SENT_ASCII
    // CHARACTERISTIC-LIST termination
    body->Des().Append( lend );
#endif  // _BOOKMARK_SENT_ASCII

	if ( capa.iFlags & TSendingCapabilities::ESupportsAttachments )
	  	{
	    // connect to file session
	    RFs fsSession;
	    User::LeaveIfError( fsSession.Connect() );
	    CleanupClosePushL<RFs>( fsSession );
	    CFileMan* fman = CFileMan::NewL( fsSession );
	    CleanupStack::PushL( fman );
	    // try to avoid inserting any 'leave' code 
	    // between WriteMessageBodyIntoFile()
	    // and CreateAndSendMessageL, or
	    // you have to take care of deleting 
	    // temporary file
	    error = WriteMessageBodyIntoFileL( fsSession, *body );
	        
	    if( error == KErrNone )
	        {
	        // send attachment
	        // this function has to be trapped
	        // because I have to be sure that the 
	        // temproray file is removed!
	        msgData->AppendAttachmentL( KAttachmentFilename );            
	        TRAP( error, iSendUi->CreateAndSendMessageL( service, 
	                                                     msgData ) );
	        }
	    // delete temporary file
	    fman->Delete( KAttachmentFilename );
	    // close file session        
	    fsSession.Close();

	    CleanupStack::PopAndDestroy( 2 ); // fsSession, fman
	  	}
	else if ( capa.iFlags & TSendingCapabilities::ESupportsBodyText )
		{
		CRichText* text = 
		        CRichText::NewL(CEikonEnv::Static()->SystemParaFormatLayerL(),
		                        CEikonEnv::Static()->SystemCharFormatLayerL());
		CleanupStack::PushL(text);
		text->InsertL(0, *body);        
		msgData->SetBodyTextL( text );
		CleanupStack::PopAndDestroy(text );
	
#ifdef _BOOKMARK_SENT_ASCII
		TRAP( error, iSendUi->CreateAndSendMessageL( service, msgData ) );
#else
    	TRAP( error, iSendUi->CreateAndSendMessageL( service, 
                                                 	 msgData,
                                                 	 KMsgBioUidWmlBrSettings ) );
#endif  // _BOOKMARK_SENT_ASCII
		}

    ResetAndDestroy();

    CleanupStack::PopAndDestroy( 2 ); // body, msgData

    // if any error occured during sending process
    User::LeaveIfError( error );
    }



// ---------------------------------------------------------------------------
// CWmlBrowserBmOTABinSender::SendOPMLFileL
// ---------------------------------------------------------------------------
//
void CWmlBrowserBmOTABinSender::SendOPMLFileL(const TDesC& aOPMLFileName)
    {
    CMessageData* msgData = CMessageData::NewLC();

    // sending error indicator
    // used with CreateAndSendMessageL()
    // to free up allocated memory
    // and than leave at the and of 
    // this function
    TInt error = KErrNone;

    TSendingCapabilities capa;
    
    CArrayFixFlat<TUid>* servicesToDim = new(ELeave) CArrayFixFlat<TUid>( 2 );
    CleanupStack::PushL( servicesToDim );
    servicesToDim->AppendL( KSenduiMtmAudioMessageUid );
	// service: audio message to dim 
	servicesToDim->AppendL( KSenduiMtmPostcardUid );    
	// service: postcard to dim						  
    TUid service = iSendUi->ShowSendQueryL(NULL, capa, servicesToDim, KNullDesC);
	iSendUi->ServiceCapabilitiesL( service, capa );
	
	CleanupStack::PopAndDestroy(); // destroy serviceToDim 

	// body contains the binary sequence
    //HBufC* body = HBufC::NewLC( aOPMLFileName.Length() );
    //body->Des().Copy( aOPMLFileName );
    
	if ( capa.iFlags & TSendingCapabilities::ESupportsAttachments )
	  	{
	    // connect to file session
	    RFs fsSession;
	    User::LeaveIfError( fsSession.Connect() );
	    CleanupClosePushL<RFs>( fsSession );
	    CFileMan* fman = CFileMan::NewL( fsSession );
	    CleanupStack::PushL( fman );
	    
	    // between WriteMessageBodyIntoFile()
	    // and CreateAndSendMessageL, or
	    // you have to take care of deleting 
	    // temporary file
	    //error = WriteMessageBodyIntoFileL( fsSession, *body );
	        
        // send attachment
        // this function has to be trapped
        // because I have to be sure that the 
        // temproray file is removed!
        msgData->AppendAttachmentL( aOPMLFileName );            
        TRAP( error, iSendUi->CreateAndSendMessageL( service, 
                                                     msgData ) );
	    // delete temporary file
	    fman->Delete( aOPMLFileName );
	    // close file session        
	    fsSession.Close();

	    CleanupStack::PopAndDestroy( 2 ); // fsSession, fman
	  	}

    CleanupStack::PopAndDestroy( 1 ); // msgData

    // if any error occured during sending process
    User::LeaveIfError( error );
    }
// ---------------------------------------------------------------------------
// CWmlBrowserBmOTABinSender::ConstructL
// ---------------------------------------------------------------------------
//
void CWmlBrowserBmOTABinSender::ConstructL()
    {
    iSendUi = CSendUi::NewL();
    }

// ---------------------------------------------------------------------------
// CWmlBrowserBmOTABinSender::CWmlBrowserBmOTABinSender
// ---------------------------------------------------------------------------
//
CWmlBrowserBmOTABinSender::CWmlBrowserBmOTABinSender()
: iBookmarks( KGranularity )
    {
    }

// ---------------------------------------------------------------------------
// CWmlBrowserBmOTABinSender::WriteMessageBodyIntoFileL
//
// This function leaves only if the file creating failed.
// Do not need to be trapped to delete file. 
// ---------------------------------------------------------------------------
//
TInt CWmlBrowserBmOTABinSender::WriteMessageBodyIntoFileL( RFs &fs, 
                                                          TDesC& aBody )
    {
    TInt result;
    RFile otaFile;
    TFileName filename( KAttachmentFilename );

    if( (result = otaFile.Replace( fs, filename, EFileShareExclusive)) == 
                                                                    KErrNone )
        {

#ifdef _BOOKMARK_SENT_ASCII
        HBufC8 *tempBody = HBufC8::NewL( aBody.Length() );
        // convert 16bits -> 8bits
        tempBody->Des().Copy( aBody );

        // write message into file
        result = otaFile.Write( *tempBody );
#else   // _BOOKMARK_SENT_ASCII
        // put 16bits binary data to 8bits (double byte -> two byte)
        TPtrC8  tempBody( (TUint8*)aBody.Ptr(), aBody.Length() * 2 );

        // write message into file
        result = otaFile.Write( tempBody );
#endif  // _BOOKMARK_SENT_ASCII

        otaFile.Close();
        }

    return result;
    }

// End of File

