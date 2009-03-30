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
* Description: 
*       Declaration of class CWmlBrowserFavouritesIncrementalDelete.
*       
*
*/


#ifndef BROWSER_FAVOURITES_INCREMENTAL_DELETE
#define BROWSER_FAVOURITES_INCREMENTAL_DELETE

// INCLUDE FILES

#include "BrowserFavouritesIncrementalOp.h"

// CLASS DECLARATION

/**
* Incremental delete operation. One step deletes one bookmark. Before deleting
* a folder, its contents are deleted manually one-by-one. (Using DeleteL of
* Bookmark Engine for a full folder would delete the folder along with its
* contents in one call. This can be too long if the folder contains many items.
* Therefore this operation deletes the folder contents manually before deleting
* the (already emptied) folder, to keep one StepL short.)
*/
class CBrowserFavouritesIncrementalDelete:
                                    public CBrowserFavouritesIncrementalOp
    {
    public:     // construct / destruct

        /**
        * Two phased constructor. Leaves on failure.
        * @param aModel Data model.
        * @param aUids Uids to delete. Ownership not taken, must remain
        * intact during operation.
        * @param aNotDeletedUids Uids of items which could not be deleted
        * will be appended to this list. Ownership not taken, must remain
        * intact during operation.
        * @param aPriority Active Object priority.
        * @return The constructed incremental delete operation.
        */
        static CBrowserFavouritesIncrementalDelete* NewL
            (
            CBrowserFavouritesModel& aModel,
            CArrayFix<TInt>& aUids,
            CArrayFix<TInt>& aNotDeletedUids,
            TInt aPriority = CActive::EPriorityStandard
            );

        /**
        * Destructor.
        */
        virtual ~CBrowserFavouritesIncrementalDelete();

    protected:  // construct / destruct

        /**
        * Constructor.
        * @param aModel Data model.
        * @param aUids Uids to delete. Ownership not taken, must remain
        * intact during operation.
        * @param aNotDeletedUids Uids of items which could not be deleted
        * will be appended to this list. Ownership not taken, must remain
        * intact during operation.
        * @param aPriority Active Object priority.
        */
        CBrowserFavouritesIncrementalDelete
            (
            CBrowserFavouritesModel& aModel,
            CArrayFix<TInt>& aUids,
            CArrayFix<TInt>& aNotDeletedUids,
            TInt aPriority
            );

        /**
        * Second phase constructor.
        */
        void ConstructL();

    protected:  // from WmlBrowserFavouritesIncrementalOp

        /**
        * Perform next step (delete one item).
        * @return Number of steps to come (0 if done).
        */
        TInt StepL();

        /**
        * Create and prepare the wait note.
        */
        void CreateWaitNoteLC();

    private:    // data

        /// Uids to delete. Not owned.
        CArrayFix<TInt>* iUids;
        /// Not deleted uids. Not owned.
        CArrayFix<TInt>* iNotDeletedUids;
        /// Uids from folder being deleted. Owned.
        CArrayFix<TInt>* iFolderContents;
        /// ETrue if deleting folder contents (the folder itself comes next).
        TBool iFolderPending;
        /// Index of item being deleted.
        TInt iIndex;

    };

#endif
// End of File
