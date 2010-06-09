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
*     Browser content view
*
*
*/


#ifndef __BROWSERCONTENTVIEW_H
#define __BROWSERCONTENTVIEW_H

//  INCLUDES
#include <apparc.h>
#include <sendui.h>
#include <FindItemDialog.h>
#include <brctlinterface.h>

#include "BrowserAppViewBase.h"
#include "BrowserGotoPane.h"
#include <AknToolbar.h>
#include <AknToolbarObserver.h>
#include <aknappui.h>
#include "WindowObserver.h"

//  FORWARD DECLARATIONS

class TVwsViewId;
class CBrowserContentViewContainer;
class CBrowserContentViewZoomModeTimer;
class CBrowserBookmarksModel;
class CAknNavigationControlContainer;
class CFindItemMenu;
class CAknInfoPopupNoteController;
class CAknStylusPopUpMenu;
class CBrowserContentViewToolbar;
class CBrowserShortcutKeyMap;

//  CONSTANTS

const TInt KStatusMessageMaxLength=40;

//The Zoom tooltip won't disappear
const TInt KZoomModeShowDisableTime( 5* 1000 * 1000 ); // 5000 seconds - disable the timer


// STRUCTS

struct TWindowsMenuItemsDimCheck
    {
    TBool dimOpenInNewWin;
    TBool dimSwitchWin   ;
    TBool dimCloseWin    ;
    TBool dimAllowPopups ;
    TBool dimBlockPopups ;
    };

