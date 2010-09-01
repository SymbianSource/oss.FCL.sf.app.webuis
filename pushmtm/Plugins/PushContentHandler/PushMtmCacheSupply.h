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
* Description:  Definition of CPushMtmCacheSupply
*
*/


#ifndef CPUSHMTMCACHESUPPLY_H
#define CPUSHMTMCACHESUPPLY_H

//  INCLUDES
#include <e32base.h>
#include <brctldefs.h>
#include "httpcachemanager.h"

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CHttpTransaction;
class CPeriodic;
class CPushMtmCacheDataSupplier;

// CLASS DECLARATION



class MCacheSupplyCallbacks
    {
    public:
        virtual RHTTPTransaction* Transaction() = 0;
        virtual void HandleEventL(THTTPEvent aEvent) = 0;
    };


/**
*  This class handles http transactions.
*  @lib PushMtmContentHandler
*  @since 3.1
*/
class CPushMtmCacheSupply : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param
        * @param
        * @param
        * @return Http cache  object.
        */
        static CPushMtmCacheSupply* NewL( CHttpCacheManager* aCacheMgr, MCacheSupplyCallbacks* aCacheSupplyCallbacks );

        /**
        * Destructor.
        */
        virtual ~CPushMtmCacheSupply();

    public: // new functions

        /**
        *
        * @since 3.1
        * @param
        * @return
        */
        TInt StartRequestL(  );

        /**
        *
        * @since 3.1
        * @param
        * @return
        */
        void CloseRequest();

        /**
        *
        * @since 3.1
        * @param
        * @return
        */
        void HeadersReceivedL();

        /**
        *
        * @since 3.1
        * @param
        * @return
        */
        void BodyReceivedL();

        /**
        *
        * @since 3.1
        * @param
        * @return
        */
        void ResponseCompleteL();

        /**
        *
        * @since 3.1
        * @param
        * @return
        */
        TInt ResponseState() const { return iReponseState;}

        /**
        *
        * @since 3.1
        * @param
        * @return
        */
        TBool IsSupplying()             { return iResponseTimer!=0; }

        /**
        *
        * @since 3.1
        * @param
        * @return
        */
		void PauseSupply();

        /**
        *
        * @since 3.1
        * @param
        * @return
        */
		void ResumeSupply();

    private:

        /**
        * Construct.
        * @param
        * @param
        * @parem
        * @return
        */
        CPushMtmCacheSupply( MCacheSupplyCallbacks* aCacheSupplyCallbacks  );

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL(CHttpCacheManager* aCacheMgr);

    private: //

        /**
        * Callback for async response
        * @since 3.1
        * @param  aAny this pinter
        * @return TInt KErrNone
        */
        static TInt ResponseCallbackL( TAny* aAny );

        /**
        *
        * @since 3.1
        * @param
        * @return
        */
        void SupplyResponseL();

        /**
        *
        * @since 3.1
        * @param
        * @return
        */
        void SendBodyL();

    private:    // Data

        //
        CHttpCacheManager*              iCacheManager;          // not owned
        //
        TInt                            iReponseState;
        //
        CPeriodic*                      iResponseTimer;         // owned
        //
        TBool                           iFailed;
        //
        TBool                           iNotModified;
        //
        THttpCacheEntry                 iCacheEntry;
        // ETrue if the cache object is closed
        TBool							iClosed;

        CPushMtmCacheDataSupplier*      iDataSupplier;
        MCacheSupplyCallbacks*          iCacheSupplyCallbacks;
  };

#endif      // CPUSHMTMCACHESUPPLY_H

// End of File
