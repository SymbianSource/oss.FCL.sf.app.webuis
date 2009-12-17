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
* Description:  Implementation of CPushMtmFetchOperation.
*
*/



// INCLUDE FILES

#include "PushMtmFetchOperation.h"
#include "PushContentHandlerPanic.h"
#include "PushMtmLog.h"
#include <CSLPushMsgEntry.h>
#include <Uri8.h>
#include <SysUtil.h>
#include <cookiefilterinterface.h>
#include <uaproffilter_interface.h>
#include <CommDbConnPref.h>
#include <cdbcols.h>
#include <browseruisdkcrkeys.h>
#include "PushMtmPrivateCRKeys.h"
#include <centralrepository.h> 
#include <commdb.h>
#include <ApDataHandler.h>
#include <ApAccessPointItem.h>
#include <ApUtils.h>
#include <ApSelect.h>
#include <httpfilteracceptheaderinterface.h>
#include <httpfilteriopinterface.h>
#include <httpfiltercommonstringsext.h>
#include <deflatefilterinterface.h>
#include <FeatMgr.h>

// ================= MACROS =======================

#ifdef _DEBUG
    /// Guard "debug-only" argument name with this (expands to argument).
    #define DEBUG_ONLY( argName ) argName
#else /* not defined _DEBUG */
    /// Guard "debug-only" argument name with this (expands to nothing).
    #define DEBUG_ONLY( argName )
#endif /* def _DEBUG */

// ================= LOCAL CONSTANTS =======================

_LIT8( KHttpProtString, "HTTP/TCP" );
_LIT8( KUserAgent, "Push MTM" );

