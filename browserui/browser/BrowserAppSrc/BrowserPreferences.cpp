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
*     It manages WmlBrowser's preferences
*
*
*/



// INCLUDE FILES
#include <s32stor.h>
#include <eikappui.h>
#include <eikapp.h>
#include <eikenv.h>

#include <ApSettingsHandlerUi.h>

#include <ApEngineConsts.h>
#include <aputils.h>
#include <ApDataHandler.h>
#include <ApDataHandler.h>
#include <ApAccessPointItem.h>
#include <ApSelect.h>
#include <ApListItem.h>
#include <ApListItemList.h>
#include <VpnAPEngine.h>
#include <VpnApItem.h>
#include <aknutils.h>

#include <COEMAIN.H>
#include <FeatMgr.h>
#include <BrCtlDefs.h>

#include "CommsModel.h"
#include "ApiProvider.h"
#include "BrowserApplication.h"
#include "BrowserPreferences.h"
#include "Browser.hrh"
#include "BrowserUiVariant.hrh"
#include <bldvariant.hrh>
#include "commonconstants.h"
#include "BrowserAppUi.h"
#include "BrowserOverriddenSettings.h"
#include "SettingsContainer.h"  // for TEncoding
#include "BrowserUtil.h" // for Util::AllocateUrlWithSchemeL
#include "logger.h"

#include "CmApplicationSettingsUi.h"
#include <cmmanager.h>
#include <cmdestination.h>

// CONSTANTS

//Shared data values for Full Screen setting
const TInt KBrowserFullScreenSettingDataSoftkeysOff			 = 0; //full screen
const TInt KBrowserFullScreenSettingDataSoftkeysOn			 = 1; //softkeys displayed

//Shared data values for Adaptive Bookmarks setting
const TInt KBrowserAdaptiveBookmarksSettingDataOn            = 0;
const TInt KBrowserAdaptiveBookmarksSettingDataHideFolder    = 1;
const TInt KBrowserAdaptiveBookmarksSettingDataOff           = 2;

// ========================= MEMBER FUNCTIONS ================================

