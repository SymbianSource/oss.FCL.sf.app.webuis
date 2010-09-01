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
* Description:  Declaration of CPushMessageInfoOp.
*
*/



#ifndef PUSHMESSAGEINFOOP_H
#define PUSHMESSAGEINFOOP_H

// INCLUDE FILES

#include "PushMtmUiOperation.h"
#include <e32base.h>

// FORWARD DECLARATIONS

class CPushMessageInfoDialog;

// CLASS DECLARATION

/**
* This asynchronous operation is responsible for executing the Message Info 
* dialog.
*/
class CPushMessageInfoOp : public CPushMtmUiOperation
    {
    public: // Constructors and destructor

        /**
        * Constructor.
        * @param aSession Message Server Session to be used by this operation.
        * @param aEntryId Id of entry to operate on.
        * @param aDontShowNotification ETrue: don't show info notes.
        * @param aObserverStatus Observer's status.
        */
        CPushMessageInfoOp( CMsvSession& aSession, 
                            TMsvId aEntryId, 
                            TBool aDontShowNotification, 
                            TRequestStatus& aObserverStatus );

        /**
        * Destructor.
        */
        virtual ~CPushMessageInfoOp();

    public: // Functions from CPushMtmOperation

        /**
        * Start or restart the operation.
        * Current implementation calls Cancel and invokes RunL.
        * Derived classes may override this.
        */
        void StartL();

    protected: // New functions

        /**
        * Decide if content changed and act accordingly.
        */
        void HandleEntryChangeL();

        /**
        * Show information note and cancel the operation.
        */
        void NotifyAndCancelL( TInt aResId );

    protected: // Functions from base classes

        /**
        * TODO.
        */
        void RunL();

        /**
        * Cancel protocol implementation.
        */
        void DoCancel();

        /**
        * Leave handler protocol implementation: it is called when 
        * RunL leaves. The observer is signaled if a leave occures. 
        * The leave code is not forwarded to the scheduler.
        */
        TInt RunError( TInt aError );

        /**
        * Call-back for entry event handling.
        */
        void HandleEntryEventL( TMsvEntryEvent aEvent, 
                                TAny* aArg1, TAny* aArg2, TAny* aArg3 );

    private: // Data

        CPushMessageInfoDialog* iDialog; ///< Owned.
        TBool iDontShowNotification;
        TBool iObserverCompleted;
        TBool* iDeletedFlag;
    };

#endif // PUSHMESSAGEINFOOP_H

// End of file.
