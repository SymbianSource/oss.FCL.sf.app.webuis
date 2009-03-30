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
*      Declaration of class CBrowserFavouritesListbox.
*      
*
*/


#ifndef BROWSER_FAVOURITES_LISTBOX_H
#define BROWSER_FAVOURITES_LISTBOX_H

//  INCLUDES
#include <aknlists.h>
#include "BrowserFavouritesSelectionState.h"
#include "BrowserFaviconHandler.h"

// FORWARD DECLARATIONS
class CFavouritesItem;
class CFavouritesItemList;
class CBrowserFavouritesIconIndexArray;
class MBrowserFavouritesListboxIconHandler;
class CBrowserFavouritesListboxModel;
class MBrowserFavouritesListboxCursorObserver;
class CBrowserFavouritesListboxState;
class MApiProvider;

// MACROS

// CLASS DECLARATION

/**
* Listbox to display and browse bookmarks and folders.
* Display is sorted: Homepage first, Last Visited next, bookmarks sorted
* alphabetically, folders come last alphabetically. Markable single-graphics
* style listbox.
*/
struct  TBrowserFavouritesListboxDefaultDatas
	{
	TInt  iPreferedId;
	TBool  iInSeamlessFolder;
	};


/**
* Listbox to display and browse bookmarks and folders.
* Display is sorted: Homepage first, Last Visited next, bookmarks sorted
* alphabetically, folders come last alphabetically. Markable single-graphics
* style listbox.
*/
class CBrowserFavouritesListbox : 	public CAknSingleGraphicStyleListBox
									,public MBrowserFaviconObserver
    {
	public:	    // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aParent Parent control.
        * @param aIconHandler The icon handler. Not owned.
        * @return The constructed listbox.
        */
        static CBrowserFavouritesListbox* NewL
            (
            MApiProvider& aApiProvider,
            const CCoeControl* aParent,
            const MBrowserFavouritesListboxIconHandler& aIconHandler
            );

        /**
        * Destructor.
        */
        virtual ~CBrowserFavouritesListbox();

    public:     // new methods

        /**
        * Set listbox observer.
        * @param Observer for this listbox.
        */
        void SetListboxCursorObserver
            ( MBrowserFavouritesListboxCursorObserver* aObserver );

        /**
        * Set new data to listbox.
        * @param aItems Items to display. Ownership is taken. Should not be on
        * cleanup stack when calling this (this method will push).
        * @param aCommsModel to be used when setting bearer icons.
        * (Ownership not taken.)
        * @param aKeepState If ETrue, try to keep current state (marks etc.).
        */
        void SetDataL
            (
            CFavouritesItemList* aItems,
            TBool aKeepState
            );

        /**
        * Get selection state.
        * @return Selection state.
        */
        TBrowserFavouritesSelectionState SelectionStateL() const;

        /**
        * Get currently highlighted item.
        * @return Pointer to highlighted object or NULL.
        */
        const CFavouritesItem* CurrentItem() const;

        /**
        * Get currently marked one item (if more is marked, or nothing
        * is marked, return NULL).
        * @return Pointer to marked one object or NULL.
        */
        const CFavouritesItem* MarkedItemL() const;

        /**
        * Highlight the item having uid aUid. Does nothing
        * if no item in the listbox has this uid.
        * @param aUid Uid of item to highlight.
        * @return ETrue if the item was found and highlighted; EFalse if not.
        */
        TBool HighlightUidNow( TInt aUid );

        /**
        * Handle markable list command. Does nothing
        * if listbox is not markable or empty.
        */
        void HandleMarkableListCommandL( TInt aCommand );

        /**
        * Return Uids of marked items.
        * @return Array of Uids. The array has been placed on the cleanup
        * stack. The caller must destroy it and pop after no longer needed.
        */
        CArrayFix<TInt>* MarkedUidsLC() const;

        /**
        * Return Uids of selected items. These are the marked ones, if no marks
        * then the highlighted one.
        * @return Array of Uids. The array has been placed on the cleanup
        * stack. The caller must destroy it and pop after no longer needed.
        */
        CArrayFix<TInt>* SelectedUidsLC() const;

        /**
        * Return a list of pointers to selected items. These are the marked
        * ones, if no marks then the highlighted one.
        * Note that the items exist within the listbox,
        * and may be updated! So if you do something with the list which
        * can be interrupted by some (other) active objects, copy the items
        * first.
        * Owner of the items is still the listbox, the owner of the list is
        * the caller (i.e. you should delete the list, but not ResetAndDestroy
        * it).
        * @param aIncludeSpecialItems When ETrue, everything is included. When EFalse,
        * homepage, last visited and folder types are excluded.
        * @return Array of pointers to items. Items are owned by the listbox.
        * The array has been placed on the cleanup stack. The caller must
        * destroy it and pop after no longer needed.
        */
        CArrayPtr<const CFavouritesItem>* SelectedItemsLC(TBool aIncludeSpecialItems=ETrue) const;

        /**
        * Return a pointer to item having aUid. Note that the item exists
        * within the listbox, and may be deleted! So if you do something with
        * it which can be interrupted by some (other) active objects, copy the
        * item first.
        * @param aUid Uid of item to look for.
        * @return Pointer to item (owned by the listbox), having this uid; or
        * NULL if no such item is found.
        */
        const CFavouritesItem* ItemByUid( TInt aUid ) const;

        /**
        * Return number of items in the listbox, not considering filters.
        */
        TInt UnfilteredNumberOfItems();
        
        
        /**
        * Return number of filtered items in the listbox
        */
        TInt FilteredNumberOfItems();


        /**
        * Update filtering (e.g. item array changed, or filter text changed
        * from outside).
        */
        void UpdateFilterL();

        /**
        * Query if there is any folder in the listbox.
        * @return ETrue if there is any folder.
        */
        TBool AnyFoldersL();

        /*
        * Return real indexes of selected items. These are the marked
        * ones, if no marks then the highlighted one.
        * @param aIncludeCurrent When ETrue, highlighted item is included, even
        * if not marked.
        * @return Array of view indexes. The array has been placed on the
        * cleanup stack. The caller must destroy it and pop after no longer
        * needed.
        */
        CArrayFix<TInt>* SelectedRealIndexesLC() const;

        /**
        * Get the real index of current item (index into the model).
        * @return View index or KErrNotFound if no current item.
        */
        TInt CurrentItemRealIndex() const;

        /**
        * Set highlight / marks / top item index state.
        * @param aState State to restore from.
        */
        void SetStateL( const CBrowserFavouritesListboxState& aState );


	public:     // Functions from CAknSingleGraphicStyleListBox

        /**
        * Handle key event.
        * @param aKeyEvent The key event.
        * @param aType Key event type.
        * @return Response (was the key event consumed?).
        */
		TKeyResponse OfferKeyEventL
            ( const TKeyEvent& aKeyEvent, TEventCode aType );

        /**
        * Handle focus change.
        * @param aDrawNow Draw now?
        */
		void FocusChanged( TDrawNow aDrawNow );

        /*
        * Create listbox model.
        * @return The model for this listbox.
        */
        void CreateModelL();

        /**
        * Create listbox view.
        * @return The view for this listbox.
        */
        CListBoxView* MakeViewClassInstanceL();

        /**
        * Create item drawer.
        */
        void CreateItemDrawerL();

		void HandleResourceChange( TInt aType );

	public: // from MBrowserFaviconObserver	
		TInt DrawFavicons();

	public: // defualt item access 		
		inline TBrowserFavouritesListboxDefaultDatas& GetDefaultData()
		    { return iDefaultDatas; }
		    
		inline CBrowserFaviconHandler* FaviconHandler() 
									{ return iFaviconHandler; }
									
		inline TBool IsSkinUpdated ()
		    { return iSkinUpdated; }
		    
		void SetSkinUpdated(TBool aSkinUpdated);

    protected:  // Construct / destruct

        /**
        * Constructor.
        */
		CBrowserFavouritesListbox( MApiProvider& aApiProvider );

        /**
        * Second phase constructor. Leaves on failure.
        * @param aParent Parent control.
        * @param aIconHandler The icon handler. Not owned.
        */
		void ConstructL
            (
            const CCoeControl* aParent,
            const MBrowserFavouritesListboxIconHandler& aIconHandler
            );

        /**
        * Get listbox flags (markable etc.).
        * @return listbox flags.
        */
        virtual TInt ListboxFlags();
		
    private:    // new methods, getting listbox data

        /**
        * Return items to be displayed in a list. Owner is the caller.
        * The returned list must have exactly as much items as aItems.
        * @param aCommsModel Comms model to be used for getting AP data.
        * @param aItems Item list.
        * @return Icon index array.
        */
        CBrowserFavouritesIconIndexArray* GetIconIndexesLC
            ( CFavouritesItemList& aItems );

        /**
        * Based on existing items and listbox state, calculate the new
        * state which will be applied if items are replaced with new ones.
        * @param aNewItems New item list.
        * @return The new state. Owner is the caller, it must free the
        * returned state.
        */
        void CalcNewStateL( CFavouritesItemList& aNewItems );


    private:    // new methods: index <--> Uid conversion

        /**
        * Convert Uid to index.
        * @param aUid Uid to convert.
        * @return View index of item having aUid, or KErrNotFound.
        */
        TInt UidToViewIndex( TInt aUid ) const;

        /**
        * Convert Uids to view indexes on the list. If any Uid is not found,
        * it is removed from the list (list may contain less elements
        * after calling).
        * @param aList List of Uids to convert.
        */
        void UidsToViewIndexes( CArrayFix<TInt>& aList ) const;

        /**
        * Convert real indexes to Uids on the list.
        * @param aList List of indexes to convert.
        */
        void RealIndexesToUids( CArrayFix<TInt>& aList ) const;

        /**
        * Return real indexes of marked items.
        * @param aIncludeCurrent When ETrue, highlighted item is included, even
        * if not marked.
        * @return Array of view indexes. The array has been placed on the
        * cleanup stack. The caller must destroy it and pop after no longer
        * needed.
        */
        CArrayFix<TInt>* MarkedRealIndexesLC() const;



    private:    // Conversion between uid <--> view index <--> real index.

        // Note: there are two kinds of indexes in the listbox. One set is
        // what is visible in the view (called View index), and the other
        // what data is actually stored in the model (called Real index).
        // When filtering kicks in, the two kind of indexes do not necessarily
        // match, and a mapping between the two must be established.


        /**
        * Convert view index to real index.
        * @param aViewlIndex View index (indexing to the visible elements).
        * @return Real index or KErrNotFound if not found / not visible.
        */
        TInt RealIndex( TInt aViewIndex ) const;

        /**
        * Convert view indexes to real indexes on the list. If any index is
        * not found / not visible, it is removed from the list (list may
        * contain less elements after calling).
        * @param aList List of indexes to convert.
        */
        void RealIndexes( CArrayFix<TInt>& aList ) const;

        /**
        * Get flags for constructing selection state.
        * @param aItem Extract flags from this item.
        * @return Flags originating from the item.
        */
        TInt SelectionFlags( const CFavouritesItem& aItem ) const;

        /**
        * Convenience method; return the model.
        * @return The model.
        */
        CBrowserFavouritesListboxModel* TheModel() const;

    protected:  // data

        /**
        * Icon handler (this creates icons and calculates icon indexes).
        */
        const MBrowserFavouritesListboxIconHandler* iIconHandler;

    protected:  // data

        /**
        * Real data for this listbox, array of items. Owned.
        */
        CFavouritesItemList* iItems;

        /**
        * Icon indexes for the items. Owned.
        */
        CBrowserFavouritesIconIndexArray* iIconIndexes;
        
        /**
        * Skin updated status
        */
        TBool iSkinUpdated;

    private:    // data

        MApiProvider& iApiProvider;	// not owned
        
        /**
        * Observer for cursor changes.
        */
        MBrowserFavouritesListboxCursorObserver* iCursorObserver;

        /**
        * Placeholder for new listbox state, when listbox content changes.
        * This should really be a local variable, but it just cannot be done!
        * because the cleanup stack cannot be managed (can't swap items in
        * the stack).
        */
        CBrowserFavouritesListboxState* 		iNewState;
		TBrowserFavouritesListboxDefaultDatas 	iDefaultDatas;
		CBrowserFaviconHandler* 				iFaviconHandler;

    };

#endif

// End of File
