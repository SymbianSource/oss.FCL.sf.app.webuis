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
*      Implementation of CBrowserFavouritesListboxView.
*      
*
*/


// INCLUDE FILES

#include "BrowserFavouritesListboxView.h"
#include "BrowserFavouritesListboxCursorObserver.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserFavouritesListboxView::CBrowserFavouritesListboxView
// ---------------------------------------------------------
//
CBrowserFavouritesListboxView::CBrowserFavouritesListboxView
( CEikListBox& aListbox ): iListbox( &aListbox )
    {
    }

// ---------------------------------------------------------
// CBrowserFavouritesListboxView::SetCursorMoveObserver
// ---------------------------------------------------------
//
void CBrowserFavouritesListboxView::SetCursorObserver
( MBrowserFavouritesListboxCursorObserver* aCursorObserver )
    {
    iCursorObserver = aCursorObserver;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListboxView::MoveCursorL
// ---------------------------------------------------------
//
void CBrowserFavouritesListboxView::MoveCursorL
( CListBoxView::TCursorMovement aCursorMovement,
    TSelectionMode aSelectionMode )
    {
    CAknColumnListBoxView::MoveCursorL( aCursorMovement, aSelectionMode );
    if ( iCursorObserver )
        {
        iCursorObserver->HandleCursorChangedL( iListbox );
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesListboxView::UpdateSelectionL
// ---------------------------------------------------------
//
void CBrowserFavouritesListboxView::UpdateSelectionL
( TSelectionMode aSelectionMode )
    {
    CAknColumnListBoxView::UpdateSelectionL( aSelectionMode );
    if ( iCursorObserver )
        {
        iCursorObserver->HandleCursorChangedL( iListbox );
        }
    }
// End of File
