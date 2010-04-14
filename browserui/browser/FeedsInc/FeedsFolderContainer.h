/*
* Copyright (c) 2005-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  A container to browse a user's list of feeds.
*
*/


#ifndef FEEDS_FOLDER_CONTAINER_H
#define FEEDS_FOLDER_CONTAINER_H


// INCLUDES
#include <coecntrl.h>
#include <coemop.h>
#include <eiklbo.h>

#include "FeedsEditFeedDialog.h"
#include "FeedsEditFolderDialog.h"

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS
class CAknDoubleGraphicStyleListBox;
class CAknNavigationDecorator;
class CAknIconArray;
class CAknPopupList;
class CAknSingleGraphicPopupMenuStyleListBox;
class CFeedsFolderView;
class CFeedsEntity;
class CGulIcon;
class TAknsItemID;
class MApiProvider;

// CLASS DECLARATION


/**
*  A container to browse a user's list of feeds.
*  @lib FeedsEngine.lib
*  @since 3.0
*/
class CFeedsFolderContainer : public CCoeControl, public MEikListBoxObserver,
        public MFeedsEditFeedDialogObserver, public MFeedsEditFolderDialogObserver
    {
    private:
        enum TIcons
            {
            EIconMark = 0,
            EIconFolder,
            EIconErrorFolder,
            EIconFeed,
            EIconFeedUnread,
            EIconErrorFeed
            };


    public:
        /**
        * Two-phased constructor.
        */
        static CFeedsFolderContainer* NewL(
        	CFeedsFolderView* aView,
            MApiProvider& aApiProvider,
            const TRect& aRect);

        /**
        * Destructor.
        */        
        virtual ~CFeedsFolderContainer();


    public: // From CoeControl
	    /**
	    * Handles key event.
	    *
        * @param aKeyEvent The key event.
        * @param aType The type of the event.
        * @return Indicates whether the key event was used by this control or not
        */
        TKeyResponse OfferKeyEventL(const TKeyEvent& aKeyEvent, TEventCode aType);

#ifdef __SERIES60_HELP
        /**
        * Get help context for the control.
        *
        * @since ?
        * @param aContext The context that is filled in.
        * @return None.
        */
        virtual void GetHelpContext(TCoeHelpContext& aContext) const;
#endif // __SERIES60_HELP


    protected: // From CoeControl
        /**
        * Called by framework when the view size is changed.
        *
        * @since ?
        * @return void.
        */
        virtual void SizeChanged();

        /**
        * Called by the framework when a display resource changes (i.e. skin or layout).
        *
        * @since ?
        * @return void.
        */
        virtual void HandleResourceChange(TInt aType);

        /**
        * Returns number of components.
	    *
        * @since ?
        * @return Number of component controls.
        */
        virtual TInt CountComponentControls() const;

        /**
        * Returns pointer to particular component.
	    *
        * @since ?
        * @param aIndex Index whose control's pointer has to returned.
        * @return Pointer to component control
        */
        virtual CCoeControl* ComponentControl(TInt aIndex) const;


    public: // MEikListBoxObserver
        /**
        * Processes key events from the listbox.
        *
        * @since ?
        * @param aListBox Listbox being observed.
        * @param aEventType Event observed.
        * @return void
        */
        virtual void HandleListBoxEventL(CEikListBox* aListBox, TListBoxEvent aEventType);


    public:  // From MFeedEditFeedDialogObserver
        /**
        * Called when a feed's name and/or url is changed -- this is called 
        * after both IsValidFeedName and IsValidFeedUrl are called.
        *
        * @since 3.0
        * @param aName The feed's new name.
        * @param aUrl The feed's new url.
        * @param aFreq The frequency for auto updating.
        * @return void.
        */
        virtual void UpdateFeedL(const TDesC& aName, const TDesC& aUrl, TInt aFreq);

        /**
        * Called when a new feed is created -- this is called after both
        * IsValidFeedName and IsValidFeedUrl are called.
        *
        * @since 3.0
        * @param aName The feed's new name.
        * @param aUrl The feed's new url.
        * @param aFreq The frequency for auto updating.
        * @return void.
        */
        virtual void NewFeedL(const TDesC& aName, const TDesC& aUrl,TInt aFreq);

        /**
        * Validates the Feed's updated name.
        *
        * @since 3.0
        * @param aName The feed's new name.
        * @param aIsEditing True if the edit dialog was opened to edit a feed.
        * @return ETrue if the value is valid.
        */
        virtual TBool IsFeedNameValidL(const TDesC* aName, TBool aIsEditing);

        /**
        * Validates the Feed's updated url.
        *
        * @since 3.0
        * @param aUrl The feed's new url.
        * @return ETrue if the value is valid.
        */
        virtual TBool IsFeedUrlValidL(const TDesC* aUrl);


    public:  // From MFeedEditFolderDialogObserver
        /**
        * Called when a folder's name is changed -- this is called 
        * after IsValidFolderName is called.
        *
        * @since 3.0
        * @param aName The feed's new name.
        * @return void.
        */
        virtual void UpdateFolderL(const TDesC& aName);

        /**
        * Called when a new folder is created -- this is called 
        * after IsValidFolderName is called.
        *
        * @since 3.0
        * @param aName The feed's new name.
        * @return void.
        */
        virtual void NewFolderL(const TDesC& aName);

        /**
        * Validates the folder's updated name.
        *
        * @since 3.0
        * @param aName The feed's new name.
        * @param aIsEditing True if the edit dialog was opened to edit a folder.        
        * @return ETrue if the value is valid.
        */
        virtual TBool IsFolderNameValidL(const TDesC* aName, TBool aIsEditing);


    public: // New methods
        /**
        * Called to notify the container that root folder has changed.
        *
        * @since 3.1
        * @param aRootFolder The new root folder.
        * @return Void.
        */
        void RootFolderChangedL(const CFeedsEntity& aRootFolder);
    
        /**
        * Update the view's title.
        *
        * @since 3.0
        * @return void.
        */
        void UpdateTitleL();

        /**
        * Returns the current folder-item.
        *
        * @since 3.0
        * @return The current folder-item..
        */
        CFeedsEntity* CurrentItem() const;


        /**
        * Returns the index of the current folder-item.
        *
        * @since 3.0
        * @return The index of the current folder-item..
        */
        TInt CurrentItemIndex() const;

        /**
        * Updates the current folder-item.
        *
        * @since 3.0
        * @param aName The new name.
        * @param aUrl The new url.
        * @param aFreq The frequency for auto updating.
        * @return void.
        */
        void UpdateCurrentItemL(const TDesC* aName, const TDesC* aUrl, TInt aFreq);

        /**
        * Add a new feed.
        *
        * @since 3.0
        * @param aName The new name.
        * @param aUrl The new url.
        * @param aFreq The frequency for auto updating.
        * @return void.
        */
        void AddNewFeedL(const TDesC& aName, const TDesC& aUrl, TInt aFreq);

        /**
        * Add a folder feed.
        *
        * @since 3.0
        * @param aName The new name.
        * @return void.
        */
        void AddNewFolderL(const TDesC& aName);

        /**
        * Delete the current feed or folder
        *
        * @since 3.0
        * @return void.
        */
        void DeleteCurrentItemL();

        /**
        * Returns an array containing the indexes of the marked items.
        *
        * @since 3.0
        * @return The array.
        */
        const CArrayFix<TInt>* MarkedItems();

        /**
        * Replaces the current folder with its parent.
        *
        * @since 3.0
        * @return Returns ETrue if iCurrentFolder != iRootFoler.
        */
        TBool HandleShowParentFolderL(void);

        /**
        * Refreshes the root folder.
        *
        * @since 3.0
        * @return void
        */
        //void HandleRefreshRootFolderL(void);

        /**
        * Handles selections and open commands
        *
        * @since 3.0
        * @return void
        */
        void HandleSelectionL(void);

        /**
        * Handles updating the current Feed.
        *
        * @since 3.1
        * @return void
        */
        void HandleUpdateFeedL(TBool aUpdateAll = EFalse);

        /**
        * Handles the add-feed command.
        *
        * @since 3.0
        * @return void.
        */
        void HandleNewFeedL();

        /**
        * Handles the add-folder command.
        *
        * @since 3.0
        * @return void.
        */
        void HandleNewFolderL();

        /**
        * Handles the edit command.
        *
        * @since 3.0
        * @return void.
        */
        void HandleEditL();

        /**
        * Handles the delete command.
        *
        * @since 3.0
        * @return void.
        */
        void HandleDeleteL();

        /**
        * Handles the move command.
        *
        * @since 3.0
        * @return void.
        */
        void HandleMoveL();

        /**
        * Handles the move to folder command.
        *
        * @since 3.0
        * @return void.
        */
        void HandleMoveToFolderL();

        /**
        * Handles the mark-related commands.
        *
        * @since 3.0
        * @param aCommand The command to process.
        * @return void.
        */
        void HandleMarkL(TInt aCommand);

        /**
        * Handles the ok commands.
        *
        * @since 3.1
        * @return void.
        */
        void HandleOkL();

        /**
        * Handles the cancel commands.
        *
        * @since 3.0
        * @return void.
        */
        void HandleCancelL();
        
        /**
        * Handles the export command.
        *
        * @since 3.2
        * @return void.
        */
        void HandleExportL();

        /**
        * Calculate the unread count based on delta.
        *
        * @since 3.1
        * @return the calculated value.
        */
        TInt UnreadCountChangedL( TInt aUnreadCountDelta );

        /**
        * Set the unread count.
        *
        * @since 3.1
        * @return void.
        */
        void SetUnreadCountL( TInt aUnreadCount );

        /**
        * Count the number of folders in a given feeds folder
        *
        * @since 5.0
        * @param folder to count
        * @param total count folders
        * @return void 
        */
        void CountFolderFolders(const CFeedsEntity* aFolder, TInt &aFolderCount);

        /**
        * Update the toolbar buttons' state
        *
        * @since 5.0
        * @return void
        */
        void UpdateToolbarButtonsState();

        /**
        * Recursive loop to find a feed
        *
        * @since 5.0
        * @return ETrue if feed item is found
        */
        TBool SearchForFeedL(const CFeedsEntity* item);
		
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF
		/**
        * To know Whether any feed item selected for move
        *
        * @since 7.2
        * @return ETrue if feed item is selected for move
        */
        TBool isMoveActive(){ return iMoveActive;}
#endif        

    private:
        /**
        * C++ default constructor.
        */
        CFeedsFolderContainer
                    ( CFeedsFolderView* aView,
                             MApiProvider& aApiProvider );
        
        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL(const TRect& aRect);

        /**
        * Inits the container.
        *
        * @since 3.0
        * @param aRect The drawing rect.
        * @return void
        */
        void InitContainerL(const TRect& aRect);

        /**
        * Inits the array of needed icons.
        *
        * @since 3.0
        * @return void.
        */
        void InitIconArrayL();

        /**
        * Loads and appends an icon to the icon array..
        *
        * @since 3.0
        * @param aIcons The icon array.
        * @param aID Item ID of the masked bitmap to be created.
        * @param aFilename Filename to be used to construct the item, 
        *        if no matching item was found in the currently active skin.
        * @param aFileBitmapId ID of the bitmap in the file. 
        *        Used only if no matching item was found in the currently 
        *        active skin.
        * @param aFileMaskId ID of the mask in the file.
        *        Used only if no matching item was found in the currently
        *        active skin.
        * @return void.
        */
        void AppendIconL(CArrayPtr<CGulIcon>* aIcons, const TAknsItemID& aID, 
                const TDesC& aFilename, const TInt aFileBitmapId, const TInt aFileMaskId);
        
        /**
        * Sets the list-box's values.
        *
        * @since 3.0
        * @param aInitialSelection The index of the selection after the update.
        * @return void
        */
        void UpdateListBoxL(TInt aInitialSelection = 0);

        /**
        * Inits the move-to-folder pop-up.
        *
        * @since 3.0
        * @param aListBox The pop-up's listbox.
        * @param aPopupList The pop-up.
        * @return void
        */
        void MoveToFolderInitL(CAknSingleGraphicPopupMenuStyleListBox*& aListBox, 
                CAknPopupList*& aPopupList);

        /**
        * Sets the move-to-folder list-box's values.
        *
        * @since 3.0
        * @param aListBoxRows Holds the values in the pop-up.
        * @return void
        */
        void MoveToFolderSetItemsL(CDesCArraySeg& aListBoxRows);
        
        void AddFoldersToListL(const CFeedsEntity& aFolder, CDesCArraySeg& aListBoxRows);

        /**
        * Moves the marked items into the target-folder.
        *
        * @since 3.0
        * @param aTargetFolder The folder where the items will be moved.
        * @return void
        */
        void MoveToFolderMoveL(const CFeedsEntity& aTargetFolder);

        /**
        * Show the feed in the TopicView.
        *
        * @since 3.0
        * @param aFolderItem The folder item to show.
        * @return void
        */
        void ShowFeedL(const CFeedsEntity& aFolderItem);

        /**
        * Moves the current or marked items -- called after the move-cmd is confirmed.
        *
        * @since 3.0
        * @return void
        */        
        void PostHandleMoveL();

        /**
        * Updates the options softkey to reflect the current state.
        *
        * @since 3.0
        * @return void.
        */
        void UpdateCbaL();
        
        /**
        * Changes the current folder.
        *
        * @since 3.1
        * @param aFolder The new current folder.
        * @param aResetSelection If ETrue the current selection is set to the first item.
        * @return void.
        */
        void SetCurrentFolder(const CFeedsEntity* aFolder, 
                TBool aResetSelection = EFalse);

        /**
        * Dynamically initializes the Mark/Unmark submenu.
        *
        * @since 3.2
        * @param aMenu Pointer to the Menu Pane
        * @return - iListBox.
        */

        void DynInitMarkUnmarkMenuPane(CEikMenuPane* aMenuPane);

        /**
        * Dim or Undim toolbar
        * 
        * @since 5.0
        * @param aDim - is ETrue, then dim toolbar ; else undim toolbar
        * @return - void
        */

        void DimToolbar(TBool aDim);
        
        /**
        * Check for Marked items are folder or feed
        * 
        * @since 5.0
        * @return - bool
        */

        TBool IsMarkedItemFolder();        

        /**
        * Clears the navigation pane.
        *
        * @since 7.1
        * @return void
        */
        void ClearNavigationPane();

        /**
        * Handles the changes needed to the Navigation Pane.
        *
        * @since 7.1
        * @return void
        */
        void UpdateNavigationPaneL();
    public:
        friend class CFeedsFolderView;
        
        
    private:
    	CFeedsFolderView* 				iView;				// not owned
    	MApiProvider& 					iApiProvider;       // not owned
        CAknDoubleGraphicStyleListBox*  iListBox;
        CDesCArraySeg*                  iListBoxRows;
        CAknIconArray*                  iListBoxIcons;
        TBool                           iOwnsListBoxIcons;

        TBool                           iMoveActive;

        const CFeedsEntity*              iRootFolder;
        const CFeedsEntity*              iCurrentFolder;
        TInt                            iCurrentFolderId;
        RPointerArray<CFeedsEntity>      iTargetFolderList;  // pointers not owned
        CAknNavigationDecorator*        iNaviDecorator;
    };

#endif      // FEEDS_FOLDER_CONTAINER_H
            
// End of File
