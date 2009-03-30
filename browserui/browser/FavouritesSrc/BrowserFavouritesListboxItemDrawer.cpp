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
*      Implementation of CWmlBrowserFavouritesListboxItemDrawer.
*      
*
*/


// INCLUDE FILES
#include "BrowserFavouritesListboxItemDrawer.h"
#include "BrowserFavouritesListboxModel.h"
#include <FavouritesItemList.h>
#include <FavouritesItem.h>

//================== MEMBER FUNCTIONS =========================================

//-----------------------------------------------------------------------------
// CBrowserFavouritesListboxItemDrawer::CBrowserFavouritesListboxItemDrawer
//-----------------------------------------------------------------------------
//
CBrowserFavouritesListboxItemDrawer::CBrowserFavouritesListboxItemDrawer
        (
        MTextListBoxModel* aTextListBoxModel,
        const CFont* aFont,
        CColumnListBoxData* aColumnData
        )
    : CColumnListBoxItemDrawer( aTextListBoxModel, aFont, aColumnData )
    {
    }

//-----------------------------------------------------------------------------
// CBrowserFavouritesListboxItemDrawer::Properties
//-----------------------------------------------------------------------------
//
TListItemProperties CBrowserFavouritesListboxItemDrawer::Properties( 
                                                        TInt aItemIndex ) const
    {
    CAknListBoxFilterItems *filter =
        STATIC_CAST(CAknFilteredTextListBoxModel*,iModel)->Filter();
    if ( filter )
        {
        aItemIndex = filter->FilteredItemIndex( aItemIndex );
        }
    TListItemProperties properties =
        CColumnListBoxItemDrawer::Properties( aItemIndex );
    if ( STATIC_CAST( CBrowserFavouritesListboxModel*, iModel ) ->
        Items()->At( aItemIndex )->IsFolder() )
        {
        // If this is a folder, set "hidden selection" true.
        properties.SetHiddenSelection( ETrue );
        }
    return properties;
    }

//  End of File