// ----------------------------------------------------------------------------
// CBrowserPreferences::ConstructL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::ConstructL( TBrowserOverriddenSettings* aSettings )
    {
    LOG_ENTERFN("CBrowserPreferences::ConstructL");
    iObservers = new ( ELeave ) CArrayPtrFlat< MPreferencesObserver >( 1 );
    iCommsModel.ApDb().AddObserverL( this );
    	
	iApDH = CApDataHandler::NewLC( iCommsModel.CommsDb() );
	CleanupStack::Pop();
	
    iVpnEngine = CVpnApEngine::NewLC( &(iCommsModel.CommsDb()) );
    CleanupStack::Pop();	
	
 	CRepository* repository = CRepository::NewL( KCRUidBrowserUiLV );

    if ( repository->Get( 
         KBrowserUiCommonFlags, iEngineLocalFeaturesBitmask ) != KErrNone )
        {
        // in case of problems, assume everything is off
        iEngineLocalFeaturesBitmask = 0;
        } 
    if ( repository->Get( 
        KBrowserUiCommonFlags, iUiLocalFeaturesBitmask ) != KErrNone )
        {
        // in case of problems, assume everything is off
        iUiLocalFeaturesBitmask = 0;
        } 
    if ( repository->Get( 
        KBrowserUiCdmaFlags, iCdmaUiLocalFeaturesBitmask ) != KErrNone )
        {
        // in case of problems, assume everything is off
        iCdmaUiLocalFeaturesBitmask = 0;
        } 

    delete repository;

    iRepository = CRepository::NewL( KCRUidBrowser );

    iEmbeddedMode = ApiProvider().IsEmbeddedModeOn(); // EFalse;
    if( iEmbeddedMode )
        {
        // embedded mode doesn't allow multiple windows feature
        iUiLocalFeaturesBitmask &= ~KBrowserMultipleWindows;
        }
    iOverriden = EFalse;

    SetIntValueL( KBrowserConfirmedDTMFOnce, EFalse );

    //Read ini values to local variables.
    TRAP_IGNORE( RestoreSettingsL() );
    
    // and overwrite some of them if needed
    if ( aSettings )
        {
        SetOverriddenSettingsL( aSettings );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::EngineLocalFeatureSupported
// ----------------------------------------------------------------------------
//
TBool CBrowserPreferences::EngineLocalFeatureSupported( const TInt aFeature ) const
    {
LOG_ENTERFN("CBrowserPreferences::EngineLocalFeatureSupported");
BROWSER_LOG( ( _L( "EnginLocalFeatureSupported returns %d" ),
        (iEngineLocalFeaturesBitmask & aFeature) ) );
    return (iEngineLocalFeaturesBitmask & aFeature);    
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::UiLocalFeatureSupported
// ----------------------------------------------------------------------------
//
TBool CBrowserPreferences::UiLocalFeatureSupported( const TInt aFeature ) const
    {
LOG_ENTERFN("CBrowserPreferences::UiLocalFeatureSupported");
BROWSER_LOG( ( _L( "UiLocalFeatureSupported returns %d" ),
        (iUiLocalFeaturesBitmask & aFeature) ) );
    return (iUiLocalFeaturesBitmask & aFeature);
    }




// ----------------------------------------------------------------------------
// CBrowserPreferences::NewLC
// ----------------------------------------------------------------------------
//
CBrowserPreferences* CBrowserPreferences::NewLC
    ( MCommsModel& aCommsModel, MApiProvider& aApiProvider , TBrowserOverriddenSettings* aSettings)
	{
LOG_ENTERFN("CBrowserPreferences::NewLC");
	CBrowserPreferences* result;
	result = new ( ELeave ) CBrowserPreferences( aCommsModel, aApiProvider );
	CleanupStack::PushL( result );
	result->ConstructL( aSettings );	
	return result;
	}

// ----------------------------------------------------------------------------
// CBrowserPreferences::NewL
// ----------------------------------------------------------------------------
//
CBrowserPreferences* CBrowserPreferences::NewL
	( MCommsModel& aCommsModel, MApiProvider& aApiProvider, TBrowserOverriddenSettings* aSettings )
	{
LOG_ENTERFN("CBrowserPreferences::NewL");
	CBrowserPreferences* result;
	result = CBrowserPreferences::NewLC( aCommsModel, aApiProvider, aSettings );
	CleanupStack::Pop();
	return result;	
	}

// ----------------------------------------------------------------------------
// CBrowserPreferences::CBrowserPreferences
// ----------------------------------------------------------------------------
//
CBrowserPreferences::CBrowserPreferences( MCommsModel& aCommsModel,
                                            MApiProvider& aApiProvider) : 
    iApiProvider( aApiProvider ),
    iCommsModel( aCommsModel ),
    iEmbeddedMode( EFalse )
    {
    LOG_ENTERFN("CBrowserPreferences::CBrowserPreferences");
    iAllPreferences.iCookies = EWmlSettingsCookieAllow;
    iAllPreferences.iHttpSecurityWarnings = ETrue;
    iAllPreferences.iDownloadsOpen = ETrue;    
    iAllPreferences.iConnDialogs = ETrue;
    iAllPreferences.iHomePgURL = NULL;
    iAllPreferences.iSearchPgURL = NULL;
    iAllPreferences.iQueryOnExit = EFalse;
    iAllPreferences.iSendReferrer = EFalse;
    iAllPreferences.iAssocVpn = KWmlNoDefaultAccessPoint;
    iAllPreferences.iDefaultAccessPoint = KWmlNoDefaultAccessPoint;
    iAllPreferences.iDefaultSnapId = KWmlNoDefaultSnapId;
    iAllPreferences.iDefaultAPDetails = NULL;
    iAllPreferences.iShortCutFuncHomePg = NULL;
    iAllPreferences.iShortCutFuncBkMark = NULL;
    iAllPreferences.iShortCutFuncFindKeyWord = NULL;
    iAllPreferences.iShortCutFuncPrePage = NULL;
    iAllPreferences.iShortCutFuncSwitchWin = NULL;
    iAllPreferences.iShortCutFuncMiniImage = NULL;
    iAllPreferences.iShortCutFuncFullScreen = NULL;
    iAllPreferences.iShortCutFuncGoAddr = NULL;
    iAllPreferences.iShortCutFuncZoomIn = NULL;
    iAllPreferences.iShortCutFuncZoomOut = NULL;
    iAllPreferences.iShortCutFuncZoomMode = NULL;
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::~CBrowserPreferences
// ----------------------------------------------------------------------------
//
CBrowserPreferences::~CBrowserPreferences()
    {
    LOG_ENTERFN("CBrowserPreferences::~CBrowserPreferences");
    delete iObservers;
    delete iApDH;
    delete iVpnEngine;
    delete iVpnItem;
    iCommsModel.ApDb().RemoveObserver( this );
    delete iSelfDownloadContentTypes;
    iSelfDownloadContentTypes = 0;

    // All Settings struct - for observers
    delete iAllPreferences.iSearchPgURL;
    iAllPreferences.iSearchPgURL = NULL;
    delete iAllPreferences.iHomePgURL;
    iAllPreferences.iHomePgURL = NULL;
	delete iAllPreferences.iDefaultAPDetails;
    delete iRepository;

    // For short cut functions
    delete iAllPreferences.iShortCutFuncHomePg;
    iAllPreferences.iShortCutFuncHomePg = NULL; 

    delete iAllPreferences.iShortCutFuncBkMark;
    iAllPreferences.iShortCutFuncBkMark = NULL; 

    delete iAllPreferences.iShortCutFuncFindKeyWord;
    iAllPreferences.iShortCutFuncFindKeyWord = NULL; 

    delete iAllPreferences.iShortCutFuncPrePage;
    iAllPreferences.iShortCutFuncPrePage = NULL; 

    delete iAllPreferences.iShortCutFuncSwitchWin;
    iAllPreferences.iShortCutFuncSwitchWin = NULL; 

    delete iAllPreferences.iShortCutFuncMiniImage;
    iAllPreferences.iShortCutFuncMiniImage = NULL; 

    delete iAllPreferences.iShortCutFuncFullScreen;
    iAllPreferences.iShortCutFuncFullScreen = NULL; 

    delete iAllPreferences.iShortCutFuncGoAddr;
    iAllPreferences.iShortCutFuncGoAddr = NULL; 

    delete iAllPreferences.iShortCutFuncZoomIn;
    iAllPreferences.iShortCutFuncZoomIn = NULL; 

    delete iAllPreferences.iShortCutFuncZoomOut;
    iAllPreferences.iShortCutFuncZoomOut = NULL; 

    delete iAllPreferences.iShortCutFuncZoomMode;
    iAllPreferences.iShortCutFuncZoomMode = NULL; 

    delete iAllPreferences.iUrlSuffixList;  
    iAllPreferences.iUrlSuffixList = NULL;  
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::AddObserverL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::AddObserverL( MPreferencesObserver* anObserver )
    {
    LOG_ENTERFN("CBrowserPreferences::AddObserverL");
    iObservers->AppendL( anObserver );
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::RemoveObserver
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::RemoveObserver( MPreferencesObserver* anObserver )
    {
    LOG_ENTERFN("CBrowserPreferences::RemoveObserver");
    TInt i( 0 );
    TInt count = iObservers->Count();
    for ( i = 0; i < count; i++ )
        {
        if ( iObservers->At( i ) == anObserver )
            {
            iObservers->Delete( i );
            break; 
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::NotifyObserversL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::NotifyObserversL( TPreferencesEvent aEvent,
        TBrCtlDefs::TBrCtlSettings aSettingType = TBrCtlDefs::ESettingsUnknown )
    {
    LOG_ENTERFN("CBrowserPreferences::NotifyObserversL");
    TInt i;
    TInt count = iObservers->Count();
    
    // Passed struct of all preferences to observers
    for ( i = 0; i < count; i++ )
        {
        iObservers->At( i )->HandlePreferencesChangeL( 
                                                    aEvent, 
                                                    iAllPreferences,
                                                    aSettingType );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::RestoreSettingsL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::RestoreSettingsL()
    {
    LOG_ENTERFN("CBrowserPreferences::RestoreSettingsL");
//
//  READ USER VARIATED SETTINGS
//
    
	//Read auto load content setting
    iAllPreferences.iAutoLoadContent = GetIntValue( KBrowserNGImagesEnabled );
    
    //Read font size
    iAllPreferences.iFontSize = GetIntValue( KBrowserNGFontSize );

	// Read Allow Cookies setting
    if ( GetIntValue( KBrowserNGCookiesEnabled ) )
        {    
        iAllPreferences.iCookies = EWmlSettingsCookieAllow;
        }
    else
        {
        iAllPreferences.iCookies = EWmlSettingsCookieReject;
        }
        
    // Read ECMA Setting
    if ( GetIntValue( KBrowserNGECMAScriptSupport ) )
        {
        iAllPreferences.iEcma = EWmlSettingsECMAEnable;
        }
    else
        {
        iAllPreferences.iEcma = EWmlSettingsECMADisable;
        }

    // Read IMEI Setting
    if ( IMEI_NOTIFICATION )
        {
        if ( GetIntValue( KBrowserIMEINotification ) )
            {
            iAllPreferences.iIMEINotification = EWmlSettingsIMEIEnable;
            }
        else
            {
            iAllPreferences.iIMEINotification = EWmlSettingsIMEIDisable;
            }
        }
    
    TInt encoding;
	iRepository->Get( KBrowserNGEncoding, encoding );
    iAllPreferences.iEncoding = ( TUint32 ) encoding;
    
    iAllPreferences.iScriptLog = GetIntValue( KBrowserNGScriptLog ) ;

    //Read Fullscreen setting
    TInt value;
    value = GetIntValue ( KBrowserNGFullScreen );

    switch ( value )
        {
        case KBrowserFullScreenSettingDataSoftkeysOff:
        	{
            iAllPreferences.iFullScreen = EWmlSettingsFullScreenFullScreen;
            break;
        	}
  
        case KBrowserFullScreenSettingDataSoftkeysOn:
            {
            iAllPreferences.iFullScreen = EWmlSettingsFullScreenSoftkeysOnly;
            break;
            }
 
        default:
            {
            break;
            }
        }

	  iAllPreferences.iSendReferrer = (TBool) GetIntValue( KBrowserSendReferrerHeader );


    //Read adaptive bookmarks setting
    if (ADAPTIVEBOOKMARKS)
        {
        TInt adBm;
        adBm = GetIntValue ( KBrowserNGAdaptiveBookmarks );

        switch(adBm)
            {
            case (KBrowserAdaptiveBookmarksSettingDataOn):
                {
                iAllPreferences.iAdaptiveBookmarks = 
                                            EWmlSettingsAdaptiveBookmarksOn;
                break;
                }
            case (KBrowserAdaptiveBookmarksSettingDataHideFolder):
                {
                iAllPreferences.iAdaptiveBookmarks = 
                                    EWmlSettingsAdaptiveBookmarksHideFolder;
                break;
                }
            case (KBrowserAdaptiveBookmarksSettingDataOff):
                {
                iAllPreferences.iAdaptiveBookmarks = 
                                    EWmlSettingsAdaptiveBookmarksOff;
                break;
                }
            default:
                {
                break;
                }
            }        
        }

//  
//  READ OPERATOR VARIATED SETTINGS
//

	iAllPreferences.iHomePgType = 
                ( TWmlSettingsHomePage ) GetIntValue( KBrowserNGHomepageType );
    iAllPreferences.iPageOverview = GetIntValue ( 
		                                    KBrowserNGPageOverview );
	iAllPreferences.iBackList = GetIntValue ( 
		                                    KBrowserNGBackList );
	iAllPreferences.iAutoRefresh = GetIntValue ( 
		                                    KBrowserNGAutoRefresh );	
    
    if(PROGRESSIVE_DOWNLOAD)
	    {
    	iAllPreferences.iDownloadsOpen = GetIntValue ( KBrowserNGOpenDownloads );
    	}
    else
    	{
    	iAllPreferences.iDownloadsOpen = EFalse;
    	}
		                                    
    //Read suppress security UI setting
    iAllPreferences.iHTTPSecuritySupressed = GetIntValue ( KBrowserSecurityUI );

    //Read show connection queries setting
    iAllPreferences.iConnDialogs = GetIntValue( KBrowserConnectionDialogs );
    
    if( !iAllPreferences.iHTTPSecuritySupressed )
        {
        // Read HTTP security warnings setting
	    iAllPreferences.iHttpSecurityWarnings = GetIntValue ( 
		                            KBrowserNGShowSecurityWarnings );
        }
    else 
        {
        //we don't want to see sec warning because they're supressed
        iAllPreferences.iHttpSecurityWarnings = EFalse;
        }    
    
    // Media Volume uses different SD ini. Revert to Browser SD ini after use
    iAllPreferences.iMediaVolume = GetIntValue( KBrowserMediaVolumeControl );
    
    // Pop up Blocking
    iAllPreferences.iPopupBlocking = GetIntValue( KBrowserNGPopupBlocking );

    // Form Data Saving
    iAllPreferences.iFormDataSaving = (TWmlSettingsFormData) GetIntValue( KBrowserFormDataSaving );

    // Access Keys
    iAllPreferences.iAccessKeys = (TBool) GetIntValue( KBrowserNGAccessKeys );
    
    // The leaving functions come at the end

    // Search Page
    if ( iAllPreferences.iSearchPgURL )
        {
        delete iAllPreferences.iSearchPgURL;
        iAllPreferences.iSearchPgURL = NULL;
        }
        
    // Web reed feeds settings - begin
    iAllPreferences.iAutomaticUpdatingAP = GetIntValue( KBrowserNGAutomaticUpdatingAccessPoint );
    
    iAllPreferences.iAutomaticUpdatingWhileRoaming = GetIntValue( KBrowserNGAutomaticUpdatingWhileRoaming );
    
    // Web reed feeds settings - end
    if (CBrowserAppUi::Static()->OrientationCanBeChanged() )
        {
        iAllPreferences.iRotateDisplay = GetIntValue( KBrowserNGRotateDisplay );
        }
    else
        {
        iAllPreferences.iRotateDisplay = 0;
        }
      if ( iAllPreferences.iUrlSuffixList )
        {
        delete iAllPreferences.iUrlSuffixList;
        iAllPreferences.iUrlSuffixList = NULL;
        }
    iAllPreferences.iUrlSuffixList = HBufC::NewL( KUrlSuffixMaxLength );
    TPtr suffix = iAllPreferences.iUrlSuffixList->Des();
    GetStringValueL( KBrowserUrlSuffix, KBrowserUrlSuffix, suffix );

    // If something fails for an option, the default value will be used
    TInt ap;

    // Read Accesss point selection mode for advanced settings
    const TInt selectionMode = GetIntValue( KBrowserAccessPointSelectionMode );
    switch ( selectionMode )
        {
        case EBrowserCenRepApSelModeUserDefined: 
             {
             iAllPreferences.iAccessPointSelectionMode = EConnectionMethod; 
             break; 
             }
        case EBrowserCenRepApSelModeAlwaysAsk: 
             {
             iAllPreferences.iAccessPointSelectionMode = EAlwaysAsk; 
             break; 
             }
        case EBrowserCenRepApSelModeDestination:
             {
             iAllPreferences.iAccessPointSelectionMode = EDestination; 
             break; 
             }
        default: // Default to Always ask.
             {
             iAllPreferences.iAccessPointSelectionMode = EAlwaysAsk; 
             break; 
             }
        }
 
    //we differentiate by connecting with a Snap or an access point                         
   if (iAllPreferences.iAccessPointSelectionMode == EConnectionMethod)
   	    {   		
        //Read default AP setting
        ap = GetIntValue( KBrowserDefaultAccessPoint );

        SetDefaultAccessPointL( ap );   
		}
    else if (iAllPreferences.iAccessPointSelectionMode == EDestination)
		{
   		iAllPreferences.iDefaultSnapId = GetIntValue( KBrowserNGDefaultSnapId );
		}
	
    // For Short Cut keys
    if ( iAllPreferences.iShortCutFuncHomePg )
        {
        delete iAllPreferences.iShortCutFuncHomePg;
        iAllPreferences.iShortCutFuncHomePg = NULL;
        }
    iAllPreferences.iShortCutFuncHomePg = HBufC::NewL( KShortCutFuncStringLength );
    TPtr func1 = iAllPreferences.iShortCutFuncHomePg->Des();
    GetStringValueL( KBrowserNGShortCutFuncHomePg, KShortCutFuncStringLength, func1 );

    if ( iAllPreferences.iShortCutFuncBkMark )
        {
        delete iAllPreferences.iShortCutFuncBkMark;
        iAllPreferences.iShortCutFuncBkMark = NULL;
        }
    iAllPreferences.iShortCutFuncBkMark = HBufC::NewL( KShortCutFuncStringLength );
    TPtr func2 = iAllPreferences.iShortCutFuncBkMark->Des();
    GetStringValueL( KBrowserNGShortCutFuncBkMark, KShortCutFuncStringLength, func2 );

    if ( iAllPreferences.iShortCutFuncFindKeyWord )
        {
        delete iAllPreferences.iShortCutFuncFindKeyWord;
        iAllPreferences.iShortCutFuncFindKeyWord = NULL;
        }
    iAllPreferences.iShortCutFuncFindKeyWord = HBufC::NewL( KShortCutFuncStringLength );
    TPtr func3 = iAllPreferences.iShortCutFuncFindKeyWord->Des();
    GetStringValueL( KBrowserNGShortCutFuncFindKeyWord, KShortCutFuncStringLength, func3 );

    if ( iAllPreferences.iShortCutFuncPrePage )
        {
        delete iAllPreferences.iShortCutFuncPrePage;
        iAllPreferences.iShortCutFuncPrePage = NULL;
        }
    iAllPreferences.iShortCutFuncPrePage = HBufC::NewL( KShortCutFuncStringLength );
    TPtr func4 = iAllPreferences.iShortCutFuncPrePage->Des();
    GetStringValueL( KBrowserNGShortCutFuncPrePage, KShortCutFuncStringLength, func4 );

    if ( iAllPreferences.iShortCutFuncSwitchWin )
        {
        delete iAllPreferences.iShortCutFuncSwitchWin;
        iAllPreferences.iShortCutFuncSwitchWin = NULL;
        }
    iAllPreferences.iShortCutFuncSwitchWin = HBufC::NewL( KShortCutFuncStringLength );
    TPtr func5 = iAllPreferences.iShortCutFuncSwitchWin->Des();
    GetStringValueL( KBrowserNGShortCutFuncSwitchWin, KShortCutFuncStringLength, func5 );

    if ( iAllPreferences.iShortCutFuncMiniImage )
        {
        delete iAllPreferences.iShortCutFuncMiniImage;
        iAllPreferences.iShortCutFuncMiniImage = NULL;
        }
    iAllPreferences.iShortCutFuncMiniImage = HBufC::NewL( KShortCutFuncStringLength );
    TPtr func6 = iAllPreferences.iShortCutFuncMiniImage->Des();
    GetStringValueL( KBrowserNGShortCutFuncMiniImage, KShortCutFuncStringLength, func6 );

    if ( iAllPreferences.iShortCutFuncGoAddr )
        {
        delete iAllPreferences.iShortCutFuncGoAddr;
        iAllPreferences.iShortCutFuncGoAddr = NULL;
        }
    iAllPreferences.iShortCutFuncGoAddr = HBufC::NewL( KShortCutFuncStringLength );
    TPtr func7 = iAllPreferences.iShortCutFuncGoAddr->Des();
    GetStringValueL( KBrowserNGShortCutFuncGoAddr, KShortCutFuncStringLength, func7 );

    if ( iAllPreferences.iShortCutFuncZoomIn )
        {
        delete iAllPreferences.iShortCutFuncZoomIn;
        iAllPreferences.iShortCutFuncZoomIn = NULL;
        }
    iAllPreferences.iShortCutFuncZoomIn = HBufC::NewL( KShortCutFuncStringLength );
    TPtr func8 = iAllPreferences.iShortCutFuncZoomIn->Des();
    GetStringValueL( KBrowserNGShortCutFuncZoomIn, KShortCutFuncStringLength, func8 );

    if ( iAllPreferences.iShortCutFuncZoomOut )
        {
        delete iAllPreferences.iShortCutFuncZoomOut;
        iAllPreferences.iShortCutFuncZoomOut = NULL;
        }
    iAllPreferences.iShortCutFuncZoomOut = HBufC::NewL( KShortCutFuncStringLength );
    TPtr func9 = iAllPreferences.iShortCutFuncZoomOut->Des();
    GetStringValueL( KBrowserNGShortCutFuncZoomOut, KShortCutFuncStringLength, func9 );

/*
    if ( iAllPreferences.iShortCutFuncZoomMode )
        {
        delete iAllPreferences.iShortCutFuncZoomMode;
        iAllPreferences.iShortCutFuncZoomMode = NULL;
        }
    iAllPreferences.iShortCutFuncZoomMode = HBufC::NewL( KShortCutFuncStringLength );
    TPtr func11 = iAllPreferences.iShortCutFuncZoomMode->Des();
    GetStringValueL( KBrowserNGShortCutFuncZoomMode, KShortCutFuncStringLength, func11 );
*/

    if ( iAllPreferences.iShortCutFuncFullScreen )
        {
        delete iAllPreferences.iShortCutFuncFullScreen;
        iAllPreferences.iShortCutFuncFullScreen = NULL;
        }
    iAllPreferences.iShortCutFuncFullScreen = HBufC::NewL( KShortCutFuncStringLength );
    TPtr func10 = iAllPreferences.iShortCutFuncFullScreen->Des();
    GetStringValueL( KBrowserNGShortCutFuncFullScreen, KShortCutFuncStringLength, func10 );

    // For Configuring Toolbar Buttons
    iAllPreferences.iToolbarOnOff       = GetIntValue( KToolbarOnOff );
    
    iAllPreferences.iToolbarButton1Cmd  = GetIntValue( KToolbarButton1Cmd );
    iAllPreferences.iToolbarButton2Cmd  = GetIntValue( KToolbarButton2Cmd );
    iAllPreferences.iToolbarButton3Cmd  = GetIntValue( KToolbarButton3Cmd );
    iAllPreferences.iToolbarButton4Cmd  = GetIntValue( KToolbarButton4Cmd );
    iAllPreferences.iToolbarButton5Cmd  = GetIntValue( KToolbarButton5Cmd );
    iAllPreferences.iToolbarButton6Cmd  = GetIntValue( KToolbarButton6Cmd );
    iAllPreferences.iToolbarButton7Cmd  = GetIntValue( KToolbarButton7Cmd );
    
    // For configuring Shortcut Keys 
    iAllPreferences.iShortcutKeysForQwerty  = GetIntValue( KShortcutKeysForQwerty );
	
    iAllPreferences.iShortcutKey1Cmd  = GetIntValue( KShortcutKey1Cmd );
    iAllPreferences.iShortcutKey2Cmd  = GetIntValue( KShortcutKey2Cmd );
    iAllPreferences.iShortcutKey3Cmd  = GetIntValue( KShortcutKey3Cmd );
    iAllPreferences.iShortcutKey4Cmd  = GetIntValue( KShortcutKey4Cmd );
    iAllPreferences.iShortcutKey5Cmd  = GetIntValue( KShortcutKey5Cmd );
    iAllPreferences.iShortcutKey6Cmd  = GetIntValue( KShortcutKey6Cmd );
    iAllPreferences.iShortcutKey7Cmd  = GetIntValue( KShortcutKey7Cmd );
    iAllPreferences.iShortcutKey8Cmd  = GetIntValue( KShortcutKey8Cmd );
    iAllPreferences.iShortcutKey9Cmd  = GetIntValue( KShortcutKey9Cmd );
    iAllPreferences.iShortcutKey0Cmd  = GetIntValue( KShortcutKey0Cmd );
    iAllPreferences.iShortcutKeyStarCmd  = GetIntValue( KShortcutKeyStarCmd );
    iAllPreferences.iShortcutKeyHashCmd  = GetIntValue( KShortcutKeyHashCmd );


    iAllPreferences.iZoomLevelMin = GetIntValue( KBrowserNGZoomMin );
    iAllPreferences.iZoomLevelMax = GetIntValue( KBrowserNGZoomMax );
    iAllPreferences.iZoomLevelDefault = GetIntValue( KBrowserNGZoomDefault );
            
    iAllPreferences.iMaxRecentUrlsToShow = GetIntValue( KBrowserNGMaxRecentUrls );

    }


// ----------------------------------------------------------------------------
// CBrowserPreferences::AllPreferencesL
// ----------------------------------------------------------------------------
//
const TPreferencesValues& CBrowserPreferences::AllPreferencesL()
    {
    return iAllPreferences;
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::VpnDataL
// ----------------------------------------------------------------------------
//
TBool CBrowserPreferences::VpnDataL( TUint aDefaultAccessPoint, CVpnApItem& aVpnItem )
    {
    TBool ret( EFalse );
    
    if( iVpnEngine->IsVpnApL( aDefaultAccessPoint ) )
        {
        iVpnEngine->VpnDataL( aDefaultAccessPoint, aVpnItem );
        ret = ETrue;
        }
    
    return ret;
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetDefaultAccessPointL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetDefaultAccessPointL( TUint aDefaultAccessPoint, TUint aAssocVpn )
	{
    LOG_ENTERFN("CBrowserPreferences::SetDefaultAccessPointL");
	BROWSER_LOG( ( _L( " aDefaultAccessPoint: %u" ), aDefaultAccessPoint ) );

    //get the connection identifier based on the connection type
    switch ( AccessPointSelectionMode() )
        {
        //get the destination identifyer based on the snap Id
        case EDestination:
        	{
		    RCmManager        cmManager;
		    cmManager.OpenL();

            // if user-defined destination then check that destination still exists.
            // if it doesn't, reset access point to always ask and homepage to blank access point home page.
            TUint snapId = iAllPreferences.iDefaultSnapId;
            if (snapId != KWmlNoDefaultSnapId)
                {            
    			TRAPD(err, RCmDestination dest = cmManager.DestinationL( snapId ));
    			if (err != KErrNone)
    			    {
    			    aDefaultAccessPoint = KWmlNoDefaultAccessPoint;
        	    	aAssocVpn = KWmlNoDefaultAccessPoint;
        	    	
        			SetAccessPointSelectionModeL(EAlwaysAsk );
        			if (iAllPreferences.iHomePgType == EWmlSettingsHomePageAccessPoint)
        				{
        				HBufC* buf = HBufC::NewLC( KMaxHomePgUrlLength );  // cleanupstack
            			TPtr ptr( buf->Des() );
            			// read user defined home page from SD shouldn't get any error
                        HomePageUrlL( ptr, ETrue );
                        SetHomePageUrlL(ptr);
            			CleanupStack::PopAndDestroy( buf );
        				}
    			    }
                }
 		    cmManager.Close();

			break;
        	}
        	
        //if connecting with Iap Id
        case EConnectionMethod:
            {
            // if user-defined access point then check that access point still exists.
            // if it doesn't, reset access point to always ask and homepage to blank access point home page.
  	        CApListItem* apItem = iCommsModel.AccessPointsL()->ItemForUid( aDefaultAccessPoint );
	        if (apItem == NULL)
		        {
    	    	aDefaultAccessPoint = KWmlNoDefaultAccessPoint;
    	    	aAssocVpn = KWmlNoDefaultAccessPoint;
    	    	delete iAllPreferences.iDefaultAPDetails;
    			iAllPreferences.iDefaultAPDetails = NULL;
    			SetAccessPointSelectionModeL(EAlwaysAsk );
    			if (iAllPreferences.iHomePgType == EWmlSettingsHomePageAccessPoint)
    				{
    				HBufC* buf = HBufC::NewLC( KMaxHomePgUrlLength );  // cleanupstack
        			TPtr ptr( buf->Des() );
        			// read user defined home page from SD shouldn't get any error
                    HomePageUrlL( ptr, ETrue );
                    SetHomePageUrlL(ptr);
        			CleanupStack::PopAndDestroy( buf );
    				}
		    	}
            
		    break;	
        	}

        default:
        	{
        	break;	
        	}
        }
    
    	
	// check web feeds for deleted access point
	CApListItem* apItem = iCommsModel.AccessPointsL()->ItemForUid( iAllPreferences.iAutomaticUpdatingAP );
	if (apItem == NULL)
		{
        SetAutomaticUpdatingApL( KWmlNoDefaultAccessPoint );
		}
    bool checkAPMode = ( (iAllPreferences.iAccessPointSelectionMode != EDestination) && (iAllPreferences.iAccessPointSelectionMode != EAlwaysAsk) );
    if( ( KWmlNoDefaultAccessPoint != aDefaultAccessPoint ) && ( checkAPMode ) && iVpnEngine->IsVpnApL( aDefaultAccessPoint ) )
        {
        delete iVpnItem;
        iVpnItem = NULL;

        iVpnItem = CVpnApItem::NewLC();
        CleanupStack::Pop();

        iVpnEngine->VpnDataL( aDefaultAccessPoint, *iVpnItem );

        TUint32 ass( aDefaultAccessPoint );
        iVpnItem->ReadUint( EApVpnWapAccessPointID, ass );
        aAssocVpn = ass;            

        // get real WAP id
        TUint32 ap( aDefaultAccessPoint );
        iVpnItem->ReadUint( EApVpnRealWapID, ap );
        aDefaultAccessPoint = ap;
        }    
    BROWSER_LOG( ( _L( "VPN OK" ) ) );
	iAllPreferences.iDefaultAccessPoint = aDefaultAccessPoint;
	iAllPreferences.iAssocVpn = aAssocVpn;
	
	CApAccessPointItem* api = CApAccessPointItem::NewLC();//maybe we can optimise this too	    
	BROWSER_LOG( ( _L( "CApAccessPointItem OK" ) ) );
    TInt err;
    if  ( iOverridenSettings )
        {
        iCustomAp = iOverridenSettings->GetBrowserSetting( 
                                                EBrowserOverSettingsCustomAp );
        }
    if ( ( iOverridenSettings ) &&  iCustomAp  )
        {
	    TRAP( err, iApDH->AccessPointDataL( iCustomAp, *api ) );
        if ( err != KErrNone )
            {
            iCustomAp = 0;
	        TRAP( err, iApDH->AccessPointDataL( iAllPreferences.iDefaultAccessPoint, *api ) );
            }
        }
    else
        {
	    TRAP( err, iApDH->AccessPointDataL( 
	                            iAllPreferences.iDefaultAccessPoint, *api ) );	        
        }
    BROWSER_LOG( ( _L( " AccessPointDataL: %d" ), err ) );
	//Reset default AP pointer and delete the data
	delete iAllPreferences.iDefaultAPDetails;
	iAllPreferences.iDefaultAPDetails = NULL;
	if  ( err != KErrNone )//Let's select the first 
		{
#ifndef __WINSCW__ //we will not select on Wins (defaultap will be assigned a uid indicating that there is 
					//no default ap) but rather let ConnMan show the Conn Dlg
		// The first valid access point has to be selected if exists
		LOG_WRITE("WE SHOULD NOT BE HERE!!!");
		CApSelect* apSelect = CApSelect::NewLC
			(
			iCommsModel.CommsDb(),
			KEApIspTypeAll, //KEApIspTypeWAPMandatory, 
			EApBearerTypeAll,
			KEApSortNameAscending,
			EIPv4 | EIPv6
			);
		if ( apSelect->MoveToFirst() )
			{
			iAllPreferences.iDefaultAccessPoint = apSelect->Uid();
			iApDH->AccessPointDataL( iAllPreferences.iDefaultAccessPoint, *api );
			iAllPreferences.iDefaultAPDetails = api;//save the ap details
			}
		else
			{
			iAllPreferences.iDefaultAccessPoint = KWmlNoDefaultAccessPoint;
			iAllPreferences.iDefaultAPDetails = NULL;
			delete api;
			}
		CleanupStack::PopAndDestroy(); // apSelect
		CleanupStack::Pop();//api
#else
		iAllPreferences.iDefaultAccessPoint = KWmlNoDefaultAccessPoint;
		iAllPreferences.iDefaultAPDetails = NULL;
		CleanupStack::PopAndDestroy( api );		
#endif//WINSCW	
		}
	else
		{
		iAllPreferences.iDefaultAPDetails = api;//store new default accesspoint details
		CleanupStack::Pop( ); // api			
		}
	/*
	* If VPN set VPN AP ID to default access point!
	*/
	if( KWmlNoDefaultAccessPoint != iAllPreferences.iAssocVpn )
	    {
	    iAllPreferences.iDefaultAccessPoint = iAllPreferences.iAssocVpn;
	    }
    if( !iEmbeddedMode )
        {
	    //Store ini value / gateway 
        SetIntValueL ( KBrowserDefaultAccessPoint, 
                                        iAllPreferences.iDefaultAccessPoint );
        }
    NotifyObserversL( EPreferencesItemChange );
	    
	//  LOG_LEAVEFN("CBrowserPreferences::SetDefaultAccessPointL");
	}
	
	
// ----------------------------------------------------------------------------
// CBrowserPreferences::SetDefaultSnapId: DestNetChange
// ----------------------------------------------------------------------------
//
 void CBrowserPreferences::SetDefaultSnapId (TUint aSnapId)
		{
	
		//storing the value of the default snap ID
		iAllPreferences.iDefaultSnapId = aSnapId;
		                                        
        SetIntValueL ( KBrowserNGDefaultSnapId, 
                                        iAllPreferences.iDefaultSnapId );
	
		}


// ----------------------------------------------------------------------------
// CBrowserPreferences::SetAutoLoadContentL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetAutoLoadContentL( TInt aAutoLoadContent )
    {
    LOG_ENTERFN("CBrowserPreferences::SetAutoLoadContentL");
    if ( aAutoLoadContent != iAllPreferences.iAutoLoadContent )
        {
        iAllPreferences.iAutoLoadContent = aAutoLoadContent;

        // Store value in Shared Data and forward new setting to BrCtl
        if( !iEmbeddedMode )
            {
            SetIntValueL( KBrowserNGImagesEnabled, 
                                    iAllPreferences.iAutoLoadContent );
            }
        NotifyObserversL( EPreferencesItemChange, 
                                        TBrCtlDefs::ESettingsAutoLoadImages );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetPageOverviewL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetPageOverviewL( TBool aPageOverview )
    {
    LOG_ENTERFN("CBrowserPreferences::SetPageOverviewL");
    if ( aPageOverview != iAllPreferences.iPageOverview )
        {
        iAllPreferences.iPageOverview = aPageOverview;

        // Store value in Shared Data and forward new setting to BrCtl
        if( !iEmbeddedMode )
            {
            SetIntValueL( KBrowserNGPageOverview, 
                                        iAllPreferences.iPageOverview );
            }
        NotifyObserversL( EPreferencesItemChange,
                            TBrCtlDefs::ESettingsPageOverview);
        }
    }
    
// ----------------------------------------------------------------------------
// CBrowserPreferences::SetBackListL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetBackListL( TBool aBackList )
    {
    LOG_ENTERFN("CBrowserPreferences::SetBackListL");
    if ( aBackList != iAllPreferences.iBackList )
        {
        iAllPreferences.iBackList = aBackList;

        // Store value in Shared Data and forward new setting to BrCtl
        if( !iEmbeddedMode )
            {
            SetIntValueL( KBrowserNGBackList, 
                                        iAllPreferences.iBackList );
            }
        NotifyObserversL( EPreferencesItemChange,
                            TBrCtlDefs::ESettingsBackList);
        }
    }
    
// ----------------------------------------------------------------------------
// CBrowserPreferences::SetAutoRefreshL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetAutoRefreshL( TBool aAutoRefresh )
    {
    LOG_ENTERFN("CBrowserPreferences::SetAutoRefreshL");
    if ( aAutoRefresh != iAllPreferences.iAutoRefresh )
        {
        iAllPreferences.iAutoRefresh = aAutoRefresh;

        // Store value in Shared Data and forward new setting to BrCtl
        if( !iEmbeddedMode )
            {
            SetIntValueL( KBrowserNGAutoRefresh, 
                                        iAllPreferences.iAutoRefresh );
            }
        NotifyObserversL( EPreferencesItemChange,
                            TBrCtlDefs::ESettingsAutoRefresh);
        }
    }    

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetTextWrapL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetTextWrapL( TBool aTextWrap )
    {
    LOG_ENTERFN("CBrowserPreferences::SetTextWrapL");
    if ( aTextWrap != iAllPreferences.iTextWrap )
        {
        iAllPreferences.iTextWrap = aTextWrap;

        // Store value in Shared Data and forward new setting to BrCtl
        if ( !iEmbeddedMode )
            {
            SetIntValueL( KBrowserTextWrapEnabled, iAllPreferences.iTextWrap );
            }            
        NotifyObserversL( EPreferencesItemChange, 
                                        TBrCtlDefs::ESettingsTextWrapEnabled );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetFontSizeL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetFontSizeL( TInt aFontSize )
    {
    LOG_ENTERFN("CBrowserPreferences::SetFontSizeL");
    if ( aFontSize != iAllPreferences.iFontSize )
        {
        iAllPreferences.iFontSize = aFontSize;

        // Store value in Shared Data and forward new setting to BrCtl
        if( !iEmbeddedMode )
            {
            SetIntValueL( KBrowserNGFontSize, iAllPreferences.iFontSize );
            }
        NotifyObserversL( EPreferencesItemChange, 
                                            TBrCtlDefs::ESettingsFontSize );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetEncodingL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetEncodingL( TUint32 aEncoding )
    {
    LOG_ENTERFN("CBrowserPreferences::SetEncodingL");
    if ( aEncoding != iAllPreferences.iEncoding )
        {
        iAllPreferences.iEncoding = aEncoding;
    
        // Store value in Shared Data and forward new setting to BrCtl
        if( !iEmbeddedMode )
            {
            SetIntValueL ( KBrowserNGEncoding, 
                            ( TInt ) iAllPreferences.iEncoding );
            }
        NotifyObserversL( EPreferencesItemChange, 
                                        TBrCtlDefs::ESettingsCharacterset );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetScriptLogL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetScriptLogL( TUint32 aScriptLog )
    {
    LOG_ENTERFN("CBrowserPreferences::SetScriptLogL");
    if ( aScriptLog != iAllPreferences.iScriptLog )
        {
        iAllPreferences.iScriptLog = aScriptLog;
    
        // Store value in Shared Data and forward new setting to BrCtl
        if( !iEmbeddedMode )
            {
            TBool ret = SetIntValueL ( KBrowserNGScriptLog, 
                            ( TInt ) iAllPreferences.iScriptLog );
            }
        NotifyObserversL( EPreferencesItemChange, 
                                        TBrCtlDefs::ESettingsScriptLog );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetAdaptiveBookmarksL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetAdaptiveBookmarksL( 
                            TWmlSettingsAdaptiveBookmarks aAdaptiveBookmarks )
    {
    LOG_ENTERFN( "CBrowserPreferences::SetAdaptiveBookmarksL" );
    
    if ( ADAPTIVEBOOKMARKS )
        {
        if ( aAdaptiveBookmarks != iAllPreferences.iAdaptiveBookmarks )
            {
            iAllPreferences.iAdaptiveBookmarks = aAdaptiveBookmarks;
            
            if( !iEmbeddedMode )
                {

            switch( iAllPreferences.iAdaptiveBookmarks )
                {
				case (EWmlSettingsAdaptiveBookmarksOn):
                    {
             
       				SetIntValueL ( KBrowserNGAdaptiveBookmarks, 
       					KBrowserAdaptiveBookmarksSettingDataOn );                   
                    break;
                    }
                case (EWmlSettingsAdaptiveBookmarksHideFolder):
                    {
                    SetIntValueL ( KBrowserNGAdaptiveBookmarks, 
                    	KBrowserAdaptiveBookmarksSettingDataHideFolder );
                    break;
                    }
                case (EWmlSettingsAdaptiveBookmarksOff):
                    {
                    SetIntValueL ( KBrowserNGAdaptiveBookmarks, 
                    	KBrowserAdaptiveBookmarksSettingDataOff );
                    break;
                    }
                    default:
                        {
                        break;
                        }
                    }
                }
            NotifyObserversL( EPreferencesItemChange ); // Bookmarks View is interested
            }
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetFullScreenL
// ---------------------------------------------------------
//
void CBrowserPreferences::SetFullScreenL( TWmlSettingsFullScreen aFullScreen )
    {
    LOG_ENTERFN("CBrowserPreferences::SetFullScreenL");
  
    if ( aFullScreen != iAllPreferences.iFullScreen )
        {
        iAllPreferences.iFullScreen = aFullScreen;
        if( !iEmbeddedMode )
            {
            switch ( iAllPreferences.iFullScreen )
                {
				case EWmlSettingsFullScreenSoftkeysOnly:
                    {
                    SetIntValueL ( KBrowserNGFullScreen, 
                    	KBrowserFullScreenSettingDataSoftkeysOn );
                    break;
                    }
                case EWmlSettingsFullScreenFullScreen:
                    {
                    SetIntValueL ( KBrowserNGFullScreen, 
                    	KBrowserFullScreenSettingDataSoftkeysOff );
                    break;
                    }
                default:
                    {
                    break;
                    }    
                }
            }
        NotifyObserversL( EPreferencesItemChange ); // ContentView is interested
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetCookiesL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetCookiesL( TWmlSettingsCookies aCookies )
    {
    LOG_ENTERFN("CBrowserPreferences::SetCookiesL");
    if ( aCookies != iAllPreferences.iCookies )
        {
        iAllPreferences.iCookies = aCookies;

        // Store value in Shared Data and forward new setting to BrCtl
        if( !iEmbeddedMode )
            {
            SetIntValueL( KBrowserNGCookiesEnabled, 
                                            iAllPreferences.iCookies );
            }
        NotifyObserversL( EPreferencesItemChange, 
                                        TBrCtlDefs::ESettingsCookiesEnabled );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetEcmaL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetEcmaL( TWmlSettingsECMA aEcma )
    {
    LOG_ENTERFN("CBrowserPreferences::SetEcmaL");
    if ( aEcma != iAllPreferences.iEcma )
        {
        iAllPreferences.iEcma = aEcma;

        // Store value in Shared Data and forward new setting to BrCtl
        if( !iEmbeddedMode )
            {
			SetIntValueL( KBrowserNGECMAScriptSupport, 
			                                    iAllPreferences.iEcma );
            }
        NotifyObserversL( EPreferencesItemChange, 
                                    TBrCtlDefs::ESettingsECMAScriptEnabled );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetDownloadsOpen
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetDownloadsOpenL( TBool aOpen )
    {
    LOG_ENTERFN("CBrowserPreferences::SetDownloadsOpenL");
    if ( aOpen != iAllPreferences.iDownloadsOpen )
        {
        iAllPreferences.iDownloadsOpen = aOpen;
        if( !iEmbeddedMode )
            {
			SetIntValueL ( KBrowserNGOpenDownloads, 
				        (TInt) iAllPreferences.iDownloadsOpen );
            }
        NotifyObserversL( EPreferencesItemChange, 
                                    TBrCtlDefs::ESettingsAutoOpenDownloads );
        }
    }


// ----------------------------------------------------------------------------
// CBrowserPreferences::SetHttpSecurityWarningsL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetHttpSecurityWarningsL( TBool aWarningsOn )
    {
    LOG_ENTERFN("CBrowserPreferences::SetHttpSecurityWarningsL");
    if ( aWarningsOn != iAllPreferences.iHttpSecurityWarnings )
        {
        iAllPreferences.iHttpSecurityWarnings = aWarningsOn;
        if( !iEmbeddedMode )
            {
			SetIntValueL ( KBrowserNGShowSecurityWarnings, 
				        (TInt) iAllPreferences.iHttpSecurityWarnings );
            }
        NotifyObserversL( EPreferencesItemChange, 
                                    TBrCtlDefs::ESettingsSecurityWarnings );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetIMEINotification
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetIMEINotificationL( 
                                        TWmlSettingsIMEI aIMEINotification )
    {
    LOG_ENTERFN("CBrowserPreferences::SetIMEINotificationL");
    if ( IMEI_NOTIFICATION )
        {
        if ( aIMEINotification != iAllPreferences.iIMEINotification )
            {
            iAllPreferences.iIMEINotification = aIMEINotification;
            
            // Store value in Shared Data and forward new setting to BrCtl
            if( !iEmbeddedMode )
                {
                SetIntValueL( KBrowserIMEINotification, 
                                        iAllPreferences.iIMEINotification );
                }
            NotifyObserversL( EPreferencesItemChange, 
                                    TBrCtlDefs::ESettingsIMEINotifyEnabled );
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::HandleApDbEventL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::HandleApDbEventL( TEvent anEvent )
    {
    LOG_ENTERFN("CBrowserPreferences::HandleApDbEventL");
    if ( anEvent == EDbChanged )//maybe somebody has modified the AP (not very likely though)
        {
        SetDefaultAccessPointL( iAllPreferences.iDefaultAccessPoint );
        }    
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::HomePageUrlL
// Get the user defined home page URL from shared data file.
// ----------------------------------------------------------------------------
//
TInt CBrowserPreferences::HomePageUrlL( TDes& aUrl, TBool aForceLoadFromSD ) const
    {
LOG_ENTERFN("CBrowserPreferences::HomePageUrlL");
    TInt homePgFound( KErrNotFound );
    TBool homepage = UiLocalFeatureSupported( KBrowserUiHomePageSetting );
    TWmlSettingsHomePage pgtype = HomePageType();
BROWSER_LOG( ( _L( " pgtype: %d" ), pgtype ) );

	if (homepage)
		{
		if (aForceLoadFromSD || (pgtype != EWmlSettingsHomePageAccessPoint))
			{
			// if we are requiring a force load from the shared data, or our homepage
		    // is set to be something other than the access point's home page, get
		    // the value of the home page from the shared data
			homePgFound = GetStringValueL( KBrowserNGHomepageURL, KMaxHomePgUrlLength, aUrl );
			}
		else if ((iAllPreferences.iAccessPointSelectionMode == EConnectionMethod) && 
				 (pgtype == EWmlSettingsHomePageAccessPoint))
        {
			// The user has a defined access point, and wants to use the access point's
    		// home page
BROWSER_LOG( ( _L( "HomePageUrl, Access Point" ) ) );
        
        TUint defaultAp = DefaultAccessPoint();
        if ( defaultAp != KWmlNoDefaultAccessPoint ) // There is an access point defined
            {
            CApAccessPointItem* apItem = iAllPreferences.iDefaultAPDetails;
            if ( apItem )
                {
                const HBufC* defaultHP = apItem->ReadConstLongTextL( EApWapStartPage );
                if ( defaultHP->Length() )
                    {
                    aUrl.Zero();
                    aUrl.Append( *defaultHP );
                    homePgFound = KErrNone;
                    }
                }
            else
            	{
BROWSER_LOG( ( _L( "HomePageUrl, Read from Central Repository" ) ) );
            	homePgFound = GetStringValueL( KBrowserNGHomepageURL, KMaxHomePgUrlLength, aUrl );
            	}
            }
        }
        }
BROWSER_LOG( ( _L( "CBrowserPreferences::HomePageUrlL returns %d" ), homePgFound ) );
    return homePgFound;
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::HomePageFromIapL
// Get the home page URL from belongs to the access point.
// ----------------------------------------------------------------------------
//
TInt CBrowserPreferences::HomePageFromIapL( TDes& aUrl, TUint aIap ) const
    {
LOG_ENTERFN("CBrowserPreferences::HomePageFromIapL");
    TInt homePgFound( KErrNotFound );

    CApListItem* apItem = iCommsModel.AccessPointsL()->ItemForUid( aIap );
    if ( apItem )
        {
        TPtrC defaultHP( apItem->StartPage() );
        if ( defaultHP.Length() )
            {
            aUrl.Zero();
            aUrl.Append( defaultHP );
            homePgFound = KErrNone;
            }
        }
BROWSER_LOG( ( _L( "CBrowserPreferences::HomePageFromIapL returns %d" ), homePgFound ) );
    return homePgFound;
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SearchPageUrlL
// Get the user defined search page URL from shared data file.
// ----------------------------------------------------------------------------
//
TPtrC CBrowserPreferences::SearchPageUrlL()
    {
LOG_ENTERFN("CBrowserPreferences::SearchPageUrlL");    
    TBool urlTooBig = EFalse;
    TInt error = KErrTooBig;
    TInt length = 512;

    do
        {
        HBufC* temp = HBufC::NewL( length );
        if ( iAllPreferences.iSearchPgURL )
            {
            delete iAllPreferences.iSearchPgURL;
            iAllPreferences.iSearchPgURL = NULL;
            }

        iAllPreferences.iSearchPgURL = temp;
        TPtr ptr = iAllPreferences.iSearchPgURL->Des();
        // Here should handle the error variable, but its still not clear
        // how big can a CenRep string be.( KMaxUnicodeStringLength )
		error = iRepository->Get( KBrowserNGSearchPageURL, ptr);

        if ( error == KErrTooBig )
            {
            if ( length >= KMaxSearchPgUrlLength )
                {
                urlTooBig = ETrue;
                break;
                }
            else
                length = length * 2;
            }
        } while ( error == KErrTooBig );

    // If the URL exceeds the limit, return NULL.
    if ( urlTooBig )
        {
        delete iAllPreferences.iSearchPgURL;
        iAllPreferences.iSearchPgURL = NULL;
        iAllPreferences.iSearchPgURL = KNullDesC().AllocL();
        }

    NotifyObserversL( EPreferencesItemChange );
    
#ifdef _DEBUG
    TPtrC logString = *iAllPreferences.iSearchPgURL;
    BROWSER_LOG( ( _L( "CBrowserPreferences::SearchPageUrlL returns %S" ), &logString ) );
#endif // _DEBUG
    return iAllPreferences.iSearchPgURL->Des();
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetPopupBlockingL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetPopupBlockingL( TBool aPopupBlocking )
    {
    LOG_ENTERFN("CBrowserPreferences::SetPopupBlockingL");
    if ( aPopupBlocking != iAllPreferences.iPopupBlocking )
        {
        iAllPreferences.iPopupBlocking = aPopupBlocking;
        SetIntValueL( KBrowserNGPopupBlocking, iAllPreferences.iPopupBlocking );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetFormDataSavingL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetFormDataSavingL( TWmlSettingsFormData aFormDataSaving )
    {
    LOG_ENTERFN("CBrowserPreferences::SetFormDataSavingL");
    if ( aFormDataSaving != iAllPreferences.iFormDataSaving )
        {
        iAllPreferences.iFormDataSaving = aFormDataSaving;


        // TO DO: Add to cenrep
        SetIntValueL( KBrowserFormDataSaving, 
                        iAllPreferences.iFormDataSaving );
        }
    NotifyObserversL( EPreferencesItemChange,
                            TBrCtlDefs::ESettingsAutoFormFillEnabled );
    }    

// ----------------------------------------------------------------------------
// CBrowserPreferences::GetIntValue
// ----------------------------------------------------------------------------
//
TInt CBrowserPreferences::GetIntValue( TUint32 aKey) const
    {
LOG_ENTERFN("CBrowserPreferences::GetIntValue");
    TInt retVal = 0; 
      
	if ( iRepository )
	   	{
    	iRepository->Get(aKey, retVal);	
    	}
                
BROWSER_LOG( ( _L( "CBrowserPreferences::GetIntValue returns %d" ), retVal ) );
    return retVal;
    }
    									   
// ----------------------------------------------------------------------------
// CBrowserPreferences::SetIntValueL
// ----------------------------------------------------------------------------
//
TBool CBrowserPreferences::SetIntValueL ( TUint32 aKey, const TInt& aValue )
    {
LOG_ENTERFN("CBrowserPreferences::SetIntValueL");
BROWSER_LOG( ( _L( "aValue: %d" ), aValue ) );
    
    // Log setting value BEFORE the change
    #ifdef I__BROWSER_LOG_ENABLED
        TInt preValue;
    	iRepository->Get( aKey, preValue );
        BROWSER_LOG( ( _L( "Prechange CenRep setting value: %d" ), preValue ) );
    #endif // I__BROWSER_LOG_ENABLED
    
    TBool retVal=EFalse;
	if ( iRepository && ( KErrNone == iRepository->Set( aKey, aValue ) ) )
    	{
    	retVal = ETrue;
    	}	

    // Log setting value AFTER the change
    #ifdef I__BROWSER_LOG_ENABLED
        TInt postValue;
    	iRepository->Get( aKey, postValue );
        BROWSER_LOG( ( _L( "Postchange CenRep setting value: %d" ), postValue ) );
    #endif // I__BROWSER_LOG_ENABLED

    BROWSER_LOG( ( _L( "CBrowserPreferences::SetIntValueL returns %d" ), retVal ) );
    return retVal;
    }	

// ----------------------------------------------------------------------------
// CBrowserPreferences::GetStringValueL
// ----------------------------------------------------------------------------
//
HBufC* CBrowserPreferences::GetStringValueL ( TUint32 aKey ) const
    {
LOG_ENTERFN("CBrowserPreferences::GetStringValueL(TUint32)");
    HBufC* retVal = NULL;
    TFileName value;
    TInt err( KErrNotFound );

	
	if( iRepository )
		{
		err = iRepository->Get( aKey, value );
		}

    if ( err == KErrNone )
       	{
       	retVal = HBufC::NewL( value.Length() );
       	TPtr ptr = retVal->Des();
       	ptr.Copy ( value );
       	}
        
    #ifdef _DEBUG
        TPtrC logString = *retVal;
        BROWSER_LOG( ( _L( "CBrowserPreferences::GetStringValueL returns %S" ), &logString ) );
    #endif // _DEBUG
    
    return retVal;
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::GetStringValueL
// ----------------------------------------------------------------------------
//
TInt CBrowserPreferences::GetStringValueL( 
		TUint32 aKey, TInt aMaxBufSize, 
		TDes& aBuf) const
    {
LOG_ENTERFN("CBrowserPreferences::GetStringValueL(TUint32,TInt,TDes&)");
    // Not sure this works, but CRepository has no information about the return
    // values of the methods. 
    // So dont know if iRepository->Get returns with an errorid on error,therefore
    // error = iRepository->Get( aKey, ptr ) may not work.
    TInt length( 256 );
    HBufC* value = HBufC::NewL( length );
    TPtr ptr = value->Des();
    TInt error;
    
	error = iRepository->Get( aKey, ptr );

    for( length=length*2; (error==KErrTooBig) && (length<aMaxBufSize); length=length*2)
        {
        delete value;
        value = HBufC::NewL( length );
        ptr = value->Des();
        
		error = iRepository->Get( aKey, ptr );
        }
    if( error == KErrNone )
        {
        aBuf.Zero();
        aBuf.Append( ptr );
        }
    delete value;
    
    BROWSER_LOG( ( _L( "CBrowserPreferences::GetStringValueL(TUint32,TInt,TDes&) returns %d" ),
        error ) );
    return error;
    }


// ----------------------------------------------------------------------------
// CBrowserPreferences::SetStringValueL
// ----------------------------------------------------------------------------
//
TBool CBrowserPreferences::SetStringValueL( TUint32 aKey, const TDesC& aValue )
    {
LOG_ENTERFN("CBrowserPreferences::SetStringValueL");

    // Log setting value BEFORE the change
    #ifdef _DEBUG
        TPtrC paramVal = aValue;
        BROWSER_LOG( ( _L( "aValue %S" ), &paramVal ) );
        HBufC* doNothing = HBufC::NewL( 512 );
        TPtr preChange = doNothing->Des();
    	iRepository->Get( aKey, preChange );	
        BROWSER_LOG( ( _L( "Prechange CenRep setting value: %S" ), &preChange ) );
        delete doNothing;
    #endif // _DEBUG

    TBool retVal=EFalse;
	
	if ( iRepository && ( KErrNone == iRepository->Set( aKey, aValue ) ) )
    	{
    	retVal = ETrue;
    	}	
    // Log setting value AFTER the change
    #ifdef _DEBUG
        HBufC* doNothing2 = HBufC::NewL( 512 );
        TPtr postChange = doNothing2->Des();
   		iRepository->Get( aKey, postChange );	
        
        BROWSER_LOG( ( _L( "Postchange CenRep setting value: %S" ), &postChange ) );
        delete doNothing2;
    #endif // _DEBUG
    
    BROWSER_LOG( ( _L( "CBrowserPreferences::SetStringValueL returns %d" ), retVal) );
    return retVal;
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetOverridenSettingsL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetOverriddenSettingsL(TBrowserOverriddenSettings* aSettings)
    {
LOG_ENTERFN("CBrowserPreferences::SetOverriddenSettingsL");

    if( aSettings != NULL )
        {
        TUint tempValue;
        iOverriden = ETrue;
        iOverridenSettings = aSettings;
        
        // Check overridden settings by looking at their default values
        // (see how Launcher2 sets them in the OverriddenSettings constructor)
        tempValue = iOverridenSettings->GetBrowserSetting( EBrowserOverSettingsCustomAp );
        if( tempValue != 0 )
            {
            iAllPreferences.iAccessPointSelectionMode = EConnectionMethod;
            iCustomAp = tempValue;
          	SetDefaultAccessPointL( iCustomAp );
            }
        tempValue = iOverridenSettings->GetBrowserSetting( EBrowserOverSettingsAutoLoadImages );
        if( tempValue != KMaxTUint )
            {           
            iAllPreferences.iAutoLoadContent = tempValue;
            }

        tempValue = iOverridenSettings->GetBrowserSetting( EBrowserOverSettingsFontSize );
        if( tempValue != EBrowserOverFontSizeLevelUndefined )
            {
            iAllPreferences.iFontSize = tempValue;
            }
        tempValue = iOverridenSettings->GetBrowserSetting( EBrowserOverSettingsFullScreen );
        switch( tempValue )
            {
            case KBrowserFullScreenSettingDataSoftkeysOff:
                {
                iAllPreferences.iFullScreen = EWmlSettingsFullScreenFullScreen;
                break;
                }
            case KBrowserFullScreenSettingDataSoftkeysOn:
            default:
                {
                iAllPreferences.iFullScreen = EWmlSettingsFullScreenSoftkeysOnly;
                break;
                }
            }
        }
    return;
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetEmbeddedModeL(TBool aEmbeddedMode)
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetEmbeddedModeL(TBool aEmbeddedMode)
    {
    LOG_ENTERFN("CBrowserPreferences::SetEmbeddedModeL");

    iEmbeddedMode = aEmbeddedMode;
    if( iEmbeddedMode /* && ApiProvider().StartedUp()*/ )
        {
        NotifyObserversL( EPreferencesItemChange, 
                                            TBrCtlDefs::ESettingsEmbedded );
        }
    return;
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetMediaVolumeL( TInt aMediaVolume )
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetMediaVolumeL( TInt aMediaVolume )
    {
    LOG_ENTERFN("CBrowserPreferences::SetMediaVolumeL");

    // Media Volume uses different SD ini. Revert to Browser SD ini after use
    iAllPreferences.iMediaVolume = aMediaVolume;
    SetIntValueL ( KBrowserMediaVolumeControl, iAllPreferences.iMediaVolume );
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetHomePageTypeL( TWmlSettingsHomePage aHomePageType )
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetHomePageTypeL( TWmlSettingsHomePage aHomePageType )
    {
    LOG_ENTERFN("CBrowserPreferences::SetHomePageTypeL");
	SetIntValueL( KBrowserNGHomepageType, aHomePageType );
    iAllPreferences.iHomePgType = aHomePageType;
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetHomePageUrlL( TDesC& aHomePageURL )
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetHomePageUrlL( const TDesC& aHomePageURL )
    {
    LOG_ENTERFN("CBrowserPreferences::SetHomePageUrlL");

    if( aHomePageURL.Length() )
        {
        // Save the homepage with a scheme (default is http if not specified).
        HBufC* temp = Util::AllocateUrlWithSchemeL( aHomePageURL ); 
        delete iAllPreferences.iHomePgURL;
        iAllPreferences.iHomePgURL = temp;
        SetStringValueL( KBrowserNGHomepageURL, 
                                        iAllPreferences.iHomePgURL->Des() );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetSearchPageUrlL( TDesC& aSearchPageURL )
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetSearchPageUrlL( const TDesC& aSearchPageURL )
    {
    LOG_ENTERFN("CBrowserPreferences::SetSearchPageUrlL");

    // Save the searchpage with a scheme (default is http if not specified).
    HBufC* temp = Util::AllocateUrlWithSchemeL( aSearchPageURL ); 
    delete iAllPreferences.iSearchPgURL;
    iAllPreferences.iSearchPgURL = temp;
    SetStringValueL( KBrowserNGSearchPageURL, 
	                            iAllPreferences.iSearchPgURL->Des() );
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::FlushSD()
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::FlushSD()
    {
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::CustomAccessPointDefined()
// ----------------------------------------------------------------------------
//
TBool CBrowserPreferences::CustomAccessPointDefined()
    {
LOG_ENTERFN("CBrowserPreferences::CustomAccessPointDefined");
    TBool retval( iOverridenSettings && iCustomAp );
    BROWSER_LOG( ( _L( "CBrowserPreferences::CustomAccessPointDefined returns %d" ),
        (TInt)retval ) );
    return retval;
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SelfDownloadContentTypesL
// ----------------------------------------------------------------------------
//
TPtrC CBrowserPreferences::SelfDownloadContentTypesL()
    {
LOG_ENTERFN("CBrowserPreferences::SelfDownloadContentTypesL");

    TInt error = KErrNone;
    const TInt KLengthIncrement = 32; // 32 characters
    TInt length = KLengthIncrement; // Initially KLengthIncrement

    do
        {
        HBufC* temp = HBufC::NewL( length );
        delete iSelfDownloadContentTypes;
        iSelfDownloadContentTypes = temp;
        TPtr ptr = iSelfDownloadContentTypes->Des();
		error = iRepository->Get( KBrowserSelfDownloadContentTypes, ptr );

        if ( error == KErrOverflow )
            {
            // Increase the size of the placeholder
            length += KLengthIncrement;
            }
        } while ( error == KErrOverflow );

    #ifdef _DEBUG
        TPtrC logString = *iSelfDownloadContentTypes;
        BROWSER_LOG( ( _L( "CBrowserPreferences::SelfDownloadContentTypesL returns %S" ),
            &logString ) );
    #endif // _DEBUG

    return (*iSelfDownloadContentTypes);
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::AccessPointAlwaysAsk()
// ----------------------------------------------------------------------------
//
TCmSettingSelectionMode CBrowserPreferences::AccessPointSelectionMode()
    {
LOG_ENTERFN("CBrowserPreferences::AccessPointSelectionMode");
BROWSER_LOG( ( _L( "CBrowserPreferences::AccessPointSelectionMode returns %d" ),
        iAllPreferences.iAccessPointSelectionMode ) );
    return iAllPreferences.iAccessPointSelectionMode;
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetAccessPointSelectionModeL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetAccessPointSelectionModeL( 
                TCmSettingSelectionMode aAccessPointSelectionMode )
    {
    LOG_ENTERFN("CBrowserPreferences::SetAccessPointSelectionModeL");

	if ( aAccessPointSelectionMode != iAllPreferences.iAccessPointSelectionMode )
		{
		iAllPreferences.iAccessPointSelectionMode = aAccessPointSelectionMode;

        switch ( aAccessPointSelectionMode )                              
            {
            case EAlwaysAsk:
                {
                SetIntValueL ( KBrowserAccessPointSelectionMode, EBrowserCenRepApSelModeAlwaysAsk ); 
                break; 
                }
            case EDestination:
                {
                SetIntValueL ( KBrowserAccessPointSelectionMode, EBrowserCenRepApSelModeDestination ); 
                break; 
                }
            case EConnectionMethod:
                {
                SetIntValueL ( KBrowserAccessPointSelectionMode, EBrowserCenRepApSelModeUserDefined ); 
                break;
                }
            default:
                {
                SetIntValueL ( KBrowserAccessPointSelectionMode, EBrowserCenRepApSelModeAlwaysAsk ); 
                break; 
                }
            }          
		}  
    }

// ----------------------------------------------------------------------------
// CBrowserPreferences::SetAutomaticUpdatingApL
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetAutomaticUpdatingApL( TUint aSetting )
    {
    LOG_ENTERFN("CBrowserPreferences::SetAutomaticUpdatingApL");

    iAllPreferences.iAutomaticUpdatingAP = aSetting;
    SetIntValueL ( KBrowserNGAutomaticUpdatingAccessPoint, aSetting );
    }


void CBrowserPreferences::SetAutomaticUpdatingWhileRoamingL( TBool aAutoUpdateRoaming )
    {
    LOG_ENTERFN("CBrowserPreferences::SetAutomaticUpdatingWhileRoamingL");
    if ( aAutoUpdateRoaming != iAllPreferences.iAutomaticUpdatingWhileRoaming )
        {
        iAllPreferences.iAutomaticUpdatingWhileRoaming = aAutoUpdateRoaming;
        SetIntValueL( KBrowserNGAutomaticUpdatingWhileRoaming, 
                                        iAllPreferences.iAutomaticUpdatingWhileRoaming );
    	}
    	
    }


   
// ---------------------------------------------------------
// CBrowserPreferences::SetToolbarOnOffL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetToolbarOnOffL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iToolbarOnOff )
        {
        iAllPreferences.iToolbarOnOff = aCommand;
        SetIntValueL( KToolbarOnOff, iAllPreferences.iToolbarOnOff );
        
        NotifyObserversL( EPreferencesItemChange, 
                          TBrCtlDefs::ESettingsToolbarOnOff );
        }
    }

    
    
// ---------------------------------------------------------
// CBrowserPreferences::SetToolbarButton1CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetToolbarButton1CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iToolbarButton1Cmd )
        {
        iAllPreferences.iToolbarButton1Cmd = aCommand;
        SetIntValueL( KToolbarButton1Cmd, iAllPreferences.iToolbarButton1Cmd );
        
        NotifyObserversL( EPreferencesItemChange, 
                          TBrCtlDefs::ESettingsToolbarButton1Cmd );
        }
    }


// ---------------------------------------------------------
// CBrowserPreferences::SetToolbarButton2CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetToolbarButton2CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iToolbarButton2Cmd )
        {
        iAllPreferences.iToolbarButton2Cmd = aCommand;
        SetIntValueL( KToolbarButton2Cmd, iAllPreferences.iToolbarButton2Cmd );
        
        NotifyObserversL( EPreferencesItemChange, 
                          TBrCtlDefs::ESettingsToolbarButton2Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetToolbarButton3CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetToolbarButton3CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iToolbarButton3Cmd )
        {
        iAllPreferences.iToolbarButton3Cmd = aCommand;
        SetIntValueL( KToolbarButton3Cmd, iAllPreferences.iToolbarButton3Cmd );
        
        NotifyObserversL( EPreferencesItemChange, 
                          TBrCtlDefs::ESettingsToolbarButton3Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetToolbarButton4CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetToolbarButton4CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iToolbarButton4Cmd )
        {
        iAllPreferences.iToolbarButton4Cmd = aCommand;
        SetIntValueL( KToolbarButton4Cmd, iAllPreferences.iToolbarButton4Cmd );
        
        NotifyObserversL( EPreferencesItemChange, 
                          TBrCtlDefs::ESettingsToolbarButton4Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetToolbarButton5CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetToolbarButton5CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iToolbarButton5Cmd )
        {
        iAllPreferences.iToolbarButton5Cmd = aCommand;
        SetIntValueL( KToolbarButton5Cmd, iAllPreferences.iToolbarButton5Cmd );
        
        NotifyObserversL( EPreferencesItemChange, 
                          TBrCtlDefs::ESettingsToolbarButton5Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetToolbarButton6CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetToolbarButton6CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iToolbarButton6Cmd )
        {
        iAllPreferences.iToolbarButton6Cmd = aCommand;
        SetIntValueL( KToolbarButton6Cmd, iAllPreferences.iToolbarButton6Cmd );
        
        NotifyObserversL( EPreferencesItemChange, 
                          TBrCtlDefs::ESettingsToolbarButton6Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetToolbarButton7CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetToolbarButton7CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iToolbarButton7Cmd )
        {
        iAllPreferences.iToolbarButton7Cmd = aCommand;
        SetIntValueL( KToolbarButton7Cmd, iAllPreferences.iToolbarButton7Cmd );
        
        NotifyObserversL( EPreferencesItemChange, 
                          TBrCtlDefs::ESettingsToolbarButton7Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetShortcutKey1CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetShortcutKey1CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iShortcutKey1Cmd )
        {
        iAllPreferences.iShortcutKey1Cmd = aCommand;
        SetIntValueL( KShortcutKey1Cmd, iAllPreferences.iShortcutKey1Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetShortcutKey2CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetShortcutKey2CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iShortcutKey2Cmd )
        {
        iAllPreferences.iShortcutKey2Cmd = aCommand;
        SetIntValueL( KShortcutKey2Cmd, iAllPreferences.iShortcutKey2Cmd );
        }
    }
    
// ---------------------------------------------------------
// CBrowserPreferences::SetShortcutKey3CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetShortcutKey3CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iShortcutKey3Cmd )
        {
        iAllPreferences.iShortcutKey3Cmd = aCommand;
        SetIntValueL( KShortcutKey3Cmd, iAllPreferences.iShortcutKey3Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetShortcutKey4CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetShortcutKey4CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iShortcutKey4Cmd )
        {
        iAllPreferences.iShortcutKey4Cmd = aCommand;
        SetIntValueL( KShortcutKey4Cmd, iAllPreferences.iShortcutKey4Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetShortcutKey5CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetShortcutKey5CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iShortcutKey5Cmd )
        {
        iAllPreferences.iShortcutKey5Cmd = aCommand;
        SetIntValueL( KShortcutKey5Cmd, iAllPreferences.iShortcutKey5Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetShortcutKey6CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetShortcutKey6CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iShortcutKey6Cmd )
        {
        iAllPreferences.iShortcutKey6Cmd = aCommand;
        SetIntValueL( KShortcutKey6Cmd, iAllPreferences.iShortcutKey6Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetShortcutKey7CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetShortcutKey7CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iShortcutKey7Cmd )
        {
        iAllPreferences.iShortcutKey7Cmd = aCommand;
        SetIntValueL( KShortcutKey7Cmd, iAllPreferences.iShortcutKey7Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetShortcutKey8CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetShortcutKey8CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iShortcutKey8Cmd )
        {
        iAllPreferences.iShortcutKey8Cmd = aCommand;
        SetIntValueL( KShortcutKey8Cmd, iAllPreferences.iShortcutKey8Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetShortcutKey9CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetShortcutKey9CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iShortcutKey9Cmd )
        {
        iAllPreferences.iShortcutKey9Cmd = aCommand;
        SetIntValueL( KShortcutKey9Cmd, iAllPreferences.iShortcutKey9Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetShortcutKey0CmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetShortcutKey0CmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iShortcutKey0Cmd )
        {
        iAllPreferences.iShortcutKey0Cmd = aCommand;
        SetIntValueL( KShortcutKey0Cmd, iAllPreferences.iShortcutKey0Cmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetShortcutKeyStarCmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetShortcutKeyStarCmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iShortcutKeyStarCmd )
        {
        iAllPreferences.iShortcutKeyStarCmd = aCommand;
        SetIntValueL( KShortcutKeyStarCmd, iAllPreferences.iShortcutKeyStarCmd );
        }
    }

// ---------------------------------------------------------
// CBrowserPreferences::SetShortcutKeyHashCmdL()
// ---------------------------------------------------------
//
void CBrowserPreferences::SetShortcutKeyHashCmdL(TInt aCommand)
    {
    if ( aCommand != iAllPreferences.iShortcutKeyHashCmd )
        {
        iAllPreferences.iShortcutKeyHashCmd = aCommand;
        SetIntValueL( KShortcutKeyHashCmd, iAllPreferences.iShortcutKeyHashCmd );
        }
    }

    
// ----------------------------------------------------------------------------
// CBrowserPreferences::SetURLSuffixList
// ----------------------------------------------------------------------------
//
void CBrowserPreferences::SetURLSuffixList( HBufC* aSetting )
    {
    LOG_ENTERFN("CBrowserPreferences::SetUpdateHoursDefined");
        
    SetStringValueL ( KBrowserUrlSuffix, aSetting->Des() );    

    TPtr ptr = iAllPreferences.iUrlSuffixList->Des();
    ptr.Copy( aSetting->Des() );
    }

// End of File
