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
*      Inline methods for CWmlBrowserFavouritesContainer
*      
*
*/


#ifndef BROWSER_FAVOURITES_CONTAINER_INL
#define BROWSER_FAVOURITES_CONTAINER_INL

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserFavouritesContainer::View
// ---------------------------------------------------------

CBrowserFavouritesView& CBrowserFavouritesContainer::View() const
    {
    return *iView;
    }

// ---------------------------------------------------------
// CBrowserFavouritesContainer::Listbox
// ---------------------------------------------------------
//
CBrowserFavouritesListbox* CBrowserFavouritesContainer::Listbox() const
    {
    return iListbox;
    }


#endif

// End of File
