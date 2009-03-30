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
* Description:  Declaration of CPushViewMessageOp.
*
*/



#ifndef PUSHVIEWMESSAGEOP_H
#define PUSHVIEWMESSAGEOP_H

// INCLUDE FILES

#include "PushMtmUiOperation.h"
#include <e32base.h>
#include <apparc.h>
#include <AknServerApp.h>

// FORWARD DECLARATIONS

class CDocumentHandler;

// CLASS DECLARATION

/**
* This asynchronous operation is responsible for launching the appropriate 
* application (embedded if possible) that is capable of rendering the text 
* of an SI message.
* The operation uses the Document Handler to lauch the appropriate handler 
* application.
*/
class CPushViewMessageOp : public CPushMtmUiOperation, 
                           public MAknServerAppExitObserver
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aSession Message Server Session to be used by this operation.
        * @param aEntryId Id of entry to operate on.
        * @param aObserverStatus Observer's status.
        * @return The constructed object.
        */
        static CPushViewMessageOp* NewL( CMsvSession& aSession, 
                                         TMsvId aEntryId, 
                                         TRequestStatus& aObserverStatus );

        /**
        * Destructor.
        */
        virtual ~CPushViewMessageOp();

    public: // Functions from CPushMtmOperation

        /**
        * Start or restart the operation.
        * Current implementation calls Cancel and invokes RunL.
        * Derived classes may override this.
        */
        void StartL();

    protected: // Constructors

        /**
        * Constructor.
        * @param aSession Message Server Session to be used by this operation.
        * @param aEntryId Id of entry to operate on.
        * @param aObserverStatus Observer's status.
        */
        CPushViewMessageOp( CMsvSession& aSession, 
                            TMsvId aEntryId, 
                            TRequestStatus& aObserverStatus );

        /**
        * Symbian OS constructor.
        */
        void ConstructL();

    protected: // New functions

        /**
        * Show information note and cancel the operation.
        */
        void NotifyAndCancelL( TInt aResId );

    protected: // Functions from base classes

        /**
        * Current implementation calls SignalObserver( KErrNone ).
        * Derived classes should override this and also should 
        * call SignalObserver.
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

    protected: // From MAknServerAppExitObserver

        void HandleServerAppExit( TInt aReason );

    private: // Data

        CDocumentHandler* iDocHandler; ///< Owned.
        TBool iIsCancelled;
    };

#endif // PUSHVIEWMESSAGEOP_H

// End of file.
