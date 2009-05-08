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


// INCLUDE FILES

#include <BrCtlDefs.h>
#include <BrCtlInterface.h>

#include "BrowserWindow.h"
#include "BrowserContentView.h"
#include "BrowserContentViewContainer.h"
#include "BrowserDialogsProvider.h"
#include "BrowserDialogsProviderProxy.h"
#include "BrowserSoftkeysObserver.h"
#include "BrowserSpecialLoadObserver.h"
#include "BrowserWindowManager.h"

#include "ApiProvider.h"
#include "BrowserAppUi.h"
#include "BrowserUIVariant.hrh"
#include "Logger.h"
#include "BrowserDisplay.h"
#include "BrowserPopupEngine.h"
#include <CDownloadMgrUiDownloadsList.h>
#include <DownloadMgrClient.h>

// for instantiating BrCtlInterface
#include "CommonConstants.h"
#include "BrowserApplication.h"
#include "BrowserAppDocument.h"

#include "Preferences.h"

// for dialog notes
#include <StringLoader.h>
#include <BrowserNG.rsg>
#include "BrowserDialogs.h"


#define CALL_ORIG iWindowManager->ApiProvider()->

TBool CBrowserWindow::iIsPageOverviewSupportQueried = EFalse;
TBool CBrowserWindow::iIsPageOverviewSupported = EFalse;

