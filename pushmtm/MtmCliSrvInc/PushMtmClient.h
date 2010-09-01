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
* Description:  Client Mtm class declaration
*
*/



#ifndef PUSHMTMCLIENT_H
#define PUSHMTMCLIENT_H

// INCLUDE FILES

#include <e32std.h>
#include <msvstd.h>
#include <mtclbase.h>

// FORWARD DECLARATIONS

class CMsvOperation;
class CMsvEntrySelection;

// CLASS DECLARATION

/**
* Client MTM for pushed messages.
*/
class CPushMtmClient : public CBaseMtm
    {
    public: // construction

        /**
        * Two-phased constructor (exported factory function).
        * Leaves on failure.
        * @param aRegisteredMtmDll Registration data for the MTM DLL.
        * @param CMsvSession Message Server Session to be used by this client.
        * @return The constructed client.
        */
        IMPORT_C static CPushMtmClient* NewL
            ( CRegisteredMtmDll& aRegisteredMtmDll, CMsvSession& aMsvSession );

        /**
        * Destructor.
        */
        virtual ~CPushMtmClient();

    private: // construction

        /**
        * Constructor.
        * @param aRegisteredMtmDll Registration data for the MTM DLL.
        * @param CMsvSession Message Server Session to be used by this client.
        */
        CPushMtmClient
            ( CRegisteredMtmDll& aRegisteredMtmDll, CMsvSession& aMsvSession );

        /**
        * Second phase constructor. Derivde classes should call this first
        * during construction.
        */
        void ConstructL();

    private: // from CBaseMtm

        /**
        * (Not supported) Save the message.
        */
        void SaveMessageL();

        /**
        * (Not supported) Load the message.
        */
        void LoadMessageL();

        /**
        * Validate the current context.
        * @param aPartList Message parts for which validation is requested.
        * @return
        * - Bitmask of the TMsvPartList IDs for each invalid part, or
        * - KErrNone if all parts are valid.
        */
        TMsvPartList ValidateMessage( TMsvPartList aPartList );

        /**
        * (Not supported) Find text in the current context.
        * @param aTextToFind Text to find.
        * @param aPartList Message parts to search.
        * @return 0 (not supported).
        */
        TMsvPartList Find
            ( const TDesC& aTextToFind, TMsvPartList aPartList );

        /**
        * (Not supported) Create a reply to the current context.
        * @param aReplyEntryId The entry to which to assign the reply.
        * @param aPartList Message parts to include in the reply.
        * @param aCompletionStatus Completion request status.
        * @return A completed operation with status KErrNotSupported.
        */
        CMsvOperation* ReplyL
            (
            TMsvId aReplyEntryId,
            TMsvPartList aPartlist,
            TRequestStatus& aCompletionStatus
            );

        /**
        * (Not supported) Create a reply to the current context.
        * @param aReplyEntryId The entry to which to assign the forwarded
        * message.
        * @param aPartList Message parts to include in the forwarded message.
        * @param aCompletionStatus Completion request status.
        * @return A completed operation with status KErrNotSupported.
        */
        CMsvOperation* ForwardL
            (
            TMsvId aForwardEntryId,
            TMsvPartList aPartList,
            TRequestStatus& aCompletionStatus
            );

        /**
        * (Not supported) Add an addressee.
        * @param aRealAddress Real address.
        * @throw KErrNotSupported The specified function is not supported.
        */
        void AddAddresseeL( const TDesC& aRealAddress );

        /**
        * (Not supported) Add an addressee.
        * @param aRealAddress Real address.
        * @param aAlias Alias information.
        * @throw KErrNotSupported The specified function is not supported.
        */
        void AddAddresseeL( const TDesC& aRealAddress, 
                            const TDesC& aAlias );

        /**
        * (Not supported) Remove an addressee.
        * @param aIndex Index of addressee.
        */
        void RemoveAddressee( TInt aIndex );

        /**
        * Call CBaseMtmUiData::QueryCapability.
        * @param aCapability UID of capability to be queried.
        * @param aResponse Response value.
        * @return
        * - KErrNone: aCapability is a recognised value
        *   and a response is returned.
        * - KErrNotSupported: aCapability is not a recognised value.
        */
        TInt QueryCapability( TUid aCapability, TInt& aResponse );

        /**
        * Invoke synchronous functions.
        * Supported function(s):
        * - EPushMtmCmdCollectGarbage Deleting expired SIs
        * @param aFunctionId ID of the requested operation.
        * @param aSelection Selection of message entries to operate on.
        *        EPushMtmCmdFetchContent does not use it. It operates on the 
        *        current MTM context.
        *        EPushMtmCmdCollectGarbage uses it. If it is not empty, then 
        *        the first element is used as folder specifier, otherwise 
        *        if the list s empty then KMsvNullIndexEntryId is passed to 
        *        the GC operation.
        * @param aParameter Ignored.
        * @return None.
        * @throw KErrNotSupported The specified function is not supported.
        */
        void InvokeSyncFunctionL
            (
            TInt aFunctionId,
            const CMsvEntrySelection& aSelection,
            TDes8& aParameter
            );

        /**
        * Invoke asynchronous operation.
        * Supported functions:
        * - EPushMtmCmdFetchContent (SL specific!)
        * - EPushMtmCmdCollectGarbage Deleting expired SIs
        * @param aFunctionId ID of the requested operation.
        * @param aSelection Selection of message entries to operate on.
        *        EPushMtmCmdFetchContent does not use it. It operates on the 
        *        current MTM context.
        *        EPushMtmCmdCollectGarbage uses it. If it is not empty, then 
        *        the first element is used as folder specifier, otherwise 
        *        if the list s empty then KMsvNullIndexEntryId is passed to 
        *        the GC operation.
        * @param aParameter Buffer containing input and output parameters.
        * @param aCompletionStatus Completion status.
        * @return
        * - If successful, an asynchronously completing operation.
        * - If failed, a completed operation, with status set to the
        *   relevant error code.
        * @throw KErrNotSupported The specified function is not supported.
        */
        CMsvOperation* InvokeAsyncFunctionL
            (
            TInt aFunctionId,
            const CMsvEntrySelection& aSelection,
            TDes8& aParameter,
            TRequestStatus& aCompletionStatus
            );

        /**
        * Context changed callback function. It does nothing.
        */
        void ContextEntrySwitched();
    };

#endif // PUSHMTMCLIENT_H

// End of file.
