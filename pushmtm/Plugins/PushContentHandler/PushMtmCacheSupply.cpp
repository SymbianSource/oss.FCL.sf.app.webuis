/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Implementation of CPushMtmCacheSupply
*
*/


// INCLUDE FILES
#include "PushMtmCacheSupply.h"
#include "httpcachemanager.h"
#include "PushMtmCacheDataSupplier.h"

#include <http/RHTTPTransaction.h>
#include <http/mhttpdatasupplier.h>
#include <http/rhttpsession.h>
#include <httperr.h>


// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS
const TInt KResponseTimeout = 0;

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::CPushMtmCacheSupply
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CPushMtmCacheSupply::CPushMtmCacheSupply(MCacheSupplyCallbacks* aCacheSupplyCallbacks ):
      // closed by default
      iReponseState( THTTPEvent::EClosed )
    {
    iCacheEntry.iCacheHandler = NULL;
    iCacheEntry.iCacheEntry = NULL;
    iCacheSupplyCallbacks = aCacheSupplyCallbacks;
    }

// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CPushMtmCacheSupply::ConstructL(CHttpCacheManager* aCacheMgr)
    {
    iCacheManager = aCacheMgr;
    }

// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CPushMtmCacheSupply* CPushMtmCacheSupply::NewL(CHttpCacheManager* aCacheMgr, MCacheSupplyCallbacks* aCacheSupplyCallbacks  )
    {
    CPushMtmCacheSupply* self = new( ELeave ) CPushMtmCacheSupply( aCacheSupplyCallbacks);

    CleanupStack::PushL( self );
    self->ConstructL(aCacheMgr);
    CleanupStack::Pop();

    return self;
    }

// Destructor
CPushMtmCacheSupply::~CPushMtmCacheSupply()
    {
    delete iDataSupplier;
    CloseRequest();
    if (iResponseTimer)
        {
        iResponseTimer->Cancel();
        }
    delete iResponseTimer;
    }

// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::StartRequestL
//
//
// -----------------------------------------------------------------------------
//
TInt CPushMtmCacheSupply::StartRequestL( )
    {
    TInt status;

    status = iCacheManager->RequestL( *(iCacheSupplyCallbacks->Transaction()), 
    TBrCtlDefs::ECacheModeNormal , iCacheEntry );
    // start a timer that feeds the content to the transaction
    if( status == KErrNone )
        {
        iClosed = EFalse;
        // set response state. start with the http headers.
        iReponseState = THTTPEvent::EGotResponseHeaders;
        //
        iResponseTimer = CPeriodic::NewL( CActive::EPriorityHigh );
        iResponseTimer->Start( KResponseTimeout, KResponseTimeout, TCallBack( &ResponseCallbackL, this ) );
        }
    return status;
    }

// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::CloseRequest
//
//
// -----------------------------------------------------------------------------
//
void CPushMtmCacheSupply::CloseRequest()
    {
    if( !iClosed )
	    {
	    iCacheManager->RequestClosed( (iCacheSupplyCallbacks->Transaction()), iCacheEntry );
	    iClosed = ETrue;
	    }
    }

// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::HeadersReceived
//
//
// -----------------------------------------------------------------------------
//
void CPushMtmCacheSupply::HeadersReceivedL()
    {
    // do not cache content we just sent off
    if( iReponseState == THTTPEvent::EClosed )
        {
        RHTTPTransaction* trans = iCacheSupplyCallbacks->Transaction();
        iCacheManager->ReceivedResponseHeadersL( *(iCacheSupplyCallbacks->Transaction()), iCacheEntry );
        //
        iNotModified = trans->Response().StatusCode() == HTTPStatus::ENotModified;
        if( iNotModified )
            {
            // change from 304 to 200 -otherwise trans exits with EFailed
            trans->Response().SetStatusCode( HTTPStatus::EOk );
            }
        }
    }

// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::BodyReceivedL
//
//
// -----------------------------------------------------------------------------
//
void CPushMtmCacheSupply::BodyReceivedL()
    {
    // do not cache content we just sent off
    if( iReponseState == THTTPEvent::EClosed )
        {
        RHTTPTransaction* trans = iCacheSupplyCallbacks->Transaction();
        MHTTPDataSupplier* supplier = trans->Response().Body();
        //
        iCacheManager->ReceivedResponseBodyDataL( *trans, *supplier, iCacheEntry );
        }
    }

// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::ResponseComplete
//
//
// -----------------------------------------------------------------------------
//
void CPushMtmCacheSupply::ResponseCompleteL()
    {
    if( iReponseState == THTTPEvent::EClosed )
        {
        RHTTPTransaction* trans = iCacheSupplyCallbacks->Transaction();
        // not modified needs body before response complete
        if( iNotModified )
            {
            // use cache
            // close response first
            iCacheManager->ResponseComplete( *trans, iCacheEntry );
            // request the item from cache
            if( iCacheManager->RequestL( *trans, TBrCtlDefs::ECacheModeOnlyCache, iCacheEntry ) == KErrNone )
                {
                // ser
                iReponseState = THTTPEvent::EGotResponseBodyData;
                //
                SendBodyL();
                CloseRequest();
                }
            }
        else
            {
            // normal response complete
            iCacheManager->ResponseComplete( *trans, iCacheEntry );
            }
        }
    }

// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::ResponseCallbackL
//
//
// -----------------------------------------------------------------------------
//
TInt CPushMtmCacheSupply::ResponseCallbackL(
    TAny* aAny )
    {
    //
    CPushMtmCacheSupply* thisObj = (CPushMtmCacheSupply*)aAny;
    thisObj->SupplyResponseL();
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::SupplyResponseL
//
//
// -----------------------------------------------------------------------------
//
void CPushMtmCacheSupply::SupplyResponseL()
    {
    RHTTPTransaction* trans = iCacheSupplyCallbacks->Transaction();
    //
    switch( iReponseState )
        {
        case THTTPEvent::EGotResponseHeaders:
            {
            //
            iFailed = EFalse;
            if( iCacheManager->RequestHeadersL( *trans, iCacheEntry ) == KErrNone )
                {
                //
                trans->Response().SetStatusCode( HTTPStatus::EOk );
                //
                iCacheSupplyCallbacks->HandleEventL( THTTPEvent::EGotResponseHeaders );
                // move to the next state
                iReponseState = THTTPEvent::EGotResponseBodyData;
                }
            else
                {
                // move to the next state
                iFailed = ETrue;
                // move to the next state
                iReponseState = THTTPEvent::EResponseComplete;
                }
            break;
            }
        case THTTPEvent::EGotResponseBodyData:
            {
            //
            SendBodyL();
            // move to the next state
            iReponseState = THTTPEvent::EResponseComplete;
            break;
            }
        case THTTPEvent::EResponseComplete:
            {
            iCacheSupplyCallbacks->HandleEventL( THTTPEvent::EResponseComplete );
            // move to the next state
            iReponseState = !iFailed ? THTTPEvent::ESucceeded : THTTPEvent::EFailed;
            break;
            }
        case THTTPEvent::ESucceeded:
            {
            // move to the next state
            iReponseState = THTTPEvent::EClosed;
            // cancel timer
            iResponseTimer->Cancel();
            //
            iCacheSupplyCallbacks->HandleEventL( THTTPEvent::ESucceeded );
            // this obj is destroyed at this point
            break;
            }
        case THTTPEvent::EFailed:
            {
            // move to the next state
            iReponseState = THTTPEvent::EClosed;
            // cancel timer
            iResponseTimer->Cancel();
            //
            iCacheSupplyCallbacks->HandleEventL( THTTPEvent::EFailed );
            // this obj is destroyed at this point
            break;
            }
        default:
            {
            //
            }
        }
    }

// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::SendBodyL
//
//
// -----------------------------------------------------------------------------
//
void CPushMtmCacheSupply::SendBodyL()
    {
    RHTTPTransaction* trans = iCacheSupplyCallbacks->Transaction();
    //
    TBool lastChunk;
    // currently it is always the last chunk
    HBufC8* body = iCacheManager->RequestNextChunkL( *trans, lastChunk, iCacheEntry );
    if( body )
        {
        CleanupStack::PushL( body );
        // create datasupplier and attach it to the transaction
        if( !iDataSupplier )
            iDataSupplier = CPushMtmCacheDataSupplier::NewL( body );
        trans->Response().SetBody( *iDataSupplier );
        CleanupStack::Pop(); // body
        //
        iCacheSupplyCallbacks->HandleEventL( THTTPEvent::EGotResponseBodyData );
        }
    else
        {
        // move to the next state
        iFailed = ETrue;
        }
    }
    
// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::PauseSupply
// -----------------------------------------------------------------------------
void CPushMtmCacheSupply::PauseSupply()
	{
	if( iResponseTimer && iResponseTimer->IsActive() )
		iResponseTimer->Cancel();
	}

// -----------------------------------------------------------------------------
// CPushMtmCacheSupply::ResumeSupply
// -----------------------------------------------------------------------------
void CPushMtmCacheSupply::ResumeSupply()
	{
	if( iResponseTimer && !iResponseTimer->IsActive() )
		iResponseTimer->Start( KResponseTimeout, KResponseTimeout, TCallBack( &ResponseCallbackL, this ) );
	}

//  End of File