// -----------------------------------------------------------------------------
// CBrowserWindow::NewLC()
// -----------------------------------------------------------------------------
//
CBrowserWindow* CBrowserWindow::NewLC(
        TInt aWindowId,
        const TDesC* aTargetName,
        CBrowserWindowManager* aWindowMgr )
    {
    CBrowserWindow* self = new (ELeave) 
        CBrowserWindow( aWindowId, aTargetName, aWindowMgr );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::NewL()
// -----------------------------------------------------------------------------
//
CBrowserWindow* CBrowserWindow::NewL(
        TInt aWindowId,
        const TDesC* aTargetName,
        CBrowserWindowManager* aWindowMgr )
    {
    CBrowserWindow* self = 
        CBrowserWindow::NewLC( aWindowId, aTargetName, aWindowMgr );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::~CBrowserWindow()
// -----------------------------------------------------------------------------
//
CBrowserWindow::~CBrowserWindow()
    {
    iClosing = ETrue;
    
LOG_ENTERFN("CBrowserWindow::~CBrowserWindow");
BROWSER_LOG( ( _L( "window id: %d" ), WindowId() ) );

    if( iBrCtlInterface )
        {
        iBrCtlInterface->RemoveLoadEventObserver( iLoadObserver );
        BROWSER_LOG( ( _L( " RemoveLoadEventObserver" ) ) );
        
        CBrowserContentView* cv = iWindowManager->ContentView();    
        iBrCtlInterface->RemoveStateChangeObserver( cv );
        BROWSER_LOG( ( _L( " RemoveStateChangeObserver" ) ) );
        iBrCtlInterface->RemoveCommandObserver( cv );
        BROWSER_LOG( ( _L( " RemoveCommandObserver" ) ) );
        
        
        // TRAP_IGNORE( iBrCtlInterface->HandleCommandL(
        //    (TInt)TBrCtlDefs::ECommandCancelFetch + (TInt)TBrCtlDefs::ECommandIdBase ) );
        BROWSER_LOG( ( _L( " HandleCommandL" ) ) );
        }

    // security indicator must be set off.
    if( iDisplay && !iWindowManager->ApiProvider()->
        Preferences().HttpSecurityWarningsStatSupressed() )
        {
        BROWSER_LOG( ( _L( " UpdateSecureIndicatorL 1" ) ) );
        TRAP_IGNORE( iDisplay->UpdateSecureIndicatorL( EAknIndicatorStateOff ) );
        BROWSER_LOG( ( _L( " UpdateSecureIndicatorL 2" ) ) );
        // we cannot do much here if error occurs... application is closing anyway
        }
    delete iDisplay;
    BROWSER_LOG( ( _L( " Display deleted.") ) );

    //Delete iBrCtlInterface before observers. If this is not
    //done then browser crashes on exit when page is being loaded.
    delete iBrCtlInterface;
    BROWSER_LOG( ( _L( " BrCtlInterface deleted.") ) );

    delete iLoadObserver;
    BROWSER_LOG( ( _L( " LoadObserver deleted.") ) );

    delete iDialogsProviderProxy;
    BROWSER_LOG( ( _L( " DialogsProviderProxy deleted.") ) );

    delete iSpecialLoadObserver;
    BROWSER_LOG( ( _L( " SpecialLoadObserver deleted.") ) );

    delete iSoftkeysObserver;
    BROWSER_LOG( ( _L( " SoftkeysObserver deleted.") ) );
    //
    delete iTargetName;
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::CBrowserWindow()
// -----------------------------------------------------------------------------
//
CBrowserWindow::CBrowserWindow(
        TInt aWindowId,
        const TDesC* aTargetName, 
        CBrowserWindowManager* aWindowMgr ) :
    iWindowId( aWindowId ),
    iActive( EFalse ),
    iFirstPage(ETrue),
    iBrCtlInterface( NULL ),
    iDialogsProviderProxy( NULL ),
    iDisplay( NULL ),
    iLoadObserver( NULL ),
    iSoftkeysObserver( NULL ),
    iSpecialLoadObserver( NULL ),
    iWindowManager( aWindowMgr )
    {
    	__ASSERT_DEBUG( (aTargetName != NULL), Util::Panic( Util::EUninitializedData ));
    	iTargetName = aTargetName->Alloc();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::ConstructL()
// -----------------------------------------------------------------------------
//
void CBrowserWindow::ConstructL()
    {
PERFLOG_LOCAL_INIT
LOG_ENTERFN("CBrowserWindow::ConstructL");
    MApiProvider* api = iWindowManager->ApiProvider();
    CleanupStack::PushL( api );
PERFLOG_STOPWATCH_START
    iDisplay = CBrowserDisplay::NewL( *this, *this );
PERFLOG_STOP_WRITE("*BrowserDisplay*")
    BROWSER_LOG( ( _L( "  Multiple Display up" ) ) );

    CBrowserContentView* cv = iWindowManager->ContentView();
    TRect rect = cv->Container()->Rect();

PERFLOG_STOPWATCH_START
    iLoadObserver = CBrowserLoadObserver::NewL( *this, *cv, *this );
    BROWSER_LOG( ( _L( "  Multiple LoadObserver up" ) ) );
PERFLOG_STOP_WRITE("*LoadObs*")
PERFLOG_STOPWATCH_START
    iDialogsProviderProxy = CBrowserDialogsProviderProxy::NewL( 
        api->DialogsProvider(), iLoadObserver, *this );
PERFLOG_STOP_WRITE("*DlgProxy*")
    BROWSER_LOG( ( _L( "  Multiple CBrowserDialogsProviderProxy UP" ) ) );
PERFLOG_STOPWATCH_START
    iSoftkeysObserver = CBrowserSoftkeysObserver::NewL( *this );
    BROWSER_LOG( ( _L( "  Multiple SoftkeyObserver up" ) ) );
PERFLOG_STOP_WRITE("*SoftKeyObs*")
PERFLOG_STOPWATCH_START    
    iSpecialLoadObserver = CBrowserSpecialLoadObserver::NewL( 
        *this, this, CBrowserAppUi::Static()->Document(), cv );
PERFLOG_STOP_WRITE("*SoftKeyObs*")
    BROWSER_LOG( ( _L( "  Multiple SpecialLoadObserver up" ) ) );

    TInt autoLaunch( TBrCtlDefs::ECapabilityLaunchViewer );
    CBrowserAppDocument* doc = STATIC_CAST( CBrowserAppDocument*,
        CBrowserAppUi::Static()->Document() );

    if( doc->IsContentHandlerRegistered() )
        {
        autoLaunch = 0;
        }                                                                  
	
PERFLOG_STOPWATCH_START
	TUint capabilityFlags = ( TBrCtlDefs::ECapabilityLoadHttpFw |
        TBrCtlDefs::ECapabilityDisplayScrollBar |
        TBrCtlDefs::ECapabilityConfirmDownloads | 
        TBrCtlDefs::ECapabilitySavedPage |
        TBrCtlDefs::ECapabilityUseDlMgr |
        TBrCtlDefs::ECapabilityCursorNavigation |
		TBrCtlDefs::ECapabilityFavicon |
        TBrCtlDefs::ECapabilityToolBar |
        TBrCtlDefs::ECapabilityFitToScreen |
        autoLaunch );
        
	if ( Preferences().UiLocalFeatureSupported( KBrowserGraphicalPage ))
		{
		capabilityFlags = capabilityFlags | TBrCtlDefs::ECapabilityGraphicalPage;
		}
	if ( Preferences().UiLocalFeatureSupported( KBrowserGraphicalPage ) &&
			Preferences().UiLocalFeatureSupported( KBrowserGraphicalHistory ))
		{
		capabilityFlags = capabilityFlags | TBrCtlDefs::ECapabilityGraphicalHistory;
		}
	if ( Preferences().UiLocalFeatureSupported( KBrowserAutoFormFill ) )
		{
		capabilityFlags = capabilityFlags | TBrCtlDefs::ECapabilityAutoFormFill;
		}
    if (Preferences().AccessKeys())
		{
		capabilityFlags = capabilityFlags | TBrCtlDefs::ECapabilityAccessKeys;
		}
		
    iBrCtlInterface = CreateBrowserControlL(
        (CCoeControl*)cv->Container(),      // parent control
        rect,                               // client rect
        capabilityFlags,                    // capabilities
        (TUint)TBrCtlDefs::ECommandIdBase,    // command base
        iSoftkeysObserver,
        (MBrCtlLinkResolver*)NULL,            // LinkResolver
        iSpecialLoadObserver,
        (MBrCtlLayoutObserver*)NULL,          // Layout Observer
        iDialogsProviderProxy,
        this                                  // Window Observer
        );
    BROWSER_LOG( ( _L( "  Multiple Plugin Br Control up, Kimono." ) ) );
        
    iBrCtlInterface->SetBrowserSettingL(TBrCtlDefs::ESettingsLaunchAppUid, KUidBrowserApplication.iUid);
    iBrCtlInterface->SetBrowserSettingL(TBrCtlDefs::ESettingsLaunchViewId, KUidBrowserBookmarksViewId.iUid);
    iBrCtlInterface->SetBrowserSettingL(TBrCtlDefs::ESettingsLaunchCustomMessageId, KUidCustomMsgDownloadsList.iUid );
    BROWSER_LOG( ( _L( "  LaunchSettings are passed." ) ) );

#if defined( __RSS_FEEDS )
    TPtrC rssMimeTypes = api->FeedsClientUtilities().SupportedMimeTypesL();
    BROWSER_LOG( ( _L( "  rss SupportedMimeTypes OK" ) ) );
    TPtrC selfDownloadContentTypes = api->Preferences().SelfDownloadContentTypesL();
    BROWSER_LOG( ( _L( "  SelfDownloadContentTypesL OK" ) ) );

    HBufC* buf = HBufC::NewLC(
        rssMimeTypes.Length() + selfDownloadContentTypes.Length() + 2 );
    TPtr ptr( buf->Des() );
    ptr.Append( rssMimeTypes );
    ptr.Append( TChar(';') );
    ptr.Append( selfDownloadContentTypes );
    ptr.ZeroTerminate();

    iBrCtlInterface->SetSelfDownloadContentTypesL( ptr );
    CleanupStack::PopAndDestroy();  // buf
#else  // __RSS_FEEDS
    TPtrC selfDownloadContentTypes = api->Preferences().SelfDownloadContentTypesL();
    BROWSER_LOG( ( _L( "  SelfDownloadContentTypesL OK" ) ) );
    iBrCtlInterface->SetSelfDownloadContentTypesL( selfDownloadContentTypes );
#endif  // __RSS_FEEDS
    BROWSER_LOG( ( _L( "  SetSelfDownloadContentTypesL") ) );

    iBrCtlInterface->AddLoadEventObserverL( iLoadObserver );
    BROWSER_LOG( ( _L( "  AddLoadEventObserverL") ) );
    iBrCtlInterface->AddStateChangeObserverL( cv );
    BROWSER_LOG( ( _L( "  AddStateChangeObserverL") ) );
    iBrCtlInterface->AddCommandObserverL( cv );
    BROWSER_LOG( ( _L( "  AddCommandObserverL") ) );

    // Window is not added as PreferencesObserver,
    // this message flow is controlled by the WindowManager.
    
    // Each window needs to notify its Browser Control of the settings
PERFLOG_STOP_WRITE("***BctlItem creation***")
PERFLOG_STOPWATCH_START    
    InitialiseLocalPreferencesL();
    UpdateBrCtlSettingsL();
PERFLOG_STOP_WRITE("***BrowserCtl Settings update***")
    CleanupStack::Pop( api );
    
    iIsPageOverviewOn = iWindowManager->ApiProvider()->Preferences().PageOverview();
    
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::OpenWindowL()
// -----------------------------------------------------------------------------
//
CBrCtlInterface* CBrowserWindow::OpenWindowL(
        TDesC& /* aUrl */, TDesC* aTargetName,
        TBool aUserInitiated, TAny* /* aReserved */ )
    {
    // do not create new window when the browser is being closed
    if( iClosing )
        {
        return NULL;
        }
LOG_ENTERFN("CBrowserWindow::OpenWindowL, OSS");
	// Do not exceed the maximum number of windows allowed
	if ( iWindowManager->WindowCount() == 
	     iWindowManager->MaxWindowCount() )
	    {
	    
	    // Show warning to user
	  	TBrowserDialogs::ErrorNoteL( R_BROWSER_NOTE_MAX_WINDOWS );
	  
        return NULL;
	    }
	    
	CBrCtlInterface* retVal( NULL );
	TBool loadUrl( EFalse );    // allow fetching of url
	
	CBrowserPopupEngine* popup = &( iWindowManager->ApiProvider()->PopupEngine() );
	// embedded mode doesn't allow multiple windows feature, hence popup object is NULL
	if (!popup) {
		return retVal;
	}

    HBufC* url = BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoUrl );
	if(!url)
	{
		//The url is already pushed on to the cleanup stack and we have the ownership of it. Pop it.
		CleanupStack::Pop(url);
		return NULL;
	}

	TPtrC ptr( url->Des() );
	
	// Check if on the whitelist, if not a user initiated open
	// or if pop-up blocking has been disabled
	if ( !( iWindowManager->ApiProvider()->Preferences().PopupBlocking() ) ||
            popup->IsUrlOnWhiteListL( ptr ) || 
            aUserInitiated )
		{		
	   	loadUrl = ETrue;    // OK to fetch the URL
	   	}	   	
    else
        {
        // Notify user that popup was blocked        
        HBufC* popupBlocked = StringLoader::LoadLC( 
                                            R_BROWSER_NOTE_POPUP_BLOCKED );
	  	iWindowManager->ApiProvider()->
	  	            DialogsProvider().ShowTooltipL( *popupBlocked );
	  	
	  	CleanupStack::PopAndDestroy( popupBlocked );
        }
        
    CleanupStack::PopAndDestroy( url );
    
	if ( loadUrl )
		{
	    // Open a new popup window
	    CBrowserWindow* window = iWindowManager->CreateWindowL( 
	        WindowId(), aTargetName );
	    if (window == NULL)
    	    {
            return retVal;
    	    }
    	    
        retVal = & ( window->BrCtlInterface() );
	    
        // do not initiate load, it does by the engine automatically
        // aUrl is empty / NULL
        
        
	    // Indicate to the user that a new window is being opened
	    HBufC* newWindow = StringLoader::LoadLC( R_BROWSER_NEW_WINDOW_OPENED );
	  	iWindowManager->ApiProvider()->DialogsProvider().
            ShowTooltipL( *newWindow );
	  	CleanupStack::PopAndDestroy( newWindow );
	    }
		
    return retVal;
    }
            
// -----------------------------------------------------------------------------
// CBrowserWindow::FindWindowL()
// -----------------------------------------------------------------------------
//
CBrCtlInterface* CBrowserWindow::FindWindowL( const TDesC& aTargetName ) const
    {
    CBrowserWindow* window = iWindowManager->FindWindowL( aTargetName );
    if( window )
        return & ( window->BrCtlInterface() );
    else
        return NULL;
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::HandleWindowCommandL()
// -----------------------------------------------------------------------------
//
void CBrowserWindow::HandleWindowCommandL(
        const TDesC& aTargetName,
        TBrCtlWindowCommand aCommand )
    {
LOG_ENTERFN("CBrowserWindow::HandleWindowCommandL");
BROWSER_LOG( ( _L( "window Id: %d, aCommand: %d" ), WindowId(), aCommand ) );
    
    // KNullDesC() refers to the current window
    
    // I am NOT that targetwindow
    if( aTargetName != KNullDesC() &&  iTargetName->Compare( aTargetName ) != 0 )
        {
        // try to find that window 
        CBrowserWindow* window = iWindowManager->FindWindowL( aTargetName );
        if( window )
            {
            window->HandleWindowCommandL( aTargetName, aCommand );
            }
        else
            {
            // no window with that name
            }
        }
    else  // I am that targetwindow
        {
        switch( aCommand )
            {
            case EShowWindow:  // to activate a window after creating it
            case EFocusWindow:  // window.focus or reload
                // focusing this window
                {
                // Changes were made to ClearHistory that cause it to keep one (current) page
                // In order to reuse a window we need to clear the history again after the first new page
                // is loaded
                if (iFirstPage)
	            	{
	            	BrCtlInterface().HandleCommandL((TInt)TBrCtlDefs::ECommandClearHistory + (TInt)TBrCtlDefs::ECommandIdBase);
	            	iFirstPage = EFalse;
	            	}
				iWindowManager->SwitchWindowL( WindowId() );
                break;
                }
            case EUnfocusWindow:  // focusing the last active window
                {
                // iWindowManager->SwitchWindowL( );
                break;
                }
            case ECloseWindow:
                {
                CBrowserAppUi::Static()->CloseWindowL( WindowId() );
                break;
                }
//            case EDisableScriptLog:
//                iWindowManager->ApiProvider()->Preferences().SetScriptLogL( TBrCtlDefs::EScriptLogDisable );
//                break;
            default:
                {
                break;
                }
            }
        }
    }

// ---------------------------------------------------------
// CBrowserWindow::IsPageOverviewSupported
// ---------------------------------------------------------
// 
TBool CBrowserWindow::IsPageOverviewSupportedL() 
    {
    if(!iIsPageOverviewSupportQueried)
        {
        iIsPageOverviewSupported = iBrCtlInterface->BrowserSettingL(
            TBrCtlDefs::ESettingsPageOverview );
        iIsPageOverviewSupportQueried = ETrue;                
        }
    
    return iIsPageOverviewSupported;    
    }
    
// -----------------------------------------------------------------------------
// CBrowserWindow::ResetPageOverviewLocalSettingL()
// -----------------------------------------------------------------------------
//	
void CBrowserWindow::ResetPageOverviewLocalSettingL()
    {
    iIsPageOverviewOn = 
        iWindowManager->ApiProvider()->Preferences().PageOverview();
    BrCtlInterface().SetBrowserSettingL(TBrCtlDefs::ESettingsPageOverview,
                                        iIsPageOverviewOn );
    }        

// -----------------------------------------------------------------------------
// CBrowserWindow::ChangeTitlePaneTextUntilL()
// -----------------------------------------------------------------------------
//	
void CBrowserWindow::ChangeTitlePaneTextUntilL(
        const TDesC& aTitle, TInt /* aInterval */ )
    {
    iWindowManager->ApiProvider()->DialogsProvider().ShowTooltipL( aTitle );
    }
    		    
// -----------------------------------------------------------------------------
// CBrowserWindow::FlipPageOverviewLocalSettingL()
// -----------------------------------------------------------------------------
//	
void CBrowserWindow::FlipPageOverviewLocalSettingL()
    {  
    iIsPageOverviewOn = !iIsPageOverviewOn;
    BrCtlInterface().SetBrowserSettingL( TBrCtlDefs::ESettingsPageOverview, 
                                         iIsPageOverviewOn );                                            
    TInt tooltipResource = iIsPageOverviewOn ? 
        R_BROWSER_TOOLTIP_PAGE_OVERVIEW_ON : R_BROWSER_TOOLTIP_PAGE_OVERVIEW_OFF;
    HBufC* overviewTooltip = StringLoader::LoadLC( tooltipResource );
  
  	iWindowManager->ApiProvider()->DialogsProvider().ShowTooltipL( *overviewTooltip );  	
  	CleanupStack::PopAndDestroy( overviewTooltip );                                     
    }
    
// -----------------------------------------------------------------------------
// CBrowserWindow::BrCtlInterface()
// -----------------------------------------------------------------------------
//
CBrCtlInterface& CBrowserWindow::BrCtlInterface() const
    {
    return *iBrCtlInterface;
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::Display()
// -----------------------------------------------------------------------------
//
MDisplay& CBrowserWindow::Display() const
    {
    return *iDisplay;
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::LoadObserver()
// -----------------------------------------------------------------------------
//
CBrowserLoadObserver& CBrowserWindow::LoadObserver() const
    {
    return *iLoadObserver;
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::SoftkeysObserver()
// -----------------------------------------------------------------------------
//
CBrowserSoftkeysObserver& CBrowserWindow::SoftkeysObserver() const
    {
    return *iSoftkeysObserver;
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::SpecialLoadObserver()
// -----------------------------------------------------------------------------
//
CBrowserSpecialLoadObserver& CBrowserWindow::SpecialLoadObserver() const
    {
    return *iSpecialLoadObserver;
    }

// ----------------------------------------------------------------------------
// CBrowserWindow::HandlePreferencesChangeL()
// ----------------------------------------------------------------------------
//
void CBrowserWindow::HandlePreferencesChangeL( 
                                    TPreferencesEvent aEvent,
                                    TPreferencesValues& aValues,
                                    TBrCtlDefs::TBrCtlSettings aSettingType  )
    {
    switch ( aEvent )
        {
        case EPreferencesActivate:
            {
            // Provide local settings
            aValues.iFontSize = iFontSize;
            aValues.iTextWrap = iTextWrap;
            aValues.iEncoding = iEncoding;
            break;
            }
        case EPreferencesDeactivate:
            {
            // Save local preferences and notify Browser Control
            if ( iActive )  // Topmost window only
                {
                UpdateLocalPreferencesL( aValues );
                }
            break;
            }
        case EPreferencesItemChange:
            {
            // All preferences changes should notify the 
            // Browser Control
            UpdateGlobalPreferencesL( aSettingType );
            
            if ( iActive )  // Topmost window only
                {
                UpdateLocalPreferencesL( aValues );
                // Change the local setting to the changed setting.
                iIsPageOverviewOn = iWindowManager->ApiProvider()->Preferences().PageOverview();
                }                                            
            break;
            }
        default:
            // Do nothing
            break;
        }
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::UpdateGlobalPreferencesL()
// -----------------------------------------------------------------------------
//
void CBrowserWindow::UpdateGlobalPreferencesL( 
                                    TBrCtlDefs::TBrCtlSettings aSettingType )
    {
    // Local Settings should be ignored
    switch ( aSettingType )
        {
        case TBrCtlDefs::ESettingsAutoLoadImages:
                {               
        // WebEngine has two different settings for Content Autoload enable/disable:
        // AutoLoad Images - True/False and Disable flash - True/False        
        // Here is how the Load Content setting in UI maps to the Image and 
        // Flash settings in Webengine in case flash Plugin is present
        // Load Content = 0, AutoLoadImages = 0, DisableFlash = 1
        // Load Content = 1, AutoLoadImages = 1, DisableFlash = 1
        // Load Content = 2, AutoLoadImages = 1, DisableFlash = 0
        // In case flash plugin is not present
        // Load Content = 0, AutoLoadImages = 0, DisableFlash = 1
        // Load Content = 1, AutoLoadImages = 1, DisableFlash = 1
        
                iBrCtlInterface->SetBrowserSettingL( 
                                            TBrCtlDefs::ESettingsAutoLoadImages,
                                            ((Preferences().AutoLoadContent() == EWmlSettingsAutoloadText)
                                             ?EFalse:ETrue));
                
                iBrCtlInterface->SetBrowserSettingL( 
                                            TBrCtlDefs::ESettingsDisableFlash,                                                            
                                            ((Preferences().AutoLoadContent() == EWmlSettingsAutoloadAll)
                                             ?EFalse:ETrue));                                                                                       
                break;
                }
        case TBrCtlDefs::ESettingsEmbedded:
                {
                iBrCtlInterface->SetBrowserSettingL( 
                                            TBrCtlDefs::ESettingsEmbedded, 
                                            Preferences().EmbeddedMode() );
                break;
                }
        case TBrCtlDefs::ESettingsCookiesEnabled:
                {
                iBrCtlInterface->SetBrowserSettingL( 
                                            TBrCtlDefs::ESettingsCookiesEnabled, 
                                            Preferences().Cookies() );
                break;
                }
        case TBrCtlDefs::ESettingsECMAScriptEnabled:
                {
                iBrCtlInterface->SetBrowserSettingL( 
                                            TBrCtlDefs::ESettingsECMAScriptEnabled, 
                                            Preferences().Ecma() );
                break;
                }
        case TBrCtlDefs::ESettingsScriptLog:
                {
                iBrCtlInterface->SetBrowserSettingL( 
                                            TBrCtlDefs::ESettingsScriptLog, 
                                            Preferences().ScriptLog() );
                break;
                }
        case TBrCtlDefs::ESettingsIMEINotifyEnabled:
                {
                iBrCtlInterface->SetBrowserSettingL( 
                                            TBrCtlDefs::ESettingsIMEINotifyEnabled, 
                                            Preferences().IMEINotification() );
                break;
                }
        case TBrCtlDefs::ESettingsSendRefererHeader:
                {
                iBrCtlInterface->SetBrowserSettingL( 
                                            TBrCtlDefs::ESettingsSendRefererHeader, 
                                            Preferences().SendReferrer() );
                break;
                }
        case TBrCtlDefs::ESettingsSecurityWarnings:
                {
                iBrCtlInterface->SetBrowserSettingL( 
                                            TBrCtlDefs::ESettingsSecurityWarnings, 
                                            Preferences().HttpSecurityWarnings() );
                break;
                }

        case TBrCtlDefs::ESettingsAutoOpenDownloads:
                {
                iBrCtlInterface->SetBrowserSettingL( 
                                            TBrCtlDefs::ESettingsAutoOpenDownloads, 
                                            Preferences().DownloadsOpen() );
                break;
                }

        case TBrCtlDefs::ESettingsPageOverview:
        		{
        		if( IsPageOverviewSupportedL() )
        		    {
                    iBrCtlInterface->SetBrowserSettingL( 
                        TBrCtlDefs::ESettingsPageOverview, 
                        Preferences().PageOverview() );
        		    }
                break;
                } 
        case TBrCtlDefs::ESettingsBackList:
        		{
                iBrCtlInterface->SetBrowserSettingL( 
                                            TBrCtlDefs::ESettingsBackList, 
                                            Preferences().BackList() );
                break;
                } 
        case TBrCtlDefs::ESettingsAutoRefresh:
        		{
                iBrCtlInterface->SetBrowserSettingL( 
                                            TBrCtlDefs::ESettingsAutoRefresh, 
                                            Preferences().AutoRefresh() );
                break;
                }
        case TBrCtlDefs::ESettingsAutoFormFillEnabled:
            {
            iBrCtlInterface->SetBrowserSettingL(
                                        TBrCtlDefs::ESettingsAutoFormFillEnabled,
                                        Preferences().FormDataSaving() );
            break;
            }
        case TBrCtlDefs::ESettingsToolbarOnOff:
        	{
        	iBrCtlInterface->SetBrowserSettingL(
                                        TBrCtlDefs::ESettingsToolbarOnOff,
                                        Preferences().ShowToolbarOnOff() );
            break;
        	}
        case TBrCtlDefs::ESettingsToolbarButton1Cmd:
        	{
        	iBrCtlInterface->SetBrowserSettingL(
                                        TBrCtlDefs::ESettingsToolbarButton1Cmd,
                                        Preferences().ShowToolbarButton1Cmd() );
            break;
        	}
        case TBrCtlDefs::ESettingsToolbarButton2Cmd:
        	{
        	iBrCtlInterface->SetBrowserSettingL(
                                        TBrCtlDefs::ESettingsToolbarButton2Cmd,
                                        Preferences().ShowToolbarButton2Cmd() );
            break;
        	}
        case TBrCtlDefs::ESettingsToolbarButton3Cmd:
        	{
        	iBrCtlInterface->SetBrowserSettingL(
                                        TBrCtlDefs::ESettingsToolbarButton3Cmd,
                                        Preferences().ShowToolbarButton3Cmd() );
            break;
        	}
        case TBrCtlDefs::ESettingsToolbarButton4Cmd:
        	{
        	iBrCtlInterface->SetBrowserSettingL(
                                        TBrCtlDefs::ESettingsToolbarButton4Cmd,
                                        Preferences().ShowToolbarButton4Cmd() );
            break;
        	}
        case TBrCtlDefs::ESettingsToolbarButton5Cmd:
        	{
        	iBrCtlInterface->SetBrowserSettingL(
                                        TBrCtlDefs::ESettingsToolbarButton5Cmd,
                                        Preferences().ShowToolbarButton5Cmd() );
            break;
        	}
        case TBrCtlDefs::ESettingsToolbarButton6Cmd:
        	{
        	iBrCtlInterface->SetBrowserSettingL(
                                        TBrCtlDefs::ESettingsToolbarButton6Cmd,
                                        Preferences().ShowToolbarButton6Cmd() );
            break;
        	}
        case TBrCtlDefs::ESettingsToolbarButton7Cmd:
        	{
        	iBrCtlInterface->SetBrowserSettingL(
                                        TBrCtlDefs::ESettingsToolbarButton7Cmd,
                                        Preferences().ShowToolbarButton7Cmd() );
            break;
        	}
        case TBrCtlDefs::ESettingsZoomLevelMin:
            {
            iBrCtlInterface->SetBrowserSettingL(
                                        TBrCtlDefs::ESettingsZoomLevelMin,
                                        Preferences().ZoomLevelMinimum() );
            break;
            }
        case TBrCtlDefs::ESettingsZoomLevelMax:
            {
            iBrCtlInterface->SetBrowserSettingL(
                                        TBrCtlDefs::ESettingsZoomLevelMax,
                                        Preferences().ZoomLevelMaximum() );
            break;
            }
        case TBrCtlDefs::ESettingsZoomLevelDefault:
            {
            iBrCtlInterface->SetBrowserSettingL(
                                        TBrCtlDefs::ESettingsZoomLevelDefault,
                                        Preferences().ZoomLevelDefault() );
            break; 
            }
        // LOCAL SETTINGS should be ignored
        case TBrCtlDefs::ESettingsTextWrapEnabled:
        case TBrCtlDefs::ESettingsFontSize:
        case TBrCtlDefs::ESettingsCharacterset:
            {            
            break;
            }
        case TBrCtlDefs::ESettingsUnknown: // ignore
        default:
            break;
        }
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::UpdateLocalPreferencesL()
// -----------------------------------------------------------------------------
//
void CBrowserWindow::UpdateLocalPreferencesL( TPreferencesValues& aValues )
    {
    if ( iFontSize != aValues.iFontSize )
        {
        iFontSize = aValues.iFontSize;
        iBrCtlInterface->SetBrowserSettingL( 
                ( TUint ) TBrCtlDefs::ESettingsFontSize, iFontSize );
        }
    
    if ( iTextWrap != aValues.iTextWrap )
        {
        iTextWrap = aValues.iTextWrap;
        iBrCtlInterface->SetBrowserSettingL( 
                ( TUint ) TBrCtlDefs::ESettingsTextWrapEnabled,  iTextWrap );
        }
    if ( iEncoding != aValues.iEncoding )
        {
        iEncoding = aValues.iEncoding;
        iBrCtlInterface->SetBrowserSettingL( 
                ( TUint ) TBrCtlDefs::ESettingsCharacterset, iEncoding );
        }
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::InitialiseLocalPreferencesL()
// -----------------------------------------------------------------------------
//
void CBrowserWindow::InitialiseLocalPreferencesL()
    {
    // Initialise local settings from Central Repository
    const TPreferencesValues& pref = Preferences().AllPreferencesL();
    iFontSize = pref.iFontSize;
    iTextWrap = pref.iTextWrap;
    iEncoding = pref.iEncoding;
    }
    
// ----------------------------------------------------------------------------
// CBrowserWindow::UpdateBrCtlSettingsL()
// ----------------------------------------------------------------------------
//
void CBrowserWindow::UpdateBrCtlSettingsL()
    {
    LOG_ENTERFN("CBrowserPreferences::UpdateBrCtlSettingsL");

    UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsSmallScreen );
    UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsAutoLoadImages );
    UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsEmbedded );
    UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsCookiesEnabled );
    UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsCSSFetchEnabled );
    UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsECMAScriptEnabled );
    UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsScriptLog );
    UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsIMEINotifyEnabled );
    UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsSendRefererHeader );
    UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsSecurityWarnings );
    UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsAutoOpenDownloads );    
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsPageOverview );
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsBackList );
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsAutoRefresh );

	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsAutoFormFillEnabled );
	
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsToolbarOnOff );
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsToolbarButton1Cmd );
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsToolbarButton2Cmd );
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsToolbarButton3Cmd );
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsToolbarButton4Cmd );
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsToolbarButton5Cmd );
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsToolbarButton6Cmd );
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsToolbarButton7Cmd );
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsZoomLevelMin );
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsZoomLevelMax );
	UpdateGlobalPreferencesL( TBrCtlDefs::ESettingsZoomLevelDefault );

    // Font Size - LOCAL Setting
    iBrCtlInterface->SetBrowserSettingL( TBrCtlDefs::ESettingsFontSize,
            iWindowManager->ApiProvider()->Preferences().FontSize() );
                        
    // Text Wrap - LOCAL Setting
    iBrCtlInterface->SetBrowserSettingL( TBrCtlDefs::ESettingsTextWrapEnabled,
            iWindowManager->ApiProvider()->Preferences().TextWrap() );
                        
    // Encoding - LOCAL Setting
    iBrCtlInterface->SetBrowserSettingL( TBrCtlDefs::ESettingsCharacterset, 
            iWindowManager->ApiProvider()->Preferences().Encoding() );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::ActivateL()
