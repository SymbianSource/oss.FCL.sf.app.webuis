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
* Description:  Declaration of PushMtmAutoFetchOperation.
*
*/



#ifndef PUSHMTMAUTOFETCHOPERATION_H
#define PUSHMTMAUTOFETCHOPERATION_H

// INCLUDE FILES

#include <e32base.h>

// FORWARD DECLARATIONS

class CPushMtmFetchOperation;

// CLASS DECLARATION

/**
* Asynchronous fetch operation that runs in silent mode and in addition to the 
* services of CPushMtmFetchOperation it retries downloading of the indicated 
* service if the first try fails.
*/
class CPushMtmAutoFetchOperation : public CActive
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aRequestedUrl The requested URL.
        * @param aTimeDelayInSec Time delay between fetches in seconds.
        * @param aObserverRequestStatus Completion status.
        * @return The constructed client.
        */
        static CPushMtmAutoFetchOperation* NewL
            (
                const TDesC& aRequestedUrl, 
                TInt aTimeDelayInSec, 
                TRequestStatus& aObserverRequestStatus 
            );

        /**
        * Destructor.
        */
        virtual ~CPushMtmAutoFetchOperation();

    public: // Functions from base classes

        /**
        * Start or restart the operation.
        */
        void StartL();

    protected: // Constructors 

        /**
        * Constructor.
        * @param aTimeDelayInSec Time delay between fetches in seconds.
        * @param aObserverRequestStatus Completion status.
        */
        CPushMtmAutoFetchOperation( TInt aTimeDelayInSec, 
                                    TRequestStatus& aObserverRequestStatus );

        /**
        * Second phase constructor.
        * @param aRequestedUrl The requested URL.
        */
        void ConstructL( const TDesC& aRequestedUrl );

    protected: // New functions

        /**
        * Issue fetching.
        */
        void FetchL();

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

    protected: // Data

        TRequestStatus& iObserver;  ///< Observer's status.
        HBufC* iRequestedUrl;       ///< Request URI or NULL. Owned.

        enum TState    ///< State.
            {
            EInit,     ///< Initial state.
            EFetch,    ///< Fetch.
            ECheck,    ///< Check the result of the fetching.
            EDone      ///< Operation done.
            };

        TState iState; ///< Current state of the state machine.

        /// The operation that does downloading. Owned.
        CPushMtmFetchOperation* iFetchOp;
        RTimer iTimer; ///< Timer for waiting.
        TInt iTimeDelayInSec; ///< Time delay between downloadings in seconds.
        TInt iTry; ///< How many times we have tried downloading so far?
    };

#endif // PUSHMTMAUTOFETCHOPERATION_H

// End of file.
