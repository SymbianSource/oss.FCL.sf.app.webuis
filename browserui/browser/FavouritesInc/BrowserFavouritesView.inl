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
*      Inline methods for CBrowserFavouritesView.
*      
*
*/


#ifndef BROWSER_FAVOURITES_VIEW_INL
#define BROWSER_FAVOURITES_VIEW_INL

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserFavouritesView::Model
// ---------------------------------------------------------
//
CBrowserFavouritesModel& CBrowserFavouritesView::Model() const
    {
    return *iModel;
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::Container
// ---------------------------------------------------------
//
CBrowserFavouritesContainer* CBrowserFavouritesView::Container() const
    {
    return iContainer;
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::CurrentFolder
// ---------------------------------------------------------
//
TInt CBrowserFavouritesView::CurrentFolder() const
    {
    return iCurrentFolder;
    }

#endif

// End of file