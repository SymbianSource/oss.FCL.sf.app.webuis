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
*      Declaration of class CBrowserFavouritesFolderSelector.
*
*
*/


#ifndef BROWSER_FAVOURITES_FOLDER_SELECTOR_H
#define BROWSER_FAVOURITES_FOLDER_SELECTOR_H

// INCLUDE FILES

#include "BrowserFavouritesFolderListboxIconHandler.h"
#include <AknLists.h>
// FORWARD DECLARATION

// class CBrowserFavouritesFolderListbox;
class CBrowserFavouritesModel;
class MApiProvider;
class CFavouritesItemList;
class CBrowserFavouritesIconIndexArray;
// class CAknSingleGraphicPopupMenuStyleListBox;
// class MBrowserFavouritesListboxIconHandler;
// CLASS DECLARATION

class CBrowserFavouritesFolderSelectorListbox : public CAknSingleGraphicPopupMenuStyleListBox
{
public :
        /**
        * Constructor.
        * @param aIconHandler Pointer to the iconhandler class. Not owned.
        */
	CBrowserFavouritesFolderSelectorListbox( const TBrowserFavouritesFolderListboxIconHandler* aIconHandler );
        /**
        * This function is called externally when skin is changed.
        * This reloads the icons when skin change happens.
		* @param aType Type of the resource change
        */
	void HandleResourceChange( TInt aType );

protected:  
    /// Icon handler for the listbox.
    const TBrowserFavouritesFolderListboxIconHandler* iIconHandler;
};

/**
* Folder selector. Contains a popup list to display all folders in a flat list.
*/
class CBrowserFavouritesFolderSelector:
                                public CBase
    {
    public:     // construct / destruct

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aModel Model to be used for reading folder data. Not owned.
        * @param aApiProvider Api provider. Not owned.
        * @param aExcludeFolder Folder to be excluded from display.
        */
        static CBrowserFavouritesFolderSelector* NewL
            (
            CBrowserFavouritesModel& aModel,
            MApiProvider& aApiProvider,
            TInt aExcludeFolder
            );

    protected:  // construct / destruct

        /**
        * Constructor.
        * @param aModel Model to be used for reading folder data. Not owned.
        * Must remain intact while the dialog exists.
        * @param aApiProvider Api provider. Not owned.
        * @param aExcludeFolder Folder to be excluded from display.
        */
        CBrowserFavouritesFolderSelector(
            CBrowserFavouritesModel& aModel,
            MApiProvider& aApiProvider,
            TInt aExcludeFolder );

        /**
        * Destructor.
        */
        virtual ~CBrowserFavouritesFolderSelector();

        /**
        * Second phase constructor.
        */
        void ConstructL();

    public:     // new methods

        /**
        * Launch popup folder selector, return the selected folder's Uid then
        * delete the selector. Must not be on the cleanup stack when called
        * (this method pushes).
        * @return Uid of selectd folder (or KFavouritesNullUid).
        */
        TInt ExecuteLD();

    private:    // new methods

        /**
        * Get data (items and icon indexes) of folders.
        */
        void GetDataL();

        /**
        * Fill listbox.
        */
        void FillListboxL();

        /**
        * Get folders to be displayed (all except aFolder).
        * @return List of items. Pushed on the cleanup stack.
        */
        CFavouritesItemList* GetItemsLC();

        /**
        * Return icon indexes to be displayed in a list. Owner is the caller.
        * The returned list must have exactly as much items as aItems.
        * @param aItems Item list.
        * @return Icon index array.
        */
        CBrowserFavouritesIconIndexArray* GetIconIndexesLC
            ( CFavouritesItemList& aItems );

    private:    // data

        /// Used for reading folder data. Not owned.
        CBrowserFavouritesModel* iModel;
        /// Api provider. Not owned.
        MApiProvider* iApiProvider;
        /// Folder to be excluded from display.
        TInt iExcludeFolder;

        /// Real data for this listbox, array of items. Owned.
        CFavouritesItemList* iItems;
        /// Icon indexes for the items. Owned.
        CBrowserFavouritesIconIndexArray* iIconIndexes;
        /// Popup list. Owned.
        CAknPopupList* iPopupList;
        /// Listbox containing data. Owned.
        CAknSingleGraphicPopupMenuStyleListBox* iListbox;
        /// Listbox item array. Owned.
        CDesCArrayFlat* iListboxItemArray;
        /// Icon handler for the listbox.
        const TBrowserFavouritesFolderListboxIconHandler iIconHandler;

    };

#endif

// End of file