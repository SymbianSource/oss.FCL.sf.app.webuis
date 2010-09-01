/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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
*      Declaration of class CBrowserBookmarksView.
*      
*
*/


#ifndef BROWSER_BOOKMARKS_VIEW_H
#define BROWSER_BOOKMARKS_VIEW_H

// INCLUDE FILES
#include "BrowserFavouritesView.h"
#include "BrowserBookmarksGotoPane.h"
#include "Preferences.h"
#include <apchangeobserver.h>
#include <brctlinterface.h>
#include <AknToolbarObserver.h>


// CONSTANTS

//FORWARD DECLARATION

class CBrowserBookmarksContainer;
class CRecentUrlStore;
class CAsyncDownloadsListExecuter;

// CLASS DECLARATION

/**
* Bookmarks View of the WML browser. Derived from CBrowserFavouritesView;
* new methods mostly process Bookmark View specific commands.
*/
class CBrowserBookmarksView:
                            public CBrowserFavouritesView, 
                            public MBookmarksGotoPaneObserver,
                            public MApChangeObserver,
                            public MPreferencesObserver,
                            public MBrCtlLoadEventObserver,
                            public MAknToolbarObserver
	{
	public:     // construction

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aApiProvider The API provider.
        * @return The constructed view.
        */
        static CBrowserBookmarksView* NewLC( MApiProvider& aApiProvider,  CRecentUrlStore& aRecentUrlStore, TInt aInitialFolderUid = KFavouritesRootUid);

        /**
        * Destructor.
        */
        virtual ~CBrowserBookmarksView();

    public:     // new methods

        /**
        * Force updating the Goto Pane. Does nothing if Goto Pane is not
        * visible.
		* @since 1.2
        */
        void UpdateGotoPaneL();
        
        /**
        * Exits Adaptive Bookmarks folder. Called also from BrowserAppUi when a page is
        * downloaded from Adaptive Bookmarks.
		* @since 2.1
        */
        void ExitAdaptiveBookmarks();

        /**
        * Opens Adaptive Bookmarks folder.
		* @since 3.1
        */
        inline void OpenAdaptiveBookmarksWhenActivated( )
        	{ iOpenAdaptiveBookmarksWhenActivated = ETrue; }

        /**
        * Gets the status of selection of the adaptive bookmarks folder.
		* @since 3.1
        */
        inline TBool GetAdaptiveBookmarksFolderSelected( )
        	{ return iAdaptiveBookmarksFolderSelected; }

        inline TBool GetAdaptiveBookmarksFolderWasActive( )
        	{ return iAdaptiveBookmarksFolderWasActive; }
        	
        void DimToolbarButtons(TBool aDimButtons);

	public:     // public methods from CBrowserFavouritesView

        /**
        * Handle command.
		* @since 1.2
        * @param aCommand Command id.
        */
		void HandleCommandL( TInt aCommand );

        /**
        * Get view id.
		* @since 1.2
        * @return The view id for this view.
        */
		TUid Id() const;

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
		void CommandSetResourceDynL(TSKPair& /*lsk*/, TSKPair& /*rsk*/, TSKPair& /*msk*/);

		/**
		* Helper function to dynamically set LSK
		* @since 5.0
		*/
		void SetLSKDynL(TSKPair& aLsk,
			CBrowserBookmarksGotoPane* aTheGotoPane);

		/**
		* Helper function to dynamically set RSK
		* @since 5.0
		*/
		void SetRSKDynL(TSKPair& aRsk,
			CBrowserBookmarksGotoPane* aTheGotoPane);

		/**
		* Helper function to dynamically set MSK
		* @since 5.0
		*/
		void SetMSKDynL(TSKPair& aMsk, const TSKPair aLsk,
			CBrowserFavouritesContainer* aTheContainer,
			CBrowserBookmarksGotoPane* aTheGotoPane);
        

    public:     // from MBookmarksGotoPaneObserver

        /**
        * Handle Goto Pane event.
		* @since 1.2
        * @param aGotoPane The Goto Pane in which the event occurred.
        * @param aEvent Goto Pane event.
        */
        virtual void HandleBookmarksGotoPaneEventL
            (
            CBrowserBookmarksGotoPane* aGotoPane,
            MBookmarksGotoPaneObserver::TEvent aEvent
            );

    public: // MAknToolbarObserver
        /**
        * Called when toolbar events occur
        *
        * @since 5.0
        * @return void
        */
        void OfferToolbarEventL( TInt aCommand ) { HandleCommandL(aCommand); }

	protected:  // construction

        /**
        * Constructor.
        * @param aApiProvider The API provider.
        */
        CBrowserBookmarksView( MApiProvider& aApiProvider, CRecentUrlStore& aRecentUrlStore, TInt aInitialFolderUid = KFavouritesRootUid );

        /**
        * Second phase constructor. Leaves on failure.
        * @param aViewResourceId Resource id for this view.
        * Derived classes should call this first during construction.
        */
		void ConstructL( TInt aViewResourceId );

	protected:	// protected CBrowserFavouritesView

        /**
        * Handle listbox event.
		* @since 1.2
        * @param aListBox Listbox in which the event occurred.
        * @param aEventType Event.
        */
        void HandleListBoxEventL(
            CEikListBox* aListBox,
            MEikListBoxObserver::TListBoxEvent aEventType );

        /**
        * Initialize context-sensitive menu.
		* @since 1.2
        * @param aResourceId Resource id of the menu pane.
        * @param aMenuPane Menu pane object being initialized.
        * @param aIndex Index into the visibility map.
        * @param aState Selection state.
        */
        virtual void DynInitMenuPaneL
                (
                TInt aResourceId,
                CEikMenuPane* aMenuPane,
                TCommandVisibility::TIndex /*aIndex*/,
                const TBrowserFavouritesSelectionState& aState
                );

        /**
        * Get resource id of text to be displayed in navi pane when
        * root folder is shown (TBUF).
		* @since 1.2
        * @return Resource id.
        */
        TInt RootTitleResourceId() const;

        /**
        * Get resource id of Options menu (RESOURCE MENU_BAR).
		* @since 1.2
        * @return Resource id.
        */
        TInt OptionsMenuResourceId() const;

        /**
        * Get resource id of OK-Options menu (RESOURCE MENU_BAR).
		* @since 1.2
        * @return Resource id.
        */
        TInt OkOptionsMenuResourceId() const;

        /**
        * Create the model. Leave on failure. The model is owned by the
        * caller (this view, base class). The created model is really
        * a CBrowserBokmarksModel.
		* @since 1.2
        * @return The constructed model.
        */
        virtual CBrowserFavouritesModel* CreateModelL();

        /**
        * Create the container (the main control of the view). The container
        * is owned by the caller (this view, base class). The created container
        * is really a CBrowserBookmarksContainer.
        * Leave on failure.
		* @since 1.2
        * @return The constructed main control.
        */
        virtual CBrowserFavouritesContainer* CreateContainerL();

        /**
        * Get items to be displayed in a folder. Expects the model's db be
        * open.
		* @since 1.2
        * @param aFolder Folder which contains the items.
        * @return List of items. Pushed on the cleanup stack.
        */
        virtual CFavouritesItemList* GetItemsLC( TInt aFolder );

        /**
        * Some displayed data has changed (Favourites database, AP-s, etc).
        * Update the view so changes become visible. Safe to call if the view
        * is not active; in this case it does nothing.
        * This method should not be called if the data change is due to direct
        * user activity (opening folders etc.); only for notification
        * processing.
        * (Same as base class method, except active Goto Pane is not reset if
        * downloading is in progress.)
		* @since 1.2
        * @param aDbErrorNote If ETrue, unsuccesful database access will bring
        * up an error note.
        */
        virtual void RefreshL( TBool aDbErrorNote = EFalse );
        
        void HandleForegroundEventL( TBool aForeground );

	protected:	// protected methods from CAknView

        /**
        * Activate the view.
		* @since 1.2
        * @param aCustomMessageId Custom Message Id.
        * @param aCustomMessage Custom Message.
        */
        virtual void DoActivateL
            (
            const TVwsViewId& aPrevViewId,
            TUid aCustomMessageId,
            const TDesC8& aCustomMessage
            );

        /**
        * Deactivate the view.
		* @since 1.2
        */
        virtual void DoDeactivate();

    private:    // command handling (additional commands)

        /**
        * Available only to process EWmlCmdDelete and EWmlCmdDeleteItems commands.Command "Go to" when the user is editing in the Goto Pane.
        */
        void DeleteItems( TInt aCommand );

        /**
        * Command "Go to" when the user is editing in the Goto Pane.
        * Go to the URL found in Goto Pane.
        * Does nothing if no Goto Pane.
        */
        void GotoUrlInGotoPaneL();

        /**
        * Command "Activate". Activate highlighted bookmark.
        * Does nothing if no bookmark is highlighted.
        */
        void ActivateCurrentBookmarkL();

        /**
        * Command "Add bookmark". Create new bookmark and launch
        * Edit bookmark dialog.
        */
        void AddNewBookmarkL();

        /**
        * Command "Edit bookmark". Edit highlighted bookmark.
        * Does nothing if no bookmark is highlighted.
        */
        void EditCurrentBookmarkL();


        /**
        * Command "Send via Unified Messaging".
        * Send selected bookmark(s).
        */
        void SendBookmarksL( );


    private:    // new methods

        /**
        * Get the container (as CBrowserBookmarksContainer).
        * @return The container.
        */
        CBrowserBookmarksContainer* TheContainer();

    private:    // from MApChangeObserver
        /**
        * Handle change in currently used Access Point.
        */
        void ApChangedL( TUint32 aNewAPId );

    private:    // from MPreferencesObserver

        /**
        * Handle preferences change.
        */
        void HandlePreferencesChangeL( 
                                    const TPreferencesEvent aEvent,            
                                    TPreferencesValues& aValues,
                                    TBrCtlDefs::TBrCtlSettings aSettingType );

    private:

        /**
        * Create Start Page bookmark. Db needs not be open (not used).
        * @return The Start Page bookmark, or NULL if no such.
        * Ownership passed to caller.
        */
        CFavouritesItem* CreateStartPageBookmarkL();

    private:    // from MBrCtlLoadEventObserver

        /**
        * A load events notification
        * @since 2.8
        * @param aLoadEvent The load event
        * @param aSize Size depends on the event
        * @param aTransactionId The transaction id of the transaction that had this event
        * @return void
        */
        void HandleBrowserLoadEventL( TBrCtlDefs::TBrCtlLoadEvent aLoadEvent,
                                      TUint aSize,
                                      TUint16 aTransactionId );
    private:

        /**
        * Clear all adaptive bookmarks
        */
        void ClearAdaptiveBookmarksL();

    private:

        /**
        * To set empty text for folders. It is different in Adaptive Bookmarks
        */
		void SetEmptyTextL();

	    // from CBrowserFavouritesView
        TKeyResponse OfferKeyEventL
            ( const TKeyEvent& aKeyEvent, TEventCode aType );

//from CBrowserFavouritesView
        /**
        * Open next folder.
        * @param aForward Open next folder if ETrue, previous if EFalse.
        */
		void OpenNextFolderL( TBool aForward );

        /**
        * Update and draw navi pane (show tabs for root, or "1/4" style text
        * for subfolders).
        * No sanity check; expects the model's table be open.
        */
        void UpdateNaviPaneL();

        /**
        * Sorts bookmark items into the saved manual order
        * associated to the given folder
        * @param aFolder The folder in which the items are stored
        * @param aItems The bookmark item list to sort
        */
        TBool ManualBMSortL( TInt aFolder, CFavouritesItemList* aItems );

        /**
        * Starts 'item moving mode', when softkeys are ok/cancel
        * and ok moves the selected items to the current position in 
        * bookmarks list
        */
        void StartMoveItemsL();

        /**
        * moves the selected items to the current position in 
        * bookmarks list
        */
        void MoveItemsL();

        /**
        * Cancels 'item moving mode', softkeys return to normal,
        * focus returns to the first selected item
        */
        void CancelMoveItemsL();

        /*
        * Adds the given uid to the last place in bookmarks list
        * @param aUid The uid to add
        */
        void AddUidToLastPlaceToCurrentListL( TInt aUid );

    public:
        /**
        * Dim or Un-dim Toolbar buttons.
        */
        void UpdateToolbarButtonsState();
        
        /**
        * Check to show downloads.
        */
        void CheckForDownloads();

        /**
        * Initiates update for favicons
        */
        void UpdateFavIconsL();

    private:    // data
        /**
         * Callback for completing the Browser App init. Normally only
         * BookmarksView is initialized first on startup. When bookmarks
         * view is shown then this callback is scheduled to complete the
         * remaining initialization. 
         */
        static TInt CompleteAppInitCallback( TAny* aApiProvider );        
        
        /**
        * Uid of AP which is to be used for creating Start Page bookmark.
        */
        TUint32 iStartPageApUid;

        /**
        * Storing current default AP, to be able to spot when it changes.
        * (We get notification when preferences change; but don't know
        * what changed in preferences. We are interested only in default AP
        * change.)
        */
        TUint32 iCurrentDefaultAp;

        /**
        * This flag is used in fetch observing: this differentiates events from
        * the newly started fetch from events of ongoing fetches (being
        * cancelled by the new one). It is ETrue when new fetch has been
        * initiated but not yet started (existing fetches will be cancelled
        * during this time).
        */
        TBool iFetchInit;

		HBufC* iDomainFolderName;
		CRecentUrlStore& iRecentUrlStore;
		TBool iInAdaptiveBookmarksFolder;

		TWmlSettingsAdaptiveBookmarks iAdaptiveBookmarksCurrentSetting;


		// For Adaptive Bookmarks - to refresh title when 
		// coming back from Preferences
		TBool iPreferencesChanged;

        // For Manual BM Sorting
        // Indicates item moving mode
        TBool iManualItemMovingGoingOn;

        // list of uids of items to move
        CArrayFixFlat<TInt>* iItemsToMove;

        // Current order of bookmarks
        CBrowserBookmarksOrder* iCurrentOrder;

        // Flag to indicate that Adaptive BM folder has just been inserted to BM list
        TBool iInsertAdaptiveBookmarksFolder;

        // Flag to indicate that current BM order needs to be saved
        TBool iSaveBMOrder;

        // Flag to indicate the presence of Start Page item
        TBool iStartPagePresent;

        // Goto Pane's entered URL and cursor's position
        HBufC* iEnteredURL;
        TInt iCursorPos;

        TUint16 iTrId;
        
        CAsyncDownloadsListExecuter* iDownloadsListExecuter; // Owned.
        
        // True if the view will show the adaptive bookmarks folder
        // on activation, otherwise false.
        TBool iOpenAdaptiveBookmarksWhenActivated;
        
        // True if the user opned the Adaptive Bookmarks folder,
        // from bookmarks items, otherwise false.
        TBool iAdaptiveBookmarksFolderSelected;
        
        // True if the view was changed, and the adaptive bookmarks folder
        // was currently acive.
        TBool iAdaptiveBookmarksFolderWasActive;       
        
        // Pen enabled flag
        TBool iPenEnabled;
        
        // Re-Entry Gate flag for Delete Bookmark routine
        TBool iHandleDeleteInProgress;

        // Async callback object for delayed App init
        CIdle *iAsyncComplete;
        
        TBool iShowDownlods;
	};

#endif

// End of file
