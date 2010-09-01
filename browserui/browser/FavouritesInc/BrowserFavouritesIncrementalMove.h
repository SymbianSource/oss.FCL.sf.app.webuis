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
*      Declaration of class CWmlBrowserFavouritesIncrementalMove.
*      
*
*/


#ifndef BROWSER_FAVOURITES_INCREMENTAL_MOVE
#define BROWSER_FAVOURITES_INCREMENTAL_MOVE

// INCLUDE FILES

#include "BrowserFavouritesIncrementalOp.h"

// FORWARD DECLARATION

class CFavouritesItem;

// CLASS DECLARATION

/**
* Incremental move operation.
*/
class CBrowserFavouritesIncrementalMove:
                                    public CBrowserFavouritesIncrementalOp
    {
    public:     // construct / destruct

        /**
        * Two phased constructor. Leaves on failure.
        * @param aModel Data model.
        * @param aUids Uids to move. Ownership not taken, must remain
        * intact during operation.
        * @param aUnmovableUids Uids of items which could not be moved (due to
        * any other reason than name conflict) will be appended to this list.
        * Ownership not taken, must remain intact during operation.
        * @param aConflictingNameUids Uids of items which could not be moved
        * (due to name conflict) will be appended to this list.
        * Ownership not taken, must remain intact during operation.
        * @param aPriority Active Object priority.
        * @param aTargetFolder Uid of the target folder. Nothing is checked.
        * @return The constructed incremental move operation.
        */
        static CBrowserFavouritesIncrementalMove* NewL
            (
            CBrowserFavouritesModel& aModel,
            CArrayFix<TInt>& aUids,
            CArrayFix<TInt>& aUnmovableUids,
            CArrayFix<TInt>& aConflictingNameUids,
            TInt aTargetFolder,
            TInt aPriority = CActive::EPriorityStandard
            );

        /**
        * Destructor.
        */
        virtual ~CBrowserFavouritesIncrementalMove();

    protected:  // construct / destruct

        /**
        * Constructor.
        * @param aModel Data model.
        * @param aUids Uids to move. Ownership not taken, must remain
        * intact during operation.
        * @param aUnmovableUids Uids of items which could not be moved (due to
        * any other reason than name conflict) will be appended to this list.
        * Ownership not taken, must remain intact during operation.
        * @param aConflictingNameUids Uids of items which could not be moved
        * (due to name conflict) will be appended to this list.
        * Ownership not taken, must remain intact during operation.
        * @param aTargetFolder Uid of the target folder. Nothing is checked.
        * @param aPriority Active Object priority.
        */
        CBrowserFavouritesIncrementalMove
            (
            CBrowserFavouritesModel& aModel,
            CArrayFix<TInt>& aUids,
            CArrayFix<TInt>& aUnmovableUids,
            CArrayFix<TInt>& aConflictingNameUids,
            TInt aTargetFolder,
            TInt aPriority = CActive::EPriorityStandard
            );

        /**
        * Second phase constructor.
        */
        void ConstructL();

    protected:  // from WmlBrowserFavouritesIncrementalOp

        /**
        * Perform next step (move one item).
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
        /// Unmovable uids. Not owned.
        CArrayFix<TInt>* iUnmovableUids;
        /// Conflicting name uids. Not owned.
        CArrayFix<TInt>* iConflictingNameUids;
        /// Target folder Uid.
        TInt iTargetFolder;
        /// Favourites item used in database updates. Owned.
        CFavouritesItem* iItem;
        /// Index of item being moved.
        TInt iIndex;

    };

#endif
// End of File
