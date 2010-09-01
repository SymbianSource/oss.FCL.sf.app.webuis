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
*       Declaration of class CWmlBrowserFavouritesIconIndexArray and
*                     class TWmlBrowserFavouritesIconIndexes.
*                     
*
*/


#ifndef BROWSER_FAVOURITES_ICON_INDEX_ARRAY_H
#define BROWSER_FAVOURITES_ICON_INDEX_ARRAY_H

// INCLUDE FILES

#include <e32base.h>

// CONSTANTS

/// No bearer icon for this item.
LOCAL_C const TInt KBrowserFavouritesNoBearerIcon = -1;

// CLASS DECLARATION

/**
* Icon indexes for one listbox item. There are two icons, one in the left
* (item/folder/homepage/last visited); and a bearer icon on the right.
* If the WAP AP is not a concrete value, there is no bearer icon.
*/
struct TBrowserFavouritesIconIndexes
    {
    public:     // data

        /**
        * Item icon index (item/folder/last visited/homepage).
        */
        TInt iItemIcon;
        /**
        * Bearer icon index; can be KBrowserFavouritesNoBearerIcon.
        */
        TInt iBearerIcon;
    };

/**
* An array of icon indexes for the listbox.
*/
class CBrowserFavouritesIconIndexArray:
                        public CArrayFixFlat<TBrowserFavouritesIconIndexes>
    {
    public:     // construct / destruct

        /**
        * Constructor (no code).
        */
        inline CBrowserFavouritesIconIndexArray( TInt aGranularity );
    };

#include "BrowserFavouritesIconIndexArray.inl"

#endif

// End of file