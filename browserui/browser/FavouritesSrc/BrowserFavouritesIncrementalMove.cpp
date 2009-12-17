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
*      Implementation of CWmlBrowserFavouritesIncrementalMove.
*      
*
*/


// INCLUDE FILES

#include <AknWaitDialog.h>
#include <favouritesdb.h>
#include <favouritesitem.h>
#include <BrowserNG.rsg>
#include "BrowserFavouritesIncrementalMove.h"
#include "BrowserFavouritesModel.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalMove::NewL
// ---------------------------------------------------------
//
CBrowserFavouritesIncrementalMove*
CBrowserFavouritesIncrementalMove::NewL
        (
        CBrowserFavouritesModel& aModel,
        CArrayFix<TInt>& aUids,
        CArrayFix<TInt>& aUnmovableUids,
        CArrayFix<TInt>& aConflictingNameUids,
        TInt aTargetFolder,
        TInt aPriority /*=CActive::EPriorityStandard*/
        )
    {
    CBrowserFavouritesIncrementalMove* incMove =
        new (ELeave) CBrowserFavouritesIncrementalMove
        (
        aModel,
        aUids,
        aUnmovableUids,
        aConflictingNameUids,
        aTargetFolder,
        aPriority
        );
    CleanupStack::PushL( incMove );
    incMove->ConstructL();
    CleanupStack::Pop();    // incMove
    return incMove;
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalMove::~CBrowserFavouritesIncrementalMove
// ---------------------------------------------------------
//
CBrowserFavouritesIncrementalMove::~CBrowserFavouritesIncrementalMove()
    {
    delete iItem;
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalMove::CBrowserFavouritesIncrementalMove
// ---------------------------------------------------------
//
CBrowserFavouritesIncrementalMove::CBrowserFavouritesIncrementalMove
    (
    CBrowserFavouritesModel& aModel,
    CArrayFix<TInt>& aUids,
    CArrayFix<TInt>& aUnmovableUids,
    CArrayFix<TInt>& aConflictingNameUids,
    TInt aTargetFolder,
    TInt aPriority
    )
: CBrowserFavouritesIncrementalOp( aModel, aPriority ),
  iUids( &aUids ),
  iUnmovableUids( &aUnmovableUids ),
  iConflictingNameUids( &aConflictingNameUids ),
  iTargetFolder( aTargetFolder ),
  iIndex( 0 )
    {
    }

// CBrowserFavouritesIncrementalMove::ConstructL
// ---------------------------------------------------------
//
void CBrowserFavouritesIncrementalMove::ConstructL()
    {
    CBrowserFavouritesIncrementalOp::ConstructL();
    iItem = CFavouritesItem::NewL();
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalMove::StepL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesIncrementalMove::StepL()
    {
    if ( iIndex < iUids->Count() )
        {
        TInt uid = iUids->At( iIndex );
        TInt err = iModel->Database().Get( uid, *iItem );
        if ( !err )
            {
            iItem->SetParentFolder( iTargetFolder );
            err = iModel->Database().Update
                            ( *iItem, uid, /*aAutoRename=*/EFalse );
            }
        switch ( err )
            {
            case KErrNone:
                {
                // All is well.
                break;
                }
            case KErrAlreadyExists:
                {
                iConflictingNameUids->AppendL( uid );
                break;
                }
            default:
                {
                iUnmovableUids->AppendL( uid );
                break;
                }
            }
        iIndex++;
        }
    return iUids->Count() - iIndex;
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalMove::CreateWaitNoteLC
// ---------------------------------------------------------
//
void CBrowserFavouritesIncrementalMove::CreateWaitNoteLC()
    {
    iWaitNote = new (ELeave) CAknWaitDialog
        ( REINTERPRET_CAST( CEikDialog**, &iWaitNote ) );
    iWaitNote->PrepareLC( R_WMLBROWSER_FAVOURITES_NOTE_MOVING );
    }

// End of File
