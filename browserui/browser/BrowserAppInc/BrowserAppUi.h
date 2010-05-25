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
* Description:  Header file for CBrowserAppUi
*
*
*/

#ifndef __BROWSERAPPUI_H
#define __BROWSERAPPUI_H

// INCLUDE FILES 
#include <browser_platform_variant.hrh>
#include <aknviewappui.h>
#include <connectionobservers.h>	// CHG!
#include <apgcli.h>
#include "ApiProvider.h"
#include "BrowserBuild.h"
#include "WmlBrowserBuild.h"
#include <recenturlstore.h>

#include <favouritesdb.h>
#include <downloadedcontenthandler.h>
#include "WindowInfoProvider.h"
#include <browseroverriddensettings.h>
#ifdef BRDO_IAD_UPDATE_ENABLED_FF
#include <iaupdateobserver.h>
#endif
// FORWARD DECLARATIONS
class CBrowserViewBase;
class MCommsModel;
class CBrowserCommsModel;
class CBrowserPreferences;
class CWmlBrowserBmOTABinSender;
class CBrowserContentView;
class CBrowserSoftkeysObserver;
class CBrowserSpecialLoadObserver;
class CBrowserAsyncExit;
class CBrowserLauncherService;

// Multiple Windows Support
class CBrowserPopupEngine;
class CBrowserWindow;
class CBrowserWindowManager;
class CBrowserBookmarksView;
#ifdef __RSS_FEEDS
	class CFeedsClientUtilities;
#endif  // __RSS_FEEDS

// CONSTANTS

//
// BROWSER UI KNOWN mime types
//


_LIT(KOPMLMimeType,      "text/x-opml");
_LIT(KMimeTypeAppXML,    "application/xml");
_LIT(KMimeTypeTextXML,   "text/xml");
//++PK
_LIT(KMimeTypeCapsTextXML,   "Text/xml");
//++PK
_LIT(KMimeTypeRSS,       "application/rss+xml");
_LIT(KMimeTypeAtom,      "application/atom+xml");
_LIT(KMimeTypeRDF,       "application/rdf+xml");


class CBrowserPushMtmObserver;
#ifdef BRDO_IAD_UPDATE_ENABLED_FF
class CIAUpdate; 
class CIAUpdateParameters;
#endif
// CLASS DECLARATION

