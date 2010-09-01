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
*      Declaration of class CWmlBrowserFavouritesSelectionState.
*      
*
*/


#ifndef BROWSER_FAVOURITES_SELECTION_STATE_H
#define BROWSER_FAVOURITES_SELECTION_STATE_H

//  INCLUDES

#include <e32def.h>

// CLASS DECLARATION

/**
* Selection state in the Favourites Views.
*/
struct TBrowserFavouritesSelectionState
    {
    public:     // types

        /**
        * Flags for storing listbox highlighted and marked item state. They
        * have has slightly different meaning for the two flags. For marking,
        * most of the flags mean that the marking *includes* that; for the
        * current, most of the flags mean that the current *is* that.
        * For example, EHomepage in the marking flags means "Marking includes
        * Homepage", while in current flag means "Current is the Homepage".
        */
        enum
            {
            ENone = 0x0,            ///< No items.
            EAny = 0x1,             ///< At least one (marking only).
            EAll = 0x2,             ///< All (marking only).
            ECurrent = 0x4,         ///< Current (marking only).
            EHomepage = 0x8,        ///< Homepage.
            EStartPage = 0x10,      ///< Start Page.
            ELastVisited = 0x20,    ///< Last Visited Page.
            EItem = 0x40,           ///< Any kind of item (incl. specials).
            EPlainItem = 0x80,      ///< Plain (non-special) item.
            EFolder = 0x100         ///< Folder.
            };

    public:     // constructor

        /**
        * Constructor. Initialize with 0.
        */
        inline TBrowserFavouritesSelectionState();

    public:     // new methods

        /**
        * Return ETrue if listbox is empty.
        * @return ETrue if listbox is empty.
        */
        inline TBool IsEmpty() const;

        /**
        * Return ETrue if any item is marked.
        * @return ETrue if any item is marked.
        */
        inline TBool AnyMarked() const;

        /**
        * Return ETrue if the current item is marked.
        * @return ETrue if the current item is marked.
        */
        inline TBool CurrentIsMarked() const;

        /**
        * Return ETrue if the all items are marked.
        * @return ETrue if the all items are marked.
        */
        inline TBool AllIsMarked() const;

        /**
        * Return ETrue if the no items are marked.
        * @return ETrue if the no items are marked.
        */
        inline TBool NoneIsMarked() const;

        /**
        * Return ETrue if the no items are marked.
        * @return ETrue if the no items are marked.
        */
        inline TBool NoItemsMarked() const;

        /**
        * Return ETrue if the no folders are marked.
        * @return ETrue if the no folders are marked.
        */
        inline TBool NoFoldersMarked() const;

        /**
        * Count marked items (items + folders).
        * @return Count of marked items (items + folders).
        */
        inline TInt MarkedCount() const;

        /**
        * Return ETrue if the current item is item.
        * @return ETrue if the current item is item.
        */
        inline TBool CurrentIsItem() const;

        /**
        * Return ETrue if the current item is folder.
        * @return ETrue if the current item is folder.
        */
        inline TBool CurrentIsFolder() const;

        /**
        * Return ETrue if selection allows deleting:
        *   - Marked items contain something deletable, or
        *   - No marking, highlighted item is deletable.
        * @return ETrue if selection allows deleting.
        */
        inline TBool AnyDeletable() const;

        /**
        * Return ETrue if selection contains at least one non-special item:
        *   - Marked items contain one non-special item, or
        *   - No marking, highlighted item is non-special item.
        * @return ETrue if selection allows deleting.
        */
        inline TBool AnyPlainItem() const;

        /**
        * Return ETrue if current item is Homepage, Start Page or Last Visited.
        * @return ETrue if current is Homepage, Start Page or Last Visited.
        */
        inline TBool CurrentIsSpecial() const;

        /**
        * Return ETrue if Homepage, Start Page and/or Last Visited is marked.
        * @return ETrue if Homepage, Start Page and/or Last Visited is marked.
        */
        inline TBool AnyMarkedSpecial() const;

		/**
        * Return ETrue if the current item is start page.
        * @return ETrue if the current item is start page.
        */
        inline TBool CurrentIsStartPage() const;

    public:     // data

        TInt iCurrentFlags;         ///< Flags showing the highlighted one.
        TInt iMarkFlags;            ///< Flags showing marking state.
        TInt iMarkedItemCount;      ///< Count of marked items.
        TInt iMarkedFolderCount;    ///< Count of marked folders.
        TInt iVisibleItemCount;     ///< Count of visible items (not folders).
        TInt iVisibleFolderCount;     ///< Count of visible folders (not items).
    };

#include "BrowserFavouritesSelectionState.inl"

#endif

// End of File