/**
*  Declaration for Browser's content view.
*
*  @lib Browser.app
*  @since Series 60 1.2
*/
class CBrowserContentView : public CBrowserViewBase,
                            public MGotoPaneObserver,
                            public MBrCtlStateChangeObserver,
                            public MBrCtlCommandObserver,
                            public MAknToolbarObserver,
                            public MWindowObserver
    {
    public:

        /**
        * Two-phased constructor.
        * @param aApiProvider Reference to the API provider.
        * @param aRect
        */
        static CBrowserContentView* NewLC( MApiProvider& aApiProvider, TRect& aRect );

        /**
        * Destructor.
        */
        ~CBrowserContentView();

        /**
        * Callback - we should initialize the list of DO elements.
        * @since 1.2
        */
        void OptionListInitL();

        /**
        * Callback - we should add an element to the list of DO elements.
        * @since 1.2
        */
        void OptionListAddItemL(
            const TDesC& aText,
            TUint32 aElemID,
            TBool aIsPrevType );

        /**
        * Update the title.
        * @since 1.2
        */
        void UpdateTitleL( MApiProvider& aApiProvider );

        CBrowserContentViewContainer* Container() const{ return iContainer; };

        void ResetPreviousViewFlag() { iWasInFeedsView = EFalse; }
        /**
        * Get the top left position of the application window.
        * This is used to place the toolbar.
        * @return The top left point of the application rect on the screen
        */
        TPoint OnScreenPosition();

        /**
        * Get a pointer to the instance of the class handling the toolbar state.
        * This is used when the Browser is shut down, to save the user settings
        * @return A pointer to the class handling the toolbar
        */
        CBrowserContentViewToolbar* BrowserContentViewToolbar() { return iBrowserContentViewToolbar; }

        /**
        * Find if the platform spports touch
        * @return ETrue if the platform supports touch, EFalse otherwise
        */
        TBool PenEnabled() { return iPenEnabled; }

        /**
        * Find if the Full Screen mode is on
        * @return ETrue if the Full Screen mode is on, EFalse otherwise
        */
        TBool FullScreenMode() { return iContentFullScreenMode; }
    public:     // from MEikStatusPaneObserver

        /**
        * Handles status pane size change.
        */
        void HandleStatusPaneSizeChange();

    public:     // from MBrCtlStateChangeObserver

        /**
        * State change event handling for ImageMapView changes.
        * @param aState which state we are in
        * @param aValue state specific value
        * @return void
        */
        void StateChanged( TBrCtlDefs::TBrCtlState aState, TInt aValue );

    public:     // from MBrCtlCommandObserver

        /**
        * Executes commands requested by the webengine
        * @param aCommand which state we are in
        * @param aAttributesNames name list
        * @param aAttributeValues value list
        * @return void
        */
        void HandleCommandL(TBrCtlDefs::TBrCtlClientCommands aCommand, const CArrayFix<TPtrC>& aAttributesNames,
                                     const CArrayFix<TPtrC>& aAttributeValues) ;
        /* Handle Goto Pane event.
        * @since 1.2
        * @param aGotoPane The Goto Pane in which the event occurred.
        * @param aEvent Goto Pane event
        */
        void HandleGotoPaneEventL(
            CBrowserGotoPane* aGotoPane,
            MGotoPaneObserver::TEvent aEvent );

        /**
        * Set Last visited bookmark.
        * @since 1.2
        */
        void SetLastVisitedBookmarkL();

        /**
        * Update navi pane.
        * @since 1.2
        * @param aStatusMsg Status message
        */
        void UpdateNaviPaneL( TDesC& aStatusMsg );

        /**
        * Update navi pane.
        * @since 1.2
        * @param aLeftScrollBar Will the left scrollbar be updated
        * @param aRightScrollBar Will the right scrollbar be updated
        */
        void UpdateNaviPaneL( TBool aLeftScrollBar, TBool aRightScrollBar );

        /**
        * Get the id.
        * @since 1.2
        * @return UID
        */
        TUid Id() const;

        /**
        * Is find item in progress.
        * @since 1.2
        * @return ETrue if the find item is in progress, otherwise EFalse
        */
        TBool FindItemIsInProgress();

        /**
        * Handles client rect changes
        */
        void HandleClientRectChange();

        /**
        * Zoom page in zoom mode
        * @since 3.2.3
        */
		void ZoomModeImagesL();

        /**
        * Zoom page in
        * @since 3.0
        */
        void ZoomImagesInL( TInt aDuration = KZoomModeShowDisableTime );

        /**
        * Zoom page out
        * @since 3.0
        */
        void ZoomImagesOutL( TInt aDuration = KZoomModeShowDisableTime );

        /**
        * Zoom slider is to be visible or not in Content View.
        * @param aVisible. ETrue if the slider is to visible, EFalse if not.
        * @since 5.0
        */
        void MakeZoomSliderVisibleL( TBool aVisible );

        /**
        * Is ZoomSlider is up?
        * @since 5.0
        * @return ETrue if the ZoomSlider is up, otherwise EFalse
        */
        TBool ZoomSliderVisible() { return iZoomSliderVisible; }

        /**
        * Is History view is up?
        * @since 3.0
        * @return ETrue if the History View is up, otherwise EFalse
        */
        TBool IsHistoryViewUp();

        /**
        * Is History view at the beginning i.e. first page in history
        * @since 5.0
        * @return ETrue if the History is at the beginning, otherwise EFalse
        */
        TBool IsHistoryAtBeginning();

        /**
        * Is History view at the end i.e. last page in history
        * @since 5.0
        * @return ETrue if the History is at the end, otherwise EFalse
        */
        TBool IsHistoryAtEnd();

        /**
        * Is Shortcut Keymap view is up?
        * @since 5.0
        * @return ETrue if the Shortcut Keymap is up, otherwise EFalse
        */
        TBool KeymapIsUp();

        /**
        * Return true if we're in zoom mode
        */
        TBool isZoomMode();

        /**
        * Gets Miniature view ( old name: Thumbnail view ) status.
        * @since 3.0
        * @return True if Miniature view  is active, othervise False.
        */
        inline TBool IsMiniatureViewUp() { return iThumbnailViewIsUp; }

        /**
        * check if the plugin player is up
        * @since 3.2
        * @return True if Miniature view  is active, othervise False.
        */
        inline TBool IsPluginPlayerUp() const   { return iPluginPlayerIsUp; }


        /**
        * Find specifies keyword on page. Set aFindString to NULL if
        * searching in directions.
        * @since 3.0
        * @param aFindString The string to find on page. If seaching for direction
        *                    this must be NULL.
        * @param aFindDirection Search direction. True: next, otherwise: previous.
        */
        void FindKeywordL( HBufC* aFindString, TBool aFindDirection = EFalse );

        /**
        * Handles opening a feed with the url
        * @since 3.1
        * @param aUrl - the url of the feed
        * @return void
        */
        void HandleSubscribeToWithUrlL(TPtrC aUrl);

        /**
        * Check status of contentview fullscreen mode
        * @since 7.x
        * @return True if currently in fullscreen mode, othervise False.
        */
        inline TBool FullScreenMode() const { return iContentFullScreenMode; }

    public:     // from MAknToolbarObserver
        /**
         * Should be used to set the properties of some toolbar components
         * before it is drawn.
         * @param aResourceId The resource ID for particular toolbar
         * @param aToolbar The toolbar object pointer
         */
        void DynInitToolbarL( TInt /*aResourceId*/, CAknToolbar* /*aToolbar*/ );

        /**
         * Handles toolbar events for a certain toolbar item.
         * @param aCommand The command ID of some toolbar item.
         */
        void OfferToolbarEventL( TInt aCommand ) { HandleCommandL(aCommand);}

    public:     // from MWindowObserver
        void WindowEventHandlerL( TWindowEvent aEvent, TInt aWindowId );

        // from CAknView

        /**
        * Handles commands.
        * @param aCommand Command to be handled
        */
        void HandleCommandL( TInt aCommand );

        // from CAknView

	    /**
	     * From @c MEikMenuObserver. Menu emphasising or de-emphasising function.
	     * @c CEikMenuBar objects call this on their observer. Updates the value of
	     * the flags for the (@c aMenuControl) on the control stack.
	     * @param aMenuControl The control to be emphasised or de-emphasisied.
	     * @param aEmphasis @c ETrue to emphasize the menu, @c EFalse to refuse the
	     *          focus.
	     */
        void SetEmphasis(CCoeControl* aMenuControl,TBool aEmphasis);
        
        TRect ResizeClientRect();
        void SetContentContainerRect();
        
    private:

        /**
        * Generic Zoom function
        * @since 3.0
        * @param aDirection 1 for In, -1 for Out
        * @param aLow low boundary for zoom level checking
        * @param aHigh high boundary for zoom level checking
        * @param aDuration the time to display zoom indicator
        */
        void ZoomImagesL( TInt aDirection, TUint aLow, TUint aHigh, TInt aDuration );

        TInt FindCurrentZoomIndex(TInt aCurrentZoomLevel);

		void SaveCurrentZoomLevel(TBool saveZoom);

    protected:      // from CAknView

        /**
        * @since 1.2
        * @param aPrevViewId Previous view id
        * @param aCustomMessageId
        * @param aCustomMessage
        */
        void DoActivateL(
            const TVwsViewId& aPrevViewId,
            TUid aCustomMessageId,
            const TDesC8& aCustomMessage );

        /**
        * @since 1.2
        */
        void DoDeactivate();

    protected:      // from CBrowserViewBase

        /**
        * @since 1.2
        */
        TInt CommandSetResourceIdL();

		/**
		* Set command set lsk,rsk,msk dynamically via pointers.
		* Derived classes should implement, though it can be empty.
		* If it does nothing, empty softkeys will be assigned
		* @since 5.0
		*/
		void CommandSetResourceDynL(TSKPair& /*lsk*/, TSKPair& /*rsk*/, TSKPair& /*msk*/) {};


    private:

        /**
        * @param aApiProvider Reference to API provider
        */
        CBrowserContentView( MApiProvider& aApiProvider );

        /**
        * Symbian OS contructor.
        * @param aRect
        */
        void ConstructL( TRect& aRect );

        /**
        * Send address.
        */
        void SendAddressL( );

        /**
        * This method is used to update the Option menu label text
        * @param aMenuPane Menu pane to be configured.
        * @param aCommandId Identifies the menu item (as defined in .hrh file) to be updated.
        * @param aResourceId New label resource ID
        */
        void UpdateOptionMenuItemLabelL(
            CEikMenuPane* aMenuPane,
            const TInt aCommandId,
            const TInt aResourceId );

        /**
        * Do search for specified items in current page.
        * @param aSearchCase items to be searched for
        */
        void DoSearchL( CFindItemEngine::TFindItemSearchCase aSearchCase );

        /**
        *
        */
        void SetNavipaneViewL();

        /**
        * Force updating the Goto Pane. Does nothing if Goto Pane is not
        * visible.
        */
        void UpdateGotoPaneL();

        /**
        * View history pane
        */
        void ViewHistoryL();

        /**
        * Dynamically initialize menu panes.
        * @param aResourceId ID for the resource to be loaded
        * @param aMenuPane Menu pane
        */
        void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

        /**
        * Handles a selection from the subscribe to sub-menu.
        * @since 3.0
        * @param aCommand - A menu command
        * @return void
        */
        void HandleSubscribeToL(TInt aCommand);

        /**
        * Deals with menu items for Multiple Windows Support
        * @since 3.1
        * @param aWindowMenuItems - bool for each window submenu item that is set to true if it should be dimmed ; false if not
        * @return true is all items were true otherwise false is returned.
        */
        TBool CheckForEmptyWindowsMenuL(TWindowsMenuItemsDimCheck* aWindowMenuItems);

        /**
        * Deals with dimming menu items for Multiple Windows Support
        * @since 3.1
        * @param aMenuPane - A menu pane
        * @param aWindowMenuItems - lists each window submenu item and whether it should be dimmed
        * @return void
        */
        void DimMultipleWindowsMenuItems( CEikMenuPane& aMenuPane, TWindowsMenuItemsDimCheck aWindowMenuItems);

        /**
        * Check if the input is activate
        * @param none
        * @return ETrue if input is
        */
		TBool IsEditMode();
    private:

        void ProcessCommandL( TInt aCommand );

        void GotoUrlInGotoPaneL();

        /**
        * Adds the saved bookmark's id to the bottom of the list.
        * @param aUid Uid of the bookmark
        */
        void AddBMUidToLastPlaceToCurrentListL( const TInt aUid );

        /**
        * callback handler for fullscreen status pane timer
        */
        static TInt CallHideFsStatusPane(TAny* aCBrowserContentView );

        /**
        * Hides the status pane for fullscreen mode and cancels timer
        */
        void HideFsStatusPane();

        /**
        * Callback handler for auto fullscreen mode timer
        */
		static TInt CallActivateAutoFullScreen(TAny* aCBrowserContentView);

        /**
        * Activate Automatic Full screen mode
        */
		void ActivateAutoFullScreenMode();

        /**
        * Start auto fullscreen timer
        */
		void StartAutoFullScreenTimer();

        /**
        * Suspend auto fullscreen timer
        */
		void SuspendAutoFullScreenTimer();

        /**
        * Start auto fullscreen Idle timer
        */
		void StartAutoFullScreenIdleTimer();

        /**
        * Suspend auto fullscreen Idle timer
        */
		void SuspendAutoFullScreenIdleTimer();
    public:

        // BrowserContentViewContainer can use this method to pop up 'Go to' pane.
        void LaunchGotoAddressEditorL();

        void UpdateFullScreenL();

        void SetFullScreenOffL();

        /**
        * Launches the Find keyword pane.
        * @since 3.0
        */
        void LaunchFindKeywordEditorL();

        /**
        * Initializes iZoomLevelArray, iCurrentZoomLevelIndex, iCurrentZoomLevel,
        * iZoomInVisible, iZoomOutVisible members.
        *
        * Note: these members must be initialized later, after the BrowserContentView was constructed
        * in BrowserAppUi, because initializing zooming, requires iBrowserControl, which is created in
        * BrowserAppUi::ConstructL(), and creating  iBrowserControl requires a created BrowserContentView
        * object. So zoom levels must be initialized after BrowserAppUi created the iBrowserControl member.
        * @since 3.0
        */
        void SetZoomLevelL();

        /**
        * Shows zoom level text in title pane for aDuration seconds.
        * @since 3.0
        * @param aResourceId Resource of the zoom level string
        * @param aLevel The zoom level which will be shown formatting
        *               with the resource string.
        */
        void SetZoomLevelTitleTextL( TInt aResourceId , TInt aDuration = KZoomModeShowDisableTime );


        /**
        * Gets the text zoom levels.
        */
        void GetTextZoomLevelsL();

        /**
        * Handle zoom mode command
        */
        void SetZoomModeL( TBool aZoomMode );

        /**
        * Handle pointer events when Goto pane is visible
        */
        void HandlePointerCommand(TInt aCommand) { TRAP_IGNORE(HandleCommandL(aCommand)); }

        /**
        * Add new bookmark to the database.
        * @param aAsLastVisited ETrue if the meaning is to update "Last visited" bookmark
        * @param aUrl The URL that should be bookmarked
        */
        void AddNewBookmarkL( TBool aAsLastVisited, HBufC* aUrl = NULL );

        void ShowKeymap();

        void HideKeymap();

        void RedrawKeymap();

        /**
        * Set whether in content view full screen mode or not.
        * @since 5.0
        * @param aEnableFullScreen boolean. True: Enter fullscreen, False: Exit
        * @return void
        */
        void EnableFullScreenModeL( TBool aEnableFullScreen );

        void HandlePluginFullScreen(TBool aFullScreen);

        /**
        * Show or hide status pane for fullscreen mode
        * @since 7.x
        * @param aShow boolean. True: show status pane, False: start timer that hides status pane on timeout
        * @return void
        */
        void ShowFsStatusPane(TBool aShow);
        
        void setFullScreenFlag(){ iWasContentFullScreenMode = iContentFullScreenMode; }
		
		void  CBrowserContentView::HandleStatusPaneCallBack();
        
    private:

        CAknNavigationControlContainer* iNaviPane;
        CAknNavigationDecorator* iNaviDecorator;
        TBufC<KStatusMessageMaxLength> iStatusMsg;
        CBrowserBookmarksModel* iBookmarksModel;
        CBrowserContentViewContainer* iContainer;


        // Find keyword pane's keyword
        HBufC* iEnteredKeyword;

        // Zoom images...
        // Array contains the zoom levels. ( 30%, 40%, ... defined in webengine)
        RArray<TUint>* iZoomLevelArray;

        // The index of the zoom level in iZoomLevelArray.
        TUint iCurrentZoomLevelIndex;
        // Zoom text...
        // Array contains the text zoom levels. ( All small,Small,Normal )
        CArrayFixFlat<TInt>* iTextZoomLevelArray;
        // The index of the zoom level in iTextZoomLevelArray.
        // This index equals to text sizes in EAllSmall...
        TUint iCurrentTextZoomLevelIndex;
        CArrayFixFlat<TInt>* iFontSizeArray;

        TBool iHistoryViewIsUp;
        TBool iThumbnailViewIsUp;
        TBool iSynchRequestViewIsUp;
        TBool iImageMapActive;
        CAknInfoPopupNoteController* iToolBarInfoNote;

        TBool iPluginPlayerIsUp;
        TBool iSmartTextViewIsUp;

        TBool iFindItemIsInProgress;
        //Flag to check if the last view was Feeds engine
        TBool iWasInFeedsView;
        CAknStylusPopUpMenu* iStylusPopupMenu; // owned
        CBrowserContentViewToolbar* iBrowserContentViewToolbar; // owned
        TBool iPenEnabled;
        CBrowserShortcutKeyMap* iShortcutKeyMap; // owned

        // True if the ZoomSlider is currently visible
        TBool iZoomSliderVisible;
        // True if in zoom mode, zoom indicator (tooltip or slider) displayed
        TBool iZoomMode;
        CBrowserContentViewZoomModeTimer *iZoomModeTimer;

        // True if the browser is in Full Screen Mode in ContentView
        TBool iContentFullScreenMode;
        TBool iHistoryAtBeginning;
        TBool iHistoryAtEnd;
        TBool iIsPluginFullScreenMode;

        CAknAppUiBase::TAppUiOrientation iOrientation;
        TBool iWasContentFullScreenMode;
        CPeriodic *iPeriodic;
        TUint iCurrentZoomLevel;
        TUint iSavedZoomLevel;
        TBool iOptionsMenuActive;
        CPeriodic *iAutoFSPeriodic;
        CPeriodic *iIdlePeriodic;
        TBool iFullScreenBeforeEditModeEntry;
        HBufC* iTitle;
    };

#endif

// End of File