/**
*  CBrowserAppUi.
* 
*  @lib Browser.app
*  @since Series 60 1.2 
*/
class CBrowserAppUi : public CAknViewAppUi,
                      public MApiProvider,
                      public MConnectionStageObserver,
                      public MDownloadedContentHandler,
                      public MWindowInfoProvider
					#ifdef BRDO_IAD_UPDATE_ENABLED_FF
                     ,public MIAUpdateObserver
					#endif
    {
    public:     // Constructors and destructor
        /**
        * Default constructor.
        */
        CBrowserAppUi();

        /**
        * Destructor.
        */
        ~CBrowserAppUi();

    public:
#if defined(__S60_50__)
        /**
        * Handling the system Event.
        * @since Series 60 1.2
        * @param aEvent The System event passed.
        */
                
        void HandleSystemEventL(const TWsEvent& aEvent);
#endif          
        /**
        * Get the active view.
		* @since Series 60 1.2
        * @return The current view.
        */
        CBrowserViewBase* ActiveView();

        /**
        * Checking if user wants to disconnect.
		* @since Series 60 1.2
        * @param aUserInitiated it can be closed by user or the system.
        */
        void ExitBrowser( TBool aUserInitiated );

        /**
        * Calls views' method (ALWAYS changes the view's CBA).
		* @since Series 60 1.2
        */
        void UpdateCbaL();

        /**
        * Handles commandline parameters.
		* @since Series 60 1.2
        * @param aDocumentName Parameter string
        * @param aDoFetch Indicates that this function shoud fetch the given
        *                 URL. This parameter is required and used 
        *                 by new embedding architecture.
        */
        void ParseAndProcessParametersL( const TDesC8& aDocumentName, TBool aDoFetch = ETrue );

        /**
        * Wait a while, contentview initializing itself
		* @since Series 60 3.2
        */
        void WaitCVInit();
        /**
        * Set flag to ETrue if called from another application.
		* @since Series 60 1.2
        * @param aValue Is browser called from some another App.
        */
        void SetCalledFromAnotherApp( TBool aValue );

        /**
        * Get flag if called from another application.
		* @since Series 60 3.1
		* @return Etrue if browser is called from some another App.
        */
        TBool CalledFromAnotherApp();

        /**
        * Override CAknViewAppUi::ProcessCommandL.  Calls CAknAppUi::StopDisplayingMenuBar() before iView one.
		* @since Series 60 
        * @param aCommand GUI command
        */
        void ProcessCommandL(TInt aCommand);

        /** 
        * Get the instance of the browser UI.
		* @since Series 60 1.2
        * @return The browser UI.
        */
        static CBrowserAppUi* Static();

        /**
        * Contentview.
		* @since Series 60 1.2
        */
        CBrowserContentView* ContentView() const;

        /**
		* Check Operator Menu or Video Services status
		* @since Series 60 2.0
		* @return ETrue if browser is running in Operator Menu or Video Services app
		*/
		TBool IsEmbeddedInOperatorMenu() const;

		/**
		* Check if avkon appshutter is running
		* @since Series60 2.0
		* @return ETrue if avkon appshutter is running
		*/
		TBool IsAppShutterActive() const;

		/**
		* Returns value of iSomeItemsNotSecure member 
		* @since Series60 2.0
		* @return ETrue if some items is unsecure in page
		*/
        TBool SomeItemsNotSecure() const;

        /**
        * Fetch the home page
        */
        void FetchHomePageL();

        /**
        * Check if the application is in foreground
        * @return ETrue if application is in foreground
        */
        inline TBool IsForeGround() const
            { return iIsForeground; }

        /**
        * Sets a private status to indicate that Browser has to use
        * a special exiting method.
        */
        inline void SetExitFromEmbeddedMode( TBool aFlag )
            { iExitFromEmbeddedMode = aFlag; }

        /**
        * Sets a the uid of the embedding application if the
        * Browser is embedded.
        */
        inline void SetEmbeddingAppliacationUid( TUid aEmbeddingApplicationUid )
        	{ iEmbeddingApplicationUid = aEmbeddingApplicationUid; }

        inline void SetBrowserLauncherService( CBrowserLauncherService* aBrowserLauncherService )
            { iBrowserLauncherService = aBrowserLauncherService; }

        inline CBrowserLauncherService* BrowserLauncherService() const
            { return iBrowserLauncherService; }

        void CloseWindowL( TInt aWindowId = 0 );

        inline CBrowserBookmarksView* GetBookmarksView()
            { return iBookmarksView; }
            
        void OpenLinkInNewWindowL( const CFavouritesItem& aBookmarkItem );

        /**
        * Launching HomePage (AP home page, or user-defined home page).
        */
        void LaunchHomePageL();

        /**
        * Set home page found flag
        * @param aPgNotFoundFlag: indicate if home page is found or not.
        */
        void SetPgNotFound(TBool aPgNotFoundFlag) {iPgNotFound = aPgNotFoundFlag;};

        /**
        * Catch any key presses that are Application wide.  Called from OfferKeyEventL() in view containers
        * @param aKeyEvent: 
        * @param aType:
        */
		TKeyResponse OfferApplicationSpecificKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType);

        /**
        * Sets requested AP.
        * @param aAp the AP to use when connectionneeded is called.
        */
        void SetRequestedAP( TInt aAp );

 		/**
        * Gets the currently focused image, and saves it to Gallery.
        */
        void SaveFocusedImageToGalleryL();

        /**
        * Check if shutdown was requested.
        * @return ETrue if shutdown was requested.
        */
        TBool IsShutdownRequested() const;

        /**
        * Check if a progress bar is shown.
        * @return ETrue if progress bar is shown.
        */
        TBool IsProgressShown() const;

        /**
        * Sets prograss bar showing status.
        * @param ETrue to show, EFalse to hide the progress bar
        */
        void SetProgressShown( TBool aProgressShown );

		/** 
		* Check if RFile content should be handled
		* by BrowserUI rather than sent off to BrCtl
		* @return ETrue if it should be handled by BrowserUI
		* @param an RFile filehandle, and an allocated buffer to hold
		*   discovered mime type
		*/
		TBool CBrowserAppUi::RecognizeUiFileL( RFile& aFileHandle, TBuf<KMaxDataTypeLength>& aDataTypeDes );
			
		/** 
		* Confirm with the user that a file should be handled by
		* BrowserUI according to it's mime type with appropriate
		* dialogs, etc. The name is used for the dialogs
		* @return ETrue if yes, EFalse otherwise
		* @param a filename to be shown in dialog, and a mime type
		*   to switch on so we can show different dialogs in different situations 
		*/
		TBool ConfirmUiHandlingL( TFileName& aFilename, TBuf<KMaxDataTypeLength>& aDataTypeDes );

		/** 
		* Given a file handle (RFile) and a file path,
		* copy the contents of the RFile to the file path given
		* @return void
		* @param a filehandle RFile, and a path to copy the contents to
		*/ 
		void CopyUiRFileToFileL( RFile& aFile, const TDesC& aFilepath );

		/** 
		* Handle a file that BrowserUi can process (as opposed to
		* BrCtl)
		* This will also setup views accordingly (the setup may be different from the HandleUiDownload case)
		* @return void
		* @param the full path of the filename to be handled and the mime type
		*   so that handling can be different for different mime types
		*/
		void HandleUiFileL( TFileName& aFilepath, TBuf<KMaxDataTypeLength>& aDataTypeDes );
		
		/** 
		* Handle a file that BrowserUi can process that is downloaded from the content view
		* This will also setup views accordingly (the setup may be different from the HandleUiFile case)
		* @return void
		* @param the full path of the filename to be handled and the mime type
		*   so that handling can be different for different mime types
		*/
		void HandleUiDownloadL( TFileName& aFilepath, TBuf<KMaxDataTypeLength>& aDataTypeDes );
	
		/**
		* Launch the browser so that it starts directly into the feeds view
		* @return void
		* @param none
		*/
		void LaunchIntoFeedsL();

    public: // From CEikAppUi
        /**
        * Handles user commands.
        * @param aCommand Command to be handled.
        */
        void HandleCommandL( TInt aCommand );
        void HandleWsEventL(const TWsEvent& aEvent,CCoeControl* aDestination);

    public: // Functions from MApiProvider

        /**
        * Returns reference to the implementation of MDisplay.
        * @return An implementation of MDisplay.
        */
        MDisplay& Display() const;

        /**
        * Returns a reference to the implementaion of MConnection,
        * through wich some high-level connection specific data can be set.
        * @return reference to MConnection object.
        */
        MConnection& Connection( ) const;

        /**
        * Returns reference to the implementation of MPrefences.
        * @return An implementation of MPrefences.
        */
        MPreferences& Preferences() const;

        /**
        * Returns reference to the implementation of MCommsModel.
        * @return An implementation of MCommsModel.
        */
        MCommsModel& CommsModel() const;

        /**
        * Returns reference to the CBrCtlInterface
		* @return A reference to CBrCtlInterface
        */
		CBrCtlInterface& BrCtlInterface() const;

		/**
        * Returns reference to CBrowserLoadObserver.
        * @return An implementation of MBrCtlLoadEventObserver
        */
		CBrowserLoadObserver& LoadObserver() const;

	    /**
        * Returns reference to CBrowserLoadObserver.
        * @return An implementation of MBrCtlLoadEventObserver
        */
		CBrowserSpecialLoadObserver& SpecialLoadObserver() const;

		CBrowserSoftkeysObserver& SoftkeysObserver() const;

		/**
        * Returns reference to CBrowserDialogsProvider.
        * @return An implementation of MBrCtlDialogsProvider
        */
		CBrowserDialogsProvider& DialogsProvider() const;

        CBrowserPopupEngine& PopupEngine() const;

        /**
        * Returns reference to the implementation of MBmOTABinSender.
        * @return An implementation of MBmOTABinSender.
        */
        MBmOTABinSender& BmOTABinSenderL();


        /**
        * Returns reference to the implementation of MStartPageHandler.
        * @return An implementation of MStartPageHandler.
        */
        CBrowserWindowManager& WindowMgr() const;

        /**
        *
        */
        inline TBool StartedUp() const { return iStartedUp; };

        /**
        *
        */
        void ConnNeededStatusL( TInt aErr );

        /**
        * Checks the last used AP.
        * @return the Id of the last used AP.
        */
        inline TUint32 RequestedAp() const { return iRequestedAp; }

        /**
        * In some cases it's not permitted to cancel fetching process.
        * @return ETrue if cancelling fetch is allowed, otherwise EFalse.
        */
        TBool IsCancelFetchAllowed() const;

        /**
        * Sets uid which should be activated if needed.
        * If not, it activates view immediately.
        * @param aUid Uid of the view.
        * @param aMessageId Id of the message.
        */
        void SetViewToBeActivatedIfNeededL( TUid aUid, TInt aMessageId = 0 );

        /**
        * Returns the Uid of the last visited Uid.
        * @return Uid of the last used view.
        */
        TUid LastActiveViewId() const;

        /**
        * Sets uid of the last used view.
        * @param aUid Uid of the latest used view.
        */
        void SetLastActiveViewId( TUid aUid );


        /**
        * Gets uid of the current view.
        * @param none
        * @return TUid of the current view
        */
        TUid GetCurrentViewId() const 
            {
            return iCurrentView;
            }
            
        /**
        * Set the View for in which the last CBA update was made
        * @since 5.0
        */
        void SetLastCBAUpdateView( TUid aView ) { iLastCBAUpdateView = aView; }
		
        /**
        * Get the view in which the last CBA update was made
        * @since 5.0
        * @return TUid of last view for which CBA was updated
        */
        TUid GetLastCBAUpdateView() { return iLastCBAUpdateView; }

        /**
        * Tells if Browser is in the middle of a shutdown process.
        * @return ETrue if the app is being closed, otherwise with EFalse.
        */
        TBool ExitInProgress() const;

        /**
        * Sets a flag in Browser to indicate shutdown process.
        * @param aValue ETrue if exit process will be started, otherwise EFalse.
        */
        void SetExitInProgress( TBool aValue );

        /**
        * Is Browser connection procedure ongoing or not.
        * @return ETrue if connection procedure is ongoing, otherwise EFalse.
        */
        TBool IsConnecting() const;

        /**
        * Updates progress bar in contentview.
        * @param aStatusMsg Progress bar text.
        */
        void UpdateNaviPaneL( TDesC& aStatusMsg );

        /**
        * Set view to return to when closing the content view.
        * @since Series60 1.2
        * @param aUid View uid
        */
        void SetViewToReturnOnClose( TUid const &aUid );

        /**
        * Close the content view by activating the bookmarks view
        * @since Series60 1.2
        */
        void CloseContentViewL();
		
		/**
        * Tells if Browser is in the embedded mode
        * @return ETrue if the Browser is in the embedded mode, 
        * otherwise with EFalse.
		*/
		TBool IsEmbeddedModeOn() const;

        /**
        * Starts prefrences view if no valid ap 
        * @since Series60 2.0
        * @return ETrue if access point has been set otherwise EFalse.
        */
        TBool StartPreferencesViewL( TUint aListItems );
        
		/**
        * Gives back a pointer to the Recent Url Store
        * @return iRecentUrlStore
		*/
        CRecentUrlStore* RecentUrlStore();

		/**
        * Returns ETrue if at least one page has been loaded.
        * @return   ETrue if at least one page has been loaded.\n
        *           EFalse if no pages has been loaded.
        */
        TBool IsPageLoaded();

		/**
        * Returns fetching status.
        * @return   ETrue if there is a fetch going on.\n 
        *           EFalse otherwise.
        */
        TBool Fetching() const;

        /**
        * Checks if the content of a new page is displayed.
        * @return ETrue if the content is displayed.
        *         EFalse otherwise.
        * (useful for zooming while page is loading)
        */
        TBool ContentDisplayed() const;

       /**
        * Sets the content displayed status
        * (useful for zooming while page is loading)
        */
        void SetContentDisplayed( TBool aValue );

        /**
        * Initiate fetching of bookmark.
        * @param aBookmarkUid  Uid of bookmark to fetch.
        */
        void FetchBookmarkL( TInt aBookmarkUid );

		/**
        * Initiate fetching of bookmark.
        * @param aBookmarkItem Bookmark item to fetch.
        */
        void FetchBookmarkL( const CFavouritesItem& aBookmarkItem );

		/**
        * Initiate fetching of URL.
        * @param aUrl URL to fetch.
        * @param aUserName User name.
        * @param aPassword Password.
        * @param aAccessPoint AccessPoint.
        */
        void FetchL
            (
            const TDesC& aUrl,
            const TDesC& aUserName,
            const TDesC& aPassword,
            const TFavouritesWapAp& aAccessPoint,
            CBrowserLoadObserver::TBrowserLoadUrlType aUrlType
                = CBrowserLoadObserver::ELoadUrlTypeOther
            );

        /**
        * Initiate fetching of URL with default AP, empty username and password.
        * @param aUrl URL to fetch.
        */
        void FetchL( const TDesC& aUrl, CBrowserLoadObserver::TBrowserLoadUrlType aUrlType
            = CBrowserLoadObserver::ELoadUrlTypeOther );

		/**
        * Cancel fetching.
        */
        void CancelFetch( TBool aIsUserInitiated = EFalse );

		/**
		* Starts animation.
		*/
		void StartProgressAnimationL();

		/**
		* Stops animation.
		*/
		void StopProgressAnimationL();
	
		/*
		* Return True if browser was launched especially into feeds view
		*/
		TBool BrowserWasLaunchedIntoFeeds() { return(iOverriddenLaunchContextId == EBrowserContextIdFeeds); }

		/*
		* record the overridden launch context id
		*/
		void SetOverriddenLaunchContextId(TUint aId) { iOverriddenLaunchContextId = aId; }


