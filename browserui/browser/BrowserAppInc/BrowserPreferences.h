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
* Description:  It manages Browser's preferences.
*
*/


#ifndef BROWSER_PREFERENCES_H
#define BROWSER_PREFERENCES_H

// INCLUDE FILES
#include <e32base.h>
#include <ActiveApDb.h>
#include <brctldefs.h>
#include "Browser.hrh"
#include "Preferences.h"
#include <BrowserNG.rsg>
#include "BrowserUiPrivateCRKeys.h"
#include <browseruisdkcrkeys.h>
#include <centralrepository.h> 
#include <bldvariant.hrh>

// CONSTANTS

// FORWARD DECLARATION

class MCommsModel;
class MApiProvider;
class CRepository;

// class CBrCtlInterface;
class TBrowserOverriddenSettings;
class CApDataHandler;
class CVpnApItem;
class CVpnApEngine;
// CLASS DECLARATION

/**
*  Stores Browser settings.
*  
*  @lib Browser.app
*  @since Series60 1.2
*/
class CBrowserPreferences : public CBase,
                                public MPreferences,
                                public MActiveApDbObserver
    {
    public:     // construction

        /**
        * Two-phased constructor.
        * @since Series60 1.2
        * @param aCommsModel Comms model
        * @return The constructed preferences object
        */
        static CBrowserPreferences* NewLC( MCommsModel& aCommsModel, 
                                            MApiProvider& aApiProvider, 
                                            TBrowserOverriddenSettings* aSettings);

        /**
        * Two-phased constructor.
        * @since Series60 1.2
        * @param aCommsModel Comms model
        * @return The constructed preferences object
        */
        static CBrowserPreferences* NewL( MCommsModel& aCommsModel,
                                            MApiProvider& aApiProvider ,
                                            TBrowserOverriddenSettings* aSettings );

    public:     // destruction

        /**
        * Destructor.
        */
        virtual ~CBrowserPreferences();


    public:     // observation

        /**
        * Adds an observer to the list of observers to be notified about changes.
        * @since Series60 1.2
        * @param anObserver Observer to be notified about events
        */
        void AddObserverL( MPreferencesObserver* anObserver );


        /**
        * Removes an observer.
        * @since Series60 1.2
        * @param anObserver The observer
        */
        void RemoveObserver( MPreferencesObserver* anObserver );

        /**
        * Notify all observers about change.
        * @since Series60 1.2 (updated in 3.0)
        */      
        void NotifyObserversL( TPreferencesEvent aEvent,
                                    TBrCtlDefs::TBrCtlSettings aSettingType );

    public:     // From MActiveApDbObserver

        /**
        * Derived classes should implement this method, and act accordingly
        * to database events.
        * @since Series60 1.2
        * @param anEvent Database-related event
        */
         void HandleApDbEventL( TEvent anEvent );

    public:    // New functions

        /**
        * Stores preferences into shared data db.
        * @since Series60 1.2
        */
        void StoreSettingsL() const; 

        /**
        * Returns a structure containing all of the settings according to the
        * central repository values
        * @since Series60 3.0
        */
        const TPreferencesValues& AllPreferencesL();
        
        /**
        * To access an int value key in ini file
        * @param aKey the ini file key to read
        * @return the value of the given key
        * @since Series60 2.0
        */
        TInt GetIntValue( TUint32 aKey) const;

        /**
        * To access an string value key in ini file
        * @param aKey the ini file key to read
        * @return the value of the given key
        * @since Series60 2.0
        */
        TInt GetStringValueL ( TUint32 aKey, TInt aMaxBufSize, TDes& aBuf ) const;
		
    private:    // New functions
        
        /**
        * restores preferences from shared data db. If fails, it uses defaults.
        * @since Series60 1.2
        */
        void RestoreSettingsL();
        
    private:    //Utility functions for accessing shared data

        /**
        * To set an int value key in ini file
        * @param aKey the ini file key to write
        * @param aValue new value for the key
        * @return ETrue if successful
        * @since Series60 2.0
        */
        TBool SetIntValueL ( TUint32 aKey, const TInt& aValue);

        /**
        * To access an string value key in ini file
        * @param aKey the ini file key to read
        * @return the value of the given key
        * @since Series60 2.0
        */
        HBufC* GetStringValueL ( TUint32 aKey ) const;
		
        /**
        * To set an int value key in ini file
        * @param aKey the ini file key to write
        * @param aValue new value for the key
        * @return ETrue if successful
        * @since Series60 2.0
        */
        TBool SetStringValueL( TUint32 aKey, const TDesC& aValue );

    private:     // New functions

        /**
        *   Enables access to functions in CBrowserAppUi
        **/
        inline MApiProvider& ApiProvider() { return iApiProvider; }

    public:     // New functions
    
        /**
        * To set overriden values
        * @param aSettings settings to verride
        * @return void
        * @since Series60 2.0
        */
        void SetOverriddenSettingsL(TBrowserOverriddenSettings* aSettings);

        /**
        * To set embedded mode
        * @param aEmbeddedMode true if embedded mode is on
        * @return void
        * @since Series60 2.0
        */
        void SetEmbeddedModeL(TBool aEmbeddedMode);
        
        /**
        * To get embedded mode
        * @return TBool
        * @since Series60 3.0
        */
        inline TBool EmbeddedMode() const { return iEmbeddedMode; }            
        
        /**
        * To access DefaultAccessPoint setting.
        * @since Series60 1.2
        * @return UID for default accesspoint
        */
        inline TUint DefaultAccessPoint() const;
        

        /**
        * Get the default snap id
        * @since Series60 1.2
        * @return
        */               
        inline TUint DefaultSnapId() const;        
        
        /**
        * To access Accossiated VPN if exist setting.
        * @since Series60 1.2
        * @return UID for Accossiated VPN or KWmlNoDefaultAccessPoint
        */        
        inline TUint AssociatedVpn() const;
        
        /**
        * To access deafult AP details if exist setting.
        * @since Series60 1.2
        * @return CApAccessPointItem* or NULL
        */         
        inline CApAccessPointItem* DefaultAPDetails();
        
        /**
        * To access vpn item if exists.
        * aDefaultAccessPoint
        * aVpnItem
        * @since Series60 1.2
        * @return TBool - if vpn item is valid
        */          
        TBool VpnDataL( TUint aDefaultAccessPoint, CVpnApItem& aVpnItem );
         
        /**
        * To access AutoLoadContent setting.
        * @since Series60 1.2
        * @return Value of setting
        */
        inline TInt AutoLoadContent() const;

        /**
        * To access PageOverview setting.
        * @return Value of setting
        */
        inline TBool PageOverview() const;
        
        /**
        * To access BackList setting.
        * @return Value of setting
        */
        inline TBool BackList() const;
        
        /**
        * To access AutoRefresh setting.
        * @return Value of setting
        */
        inline TBool AutoRefresh() const;

        /**
        * Sets the automatic updating access point of Web Feeds 
        */
        void SetAutomaticUpdatingApL( TUint aAutomaticUpdatingAP );


        /**
        * Sets the automatic updating while roaming of News & Blogs 
        */
        void SetAutomaticUpdatingWhileRoamingL( TBool aAutoUpdateRoaming ) ;
        
        /**
        * To access user-defined URL suffix list (.com, org, .net)
        */
        inline HBufC* URLSuffixList( ) const;

        /**
        * To modify user-defined URL suffix list (.com, .org, .net)
        */
        void SetURLSuffixList( HBufC* aValue );

        /**
        * automatic updating access point of Web Feeds
        */
        inline TUint AutomaticUpdatingAP() const;
        

        /**
        * automatic updating while roaming of News & Blogs 
        */
        inline TBool AutomaticUpdatingWhileRoaming() const;
        
        /**
        * To access TextWrap setting
        * @since Series60 1.2
        * @return setting item value
        */
        inline TBool TextWrap() const;

        /**
        * To access FontSize setting
        * @since Series60 1.2
        * @return Font size
        */
        inline TInt FontSize() const;

        /**
        * To access Encoding setting
        * @since Series60 1.2
        * @return Default Encoding
        */
        inline TUint32 Encoding() const;

        /**
        * To access Adaptive Bookmarks setting
        * @since Series60 1.2
        * @return TWmlSettingsAdaptiveBookmarks
        */
        inline TWmlSettingsAdaptiveBookmarks AdaptiveBookmarks() const;

        /**
        * To access FullScreen setting
        * @since Series60 2.1
        * @return TWmlSettingsFullScreen
        */           
        inline TWmlSettingsFullScreen FullScreen() const;

        /**
        * To access Cookies setting
        * @since Series60 1.2
        * @return TWmlSettingsCookies
        */
        inline TWmlSettingsCookies Cookies() const;

        /**
        * To access ECMA setting
        * @since Series60 2.x
        * @return TWmlSettingsECMA
        */
        inline TWmlSettingsECMA Ecma() const;

        /**
        * To access SCRIPTLOG setting
        * @since Series60 3.2
        * @return TUint32
        */
        inline TUint32 ScriptLog() const;

        /** 
        * To access HTTP security warnings setting
        * @since Series60 1.2S
        * @return ETrue if security warnings are enabled
        */
        inline TBool HttpSecurityWarnings() const;

        /** 
        * To access IMEI notification setting
        * @since Series60 2.6
        * @return ETrue if notification is enabled
        */
        inline TWmlSettingsIMEI IMEINotification() const;
               
        /**
        * Call this to check whether Http
        */
        inline TBool HttpSecurityWarningsStatSupressed() const;

        /**
        * To access Downloads Open setting
        * @since Series60 3.x
        * @return TBool
        */
        inline TBool DownloadsOpen() const;

        /**
        * To access user-defined homepage URL setting
        */
        TInt HomePageUrlL( TDes& aUrl, TBool aForceLoadFromSD = EFalse ) const;

        /**
        * Get the home page URL from belongs to the access point.
        */
        TInt HomePageFromIapL( TDes& aUrl, TUint aIap ) const;

        /**
        * To access user-defined search page URL setting
        */
        TPtrC SearchPageUrlL();

        /**
        * To determine that is there an overridden default access point defined or not
        */
        TBool CustomAccessPointDefined();

        /**
        * Shows/Hides the query on exit dialog
        * 0 = Not show the dialog
        * 1 = Show the dialog
        *
        * @since Series60 2.7
        * @return ETrue if query on exit is enabled
        */
        inline TBool QueryOnExit() const;

        /**
        * Sends/Don not send the referrer header
        * 0 = Do not send the referrer header
        * 1 = Send the referrer header
        *
        * @since Series60 2.7
        * @return ETrue if Send referrer header is enabled
        */
        inline TBool SendReferrer() const;

        /**
        * To access ShowConnectionDialogs setting
        * @since Series60 2.0
        * @return value of setting
        */
        inline TBool ShowConnectionDialogs() const;

        /**
        * To check that an engine local feature is supported or not
        * @since Series60 2.1
        * @return Returns ETrue if feature is supported
        * @param aBitmask Feature to check
        */
        inline TBool EngineLocalFeatureSupported( const TInt aFeature ) const;

        /**
        * To check that a ui local feature is supported or not
        * @since Series60 2.1
        * @return Returns ETrue if feature is supported
        * @param aBitmask Feature to check
        */
        inline TBool UiLocalFeatureSupported( const TInt aFeature ) const;
        
        /**
         * To access MediaVolume setting
         * @since Series60 2.6
         * @return TInt
         */         
        inline TInt MediaVolume() const;

        /**
        * To access home page's type: AP/user defined.
        * @since Series60 2.6
        * @return value of setting
        */
        inline TWmlSettingsHomePage HomePageType() const;

        /**
        * To access Popup Blocking Enabled setting.
        * @since Series60 3.0
        * @return value of setting
        */
        inline TBool PopupBlocking() const;

        /**
        * To access Form Data Saving setting.
        * @since Series60 3.1
        * @return value of setting
        */
        inline TWmlSettingsFormData FormDataSaving() const;
        
        /**
        * To access Access Keys setting.
        * @since Series60 3.2
        * @return value of setting: 1-Enabled, 0-Disabled
        */
        inline TBool AccessKeys() const;

        /**
        * To modify DefaultAccessPoint setting
        * @since Series60 1.2
        * @param aDefaultAccessPoint Id of default accesspoint
        */
        void SetDefaultAccessPointL( TUint aDefaultAccessPoint, TUint aAssocVpn = KWmlNoDefaultAccessPoint );
        
        /**
        * Set default snapid
        * @since Series60 1.2
        * @param
        */
       void SetDefaultSnapId (TUint aSnapId);

        /**
        * To modify AutoLoadContent setting
        * @since Series60 1.2
        * @param aAutoLoadContent Change the setting on/off
        */
        void SetAutoLoadContentL( TInt aAutoLoadContent );

        /**
        * To modify PageOverview setting
        * @param aPageOverview Change the setting on/off
        */
        void SetPageOverviewL( TBool aPageOverview );
        
        /**
        * To modify BackList setting
        * @param aBackList Change the setting on/off
        */
        void SetBackListL( TBool aBackList );
        
        /**
        * To modify AutoRefresh setting
        * @param aAutoRefresh Change the setting on/off
        */
        void SetAutoRefreshL( TBool aAutoRefresh );

        /**
        * To modify TextWrap setting
        * @since Series60 1.2
        * @param aTextWrap Change the setting on/off
        */
        void SetTextWrapL( TBool aTextWrap );

        /**
        * To modify FontSize setting
        * @since Series60 1.2
        * @param aFontSize Set new font size
        */
        void SetFontSizeL( TInt aFontSize );

        /**
        * To modify Encoding setting
        * @since Series60 1.2
        * @param aEncoding Set new font size
        */
        void SetEncodingL( TUint32 aEncoding );

        /**
        * To modify Adaptive Bookmarks setting
        * @since Series60 1.2
        * @param aAdaptiveBookmarks 
        */
        void SetAdaptiveBookmarksL( TWmlSettingsAdaptiveBookmarks aAdaptiveBookmarks );

        /**
        * To modify FullScreen setting
        * @since Series60 2.1
        * @param aFullScreen 
        */
        void SetFullScreenL( TWmlSettingsFullScreen aFullScreen );  

        /**
        * To modify Cookies setting
        * @since Series60 1.2
        * @param aCookies 
        */
        void SetCookiesL( TWmlSettingsCookies aCookies );

        /**
        * To modify ECMA setting
        * @since Series60 2.x
        * @param aEcma 
        */
        void SetEcmaL( TWmlSettingsECMA aEcma );

        /**
        * To modify ScriptLog setting
        * @since Series60 3.2
        * @param aScriptLog 
        */
        void SetScriptLogL( TUint32 aScriptLog );

        /**
        * To modify HTTP security warnings setting
        * @since Series60 1.2S
        * @param aWarningsOn True to enable security warnings, false to disable
        */
        void SetHttpSecurityWarningsL( TBool aWarningsOn );

        /**
        * To modify IMEI notification setting
        * @since Series60 2.6
        * @param aIMEINotification True to enable IMEI notification
        */
        void SetIMEINotificationL( TWmlSettingsIMEI aIMEINotification );

        /**
        * To modify MediaVolume setting
        * @since Series60 2.6
        * @param aMediaVolume Change the media volume value
        */
        void SetMediaVolumeL( TInt aMediaVolume );

        /**
        * To modify Downloads Open setting
        * @since Series60 3.x
        * @param aOpen True to enable download open 
        */
        void SetDownloadsOpenL( TBool aOpen );

        /**
        * To flush all the shared data key settings
        * @since Series60 2.6
        */
        void FlushSD();

        /**
        * To modify HomePage's type setting
        * @since Series60 2.6
        * @param aHomePageType Change the home page type: AP/user defined.
        */
        void SetHomePageTypeL( TWmlSettingsHomePage aHomePageType );

        /**
        * To modify user-defined HomePage.
        * @since Series60 2.6
        * @param aHomePageURL Change the home page's url.
        */
        void SetHomePageUrlL( const TDesC& aHomePageURL );

        /**
        * To modify user-defined Search Page URL
        * @since Series60 2.6
        * @param aSearchPageURL Change the search page's url.
        */
        void SetSearchPageUrlL( const TDesC& aSearchPageURL );

        /**
        * To modify user-defined Pop-up blocking enabled setting
        * @since Series60 3.0
        * @param aPopupBlocking Change setting on/off
        */
        void SetPopupBlockingL( TBool aPopupBlocking );

        /**
        * To modify Form Data Saving setting
        * @since Series60 3.1
        * @param aFormDataSaving Change setting off/form data only/ form data plus password
        */
        void SetFormDataSavingL( TWmlSettingsFormData aFormDataSaving );

        /**
        * To access to the list of self download content types.
        * See CBrCtlInterface::SetSelfDownloadContentTypesL()!
        */
        TPtrC SelfDownloadContentTypesL();

        /**
        * To access Default Access Point selection mode settings.
        */        
        TCmSettingSelectionMode AccessPointSelectionMode();

        /**
        * To modify Default Access Point Settings
        */
        void SetAccessPointSelectionModeL( TCmSettingSelectionMode aAccessPointSelectionMode );


        /**
        * To access pre defined short cut keys
        */
        inline HBufC* ShortCutFuncHomePg() const;

        /**
        * To access pre defined short cut keys
        */
        inline HBufC* ShortCutFuncBkMark() const;

        /**
        * To access pre defined short cut keys
        */
        inline HBufC* ShortCutFuncFindKeyWord() const;

        /**
        * To access pre defined short cut keys
        */
        inline HBufC* ShortCutFuncPrePage() const;

        /**
        * To access pre defined short cut keys
        */
        inline HBufC* ShortCutFuncSwitchWin() const;

        /**
        * To access pre defined short cut keys
        */
        inline HBufC* ShortCutFuncMiniImage() const;

        /**
        * To access pre defined short cut keys
        */
        inline HBufC* ShortCutFuncFullScreen() const;

        /**
        * To access pre defined short cut keys
        */
        inline HBufC* ShortCutFuncGoAddr() const;

        /**
        * To access pre defined short cut keys
        */
        inline HBufC* ShortCutFuncZoomIn() const;

        /**
        * To access pre defined short cut keys
        */
        inline HBufC* ShortCutFuncZoomOut() const;

        /**
        * To access pre defined short cut keys
        */
        inline HBufC* ShortCutFuncZoomMode() const;

        /**
        * To access rotate display setting
        */
        inline TBool RotateDisplay() const;

        /** 
        * To get Toolbar On/Off value
        */
        inline TInt ShowToolbarOnOff() const;
                
        /** 
        * To get Toolbar Button 1 command value
        */
        inline TInt ShowToolbarButton1Cmd() const;

        /** 
        * To set Toolbar On/Off value
        */
        void SetToolbarOnOffL(TInt aCommand);
        
        /** 
        * To set Toolbar Button 1 command value
        */
        void SetToolbarButton1CmdL(TInt aCommand);
        
        /** 
        * To get Toolbar Button 2 command value
        */
        inline TInt ShowToolbarButton2Cmd() const;
       
        /** 
        * To set Toolbar Button 2 command value
        */
        void SetToolbarButton2CmdL(TInt aCommand);
        
        /** 
        * To get Toolbar Button 3 command value
        */
        inline TInt ShowToolbarButton3Cmd() const;
       
        /** 
        * To set Toolbar Button 3 command value
        */
        void SetToolbarButton3CmdL(TInt aCommand);
        
        /** 
        * To get Toolbar Button 4 command value
        */
        inline TInt ShowToolbarButton4Cmd() const;
       
        /** 
        * To set Toolbar Button 4 command value
        */
        void SetToolbarButton4CmdL(TInt aCommand);
        
        /** 
        * To get Toolbar Button 5 command value
        */
        inline TInt ShowToolbarButton5Cmd() const;
       
        /** 
        * To set Toolbar Button 5 command value
        */
        void SetToolbarButton5CmdL(TInt aCommand);
        
        /** 
        * To get Toolbar Button 6 command value
        */
        inline TInt ShowToolbarButton6Cmd() const;
       
        /** 
        * To set Toolbar Button 6 command value
        */
        void SetToolbarButton6CmdL(TInt aCommand);
        
        /** 
        * To get Toolbar Button 7 command value
        */
        inline TInt ShowToolbarButton7Cmd() const;
       
        /** 
        * To set Toolbar Button 7 command value
        */
        void SetToolbarButton7CmdL(TInt aCommand);

        /** 
        * To get Shortcut Key 1 command value
        */
        inline TInt ShortcutKey1Cmd() const;
       
        /** 
        * To set Shortcut Key 1 command value
        */
        void SetShortcutKey1CmdL(TInt aCommand);
 
         /** 
        * To get Shortcut Key 2 command value
        */
        inline TInt ShortcutKey2Cmd() const;
       
        /** 
        * To set Shortcut Key 2 command value
        */
        void SetShortcutKey2CmdL(TInt aCommand);

        /** 
        * To get Shortcut Key 3 command value
        */
        inline TInt ShortcutKey3Cmd() const;
       
        /** 
        * To set Shortcut Key 3 command value
        */
        void SetShortcutKey3CmdL(TInt aCommand);

        /** 
        * To get Shortcut Key 4 command value
        */
        inline TInt ShortcutKey4Cmd() const;
       
        /** 
        * To set Shortcut Key 4 command value
        */
        void SetShortcutKey4CmdL(TInt aCommand);

        /** 
        * To get Shortcut Key 5 command value
        */
        inline TInt ShortcutKey5Cmd() const;
       
        /** 
        * To set Shortcut Key 5 command value
        */
        void SetShortcutKey5CmdL(TInt aCommand);

        /** 
        * To get Shortcut Key 6 command value
        */
        inline TInt ShortcutKey6Cmd() const;
       
        /** 
        * To set Shortcut Key 6 command value
        */
        void SetShortcutKey6CmdL(TInt aCommand);

        /** 
        * To get Shortcut Key 7 command value
        */
        inline TInt ShortcutKey7Cmd() const;
       
        /** 
        * To set Shortcut Key 7 command value
        */
        void SetShortcutKey7CmdL(TInt aCommand);

        /** 
        * To get Shortcut Key 8 command value
        */
        inline TInt ShortcutKey8Cmd() const;
       
        /** 
        * To set Shortcut Key 8 command value
        */
        void SetShortcutKey8CmdL(TInt aCommand);

        /** 
        * To get Shortcut Key 9 command value
        */
        inline TInt ShortcutKey9Cmd() const;
       
        /** 
        * To set Shortcut Key 9 command value
        */
        void SetShortcutKey9CmdL(TInt aCommand);

        /** 
        * To get Shortcut Key 0 command value
        */
        inline TInt ShortcutKey0Cmd() const;
       
        /** 
        * To set Shortcut Key 0 command value
        */
        void SetShortcutKey0CmdL(TInt aCommand);
        
        /** 
        * To get Shortcut Key Star command value
        */
        inline TInt ShortcutKeyStarCmd() const;

        /** 
        * To set Shortcut Key Star command value
        */
        void SetShortcutKeyStarCmdL(TInt aCommand);

        /** 
        * To get Shortcut Key Hash command value
        */
        inline TInt ShortcutKeyHashCmd() const;
            
        /** 
        * To set Shortcut Key Hash command value
        */
        void SetShortcutKeyHashCmdL(TInt aCommand);
        
        /** 
        * To get Shortcut Key for Qwerty command value
        */
        inline TInt ShortcutKeysForQwerty() const;

        /** 
        * To get Zoom Level minimum value
        */
        inline TUint32 ZoomLevelMinimum() const;

        /** 
        * To get Zoom Level maximum value
        */
        inline TUint32 ZoomLevelMaximum() const;

        /** 
        * To get Zoom Level default value
        */
        inline TUint32 ZoomLevelDefault() const;
        
        /** 
        * To get Maximum number of Recent URLs to be displayed in pop up
        */
        inline TInt MaxRecentUrls() const;

        /** 
        * To get Search Feature Flag.
        */
        inline TInt SearchFeature() const;

        /** 
        * To get Service Feature Flag.
        */
        inline TInt ServiceFeature() const;
		
        /** 
        * To get cursor show mode
        */
        inline TBrCtlDefs::TCursorSettings CursorShowMode() const;
        
        inline TBrCtlDefs::TEnterKeySettings EnterKeyMode() const;
        
    private:

        /**
        * Constructor.
        * @param aCommsModel Comms model.
        */
        CBrowserPreferences( MCommsModel& aCommsModel, MApiProvider& aApiProvider );

        /**
        * By default constructor is private.
        */
        void ConstructL( TBrowserOverriddenSettings* aSettings );
        TInt GetSearchFeatureStatusL( TUint32 aKey, RApaLsSession& aLs );
    private: 

        MApiProvider& iApiProvider; // enables access to CBrowserAppUi

        // Application settings to store in file        
        TUint   iCustomAp;
    public:
        enum        ///< Text font size
            {
            EAllSmall,
            ESmall,
            ENormal,
            ELarge,
            EAllLarge
            } TFontSize;
    private:
        // Observers
        CArrayPtrFlat< MPreferencesObserver >* iObservers;   
        MCommsModel& iCommsModel;

        CRepository* iRepository;
        CApDataHandler* iApDH;
        CVpnApItem* iVpnItem;
        CVpnApEngine* iVpnEngine;        
        
        // OSS needs a separate repository for non shared settings.
        
        TInt iEngineLocalFeaturesBitmask; //Engine Local Features Bitmask

        TInt iUiLocalFeaturesBitmask; //Ui Local Features Bitmask

        TBrowserOverriddenSettings* iOverridenSettings;

        TBool iEmbeddedMode; //ETrue = the browser is in embedded mode

        TInt iCdmaUiLocalFeaturesBitmask;  // CDMA UI Local Features Bitmask

        TBool iOverriden;//Settings were overriden or not

        HBufC* iSelfDownloadContentTypes; ///< List of such content types. Owned.

        // A buffer struct for holding each setting value
        TPreferencesValues  iAllPreferences;
    };

