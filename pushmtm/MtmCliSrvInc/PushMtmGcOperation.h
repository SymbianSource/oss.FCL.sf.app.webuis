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
* Description:  Declaration of CPushMtmGcOperation.
*
*/



#ifndef PUSHMTMGCOPERATION_H
#define PUSHMTMGCOPERATION_H

// INCLUDE FILES

#include "PushMtmOperation.h"
#include <e32base.h>
#include <e32std.h>
#include <Msvstd.h>

// FORWARD DECLARATIONS

class CMsvSession;
class CPushMtmUtil;
class CMsvEntrySelection;
class CSIPushMsgEntry;

// CLASS DECLARATION

/**
* Garbage collection operation. It removes some or all expired SIs from the 
* Message Server.
*/
class CPushMtmGcOperation : public CPushMtmOperation
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aSession Message server session to be used by this operation.
        * @param aFolderId aFolderId Garbage collection is made in this folder.
        *        If it's KMsvNullIndexEntryId, then gc is made in all the 
        *        folders that may contain push messages.
        * @param aObserverRequestStatus Observer's status.
        * @return The constructed operation.
        */
        static CPushMtmGcOperation* NewL( CMsvSession& aSession, 
                                          TMsvId aFolderId, 
                                          TRequestStatus& aObserverRequestStatus );

        /**
        * Destructor.
        */
        virtual ~CPushMtmGcOperation();

    public: // New functions

        /**
        * Do the garbage collection synchronously.
        */
        void DoSyncL();

    protected: // Constructors

        /**
        * Constructor.
        * @param aSession Message server session to be used by this operation.
        * @param aFolderId Garbage collection is made in this folder.
        * @param aObserverRequestStatus Observer's status.
        */
        CPushMtmGcOperation( CMsvSession& aSession, 
                             TMsvId aFolderId, 
                             TRequestStatus& aObserverRequestStatus );

        /**
        * Symbian OS constructor.
        */
        void ConstructL();

    protected: // New functions

        /**
        * Initialize the operation. Creates an array of SI messages.
        */
        void InitL();

        /**
        * Delete the current entry if it is expired.
        */
        void GcCurrentL();

    protected: // Functions from base classes

        /**
        * Do garbage collection. Initializes an array of SI messages 
        * and check the expiration state: one check per one RunL shot.
        */
        void RunL();

        /**
        * Cancel protocol implementation. CPushMtmOperation::DoCancel() is 
        * called.
        */
        void DoCancel();

        /**
        * Leave handler protocol implementation: it is called when 
        * RunL leaves. It continues the operation if the current entry is not 
        * found, otherwise it is stopped.
        */
        TInt RunError( TInt aError );

    protected: // Data members

        enum TState
            {
            EInit,
            EGarbageCollecting
            };

        TState iState; ///< Operation state.
        TMsvId iFolderId;   /**< Gc is done only in this specific folder, 
                            * if it is not KMsvNullIndexEntryId. */
        CPushMtmUtil* iUtil; ///< Utility class. Owned.
        CMsvEntrySelection* iEntrySel; ///< Selection of entries to be deleted. Owned.
        CSIPushMsgEntry* iEntryWrapper; ///< Msg class. Owned.
        TInt iCount; ///< Count of the selection.
        TInt iCurrentIndex; ///< Index of the current entry in the selection.
    };

#endif // PUSHMTMGCOPERATION_H

// End of file.