#ifdef __RSS_FEEDS
        /**
        * Returns reference to FeedsClientUtilities.
        */
        CFeedsClientUtilities& FeedsClientUtilities() const;
#endif  // __RSS_FEEDS


 		/**
        * Returns the Uid of the previous active view from the view history.
        * @return Uid of the previous active view.
        */
		TUid GetPreviousViewFromViewHistory();

        /**
        * Return if Flash plugin is present in the system or not
        * @param none
        * @return TBool
        */
        TBool FlashPresent( );
        
				
		/**
		* Check state of LaunchHomePage command for dimmed state in options menu and shortcut keymap
		* @return ETrue is LaunchHomePage command should be dimmed in options menu and shortcut keymap
		*         EFalse otherwise
		* @param none
		*/		
		TBool IsLaunchHomePageDimmedL(); 
		/**
		* Check from CCoeAppUi if a Dialog or Menu is diplayed on top of the view
		* @return ETrue if Dialog or Menu is displayed
		*         EFalse otherwise
		* @param none
		*/		
		TBool IsDisplayingMenuOrDialog();
		        
        
    public:  // from MWindowInfoProvider

        HBufC* CreateWindowInfoLC( const CBrowserWindow& aWindow ) ;
        void SwitchWindowL();

		/**
		* Initializes the browser, if it is in embedded mode.
		* In embedded mode the browser is initialized later, not when it
		* is constructed. First the browser must wait for the embedding
		* application to send the startup parameters, and only after that
		* can be initialized.
		* This method is required and used by new embedding architecture.
		*/
		void InitBrowserL();
		
        /**
        * From CAknViewAppUi, CEikAppUi
        * Handles changes in keyboard focus when an 
        * application switches to foreground
        * @param aForeground Is foreground event?
        */
        void HandleForegroundEventL( TBool aForeground );
		
			public: // From MDownloadedContentHandler
	
        virtual TBool HandleContentL( const TDesC& aFileName, 
                                      const CAiwGenericParamList& aParamList, 
                                      TBool& aContinue );

        virtual TAny* DownloadedContentHandlerReserved1( TAny* /*aAnyParam*/ );

        void SetContentHandlingResult( TBool aClientWantsToContinue, TBool aWasContentHandled );

        /**
        * Loads the defined search page into content view.
        */
        void LoadSearchPageL();
        
        // This member is public, so BrowserLauncher.dll can close the browser.
        CBrowserAsyncExit *iBrowserAsyncExit;
    public:
        void StopConnectionObserving();