#include "BrowserPreferences.inl"

// Toolbar button command translation matrix
// 
//  translates cenrep value to browser command and setting string resource id 
//  for each toolbar button
//
//  NOTE: THIS MUST STAY IN SYNC WITH SAME DEFINITION IN toolbar.h in webengine.
// 
typedef const TInt TToolbarCommandDecodeEntry[1];

#define KToolbarCommandDecodeMatrixCount  23
#define KToolbarCommandDecodeMatrixStringId  0 // first entry is the Setting String Id

static const TToolbarCommandDecodeEntry    ToolbarCommandDecodeMatrix[] =
{
    // Index #          Setting String Id
    // = cenrep value
       /* 0  */       { R_BROWSER_SETTING_NONE               },
       /* 1  */       { R_BROWSER_SETTING_SHOW_HELP          },
       /* 2  */       { R_BROWSER_SETTING_SWITCH_WINDOW      },
       /* 3  */       { R_BROWSER_SETTING_SAVE_PAGE          },
       /* 4  */       { R_BROWSER_SETTING_VIEW_IMAGES        },
       /* 5  */       { R_BROWSER_SETTING_VISUAL_HISTORY     },
       /* 6  */       { R_BROWSER_SETTING_MANAGE_BOOKMARKS   },
       /* 7  */       { R_BROWSER_SETTING_SUBSCRIBE_TO_FEEDS },
       /* 8  */       { R_BROWSER_SETTING_ROTATE_SCREEN      },
       /* 9  */       { R_BROWSER_SETTING_SETTINGS           },
       /* 10 */       { R_BROWSER_SETTING_ZOOM_OUT           },
       /* 11 */       { R_BROWSER_SETTING_ZOOM_IN            },
       /* 12 */       { R_BROWSER_SETTING_GO_TO_HOMPAGE      },
       /* 13 */       { R_BROWSER_SETTING_PREVIOUS_PAGE      },
       /* 14 */       { R_BROWSER_SETTING_MINIATURE_SHOW     },
       /* 15 */       { R_BROWSER_SETTING_RELOAD             },
       /* 16 */       { R_BROWSER_SETTING_FIND_KEYWORD       },
       /* 17 */       { R_BROWSER_SETTING_RECENT_URLS        }, // toolbar only - recent urls drop down list
       /* 18 */       { R_BROWSER_SETTING_SAVE_AS_BOOKMARK   },
       /* 19 */       { R_BROWSER_SETTING_LIST_BOOKMARKS     }, // toolbar only - bookmarks drop down list
       /* 20 */       { R_BROWSER_SETTING_GO_TO_WEB_ADDRESS  },
       /* 21 */       { R_BROWSER_SETTING_SHOW_KEYMAP        },
       /* 22 */       { R_BROWSER_SETTING_FULL_SCREEN        },
};


