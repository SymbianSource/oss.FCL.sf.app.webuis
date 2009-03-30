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
* Description:  Declaration of CPushMtmOperation.
*
*/



#ifndef PUSHMTMOPERATION_H
#define PUSHMTMOPERATION_H

// INCLUDE FILES

#include "PushMtmProgress.h"
#include <e32base.h>
#include <msvapi.h>

// CLASS DECLARATION

/**
* Base class for mtm operations controlling asynchronous functions on the 
* client side.
*/
class CPushMtmOperation : public CMsvOperation, 
                          public MMsvEntryObserver
    {
    public: // Constructors and destructor

        /**
        * Destructor.
        */
        IMPORT_C virtual ~CPushMtmOperation();

    public: // New functions

        /**
        * Start or restart the operation.
        * Current implementation calls Cancel and invokes RunL.
        * Derived classes may override this.
        */
        IMPORT_C virtual void StartL();

    public: // Functions from base classes

        /**
        * Get progress information.
        * @return A package buffer (of type TPushMtmProgressBuf), containing 
        *         progress information.
        */
        IMPORT_C const TDesC8& ProgressL();

    protected: // Constructors

        /**
        * Constructor. Calls CActiveScheduler::Add( this )!
        * @param aSession Message Server Session to be used by this operation.
        * @param aId Id of entry to operate on.
        * @param aObserverRequestStatus Observer's status.
        */
        IMPORT_C CPushMtmOperation( CMsvSession& aSession, 
                                    TMsvId aId, 
                                    TRequestStatus& aObserverRequestStatus );

    protected: // New functions

        /**
        * Invoke RunL. Complete request with KErrNone.
        * @return None.
        */
        IMPORT_C void InvokeRun();

        /**
        * Signal the observer that the asynchronous request is complete.
        * @param aCompletionCode The value for which the observer's request 
        *        status is set.
        */
        IMPORT_C void SignalObserver( TInt aCompletionCode );

        /**
        * Request entry event observation.
        * @return None.
        */
        IMPORT_C void ObserveEntryEventL();

        /**
        * Cancel the request for entry event observation.
        * @return None.
        */
        IMPORT_C void CancelObserveEntryEvent();

    protected: // Functions from base classes

        /**
        * Current implementation calls SignalObserver( KErrNone ).
        * Derived classes should override this and also should 
        * call SignalObserver.
        */
        IMPORT_C void RunL();

        /**
        * Cancel protocol implementation.
        */
        IMPORT_C void DoCancel();

        /**
        * Leave handler protocol implementation: it is called when 
        * RunL leaves. The observer is signaled if a leave occures. 
        * The leave code is not forwarded to the scheduler.
        */
        IMPORT_C TInt RunError( TInt aError );

        /**
        * Call-back for entry event handling. Derived classes may override it.
        */
        IMPORT_C void HandleEntryEventL( TMsvEntryEvent aEvent, 
                      TAny* aArg1, TAny* aArg2, TAny* aArg3 );

    protected: // Data

        TMsvId iEntryId; ///< Id of entry to operate on.
        CMsvEntry* iCEntry; ///< Context. Owned.
        TPushMtmProgress iProgress; ///< Progress information.
        /// 8 bit descriptor package pointing to the progress information.
        TPushMtmProgressPckg iProgressPckg;
    };

#endif // PUSHMTMOPERATION_H

// End of file.
