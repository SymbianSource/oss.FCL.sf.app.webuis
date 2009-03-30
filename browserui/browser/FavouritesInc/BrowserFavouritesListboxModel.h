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
*      Declaration of class CBrowserFavouritesListboxModel.
*      
*
*/


#ifndef BROWSER_FAVOURITES_LISTBOX_MODEL_H
#define BROWSER_FAVOURITES_LISTBOX_MODEL_H

// INCLUDE FILES
// #include <e32base.h>
// #include <eiktxlbm.h>
#include <calslbs.h>
#include "Browser.hrh"

// CONSTANTS

/**
* Buffer size for formatting listbox text. Maximum item name
* plus listbox internals (tabulators for icons etc.) must fit into it.
* The format is "<icon index max 3 char>\t<name>\t<mark 1 char>\t",
* so the added length is 7.
*/
const TInt KMaxFavouritesItemListboxName =
                        KFavouritesMaxBookmarkNameDefine + 7;

// FORWARD DECLARATION

class CFavouritesItemList;
class CBrowserFavouritesListboxModel;
class CBrowserFavouritesIconIndexArray;
// class CAknListBoxFilterItems;
// class CAknSearchField;
// class CEikListBox;

// CLASS DECLARATION

/**
* Item text array for the favourites view listbox; converts item data
* to format what the listbox needs. (This means the returned text includes
* listbox internals (tabulators, icon numbers).
*/
class TBrowserFavouritesListboxItemTextArray: public MDesCArray
    {
    public:     // from MDesCArray

        /**
        * Get number of items in the array.
        * @return Number of items.
        */
        TInt MdcaCount() const;

        /**
        * Text to be displayed for this item.
        * @param aIndex Index of item.
        * @return TPtrC for this item text.
        */
        TPtrC MdcaPoint( TInt aIndex ) const;

        /**
        * The model uses this class.
        */
        friend class CBrowserFavouritesListboxModel;

    private:    // data

        /**
        * Pointer to real data. Not owned.
        */
        const CFavouritesItemList* iItems;

        /**
        * Icon indexes. Not owned.
        */
        const CBrowserFavouritesIconIndexArray* iIconIndexes;

        /**
        * MdcaPoint() cannot leave, so this buffer is allocated
        * to allow formatting. Fortunately, doesn't have to be very
        * large.
        * We have to use MUTABLE CAST, as MdcaPoint is const (odd enough),
        * so wouldn't allow formatting the text in a member...
        */
        __MUTABLE TBuf<KMaxFavouritesItemListboxName> iBuf;
    };

/**
* Item text array for the favourites view listbox filtering; converts item data
* to format what the filter needs. (This means the returned text is what the
* user sees, and does not include listbox internals (tabulators, icon numbers).
*/
class TBrowserFavouritesListboxFilterTextArray: public MDesCArray
    {
    public:     // from MDesCArray

        /**
        * Get number of items in the array.
        * @return Number of items.
        */
        TInt MdcaCount() const;

        /**
        * Text to be displayed for this item.
        * @param aIndex Index of item.
        * @return TPtrC for this item text.
        */
        TPtrC MdcaPoint( TInt aIndex ) const;

        /**
        * The model uses this class.
        */
        friend class CBrowserFavouritesListboxModel;

    private:    // data

        /**
        * Pointer to real data. Not owned.
        */
        const CFavouritesItemList* iItems;

    };

/**
* Model for the bookmark item listbox. Contains an array of
* CBrowserFavouritesItem pointers; items are owned.
*/
class CBrowserFavouritesListboxModel: public CAknFilteredTextListBoxModel
    {
    public:     // Construct / destruct

        /**
        * Two-phase constructor. Leaves on failure.
        * The two arrays passed must have the same number of items.
        * @param aItems Itemarray; not owned.
        * @param aIconIndexes Icon index array, not owned.
        */
        static CBrowserFavouritesListboxModel* NewL
            (
            const CFavouritesItemList& aItems,
            const CBrowserFavouritesIconIndexArray& aIconIndexes
            );

        /**
        * Overcoming base class bugs. Need to set itemarray after the whole
        * listbox has been constructed; as the listbox construction
        * overwrites itemarray to NULL. To be called as part of the
        * construction ("third phase constructor"), AFTER the listbox's
        * ConstructL.
        */
        void SetItemArray();

        /**
        * Destructor.
        */
        virtual ~CBrowserFavouritesListboxModel();

    public:     // new methods

        /**
        * Set new data into the model.
        * The two arrays passed must have the same number of items.
        * @param aItems Itemarray; not owned.
        * @param aIconIndexes Icon index array, not owned.
        */
        void SetData
            (
            const CFavouritesItemList& aItems,
            const CBrowserFavouritesIconIndexArray& aIconIndexes
            );

        /**
        * Returns pointer to item list.
        * @return Pointer to item list.
        */
        inline const CFavouritesItemList* Items() const;

    public:     // from CTextListBoxModel

        /**
        * Get matchable text array (for filtering).
        * @return The matchable text array.
        */
        const MDesCArray* MatchableTextArray() const;

    protected:  // Construct / destruct

        /**
        * Constructor.
        * The two arrays passed must have the same number of items.
        * @param aItems Itemarray; not owned.
        * @param aIconIndexes Icon index array, not owned.
        */
        CBrowserFavouritesListboxModel
            (
            const CFavouritesItemList& aItems,
            const CBrowserFavouritesIconIndexArray& aIconIndexes
            );

        /**
        * Second phase constructor. Leaves on failure.
        */
        void ConstructL();

    private:    // data

        /**
        * The item array (real data, array of CFavouritesItem-s). Not owned.
        */
        const CFavouritesItemList* iItems;

        /**
        * Icon indexes. Not owned.
        */
        const CBrowserFavouritesIconIndexArray* iIconIndexes;

        /**
        * The item text array (formatting for listbox).
        */
        TBrowserFavouritesListboxItemTextArray iItemTexts;

        /**
        * The filter text array (formatting for filtering).
        */
        TBrowserFavouritesListboxFilterTextArray iFilterTexts;

    };

#include "BrowserFavouritesListboxModel.inl"

#endif

// End of file
