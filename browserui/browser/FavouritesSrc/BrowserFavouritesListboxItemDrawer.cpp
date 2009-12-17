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
#include <favouritesitemlist.h>
#include <favouritesitem.h>

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

    // SEARCH CUSTOM:  Updated below for Italics in bookmarks view    
    // Selection of Folders is allowed, so no need to set it as Hidden.
    // if ( STATIC_CAST( CBrowserFavouritesListboxModel*, iModel ) ->
    //    Items()->At( aItemIndex )->IsFolder() )
    //    {
    //    // If this is a folder, set "hidden selection" true.
    //    properties.SetHiddenSelection( ETrue );
    //    }

        
     //Better solution would be adding italics properties to CFavouritesItem, but it requires changes in FavouritesEngine API
     CFavouritesItem* fi = STATIC_CAST( CBrowserFavouritesListboxModel*, iModel ) ->
        Items()->At( aItemIndex );
    if ( fi->ContextId() == KFavouritesServiceContextId )
    	{
    	properties.SetItalics(ETrue);
    	}
    return properties;
    }

//  End of File
