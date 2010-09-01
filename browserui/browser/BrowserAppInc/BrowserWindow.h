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
*      API provider.
*  
*
*/


#ifndef __MULTIPLE_WINDOWS_WINDOW_H__
#define __MULTIPLE_WINDOWS_WINDOW_H__

// INCLUDES FILES
#include "WmlBrowserBuild.h"
#include <e32base.h>
#include "ApiProvider.h"
#include "Preferences.h"
#include <brctldefs.h>
#include "BrowserLoadObserver.h"
#include "BrowserDialogsProviderProxy.h"
#include <brctlwindowobserver.h>

// FORWARD DECLARATIONS
class CBrCtlInterface;
class CBrowserDisplay;
class CBrowserSpecialLoadObserver;
class CBrowserSoftkeysObserver;
class CBrowserWindowManager;

class MDisplay;
class MConnection;
class CBrowserDialogsProvider;
class MBmOTABinSender;
class CBrowserPopupEngine;

// CLASS DECLARATION

/**
*  Interface to query several API implementations via Window class.
*  
*  @lib Browser.app
*  @since Series 60 3.0
*/
class CBrowserWindow :  public CBase,
                        public MApiProvider,
                        public MPreferencesObserver,
                        public MBrCtlWindowObserver
    {
    public:
        static CBrowserWindow* NewLC(
            TInt aWindowId,
            const TDesC* aTargetName,
            CBrowserWindowManager* aWindowMgr );
        static CBrowserWindow* NewL(
            TInt aWindowId,
            const TDesC* aTargetName,
            CBrowserWindowManager* aWindowMgr );
        ~CBrowserWindow();

    public:  // new functions
        
        /*
        * Return the window's id.
        */
        inline TInt WindowId() const
            { return iWindowId; }

        /*
        * Return the window's targetname.
        */
        inline const TDesC* TargetName() const
            { return iTargetName; }

        /*
        * Return the window's active status.
        */
        inline TBool IsWindowActive() const
            { return iActive; }
            
        /*
        * Activates this window, redraw it, updates title, etc.
        */
        void ActivateL( TBool aDraw = ETrue );

        /*
        * Deactivates this window.
        */
        void DeactivateL();
        
        /*
        * Returns a reference to the DialogsProviderProxy
        */
        inline CBrowserDialogsProviderProxy& DialogsProviderProxy() const
                                            { return *iDialogsProviderProxy; };
        /**
		* To check that Page Overview feature is supported or not
		* @since Series60 3.0
		* @return Returns ETrue if feature is supported
		*/
		TBool IsPageOverviewSupportedL();
		
        /*
        * Resets Page Overview setting as defined in Preferences.
        *  @since Series 60 3.0        
        */                                            
		void ResetPageOverviewLocalSettingL();
		
        /*
        * Temporarily changes the Page Overview setting.
        * This function does not change the Page Overview setting
        * in Preferences.
        * @since Series 60 3.0                
        */		
		void FlipPageOverviewLocalSettingL();
		
        /*
        * Changes the title pane text until the given interval.
        * @since Series 60 3.0                
        * @param aTitle The new text of the title pane
        * @param aInterval The interval while the new text will be shown on title pane
        */                                            
		void ChangeTitlePaneTextUntilL( const TDesC& aTitle, TInt aInterval );
        
        /*
        * Set if window contains WML page(s).
        * @since Series 60 3.1                
        * @param aHasWMLContent 
        */                                            
        void SetHasWMLContent(TBool aHasWMLContent);
        
        /*
        * Set if current window contains WML page(s).
        * @since Series 60 3.1                
        * @param aCurrWinHasWMLContent 
        */                                            
        void SetCurrWinHasWMLContent(TBool aCurrWinHasWMLContent);
        
        /*
        * Returns wml content state for current page only or any page in window history based on param
        * @since Series 60 3.1                
        * @param aCurrWinOnly - If ETrue, Returns ETrue if current window has WML content.
        * 						If EFalse, Returns ETrue if any window in history has WML content.
        * @return TBool
        */                                            
        TBool HasWMLContent (TBool aCurrWinOnly);
        
        /*
        * Returns true if current window has Feeds content.
        * @since Series 60 3.1                
        * @return TBool
        */                                            
        TBool HasFeedsContent ()const { return iHasFeedsContent; };
        
        /*
        * Set if current page of window contains WML.
        * @since Series 60 3.1                
        * @param aWMLMode 
        */                                            
        void SetWMLMode(TBool aWMLMode);
        
        /*
        * Set if hisory needs to be cleared after loading next page.
        * @since Series 60 3.1                
        * @param aFirstPage 
        */                                            
        void SetFirstPage(TBool aFirstPage);
        
        /*
        * Set if current window contains Feeds Full Story.
        * Used to go back to feeds view when window is closed
        * @since Series 60 3.1                
        * @param aHasFeedsContent 
        */                                            
        void SetHasFeedsContent(TBool aHasFeedsContent);
        
        /*
        * Returns true if current page has WML content.
        * @since Series 60 3.1                
        * @return TBool
        */                                            
        TBool WMLMode ()const { return iWMLMode; };
        
        /*
        * Returns true if current page has images loaded.
        * @since Series 60 5.0
        * @return TBool
        */                                            
        TBool HasLoadedImages ()const { return iImagesLoaded; };
        
        /*
        * Set true if user selects current page to load images. false when page loads
        * @since Series 60 5.0
        * @param aImagesLoaded
        */                                            
        void SetImagesLoaded (TBool aImagesLoaded) { iImagesLoaded = aImagesLoaded; };
        
        
    public:  // from MApiProvider
    
        CBrCtlInterface&              BrCtlInterface() const ;
        MDisplay&                     Display() const ;
        CBrowserLoadObserver&         LoadObserver() const ;
        CBrowserSoftkeysObserver&     SoftkeysObserver() const ;
        CBrowserSpecialLoadObserver&  SpecialLoadObserver() const ;
        inline CBrowserWindowManager& WindowMgr() const 
            { return *iWindowManager; }
            
        MConnection& Connection() const;
        MPreferences& Preferences() const;
        MCommsModel& CommsModel() const;
        CBrowserDialogsProvider& DialogsProvider() const;
        MBmOTABinSender& BmOTABinSenderL();
        CBrowserPopupEngine& PopupEngine() const;
        
        void SetViewToBeActivatedIfNeededL( TUid aUid, TInt aMessageId = 0 );
        TBool IsCancelFetchAllowed() const;
        TUid LastActiveViewId() const;
        void SetLastActiveViewId( TUid aUid );
        TBool ExitInProgress() const;
        void SetExitInProgress( TBool aValue );
        TBool IsConnecting() const;
        void UpdateNaviPaneL( TDesC& aStatusMsg );
        void SetViewToReturnOnClose( TUid const &aUid );
        void CloseContentViewL();
        TBool IsEmbeddedModeOn() const;
        TBool IsShutdownRequested() const;
        void ExitBrowser(TBool aUserShutdown);
        TBool IsAppShutterActive() const;
        TBool StartPreferencesViewL( TUint aListItems );
        TBool IsPageLoaded();
        TBool IsForeGround() const;
        TBool Fetching() const;
        TBool ContentDisplayed() const;
        void  SetContentDisplayed( TBool aValue );
        void FetchBookmarkL( TInt aBookmarkUid );
        void FetchBookmarkL( const CFavouritesItem& aBookmarkItem );
        void FetchL(
					const TDesC& aUrl,
					const TDesC& aUserName,
					const TDesC& aPassword,
					const TFavouritesWapAp& aAccessPoint,
                    CBrowserLoadObserver::TBrowserLoadUrlType aUrlType
					);
		void FetchL( const TDesC& aUrl, CBrowserLoadObserver::TBrowserLoadUrlType aUrlType );
		void CancelFetch( TBool aIsUserInitiated );
		void SetRequestedAP( TInt aAp );
		void SetExitFromEmbeddedMode( TBool aFlag );
        TBool IsProgressShown() const;
        void SetProgressShown( TBool aProgressShown );
        TBool StartedUp() const;
        TUint32 RequestedAp() const;
      	void LogAccessToRecentUrlL( CBrCtlInterface& aBrCtlInterface );
     	void LogRequestedPageToRecentUrlL( const TDesC& aUrl );
        void StartProgressAnimationL();
        void StopProgressAnimationL();
        #ifdef __RSS_FEEDS
        virtual CFeedsClientUtilities& FeedsClientUtilities() const;
		#endif  // __RSS_FEEDS
		TUid GetPreviousViewFromViewHistory();
						
		void SetLastCBAUpdateView( TUid aView );
		TUid GetLastCBAUpdateView();
		
        TBool FlashPresent();
        TBool IsLaunchHomePageDimmedL();
        void SetCalledFromAnotherApp( TBool aValue );
        TBool CalledFromAnotherApp();
		TBool BrowserWasLaunchedIntoFeeds();
		TBool IsDisplayingMenuOrDialog();
		TBool CompleteDelayedInit();

    //-------------------------------------------------------------------------
    // Preferences Handling
    //-------------------------------------------------------------------------
    
    public:  // from MPreferencesObserver
        
        /**
        *   Observer for Preference changes
        *   @param aEvent the type of preference event which happened
        *   @param aValues a struct of all settings values
        *   @since Series60 3.0
        */
        virtual void HandlePreferencesChangeL( 
                                    const TPreferencesEvent aEvent,
	                                TPreferencesValues& aValues,
	                                TBrCtlDefs::TBrCtlSettings aSettingType );
	                                
    //-------------------------------------------------------------------------
    // Window Observing
    //-------------------------------------------------------------------------
    
    public: // from MBrCtlWindowObserver

        /**
        * Request the host applicaion to open the URL in a new window
        * @since 3.0
        * @param aUrl The Url of the request to be done in the new window
        * @param aTargetName The name of the new window
        * @param aUserInitiated ETrue if the new window is initiated by a user event (click)
        * @param aReserved For future use
        * @return Return Value is the new browser control associated with the new window
        */
        CBrCtlInterface* OpenWindowL( TDesC& aUrl, TDesC* aTargetName, TBool aUserInitiated,
            TAny* aReserved );
            
        /**
        * Find a window by target name
        * @since 3.0
        * @param aTargetName name of the window to find
        * @return Return Value is the browser control associated with the window name
        */
        CBrCtlInterface* FindWindowL( const TDesC& aTargetName ) const;

        /**
        * Handle window events such as close/focus etc
        * @since 3.0
        * @param aTargetName name of the window to send the event to
        * @param aCommand Command to pass to the window
        * @return void
        */
        void HandleWindowCommandL( const TDesC& aTargetName, TBrCtlWindowCommand aCommand );
        
    //-------------------------------------------------------------------------
    
    protected:
        
        /**
        *   Checks for any changes in Global Prefeernces and notifies 
        *   Browser Control
        *   @param aSettingType an enum defining a setting
        *   @since Series60 3.0
        **/
        void UpdateGlobalPreferencesL( TBrCtlDefs::TBrCtlSettings aSettingType  );
        
        /**
        *   Checks for any changes in Local Preferences and notifies 
        *   Browser Control
        *   @param aValues a struct of all settings values        
        *   @since Series60 3.0
        **/
        void UpdateLocalPreferencesL( TPreferencesValues& aValues );
        
        /**
        *   Initialises Local Preferences
        *   @since Series60 3.0
        **/
        void InitialiseLocalPreferencesL();
        
        /**
        *   Passes a subset of settings to the Browser Control
        *   @since Series60 3.0
        */
        void UpdateBrCtlSettingsL();
        
    //-------------------------------------------------------------------------
        
    protected:
        /**
        * Default C++ constructor.
        */
        CBrowserWindow(
            TInt aWindowId,
            const TDesC* aTargetName,
            CBrowserWindowManager* aWindowMgr );
            
        /*
        * 2nd phase constructor.
        */
        void ConstructL();

    //-------------------------------------------------------------------------
    
    private:
        TBool        iClosing;
        TInt         iWindowId;
        const TDesC* iTargetName;  // owned
        TBool        iActive;
        TBool		 iHasWMLContent;
        TBool		 iCurrWinHasWMLContent;
        TBool		 iHasFeedsContent;
        TBool		 iWMLMode;
        TBool        iFirstPage;
        TBool        iImagesLoaded;

        CBrCtlInterface*              iBrCtlInterface;
        CBrowserDialogsProviderProxy* iDialogsProviderProxy;
        CBrowserDisplay*              iDisplay;
        CBrowserLoadObserver*         iLoadObserver;
        CBrowserSoftkeysObserver*     iSoftkeysObserver;
        CBrowserSpecialLoadObserver*  iSpecialLoadObserver;
        CBrowserWindowManager*        iWindowManager;  // not owned
        
    private:    // Local Settings
        
        TInt    iFontSize;
        TUint32 iEncoding;
        TBool   iTextWrap;
        TBool 	iIsPageOverviewOn;
        static TBool    iIsPageOverviewSupported;
		static TBool    iIsPageOverviewSupportQueried;
    };

#endif  // __MULTIPLE_WINDOWS_WINDOW_H__

// End of file
