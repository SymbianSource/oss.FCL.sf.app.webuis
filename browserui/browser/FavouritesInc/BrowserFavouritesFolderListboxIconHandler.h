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
*      Declaration of class TBrowserFavouritesFolderListboxIconHandler.
*
*
*/


#ifndef BROWSER_FAVOURITES_FOLDER_LISTBOX_ICON_HANDLER_H
#define BROWSER_FAVOURITES_FOLDER_LISTBOX_ICON_HANDLER_H

// INCLUDE FILES

#include "BrowserFavouritesListboxIconHandler.h"

// CLASS DECLARATION

/**
* Icon handling in the WML Browser's Favourites Views Folder Select listboxes.
* This one presents only folder icons:
*   - Mark icon (not used but safer this way)
*   - Folder icon
*   - Root folder icon
*/

class TBrowserFavouritesFolderListboxIconHandler:
                                public MBrowserFavouritesListboxIconHandler
    {
    public:
            /**
        * Get the icon indexes for this item (index into the array).
        * @param aItem Item to get icon indexes for.
        * @param aAccessPoints Access Points to use (if bearer info is needed).
        * May be NULL.
        * @return Icon indexes for the item.
        */
        TBrowserFavouritesIconIndexes IconIndexes
            (
            const CFavouritesItem& aItem
          /*  const CApListItemList* aAccessPoints,*/
            ) const;
            
    protected:  // from MBrowserFavouritesListboxIconHandler

        /**
        * Get resource id for the iconarray.
        * @return The resource id for the iconarray.
        */
        TInt IconArrayResourceId() const;

        /**
        * Get item icon index for this item (index into the array).
        * @param aItem Item to get icon index for.
        * @return Icon index for the item.
        */
        TInt ItemIconIndex( const CFavouritesItem& aItem ,CBrowserFavouritesListbox* aListbox  ) const;

        TInt ItemIconIndex( const CFavouritesItem& aItem ) const;


        /**
        * Dynamically load Icon from files (AVKON skin supported)
        * @return pointer to the created icon array
        */
		CAknIconArray* LoadIconsL() const;
    };

#endif

// End of file