#ifdef BRDO_OCC_ENABLED_FF
        //Retry flags
        void SetRetryFlag(TBool flag);
        TBool GetRetryFlag();
#endif

	private:    
	
        /**
        *Callback for late constructing of the SendUi in idle
        *Needed to provide better response time for the user
        *@return  Returns 0
        */
        static TInt DelayedSendUiConstructL(TAny* aCBrowserAppUi);


        /**
        * Calls ProcessCommandParametersL with the given parameters.
        * @param aParams startup parameter (will be casted).
        * @return Returns 0 (called only once).
        */
        //static TInt DelayedProcessParametersL( TAny* aParams );

        /**
        * Callback function for CIdle. Calls DoStopDelay.
        * @param aCBrowserAppUi 
        * @return Returns 0 (called only once).
        */
        static TInt StopDelay( TAny* aCBrowserAppUi);

        /**
        * Stops CActiveSchedulerWait loop started 
        * by ParseAndProcessParametersL().
        */
        void DoStopDelay();

        /**
        *Construct SendUI now
        */
        void DoConstructSendUiL();

        /*
        * Changes the connection
        * @param 
        * @return None.
        */
        void ChangeConnectionL();

        /*
        * send disconnect event to multiple windows
        */
        void SendDisconnectEventL();

        /**
        * Check if Flash plugin is present in the system or not
        * @param none
        * @return TBool
        */
        TBool CheckFlashPresent( );


        /**
        * Checks if a HomePage will be launched
        * @param none
        * @return TBool - ETrue if no homepage will be launched otherwise return EFalse
        */
        TBool NoHomePageToBeLaunchedL();

        /**
        * Show the name of browser and version of the browser
        * @param none
        */
		void ShowNameAndVersionL();
		
		/**
		* There's a homepage to be launched so start in content view
		* [Convenience function that gets used in 2 places]
        * @param none
        */		
		void StartFetchHomePageL();		
        TUid FindAppIdL(TUid aMessageUid);
