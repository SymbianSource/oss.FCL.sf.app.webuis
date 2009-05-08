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
*      Interface for Preferences handling.
*
*
*/


#ifndef MPREFERENCES_H
#define MPREFERENCES_H

// INCLUDES
#include <e32def.h>
#include <e32const.h>
#include <e32std.h>

#include "Browser.hrh"
#include <bldvariant.hrh>

#include <BrCtlDefs.h>
#include <ApAccessPointItem.h>

#include <CmApplicationSettingsUi.h>

using namespace CMManager;
// FORWARD DECLARATIONS

// CONSTS

const TUint KWmlNoDefaultAccessPoint = KMaxTUint;
const TUint KWmlNoDefaultSnapId = KMaxTUint;

const TInt KShortCutFuncStringLength = 32;

const TInt KUrlSuffixMaxLength = 256;

// CLASS DECLARATION
class CVpnApItem;

//=============================================================================
// Observer for the MPreferences Class
//

/**
*   Helper struct for MPreferencesObserver
*/
struct TPreferencesValues
    {
    // Setting Values
    TUint                           iDefaultAccessPoint;
    TUint 							iDefaultSnapId;
    CApAccessPointItem* 			iDefaultAPDetails;  //cache all data
    // It is stored because of VPN. Default value = KWmlNoDefaultAccessPoint,
    // means it is not VPN AP, otherwise iAssocVpn = iDefaultAccessPoint
    // iDefaultAPDetails always stores real AP specific settings.
    TUint                           iAssocVpn;
    TCmSettingSelectionMode         iAccessPointSelectionMode;
    TInt                            iAutoLoadContent;
    TInt                            iFontSize;
    TBool                           iTextWrap;
    TWmlSettingsCookies             iCookies;
    TBool   						iPageOverview;
    TBool   						iBackList;
    TBool   						iAutoRefresh;
    TWmlSettingsECMA                iEcma;
    TWmlSettingsIMEI                iIMEINotification;
    TUint32                         iEncoding;
    TWmlSettingsFullScreen          iFullScreen;
    TBool                           iQueryOnExit;
    TBool                           iSendReferrer;
    TWmlSettingsAdaptiveBookmarks   iAdaptiveBookmarks;
    TWmlSettingsHomePage            iHomePgType;
    TBool                           iHTTPSecuritySupressed;
    TBool                           iDownloadsOpen;    
    TBool                           iConnDialogs;
    TBool                           iHttpSecurityWarnings;
    TInt                            iMediaVolume;
    HBufC*                          iSearchPgURL;
    HBufC*                          iHomePgURL;
    TBool                           iPopupBlocking;
    TWmlSettingsFormData            iFormDataSaving;
    TUint                           iAutomaticUpdatingAP;
    TBool                           iAutomaticUpdatingWhileRoaming;
    TBool                           iAccessKeys;
    HBufC*                          iShortCutFuncHomePg;
    HBufC*                          iShortCutFuncBkMark;
    HBufC*                          iShortCutFuncFindKeyWord;
    HBufC*                          iShortCutFuncPrePage;
    HBufC*							iUrlSuffixList;
    HBufC*                          iShortCutFuncSwitchWin;
    HBufC*                          iShortCutFuncMiniImage;
    HBufC*                          iShortCutFuncFullScreen;
    HBufC*                          iShortCutFuncGoAddr;
    HBufC*                          iShortCutFuncZoomIn;
    HBufC*                          iShortCutFuncZoomOut;
    HBufC*                          iShortCutFuncZoomMode;
    TBool                           iRotateDisplay;
    TInt                            iToolbarOnOff;          // Turns Toolbar On or Off ( 0 = Off ; 1 = On)
    TInt                            iToolbarButton1Cmd;     // see Toolbar Button Command List for cenrep values
    TInt                            iToolbarButton2Cmd;     // see Toolbar Button Command List for cenrep values
    TInt                            iToolbarButton3Cmd;     // see Toolbar Button Command List for cenrep values
    TInt                            iToolbarButton4Cmd;     // see Toolbar Button Command List for cenrep values
    TInt                            iToolbarButton5Cmd;     // see Toolbar Button Command List for cenrep values
    TInt                            iToolbarButton6Cmd;     // see Toolbar Button Command List for cenrep values
    TInt                            iToolbarButton7Cmd;     // see Toolbar Button Command List for cenrep values
	TInt							iShortcutKeysForQwerty;
	TInt 							iShortcutKey1Cmd;    
	TInt 							iShortcutKey2Cmd;    
	TInt 							iShortcutKey3Cmd;    
	TInt 							iShortcutKey4Cmd;    
	TInt 							iShortcutKey5Cmd;    
	TInt 							iShortcutKey6Cmd;    
	TInt 							iShortcutKey7Cmd;    
	TInt 							iShortcutKey8Cmd;    
	TInt 							iShortcutKey9Cmd;    
	TInt 							iShortcutKey0Cmd;    
	TInt 							iShortcutKeyStarCmd;    
	TInt 							iShortcutKeyHashCmd;
	TUint32							iScriptLog;
	// Minimum, Maximum and Default (for new page) Zoom Level settings
	TUint32							iZoomLevelMin;
	TUint32							iZoomLevelMax;
	TUint32							iZoomLevelDefault;
	TInt							iMaxRecentUrlsToShow;
    TBool                           iSearch;                // Search Feature
    TBool                           iService;               // Service Feature
    };

