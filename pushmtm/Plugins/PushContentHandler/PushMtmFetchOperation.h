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
* Description:  Declaration of CPushMtmFetchOperation.
*
*/



#ifndef PUSHMTMFETCHOPERATION_H
#define PUSHMTMFETCHOPERATION_H

// INCLUDE FILES

#include <e32std.h>
#include <e32base.h>
#include <http.h>
#include <stringpool.h>
#include <apmstd.h>
#include <es_sock.h>
#include "HttpCacheManager.h"
#include "PushMtmCacheSupply.h"

// FORWARD DECLARATIONS

class CMsvEntry;

// CLASS DECLARATION

/**
* Asynchronous fetch operation that downloads the service indicated by an SL 
* pushed service message.
* 
* The operation can run in silent mode without requiring any user intervention.
*/
class CPushMtmFetchOperation : public CActive, 
                               public MHTTPTransactionCallback,
                               public MCacheSupplyCallbacks
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aRequestedUrl The requested URL.
        * @param aObserverRequestStatus Observer's status.
        * @return The constructed operation.
        */
        static CPushMtmFetchOperation* NewL
            (
                const TDesC& aRequestedUrl, 
                TRequestStatus& aObserverRequestStatus 
            );

        /**
        * Destructor.
        */
        virtual ~CPushMtmFetchOperation();

    public: // Functions from base classes

        /**
        * Start or restart the operation.
        */
        void StartL();

        RHTTPTransaction* Transaction(){ return &iHttpTrans; }
        void HandleEventL(THTTPEvent aEvent) { MHFRunL(iHttpTrans, aEvent); }

    protected: // Constructors

        /**
        * Constructor.
        * @param aObserverRequestStatus Observer's status.
        */
        CPushMtmFetchOperation( TRequestStatus& aObserverRequestStatus );

        /**
        * Second phase constructor.
        * @param aRequestedUrl The requested URL.
        * @return None.
        */
        void ConstructL( const TDesC& aRequestedUrl );

    protected: // New functions

        /**
        * Get the ID of the defauld internet access point set in the Browser 
        * Preferences that is used for connection creation.
        * @param aIapId The ID of the access point, if found. The value of 
        *        this variable is not changed if no default ap can be found.
        * @return Indicate if default ap could be found.
        */
        TBool GetDefaultInetAccessPointL( TUint32& aIapId ) const;

    protected:    // state machine parts

        /**
        * Initiate connecting.
        */
        void InitializeL();

        /**
        * StartRequestL
        */
        void StartRequestL();

        /**
        * CheckCacheL
        */
        void CheckCacheL();

        /**
        * Initiate connecting.
        */
        void RequestL();

        /**
        * Clean up and notify parent.
        */
        void Done();

        /**
        * Create transaction, set request headers (and body for POST request).
        */
        void CreateTransactionL();

        /**
        * Submit the transaction (make the HTTP request).
        */
        void SubmitTransactionL();

        /**
        * Handle response from server.
        * @param aResponse Response from server.
        */
        void HandleResponseHeadersL( RHTTPResponse aResponse );

        /**
        * HTTP Version Not Supported. Resubmit transaction using HTTP/1.0.
        * @return ETrue if transaction was resubmitted, EFalse if not (i.e
        * we already use HTTP/1.0).
        */
        TBool VersionRetryL();

        /**
        * Append a new chunk of response data to the reponse buffer.
        * @return KErrNone or KErrNoMemory.
        */
        TInt AppendResponse( const TDesC8& aDataChunk );

    protected:    // helpers

        /**
        * Helper function to set a header.
        * @param aHeaders Set the header to this header set.
        * @param aHdrField Header field name.
        * @param aHdrValue Header value.
        */
        void SetHeaderL
            (
            RHTTPHeaders aHeaders,
            HTTP::TStrings aHdrField,
            const TDesC8& aHdrValue
            );

        /**
        * Helper function to set a header.
        * @param aHeaders Set the header to this header set.
        * @param aHdrField Header field name.
        * @param aHdrValue Header value.
        */
        void SetHeaderL
            (
            RHTTPHeaders aHeaders,
            HTTP::TStrings aHdrField,
            HTTP::TStrings aHdrValue
            );

        /**
        * Helper function to set a header.
        * @param aHeaders Set the header to this header set.
        * @param aHdrField Header field name.
        * @param aHdrValue Header value.
        */
        void SetHeaderL
            (
            RHTTPHeaders aHeaders,
            HTTP::TStrings aHdrField,
            const RStringF aHdrValue
            );

        /**
        * Helper function to get the Content-Type header value.
        * @param aHeaders HTTP header set.
        * @return Content-Type header value.
        */
        const TDesC8& GetContentTypeL( RHTTPHeaders aHeaders );

        /**
        * Helper function to get the source URI of a transaction.
        * @param aTransaction Transaction.
        * @return Source URI.
        */
        const TDesC8& GetSourceUriL( RHTTPTransaction aTransaction );

        /**
        * Get a shared string from the session stringpool.
        * @param aId Id of string.
        * @return Shared string from HTTP string table. No need to close.
        */
        inline RStringF StringF( HTTP::TStrings aId );

    protected:    // debug utilities

        /**
        * In debug build, log headers. In release build, do nothing.
        * @param aHeaders Headers to log.
        */
        void LogHeadersL( RHTTPHeaders aHeaders );

    protected: // Functions from base classes

        /**
        * Cancel fetching.
        * @return None.
        */
        void DoCancel();

        /**
        * Start fetching and make an entry into the operation mapper.
        * @return None.
        */
        void RunL();

        /**
        * Leave handler protocol implementation: it is called when 
        * RunL leaves.
        */
        TInt RunError( TInt aError );

    protected:  // from MHTTPTransactionCallback

        /**
        * Handle transaction event.
        * @param aTransaction The transaction that the event has occurred on.
        * @param aEvent The event that has occurred.
        */
        void MHFRunL( RHTTPTransaction aTransaction, 
                      const THTTPEvent& aEvent );

        /**
        * Handle errors occured in MHFRunL().
        * @param aError The leave code that RunL left with.
        * @param aTransaction The transaction that was being processed.
        * @param aEvent The Event that was being processed.
        * @return KErrNone.
        */
        TInt MHFRunError( TInt aError, 
                          RHTTPTransaction aTransaction, 
                          const THTTPEvent& aEvent );

    protected: // Data

        TRequestStatus& iObserver;  ///< Observer's status.
        HBufC8* iRequestedUrl;      ///< Request URI or NULL. Owned.

        enum TState         ///< State.
            {
            EInit,          ///< Initial state.
            EStartRequest,  ///< Create HTTP session & request
            ECheckCache,    ///< Check if content is in cache
            ERequest,       ///< HTTP request.
            EDone
            };

        TState iState;              ///< Current state.
        TInt iResult;               ///< Resulting error code.

        RSocketServ iSockServ;      ///< Socket Server session handle.
        RConnection iConn;          ///< Connection handle.

        CHttpCacheManager* iCacheMgr;///< Cache Manager. Owned.

        RHTTPSession iHttpSess;     ///< HTTP session. Owned.
        RHTTPTransaction iHttpTrans;///< Transaction. Owned.

        HBufC8* iSourceUri;         ///< Source URI. Owned.
        HBufC8* iBody;              ///< Response body. Owned.
        TDataType iDataType;        ///< Data type.
        THttpCacheEntry iCacheEntry;///< Holds the cache entry
        CPushMtmCacheSupply* iCacheSupply;
        TBool iCachedResponse;
    };

#include "PushMtmFetchOperation.inl"

#endif // PUSHMTMFETCHOPERATION_H

// End of file.