/// Base HTTP error code. See Extended Error Handling UI Specification!
LOCAL_C const TInt KPushMtmHttpErrorBase = -25000;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMtmFetchOperation::NewL
// ---------------------------------------------------------
//
CPushMtmFetchOperation* CPushMtmFetchOperation::NewL
                                    (
                                        const TDesC& aRequestedUrl, 
                                        TRequestStatus& aObserverRequestStatus 
                                    )
    {
    PUSHLOG_ENTERFN("CPushMtmFetchOperation::NewL")

    CPushMtmFetchOperation* op = 
        new (ELeave) CPushMtmFetchOperation( aObserverRequestStatus );
    CleanupStack::PushL( op );
    op->ConstructL( aRequestedUrl );
    CleanupStack::Pop( op ); // op

    PUSHLOG_LEAVEFN("CPushMtmFetchOperation::NewL")
    return op;
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::~CPushMtmFetchOperation
// ---------------------------------------------------------
//
CPushMtmFetchOperation::~CPushMtmFetchOperation()
    {
    Cancel();
    delete ( iCacheMgr );
    delete iCacheSupply;
    iHttpTrans.Close();
    iHttpSess.Close();
    iConn.Close();
    iSockServ.Close();
    delete iRequestedUrl;
    delete iSourceUri;
    delete iBody;
    FeatureManager::UnInitializeLib();
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::StartL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::StartL()
    {
    PUSHLOG_ENTERFN("CPushMtmFetchOperation::StartL")

    Cancel();

    iResult = KErrNone;
    iState = EInit;

	SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );

    PUSHLOG_LEAVEFN("CPushMtmFetchOperation::StartL")
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::CPushMtmFetchOperation
// ---------------------------------------------------------
//
CPushMtmFetchOperation::CPushMtmFetchOperation
                        ( TRequestStatus& aObserverRequestStatus ) 
:   CActive( EPriorityStandard ), 
    iObserver( aObserverRequestStatus ),
    iState( EInit ),
    iResult( KErrNone )
    {
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::ConstructL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::ConstructL( const TDesC& aRequestedUrl )
    {
    PUSHLOG_ENTERFN("CPushMtmFetchOperation::ConstructL")

    iRequestedUrl = HBufC8::NewMaxL( aRequestedUrl.Length() );
    iRequestedUrl->Des().Copy( aRequestedUrl );

    PUSHLOG_LEAVEFN("CPushMtmFetchOperation::ConstructL")
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::GetDefaultInetAccessPointL
// ---------------------------------------------------------
//
TBool CPushMtmFetchOperation::GetDefaultInetAccessPointL
                              ( TUint32& aIapId ) const
    {
    PUSHLOG_ENTERFN("CPushMtmFetchOperation::GetDefaultInetAccessPointL")

    // Try to find the default AP.
    // The Browser may stores the default AP in the shared data.
    CRepository* repository = CRepository::NewL( KCRUidBrowser );
    CleanupStack::PushL( repository );

    TInt wapApId;
    User::LeaveIfError
        ( repository->Get( KBrowserDefaultAccessPoint, wapApId ) );
    TUint32 wapApId32( wapApId );
    TUint32 iapId32( 0 );

    CleanupStack::PopAndDestroy( repository );
    repository = 0;

    CCommsDatabase* commsDb = CCommsDatabase::NewL( EDatabaseTypeIAP );
    CleanupStack::PushL( commsDb );
    CApDataHandler* apDataHandler = CApDataHandler::NewLC( *commsDb );
    CApAccessPointItem* apItem = CApAccessPointItem::NewLC();
    TRAPD( err, apDataHandler->AccessPointDataL( wapApId32, *apItem ) );
    CApUtils* apUtils = CApUtils::NewLC( *commsDb );

    TBool wapApFound( EFalse );
    TBool iapFound( EFalse );

    PUSHLOG_WRITE_FORMAT("  err: <%d>",err)
    if ( err == KErrNone )
        {
        wapApFound = ETrue;
        }
    else // err != KErrNone
        {
        // The first valid access point has to be selected if exists
        CApSelect* apSelect = CApSelect::NewLC
                                            (
                                                *commsDb, 
                                                KEApIspTypeAll, 
                                                EApBearerTypeAll, 
                                                KEApSortNameAscending
                                            );
        if ( apSelect->MoveToFirst() )
            {
            wapApFound = ETrue;
            wapApId32 = apSelect->Uid();
            }
        CleanupStack::PopAndDestroy( apSelect ); // apSelect
        }

    if ( wapApFound )
        {
        // Get the IAP ID from the WAP AP ID.
#ifndef __SERIES60_WSP
        iapId32 = 0;
        TRAPD( errTransl, iapId32 = apUtils->IapIdFromWapIdL( wapApId32 ) );
        PUSHLOG_WRITE_FORMAT("  IapIdFromWapIdL errTransl: <%d>",errTransl)
        if ( errTransl == KErrNone )
            {
            iapFound = ETrue;
            }
#else
        iapId32 = wapApId32;
        iapFound = ETrue;
#endif // __SERIES60_WSP
        }

    CleanupStack::PopAndDestroy( 4 ); // apUtils, apItem, 
                                            // apDataHandler, commsDb

    if ( iapFound )
        {
        aIapId = iapId32;
        }
    PUSHLOG_LEAVEFN("CPushMtmFetchOperation::GetDefaultInetAccessPointL")
    return iapFound;
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::InitializeL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::InitializeL()
    {
    PUSHLOG_ENTERFN("CPushMtmFetchOperation::InitializeL")

    User::LeaveIfError( iSockServ.Connect() );
    PUSHLOG_WRITE("CPushMtmFetchOperation iSockServ.Connect() OK")
    User::LeaveIfError( iConn.Open( iSockServ ) );
    PUSHLOG_WRITE("CPushMtmFetchOperation iConn.Open() OK")

    // Open the HTTP session.
    iHttpSess.OpenL( KHttpProtString );
    PUSHLOG_WRITE("CPushMtmFetchOperation iHttpSess.OpenL OK")

    // Set our Socket Server handle and Connection as session properties.
    iHttpSess.ConnectionInfo().SetPropertyL
        (
            StringF( HTTP::EHttpSocketServ ),
            THTTPHdrVal( iSockServ.Handle() )
        );
    PUSHLOG_WRITE("CPushMtmFetchOperation EHttpSocketServ set")
    iHttpSess.ConnectionInfo().SetPropertyL
        (
            StringF( HTTP::EHttpSocketConnection ),
            THTTPHdrVal( REINTERPRET_CAST( TInt, &iConn ) )
        );
    PUSHLOG_WRITE("CPushMtmFetchOperation EHttpSocketConnection set")

    // Open different string tables to the session. These are required 
    // by the Accept Header Filter.
    iHttpSess.StringPool().OpenL( HttpFilterCommonStringsExt::GetTable() );
    PUSHLOG_WRITE("CPushMtmFetchOperation table 1 opened");
    iHttpSess.StringPool().OpenL( HttpFilterCommonStringsExt::GetLanguageTable() );
    PUSHLOG_WRITE("CPushMtmFetchOperation table 2 opened");

    // Install the very same set of filters that BrowserEngine installs.
    // The following filters must not be installed:
    // - Authentication Filter, because the fetch operation and the push mtm 
    //   subsystem does not support credentials.
    // - Conn Handler Filter, because the fetch operation let it to the HTTP 
    //   core framework to create connection - it just sets the acces point.

    // Install UAProf filter.
    CHttpUAProfFilterInterface::InstallFilterL( iHttpSess );
    PUSHLOG_WRITE(" UAProf installed")

    // Install cookie filter.
    CHttpCookieFilter::InstallFilterL( iHttpSess );
    PUSHLOG_WRITE(" Cookie filter installed")

    // Create cache manager
    FeatureManager::InitializeLibL();
    iCacheMgr = CHttpCacheManager::NewL();
    PUSHLOG_WRITE(" Cache Manager installed")

    // Install Accept Header Filter.
    CHttpFilterAcceptHeaderInterface::InstallFilterL( iHttpSess );
    PUSHLOG_WRITE(" Accept Header Filter installed")

    // Install IOP Filter.
    CHttpFilterIopInterface::InstallFilterL( iHttpSess, iopOptionHostHeader );
    PUSHLOG_WRITE(" IOP Filter installed")

    // Install Deflate Filter.
    CHttpDeflateFilter::InstallFilterL( iHttpSess );
    PUSHLOG_WRITE(" Deflate Filter installed");

    TUint32 defIapId( 0 );
    TBool defIapIdFound = EFalse;
    TCommDbConnPref commPrefs;
#ifndef __WINS__
    // Use Browser default AP & comms db:
    defIapIdFound = GetDefaultInetAccessPointL( defIapId );
    PUSHLOG_WRITE_FORMAT("  defIapIdFound: <%d>",(TInt)defIapIdFound)
    PUSHLOG_WRITE_FORMAT("  defIapId:      <%d>",defIapId)
    commPrefs.SetDialogPreference( ECommDbDialogPrefDoNotPrompt );
    if ( defIapIdFound )
        {
        commPrefs.SetIapId( defIapId );
        }
#else // __WINS__
    // Prompt the user to select access point
    commPrefs.SetDialogPreference( ECommDbDialogPrefPrompt );
#endif // __WINS__

    iState = EStartRequest;

    iStatus = KRequestPending;
    SetActive();
    iConn.Start( commPrefs, iStatus );

    PUSHLOG_LEAVEFN("CPushMtmFetchOperation::InitializeL")
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::StartRequestL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::StartRequestL()
    {
    PUSHLOG_ENTERFN("CPushMtmFetchOperation::StartRequestL")

    // Create the transaction.
    CreateTransactionL();

    CheckCacheL();

    PUSHLOG_LEAVEFN("CPushMtmFetchOperation::StartRequestL")
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::CheckCacheL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::CheckCacheL()

    {
    PUSHLOG_ENTERFN("CPushMtmFetchOperation::CheckCacheL")
    iCacheSupply = CPushMtmCacheSupply::NewL(iCacheMgr, this);
    TInt ret = iCacheSupply->StartRequestL();
    iStatus = KRequestPending;
	SetActive();
    if (ret != KErrNone) // Not in cache
        {
        iState = ERequest;
        iCachedResponse = ETrue;
        TRequestStatus* status = &iStatus;
        User::RequestComplete( status, KErrNone);
        }
    else
        {
        iState = EDone;
        iCachedResponse = EFalse;
        }

    PUSHLOG_LEAVEFN("CPushMtmFetchOperation::CheckCacheL")
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::RequestL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::RequestL()
    {
    PUSHLOG_ENTERFN("CPushMtmFetchOperation::RequestL")

    iState = EDone;

    iStatus = KRequestPending;
	SetActive();
    SubmitTransactionL();
    // Do not call InvokeRun(). Let MHFRunL to handle the request.

    PUSHLOG_LEAVEFN("CPushMtmFetchOperation::RequestL")
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::Done
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::Done()
    {
    PUSHLOG_WRITE_FORMAT("CPushMtmFetchOperation::Done iResult: <%d>",iResult);

    delete iRequestedUrl;
    iRequestedUrl = NULL;

    // Notify parent.
    TRequestStatus* status = &iObserver;
    User::RequestComplete( status, iResult );
//  iState = EInit;
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::CreateTransactionL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::CreateTransactionL()
    {
    PUSHLOG_ENTERFN("CPushMtmFetchOperation::CreateTransactionL")

    __ASSERT_DEBUG( iRequestedUrl, 
                    ContHandPanic( EPushContHandPanNotInitUrl ) );

    // Create the transaction.
    TUriParser8 uri;
    User::LeaveIfError( uri.Parse( *iRequestedUrl ) );
    iHttpTrans = 
        iHttpSess.OpenTransactionL( uri, *this, StringF( HTTP::EGET ) );
    PUSHLOG_WRITE("CPushMtmFetchOperation OpenTransactionL OK")

    // Set request headers.
    RHTTPHeaders hdr = iHttpTrans.Request().GetHeaderCollection();
    SetHeaderL( hdr, HTTP::EUserAgent, KUserAgent );
    SetHeaderL( hdr, HTTP::EAccept, HTTP::EAnyAny );

    PUSHLOG_LEAVEFN("CPushMtmFetchOperation::CreateTransactionL")
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::SubmitTransactionL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::SubmitTransactionL()
    {
    iHttpTrans.SubmitL();
    PUSHLOG_WRITE("CPushMtmFetchOperation iHttpTrans.SubmitL OK")
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::HandleResponseHeadersL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::HandleResponseHeadersL( RHTTPResponse aResponse )
    {
    PUSHLOG_ENTERFN("CPushMtmFetchOperation::HandleResponseHeadersL")

    TInt httpCode = aResponse.StatusCode();
    PUSHLOG_WRITE_FORMAT(" HttpStatus: %d",httpCode)

#ifdef __TEST_LOG__
    LogHeadersL( aResponse.GetHeaderCollection() );
#endif // __TEST_LOG__

    if ( HTTPStatus::IsSuccessful( httpCode ) && 
         ( httpCode != HTTPStatus::ENoContent ) )
        {
        // Successful GET.
        PUSHLOG_WRITE(" Successful GET")
        // Get content type.
        iDataType = TDataType
            ( GetContentTypeL( aResponse.GetHeaderCollection() ) );
        // Get source URI.
        HBufC8* buf = GetSourceUriL( iHttpTrans ).AllocL();
        PUSHLOG_WRITE(" Source URI got")
        delete iSourceUri;
        iSourceUri = buf;
        // No error:
        iResult = KErrNone;
        }
    else
        {
        // See Extended Error Handling UI Specification!
        iResult = KPushMtmHttpErrorBase - httpCode;
        PUSHLOG_WRITE_FORMAT(" iResult: <%d>",iResult)
        }

    PUSHLOG_LEAVEFN("CPushMtmFetchOperation::HandleResponseHeadersL")
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::VersionRetryL
// ---------------------------------------------------------
//
TBool CPushMtmFetchOperation::VersionRetryL()
    {
    TBool retryDone( EFalse );

    // Check what http version we are using.
    THTTPHdrVal val;
    TBool exists = iHttpSess.ConnectionInfo().Property
        (
        StringF( HTTP::EHTTPVersion ), 
        val
        );
    if ( exists && TInt( val ) == HTTP::EHttp11 )
        {
        // Currently using HTTP/1.1. Cancel transaction and resubmit it using
        // HTTP/1.0.
        iHttpTrans.Cancel();
        iHttpSess.ConnectionInfo().SetPropertyL
            (
            StringF( HTTP::EHTTPVersion ),
            THTTPHdrVal( StringF( HTTP::EHttp10 ) )
            );
        SubmitTransactionL();
        retryDone = ETrue;
        }

    return retryDone;
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::AppendResponse
// ---------------------------------------------------------
//
TInt CPushMtmFetchOperation::AppendResponse( const TDesC8& aDataChunk )
    {
    TInt err( KErrNone );
    HBufC8* newBodyBuf = NULL;

    if ( !iBody )
        {
        // iBody does not exist yet; allocate buffer for the first data chunk.
        newBodyBuf = HBufC8::New( aDataChunk.Length() );
        }
    else
        {
        const TInt needed( iBody->Des().Length() + aDataChunk.Length() );
        if ( iBody->Des().MaxLength() < needed )
            {
            // iBody exists, but small.
            newBodyBuf = iBody->ReAlloc( needed );
            iBody = NULL; // ReAlloc deleted it!
            }
        else
            {
            // iBody exists and the new chunk fits into it.
            }
        }

    if ( newBodyBuf )
        {
        __ASSERT_DEBUG( iBody == NULL, 
                        ContHandPanic( EPushContHandPanAlreadyInitialized ) );
        delete iBody; // Not necessary, JIC.
        iBody = newBodyBuf;
        }
    else
        {
        if (aDataChunk.Length() > 0)
            {
            err = KErrNoMemory;
            }
        }

    if ( !err )
        {
        iBody->Des().Append( aDataChunk );
        }

    return err;
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::SetHeaderL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::SetHeaderL( RHTTPHeaders aHeaders, 
                                         HTTP::TStrings aHdrField, 
                                         const TDesC8& aHdrValue )
    {
    RStringF valStr = iHttpSess.StringPool().OpenFStringL( aHdrValue );
    CleanupClosePushL<RStringF>( valStr );
    SetHeaderL( aHeaders, aHdrField, valStr );
    CleanupStack::PopAndDestroy();  // close valStr
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::SetHeaderL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::SetHeaderL( RHTTPHeaders aHeaders, 
                                         HTTP::TStrings aHdrField, 
                                         HTTP::TStrings aHdrValue )
    {
    SetHeaderL( aHeaders, aHdrField, StringF( aHdrValue ) );
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::SetHeaderL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::SetHeaderL( RHTTPHeaders aHeaders, 
                                         HTTP::TStrings aHdrField, 
                                         const RStringF aHdrValue )
    {
    THTTPHdrVal val( aHdrValue );
    aHeaders.SetFieldL( StringF( aHdrField ), val );
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::GetContentTypeL
// ---------------------------------------------------------
//
const TDesC8& CPushMtmFetchOperation::GetContentTypeL( RHTTPHeaders aHeaders )
    {
    THTTPHdrVal hdrVal;
    User::LeaveIfError
        ( aHeaders.GetField( StringF( HTTP::EContentType ), 0, hdrVal ) );
    if ( hdrVal.Type() != THTTPHdrVal::KStrFVal )
        {
        User::Leave( KErrArgument );
        }
    return hdrVal.StrF().DesC();
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::GetSourceUriL
// ---------------------------------------------------------
//
const TDesC8& CPushMtmFetchOperation::GetSourceUriL
                                      ( RHTTPTransaction aTransaction )
    {
    // TODO if redirected URI exists, how do we get it?
    // For the moment request URI is used.
    return aTransaction.Request().URI().UriDes();
    }


// ---------------------------------------------------------
// CPushMtmFetchOperation::LogHeadersL
// ---------------------------------------------------------
//
#ifndef __TEST_LOG__

void CPushMtmFetchOperation::LogHeadersL( RHTTPHeaders /*aHeaders*/ )
    {
    }

#else // #ifdef __TEST_LOG__

void CPushMtmFetchOperation::LogHeadersL( RHTTPHeaders aHeaders )
    {
    PUSHLOG_ENTERFN("CPushMtmFetchOperation::LogHeadersL")

    _LIT( KDateFormat, "%D%M%Y%/0%1%/1%2%/2%3%/3 %:0%H%:1%T%:2%S.%C%:3" );

    PUSHLOG_WRITE("Headers:");

    RStringPool strP = iHttpTrans.Session().StringPool();
    THTTPHdrFieldIter it = aHeaders.Fields();

    while ( !it.AtEnd() )
        {
        RStringTokenF fieldName = it();
        RStringF fieldNameStr = strP.StringF( fieldName );
        THTTPHdrVal fieldVal;
        if ( aHeaders.GetField( fieldNameStr, 0, fieldVal ) == KErrNone )
            {
            const TInt KMaxLogStrLen = 64;
            TPtrC8 fieldName8 = fieldNameStr.DesC();
            TInt fieldNameLen = fieldName8.Length();
            fieldNameLen = ( KMaxLogStrLen < fieldNameLen ) ? 
                             KMaxLogStrLen : fieldNameLen;
            fieldName8.Set( fieldName8.Mid( 0, fieldNameLen ) );
            HBufC* fieldNameHBufC = HBufC::NewMaxLC( fieldName8.Length() );
            TPtr fieldName = fieldNameHBufC->Des();
            fieldName.Copy( fieldName8 );

            switch ( fieldVal.Type() )
                {
                case THTTPHdrVal::KTIntVal:
                    {
                    PUSHLOG_WRITE_FORMAT2
                        (" <%S> (%d)",&fieldName,fieldVal.Int())
                    break;
                    }

                case THTTPHdrVal::KStrFVal:
                    {
                    RStringF fieldValStr = strP.StringF( fieldVal.StrF() );
                    TPtrC8 fieldVal8 = fieldValStr.DesC();
                    // Truncate if too long.
                    TInt fieldValLen = fieldVal8.Length();
                    fieldNameLen = ( KMaxLogStrLen < fieldValLen ) ? 
                                     KMaxLogStrLen : fieldValLen;
                    fieldVal8.Set( fieldVal8.Mid( 0, fieldValLen ) );
                    // Truncate end.
                    HBufC* fieldValHBufC = HBufC::NewMaxLC( fieldVal8.Length() );
                    TPtr fieldVal = fieldValHBufC->Des();
                    fieldVal.Copy( fieldVal8 );
                    PUSHLOG_WRITE_FORMAT2
                        (" <%S> (%S)",&fieldName,&fieldVal)
                    CleanupStack::PopAndDestroy( fieldValHBufC ); 
                                                // fieldValHBufC
                    break;
                    }

                case THTTPHdrVal::KStrVal:
                    {
                    RString fieldValStr = strP.String( fieldVal.Str() );
                    TPtrC8 fieldVal8 = fieldValStr.DesC();
                    // Truncate if too long.
                    TInt fieldValLen = fieldVal8.Length();
                    fieldNameLen = ( KMaxLogStrLen < fieldValLen ) ? 
                                     KMaxLogStrLen : fieldValLen;
                    fieldVal8.Set( fieldVal8.Mid( 0, fieldValLen ) );
                    // Truncate end.
                    HBufC* fieldValHBufC = HBufC::NewMaxLC( fieldVal8.Length() );
                    TPtr fieldVal = fieldValHBufC->Des();
                    fieldVal.Copy( fieldVal8 );
                    PUSHLOG_WRITE_FORMAT2
                        (" <%S> (%S)",&fieldName,&fieldVal)
                    CleanupStack::PopAndDestroy( fieldValHBufC ); 
                                                // fieldValHBufC
                    break;
                    }

                case THTTPHdrVal::KDateVal:
                    {
                    TDateTime date = fieldVal.DateTime();
                    TBuf<40> dateTimeString;
                    TTime t( date );
                    TRAP_IGNORE( t.FormatL( dateTimeString, KDateFormat ) );
                    PUSHLOG_WRITE_FORMAT2
                        (" <%S> (%S)",&fieldName,&dateTimeString)
                    break;
                    }

                default:
                    {
                    PUSHLOG_WRITE_FORMAT2
                        (" <%S> unrecognised value type (%d)",
                           &fieldName,fieldVal.Type())
                    break;
                    }
                }

            // Display realm for WWW-Authenticate header.
            RStringF wwwAuth = strP.StringF
                ( HTTP::EWWWAuthenticate, RHTTPSession::GetTable() );
            if ( fieldNameStr == wwwAuth )
                {
                PUSHLOG_WRITE(" fieldNameStr == wwwAuth")
                // check the auth scheme is 'basic'
                RStringF basic = strP.StringF
                    ( HTTP::EBasic, RHTTPSession::GetTable() );
                RStringF realm = strP.StringF
                    ( HTTP::ERealm, RHTTPSession::GetTable() );
                THTTPHdrVal realmVal;
                if ( ( fieldVal.StrF() == basic ) && 
                    ( !aHeaders.GetParam( wwwAuth, realm, realmVal ) ) )
                    {
                    RStringF realmValStr = strP.StringF( realmVal.StrF() );
                    PUSHLOG_WRITE_FORMAT(" Realm <%S>",&realmValStr);
                    }
                }

            CleanupStack::PopAndDestroy( fieldNameHBufC ); // fieldNameHBufC
            }
        ++it;
        }

    PUSHLOG_LEAVEFN("CPushMtmFetchOperation::LogHeadersL")
    }

#endif // __TEST_LOG__

// ---------------------------------------------------------
// CPushMtmFetchOperation::DoCancel
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::DoCancel()
    {
    switch ( iState )
        {
        /*case EStartConnect:
            {
            // Complete ourselves - nothing is outstanding.
            TRequestStatus* ownStatus = &iStatus;
            User::RequestComplete( ownStatus, KErrCancel );
            break;
            }*/

        /*case EConnect:
            {
            //__ASSERT_DEBUG( iConn, ContHandPanic( ECodInternal ) );
            //iConn->Cancel(); // This will complete our status.
            break;
            }*/

        case EStartRequest:
        case ECheckCache:
        case ERequest:
            {
            delete iCacheSupply;
            iCacheSupply = NULL;

            iHttpTrans.Close();
            // Now the transaction has been closed. Closing it does not
            // complete our status, so we do it manually.
            TRequestStatus* status = &iStatus;
            SetActive();
            User::RequestComplete( status, iResult );
            break;
            }

        case EInit:
        default:
            {
            // No requests should be outstanding in these states.
            //ContHandPanic( ECodInternal );
            break;
            }
        }

    iResult = KErrCancel;
    Done();
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::RunL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::RunL()
    {
    PUSHLOG_WRITE_FORMAT("CPushMtmFetchOperation::RunL status = %d", iState);
    // Handle errors in RunError().
    User::LeaveIfError( iStatus.Int() );

    switch ( iState )
        {
        case EInit:
            {
            InitializeL();
            break;
            }

        case EStartRequest:
            {
            StartRequestL();
            break;
            }

        case ERequest:
            {
            RequestL();
            break;
            }

        case EDone:
            {
            Done();
            break;
            }

        default:
            {
            // JIC.
            Done();
            break;
            }
        }
    PUSHLOG_WRITE_FORMAT("CPushMtmFetchOperation::RunL moving to status = %d", iState);
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::RunError
// ---------------------------------------------------------
//
TInt CPushMtmFetchOperation::RunError( TInt aError )
    {
    PUSHLOG_WRITE_FORMAT("CPushMtmFetchOperation::RunError <%d>",aError)

    iResult = aError;
    delete iCacheSupply;
    iCacheSupply = NULL;
    iHttpTrans.Close();
    Done();

    return KErrNone;
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::MHFRunL
// ---------------------------------------------------------
//
void CPushMtmFetchOperation::MHFRunL
    ( 
        RHTTPTransaction DEBUG_ONLY( aTransaction ), 
        const THTTPEvent& aEvent 
    )
    {
    PUSHLOG_ENTERFN("CPushMtmFetchOperation::MHFRunL")
    __ASSERT_DEBUG( aTransaction == iHttpTrans, 
                    ContHandPanic( EPushContHandPanBadTransaction ) );
    switch ( aEvent.iStatus )
        {
        case THTTPEvent::EGotResponseHeaders:
            {
            PUSHLOG_WRITE(" EGotResponseHeaders")
            // pass headers to the cache first
            TRAP_IGNORE( iCacheSupply->HeadersReceivedL() );
            // Now we know that the request was processed by the server.
            HandleResponseHeadersL( iHttpTrans.Response() );
            break;
            }

        case THTTPEvent::EGotResponseBodyData:
            {
            PUSHLOG_WRITE(" EGotResponseBodyData")
            // pass chunk to the cache first
            TRAP_IGNORE( iCacheSupply->BodyReceivedL() );
            // Get body data and save it.
            TInt err( KErrNone );
            MHTTPDataSupplier* body = iHttpTrans.Response().Body();
            if ( !body )
                {
                PUSHLOG_WRITE(" NULL body")
                }
            else
                {
                TPtrC8 bodyPtr;
                // Caution: no leaving between body->GetNextDataPart and
                // body->ReleaseData calls! Data must always be released.
#ifdef __TEST_LOG__
                TBool isLast = body->GetNextDataPart( bodyPtr ); // No leave...
                PUSHLOG_WRITE_FORMAT(" last? (%d)",(isLast?1:0))
#else // ifndef __TEST_LOG__
                (void)body->GetNextDataPart( bodyPtr ); // No leave...
#endif // __TEST_LOG__
                //if ( iHttpTrans.Request().Method() == StringF( HTTP::EGET ) )
                err = AppendResponse( bodyPtr );
                body->ReleaseData();                          // ...until here.
                PUSHLOG_WRITE_FORMAT(" err: (%d)",err)
                User::LeaveIfError( err );
                }
            break;
            }

        case THTTPEvent::EResponseComplete:
            {                  
            // do not mix it up with the ESucceeded
            // The transaction's response is complete. An incoming event. 
            TRAP_IGNORE( iCacheSupply->ResponseCompleteL() );
            break;
            }

        case THTTPEvent::ERequestComplete:
            {
            // request is all set
            iCacheSupply->CloseRequest();
            break;
            }

        case THTTPEvent::EFailed:
            {
            PUSHLOG_WRITE(" EFailed")
            // Safety code: we should already have an error code.
            if ( iResult == KErrNone )
                {
                iResult = KErrGeneral;
                }
            // TODO. Fall through.
            }

        case THTTPEvent::ESucceeded:
            {
            PUSHLOG_WRITE(" ESucceeded")
            delete iCacheSupply;
            iCacheSupply = NULL;
            iHttpTrans.Close();
            // Transaction is finished, invoke RunL now.
            TRequestStatus* status = &iStatus;
            if (!IsActive())
                {
                SetActive();
                }
            User::RequestComplete( status, iResult );
            break;
            }

        case THTTPEvent::ERedirectedPermanently:
            {
            PUSHLOG_WRITE(" ERedirectedPermanently")
            // Do nothing just continue.
            break;
            }

        default:
            {
            PUSHLOG_WRITE(" default case")
            // Handle errors in MHFRunError.
            User::LeaveIfError( aEvent.iStatus );
            break;
            }
        }

    PUSHLOG_LEAVEFN("CPushMtmFetchOperation::MHFRunL")
    }

// ---------------------------------------------------------
// CPushMtmFetchOperation::MHFRunError
// ---------------------------------------------------------
//
TInt CPushMtmFetchOperation::MHFRunError
    ( 
        TInt aError, 
        RHTTPTransaction DEBUG_ONLY( aTransaction ), 
        const THTTPEvent& /*aEvent*/ 
    )
    {
    __ASSERT_DEBUG( aTransaction == iHttpTrans, 
                    ContHandPanic( EPushContHandPanBadTransaction ) );
    iResult = aError;
    iCacheSupply->CloseRequest();
    iHttpTrans.Close();

    // Transaction is finished, invoke RunL now. 
    // iResult contains the (error) code.
    if ( !IsActive() )
        {
	    SetActive();
        }
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );

    return KErrNone;
    }

// End of file.
