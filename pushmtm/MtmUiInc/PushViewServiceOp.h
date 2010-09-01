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
* Description:  Declaration of CPushViewServiceOp.
*
*/



#ifndef PUSHVIEWSERVICEOP_H
#define PUSHVIEWSERVICEOP_H

// INCLUDE FILES

#include "PushMtmUiOperation.h"
#include <e32base.h>
#include <apparc.h>
#include <apaserverapp.h>

// FORWARD DECLARATIONS

class CDocumentHandler;

// CLASS DECLARATION

/**
* This asynchronous operation is responsible for launching the appropriate 
* application (embedded if possible) that is capable of rendering the content 
* of a SP simple push message.
* The operation uses the Document Handler to lauch the appropriate handler 
* application for the given content.
*/
class CPushViewServiceOp : public CPushMtmUiOperation,
                           public MAknServerAppExitObserver
    {
    public: // Constructors and destructor

        /**
        * Constructor.
        * @param aSession Message Server Session to be used by this operation.
        * @param aEntryId Id of entry to operate on.
        * @param aObserverStatus Observer's status.
        */
        CPushViewServiceOp( CMsvSession& aSession, 
                            TMsvId aEntryId, 
                            TRequestStatus& aObserverStatus );

        /**
        * Destructor.
        */
        virtual ~CPushViewServiceOp();

    public: // Functions from CPushMtmOperation

        /**
        * Start or restart the operation.
        * Current implementation calls Cancel and invokes RunL.
        * Derived classes may override this.
        */
        void StartL();

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

    protected: // From MAknServerAppExitObserver

        void HandleServerAppExit( TInt aReason );

    private: // Data

        CDocumentHandler* iDocHandler; ///< Owned.
    };

#endif // PUSHVIEWSERVICEOP_H

// End of file.
