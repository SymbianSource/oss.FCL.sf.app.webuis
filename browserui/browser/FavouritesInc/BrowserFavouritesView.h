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
*      Declaration of class CBrowserFavouritesView.
*      
*
*/


#ifndef BROWSER_FAVOURITES_VIEW_H
#define BROWSER_FAVOURITES_VIEW_H

// INCLUDE FILES
#include <eiklbo.h>

#include "BrowserFavouritesModelObserver.h"
#include "BrowserAppViewBase.h"
#include "CommandVisibility.h"
#include "CommsModel.h"
#include "BrowserFavouritesModel.h"
#include <favouriteslimits.h>

// FORWARD DECLARATION
class CFavouritesItemList;
class CBrowserFavouritesContainer;
class CBrowserFavouritesModel;
class CBrowserFavouritesIncrementalOp;
class CBrowserFavouritesListboxState;

// CLASS DECLARATION

/**
* Base class for Bookmarks View and Saved Decks View of the WML Browser.
* The common behaviour of these views is collected here. This includes:
* Handling view activation / decativation;
* Interpreting (common) commands;
* Providing context-specific menus.
* Pure virtual; derived classes must provide model and control components
* and items to display. A number of methods can be overridden.
*/
class CBrowserFavouritesView:public CBrowserViewBase,
                             public MEikListBoxObserver,
                             public MBrowserFavouritesModelObserver,
                             public MCommsModelObserver
	{
	public:     // construction

        /**
        * Destructor.
        */
        virtual ~CBrowserFavouritesView();

	public:     // From MCbaSetter (CBrowserViewBase)
        
        /**
        * Return command set id, to be displayed.
		* @since 1.2
        * @return The command set's resource id.
        */
        virtual TInt CommandSetResourceIdL();

		/**
		* Set command set lsk,rsk,msk dynamically via pointers.
		* Derived classes should implement, though it can be empty.
		* If it does nothing, empty softkeys will be assigned
		* @since 5.0
		*/
		void CommandSetResourceDynL(TSKPair& /*lsk*/, TSKPair& /*rsk*/, TSKPair& /*msk*/) {};
        
        /**
        * Return the resource ID of the seamless folder.
		* @param aContextID Context ID of the seamless folder.
        * @return The resource ID of the seamless folder or zero if it is not
        * seamless.
        */
        static TInt GetSeamlessFolderResourceID( TInt aContextID );
        
        /**
        * Return the resource ID of the seamless folder title.
		* @param aContextID Context ID of the seamless folder.
        * @return The resource ID of the seamless folder title or zero if it is not
        * seamless.
        */
        static TInt GetSeamlessFolderTitleResourceID( TInt aContextID );
        
        /**
        * Examines if the given ID is an ID of a seamless folder.
		* @param aContextID Context ID of the folder.
        * @return The ETrue if the given ID is a seamless ID otherwise EFalse.
        * seamless.
        */
        static TBool IsSeamlessFolder( TInt aContextID );
        
        
        /**
        * Configure Context Menu Resource
        *
        *   context-sensitive menu when item(s) marked
        */
        void ConfigContextMenu();


	public:		// public methods from CAknView (CBrowserViewBase)

        /**
        * Handle command.
		* @since 1.2
        * @param aCommand Command id.
        */
		virtual void HandleCommandL( TInt aCommand );

        /**
        * Get view id. Derived classes must provide this method.
        * @return The view id for this view.
        */
		virtual TUid Id() const = 0;

        void OpenFixedFolderL(TInt aUid);

	protected :

		// For Adaptive bookmarks folder navigation, used in Bookmarks View

		void SetRootFolderForAdaptiveBookmarks();

	protected :	// construction

        /**
        * Constructor.
        * @param aApiProvider The API provider.
        */
		CBrowserFavouritesView( MApiProvider& aApiProvider, TInt aInitialFolderId = KFavouritesRootUid );

        /**
        * Second phase constructor. Leaves on failure.
        * @param aViewResourceId Resource id for this view.
        * Derived classes should call this first during construction.
        */
		void ConstructL( TInt aViewResourceId );

    protected:  // menu initialization

        /**
        * Initialize context-sensitive menu.
        * @param aResourceId Resource id of the menu pane.
        * @param aMenuPane Menu pane object being initialized.
        * @param aIndex Index into the visibility map.
        * @param aState Selection state.
        */
        virtual void DynInitMenuPaneL
                (
                TInt aResourceId,
                CEikMenuPane* aMenuPane,
                TCommandVisibility::TIndex aIndex,
                const TBrowserFavouritesSelectionState& aState
                ) = 0;

    protected:  // new methods

        /**
        * Get resource id of text to be displayed in navi pane when
        * root folder is shown (TBUF).
        * Derived classes must provide this method.
		* @since 1.2
        * @return Resource id.
        */
        virtual TInt RootTitleResourceId() const = 0;

        /**
        * Get resource id of Options menu (RESOURCE MENU_BAR).
        * Derived classes must provide this method.
		* @since 1.2
        * @return Resource id.
        */
        virtual TInt OptionsMenuResourceId() const = 0;

        /**
        * Get resource id of OK-Options menu (RESOURCE MENU_BAR).
        * Derived classes must provide this method.
		* @since 1.2
        * @return Resource id.
        */
        virtual TInt OkOptionsMenuResourceId() const = 0;

    protected:  // component creation

        /**
        * Create the model. Leave on failure. The model is owned by the
        * caller (this view).
        * Derived classes must provide this method.
        * @return The constructed model.
        */
        virtual CBrowserFavouritesModel* CreateModelL() = 0;

        /**
        * Create the container (the main control of the view). The container
        * is owned by the caller (this view).
        * Leave on failure.
        * Derived classes must provide this method.
        * @return The constructed main control.
        */
        virtual CBrowserFavouritesContainer* CreateContainerL() = 0;

    protected: // data retrieval

        /**
        * Get items to be displayed in a folder. Expects the model's db be
        * open.
        * @param aFolder Folder which contains the items.
        * @return List of items. Pushed on the cleanup stack.
        */
        virtual CFavouritesItemList* GetItemsLC( TInt aFolder );

	public:
	
        /**
        * Get the model.
        * @return The model.
        */
        inline CBrowserFavouritesModel& Model() const;

        inline void ShowNextFolerL(TBool aNext) { OpenNextFolderL(aNext); }

    protected:  // component retrieval
    
        /**
        * Get the container (the view's main control).
        * @return The container. Can be NULL (when view is not activated,
        * container does not exist).
        */
        inline CBrowserFavouritesContainer* Container() const;

        /**
        * Get Uid of the folder shown.
        * @return Current folder's Uid.
        */
        inline TInt CurrentFolder() const;
        

    protected:  // new methods

        /**
        * If there is a preferred highlight uid, attempt to highlight that.
        * If succeeded, preferred highlight uid is cleared ("used up").
        * Otherwise do nothing.
        */
        void HighlightPreferredL();

        /**
        * Some displayed data has changed (Favourites database, AP-s, etc).
        * Update the view so changes become visible. Safe to call if the view
        * is not active; in this case it does nothing.
        * This method should not be called if the data change is due to direct
        * user activity (opening folders etc.); only for notification
        * processing.
        * @param aDbErrorNote If ETrue, unsuccesful database access will bring
        * up an error note.
        */
        virtual void RefreshL( TBool aDbErrorNote = EFalse );

		/**
        * Redraw.
        */
        void HandleClientRectChange();

	protected:	// from CAknView

        /**
        * Activate the view. Open the database. If it was activated
        * previously, try to restore the view as it was before deactivation.
        * @param aPrevViewId Previous View Id.
        * @param aCustomMessageId Custom Message Id.
        * @param aCustomMessage Custom Message.
        */
        void DoActivateL
            (
            const TVwsViewId& aPrevViewId,
            TUid aCustomMessageId,
            const TDesC8& aCustomMessage
            );

        /**
        * Deactivate the view. Close the database. Save the view state,
        * so it can be restored on future activations.
        */
        void DoDeactivate();
      
        /**
        * Initialize context-sensitive menu. Do not override this one;
        * override the other.
        * @param aResourceId Resource id of the menu pane.
        * @param aMenuPane Menu pane object being initialized.
        */
        void DynInitMenuPaneL
            ( TInt aResourceId, CEikMenuPane* aMenuPane );

        /**
        * Handle foreground event.
        * @param aForeground ETrue if foreground.
        */
        void HandleForegroundEventL( TBool aForeground );

    protected:	// from MEikListBoxObserver

        /**
        * Handle listbox event.
        * @param aListBox Listbox in which the event occurred.
        * @param aEventType Event.
        */
        virtual void HandleListBoxEventL
			( CEikListBox* aListBox, 
			MEikListBoxObserver::TListBoxEvent aEventType );

    private:    // command handling

        /**
        * Command "Back", when showing a folder. Open the parent folder.
        * of the currently shown folder. Highlight the folder just closed.
        */
        void CloseCurrentFolderL();

        /**
        * Command "Open (folder)". Open the highlighted folder.
        * Does nothing if no folder is highlighted.
        */
        void OpenCurrentFolderL();

        /**
        * Command "Open (folder)" from OK-Options menu. Open the marked folder.
        * Does nothing if no folder is marked or more is marked.
        */
        void OpenMarkedFolderL();

        /**
        * Command "Remove" or "Erase". Delete marked item(s).
        * Does nothing if no item is highlighted / marked.
        */
        virtual void DeleteMarkedItemsL();

    protected:
        /**
        * Command "New folder". New folders are alwas created in the root
        * folder.
        * @return The uid of the newly created folder, or KFavouritesNullUid
        * (if not created).
        */
        TInt AddNewFolderL();
        
    private:

        /**
        * Command "Move To Folder". Launch target folder selection dialog and
        * move marked item(s).
        */
        void MoveMarkedItemsL();


        /**
        * Command "Rename". Rename current item.
        * Does nothing if no item is highlighted.
        */
        void RenameCurrentItemL();

	protected:    // implementation details.

        /**
        * Open a folder by Uid and display its contents. Expects the
        * model's db be open.
        * @param aFolder Folder to open.
        */
        void OpenFolderL( TInt aFolder );

        /**
        * Open next folder.
        * @param aForward Open next folder if ETrue, previous if EFalse.
        */
        virtual void OpenNextFolderL( TBool aForward );
        
	private:

        /**
        * Fill listbox with folder contents. Expects the model's db be open.
        * @param aFolder Folder to display.
        * @param aKeepState If ETrue, try to restore marks/highlight/top item
        * position.
        */
        void FillListboxL( TInt aFolder, TBool aKeepState );

        /**
        * Display confirmation dialog before deleting items. The dialog
        * displayed is context-dependent (different dialog for one/more
        * items, and for non-empty folders).
        * @param aUids List of Uids (both item & folder) about to be deleted.
        * @param aFolderUids List of folder uids about to be deleted. These
        * uids are also in aUids, and are needed only to improve performance.
        * (The non-empty folder checking, which is the only database query in
        * this method, is performed only for folders. The effect of this is
        * visible only if many items are selected.)
        * @return ETrue if delete was confirmed; EFalse if not, or in case of
        * any error (e.g. database error).
        */
        TBool ConfirmDeleteL
            ( CArrayFix<TInt>& aUids, CArrayFix<TInt>& aFolderUids );

        /**
        * After deleting items, display appropriate note about the outcome.
        * @param aUids List of Uids that were selected for deletion.
        * @param aNotDeletedUids List of Uids which are not deleted.
        */
        void DisplayResultOfDeleteL
            (
            const CArrayFix<TInt>& aUids,
            const CArrayFix<TInt>& aNotDeletedUids
            );

        /**
        * Launch target folder selection dialog and return uid of selected
        * folder (or KFavouritesNullUid if cancelled or in case of any error).
        * If no folders exist, user is prompted to create one.
        * @param aExcludeFolder Exclude this folder from selection (current
        * folder).
        * @return Uid of selected folder, or KFavouritesNullUid.
        */
        TInt SelectTargetFolderL( TInt aExcludeFolder );

        /**
        * "First pass" of moving is done; items that could be moved without
        * user intervention, has been moved. Some of them may be still there,
        * because it is unmovable (e.g. Homepage), or conflicts with a name in
        * the target folder. For the latter ones, ask for each if replace the
        * old ones, and act accordingly.
        * Display appropriate note about what happened.
        * @param aFolder The target folder.
        * @param aUids List of Uids that were selected for moving.
        * @param aUnmovableUids List of Uids which could never be moved.
        * @param aConflictingNameUids List of conflicting name Uids.
        */
        void HandleMoveResultL
            (
            TInt aFolder,
            const CArrayFix<TInt>& aUids,
            const CArrayFix<TInt>& aUnmovableUids,
            const CArrayFix<TInt>& aConflictingNameUids
            );

        /**
        * For each of the items in the list, display a rename dialog before
        * moving them into the folder.
        * @param aFolder The target folder.
        * @param aUids List of Uids that were selected for moving.
        */
        void RenameAndMoveItemsL
            ( const CArrayFix<TInt>& aUids, TInt aFolder );

    protected:    // new methods

        /**
        * Update and draw navi pane (show tabs for root, or "1/4" style text
        * for subfolders).
        * No sanity check; expects the model's table be open.
        */
        virtual void UpdateNaviPaneL();

    private:    // new methods

        /**
        * The container passes back key events to the view first, to
        * allow moving between folders. (The container does not know
        * anything about folders.)
        */
        friend class CBrowserFavouritesContainer;
        
	protected:
        /**
        * Handle key event (called from the container, to allow moving between
        * folders.
        * @param aKeyEvent The key event.
        * @param aType Key event type.
        * @return Response (was the key event consumed?).
        */
        virtual TKeyResponse OfferKeyEventL
            ( const TKeyEvent& aKeyEvent, TEventCode aType );

        /**
        * Reset state with deactivation event
        */
        void ResetStateInDeActivation();

        /**  
        * Needed for updating tool bar buttons when there is a up/down arrow key events received.  
        **/  
        virtual void UpdateToolbarButtonsState(); 

    private:    // from MWmlBrowserFavouritesModelObserver

        /**
        * Handle model change.
        */
        void HandleFavouritesModelChangeL();

    private:    // from MCommsModelObserver

        /**
        * Handle Comms Model change (AP update).
        */
        void HandleCommsModelChangeL();

    private:    // default item settings 

        /**
        * Set Prefered the current item 
        */
        void  SetPreferedCurrentItemL();
		
        /**
        * Set Prefered the current item 
        */
        void  SetUnPreferedCurrentItemL();
        

    protected:  // data

        /// Preferred highlight uid.
        TInt iPreferredHighlightUid;

        /// If ETrue, OK-Options menu is shown instead of normal Options menu.
        TBool iShowOkOptions;

        /// Top-level control of the view. Owned
		CBrowserFavouritesContainer* iContainer;

        /// Uid of the current folder.
        TInt iCurrentFolder;
        
        /// ETrue between DoActivateL() and DoDeactivate().
        TBool iIsActivated;

	private:	// data

		/// The model (data handling). Owned.
        CBrowserFavouritesModel* iModel;
		TInt iLastSelection;
        /// Incremental operation, or NULL if no such. Owned.
        CBrowserFavouritesIncrementalOp* iIncrementalOp;
        /// ETrue if data update is missed due to incremental operation.
        TBool iUpdatePending;

    protected:

        TBool iRefresh; // Flag to indicate that Bookmarks list should be refreshed.
                        // ETrue by default, turned off only in cases we want to avoid
                        // double refresh. Checked in HandleFavouritesModelChangeL.
        
		TInt iInitialFolder; //Folder to which browser is launched in embedded mode
       
        // Flag to indicate that we are about to open Preferences view,
        // in order not to return to root folder afterwards.
        TBool iSaveStateOnDeactivate;

        CBrowserFavouritesListboxState* iSavedListboxState;
	};

#include "BrowserFavouritesView.inl"

#endif

// End of file