// -----------------------------------------------------------------------------
//
void CBrowserWindow::ActivateL( TBool aDraw )
    {
LOG_ENTERFN("CBrowserWindow::ActivateL");
BROWSER_LOG( ( _L( "Activating window no. %d" ), WindowId() ) );
    iDialogsProviderProxy->WindowActivated();
    iBrCtlInterface->MakeVisible( ETrue );  // notify the engine
    iActive = ETrue;
    if( aDraw )
        {
        iBrCtlInterface->DrawNow();
        CBrowserContentView* cv = iWindowManager->ContentView();
        cv->UpdateFullScreenL();
        TRAP_IGNORE(Display().StopProgressAnimationL());//also updates title
        }       
    WindowMgr().NotifyObserversL( EWindowActivate, WindowId() );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::DeactivateL()
// -----------------------------------------------------------------------------
//
void CBrowserWindow::DeactivateL()
    {
LOG_ENTERFN("CBrowserWindow::DeactivateL");
BROWSER_LOG( ( _L( "Deactivating window no. %d" ), WindowId() ) );
    iActive = EFalse;
    iBrCtlInterface->MakeVisible( EFalse ); // notify the engine
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::Connection
// -----------------------------------------------------------------------------
//
MConnection& CBrowserWindow::Connection() const
    {
    return CALL_ORIG Connection();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::Preferences()
// -----------------------------------------------------------------------------
//
MPreferences& CBrowserWindow::Preferences() const
    {
    return CALL_ORIG Preferences(); 
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::CommsModel
// -----------------------------------------------------------------------------
//
MCommsModel& CBrowserWindow::CommsModel() const
    {
    return CALL_ORIG CommsModel();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::DialogsProvider()
// -----------------------------------------------------------------------------
//
CBrowserDialogsProvider& CBrowserWindow::DialogsProvider() const
    {
	__ASSERT_DEBUG( (iDialogsProviderProxy != NULL), Util::Panic( Util::EUninitializedData ));
    return *( (CBrowserDialogsProvider*)iDialogsProviderProxy );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::BmOTABinSender
// -----------------------------------------------------------------------------
//
MBmOTABinSender& CBrowserWindow::BmOTABinSenderL()
    {
    return CALL_ORIG BmOTABinSenderL();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::PopupEngine
// -----------------------------------------------------------------------------
//
CBrowserPopupEngine& CBrowserWindow::PopupEngine() const
    {
    return CALL_ORIG PopupEngine();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::SetViewToBeActivatedIfNeededL
// -----------------------------------------------------------------------------
//
void CBrowserWindow::SetViewToBeActivatedIfNeededL(
    TUid aUid,
    TInt aMessageId )
    {
    CALL_ORIG SetViewToBeActivatedIfNeededL( aUid, aMessageId );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::IsCancelFetchAllowed
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::IsCancelFetchAllowed() const
    {
    return CALL_ORIG IsCancelFetchAllowed();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::LastActiveViewId
// -----------------------------------------------------------------------------
//
TUid CBrowserWindow::LastActiveViewId() const
    {
    return CALL_ORIG LastActiveViewId();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::SetLastActiveViewId
// -----------------------------------------------------------------------------
//
void CBrowserWindow::SetLastActiveViewId( TUid aUid )
    {
    CALL_ORIG SetLastActiveViewId( aUid );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::ExitInProgress
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::ExitInProgress() const
    {
    return CALL_ORIG ExitInProgress();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::SetExitInProgress
// -----------------------------------------------------------------------------
//
void CBrowserWindow::SetExitInProgress( TBool aValue )
    {
    CALL_ORIG SetExitInProgress( aValue );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::IsConnecting
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::IsConnecting() const
    {
    return CALL_ORIG IsConnecting();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::UpdateNaviPaneL
// -----------------------------------------------------------------------------
//
void CBrowserWindow::UpdateNaviPaneL( TDesC& aStatusMsg )
    {
    CALL_ORIG UpdateNaviPaneL( aStatusMsg );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::SetViewToReturnOnClose
// -----------------------------------------------------------------------------
//
void CBrowserWindow::SetViewToReturnOnClose( TUid const &aUid )
    {
    CALL_ORIG SetViewToReturnOnClose( aUid );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::CloseContentViewL
// -----------------------------------------------------------------------------
//
void CBrowserWindow::CloseContentViewL()
    {
    CALL_ORIG CloseContentViewL();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::IsEmbeddedModeOn
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::IsEmbeddedModeOn() const
    {
    return CALL_ORIG IsEmbeddedModeOn();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::IsShutdownRequested
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::IsShutdownRequested() const
    {
    return CALL_ORIG IsShutdownRequested();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::ExitBrowser
// -----------------------------------------------------------------------------
//
void CBrowserWindow::ExitBrowser( TBool aUserShutdown )
    {
    CALL_ORIG ExitBrowser( aUserShutdown );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::IsAppShutterActive
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::IsAppShutterActive() const
    {
    return CALL_ORIG IsAppShutterActive();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::StartPreferencesViewL
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::StartPreferencesViewL( TUint aListItems )
    {
    return CALL_ORIG StartPreferencesViewL( aListItems );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::IsPageLoaded
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::IsPageLoaded()
    {
    return CALL_ORIG IsPageLoaded();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::IsForeGround
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::IsForeGround() const
    {
    return CALL_ORIG IsForeGround();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::Fetching
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::Fetching() const
    {
    return CALL_ORIG Fetching();
    }
// -----------------------------------------------------------------------------
// CBrowserWindow::ContentDisplayed
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::ContentDisplayed() const
    {
    return CALL_ORIG ContentDisplayed();
    }
// -----------------------------------------------------------------------------
// CBrowserWindow::SetContentDisplayed
// -----------------------------------------------------------------------------
//
void CBrowserWindow::SetContentDisplayed(TBool aValue)
    {
    return CALL_ORIG SetContentDisplayed( aValue );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::FetchBookmarkL
// -----------------------------------------------------------------------------
//
void CBrowserWindow::FetchBookmarkL( TInt aBookmarkUid )
    {
    CALL_ORIG FetchBookmarkL( aBookmarkUid );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::FetchBookmarkL
// -----------------------------------------------------------------------------
//
void CBrowserWindow::FetchBookmarkL( const CFavouritesItem& aBookmarkItem )
    {
    CALL_ORIG FetchBookmarkL( aBookmarkItem );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::FetchL
// -----------------------------------------------------------------------------
//
void CBrowserWindow::FetchL(
	const TDesC& aUrl,
	const TDesC& aUserName,
	const TDesC& aPassword,
	const TFavouritesWapAp& aAccessPoint,
    CBrowserLoadObserver::TBrowserLoadUrlType aUrlType )
    {
    CALL_ORIG FetchL( aUrl, aUserName, aPassword, aAccessPoint, aUrlType );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::FetchL
// -----------------------------------------------------------------------------
//
void CBrowserWindow::FetchL( 
    const TDesC& aUrl, 
    CBrowserLoadObserver::TBrowserLoadUrlType aUrlType )
    {
    CALL_ORIG FetchL( aUrl, aUrlType );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::CancelFetch
// -----------------------------------------------------------------------------
//
void CBrowserWindow::CancelFetch( TBool aIsUserInitiated )
    {
    CALL_ORIG CancelFetch( aIsUserInitiated );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::SetRequestedAP
// -----------------------------------------------------------------------------
//
void CBrowserWindow::SetRequestedAP( TInt aAp )
    {
    CALL_ORIG SetRequestedAP( aAp );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::SetExitFromEmbeddedMode
// -----------------------------------------------------------------------------
//
void CBrowserWindow::SetExitFromEmbeddedMode( TBool aFlag )
    {
    CALL_ORIG SetExitFromEmbeddedMode( aFlag );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::IsProgressShown
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::IsProgressShown() const
    {
    return CALL_ORIG IsProgressShown();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::SetProgressShown
// -----------------------------------------------------------------------------
//
void CBrowserWindow::SetProgressShown( TBool aProgressShown )
    {
    CALL_ORIG SetProgressShown( aProgressShown );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::StartedUp
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::StartedUp() const
    {
    return CALL_ORIG StartedUp();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::RequestedAp
// -----------------------------------------------------------------------------
//
TUint32 CBrowserWindow::RequestedAp() const
    {
    return CALL_ORIG RequestedAp();
    }
// -----------------------------------------------------------------------------
// CBrowserWindow::LogAccessToRecentUrlL
// -----------------------------------------------------------------------------
//
void CBrowserWindow::LogAccessToRecentUrlL( CBrCtlInterface& aBrCtlInterface)
    {
    CALL_ORIG LogAccessToRecentUrlL( aBrCtlInterface );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::LogRequestedPageToRecentUrlL
// -----------------------------------------------------------------------------
//
void CBrowserWindow::LogRequestedPageToRecentUrlL( const TDesC& aUrl)
    {
    CALL_ORIG LogRequestedPageToRecentUrlL( aUrl );
    }

// ---------------------------------------------------------
// CBrowserWindow::StartProgressAnimationL
// ---------------------------------------------------------
//
void CBrowserWindow::StartProgressAnimationL()
    {
    CALL_ORIG StartProgressAnimationL();
    }

// ---------------------------------------------------------
// CBrowserWindow::StopProgressAnimationL
// ---------------------------------------------------------
//
void CBrowserWindow::StopProgressAnimationL()
    {
    CALL_ORIG StopProgressAnimationL();
    }

#ifdef __RSS_FEEDS
// ---------------------------------------------------------
// CBrowserWindow::FeedsClientUtilities
// ---------------------------------------------------------
//
CFeedsClientUtilities& CBrowserWindow::FeedsClientUtilities() const
    {
    return CALL_ORIG FeedsClientUtilities();
    }
#endif  // __RSS_FEEDS

// ---------------------------------------------------------
// CBrowserWindow::BrowserWasLaunchedIntoFeeds
// ---------------------------------------------------------
//        
TBool CBrowserWindow::BrowserWasLaunchedIntoFeeds()
{ 
	return CALL_ORIG BrowserWasLaunchedIntoFeeds();
}

// ---------------------------------------------------------
// CBrowserWindow::SetHasWMLContent
// ---------------------------------------------------------
//
void CBrowserWindow::SetHasWMLContent(TBool aHasWMLContent) 
	{
	iHasWMLContent = aHasWMLContent;
	}

// ---------------------------------------------------------
// CBrowserWindow::SetCurrWinHasWMLContent
// ---------------------------------------------------------
//
void CBrowserWindow::SetCurrWinHasWMLContent(TBool aCurrWinHasWMLContent)
	{
	iCurrWinHasWMLContent = aCurrWinHasWMLContent;
	}
	
// ---------------------------------------------------------
// CBrowserWindow::HasWMLContent
// ---------------------------------------------------------
//
TBool CBrowserWindow::HasWMLContent (TBool aCurrWinOnly) 
	{ 
	if (aCurrWinOnly)
		{
		return iCurrWinHasWMLContent; 
		}
	else
		{
		return iHasWMLContent;	
		}
	}


// ---------------------------------------------------------
// CBrowserWindow::SetWMLMode
// ---------------------------------------------------------
//
void CBrowserWindow::SetWMLMode(TBool aWMLMode) 
	{
	iWMLMode = aWMLMode;
	}

// ---------------------------------------------------------
// CBrowserWindow::SetFirstPage
// ---------------------------------------------------------
//
void CBrowserWindow::SetFirstPage(TBool aFirstPage) 
	{
	iFirstPage = aFirstPage;
	}

// ---------------------------------------------------------
// CBrowserWindow::SetHasFeedsContent
// ---------------------------------------------------------
//
void CBrowserWindow::SetHasFeedsContent(TBool aHasFeedsContent) 
	{
	iHasFeedsContent = aHasFeedsContent;
	}

// -----------------------------------------------------------------------------
// CBrowserWindow::SetLastCBAUpdateView
// -----------------------------------------------------------------------------
//
void CBrowserWindow::SetLastCBAUpdateView( TUid aView )
    {
    CALL_ORIG SetLastCBAUpdateView( aView );
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::GetLastCBAUpdateView
// -----------------------------------------------------------------------------
//
TUid CBrowserWindow::GetLastCBAUpdateView()
    {
    return CALL_ORIG GetLastCBAUpdateView();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::GetPreviousViewFromViewHistory
// -----------------------------------------------------------------------------
//
TUid CBrowserWindow::GetPreviousViewFromViewHistory()
    {
    return CALL_ORIG GetPreviousViewFromViewHistory();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::CalledFromAnotherApp
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::CalledFromAnotherApp()
    {
    return CALL_ORIG CalledFromAnotherApp();
    }

// -----------------------------------------------------------------------------
// CBrowserWindow::SetCalledFromAnotherApp
// -----------------------------------------------------------------------------
//
void CBrowserWindow::SetCalledFromAnotherApp( TBool aValue )
    {
    return CALL_ORIG SetCalledFromAnotherApp( aValue );
    }
    
// -----------------------------------------------------------------------------
// CBrowserWindow::FlashPresent
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::FlashPresent()
    {
    return CALL_ORIG FlashPresent();
    }
    
    
// -----------------------------------------------------------------------------
// CBrowserWindow::IsLaunchHomePageDimmedL
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::IsLaunchHomePageDimmedL()
    {
    return CALL_ORIG IsLaunchHomePageDimmedL();
    }    
// -----------------------------------------------------------------------------
// CBrowserWindow::IsDisplayingMenuOrDialog
// -----------------------------------------------------------------------------
//
TBool CBrowserWindow::IsDisplayingMenuOrDialog()
    {
    return CALL_ORIG IsDisplayingMenuOrDialog();
    } 

#undef CALL_ORIG

// End of file