/**
*   Helper enum for MPreferencesObserver
*/
enum TPreferencesEvent 
    {
	EPreferencesActivate,   // Preferences opened
	EPreferencesDeactivate, // Preferences closed
	EPreferencesItemChange  // Setting change has occurred
    };


/**
* Mixin class for observing changes in preferences.
*/
class MPreferencesObserver
	{
	public:

        /**
        * Derived classes should implement this method, and act accordingly.
        */
		virtual void HandlePreferencesChangeL( 
	                            const TPreferencesEvent aEvent,
                                TPreferencesValues& aValues,
                                TBrCtlDefs::TBrCtlSettings aSettingType ) = 0;
	};
	
//=============================================================================

/**
*  Interface for Preferences handling.
*/
class MPreferences
    {
    public: // New functions

        /**
        * To read all of the preferences in one go
        */
        virtual const TPreferencesValues& AllPreferencesL() = 0;
        
        /**
        * To access embedded mode
        */
        virtual TBool EmbeddedMode() const = 0;
        
        /**
		* To access DefaultAccessPoint setting
        */
		virtual TUint DefaultAccessPoint() const = 0;
		
	
		/**
		* To access DefaultSnapId setting
        */

		virtual TUint DefaultSnapId() const = 0;
		
        /**
        * To access deafult AP details if exist setting.
        */         
        virtual CApAccessPointItem* DefaultAPDetails() = 0;		
		
		/**
		* To access Associated VPN Ap if exist
		*/
		virtual TUint AssociatedVpn() const = 0;
		
		/**
		* Get the vpn item if it is vpn
		*/		
		virtual TBool VpnDataL( TUint aDefaultAccessPoint, CVpnApItem& aVpnItem ) = 0;

        /**
		* To access AutoLoadContent setting
        */
		virtual TInt AutoLoadContent() const = 0;

        /**
		* To access PageOverview setting
        */
		virtual TBool PageOverview() const = 0;
		
		/**
		* To access BackList setting
        */
		virtual TBool BackList() const = 0;
		
		/**
		* To access AutoRefresh setting
        */
		virtual TBool AutoRefresh() const = 0;
		
		/*
        * Sets the automatic updating access point of Web Feeds 
        */
        virtual void SetAutomaticUpdatingApL( TUint aSetting ) = 0;
		/*
        * Sets the automatic updating while roaming of News & Blogs 
        */
        virtual void SetAutomaticUpdatingWhileRoamingL( TBool aAutoUpdateRoaming ) = 0;
        
        /**
        * To modify user-defined URL suffix list (.com, .org, .net)
        */
        virtual void SetURLSuffixList( HBufC* aValue ) = 0;
        
		/**
		* To access user-defined URL suffix list (.com, org, .net)
        */
        virtual HBufC* URLSuffixList( ) const = 0;
        /**
        * automatic updating access point of Web Feeds 
        */
        virtual TUint AutomaticUpdatingAP() const = 0;
        /**
        * automatic updating while roaming of News & Blogs 
        */
        virtual TBool AutomaticUpdatingWhileRoaming() const = 0;
        
        /**
		* To access TextWrap setting
        @ return setting item value
        */
		virtual TBool TextWrap() const = 0;

        /**
		* To access FontSize setting
        */
		virtual TInt FontSize() const = 0;

        /**
        * To access Encoding setting
        */
        virtual TUint32 Encoding() const = 0;

        /**
		* To access Adaptive Bookmarks setting
        */
		virtual enum TWmlSettingsAdaptiveBookmarks AdaptiveBookmarks() const = 0;

        /**
 		* To access FullScreen setting
        */
 		virtual enum TWmlSettingsFullScreen FullScreen() const = 0;

        /**
		* To access Cookies setting
        */
		virtual enum TWmlSettingsCookies Cookies() const = 0;

        /**
		* To access ECMA setting
        */
		virtual enum TWmlSettingsECMA Ecma() const = 0;

        /**
		* To access ScriptLog setting
        */
		virtual TUint32 ScriptLog() const = 0;

		/**
		* To access HTTP security warning setting
        */
		virtual TBool HttpSecurityWarnings() const = 0;

		/**
		* To access IMEI notification setting
		*/
		virtual enum TWmlSettingsIMEI IMEINotification() const = 0;

		/**
		*To access HTTP security warning supression - read only pref.
		*/
		virtual TBool HttpSecurityWarningsStatSupressed() const = 0;

        /**
		* To access Downloads Open setting
        */
		virtual TBool DownloadsOpen() const = 0;
		     
		/**
		* To access user-defined homepage URL setting
        */
		virtual TInt HomePageUrlL( TDes& aUrl, TBool aForceLoadFromSD = EFalse ) const = 0;

		/**
		* Get the home page URL from belongs to the access point.
        */
        virtual TInt HomePageFromIapL( TDes& aUrl, TUint aIap ) const = 0;

        /**
        * To access user-defined search page URL setting
        */
        virtual TPtrC SearchPageUrlL() = 0;

		/**
		* To access Query Exit setting
        */
        virtual TBool QueryOnExit() const = 0;

		/**
		* To access Send Referrer setting
        */
        virtual TBool SendReferrer() const = 0;

		/**
		* To access Media Volume setting
        */
        virtual TInt MediaVolume() const = 0;

		/**
		* To access ShowConnectionDialogs setting
		*/
		virtual TBool ShowConnectionDialogs() const = 0;

		/**
		* To check that an engine local feature is supported or not
		*/

		virtual TBool EngineLocalFeatureSupported( const TInt aFeature ) const  = 0;

		/**
		* To check that a ui local feature is supported or not
		*/

		virtual TBool UiLocalFeatureSupported( const TInt aFeature ) const  = 0;

        /**
        * To access home page's type: AP/user defined.
        */
        virtual enum TWmlSettingsHomePage HomePageType() const = 0;

        /**
        * To access Popup Blocking Enabled setting#
        */
        virtual TBool PopupBlocking() const = 0;

        /**
        * To access Form Data Saving setting
        */
        virtual TWmlSettingsFormData FormDataSaving() const = 0;
        
        /**
        * To access Access Keys setting : Enabled/Disabled
        */
        virtual TBool AccessKeys() const = 0;

        /**
		* To modify DefaultAccessPoint setting
        */
		virtual void SetDefaultAccessPointL( TUint aDefaultAccessPoint, TUint aAssocVpn = KWmlNoDefaultAccessPoint  ) = 0;
		
		/**
		* To modify DefaultSnapId setting
        */
		virtual void SetDefaultSnapId (TUint aSnapId) = 0;		

        /**
		* To modify AutoLoadContent setting
        */
		virtual void SetAutoLoadContentL( TInt aAutoLoadContent ) = 0;

        /**
		* To modify PageOverview setting
        */
		virtual void SetPageOverviewL( TBool aPageOverview ) = 0;
		
		/**
		* To modify BackList setting
        */
		virtual void SetBackListL( TBool aBackList ) = 0;
		
		/**
		* To modify AutoRefresh setting
        */
		virtual void SetAutoRefreshL( TBool aAutoRefresh ) = 0;

        /**
		* To modify TextWrap setting
        */
		virtual void SetTextWrapL( TBool aTextWrap ) = 0;

        /**
		* To modify FontSize setting
        */
		virtual void SetFontSizeL( TInt aFontSize ) = 0;

        /**
        * To modify encoding settings
        */
        virtual void SetEncodingL( TUint32 aEncoding ) = 0;

        /**
		* To modify Adaptive Bookmarks setting
        */
		virtual void SetAdaptiveBookmarksL( TWmlSettingsAdaptiveBookmarks aAdaptiveBookmarks ) = 0;

        /**
 		* To modify FullScreen setting
        */
 		virtual void SetFullScreenL( TWmlSettingsFullScreen aFullScreen ) = 0;

        /**
		* To modify Cookies setting
        */
		virtual void SetCookiesL( TWmlSettingsCookies aCookies ) = 0;

        /**
 		* To modify ECMA setting
        */
 		virtual void SetEcmaL( TWmlSettingsECMA aEcma ) = 0;

        /**
 		* To modify Script Console setting
        */
 		virtual void SetScriptLogL( TUint32 aScriptLog ) = 0;

		/**
		* To modify HTTP security warnings setting
        */
		virtual void SetHttpSecurityWarningsL( TBool aWarningsOn ) = 0;

		/**
		* To modify IMEI notification setting
		*/
		virtual void SetIMEINotificationL( TWmlSettingsIMEI aIMEINotification )  = 0;

        /**
        * To modify Downloads Open setting
        */
        virtual void SetDownloadsOpenL( TBool aOpen ) = 0;

		/**
		* To modify Media Volume setting
        */
		virtual void SetMediaVolumeL( TInt aMediaVolume ) = 0;

        /**
        * To modify HomePage's type: AP/user defined.
        */
        virtual void SetHomePageTypeL( TWmlSettingsHomePage aHomePageType ) = 0;

        /**
        * To modify user-defined HomePage.
        */
        virtual void SetHomePageUrlL( const TDesC& aHomePageURL ) = 0;

        /**
        * To modify user-defined Search page URL
        */
        virtual void SetSearchPageUrlL( const TDesC& aSearchPageURL ) = 0;

        /**
        * To determine that is there an overridden default access point defined or not
        */
        virtual TBool CustomAccessPointDefined() = 0;

        /**
        * To access to the list of self download content types.
        * See CBrCtlInterface::SetSelfDownloadContentTypesL()!
        */
        virtual TPtrC SelfDownloadContentTypesL() = 0;

	    /**
	    * To access Default Access Point selection mode settings.
        */        
        virtual TCmSettingSelectionMode AccessPointSelectionMode() = 0;

        /**
		* To modify Default Access Point Settings
        */
        virtual void SetAccessPointSelectionModeL( TCmSettingSelectionMode aAccessPointSelectionMode ) = 0;

        /**
        * To modify Popup Blocking Enabled settign
        */
        virtual void SetPopupBlockingL( TBool aPopupBlocking ) = 0;
        
        /**
        * To modify Form Data Saving setting
        */
        virtual void SetFormDataSavingL( TWmlSettingsFormData aFormDataSaving ) = 0;

        /**
        * To access pre defined short cut keys
        */
        virtual HBufC* ShortCutFuncHomePg() const = 0;

        /**
        * To access pre defined short cut keys
        */
        virtual HBufC* ShortCutFuncBkMark() const = 0;

        /**
        * To access pre defined short cut keys
        */
        virtual HBufC* ShortCutFuncFindKeyWord() const = 0;

        /**
        * To access pre defined short cut keys
        */
        virtual HBufC* ShortCutFuncPrePage() const = 0;

        /**
        * To access pre defined short cut keys
        */
        virtual HBufC* ShortCutFuncSwitchWin() const = 0;

        /**
        * To access pre defined short cut keys
        */
        virtual HBufC* ShortCutFuncMiniImage() const = 0;

        /**
        * To access pre defined short cut keys
        */
        virtual HBufC* ShortCutFuncFullScreen() const = 0;

        /**
        * To access pre defined short cut keys
        */
        virtual HBufC* ShortCutFuncGoAddr() const = 0;

        /**
        * To access pre defined short cut keys
        */
        virtual HBufC* ShortCutFuncZoomIn() const = 0;

        /**
        * To access pre defined short cut keys
        */
        virtual HBufC* ShortCutFuncZoomOut() const = 0;

        /**
        * To access pre defined short cut keys
        */
        virtual HBufC* ShortCutFuncZoomMode() const = 0;

        /**
        * To get value of Rotate Display setting
        */
        virtual TBool RotateDisplay() const = 0;
    
        /** 
        * To get Toolbar On/Off value
        */
        virtual TInt ShowToolbarOnOff() const = 0;
       
        /** 
        * To set Toolbar On/Off value
        */
        virtual void SetToolbarOnOffL(TInt aCommand) = 0;

        /** 
        * To get Toolbar Button 1 command value
        */
        virtual TInt ShowToolbarButton1Cmd() const = 0;
       
        /** 
        * To set Toolbar Button 1 command value
        */
        virtual void SetToolbarButton1CmdL(TInt aCommand) = 0;

        /** 
        * To get Toolbar Button 2 command value
        */
        virtual TInt ShowToolbarButton2Cmd() const = 0;
       
        /** 
        * To set Toolbar Button 2 command value
        */
        virtual void SetToolbarButton2CmdL(TInt aCommand) = 0;
        
        /** 
        * To get Toolbar Button 3 command value
        */
        virtual TInt ShowToolbarButton3Cmd() const = 0;
       
        /** 
        * To set Toolbar Button 3 command value
        */
        virtual void SetToolbarButton3CmdL(TInt aCommand) = 0;

        /** 
        * To get Toolbar Button 4 command value
        */
        virtual TInt ShowToolbarButton4Cmd() const = 0;
       
        /** 
        * To set Toolbar Button 4 command value
        */
        virtual void SetToolbarButton4CmdL(TInt aCommand) = 0;

        /** 
        * To get Toolbar Button 5 command value
        */
        virtual TInt ShowToolbarButton5Cmd() const = 0;
       
        /** 
        * To set Toolbar Button 5 command value
        */
        virtual void SetToolbarButton5CmdL(TInt aCommand) = 0;

        /** 
        * To get Toolbar Button 6 command value
        */
        virtual TInt ShowToolbarButton6Cmd() const = 0;
       
        /** 
        * To set Toolbar Button 6 command value
        */
        virtual void SetToolbarButton6CmdL(TInt aCommand) = 0;

        /** 
        * To get Toolbar Button 7 command value
        */
        virtual TInt ShowToolbarButton7Cmd() const = 0;
       
        /** 
        * To set Toolbar Button 7 command value
        */
        virtual void SetToolbarButton7CmdL(TInt aCommand) = 0;

        /** 
        * To get Shortcut Key 1 command value
        */
        virtual TInt ShortcutKey1Cmd() const = 0;
       
        /** 
        * To set Shortcut Key 1 command value
        */
        virtual void SetShortcutKey1CmdL(TInt aCommand) = 0;
 
         /** 
        * To get Shortcut Key 2 command value
        */
        virtual TInt ShortcutKey2Cmd() const = 0;
       
        /** 
        * To set Shortcut Key 2 command value
        */
        virtual void SetShortcutKey2CmdL(TInt aCommand) = 0;

        /** 
        * To get Shortcut Key 3 command value
        */
        virtual TInt ShortcutKey3Cmd() const = 0;
       
        /** 
        * To set Shortcut Key 3 command value
        */
        virtual void SetShortcutKey3CmdL(TInt aCommand) = 0;

        /** 
        * To get Shortcut Key 4 command value
        */
        virtual TInt ShortcutKey4Cmd() const = 0;
       
        /** 
        * To set Shortcut Key 4 command value
        */
        virtual void SetShortcutKey4CmdL(TInt aCommand) = 0;

        /** 
        * To get Shortcut Key 5 command value
        */
        virtual TInt ShortcutKey5Cmd() const = 0;
       
        /** 
        * To set Shortcut Key 5 command value
        */
        virtual void SetShortcutKey5CmdL(TInt aCommand) = 0;

        /** 
        * To get Shortcut Key 6 command value
        */
        virtual TInt ShortcutKey6Cmd() const = 0;
       
        /** 
        * To set Shortcut Key 6 command value
        */
        virtual void SetShortcutKey6CmdL(TInt aCommand) = 0;

        /** 
        * To get Shortcut Key 7 command value
        */
        virtual TInt ShortcutKey7Cmd() const = 0;
       
        /** 
        * To set Shortcut Key 7 command value
        */
        virtual void SetShortcutKey7CmdL(TInt aCommand) = 0;

        /** 
        * To get Shortcut Key 8 command value
        */
        virtual TInt ShortcutKey8Cmd() const = 0;
       
        /** 
        * To set Shortcut Key 8 command value
        */
        virtual void SetShortcutKey8CmdL(TInt aCommand) = 0;

        /** 
        * To get Shortcut Key 9 command value
        */
        virtual TInt ShortcutKey9Cmd() const = 0;
       
        /** 
        * To set Shortcut Key 9 command value
        */
        virtual void SetShortcutKey9CmdL(TInt aCommand) = 0;

        /** 
        * To get Shortcut Key 0 command value
        */
        virtual TInt ShortcutKey0Cmd() const = 0;
       
        /** 
        * To set Shortcut Key 0 command value
        */
        virtual void SetShortcutKey0CmdL(TInt aCommand) = 0;
        
        /** 
        * To get Shortcut Key Star command value
        */
        virtual TInt ShortcutKeyStarCmd() const = 0;

        /** 
        * To set Shortcut Key Star command value
        */
        virtual void SetShortcutKeyStarCmdL(TInt aCommand) = 0;

        /** 
        * To get Shortcut Key Hash command value
        */
        virtual TInt ShortcutKeyHashCmd() const = 0;
            
        /** 
        * To set Shortcut Key Hash command value
        */
        virtual void SetShortcutKeyHashCmdL(TInt aCommand) = 0;
        
        /** 
        * To get Shortcut Key for Qwerty command value
        */
        virtual TInt ShortcutKeysForQwerty() const = 0;

        /** 
        * Zoom Level minimum (percentage) value
        */
        virtual inline TUint32 ZoomLevelMinimum() const = 0;

        /** 
        * Zoom Level maximum (percentage) value
        */
        virtual inline TUint32 ZoomLevelMaximum() const = 0;

        /** 
        * Zoom Level default (percentage) value
        */
        virtual inline TUint32 ZoomLevelDefault() const = 0;
        
   		/**
		* Maximum Recent URLs to show
        */
        virtual TInt MaxRecentUrls() const = 0;

        /**
        * Search Feature Enabled or Disabled. 
        */
        virtual inline TBool SearchFeature() const = 0;
        
        /**
        * Service Feature Enabled or Disabled. 
        */
        virtual inline TBool ServiceFeature() const = 0;
                
        /**
        * To access an int value key in ini file
        * @param aKey the ini file key to read
        * @return the value of the given key
        */
        virtual TInt GetIntValue( TUint32 aKey) const = 0;

        /**
        * To access an string value key in ini file
        * @param aKey the ini file key to read
        * @return the value of the given key
        */
        virtual TInt GetStringValueL ( TUint32 aKey, TInt aMaxBufSize, TDes& aBuf ) const = 0;

        
    public:     // observer support

        /**
        * Adds an observer to be notified about changes. Derived classes MUST
        * notify all observers when preferences changed!
        * @param anObserver Observer to be notified about events.
        */
        virtual void AddObserverL( MPreferencesObserver* anObserver ) = 0;

        /**
        * Removes an observer.
        * @param anObserver The observer.
        */
        virtual void RemoveObserver( MPreferencesObserver* anObserver ) = 0;

		/**
		* To flush share data key settings
		*/
		virtual void FlushSD() = 0;
		
		virtual void NotifyObserversL( TPreferencesEvent aEvent,
                                    TBrCtlDefs::TBrCtlSettings aSettingType ) = 0;
    };

#endif
            
// End of File
