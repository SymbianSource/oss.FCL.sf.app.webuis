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
* Description:  Ui Mtm class declaration
*
*/



#ifndef PUSHMTMUI_H
#define PUSHMTMUI_H

// INCLUDE FILES

#include <mtmuibas.h>

// CLASS DECLARATION

/**
* UI MTM for pushed messages.
*/
class CPushMtmUi: public CBaseMtmUi
    {
    public:     // Constructors and destructor

        /**
        * Two-phased constructor (exported factory function).
        * Leaves on failure.
        * @param aBaseMtm The client MTM.
        * @param aRegisteredMtmDll Registration data for the MTM DLL.
        * @return The constructed UI.
        */
        IMPORT_C static CPushMtmUi* NewL
            ( CBaseMtm& aBaseMtm, CRegisteredMtmDll& aRegisteredMtmDll );

        /**
        * Destructor.
        */
        virtual ~CPushMtmUi();

    private:  // Constructors 

        /**
        * Constructor.
        * @param aBaseMtm The client MTM.
        * @param aRegisteredMtmDll Registration data for the MTM DLL.
        * @return The constructed UI.
        */
        CPushMtmUi( CBaseMtm& aBaseMtm, CRegisteredMtmDll& aRegisteredMtmDll );

    private:  // New functions

        /**
        * Utility to set a TMsvLocalOperationProgress completed.
        * @param aProgress Progress package buffer to set.
        * @param aId Id to set in the buffer.
        */
        void SetProgressSuccess
            ( TPckgBuf<TMsvLocalOperationProgress>& aProgress, TMsvId aId );

        /**
        * Create a CMsvCompletedOperation object.
        * @param aObserverStatus Observer's status.
        * @return The operation object.
        */
        CMsvOperation* CompletedOperationL( TRequestStatus& aObserverStatus );

        /**
        * Ensure that a service entry exists with which the settings can be 
        * reached from a messaging application. It creates a service entry 
        * if necessary.
        * @return None.
        */
        void EnsureServiceEntryL() const;

    private:  // Functions from base classes

        /**
        * Unsupported, leaves with KErrNotSupported.
        * @param aEntry (Unused) The data to be copied into the new entry.
        * @param aParent (Unused) The parent of the new entry.
        * @param aStatus (Unused) Completion status.
        * @return (NULL) The operation object.
        */
        CMsvOperation* CreateL(
            const TMsvEntry& aEntry,
            CMsvEntry& aParent,
            TRequestStatus& aStatus );

		// --------------------------------------------------
        // --- Functions dependent on the current context ---
		// --------------------------------------------------

        /**
        * Open the current context. If it is a KUidMsvMessageEntry then it is 
        * equivalent to ViewL. If it is a KUidMsvServiceEntry then it is 
        * equivalent to EditL. Otherwise it leaves with KErrNotSupported.
        * @param aStatus Completion status.
        * @return The operation object.
        */
        CMsvOperation* OpenL( TRequestStatus& aStatus );

        /**
        * Close the current context. It does nothing just returns a completed 
        * operation.
        * @param aStatus Completion status.
        * @return The operation object.
        */
        CMsvOperation* CloseL( TRequestStatus& aStatus );

        /**
        * Edit the current context. For KUidMsvServiceEntry it executes the 
        * "Service Settings" dialog. It returns a completed operation. For 
        * other entry types it leaves with KErrNotSupported.
        * @param aStatus Completion status.
        * @return The operation object.
        */
        CMsvOperation* EditL( TRequestStatus& aStatus );

        /**
        * View the current context. Only for KUidMsvMessageEntry. It launches the 
        * Push Viewer application embedded for SI and it lauches the viewer 
        * application embedded for SP. For other entry types it leaves 
        * with KErrNotSupported.
        * @param aStatus Completion status.
        * @return The operation object.
        */
        CMsvOperation* ViewL( TRequestStatus& aStatus );

		// --------------------------------------------------
        // -- Functions independent on the current context --
		// --------------------------------------------------

        // --- Actions upon message selections ---
        // Selections must be in same folder and all of the correct MTM type.
        // Context may change after calling these functions.

        /**
        * This function is supported only for one selected entry. If the selection 
        * contains more than one entry then it leaves with KErrNotSupported.
        * @param aStatus Completion status.
        * @param aSelection Entry selection to operate on.
        * @return The operation object.
        */
        CMsvOperation* OpenL
            ( TRequestStatus& aStatus, const CMsvEntrySelection& aSelection );

        /**
        * This function is supported only for one selected entry. If the selection 
        * contains more than one entry then it leaves with KErrNotSupported.
        * Close the first entry from the selection. It does nothing however.
        * @param aStatus Completion status.
        * @param aSelection Entry selection to operate on.
        * @return The operation object.
        */
        CMsvOperation* CloseL
            ( TRequestStatus& aStatus, const CMsvEntrySelection& aSelection );

        /**
        * This function is supported only for one selected entry. If the selection 
        * contains more than one entry then it leaves with KErrNotSupported.
        * Edit the first selected entry.
        * @param aStatus Completion status.
        * @param aSelection Entry selection to operate on.
        * @return The operation object.
        */
        CMsvOperation* EditL
            ( TRequestStatus& aStatus, const CMsvEntrySelection& aSelection );

        /**
        * This function is supported only for one selected entry. If the selection 
        * contains more than one entry then it leaves with KErrNotSupported.
        * View the first entry from the selection.
        * @param aStatus Completion status.
        * @param aSelection Entry selection to operate on.
        * @return The operation object.
        */
        CMsvOperation* ViewL
            ( TRequestStatus& aStatus, const CMsvEntrySelection& aSelection );

        /**
        * It does nothing just leaves with KErrNotSupported.
        * @param aStatus Completion status.
        * @param aSelection Entry selection to operate on.
        * @return The operation object.
        */
        CMsvOperation* CancelL
            ( TRequestStatus& aStatus, const CMsvEntrySelection& aSelection );

        // --- Deletion ---

        /**
        * It does nothing just leaves with KErrNotSupported.
        * @param aSelection Entry selection to operate on.
        * @param aStatus Completion status.
        * @return The operation object.
        */
        CMsvOperation* DeleteFromL
            ( const CMsvEntrySelection& aSelection, TRequestStatus& aStatus );

		/**
        * It does nothing just leaves with KErrNotSupported.
        * @param aSelection (Unused) Entry selection to operate on. They must
        * be children of the current context.
        * @param aStatus (Unused) Completion status.
        * @return (NULL) The operation object.
        */
        CMsvOperation* UnDeleteFromL
            ( const CMsvEntrySelection& aSelection, TRequestStatus& aStatus );

        /* *
        * It does nothing just leaves with KErrNotSupported.
        * @param (Unused) aService Service to delete.
        * @param (Unused) aStatus Completion status.
        * @return (NULL) The operation object.
        */
        CMsvOperation* DeleteServiceL
            ( const TMsvEntry& aService, TRequestStatus& aStatus );

        // --- Message responding ---

        /**
        * It does nothing just leaves with KErrNotSupported.
        * @param aDestination (Unused) The entry to which to assign the reply.
        * @param aPartList (Unused) Parts to be contained in the reply.
        * @param aStatus (Unused) Completion status.
        * @return (NULL) The operation object.
        */
        CMsvOperation* ReplyL(
            TMsvId aDestination,
            TMsvPartList aPartlist,
            TRequestStatus& aCompletionStatus );

        /**
        * It does nothing just leaves with KErrNotSupported.
        * @param aDestination (Unused) The entry to which to assign the
        * forwarded message.
        * @param aPartList (Unused) Parts to be contained in the forwarded
        * message.
        * @param aStatus (Unused) Completion status.
        * @return (NULL) The operation object.
        */
        CMsvOperation* ForwardL(
            TMsvId aDestination,
            TMsvPartList aPartList,
            TRequestStatus& aCompletionStatus );

        // --- Copy and move functions ---
        // Context should be set to folder or entry of this MTM.

        /**
        * It does nothing just leaves with KErrNotSupported.
        * @param aSelection (Unused) Entry selection to operate on. They all
        * must have the same parent.
        * @param aStatus (Unused) Completion status.
        * @return (NULL) The operation object.
        */
        CMsvOperation* CopyToL
            ( const CMsvEntrySelection& aSelection, TRequestStatus& aStatus );

        /* *
        * It does nothing just leaves with KErrNotSupported.
        * @param aSelection (Unused) Entry selection to operate on. They all
        * must have the same parent.
        * @param aStatus (Unused) Completion status.
        * @return (NULL) The operation object.
        */
        CMsvOperation* MoveToL
            ( const CMsvEntrySelection& aSelection, TRequestStatus& aStatus );

        /**
        * It does nothing just leaves with KErrNotSupported.
        * @param aSelection (Unused) Entry selection to operate on. They all
        * must have the same parent.
        * @param aTargetId (Unused) The ID of the entry to own the copies.
        * @param aStatus (Unused) Completion status.
        * @return (NULL) The operation object.
        */
        CMsvOperation* CopyFromL(
            const CMsvEntrySelection& aSelection,
            TMsvId aTargetId,
            TRequestStatus& aStatus );

        /**
        * It does nothing just leaves with KErrNotSupported.
        * @param aSelection Entry selection to operate on. They all
        * must have the same parent.
        * @param aTargetId The ID of the entry to move to.
        * @param aStatus Completion status.
        * @return The operation object.
        */
        
		CMsvOperation* MoveFromL(
            const CMsvEntrySelection& aSelection,
            TMsvId aTargetId,
            TRequestStatus& aStatus );

        /**
        * Query if the MTM supports a particular (standard) capability.
        * This feunction is also called by CPushMtmClient::QueryCapability.
        * @param aCapability UID of capability to be queried.
        * @param aResponse Response value.
        * @return
        * - KErrNone: aCapability is a recognized value
        *   and a response is returned.
        * - KErrNotSupported: aCapability is not a recognized value.
        */
        TInt QueryCapability( TUid aCapability, TInt& aResponse );

        /**
        * Invoke synchronous operation.
        * Only KMtmUiMessagingInitialisation and 
        * KMtmUiFunctionRestoreFactorySettings are supported.
        * It will leave with KErrNotSupported otherwise.
        * @param aFunctionId ID of the requested operation.
        * @param aSelection Selection of message entries to operate on.
        * @param aParameter Buffer containing input and output parameters.
        * @return None.
        * @throw KErrNotSupported The selected function is not supported.
        */
        void InvokeSyncFunctionL(
            TInt aFunctionId,
            const CMsvEntrySelection& aSelection,
            TDes8& aParameter );

		/**
        * Invoke asynchronous operation.
        * Supported operations:
        * - KMtmUiFunctionMessageInfo  Launch message info dialog.
        * - EPushMtmCmdLoadService     Handle the URL in SI (or SL).
        * - EPushMtmCmdViewService     Render the content of the SP.
        * - otherwise it calls CBaseMtmUi::InvokeAsyncFunctionL
        * @param aFunctionId ID of the requested operation.
        * @param aSelection Selection of message entries to operate on.
        * @param aCompletionStatus Completion status.
        * @param aParameter Buffer containing input and output parameters.
        * @return
        * - If successful, an asynchronously completing operation.
        * - If failed, a completed operation, with status set to the
        *   relevant error code.
        * @throw KErrNotSupported The selected function is not supported.
        */
        CMsvOperation* InvokeAsyncFunctionL(
            TInt aFunctionId,
            const CMsvEntrySelection& aSelection,
            TRequestStatus& aCompletionStatus,
            TDes8& aParameter );

        /**
        * Get the resource file name for this MTM UI.
        */
        void GetResourceFileName( TFileName& aFileName ) const;
    private:
        TInt   iPushSLEnabled;
    };

#endif // PUSHMTMUI_H

// End of File