#ifdef BRDO_OCC_ENABLED_FF
        //For Call back for reconnectivity
        static TInt RetryConnectivity(TAny* aCBrowserAppUi);
        TInt RetryInternetConnection();		
        void CheckOccConnectionStage();
#endif

#ifdef BRDO_IAD_UPDATE_ENABLED_FF
		/**
        * Check updates
        */
        void CheckUpdatesL(); 

        /**
        * Clean IAD update parameters
        */
        void CleanUpdateParams(); 

        /**
        * This callback function is called when the update checking operation has completed.
        *
        * @param aErrorCode The error code of the observed update operation.
        *                   KErrNone for successful completion,
        *                   otherwise a system wide error code.
        * @param aAvailableUpdates Number of the updates that were found available.
        */
        void CheckUpdatesComplete( TInt aErrorCode, TInt aAvailableUpdates );

        /**
        * This callback function is called when an update operation has completed.
        *
        * @param aErrorCode The error code of the completed update operation.
        *                   KErrNone for successful completion,
        *                   otherwise a system wide error code.
        * @param aResult Details about the completed update operation.
        *                Ownership is transferred.
        */
        void UpdateComplete( TInt aErrorCode, CIAUpdateResult* aResultDetails );

		/**
        * This callback function is called when an update query operation has completed.
        *
        * @param aErrorCode The error code of the observed query operation.
        *                   KErrNone for successful completion,
        *                   otherwise a system wide error code.
        * @param aUpdateNow ETrue informs that an update operation should be started.
        *                   EFalse informs that there is no need to start an update
        *                   operation.
        */
        void UpdateQueryComplete( TInt aErrorCode, TBool aUpdateNow ){return;}

        /**
        * This function is checking the existence of the file containing last update time
        * @param None
        * @return TBool.
        */
        TBool CheckUpdateFileAvailable();
        
        /**
        * This function is deleting of the file(if exist) containing last update time
        * @param None
        * @return None.
        */
        void DeleteUpdateFile();
        
        /**
        * This function is will write the current time in file
        * @param None
        * @return None.
        */
        void WriteUpdateFile();
        
        /**
        * This function will read the content of the file if it exists
        * @param None
        * @return TBool.
        */
        TInt64 ReadUpdateFile();
