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
*       Declaration of class CWmlBrowserFavouritesListboxItemDrawer.
*       
*
*/


#ifndef BROWSER_FAVOURITES_LISTBOX_ITEM_DRAWER_H
#define BROWSER_FAVOURITES_LISTBOX_ITEM_DRAWER_H

//  INCLUDES

#include <eikclb.h>

// CLASS DECLARATION

/**
* Custom item drawer, to support the "no-folders-marked" feature.
*/
class CBrowserFavouritesListboxItemDrawer: public CColumnListBoxItemDrawer
    {
    public:     // construction

        /**
        * Constructor.
        * @param aTextListBoxModel Listbox model.
        * @param aFont Font.
        * @param aColumnData Listbox column data.
        */
        CBrowserFavouritesListboxItemDrawer
            (
            MTextListBoxModel* aTextListBoxModel,
            const CFont* aFont,
            CColumnListBoxData* aColumnData
            );

    public:     // from CColumnListBoxItemDrawer

        /**
        * Get item properties for a listbox item. Same as base class behaviour,
        * but marking is masked out for folders.
        * @param aItemIndex Index of listbox item to get properties of.
        * @return Properties of listbox item.
        */
        TListItemProperties Properties( TInt aItemIndex ) const;

    };

#endif

// End of File
