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
*      Inline methods for CWmlBrowserFavouritesSelectionState.
*      
*
*/


#ifndef BROWSER_FAVOURITES_SELECTION_STATE_INL
#define BROWSER_FAVOURITES_SELECTION_STATE_INL

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::TBrowserFavouritesSelectionState
// ---------------------------------------------------------
//
TBrowserFavouritesSelectionState::TBrowserFavouritesSelectionState()
    {
    iCurrentFlags = 0;
    iMarkFlags = 0;
    iMarkedItemCount = 0;
    iMarkedFolderCount = 0;
    iVisibleItemCount = 0;
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::IsEmpty
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::IsEmpty() const
    {
    return !(iCurrentFlags & EAny);
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::AnyMarked
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::AnyMarked() const
    {
    return iMarkFlags & EAny;
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::CurrentIsMarked
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::CurrentIsMarked() const
    {
    return iMarkFlags & ECurrent;
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::AllIsMarked
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::AllIsMarked() const
    {
    return iMarkFlags & EAll;
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::NoneIsMarked
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::NoneIsMarked() const
    {
    return !AnyMarked();
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::NoItemsMarked
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::NoItemsMarked() const
    {
    return !(iMarkFlags & EItem);
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::NoFoldersMarked
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::NoFoldersMarked() const
    {
    return !(iMarkFlags & EFolder);
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::MarkedCount
// ---------------------------------------------------------
//
TInt TBrowserFavouritesSelectionState::MarkedCount() const
    {
    return iMarkedItemCount + iMarkedFolderCount;
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::CurrentIsItem
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::CurrentIsItem() const
    {
    return iCurrentFlags & EItem;
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::CurrentIsFolder
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::CurrentIsFolder() const
    {
    return iCurrentFlags & EFolder;
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::AnyDeletable
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::AnyDeletable() const
    {
    return (EPlainItem | EFolder | ELastVisited) &
        (AnyMarked() ? iMarkFlags : iCurrentFlags);
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::AnyPlainItem
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::AnyPlainItem() const
    {
    return EPlainItem & (AnyMarked() ? iMarkFlags : iCurrentFlags);
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::CurrentIsSpecial
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::CurrentIsSpecial() const
    {
    return iCurrentFlags & (EHomepage | EStartPage | ELastVisited);
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::AnyMarkedSpecial
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::AnyMarkedSpecial() const
    {
    return iMarkFlags & (EHomepage | EStartPage | ELastVisited);
    }

// ---------------------------------------------------------
// TBrowserFavouritesSelectionState::CurrentIsStartPage
// ---------------------------------------------------------
//
TBool TBrowserFavouritesSelectionState::CurrentIsStartPage() const
    {
    return iCurrentFlags & EStartPage;
    }
#endif

// End of file