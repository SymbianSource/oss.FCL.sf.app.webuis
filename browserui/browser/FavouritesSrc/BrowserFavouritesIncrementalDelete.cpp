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
*      Implementation of CWmlBrowserFavouritesIncrementalDelete.
*      
*
*/


// INCLUDE FILES

#include <AknWaitDialog.h>
#include <FavouritesDb.h>
#include <BrowserNG.rsg>
#include "BrowserFavouritesIncrementalDelete.h"
#include "BrowserFavouritesModel.h"

// CONSTANTS

/// Granularity of the folder-contents array.
LOCAL_D const TInt KGranularity = 4;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalDelete::NewL
// ---------------------------------------------------------
//
CBrowserFavouritesIncrementalDelete*
CBrowserFavouritesIncrementalDelete::NewL
        (
        CBrowserFavouritesModel& aModel,
        CArrayFix<TInt>& aUids,
        CArrayFix<TInt>& aNotDeletedUids,
        TInt aPriority /*=CActive::EPriorityStandard*/
        )
    {
    CBrowserFavouritesIncrementalDelete* incDel =
        new (ELeave) CBrowserFavouritesIncrementalDelete
        ( aModel, aUids, aNotDeletedUids, aPriority );
    CleanupStack::PushL( incDel );
    incDel->ConstructL();
    CleanupStack::Pop();    // incDel
    return incDel;
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalDelete::~CBrowserFavouritesIncrementalDelete
// ---------------------------------------------------------
//
CBrowserFavouritesIncrementalDelete::~CBrowserFavouritesIncrementalDelete()
    {
    delete iFolderContents;
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalDelete::CBrowserFavouritesIncrementalDelete
// ---------------------------------------------------------
//
CBrowserFavouritesIncrementalDelete::CBrowserFavouritesIncrementalDelete
    (
    CBrowserFavouritesModel& aModel,
    CArrayFix<TInt>& aUids,
    CArrayFix<TInt>& aNotDeletedUids,
    TInt aPriority
    )
: CBrowserFavouritesIncrementalOp( aModel, aPriority ), iUids( &aUids ),
  iNotDeletedUids( &aNotDeletedUids ), iFolderPending( EFalse ), iIndex( 0 )
    {
    }

// CBrowserFavouritesIncrementalDelete::ConstructL
// ---------------------------------------------------------
//
void CBrowserFavouritesIncrementalDelete::ConstructL()
    {
    CBrowserFavouritesIncrementalOp::ConstructL();
    iFolderContents = new (ELeave) CArrayFixFlat<TInt>( KGranularity );
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalDelete::StepL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesIncrementalDelete::StepL()
    {
    if ( iFolderContents->Count() )
        {
        // Deleting one item from folder.
        (void)iModel->Database().Delete( iFolderContents->At( 0 ) );
        iFolderContents->Delete( 0 );
        }
    else if ( iIndex < iUids->Count() )
        {
        // Still have item in the main list to delete.
        TInt uid = iUids->At( iIndex );
        // Check if it is a non-empty folder: get contents.
        //
        // The variable iFolderPending is a performance optimization: avoid
        // querying contents of a folder twice. (If iFolderPending == ETrue,
        // and we reach this point, that means that the last item in a folder
        // was deleted in the last StepL. The contents of this folder were
        // already queried).
        if ( !iFolderPending )
            {
            // The contents of this folder were not queried yet: do it now.
            iModel->Database().GetUids( *iFolderContents, uid );
            }
        if ( iFolderContents->Count() )
            {
            // Folder is not empty, do nothing now. iIndex is not incremented
            // (deletion of this folder becomes pending) and next StepL will
            // begin deleting the contents.
            iFolderPending = ETrue;
            }
        else
            {
            // Deleting an empty (pending) folder or an ordinary item.
            iFolderPending = EFalse;
            if ( iModel->Database().Delete( uid ) != KErrNone )
                {
                // Error deleting.
                iNotDeletedUids->AppendL( uid );
                }
            iIndex++;
            }
        }
    return iUids->Count() - iIndex;
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalDelete::CreateWaitNoteLC
// ---------------------------------------------------------
//
void CBrowserFavouritesIncrementalDelete::CreateWaitNoteLC()
    {
    iWaitNote = new (ELeave) CAknWaitDialog
        ( REINTERPRET_CAST( CEikDialog**, &iWaitNote ) );
    iWaitNote->PrepareLC( R_WMLBROWSER_FAVOURITES_NOTE_DELETING );
    }

// End of File
