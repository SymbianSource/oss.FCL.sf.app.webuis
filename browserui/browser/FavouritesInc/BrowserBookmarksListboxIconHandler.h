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
*       Declaration of class TBrowserBookmarksListboxIconHandler.
*       
*
*/


#ifndef BROWSER_BOOKMARKS_LISTBOX_ICON_HANDLER_H
#define BROWSER_BOOKMARKS_LISTBOX_ICON_HANDLER_H

// INCLUDE FILES

#include "BrowserFavouritesListboxIconHandler.h"

// CLASS DECLARATION

/**
* Icon handling in the WML Browser's Bookmark View listbox.
* The following icons are presented:
*   - Mark icon
*   - Folder icon
*   - Homepage icon
*   - Last Visited Page icon
*   - Bookmark icon
*   - CSD bearer bookmark icon
*   - HSCSD bearer bookmark icon
*   - GPRS bearer bookmark icon
*   - SMS bearer bookmark icon
*/

class TBrowserBookmarksListboxIconHandler: 
                                public MBrowserFavouritesListboxIconHandler
    {
    protected:  // from MBrowserFavouritesListboxIconHandler

        /**
        * Get item icon index for this item (index into the array).
        * @param aItem Item to get icon index for.
        * @return Icon index for the item.
        */
        TInt ItemIconIndex( const CFavouritesItem& aItem,CBrowserFavouritesListbox* aListbox  ) const;

        /**
        * Get bearer icon index for this access point (index into the array).
        * @param aAccessPoint Access Points to get bearer icon for.
        * @param aAccessPoints Access Points to use.
        * @return Icon index for the access point.
        */
        TInt BearerIconIndex
            (
            const TFavouritesWapAp aAccessPoint,
            const CApListItemList& aAccessPoints
            ) const;
        /**
        * Dynamically load Icon from files (AVKON skin supported)
        * @return pointer to the created icon array
        */
		CAknIconArray* LoadIconsL() const;
    };

#endif

// End of file