/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
*      API provider.
*  
*
*/


#ifndef MAPIPROVIDER_H
#define MAPIPROVIDER_H

// INCLUDE FILES
#include "WmlBrowserBuild.h"
#include "BrowserLoadObserver.h"  // for LoadUrlType
#ifdef __RSS_FEEDS
#include "FeedsClientUtilities.h"
#endif

// FORWARD DECLARATIONS

class MPreferences;
class MCommsModel;
class MConnection;
class MDisplay;
class MBmOTABinSender;

//class MStartPageHandler;
class CBrCtlInterface;
class CBrowserDialogsProvider;
class CFavouritesItem;
class TFavouritesWapAp;
class RHttpDownloadMgr;
class CDownloadMgrUiDownloadsList;
class CBrowserSpecialLoadObserver;
class CBrowserSoftkeysObserver;
class CBrowserPopupEngine;
class CBrowserWindowManager;

// CLASS DECLARATION

/**
*  Interface to query several API implementations.
*  
*  @lib Browser.app
*  @since Series 60 1.2
*/
class MApiProvider
    {
    public: // New functions

        /**
        * Returns reference to the implementation of MDisplay.
        * @since Series 60 1.2
        * @return An implementation of MDisplay
        */
        virtual MDisplay& Display() const = 0;

	    /**
	    * Returns a reference to the implementaion of MConnection,
	    * through wich some high-level connection specific data can be set
	    * @since Series 60 1.2
        * @return reference to MConnection object
	    */
        virtual MConnection& Connection() const = 0;

        /**
        * Returns reference to the implementation of MPrefences.
        * @since Series 60 1.2
        * @return An implementation of MPrefences
        */
        virtual MPreferences& Preferences() const = 0;

        /**
        * Returns reference to the implementation of MCommsModel.
        * @since Series 60 1.2
        * @return An implementation of MCommsModel
        */
        virtual MCommsModel& CommsModel() const = 0;

        /**
        * Returns reference to the CBrCtlInterface
        * @since Series 60 2.8
		* @return A reference to CBrCtlInterface
        */        
		virtual CBrCtlInterface& BrCtlInterface() const = 0;

		/**
        * Returns reference to CBrowserLoadObserver.
        * @since Series 60 2.8
        * @return An implementation of MBrCtlLoadEventObserver
        */
		virtual CBrowserLoadObserver& LoadObserver() const = 0;

		virtual CBrowserSpecialLoadObserver& SpecialLoadObserver() const = 0;

		virtual CBrowserSoftkeysObserver& SoftkeysObserver() const = 0;
		/**
        * Returns reference to CBrowserDialogsProvider.
        * @since Series 60 2.8
        * @return An implementation of MBrCtlDialogsProvider
        */   
		virtual CBrowserDialogsProvider& DialogsProvider() const = 0;

        /**
        * Returns reference to the implementation of MBmOTABinSender.
        * @since Series 60 1.2
        * @return An implementation of MBmOTABinSender
        */
		virtual MBmOTABinSender& BmOTABinSenderL() = 0;

        virtual CBrowserWindowManager& WindowMgr() const = 0;

        virtual CBrowserPopupEngine& PopupEngine() const = 0;

        /**
		* Sets uid which should be activated if needed
        * If not, it activates view immediately.
        * @since Series 60 1.2
		* @param aUid Uid of the view
		*/
        virtual void SetViewToBeActivatedIfNeededL( TUid aUid, TInt aMessageId = 0 ) = 0;

        /**
        * In some cases it's not permitted to cancel fetching process.
        * @since Series 60 1.2
        * @return true is user can cancel fetch
        */
        virtual TBool IsCancelFetchAllowed() const = 0;

		/**
        * Returns Uid of the last used view.
        * @since Series 60 1.2
		* @return Uid of the last used view
		*/
        virtual TUid LastActiveViewId() const = 0;

		/**
		* Sets uid of the last used view
        * @since Series 60 1.2
		* @param aUid Uid of the latest used view
		*/
        virtual void SetLastActiveViewId( TUid aUid ) = 0;

		/**
		* Tells if WmlBrowser is in the middle of a shutdown process.
        * @since Series 60 1.2
		* @return ETrue if the app is being closed, otherwise with EFalse
		*/
		virtual TBool ExitInProgress() const = 0;

		/**
		* Sets a flag in WmlBrowser to indicate shutdown process.
        * @since Series 60 1.2
		* @param aValue Determines if exit is in progress or not
		*/
		virtual void SetExitInProgress( TBool aValue ) = 0;

		/**
        * Is there connection procedure ongoing?
        * @since Series 60 1.2
		* @return ETrue if connection procedure is ongoing
		*/
        virtual TBool IsConnecting() const = 0;

        /**
        * Updates progress bar in contentview.
        * @since Series 60 1.2
        * @param aStatusMsg Progress bar text
        */
        virtual void UpdateNaviPaneL( TDesC& aStatusMsg ) = 0;

        /**
        * Set view to return to when closing the content view.
        * @since Series60 1.2
        * @param aUid View uid
        */
        virtual void SetViewToReturnOnClose( TUid const &aUid ) = 0;
        
        /**
        * Close the content view by activating the bookmarks view
        * @since Series60 1.2
        */
        virtual void CloseContentViewL() = 0;  
		
		/**
        * Is embedded mode ongoing?
        * @since Series60 1.2
		* @return ETrue if embedded mode is ongoing
        */
        virtual TBool IsEmbeddedModeOn() const = 0;
        
        virtual TBool IsShutdownRequested() const = 0;

        virtual void ExitBrowser(TBool aUserShutdown) = 0;

		/**
		* Is the avkon appshutter running
		* @since Series60 2.0
		* @return ETrue if AppShutter is active
		*/
		virtual TBool IsAppShutterActive() const = 0;
        
        /**
        * Starts prefrences view if no valid ap 
        * @since Series60 2.0
        * @return ETrue if access point has been set.
        */
        virtual TBool StartPreferencesViewL( TUint aListItems ) = 0;

		/**
        * Returns ETrue if at least one page has been loaded.
        * @return   ETrue if at least one page has been loaded.\n
        *           EFalse if no pages has been loaded.
        */
        virtual TBool IsPageLoaded() = 0;

        /**
        *
        */
        virtual TBool IsForeGround() const = 0;

        /**
        * Returns fetching status.
        * @return ETrue if there is a fetch going on, EFalse otherwise.
        */
        virtual TBool Fetching() const = 0;

        /**
        * Returns content display status.
        * @return ETrue if content of a new page is displayed, EFalse otherwise.
        * (useful for zooming while page is loading)
        */
        virtual TBool ContentDisplayed() const = 0;

        /**
        * Sets the content display status.
        * (useful for zooming while page is loading)
        */
        virtual void SetContentDisplayed( TBool aValue ) = 0;
		
         /**
        * Initiate fetching of bookmark.
        * @param aBookmarkUid  Uid of bookmark to fetch.
        */
        virtual void FetchBookmarkL( TInt aBookmarkUid ) = 0;

		/**
        * Initiate fetching of bookmark.
        * @param aBookmarkItem Bookmark item to fetch.
        */
        virtual void FetchBookmarkL( const CFavouritesItem& aBookmarkItem ) = 0;

		/**
        * Initiate fetching of URL.
        * @param aUrl URL to fetch.
        * @param aUserName User name.
        * @param aPassword Password.
        * @param aAccessPoint AccessPoint.
        */
        virtual void FetchL(
							const TDesC& aUrl,
							const TDesC& aUserName,
							const TDesC& aPassword,
							const TFavouritesWapAp& aAccessPoint,
                            CBrowserLoadObserver::TBrowserLoadUrlType aUrlType
							) = 0;

        /**
        * Initiate fetching of URL with default AP, empty username and password.
        * @param aUrl URL to fetch.
        */
        virtual void FetchL( const TDesC& aUrl, CBrowserLoadObserver::TBrowserLoadUrlType aUrlType = CBrowserLoadObserver::ELoadUrlTypeOther ) = 0;

		/**
        * Cancel fetching.
        */
		virtual void CancelFetch( TBool aIsUserInitiated ) = 0;		

		/**
        * Sets requested AP.
        * @param aAp the AP to use when connectionneeded is called.
        */
		virtual void SetRequestedAP( TInt aAp ) = 0;

        virtual void SetExitFromEmbeddedMode( TBool aFlag ) = 0;

        //To check and set iShowProgress flag that indicates that
        //real downloading is going on ( we are not loading from cache )

        virtual TBool IsProgressShown() const = 0;

        virtual void SetProgressShown( TBool aProgressShown ) = 0;

        virtual TBool StartedUp() const = 0;

        virtual TUint32 RequestedAp() const = 0;
      
        /**
        * Logs a record of opened pages to AHLE
        */
        virtual void LogAccessToRecentUrlL( CBrCtlInterface& aBrCtlInterface ) = 0;
        
        /**
        * Logs a record of requested pages to Recent Url Store
        */
        virtual void LogRequestedPageToRecentUrlL( const TDesC& aUrl ) = 0;

		/**
		* Starts animation.
		*/
		virtual void StartProgressAnimationL() = 0;

		/**
		* Stops animation.
		*/
		virtual void StopProgressAnimationL() = 0;
		
#ifdef __RSS_FEEDS
        /**
        * Returns reference to FeedsClientUtilities.
        */
        virtual CFeedsClientUtilities& FeedsClientUtilities() const = 0;
#endif  // __RSS_FEEDS

	/**
        * Returns Uid of the previous active view from the view history.
        * @since Series 60 3.1
		* @return Uid of the previous active view
		*/
        virtual TUid GetPreviousViewFromViewHistory() = 0;

        /**
        * Set the View for in which the last CBA update was made
        * @since 5.0
        */
        virtual void SetLastCBAUpdateView( TUid aView ) = 0;		
		
        /**
        * Get the view in which the last CBA update was made
        * @since 5.0
        * @return TUid of last view for which CBA was updated
        */
        virtual TUid GetLastCBAUpdateView() = 0;
		

        /**
        * Return if Flash plugin is present in the system or not
        * @param none
        * @return TBool
        */
        virtual TBool FlashPresent( ) = 0;
        
        /**
		* Check state of LaunchHomePage command for dimmed state in options menu and shortcut keymap
		* @return ETrue is LaunchHomePage command should be dimmed in options menu and shortcut keymap
		*         EFalse otherwise
		* @param none
		*/	        
        virtual TBool IsLaunchHomePageDimmedL() = 0; 
       
        /**
        * Return if the Browser was originally launched directly into the feeds view
        * @param none
        * @return TBool
        */
        virtual TBool BrowserWasLaunchedIntoFeeds() = 0;
        
        /**
        * Set flag to ETrue if called from another application.
		* @since Series 60 1.2
        * @param aValue Is browser called from some another App.
        */
        virtual void SetCalledFromAnotherApp( TBool aValue ) = 0;

        /**
        * Get flag if called from another application.
		* @since Series 60 3.1
		* @return Etrue if browser is called from some another App.
        */
        virtual TBool CalledFromAnotherApp() = 0;
		/**
		* Check from CCoeAppUi if a Dialog or Menu is diplayed on top of the view
		* @return ETrue if Dialog or Menu is displayed
		*         EFalse otherwise
		* @param none
		*/		        
		virtual TBool IsDisplayingMenuOrDialog() = 0;

};

#endif

// End of File