#endif		    
	protected:

        /**
        * Symbian OS constructor.
        */
        void ConstructL();
		
        /**
        * Sends Browser to the background. The task whose window group 
        * is at the next ordinal position is brought up to the foreground.
        */
        void SendBrowserToBackground();

        /**
        *
        */
        void DelayedConstructL();

        /**
        * Clear the cache.
        */
        void ClearTheCacheL(TBool afterQuery = EFalse, TBool aShowDoneNote = ETrue);


        /**
        * Delete cookies.
        */
        void DeleteCookiesL();

        
        /**
        * Delete Form and Password Data
        * @param aShowPromptAndComplete - show confirmation prompt and completion info note if True
        */
        void ClearFormAndPasswdDataL(TBool aShowPromptAndComplete);


        /**
        * Clear History of all open windows
        */
        void CBrowserAppUi::ClearHistoryL();
        
        
        /**
        * Prompt before clearing history
        */
        void CBrowserAppUi::ClearHistoryWithPromptL();

        
        /**
        * Clear All Privacy Data
        */
        void CBrowserAppUi::ClearAllPrivacyL();

        /**
        * Disconnect.
        */
        void DisconnectL();

		/**
		* Updates the soft keys. Previously a callback in ViewShellObserver
		*/
		void UpdateSoftKeys();
        /**
        * Logs a record of opened pages to Recent Url Store
        */
		void LogAccessToRecentUrlL( CBrCtlInterface& aBrCtlInterface );

        /**
        * Logs a record of requested pages to Recent Url Store
        */
		void LogRequestedPageToRecentUrlL( const TDesC& aUrl );
   
        /**
        * Load previous page from history (back button)
        * @since Series60 1.2
        */
        void HistoryLoadPrevious();

        /**
        * Load next page from history (forward button)
        * @since Series60 1.2
        */
        void HistoryLoadNext();

        /**
        * Creating a full url containing username, password, 
        * server domain and document path.
        * @return buffer pointing to the url
        */
        HBufC* CreateFullUrlLC( const TDesC& aUrl,
                                const TDesC& aUsername,
                                const TDesC& aPassword );


    protected:  // from MConnectionStageObserver
        /**
        * Connection stage achieved. 
        */
        void ConnectionStageAchievedL();

    protected:  // From CAknViewAppUi, CEikAppUi

        /**
        * Processes shell commands.
        * @param aCommand Command to process.
        * @return //TODO
        */
        TBool ProcessCommandParametersL(TApaCommand aCommand,TFileName& /*aDocumentName*/,const TDesC8& /*aTail*/ );


        /**
        * Handle the external message.
        * @param aClientHandleOfTargetWindowGroup Handle.
        * @param aMessageUid Message id.
        * @param aMessageParameters Message parameters.
        * @return Response to the message.
        */
        MCoeMessageObserver::TMessageResponse HandleMessageL( TUint32 aClientHandleOfTargetWindowGroup,
                                                              TUid aMessageUid,
                                                              const TDesC8& aMessageParameters );

        /**
        * Handles screen resolution changes
        */
        void HandleResourceChangeL( TInt aType );

		/**
        * Handles application specific events like OOM from window server.
        * @param aEventType The type of event.
        * @param aWsEvent window server event.
        * @return Key response.
        */
		void HandleApplicationSpecificEventL(TInt aEventType, const TWsEvent& aWsEvent);
		
    private:

        TUint32 iRequestedAp;
        TBool iCalledFromAnotherApp;

        TBool iIsForeground;
        TUid iViewToBeActivatedIfNeeded;
        TUid iViewToReturnOnClose;

        TUid iLastViewId;
        TBool iSecureSiteVisited;


        CWmlBrowserBmOTABinSender* iSender;

        CConnectionStageNotifierWCB* iConnStageNotifier;

        CIdle* iIdle;   // at construction phase
        CActiveSchedulerWait iWait;

        TBool   iExitInProgress;

        RSocketServ iSockServSess;

        TBool iShutdownRequested;
		TBool iHTTPSecurityIndicatorSupressed;
		CRecentUrlStore* iRecentUrlStore;
		TInt iParametrizedLaunchInProgress;  //ETrue when launch with parameters is in idling

        TBool iExitFromEmbeddedMode;

        // Flag to indicate that real downloading is
        // going on ( we are not loading from cache )

        TBool iShowProgress;
        TBool iLongZeroPressed;
        TBool iStartedUp;
        TBool iFeatureManager;
        TBool iUserExit;
		TBool iPgNotFound;
        TBool iSuppressAlwaysAsk;
        TBool iFlashPresent;
        TBool iSpecialSchemeinAddress;
        
        RFavouritesSession iFavouritesSess;

        // The Uid of the embedding application.
        TUid iEmbeddingApplicationUid;
        
		TUid iPreviousView;
		TUid iCurrentView;		

		// Uid for tracking what view the last CBA update was made under,
		// used to avoid extraneous updates
		TUid iLastCBAUpdateView;		

        // Observing changes in PushMtm
        CBrowserPushMtmObserver* iPushMtmObserver; // owned
		
		// Record browser launch's overridden context id
		TUint iOverriddenLaunchContextId;
		
		TBool iBrowserAlreadyRunning; 
		
