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
* Description:  Push Ui Data Mtm class declaration
*
*/



#ifndef PUSHMTMUIDATA_H
#define PUSHMTMUIDATA_H

// INCLUDE FILES

#include <msvstd.h>
#include <mtudcbas.h>

// CLASS DECLARATION

/**
* UI Data MTM for pushed messages.
*/
class CPushMtmUiData: public CBaseMtmUiData
    {
    public:     // construction

        /**
        * Two-phased constructor (exported factory function).
        * Leaves on failure.
        * @param aRegisteredMtmDll Registration data for the MTM DLL.
        * @return The constructed UI Data.
        */
        IMPORT_C static CPushMtmUiData* NewL( CRegisteredMtmDll& aRegisteredDll );

        /**
        * Destructor.
        */
        virtual ~CPushMtmUiData();

    private:  // construction

        /**
        * Constructor.
        * @param aRegisteredMtmDll Registration data for the MTM DLL.
        */
        CPushMtmUiData( CRegisteredMtmDll& aRegisteredDll );

    private:     // from CBaseMtmUiData

        /**
        * Get an array of bitmaps relevant to the passed context entry.
        * @param aContext Context entry to return the icons for.
        * @param aStateFlags 
        * @return Bitmap array for the given context.
        */
        const CBaseMtmUiData::CBitmapArray& ContextIcon
            ( const TMsvEntry& aContext, TInt aStateFlags ) const;

        /**
        * Check if aNewEntry can be created as a child of aContext.
        * (Yes for services in the root).
        * @param aParent The entry under which aNewEntry should be created.
        * @param aNewEntry The entry to which the operation applies.
        * @param aReasonResourceId On return, a resource string ID or 0.
        */
        TBool CanCreateEntryL( const TMsvEntry& aParent,TMsvEntry& aNewEntry,
                                            TInt& aReasonResourceId ) const;

        /**
        * Check if a delete operation is supported for aContext (yes).
        * @param aContext The entry to which the operation applies.
        * @param aReasonResourceId On return, a resource string ID or 0.
        */
        TBool CanDeleteFromEntryL
            ( const TMsvEntry& aContext, TInt& aReasonResourceId ) const;

        /**
        * Check if the service aService can be deleted (yes).
        * @param aService The entry to which the operation applies.
        * @param aReasonResourceId On return, a resource string ID or 0.
        */
        TBool CanDeleteServiceL
            ( const TMsvEntry& aService, TInt& aReasonResourceId ) const;

        /**
        * Check if aContext can be replied to (no).
        * @param aContext The entry to which the operation applies.
        * @param aReasonResourceId On return, a resource string ID or 0.
        */
        TBool CanReplyToEntryL
            ( const TMsvEntry& aContext, TInt& aReasonResourceId ) const;

        /**
        * Check if aContext can be forwarded (no).
        * @param aContext The entry to which the operation applies.
        * @param aReasonResourceId On return, a resource string ID or 0.
        */
        TBool CanForwardEntryL
            ( const TMsvEntry& aContext, TInt& aReasonResourceId ) const;

        /**
        * Check if aContext can be edited (no).
        * @param aContext The entry to which the operation applies.
        * @param aReasonResourceId On return, a resource string ID or 0.
        */
        TBool CanEditEntryL
            ( const TMsvEntry& aContext, TInt& aReasonResourceId ) const;

        /**
        * Check if aContext can be viewed (yes for messages).
        * @param aContext The entry to which the operation applies.
        * @param aReasonResourceId On return, a resource string ID or 0.
        */
        TBool CanViewEntryL
            ( const TMsvEntry& aContext, TInt& aReasonResourceId ) const;

        /**
        * Check if aContext can be opened (yes for messages).
        * @param aContext The entry to which the operation applies.
        * @param aReasonResourceId On return, a resource string ID or 0.
        */
        TBool CanOpenEntryL
            ( const TMsvEntry& aContext, TInt& aReasonResourceId ) const;

        /**
        * Check if aContext can be closed (yes for messages).
        * @param aContext The entry to which the operation applies.
        * @param aReasonResourceId On return, a resource string ID or 0.
        */
        TBool CanCloseEntryL
            ( const TMsvEntry& aContext, TInt& aReasonResourceId ) const;

        /**
        * Check if a copy to or move to operation is supported
        * for aContext (yes).
        * @param aContext The entry to which the operation applies.
        * @param aReasonResourceId On return, a resource string ID or 0.
        */
        TBool CanCopyMoveToEntryL
            ( const TMsvEntry& aContext, TInt& aReasonResourceId ) const;

        /**
        * Check if a copy from or move from operation is supported
        * for aContext (yes).
        * @param aContext The entry to which the operation applies.
        * @param aReasonResourceId On return, a resource string ID or 0.
        */
        TBool CanCopyMoveFromEntryL
            ( const TMsvEntry& aContext, TInt& aReasonResourceId ) const;

        /**
        * Check if a cancel operation is supported for aContext (no).
        * @param aContext The entry to which the operation applies.
        * @param aReasonResourceId On return, a resource string ID or 0.
        */
        TBool CanCancelL
            ( const TMsvEntry& aContext, TInt& aReasonResourceId ) const;

        /**
        * Check if an MTM-specific operation is appropriate to the entry
        * specified in aContext.
        * @param aOperationId ID of operation to query.
        * @param aContext The entry to which the operation applies.
        * @return
        * - KErrNone if operation is supported,
        * - KErrCancel if the operation is not supported.
        */
        TInt OperationSupportedL
            ( TInt aOperationId, const TMsvEntry& aContext ) const;

        /**
        * Query if the MTM supports a particular (standard) capability.
        * This function is also called by CPushMtmBaseClient::QueryCapability.
        * @param aCapability UID of capability to be queried.
        * @param aResponse Response value.
        * @return
        * - KErrNone: aCapability is a recognised value
        *   and a response is returned.
        * - KErrNotSupported: aCapability is not a recognised value.
        */
        TInt QueryCapability( TUid aCapability, TInt& aResponse ) const;

        /**
		* Since we don't support sending, we don't need to support this 
        * function.
        * @param aContext 
        * @return NULL.
        */
        HBufC* StatusTextL( const TMsvEntry& aContext ) const;

        /**
        * Load and populate icons and functions.
        */
        void PopulateArraysL();

        /**
        * Get the resource file name for this component.
        */
        void GetResourceFileName( TFileName& aFileName ) const;

    private:     // New functions

        /**
        * Create skinned icons in iIconArrays.
        */
        void CreateSkinnedBitmapsL();
    };

#endif // PUSHMTMUIDATA_H
