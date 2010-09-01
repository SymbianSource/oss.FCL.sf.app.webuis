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
*      Declaration of class MWmlBrowserFavouritesListboxIconHandler.
*      
*
*/


#ifndef BROWSER_FAVOURITES_LISTBOX_ICON_HANDLER_H
#define BROWSER_FAVOURITES_LISTBOX_ICON_HANDLER_H

// INCLUDE FILES

#include <e32def.h>
// #include <AknIconArray.h>

// FORWARD DECLARATION

class CAknIconArray;
class CFavouritesItem;
class CBrowserFavouritesListbox;
class CApListItemList;
class TFavouritesWapAp;
struct TBrowserFavouritesIconIndexes;

// CLASS DECLARATION

/**
* Icon handling in the WML Browser's Favourites Views listboxes.
* The problem with listbox icons is that iconarray is held by the listbox's
* itemdrawer, while their indexes are used in the model; and there is no safe
* way to enforce that the indexes used are really exist. So this class was
* introduced to make it safe: an instance is given to the listbox upon
* creation; the listbox uses the factory to create the iconarray for the
* itemdrawer, and setting icon indexes for items. This way, creating the
* iconarray and indexing to it is in the same class.
* Pure virtual.
*/

class MBrowserFavouritesListboxIconHandler
    {
    public:     // new methods

        /**
        * Create the iconarray. Derived classes please don't forget that the
        * 0th icon must be the markable listbox's mark icon.
        * @return The created iconarray. Owner is the caller.
        */
        CAknIconArray* CreateIconArrayL() const;

        /**
        * Get the icon indexes for this item (index into the array).
        * @param aItem Item to get icon indexes for.
        * @param aAccessPoints Access Points to use (if bearer info is needed).
        * May be NULL.
        * @return Icon indexes for the item.
        */
        TBrowserFavouritesIconIndexes IconIndexes
            (
            const CFavouritesItem& aItem,
          /*  const CApListItemList* aAccessPoints,*/
			CBrowserFavouritesListbox* aListbox 
            ) const;

    protected:  // new methods

        /**
        * Get item icon index for this item (index into the array).
        * Derived classes must implement this method.
        * @param aItem Item to get icon index for.
        * @return Icon index for the item.
        */
        virtual TInt ItemIconIndex( const CFavouritesItem& aItem ,CBrowserFavouritesListbox* aListbox  ) const = 0;

        /**
        * Get bearer icon index for this access point (index into the array).
        * Derived classes may override this method; current
        * implementation returns KBrowserFavouritesNoBearerIcon.
        * @param aAccessPoint Access Points to get bearer icon for.
        * @param aAccessPoints Access Points to use.
        * @return Icon index for the access point.
        */
        virtual TInt BearerIconIndex
            (
            const TFavouritesWapAp aAccessPoint,
            const CApListItemList& aAccessPoints
            ) const;
		
        /**
        * Dynamically load Icon from files (AVKON skin supported)
        * @return pointer to the created icon array
        */
		virtual CAknIconArray* LoadIconsL( ) const = 0;
    };

#endif

// End of file