#ifdef BRDO_OCC_ENABLED_FF
        CPeriodic *iRetryConnectivity;
        TBool reConnectivityFlag;
#endif		

#ifdef BRDO_IAD_UPDATE_ENABLED_FF		
        CIAUpdate* iUpdate;  
        CIAUpdateParameters* iParameters; 
        RFs iFs;
#endif        
        TBool iCalledFromExternApp;
        TInt iWindowIdFromFromExternApp;
	protected:

		CBrowserCommsModel* iCommsModel;
        CBrowserPreferences* iPreferences;

        MConnection* iConnection;

		CBrowserDialogsProvider* iDialogsProvider;
        mutable CBrowserPopupEngine* iPopupEngine;
        CBrowserLauncherService* iBrowserLauncherService; // Not owned
        CActiveSchedulerWait iLauncherServiceWait;
		TBool iClientWantsToContinue;
		TBool iWasContentHandled;
        RArray<TUint>* iZoomLevelArray;
        TUint iCurrentZoomLevelIndex;
        TUint iCurrentZoomLevel;

        CBrowserWindowManager* iWindowManager;
		CIdle* iLateSendUi;
		CBrowserBookmarksView* iBookmarksView; // not owned
#ifdef __RSS_FEEDS
		CFeedsClientUtilities* iFeedsClientUtilities;
#endif  // __RSS_FEEDS
    };
#endif

// End of File