// Shortcut Keys command translation matrix
// 
//  translates cenrep value to browser command and setting string resource id 
//  for each shortcut key
//
// 
typedef const TInt TShortcutsCommandDecodeEntry[2];

#define KShortcutsCommandDecodeMatrixCount      22
#define KShortcutsCommandDecodeMatrixCmdVal     0   // First entry is the Command Value
#define KShortcutsCommandDecodeMatrixStringId   1   // Second entry is the Setting String Id

static const TShortcutsCommandDecodeEntry    ShortcutsCommandDecodeMatrix[] =
{
    // Index #    Command Value                     Setting String Id
    // = cenrep value
    /* 0  */    { EWmlNoCmd,                        R_BROWSER_SETTING_NONE               },
    /* 1  */    { EAknCmdHelp,                      R_BROWSER_SETTING_SHOW_HELP          },
    /* 2  */    { EWmlCmdSwitchWindow,              R_BROWSER_SETTING_SWITCH_WINDOW      },
    /* 3  */    { EWmlCmdSavePage,                  R_BROWSER_SETTING_SAVE_PAGE          },
    /* 4  */    { EWmlCmdShowImages,                R_BROWSER_SETTING_VIEW_IMAGES        },
    /* 5  */    { EWmlCmdHistory,                   R_BROWSER_SETTING_VISUAL_HISTORY     },
    /* 6  */    { EWmlCmdFavourites,                R_BROWSER_SETTING_MANAGE_BOOKMARKS   },
    /* 7  */    { EWmlCmdShowSubscribeList,         R_BROWSER_SETTING_SUBSCRIBE_TO_FEEDS },
    /* 8  */    { EWmlCmdRotateDisplay,             R_BROWSER_SETTING_ROTATE_SCREEN      },
    /* 9  */    { EWmlCmdPreferences,               R_BROWSER_SETTING_SETTINGS           },
    /* 10 */    { EWmlCmdZoomOut,                   R_BROWSER_SETTING_ZOOM_OUT           },
    /* 11 */    { EWmlCmdZoomIn,                    R_BROWSER_SETTING_ZOOM_IN            },
    /* 12 */    { EWmlCmdLaunchHomePage,            R_BROWSER_SETTING_GO_TO_HOMPAGE      },
    /* 13 */    { EWmlCmdOneStepBack,               R_BROWSER_SETTING_PREVIOUS_PAGE      },
    /* 14 */    { EWmlCmdShowMiniature,             R_BROWSER_SETTING_MINIATURE_SHOW     },
    /* 15 */    { EWmlCmdReload,                    R_BROWSER_SETTING_RELOAD             },
    /* 16 */    { EWmlCmdFindKeyword,               R_BROWSER_SETTING_FIND_KEYWORD       },
    /* 17 */    { EWmlCmdSaveAsBookmark,            R_BROWSER_SETTING_SAVE_AS_BOOKMARK   },
    /* 18 */    { EWmlCmdGoToAddress,               R_BROWSER_SETTING_GO_TO_WEB_ADDRESS  },
    /* 19 */    { EWmlCmdShowToolBar,               R_BROWSER_SETTING_SHOW_TOOLBAR       }, 
    /* 20 */    { EWmlCmdShowShortcutKeymap,        R_BROWSER_SETTING_SHOW_KEYMAP        },
    /* 21 */    { EWmlCmdEnterFullScreenBrowsing,   R_BROWSER_SETTING_FULL_SCREEN        },
};

#endif  //BROWSER_PREFERENCES_H

// End of File
