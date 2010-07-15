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
*      Implementation of CBrowserAppUi
*
*
*/

// INCLUDES
#include <browser_platform_variant.hrh>
#include <BrowserNG.rsg>
#include <uri16.h>
#include <uri8.h>
#include <uriutils.h>
#include <ApUtils.h>
#include <StringLoader.h>
#include <e32def.h>
#include <aplistitemlist.h>
#include <aplistitem.h>
#include <apparc.h>
#include <favouritesfile.h>
#include <aknmessagequerydialog.h>
#include <FeatMgr.h>
#include <internetconnectionmanager.h>
#include <APSettingsHandlerUi.h>
#include <UriUtilsCommon.h>
#include <aknnavi.h>
#include <AiwGenericParam.h>
#include <AknDef.h>
#include <DocumentHandler.h>
#include <SysUtil.h>

#ifdef BRDO_APP_GALLERY_SUPPORTED_FF
#include <MGXFileManagerFactory.h>
#include <CMGXFileManager.h>
#endif

#include <browserplugininterface.h>
#include <oommonitorplugin.h>

#include <browseroverriddensettings.h>
#include "BrowserLauncherService.h"

#include "cookiemanagerclient.h"

#ifdef __SERIES60_HELP
#include <hlplch.h>
#endif //__SERIES60_HELP

#include "BrowserAppDocument.h"
#include <downloadedcontenthandler.h>
#include "BrowserBmOTABinSender.h"
#include "BrowserCommandLineParser.h"
#include "BrowserApplication.h"
#include "BrowserContentView.h"
#include "BrowserContentViewContainer.h"
#include "BrowserContentViewToolbar.h"
#include "BrowserBookmarksView.h"
#include "SettingsView.h"
#include "BrowserWindowSelectionView.h"
#include "BrowserInitialView.h"
#include "CommonConstants.h"
#include "BrowserDialogs.h"
#include "BrowserDisplay.h"
#include "BrowserCommsModel.h"
#include "BrowserUtil.h"
#include "BrowserPreferences.h"
#include "SessionAndSecurity.h"
#include "BrowserUIVariant.hrh"
#include "BrowserWindowQueue.h"
#include "Logger.h"
#include <data_caging_path_literals.hrh>

#include <brctldefs.h>
#include <browserdialogsprovider.h>
#include "BrowserSoftkeysObserver.h"
#include "BrowserLoadObserver.h"
#include "BrowserSpecialLoadObserver.h"
#include "BrowserAsyncExit.h"
#include "CommonConstants.h"


// Dialogs Provider
#include <browserdialogsproviderobserver.h>
#include <brctlinterface.h>
#include <browserdialogsprovider.h>

// Multiple Windows
#include "BrowserPopupEngine.h"
#include "BrowserDialogsProviderProxy.h"
#include "BrowserWindow.h"
#include "BrowserWindowManager.h"
#include "AknInfoPopupNoteController.h"

#include "BrowserAppUi.h"

#include "BrowserPushMtmObserver.h"

#ifdef BRDO_IAD_UPDATE_ENABLED_FF
#include <iaupdate.h>
#include <iaupdateparameters.h>
#include <iaupdateresult.h>
#endif

//CONSTANTS
const TUint KBookmarkId = 1;
const TUint KUrlId = 4;
_LIT8 ( KLongZeroIdString, "5" );
const TUint KFolderId = 6;
const TUint KLaunchFeeds = 7;

const TInt KMaxNumOfOpenedWindows = 5;
const TInt KMinNumOfOpenedWindows = 2; // must allow at least 2 windows for most use cases

const TUint KDot('.');
const TUint KSlash('/');
_LIT( KDefaultSchema, "http://" );
const TInt KDefaultSchemaLength = 7;

const TInt KMinimumCDriveDiskSpace = 512 * 1024;

const TInt KRetryConnectivityTimeout( 2*1000*1000 ); // 2 seconds

#ifdef BRDO_IAD_UPDATE_ENABLED_FF
const TUint KBrowser8xUID = 0x200267CC;
_LIT( KUpdateFileName, "lastupdatechecked.txt" );
const TInt64 KMaxTimeToPostponeUpdate = 604800000000;
#endif

//Following array stores Uids for external applications. 
//This can be appended. This is used in HandleMessageL to enable Single Window browsing.
static TInt mArrayOfExternalAppUid[] = { 0x2001f3a9, 0x200159D0};
// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBrowserAppUi::CBrowserAppUi()
// -----------------------------------------------------------------------------
//
CBrowserAppUi::CBrowserAppUi():
iCalledFromAnotherApp( EFalse ),
iIsForeground( EFalse ),
iLastViewId( KUidBrowserNullViewId ),
iExitInProgress( EFalse ),
iShutdownRequested( EFalse ),
iParametrizedLaunchInProgress( 0 ),
iExitFromEmbeddedMode( EFalse ),
iLongZeroPressed( EFalse ),
iStartedUp( EFalse ),
iFeatureManager( EFalse ),
iUserExit( EFalse ),
iPgNotFound( EFalse ),
iOverriddenLaunchContextId( EBrowserContextIdNormal ),
iBrowserAlreadyRunning (EFalse),
iSameWinApp( EFalse ),
iFeedsClientUtilities( 0 )
    {
    iViewToBeActivatedIfNeeded.iUid = 0;
    iViewToReturnOnClose.iUid = 0;
	}

// -----------------------------------------------------------------------------
// CBrowserAppUi::~CBrowserAppUi()
// -----------------------------------------------------------------------------
//
CBrowserAppUi::~CBrowserAppUi()
    {
    LOG_ENTERFN("CBrowserAppUi::~CBrowserAppUi");

#ifdef BRDO_IAD_UPDATE_ENABLED_FF
    delete iDelayedUpdate;
    iFs.Close();
#endif
    
    SetExitInProgress( ETrue );
    if(iBrowserAsyncExit)
    	{
    	iBrowserAsyncExit->Cancel();
    	}
    if(iIdle)
    	{
    	iIdle->Cancel();
    	}


    if( iFeatureManager )
        {
        FeatureManager::UnInitializeLib();
        iFeatureManager = EFalse;
        }

    //
    delete iPushMtmObserver;

#ifdef __RSS_FEEDS
    delete iFeedsClientUtilities;
    LOG_WRITE( "iFeedsClientUtilities deleted" );
#endif //__RSS_FEEDS

    delete iPopupEngine;
    LOG_WRITE(" iPopupEngine deleted.");

    // set user exit for 'save launch param' command handling
    if ( iWindowManager )
        {
        iWindowManager->SetUserExit( iUserExit );
        }
    delete iWindowManager;
    LOG_WRITE( " iWindowManager deleted" );
    
#ifdef BRDO_OCC_ENABLED_FF    
    if(iRetryConnectivity)
    	iRetryConnectivity->Cancel();
    	
    delete iRetryConnectivity;
    iRetryConnectivity = NULL;
#endif    

    // Delete the inetconman after deleting window manager
    CInternetConnectionManager* inetconman = (CInternetConnectionManager*)iConnection;
    delete inetconman;
    LOG_WRITE( " inetconman deleted" );
    delete iConnStageNotifier;
    LOG_WRITE( " iConnStageNotifier deleted" );

    delete iPreferences;
    LOG_WRITE( " iPreferences deleted" );
    delete iCommsModel;
    LOG_WRITE( " iCommsModel deleted" );

    delete iLateSendUi;
	LOG_WRITE( " iLateSendUi deleted" );

    delete iSender;
    LOG_WRITE( " iSender deleted" );

    delete iIdle;
    LOG_WRITE( " iIdle deleted" );

	delete iRecentUrlStore;
	LOG_WRITE( " iRecentUrlStore deleted" );

    if ( iDoorObserver )
      {
        iDoorObserver->NotifyExit(MApaEmbeddedDocObserver::ENoChanges);
        LOG_WRITE( " NotifyExit deleted" );
      }
    delete iDialogsProvider;
    LOG_WRITE( " iDialogsProvider deleted" );

	TBool isStandAlone = !IsEmbeddedModeOn();
	LOG_WRITE_FORMAT( " isStandAlone: %d", isStandAlone );

    delete iBrowserAsyncExit;
    LOG_WRITE( " iBrowserAsyncExit deleted" );

    iFavouritesSess.Close();
    LOG_WRITE( " iFavouritesSess.Close() deleted" );
#ifdef BRDO_IAD_UPDATE_ENABLED_FF
    CleanUpdateParams(); 
#endif
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::ConstructL()
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::ConstructL()
    {
    LOG_CREATE;
    LOG_ENTERFN( "CBrowserAppUi::ConstructL" );
PERFLOG_CREATE;
PERFLOG_LOCAL_INIT;
PERFLOG_STOPWATCH_START;


#ifdef BRDO_SINGLE_CLICK_ENABLED_FF
    BaseConstructL( EAknEnableSkin | EAknEnableMSK | EAknSingleClickCompatible);
#else 
    BaseConstructL( EAknEnableSkin | EAknEnableMSK );
#endif
    if ( !IsEmbeddedModeOn( ) )
    	{
#ifdef BRDO_PERF_IMPROVEMENTS_ENABLED_FF     
        InitBookmarksL();
#else
        InitBrowserL();
#endif        
    	LOG_WRITE( "Browser started standalone" );
    	}
	else
		{
		iStartedUp = EFalse;
		LOG_WRITE( "Browser started embedded" );
		}        
PERFLOG_STOP_WRITE("BrowserUI::ConstructL");

    }
#ifdef BRDO_IAD_UPDATE_ENABLED_FF
// ---------------------------------------------------------
// CBrowserAppUi::CheckUpdatesL
// ---------------------------------------------------------
void CBrowserAppUi::CheckUpdatesL()
    {
    LOG_ENTERFN("CBrowserAppUi::CheckUpdatesL");
    LOG_WRITE( "CBrowserAppUi::CheckUpdatesL() entering" );
    User::LeaveIfError(iFs.Connect());
    if ( FeatureManager::FeatureSupported( KFeatureIdIAUpdate ) )
        {
        LOG_WRITE( "CBrowserAppUi::CheckUpdatesL() IAD Update supported" );
        TRAP_IGNORE( iUpdate = CIAUpdate::NewL( *this ) );
        LOG_WRITE( "CBrowserAppUi::CheckUpdatesL() IAD Update Client Created" );
        if ( iUpdate )
            {
            LOG_WRITE( "CBrowserAppUi::CheckUpdatesL() creating IAD Update paramentes" );
            iParameters = CIAUpdateParameters::NewL();
            // Search for updates using SIS package UID
            iParameters->SetUid( TUid::Uid( KBrowser8xUID ) );
            //check the updates
            iUpdate->CheckUpdates( *iParameters );
            }
        }
    LOG_WRITE( "CBrowserAppUi::CheckUpdatesL() exiting" );
    }

// ---------------------------------------------------------
// CBrowserAppUi::CheckUpdatesComplete
// rest of the details commented in the header
// ---------------------------------------------------------
//
void CBrowserAppUi::CheckUpdatesComplete( TInt aErrorCode, TInt aAvailableUpdates )
    {
    LOG_ENTERFN("CBrowserAppUi::CheckUpdatesComplete");
    LOG_WRITE( "CBrowserAppUi::CheckUpdatesComplete - Entry" );

    TBool result = EFalse;
    TBool showDialog = EFalse;

    if ( aErrorCode == KErrNone )
        {
		if ( aAvailableUpdates > 0 )
            {
            LOG_WRITE( "CBrowserAppUi::CheckUpdatesComplete - update available" );
            //Check if the file is available in folder or not
            if(CheckUpdateFileAvailable())
                {
                LOG_WRITE( "CBrowserAppUi::CheckUpdatesComplete - update file available" );
                TTime timenow;
                timenow.HomeTime();
                TInt64 time = timenow.Int64();
                TInt64 dataValue = ReadUpdateFile();
                //If the diference of the current time and the Previous Check time is more than 1 Week
                //then show the dialog
                if((time - dataValue)>KMaxTimeToPostponeUpdate)
                    {
                    LOG_WRITE( "CBrowserAppUi::CheckUpdatesComplete - diference of the current time and the time available in th file is more than 7 days" );
                    showDialog = ETrue;
                    }
                }
            else
                {
                LOG_WRITE( "CBrowserAppUi::CheckUpdatesComplete - update file is not available" );
                showDialog = ETrue;
                }
                    
            if(showDialog)
                {
                HBufC* message = StringLoader::LoadLC(R_INSTALL_ADDON_BROWSER);
                HBufC* lsc_now = StringLoader::LoadLC(R_INSTALL_BROWSER_NOW);
                HBufC* rsc_later = StringLoader::LoadLC(R_INSTALL_BROWSER_LATER);
                
                TRAPD(err, result = iDialogsProvider->DialogConfirmL(_L(""),
                *message,
                *lsc_now,
                *rsc_later));
                                   
                CleanupStack::PopAndDestroy(3); //message, lsc_now, rsc_later
                
                if (err != KErrNone)
                    {
                    return ;
                    }
                if ( result )  //  user selected NOW
                    {
                    LOG_WRITE( "CBrowserAppUi::CheckUpdatesComplete - if file exists, just delete it." );
                    // if file exists, just delete it.
                    DeleteUpdateFile();
                    iUpdate->ShowUpdates( *iParameters );
                    }
                if ( !result )  // user selected LATER
                    {
                    LOG_WRITE( "CBrowserAppUi::CheckUpdatesComplete - create the file and store the current time." );
                    //create the file and store the current time.
                    WriteUpdateFile();
                    }
                }
            LOG_WRITE( "CBrowserAppUi::CheckUpdatesComplete - update available" );
            }
        else
            {
            LOG_WRITE( "CBrowserAppUi::CheckUpdatesComplete - no update available" );
            // The answer was 'Later'. CIAUpdate object could be deleted
            CleanUpdateParams();
            }
        }
    LOG_WRITE( "CBrowserAppUi::CheckUpdatesComplete - Exit" );
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::CleanUpdateParams
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::CleanUpdateParams()
    {
    LOG_ENTERFN("CBrowserAppUi::CleanUpdateParams");
    LOG_WRITE( "CBrowserAppUi::CleanUpdateParams() entering" );
    if(iUpdate)
        {
        delete iUpdate;
        iUpdate = NULL;
        }
    if(iParameters)
        {
        delete iParameters;
        iParameters = NULL;
        }
    LOG_WRITE( "CBrowserAppUi::CleanUpdateParams() exiting" );
    }

// ---------------------------------------------------------
// CBrowserAppUi::UpdateComplete
// rest of the details commented in the header
// ---------------------------------------------------------
//
void CBrowserAppUi::UpdateComplete( TInt aErrorCode, CIAUpdateResult* aResult )
    {
    LOG_ENTERFN("CBrowserAppUi::UpdateComplete");
    LOG_WRITE( "CBrowserAppUi::UpdateComplete - Entry" );
    delete aResult; // Ownership was transferred, so this must be deleted by the client
    CleanUpdateParams();
    LOG_WRITE( "CBrowserAppUi::UpdateComplete - Exit" );
    }
#endif


// -----------------------------------------------------------------------------
// CBrowserAppUi::InitBookmarksL()
// Initialize only bookmarks view related dependencies here.
// Note - Do not add unnecessary code here, it increases startup time for bookmarks view.
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::InitBookmarksL()
    {  
    //New constructor that just replaces the default primary storage size with this one.
    iRecentUrlStore = CRecentUrlStore::NewL();

    // Init CommsModel
    iCommsModel = CBrowserCommsModel::NewL();

    // check if it can be delayed ??
#ifdef BRDO_OCC_ENABLED_FF
    iConnection = CInternetConnectionManager::NewL( &iCommsModel->CommsDb(), ETrue );
#else
    iConnection = CInternetConnectionManager::NewL( &iCommsModel->CommsDb(), EFalse );
#endif    

    // Creating object to hold application settings
    CBrowserAppDocument* doc = STATIC_CAST(CBrowserAppDocument*, Document());    
    iPreferences = CBrowserPreferences::NewL( *iCommsModel, *this, doc->GetOverriddenSettings());
    
    // Create bookmarkview
     CBrowserBookmarksView* bookmarksView = NULL;
     TInt folderUid = doc->GetFolderToOpen();
     if ( IsEmbeddedModeOn() && folderUid!= KFavouritesRootUid)
         {
         bookmarksView = CBrowserBookmarksView::NewLC( *this, *iRecentUrlStore, folderUid );
         }
     else
         {
         bookmarksView = CBrowserBookmarksView::NewLC( *this, *iRecentUrlStore );
         }

     iBookmarksView = bookmarksView;
     AddViewL( bookmarksView );  // transfer ownership to CAknViewAppUi    
     CleanupStack::Pop(); // bookmarksView	           
    }


// -----------------------------------------------------------------------------
// CBrowserAppUi::CompleteDelayedInit()
// Delayed (async) init callback. This method can be invoked explicitly in case
// some early startup cases fail if Browser has not initialized fully. No harm
// if called multiple times since there is check in the beginning of thsi function.
// -----------------------------------------------------------------------------
//
TBool CBrowserAppUi::CompleteDelayedInit()
    { 
    // Should not be called for other that 9.2 onward devices
#ifdef BRDO_PERF_IMPROVEMENTS_ENABLED_FF
    if ( iStartedUp )
        return EFalse; // no need to re-invoke automatically
    // complete initialization
    TRAP_IGNORE(DelayedInitL());
    
#ifdef BRDO_IAD_UPDATE_ENABLED_FF
    // complete the IAD check asynchronously
    iDelayedUpdate = CIdle::NewL( CActive::EPriorityIdle );
    iDelayedUpdate->Start(TCallBack( CompleteIADUpdateCallback, this ));
#endif    
#endif    
    
    return EFalse; // no need to re-invoke automatically
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::DelayedInitL()
// Delayed (Async) initialization - whatever remains after InitBookmarksL(), do it here. 
// Note: - Do not add unnecessary code here, it increases startup time for contenview.
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::DelayedInitL()
    {
    LOG_ENTERFN("CBrowserAppUi::DelayedInitL");
    // Check for ciritical disk space
    RFs fs;
    User::LeaveIfError(fs.Connect());
    TInt drive( EDriveC );
    TBool isSpace( EFalse );
    TInt err( KErrNone );
    TRAP( err, isSpace = !SysUtil::DiskSpaceBelowCriticalLevelL(&fs, KMinimumCDriveDiskSpace, drive ));
    fs.Close();
    if (!isSpace)  User::Leave(KErrDiskFull);

    // Create Favengine session
    User::LeaveIfError( iFavouritesSess.Connect() );
        
    // Init FeatureManager
    FeatureManager::InitializeLibL();
    iFeatureManager = ETrue;
    
    // check flash present
    iFlashPresent = CheckFlashPresent();    

    // this is required, browser's connection oberver should be hit first.
	// (incase of netscape plgins, transactions will be closed.)
    iConnStageNotifier = CConnectionStageNotifierWCB::NewL();    
    iConnStageNotifier->SetPriority(CActive::EPriorityHigh);

    // Starts a background processing, so it must be started early, to get
    // finished before the first send operation! Or it must be synchronized!
    iLateSendUi  = CIdle::NewL( CActive::EPriorityIdle );
    iLateSendUi ->Start( TCallBack( DelayedSendUiConstructL, this ) );
    
    iHTTPSecurityIndicatorSupressed = iPreferences->HttpSecurityWarningsStatSupressed();
    
    // set AP to be a default one (for Push messages)
    SetRequestedAP( Preferences().DefaultAccessPoint() );
    
    // Create ContentView
    TRect rect = ClientRect();
    CBrowserContentView* contentView = CBrowserContentView::NewLC( *this, rect );
    AddViewL( contentView ); // transfer ownership to CAknViewAppUi
    CleanupStack::Pop(); // contentView
    
    // proxy will handle dialog events through load observer
    iDialogsProvider = CBrowserDialogsProvider::NewL( NULL);

#ifdef __RSS_FEEDS
    iFeedsClientUtilities = CFeedsClientUtilities::NewL( *this, *this );
    BROWSER_LOG( ( _L("Feeds up.") ) );
#endif //__RSS_FEEDS
    
    // Is Multiple Window feature suported?
    if ( Preferences().UiLocalFeatureSupported( KBrowserMultipleWindows ) )
        {
        if (Preferences().UiLocalFeatureSupported( KBrowserMinimalMultipleWindows ))//midrange
            {
            iWindowManager = CBrowserWindowManager::NewL( *this, *contentView, KMinNumOfOpenedWindows );
            }
        else
            {
            iWindowManager = CBrowserWindowManager::NewL( *this, *contentView, KMaxNumOfOpenedWindows );
            }
        LOG_WRITE_FORMAT("WindowManager Up. Max windows number. %d", KMaxNumOfOpenedWindows );            
        }
    else
        {
        iWindowManager = CBrowserWindowManager::NewL( *this, *contentView, KMinNumOfOpenedWindows );
        BROWSER_LOG( ( _L( "WindowManager Up. MWs not supported." ) ) );
        }
    
    contentView->SetZoomLevelL();
    BrCtlInterface().AddLoadEventObserverL(iBookmarksView);        
    
    // create settings view
    CBrowserSettingsView* settingsView = CBrowserSettingsView::NewLC( *this );
    AddViewL( settingsView );   // transfer ownership to CAknViewAppUi
    CleanupStack::Pop(); // settingsView
    BROWSER_LOG( ( _L( "SettingsView up" ) ) );

    // window selection view
    CBrowserWindowSelectionView* windowSelectionView = CBrowserWindowSelectionView::NewLC( *this );
    AddViewL( windowSelectionView );   // transfer ownership to CAknViewAppUi
    CleanupStack::Pop(); // windowSelectionView
    BROWSER_LOG( ( _L( "windowSelectionView up" ) ) );
    
    // Create asyncronous object to call when exit requires it.
    iBrowserAsyncExit = CBrowserAsyncExit::NewL( this );
    iIdle = CIdle::NewL( CActive::EPriorityIdle );
    
    iPushMtmObserver = CBrowserPushMtmObserver::NewL( this );
    iPushMtmObserver->StartObserver();
    
#ifdef BRDO_OCC_ENABLED_FF
    iRetryConnectivity = CPeriodic::NewL(CActive::EPriorityStandard);
#endif
    
    // Create two Panes of CBrowserContentViewContainer
    CBrowserGotoPane* gotoPane = CBrowserGotoPane::NewL( contentView->Container(),
             EMbmAvkonQgn_indi_find_goto,
             EMbmAvkonQgn_indi_find_goto_mask,
             ETrue,
             contentView );
                      
    // Create the find pane with magnifier glass icon, and
    // without adaptive popup list...
    CBrowserGotoPane* findKeywordPane = CBrowserGotoPane::NewL( contentView->Container(),
             EMbmAvkonQgn_indi_find_glass,
             EMbmAvkonQgn_indi_find_glass_mask,
             EFalse,
             contentView,
             ETrue );
    contentView->Container()->SetGotoPane(gotoPane);
    contentView->Container()->SetFindKeywordPane(findKeywordPane);
    //contentView->Container()->SetRect( rect ); // causes suncRepaint
    contentView->Container()->GotoPane()->SetGPObserver(contentView);
    contentView->Container()->FindKeywordPane()->SetGPObserver(contentView);
    contentView->Container()->FindKeywordPane()->SetOrdinalPosition( 0 );
    contentView->Container()->GotoPane()->SetOrdinalPosition( 0 );
    
    iStartedUp = ETrue;           
    iSecureSiteVisited = EFalse;
    
#ifdef BRDO_PERF_IMPROVEMENTS_ENABLED_FF
    if(LastActiveViewId() == KUidBrowserBookmarksViewId)
        {
        iBookmarksView->CheckForDownloads();
        iBookmarksView->UpdateFavIconsL();
        }
#endif    
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::InitBrowserL() - THIS METHOD IS NOT USED FOR NORMAL STARTUP
// This method is just for supporting Browser initialization if launched in Embedded mode
// Normal initialization if split in BookmarksInit() and DelayedInit(). iStartedUp is FALSE
// if BRowser has not initialized or partially initialized.
// NOTE: DO NOT ADD ANY CODE HERE. IT IS JUST A WRAPPER.
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::InitBrowserL()
    {
    // Bookmarks initialization
    InitBookmarksL();
    
    // 2nd part of initialization
    DelayedInitL();
    
#ifdef BRDO_IAD_UPDATE_ENABLED_FF
    // complete the IAD check asynchronously
    if(!IsEmbeddedModeOn())
        {
        iDelayedUpdate = CIdle::NewL( CActive::EPriorityIdle );
        iDelayedUpdate->Start(TCallBack( CompleteIADUpdateCallback, this ));
        }
#endif
    }


// -----------------------------------------------------------------------------
// CBrowserAppUi::ProcessCommandL(TInt aCommand)
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::ProcessCommandL(TInt aCommand)
    {
    MEikAppUiFactory* appUiFactory = (iEikonEnv)->AppUiFactory();
    if (appUiFactory->MenuBar())
        {
        StopDisplayingMenuBar();
        }
    CAknViewAppUi::ProcessCommandL(aCommand);
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::HandleCommandL()
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::HandleCommandL(TInt aCommand)
    {
    // Default handling for all the thingies.
    switch (aCommand)
        {
        case EEikCmdExit:
			{
			ExitBrowser ( IsAppShutterActive() );
			break;
			}
    case EAknSoftkeyExit://  closes the whole application chain if embedded browser
    case EAknCmdExit:
			{
			if ( IsEmbeddedModeOn() )
				{
				CAknEnv::RunAppShutter();
				break;
				}
			else
				{
				ExitBrowser ( ETrue ) ;
				break;
				}
			}
		//Fix for TSW error ICHV-75UFKZ	
		case EWmlCmdUserExit://closes the current browser instance only
		case EWmlCmdCloseBrowser: 
			{
			ExitBrowser ( ETrue );
			break;
			}
        case EWmlCmdChangeConnection :
            {
            ChangeConnectionL();
            break;
            }
        case EWmlCmdPreferences:
            {
            SetViewToBeActivatedIfNeededL( KUidBrowserSettingsViewId );
            break;
            }
        case EWmlCmdPreferencesToolbar:
            {
            SetViewToBeActivatedIfNeededL( KUidBrowserSettingsViewId, KUidSettingsGotoToolbarGroup.iUid );
            break;
            }            
        case EWmlCmdFavourites:
            {
            SetViewToBeActivatedIfNeededL( KUidBrowserBookmarksViewId );
            break;
            }
        case EWmlCmdSearchWeb:
            {
            LoadSearchPageL();
            break;
            }
        case EWmlCmdReload:
            {
			BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandReload +
												(TInt)TBrCtlDefs::ECommandIdBase );
            break;
            }
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF			
        case EWmlCmdSetAsHomePage:
            {
            ContentView()->HandleCommandL(EWmlCmdSetAsHomePage);
            break;
            }
#endif			
        case EAknSoftkeyBack:
        case EWmlCmdBack:
            {
            HistoryLoadPrevious();
            break;
            }
        case EWmlCmdBackToFeedsView:
            {
   			//Note: iPreviousView in this case will always be one of the Feeds view.
   			//In general iPreviousView holds the UID of the previous view.
   			SetViewToBeActivatedIfNeededL( iPreviousView );
            break;
            }
        case EWmlCmdClearTheCache:
            {
            ClearTheCacheL(ETrue, ETrue);
            break;
            }
        case EWmlCmdDeleteCookies:
            {
            DeleteCookiesL();
            TBrowserDialogs::InfoNoteL(
                R_BROWSER_OK_NOTE, R_QTN_BROWSER_NOTE_COOKIES_DELETED );
            break;
            }

        case EWmlCmdClearAllPrivacy:
            {
            ClearAllPrivacyL();
            break;
            }

        case EWmlCmdClearHistory:
            {
            ClearHistoryWithPromptL();
            break;
            }

        case EWmlCmdClearFormAndPasswd:
            {
            ClearFormAndPasswdDataL(ETrue);
            break;
            }

        case EWmlCmdLaunchHomePage:
            {
            if (!IsLaunchHomePageDimmedL())
                {
                LaunchHomePageL( /* EFalse */ );
                }
            break;
            }
        case EWmlCmdDisconnect :
            {
            DisconnectL();
            TBrowserDialogs::InfoNoteL(
									R_BROWSER_INFO_NOTE, R_WML_DISCONNECT );
            break;
            }
        case EWmlCmdCancelFetch:
            {
            if (SpecialLoadObserver().IsConnectionStarted()) // If Connection request is in processing 
            {                                                // try to Cancel.
                SpecialLoadObserver().CancelConnection();
                Connection().Disconnect();
            }
            else
            {
                CancelFetch( ETrue );
            }
            break;
            }
        case EWmlCmdPageInfo:
            {
            CSessionAndSecurity* security = CSessionAndSecurity::NewLC(*this);
            security->ShowPageInfoL();
            CleanupStack::PopAndDestroy(); // security
            break;
            }
        case EWmlCmdInfoSession:
            {
            CSessionAndSecurity* session = CSessionAndSecurity::NewLC(*this);
            session->ShowSessionInfoL();
            CleanupStack::PopAndDestroy();  // session
            break;
            }
        case EWmlCmdInfoSecurity:
            {
            CSessionAndSecurity* security = CSessionAndSecurity::NewLC(*this);
            security->ShowSecurityInfoL();
            CleanupStack::PopAndDestroy();  // security
            break;
            }
        // browser engine specific command
#ifdef __SERIES60_HELP

        case EAknCmdHelp:
            {
            HlpLauncher::LaunchHelpApplicationL
                ( iEikonEnv->WsSession(), AppHelpContextL() );
            break;
            }

#endif //__SERIES60_HELP

		case EWmlCmdForward:
			{
            HistoryLoadNext();
			break;
			}

        case EWmlCmdRotateDisplay:
            {
            if ( Orientation() == EAppUiOrientationPortrait )
                {
                SetOrientationL(EAppUiOrientationLandscape);
                }
            else if (Orientation() == EAppUiOrientationLandscape)
                {
                SetOrientationL(EAppUiOrientationPortrait);
                }
            else // EAppUiOrientationUnspecified
                {
                TRect rect = ApplicationRect();
                TInt width = rect.Width();
                TInt height = rect.Height();

                if (width > height)
                    {
                    SetOrientationL(EAppUiOrientationPortrait);
                    }
                else
                    {
                    SetOrientationL(EAppUiOrientationLandscape);
                    }
                }
             if(ContentView()->KeymapIsUp())
             	{             	          
             	ContentView()->RedrawKeymap();
             	}
            break;
            }

        case EWmlCmdAboutProduct:
            {
            // get version info from browser control and display it
            ShowNameAndVersionL();
            break;
            }
		case EWmlCmdProductUpdate:
            {
    			/*if(!iAppUpdate)
				    {
				    iAppUpdate = CBrowserAppUpdate::NewL();
				    }
			    iAppUpdate->CheckUpdates();*/            
            break;
            }

        //=====================================================================
        // Multiple Windows Support
        //
    	case EWmlCmdSwitchWindow:
            {
            if(WindowMgr().WindowCount() > 1)
	            {
#ifdef BRDO_MULTITOUCH_ENABLED_FF
	            // use switch window tab view if pageoverview bitmaps are available
	            if (Preferences().UiLocalFeatureSupported( KBrowserGraphicalPage ) 
	                    || Preferences().UiLocalFeatureSupported( KBrowserGraphicalHistory ))
#else	                
                // use switch window tab view if pageoverview bitmaps are available
                if (Preferences().UiLocalFeatureSupported( KBrowserGraphicalPage ))
#endif                    
	            	{
	            	SetViewToBeActivatedIfNeededL( KUidBrowserWindowSelectionViewId );
	            	}
	            else
	            	{
	            	SwitchWindowL();
		            }            	
            	}
            break;
            }
    	case EWmlCmdCloseWindow:
    		{
    		if ( IsEmbeddedInOperatorMenu() )
    			{
    			ExitBrowser( ETrue );
    			}
    		else
				{
    			CloseWindowL();
				}
    		break;
    		}
    	case EWmlCmdAllowPopups:
        	{
        	// Add url to whitelist
        	HBufC* url( WindowMgr().CurrentWindow()->BrCtlInterface().
                PageInfoLC( TBrCtlDefs::EPageInfoUrl ) );
            PopupEngine().AddUrlToWhiteListL( *url );
            CleanupStack::PopAndDestroy( url );
    		break;
	    	}
    	case EWmlCmdBlockPopups:
        	{
        	// Remove from whitelist
        	HBufC* url( WindowMgr().CurrentWindow()->BrCtlInterface().
                PageInfoLC( TBrCtlDefs::EPageInfoUrl ) );
        	PopupEngine().RemoveUrlFromWhiteListL( *url );
            CleanupStack::PopAndDestroy( url );
        	break;
        	}
        case EWmlCmdShowAnchorHref:
            {
			BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandIdBase + (TInt)TBrCtlDefs::ECommandShowAnchorHref );
            break;
            }
        case EWmlCmdSaveToGallery:
            {
            SaveFocusedImageToGalleryL();
            break;
            }
        case EWmlCmdOpenToViewer:
            {
            TBrCtlImageCarrier* imageCarrier = BrCtlInterface().FocusedImageLC();
            if (imageCarrier)
                {
                CArrayFixFlat<TBrCtlImageCarrier>* imageArray = new( ELeave ) CArrayFixFlat<TBrCtlImageCarrier>(1);
                CleanupStack::PushL(imageArray);
                imageArray->AppendL(*imageCarrier);
                iDialogsProvider->DialogDisplayPageImagesL( *imageArray );
                CleanupStack::PopAndDestroy(2); // imageArray, imageCarrier
                }
            break;
            }
        case EWmlCmdSaveUrlAsBookmark:
            {
            HBufC* url = BrCtlInterface().PageInfoLC(TBrCtlDefs::EPageInfoFocusedNodeUrl);
            ContentView()->AddNewBookmarkL(EFalse, url);
            CleanupStack::PopAndDestroy(); // url
            break;
            }
        case EWmlCmdSmartLinkMakeCall:
            {
			BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandIdBase + (TInt)TBrCtlDefs::ECommandSmartLinkMakeCall );
            break;
            }
        case EWmlCmdSmartLinkSendEmail:
            {
			BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandIdBase + (TInt)TBrCtlDefs::ECommandSmartLinkSendEmail );
            break;
            }
        case EWmlCmdSmartLinkSendSms:
            {
			BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandIdBase + (TInt)TBrCtlDefs::ECommandSmartLinkSendMessage );
            break;
            }
        case EWmlCmdSmartLinkAddToContacts:
            {
			BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandIdBase + (TInt)TBrCtlDefs::ECommandSmartLinkAddToPhoneBook );
            break;
            }
        case EWmlCmdLoadFocusedImage:
            {
			BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandIdBase + (TInt)TBrCtlDefs::ECommandLoadFocusedImage );
            break;
            }
        case EWmlCmdLoadImagesOnly:
            {
            iPreferences->SetAutoLoadContentL(EWmlSettingsAutoloadImagesNoFlash);
            break;
            }
        case EWmlCmdLoadImagesAndFlash:
            {
            iPreferences->SetAutoLoadContentL(EWmlSettingsAutoloadAll);
            break;
            }
        case EWmlCmdOneStepBack:
            {
			BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandIdBase + (TInt)TBrCtlDefs::ECommandOneStepBack );
            break;
            }
        case EEikCmdEditPaste:
            {
            TKeyEvent keyEvent;
            keyEvent.iCode = EKeyF18;  //member of TKeyCode	
            keyEvent.iScanCode = EEikCmdEditPaste;	
            keyEvent.iModifiers = EModifierCtrl;
            keyEvent.iRepeats = 0;
            TRAP_IGNORE( BrCtlInterface().OfferKeyEventL(keyEvent, EEventKey));
            }
            break;
	    //=====================================================================
        default:
            {
            if( iStartedUp )
                {
                // pass to Browser Control
                BrCtlInterface().HandleCommandL( aCommand );
                }
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::ActiveView()
// -----------------------------------------------------------------------------
//
CBrowserViewBase* CBrowserAppUi::ActiveView()
    {
    TVwsViewId activeViewId;
    TInt err = GetActiveViewId( activeViewId );

    if ( !err )
        {
        return STATIC_CAST( CBrowserViewBase*, View(activeViewId.iViewUid) );
        }
    else
        {
        //We should not panic
        //On 3.0 wk40 it was observed that GetActiveViewId returned with KErrNotFound
        //in case Browser was in the background
        //Symbian error?
        return NULL;
        }
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::SetRequestedAP()
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::SetRequestedAP( TInt aAp )
    {
    iRequestedAp = aAp;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::IsPageLoaded()
// -----------------------------------------------------------------------------
//
TBool CBrowserAppUi::IsPageLoaded()
    {
    TBool ret( EFalse );
    TInt winCount( WindowMgr().WindowCount() );

    // at least 2 window -> there is a page loaded
    if( winCount > 1 )
        {
        ret = ETrue;
        }
    // 1 window with a loaded content
    else if( winCount == 1 )
        {
        // This checks to see if a dialog is launched and if that is the case, then
        // the current window is being used, hence returning true.
        if( iDialogsProvider->IsDialogLaunched() )
            {
            ret = ETrue;
            }
        else
            {
            ret = WindowMgr().IsContentExist();
            }
        }
    // else (winCount == 0), no content view -> no page loaded
    return ret;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::Fetching()
// -----------------------------------------------------------------------------
//
TBool CBrowserAppUi::Fetching() const
    {
	// TO DO:
	// use load obs., remove ifetchstate from this class and use from loadobs.
	// iLoadState
    //return ( iFetchState == MFetchObserver::ELoadStart );
	return ( LoadObserver().LoadState() != CBrowserLoadObserver::ELoadStateIdle );
    }
// -----------------------------------------------------------------------------
// CBrowserAppUi::ContentDisplayed()
// -----------------------------------------------------------------------------
//
TBool CBrowserAppUi::ContentDisplayed() const
    {
    return ( LoadObserver().ContentDisplayed() );
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::SetContentDisplayed()
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::SetContentDisplayed(TBool aValue) 
    {
        LoadObserver().SetContentDisplayed( aValue );
    }
// -----------------------------------------------------------------------------
// CBrowserAppUi::FetchBookmarkL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::FetchBookmarkL( TInt aBookmarkUid )
    {
    // complete initialization if not done yet, can happen if user selects
    // a bookmark quickly after launch (within 1 second)
    if ( !iStartedUp )
        CompleteDelayedInit();
        
    SetViewToReturnOnClose( KUidBrowserBookmarksViewId );
    if ( aBookmarkUid == KFavouritesStartPageUid )
        {
        TUint defaultAp = Preferences().DefaultAccessPoint();
        if ( defaultAp != KWmlNoDefaultAccessPoint ) // There is no access point defined
            {
            CApAccessPointItem* apItem = Preferences().AllPreferencesL().iDefaultAPDetails;
            if ( apItem )
                {
                const HBufC* currentHP = apItem->ReadConstLongTextL( EApWapStartPage );
                //TPtrC currentHP(apItem->StartPage()); // Get the start page of the access point.
                TFavouritesWapAp accessPoint;

                FetchL( *currentHP, KNullDesC, KNullDesC, accessPoint,
                    CBrowserLoadObserver::ELoadUrlTypeStartPage ); // Fetch the start page.
				}
            }
        }
    else
        {
        RFavouritesDb db;
        User::LeaveIfError( db.Open( iFavouritesSess, KBrowserBookmarks ) );
        CleanupClosePushL<RFavouritesDb>( db );

        CFavouritesItem* item = CFavouritesItem::NewLC();
        User::LeaveIfError( db.Get( aBookmarkUid, *item ) );

        FetchBookmarkL( *item );
        CleanupStack::PopAndDestroy( 2 );  // item, db
        }
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::FetchBookmarkL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::FetchBookmarkL( const CFavouritesItem& aBookmarkItem )
    {
    // complete initialization if not done yet, can happen if user selects
    // a bookmark quickly after launch (within 1 second)
    if ( !iStartedUp )
        CompleteDelayedInit();
    
    SetViewToReturnOnClose( KUidBrowserBookmarksViewId );
    if ( Util::CheckBookmarkApL( *this, aBookmarkItem.WapAp()) )
        FetchL
            (
            aBookmarkItem.Url() ,
            aBookmarkItem.UserName(),
            aBookmarkItem.Password(),
            aBookmarkItem.WapAp(),
            CBrowserLoadObserver::ELoadUrlTypeOther
            );
    else
        TBrowserDialogs::ErrorNoteL( R_WML_INVALID_AP );
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::SetViewToBeActivatedIfNeededL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::SetViewToBeActivatedIfNeededL( TUid aUid, TInt aMessageId )
    {
	CBrowserAppDocument* doc = STATIC_CAST(CBrowserAppDocument*, Document());

	if((iCurrentView != aUid) && (aUid != KUidBrowserSettingsViewId))
	{
		iPreviousView = iCurrentView;
		iCurrentView = aUid;
	}

	//Check view id validity first
	if ( aUid == KUidBrowserNullViewId )
		{
		SetLastActiveViewId( KUidBrowserBookmarksViewId );
		SetViewToBeActivatedIfNeededL( LastActiveViewId() );
           return;
		}

	if ( ( aUid != KUidBrowserContentViewId ) && 
	   ( LastActiveViewId() == KUidBrowserContentViewId ) )
		{
		ContentView()->SetFullScreenOffL();
		}
	
	if ( iWindowManager && iWindowManager->ContentView()->FullScreenMode() )
	    {
    	if ( aUid == KUidBrowserFeedsFeedViewId )
    	    {
    	    BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandIdBase + 
    	        (TInt)TBrCtlDefs::ECommandLeaveFullscreenBrowsing );
    	    }
    	
    	else  if ( aUid != KUidBrowserFeedsTopicViewId && 
            aUid != KUidBrowserFeedsFolderViewId && 
            aUid != KUidBrowserBookmarksViewId )
            {
            BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandIdBase + 
                (TInt)TBrCtlDefs::ECommandEnterFullscreenBrowsing );
            }
    	}


	if ( iIsForeground )
		{
        if ( aUid == KUidBrowserSettingsViewId )
	        {
            //complete initialisation 
            if( !iStartedUp )
                CompleteDelayedInit();

            CEikStatusPane* sp = STATIC_CAST( CAknAppUi*,
						  CEikonEnv::Static()->EikAppUi() )
			                ->StatusPane();
            sp->DrawNow();
            }

		ActivateLocalViewL(  aUid, TUid::Uid( aMessageId ), KNullDesC8 );
		}
	else
		{
		iViewToBeActivatedIfNeeded = aUid;
		}
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::LastActiveViewId
// -----------------------------------------------------------------------------
//
TUid CBrowserAppUi::LastActiveViewId() const
    {
    return iLastViewId;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::SetLastActiveViewId
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::SetLastActiveViewId( TUid aUid )
    {
LOG_ENTERFN("CBrowserAppUi::SetLastActiveViewId");
    LOG_WRITE_FORMAT(" SetLastActiveViewId: %d", aUid);
    iLastViewId = aUid;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::ExitInProgress
// -----------------------------------------------------------------------------
//
TBool CBrowserAppUi::ExitInProgress() const
    {
    return iExitInProgress;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::ExitInProgress
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::SetExitInProgress( TBool aValue )
    {
    iExitInProgress = aValue;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::IsConnecting
// -----------------------------------------------------------------------------
//
TBool CBrowserAppUi::IsConnecting() const
    {
    return ( LoadObserver().LoadState() != CBrowserLoadObserver::ELoadStateIdle );
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::HandleForegroundEventL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::HandleForegroundEventL( TBool aForeground )
    {
    // Handle this event only if the browser is started up
    iIsForeground = IsForeground();
    if (!iStartedUp)
    	return;

    if( iIsForeground )
    	{
 	    if (iViewToBeActivatedIfNeeded.iUid)
	        {
	        ActivateLocalViewL( iViewToBeActivatedIfNeeded );
	        }
	    else
	        {
	        TVwsViewId activeViewId;
	        if( ( GetActiveViewId( activeViewId ) == KErrNone ) &&	        		
	            (  LastActiveViewId()  != activeViewId.iViewUid ) )
	             {
	             // LastActiveViewId() might return with 0!
	             TRAP_IGNORE( ActivateLocalViewL( LastActiveViewId() ) );
	             }
	        }
    	}
	iViewToBeActivatedIfNeeded.iUid = 0;    
    CAknViewAppUi::HandleForegroundEventL( aForeground );
            
    if( iIsForeground )
        {
        if ( Preferences().FullScreen() == EWmlSettingsFullScreenFullScreen )
            {
            ContentView()->UpdateFullScreenL();
            }

        if( !Fetching() )
            {
            Display().StopProgressAnimationL();
            }
        }

    if (iWindowManager)
        {
        iWindowManager->SendCommandToAllWindowsL( iIsForeground
            ? (TInt)TBrCtlDefs::ECommandAppForeground + (TInt)TBrCtlDefs::ECommandIdBase
            : (TInt)TBrCtlDefs::ECommandAppBackground + (TInt)TBrCtlDefs::ECommandIdBase);
        }

    // if browser comes to foreground and exit is requested,
    // asyncronous exit is started
    if( iIsForeground && iExitFromEmbeddedMode && !IsEmbeddedInOperatorMenu() )
        {
        iBrowserAsyncExit->Start();
        } 
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::FetchL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::FetchL(
						   const TDesC& aUrl,
                           const TDesC& aUsername,
                           const TDesC& aPassword,
                           const TFavouritesWapAp& aAccessPoint,
                           CBrowserLoadObserver::TBrowserLoadUrlType aUrlType )
    {
LOG_ENTERFN("CBrowserAppUi::FetchL");
    
    // complete the initialization if not done yet
    if(!iStartedUp)
        CompleteDelayedInit();
    
    // Let's cancel the previous fetch if any
    if ( Fetching() )
		{
        CancelFetch();
		}
#ifndef __WINS__
//Fix for bookmark specific access point
    if( iConnection && iConnection->Connected() )
       {
       TUint32 passedIap( 0 );
       if ( aAccessPoint.ApId() != KWmlNoDefaultAccessPoint )
           {
           passedIap = Util::IapIdFromWapIdL( *this, aAccessPoint.ApId());
           BROWSER_LOG((_L("CBrowserAppUi::FetchL Passed Iap: %d"), passedIap));
           TUint32 connectedAp = iConnection->CurrentAPId();
           BROWSER_LOG((_L("CBrowserAppUi::FetchL Existing connected Iap: %d"), connectedAp));
           if(passedIap != connectedAp)
               {
               StopConnectionObserving();
               BROWSER_LOG((_L("CBrowserAppUi::FetchL Stopped the connection observation.")));
               DisconnectL();
               BROWSER_LOG((_L("CBrowserAppUi::FetchL Given iap and Connected iap are not same. Disconnected the existing connection.")));
               }
           }

       }
#endif // __WINS__
    // iDoNotRestoreContentFlag = EFalse;

    TInt toPop = 0;
    TUriParser uriParser;
    TInt err = uriParser.Parse(aUrl);
    if ( !err && aUrl.Length() )
        {
        // check url for missing scheme and add default scheme if needed
        HBufC* url = NULL;
        HBufC* resultUrlBuf = NULL;
        //find ://
        TPtrC scheme = uriParser.Extract( EUriScheme );
        // add http:// scheme as default if it is missing
        if( scheme.Length() == 0 )
            { 
            //put the "http://" schema before the string
            url = HBufC::NewLC( aUrl.Length() + KDefaultSchemaLength );
            url->Des().Copy( KDefaultSchema );
            url->Des().Append( aUrl );
            ++toPop;  // url
            resultUrlBuf = CreateFullUrlLC( url->Des(), aUsername, aPassword );
		    LogRequestedPageToRecentUrlL(  url->Des() );
            }
        else
            {
            resultUrlBuf = CreateFullUrlLC( aUrl, aUsername, aPassword );
		    LogRequestedPageToRecentUrlL(  aUrl );
            }
        ++toPop;  // resultUrlBuf
    
    
        if ( aAccessPoint.IsNull() || aAccessPoint.IsDefault() || (!Util::IsValidAPFromParamL( *this, aAccessPoint.ApId() )) )
			{
            if ( (Preferences().AccessPointSelectionMode() == EConnectionMethod) || 
                 (Preferences().AccessPointSelectionMode() == EAlwaysAsk) )
                {
                iRequestedAp = Preferences().DefaultAccessPoint();
                }
            else if ( Preferences().AccessPointSelectionMode() == EDestination ) 
                {
                // for bug fix MLAN-7EKFV4
    			iRequestedAp = KWmlNoDefaultAccessPoint;
                }
            }
		else
			{
			iRequestedAp = aAccessPoint.ApId();
			}
#ifndef __WINSCW__ //On WINS we don't want this dlg to come up
					//as the ConnMan will present us with a selection
        BROWSER_LOG( ( _L( "requestedAp: %d" ), iRequestedAp ) );
        // it's ok to assign in the case of EDest, since CBrCtl doesn't use the apId passed in LoadUrlL()
        TUint32 IAPid = iRequestedAp;
        TUint32 defaultSnap =  iPreferences->DefaultSnapId();
#ifndef BRDO_OCC_ENABLED_FF
        // if no AP or SNAP is defined, define it
        if( (iRequestedAp == KWmlNoDefaultAccessPoint && iPreferences->AccessPointSelectionMode() == EConnectionMethod) || 
            (defaultSnap == KWmlNoDefaultSnapId && iPreferences->AccessPointSelectionMode() == EDestination) )
            {
            StartPreferencesViewL( EShowAlwaysAsk | EShowDestinations | EShowConnectionMethods );
            }
        //Check AP and snap if it is still not defined, set always ask mode this case
        if ( (iRequestedAp == KWmlNoDefaultAccessPoint && iPreferences->AccessPointSelectionMode() == EConnectionMethod) ||
             (defaultSnap == KWmlNoDefaultSnapId && iPreferences->AccessPointSelectionMode() == EDestination) )
            {
            BROWSER_LOG( ( _L( "AP or SNAP was not added set to Always ask mode" ) ) );
            iPreferences->SetAccessPointSelectionModeL(EAlwaysAsk );
            }
        if ( iRequestedAp != KWmlNoDefaultAccessPoint )
            {
    		    BROWSER_LOG( ( _L( "AP added" ) ) );
            IAPid = Util::IapIdFromWapIdL( *this, iRequestedAp );  // Get currently active ap
            BROWSER_LOG( ( _L( "Access point: %d" ), IAPid ) );
            }

#endif // BRDO_OCC_ENABLED_FF
#else	//we can use any numbers here
        // alr: snap on emulator should not exist; use cm mode instead?
		iRequestedAp = 2;
		TUint32 IAPid = 2;
#endif //__WINSCW__

		err = KErrNone;
		BROWSER_LOG( ( _L( "Load the URL" ) ) );
		
		if ( iPreferences->SearchFeature() || iPreferences->ServiceFeature())
		    {
    		iSpecialSchemeinAddress = EFalse;
            
            HBufC* searchScheme = HBufC::NewLC( KMaxHomePgUrlLength );  // cleanupstack
            TPtr searchSchemePtr( searchScheme->Des() );
            ++toPop;
            
            HBufC* serviceScheme = HBufC::NewLC( KMaxHomePgUrlLength );  // cleanupstack
            TPtr serviceSchemePtr( serviceScheme->Des() );
            ++toPop;
            
            iPreferences->GetStringValueL( KBrowserSearchProviderUrl, KMaxHomePgUrlLength, searchSchemePtr );
            iPreferences->GetStringValueL( KBrowserServiceUrl, KMaxHomePgUrlLength, serviceSchemePtr );
                    
            TInt err = uriParser.Parse(searchSchemePtr);
            if (!err)
                searchSchemePtr = uriParser.Extract( EUriScheme );
            
            err = uriParser.Parse(serviceSchemePtr);
            if (!err)
                serviceSchemePtr = uriParser.Extract( EUriScheme );


            if( scheme.Length() != 0 && (!scheme.Compare(serviceSchemePtr) || !scheme.Compare(searchSchemePtr)) )
                {
                iSpecialSchemeinAddress = ETrue;
                TRAP( err, BrCtlInterface().LoadUrlL( resultUrlBuf->Des(), IAPid ) );
                }
            else
                {
                LoadObserver().DoStartLoad( aUrlType );
                TRAP( err, BrCtlInterface().LoadUrlL( resultUrlBuf->Des(), IAPid ) );
                }            
	        }
	    else
	        {
	        LoadObserver().DoStartLoad( aUrlType );
	        BROWSER_LOG( ( _L( "PASSED IAP: %d" ), IAPid ) );
            TRAP( err, BrCtlInterface().LoadUrlL( resultUrlBuf->Des(), IAPid ) );
	        }
        
		CleanupStack::PopAndDestroy(toPop); //resultUrlBuf and url if needed
		BROWSER_LOG( ( _L( "No AP 4 %d" ), err  ) );
		if ( err != KErrNone )
    		{
		    CancelFetch();
	    	}
        }

    BROWSER_LOG( ( _L( "Check the result" ) ) );
	switch ( err )
		{
		case KErrNotFound:
			{
			(*(CBrowserBookmarksView*)View( KUidBrowserBookmarksViewId )).UpdateGotoPaneL();
			iCoeEnv->HandleError( KBrsrMalformedUrl );
			BROWSER_LOG( ( _L( "KErrNotFound" ) ) );
			break;
			}
		case KUriUtilsErrInvalidUri: // return value of TUriParser::Parse
			{
			(*(CBrowserBookmarksView*)View( KUidBrowserBookmarksViewId )).UpdateGotoPaneL();
			iCoeEnv->HandleError( KBrsrUnknownScheme );
			BROWSER_LOG( ( _L( "KUriUtilsErrInvalidUri" ) ) );
			break;
			}
		case KErrNone:
		    {
		    BROWSER_LOG( ( _L( "KErrNone" ) ) );
            ContentView()->ResetPreviousViewFlag();
		    break;
		    }
        case KErrCancel:
            {
            BROWSER_LOG( ( _L( "KErrCancel" ) ) );
            // do nothing, it is already cancelled by CancelFetch().
            break;
            }
        default:
			{
			BROWSER_LOG( ( _L( "default" ) ) );
			User::Leave( err );  // pass error code to caller
			break;
			}
		}
	}

// -----------------------------------------------------------------------------
// CBrowserAppUi::ExitBrowser()
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::ExitBrowser( TBool aUserInitiated )
    {
    LOG_ENTERFN("CBrowserAppUi::ExitBrowser");

    TInt err( KErrNone );
	TBool isStandAlone = !IsEmbeddedModeOn();
	BROWSER_LOG( ( _L( " isStandAlone: %d" ), isStandAlone ) );

	if(isStandAlone && aUserInitiated )
	    {
		if( iStartedUp && !BrCtlInterface().OkToExit() )
		    {
		    return;
		    }
	    }

	//If launch with parameters is in progress, do not exit now
	//as it can lead to synchronisation problems in embedded mode
	if ( iParametrizedLaunchInProgress == 1 )
		{
		BROWSER_LOG( ( _L( " iParametrizedLaunchInProgress" ) ) );
		return;
		}

    // Operator feature - Display confirmation note on exit only if exit is initiated by user.
    // Skip displaying exit confirmation if exit is initiated from fast swap window.
    BROWSER_LOG(  ( _L( " iPreferences:  %d" ), iPreferences ) );
    if ( iPreferences->QueryOnExit() && aUserInitiated )
        {
        BROWSER_LOG( ( _L( " iPreferences->QueryOnExit()" ) ) );
        TBool confirmdialog( EFalse );
        TRAP( err, confirmdialog = TBrowserDialogs::ConfirmQueryYesNoL( R_TEXT_WML_EXIT_CONFIRM ) );
        if ( !confirmdialog )
        	{
            return;
        	}
		}

    //delete attacment from mailer if existing
	CCoeEnv::Static()->FsSession().Delete( KAttachment() );
	
    BROWSER_LOG( ( _L(" iConnection:  %d"), iConnection ) );
    BROWSER_LOG( ( _L(" iExitFromEmbeddedMode:  %d"), iExitFromEmbeddedMode ) );
    // if browser is embedded, should not call Exit(),
    // just delete the object, otherwise leave occurs.
    if( ( IsEmbeddedInOperatorMenu() || IsEmbeddedModeOn() ) &&
            !ExitInProgress() &&
             ((LoadObserver().LoadUrlType() == CBrowserLoadObserver::ELoadUrlTypeEmbeddedBrowserWithUrl) ||
             (LoadObserver().LoadUrlType() == CBrowserLoadObserver::ELoadUrlTypeOther)   ) )
               // ELoadUrlTypeEmbeddedBrowserWithUrl is typical for load via Phonebook, MMS, OperatorMenu
               // ELoadUrlTypeOther is typical via Media download since those are via GotoPane entered urls
        {
        // Simulate an escape key event in order to close any open resources (for example an open dialog)
        TKeyEvent keyEvent;
        keyEvent.iModifiers = 0;
        keyEvent.iRepeats = 0;
        keyEvent.iCode = EKeyEscape;
        keyEvent.iScanCode = EStdKeyEscape;
        TRAP_IGNORE(iCoeEnv->SimulateKeyEventL( keyEvent, EEventKey ));

        // force not to load Exit() when next time call this function,
        // just delete the object
        SetExitFromEmbeddedMode( ETrue );
        SetExitInProgress( ETrue );
        BROWSER_LOG( ( _L( " iBrowserAsyncExit->Start()" ) ) );
        iBrowserAsyncExit->Start();
        }
    else if( iExitFromEmbeddedMode )
    	{
    	BROWSER_LOG( ( _L( " delete this;" ) ) );
    	if ( iConnection )
    		{
            TRAP_IGNORE( SendDisconnectEventL() );
    		iConnection->Disconnect();
#ifdef __RSS_FEEDS
            BROWSER_LOG( ( _L( " iFeedsClientUtilities->DisconnectFeedsViewL()" ) ) );
            if ( iFeedsClientUtilities )
                {
            TRAP_IGNORE( iFeedsClientUtilities->DisconnectFeedsViewL() );
            //notify feeds engine to close the connection
            TRAP_IGNORE( iFeedsClientUtilities->DisconnectManualUpdateConnectionL() );
                } 
#endif
    		}
        if (SpecialLoadObserver().IsConnectionStarted()) 
            {
            if ( iWindowManager ) 
               {
               BROWSER_LOG( ( _L( " iWindowManager->SetUserExit( iUserExit )" ) ) );
               iWindowManager->SetUserExit( iUserExit );
               }
               delete iWindowManager;
               BROWSER_LOG( ( _L( " User::Exit(KErrNone)" ) ) );
               User::Exit(KErrNone);
            }
        else
            {
            PrepareToExit();
            Exit();
            }
    	}
    else
    	{
    	BROWSER_LOG( ( _L( " Exit()" ) ) );
    	if ( iConnection )
    		{
            TRAP_IGNORE( SendDisconnectEventL() );
    		iConnection->Disconnect();
#ifdef __RSS_FEEDS
            if ( iFeedsClientUtilities )
                {
    		BROWSER_LOG( ( _L( " iFeedsClientUtilities->DisconnectFeedsViewL()" ) ) );
    		TRAP_IGNORE( iFeedsClientUtilities->DisconnectFeedsViewL() );
    		//notify feeds engine to close the connection
			TRAP_IGNORE( iFeedsClientUtilities->DisconnectManualUpdateConnectionL() );
                }
#endif
    		}
    	if (SpecialLoadObserver().IsConnectionStarted()) // If Connection request is in processing calling CAknAppUI::Exit() causes crash (JSAA-84RG9R)
    	    {                                               
    	    //ensure that the params are saved in BrCtl            
    	    if ( iWindowManager ) 
    	       {
    	       BROWSER_LOG( ( _L( " iWindowManager->SetUserExit( iUserExit )" ) ) );
    	       iWindowManager->SetUserExit( iUserExit );
    	       }
    	       delete iWindowManager;
    	       BROWSER_LOG( ( _L( " User::Exit(KErrNone)" ) ) );
    	       User::Exit(KErrNone);                      
    	     }
        else
    	     {      
    	     // normal exit
             Exit();
    	     }
    	}
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::Display
// -----------------------------------------------------------------------------
//
MDisplay& CBrowserAppUi::Display() const
    {
    CBrowserWindow *window = iWindowManager->CurrentWindow();
    __ASSERT_DEBUG( window, Util::Panic( Util::EUninitializedData ) );
    return window->Display();
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::SoftkeysObserver()
// -----------------------------------------------------------------------------
//
CBrowserSoftkeysObserver& CBrowserAppUi::SoftkeysObserver() const
    {
    CBrowserWindow *window = iWindowManager->CurrentWindow();
    __ASSERT_DEBUG( window, Util::Panic( Util::EUninitializedData ) );
    return window->SoftkeysObserver();
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::CommsModel
// -----------------------------------------------------------------------------
//
MCommsModel& CBrowserAppUi::CommsModel() const
    {
    __ASSERT_DEBUG( iCommsModel,
        Util::Panic( Util::EUninitializedData ) );
    return *iCommsModel;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::Preferences
// -----------------------------------------------------------------------------
//
MPreferences& CBrowserAppUi::Preferences() const
    {
    __ASSERT_DEBUG( iPreferences,
        Util::Panic( Util::EUninitializedData ) );
    return *iPreferences;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::Connection
// -----------------------------------------------------------------------------
//
MConnection& CBrowserAppUi::Connection() const
    {
    __ASSERT_DEBUG( iConnection,
        Util::Panic( Util::EUninitializedData ) );
    return *iConnection;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::BrCtlInterface
// -----------------------------------------------------------------------------
CBrCtlInterface& CBrowserAppUi::BrCtlInterface() const
    {
    CBrowserWindow *window = iWindowManager->CurrentWindow();
    __ASSERT_DEBUG( window, Util::Panic( Util::EUninitializedData ) );
    return window->BrCtlInterface();
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::LoadObserver
// -----------------------------------------------------------------------------
//
CBrowserLoadObserver& CBrowserAppUi::LoadObserver() const
    {
    CBrowserWindow *window = iWindowManager->CurrentWindow();
    __ASSERT_DEBUG( window, Util::Panic( Util::EUninitializedData ) );
    return window->LoadObserver();
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::SpecialLoadObserver
// -----------------------------------------------------------------------------
//
CBrowserSpecialLoadObserver& CBrowserAppUi::SpecialLoadObserver() const
    {
    CBrowserWindow *window = iWindowManager->CurrentWindow();
    __ASSERT_DEBUG( window, Util::Panic( Util::EUninitializedData ) );
    return window->SpecialLoadObserver();
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::DialogsProvider
// -----------------------------------------------------------------------------
//
CBrowserDialogsProvider& CBrowserAppUi::DialogsProvider() const
	{
	return *iDialogsProvider;
	}

// -----------------------------------------------------------------------------
// CBrowserAppUi::PopupEngine
// -----------------------------------------------------------------------------
//
CBrowserPopupEngine& CBrowserAppUi::PopupEngine() const
    {
    LOG_ENTERFN("CBrowserAppUi::PopupEngine");
    if(!iPopupEngine)
        { 
            if ( Preferences().UiLocalFeatureSupported( KBrowserMultipleWindows ) )
                {
                    iPopupEngine = CBrowserPopupEngine::NewL();
                    BROWSER_LOG( ( _L( "PopupEngine up" ) ) );
                }
        }
    return *iPopupEngine;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::WindowMgr()
// -----------------------------------------------------------------------------
//
CBrowserWindowManager& CBrowserAppUi::WindowMgr() const
    {
    return *iWindowManager;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::IsCancelFetchAllowed
// -----------------------------------------------------------------------------
//
TBool CBrowserAppUi::IsCancelFetchAllowed() const
    {
    return ETrue;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::UpdateCbaL()
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::UpdateCbaL()
    {
    if( ActiveView() )
        {
        ActiveView()->UpdateCbaL();
        }
    }

void CBrowserAppUi::LogAccessToRecentUrlL( CBrCtlInterface& aBrCtlInterface )
    {
    if ((ADAPTIVEBOOKMARKS)&&(!(Preferences().AdaptiveBookmarks()==EWmlSettingsAdaptiveBookmarksOff )))
        {
        HBufC* url = aBrCtlInterface.PageInfoLC( TBrCtlDefs::EPageInfoUrl );
        HBufC* name = aBrCtlInterface.PageInfoLC( TBrCtlDefs::EPageInfoTitle );
        if( url && url->Length() )
            {
            // If url is same as the one in the store delete it because the url will be rewritten 
            // along with the name 
            iRecentUrlStore->DeleteData(*url);
            
            TInt index;
            index = url->Des().LocateReverse(KSlash);
            // If the only difference between the url in the store and the one we are writing is a "/" 
            // at the end of the url, delete the one in the store because it will be rewritten along 
            // with the name
            if (index == url->Length()-1)
            	{
	            HBufC* urlwoslash = HBufC::NewLC( url->Length() );
				urlwoslash->Des().Copy( *url );
				urlwoslash->Des().Delete(url->Length() -1, 2);         
	            
	            iRecentUrlStore->DeleteData(*urlwoslash);

	            CleanupStack::PopAndDestroy(); //urlwoslash            	
            	}
                        
            if (name && name->Length())
            	{
            	iRecentUrlStore->SaveDataL(*url,*name);
            	}
            else
            	{
            	if (url->Length() > KFavouritesMaxName)
                	{
                	// name will be the url but it needs to be compressed to fit max name length
                	HBufC* cname = HBufC::NewLC(KFavouritesMaxName);
                	TInt edgeStringLen = ((KFavouritesMaxName / 2) - 3);
                	TPtrC cnameRight( url->Right( edgeStringLen ) );
                	TPtrC cnameLeft( url->Left( edgeStringLen ));
                	TPtr cnameDes = cname->Des();

                  	cnameDes.Append( cnameLeft );
                   	cnameDes.AppendFill( TChar(KDot),5 );   // '.....'
                   	cnameDes.Append( cnameRight );
                	iRecentUrlStore->SaveDataL(*url,*cname);
                    CleanupStack::PopAndDestroy();//cname
                	}
                else
            	    {
            	    iRecentUrlStore->SaveDataL(*url,*url);
            	    }
            	}
            }
        CleanupStack().PopAndDestroy(2); // url, name
        }
    }

void CBrowserAppUi::LogRequestedPageToRecentUrlL( const TDesC& aUrl )
    {
    if ((ADAPTIVEBOOKMARKS)&&(!(Preferences().AdaptiveBookmarks()==EWmlSettingsAdaptiveBookmarksOff )))
        {
        HBufC *url = HBufC::NewLC( aUrl.Length() );
		url->Des().Copy( aUrl );

        if( url && url->Length() )
            {
           	if (url->Length() > KFavouritesMaxName)
            	{
            	// name will be the url but it needs to be compressed to fit max name length
            	HBufC* cname = HBufC::NewLC(KFavouritesMaxName);
            	TInt edgeStringLen = ((KFavouritesMaxName / 2) - 3);
            	TPtrC cnameRight( url->Right( edgeStringLen ) );
            	TPtrC cnameLeft( url->Left( edgeStringLen ));
            	TPtr cnameDes = cname->Des();

              	cnameDes.Append( cnameLeft );
               	cnameDes.AppendFill( TChar(KDot),5 );   // '.....'
               	cnameDes.Append( cnameRight );
            	iRecentUrlStore->SaveDataL(*url,*cname);
                CleanupStack::PopAndDestroy();//cname
            	}
            else
        	    {
        	    iRecentUrlStore->SaveDataL(*url,*url);
        	    }
            }
        CleanupStack().PopAndDestroy(); // url
        }
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::SwitchWindowL()
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::SwitchWindowL()
    {
LOG_ENTERFN("CBrowserAppUi::SwitchWindowL");
    CArrayFixFlat<CWindowInfo*>* windowBuf = iWindowManager->GetWindowInfoL( this );
    CleanupStack::PushL( windowBuf );

    // Open 'Switch Window' selection dialog
    HBufC* title = StringLoader::LoadLC( R_BROWSER_SWITCH_WINDOW_HEADER );

    CArrayFixFlat<TBrCtlSelectOptionData>* optDataArray = new ( ELeave )
        CArrayFixFlat<TBrCtlSelectOptionData>( KGranularityMedium );
    CleanupStack::PushL( optDataArray );

    TInt i( 0 );
    TInt numWindows = windowBuf->Count();

LOG_WRITE_FORMAT("Window's count: %d", numWindows );
    for ( i = 0; i < numWindows; ++i )
        {
        TBrCtlSelectOptionData optData( *((*windowBuf)[i]->iWindowText),
                                        (*windowBuf)[i]->iCurrent,
                                        EFalse,
                                        EFalse );
        optDataArray->AppendL( optData );
        }

    TBool ret = DialogsProvider().DialogSelectOptionL( *title,
                                                        ESelectTypeSingle,
                                                        *optDataArray );
    if ( ret )
        {
        for( i = 0; i < numWindows; ++i)
            {
            if( (*optDataArray)[i].IsSelected() )
                {
                LOG_WRITE_FORMAT("Selected window: %d", (*windowBuf)[i]->iWindowId );
                iWindowManager->SwitchWindowL( (*windowBuf)[i]->iWindowId );
                break;
                }
            }
        }
    CleanupStack::PopAndDestroy( 3 );           // optDataArray, title, windowBuf
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::SetCalledFromAnotherApp
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::SetCalledFromAnotherApp( TBool aValue )
    {
    iCalledFromAnotherApp = aValue;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::CalledFromAnotherApp
// -----------------------------------------------------------------------------
//
TBool CBrowserAppUi::CalledFromAnotherApp()
    {
    return iCalledFromAnotherApp;
    }
    
// -----------------------------------------------------------------------------
// CBrowserAppUi::NoHomePageToBeLaunchedL
// -----------------------------------------------------------------------------
//
TBool CBrowserAppUi::NoHomePageToBeLaunchedL()
    {
    LOG_ENTERFN("CBrowserAppUi::NoHomePageToBeLaunchedL");
    TWmlSettingsHomePage pgtype = iPreferences->HomePageType();
    TBool alwaysAskCase = (( Preferences().AccessPointSelectionMode() == EAlwaysAsk ) &&
                           ( EWmlSettingsHomePageAccessPoint == pgtype ));
    BROWSER_LOG( ( _L( "pgtype: %d, alwaysAsk: %d" ), pgtype, alwaysAskCase ) );

    if( ( pgtype == EWmlSettingsHomePageBookmarks ) || alwaysAskCase )
        {   // Can't load home page since set to Bookmarks or (AlwaysAsk AP and homepage set to AP homepage)
        return ETrue;
        }
    else
        {
        // now check for other cases where homepage string might be defined and will be used    
        HBufC* buf = HBufC::NewLC( KMaxHomePgUrlLength );  // cleanupstack
        TPtr ptr( buf->Des() );
        TInt pgFound = Preferences().HomePageUrlL( ptr );
        CleanupStack::PopAndDestroy( buf );
        if (pgFound != KErrNone)
            {   // no homepage string found
            return ETrue;
            }
        else
            {   // homepage string was found 
            return EFalse;
            }
        }        
    }

// CBrowserAppUi::ProcessCommandParametersL()
// -----------------------------------------------------------------------------
//
TBool CBrowserAppUi::ProcessCommandParametersL( TApaCommand aCommand,
                                               TFileName& /*aDocumentName*/ ,
                                               const TDesC8& /*aTail*/ )
    {
    LOG_ENTERFN("CBrowserAppUi::ProcessCommandParametersL");
    LOG_WRITE_FORMAT(" aCommand: %d", aCommand);

    // The browser is in embedded mode and it is not initialized yet
    if ( IsEmbeddedModeOn() && !iStartedUp)
    	{
    	return EFalse;
    	}

    if ( aCommand == EApaCommandOpen )
        {
		//Startup parameters are given to browser at CBrowserAppDocument::OpenFileL().
		//- a part of shell starting (ConstructAppFromCommandLineL calls OpenFileL)
        }

    else if ( aCommand == EApaCommandRun )
        {

   	    if ( LastActiveViewId() == KUidBrowserNullViewId )
	        {
	        TBool specialSchemeInHomePageAddress = EFalse;
	        
	        if (NoHomePageToBeLaunchedL())
	            {
	            // No homepage to be launched so start browser in bookmarks view
    	        SetLastActiveViewId(KUidBrowserBookmarksViewId);
	            }
    	    else if ( iPreferences->SearchFeature() || iPreferences->ServiceFeature())
    	        {
    	        
    	        HBufC* buf = HBufC::NewLC( KMaxHomePgUrlLength );  // cleanupstack
    	        TPtr ptr( buf->Des() );
    	        
    			HBufC* searchScheme = HBufC::NewLC( KMaxHomePgUrlLength );  // cleanupstack
    			TPtr searchSchemePtr( searchScheme->Des() );
    			
    			HBufC* serviceScheme = HBufC::NewLC( KMaxHomePgUrlLength );  // cleanupstack
    			TPtr serviceSchemePtr( serviceScheme->Des() );
    			
    	        iPreferences->GetStringValueL( KBrowserSearchProviderUrl, KMaxHomePgUrlLength, searchSchemePtr );
    	        iPreferences->GetStringValueL( KBrowserServiceUrl, KMaxHomePgUrlLength, serviceSchemePtr );

    			
	            if( !ptr.Compare(serviceSchemePtr) || !ptr.Compare(searchSchemePtr) )
	            	{    
	            	specialSchemeInHomePageAddress = ETrue;
	            	SetLastActiveViewId(KUidBrowserBookmarksViewId);
	            	SetViewToBeActivatedIfNeededL(KUidBrowserContentViewId, 0);
	            	TRAPD( err, FetchL( ptr, CBrowserLoadObserver::ELoadUrlTypeOther ) );
	            	}
	            else
	                {
	                SetViewToBeActivatedIfNeededL(KUidBrowserContentViewId, 0);
                    StartFetchHomePageL();
	            	}
    	        CleanupStack::PopAndDestroy( 3,buf );
    	        }
            else
            	{
                SetViewToBeActivatedIfNeededL(KUidBrowserContentViewId, 0);
                StartFetchHomePageL();
            	}
           
	        if( !specialSchemeInHomePageAddress )
	            {
                if( iCurrentView != iLastViewId )
                    ActivateLocalViewL( LastActiveViewId() );
	            }
        	}        	   	  
        }
    else if ( aCommand == EApaCommandViewActivate )
    // Get when activated from soft notification (Downloads List).
    // Must behave as in case of EApaCommandOpen, but no OpenFileL() is called.
        {
        ParseAndProcessParametersL( KNullDesC8 );
        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::Static()
// -----------------------------------------------------------------------------
//
CBrowserAppUi* CBrowserAppUi::Static()
    {
    return REINTERPRET_CAST( CBrowserAppUi*, CEikonEnv::Static()->EikAppUi() );
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::ParseAndProcessParametersL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::ParseAndProcessParametersL( const TDesC8& aDocumentName, TBool aDoFetch )
    {
    LOG_ENTERFN("CBrowserAppUi::ParseAndProcessParametersL");
    TInt err = KErrNone;

    TUid ViewToActivate = { 0 };

    HBufC8* params = NULL;
    TLex8 parameter(aDocumentName);

    // Here we check if the url contains control characters.
    TBool urlError( EFalse );  // If there is a Control caracter in the url.
    parameter.Mark(); // Save the start position in the string.
    while ( !parameter.Eos() && !urlError )
        {
        if ( parameter.Get().IsControl() )
            {
            urlError = ETrue;
            }
        }
    parameter.UnGetToMark(); // Now go back to the beginning of the string.

    //Check for parameter validity
    if(	!parameter.Peek().IsDigit() || urlError )    //First char of aDocumentName is NOT a digit
        {
        if( ( aDocumentName.Locate( ':' ) > 0) && (!urlError) )    //There is a ":" character in aDocumentName (->probably a valid url)
            {
            //Add "4" + <space> before the given parameter to make the parameter legal for url launch
            _LIT8(KUrlId, "4 ");
            params = HBufC8::NewL(aDocumentName.Length() + 2 + 1);
            params->Des().Copy( KUrlId );
            params->Des().Append(aDocumentName);
            params->Des().ZeroTerminate();
            ViewToActivate = KUidBrowserContentViewId;
            }
        else
            {
            //Activate bookmarks view if parameters are invalid and browser is not embedded.
            // In embedded mode bookmarks are not shown, so if there is an
            // error in the url, we make the browser to pop up a
            // "Page not found" note, by defining a not existing url.
            if ( IsEmbeddedModeOn() )
                {
                params = HBufC8::NewL( 10 );
                params->Des().Copy(_L8("4 http://") );
                }
            else
                {
                params = HBufC8::NewL(aDocumentName.Length() + 1 );
                params->Des().Copy(aDocumentName);
                }
            params->Des().ZeroTerminate();
            ViewToActivate = KUidBrowserBookmarksViewId;
            }
        }
    else
        {
        params = HBufC8::NewL(aDocumentName.Length() + 1);
        params->Des().Copy(aDocumentName);
        params->Des().ZeroTerminate();
        ViewToActivate = KUidBrowserContentViewId;
        }
    CleanupStack::PushL( params );

    //Set view to activate if no view yet activated
    if ( LastActiveViewId() == KUidBrowserNullViewId )
        {
        SetLastActiveViewId(ViewToActivate);
        }

    CBrowserCommandLineParser8* command = CBrowserCommandLineParser8::NewL( *params );
    CleanupStack::PopAndDestroy( params );
    CleanupStack::PushL( command );


    // complete the delayed initialization if bookmarks view is not the first view
    if(ViewToActivate != KUidBrowserBookmarksViewId && !iStartedUp)
        CompleteDelayedInit();

	//wait a while, contentview initializing itself
    WaitCVInit();
    switch ( command->Count() )
        {
        case 0:
            {
            SetViewToBeActivatedIfNeededL( LastActiveViewId() );
            break;
            }
        case 1:
            {
            if ( !aDocumentName.Compare( KLongZeroIdString ) )
                {
                // Long 0 is pressed and Browser is started up for the first time...
                // if no AP or SNAP is defined, define it
                TUint32 defaultAp = iPreferences->DefaultAccessPoint();
                TUint32 defaultSnap =  iPreferences->DefaultSnapId();
                // AP or SNAP is not defined
                if( (defaultAp == KWmlNoDefaultAccessPoint && iPreferences->AccessPointSelectionMode() == EConnectionMethod) || 
                    (defaultSnap == KWmlNoDefaultSnapId && iPreferences->AccessPointSelectionMode() == EDestination) )
                    {
                    StartPreferencesViewL( EShowAlwaysAsk | EShowDestinations | EShowConnectionMethods );
                    }
                // If there is a homepage url then try to fetch it
                if ( !NoHomePageToBeLaunchedL() )
                    {
                    iLongZeroPressed = ETrue;
                    FetchHomePageL();
                    }
                else  
                    {
                    if (iStartedUp)
                    ContentView()->SetFullScreenOffL();

                    if ( !IsEmbeddedModeOn() )
                        {
                        SetLastActiveViewId( KUidBrowserBookmarksViewId );
                        }
                    }
                }
            SetViewToBeActivatedIfNeededL( LastActiveViewId() );
            break;
            }
        case 2:
        case 3:
            {
            // UID and wml adress and optional access point UID

            TUint typeId;
            TInt dataId;
            TLex8 param0( command->Param( 0 ) );
            TLex8 param1( command->Param( 1 ) );
            err = param0.Val( typeId );
            if ( !err )
                {
                BROWSER_LOG( ( _L( " typeId: %d" ), typeId ) );

                if ( typeId == KUrlId )
                    {  // FIX THIS!
                    TUriParser8 uri8;
                    err = uri8.Parse(command->Param(1));      // what if parsing fails?
                    User::LeaveIfError(err);
                    // Url decoding section was removed from here.
                    HBufC* url = HBufC::NewL(command->Param(1).Length()+1);
                    url->Des().Copy( command->Param(1) );
                    CleanupStack::PushL(url);
                    BROWSER_LOG( ( _L( " url: %S" ), url ) );

                    TFavouritesWapAp accessPoint;   // Initially "default".
                    BROWSER_LOG( ( _L( " Count: %d" ), command->Count() ) );
                    if ( command->Count() == 3 )
                        {
                        // URL Typeid, url, ap
                        TUint ap;
                        TLex8 param2( command->Param( 2 ) );
                        err = param2.Val( ap );
                        if ( !err )
                            {
                            BROWSER_LOG( ( _L( " ap: %d" ), ap ) );
                            if (iSuppressAlwaysAsk)
                                {
                                // Always ask was temporarily disabled, enable it again
                                iPreferences->SetAccessPointSelectionModeL(EAlwaysAsk);
                                
                                iSuppressAlwaysAsk = EFalse;
                                }

                            // Note: OTA Provisioning application requires the 
                            // KBrowserAccessPointSelectionMode key value to be
                            // read directly from cenrep, rather than use the
                            // cached cenrep value taken at browser initialization 
                            // time. 
                            TInt selectionMode;

                            CRepository* repository = CRepository::NewLC( KCRUidBrowser );
                            User::LeaveIfError( repository->Get( KBrowserAccessPointSelectionMode, selectionMode ) );
                            CleanupStack::PopAndDestroy( repository );

                            // If selectionMode cenrep value is always ask, then 
                            // temporarily change selection mode
                            if ( EBrowserCenRepApSelModeAlwaysAsk == selectionMode )
                                {
                                iSuppressAlwaysAsk = ETrue;
                                iPreferences->SetAccessPointSelectionModeL(EConnectionMethod);
                                }
                            accessPoint.SetApId( ap );
                            SetCalledFromAnotherApp( ETrue );
                            CBrowserLoadObserver::TBrowserLoadUrlType urlType =
                                IsEmbeddedModeOn() ?
                                    CBrowserLoadObserver::ELoadUrlTypeEmbeddedBrowserWithUrl :
                                    CBrowserLoadObserver::ELoadUrlTypeOther;
                            if( IsPageLoaded() &&
                            	!(WindowMgr().CurrentWindow()->HasWMLContent(EFalse)) &&
            		            Preferences().UiLocalFeatureSupported( KBrowserMultipleWindows ) &&
            	                !Preferences().UiLocalFeatureSupported( KBrowserMinimalMultipleWindows ))
                                {
                                // Cancel history view, mini map, toolbar or any other active control on the current window 
                                BrCtlInterface().HandleCommandL( TBrCtlDefs::ECommandCancel + TBrCtlDefs::ECommandIdBase );
                                // there is already a window, so create a new one
                                CBrowserWindow *win = WindowMgr().CreateWindowL( 0, &KNullDesC );
                                if (win != NULL)
                                    {
                                        
                                    CleanupStack::PushL( win );
                                    WindowMgr().SwitchWindowL( win->WindowId() );
                                    TRAP( err, FetchL(  *url,
                                                                KNullDesC,
                                                                KNullDesC,
                                                                accessPoint,
                                                                urlType ) );
                                    CleanupStack::Pop();  // win
                                    SetLastActiveViewId( KUidBrowserContentViewId );
                                    }
                                }                          
                            else
                                {
                                // Cancel history view, mini map, toolbar or any other active control on the current window 
                                BrCtlInterface().HandleCommandL( TBrCtlDefs::ECommandCancel + TBrCtlDefs::ECommandIdBase );
                                TRAP( err, FetchL(  *url,
                                                            KNullDesC,
                                                            KNullDesC,
                                                            accessPoint,
                                                            urlType ) );                                
                                SetLastActiveViewId( KUidBrowserContentViewId );                                
                                }                        
                            }
                        }
                    else
                        {
                        // URL Typeid, url
                        SetCalledFromAnotherApp( ETrue );

                        if ( aDoFetch )
                            {
                            CBrowserLoadObserver::TBrowserLoadUrlType urlType =
                                IsEmbeddedModeOn() ?
                                    CBrowserLoadObserver::ELoadUrlTypeEmbeddedBrowserWithUrl :
                                    CBrowserLoadObserver::ELoadUrlTypeOther;
                            if( IsPageLoaded() &&
                            	!(WindowMgr().CurrentWindow()->HasWMLContent(EFalse)) &&                            
                        		Preferences().UiLocalFeatureSupported( KBrowserMultipleWindows ) &&
                            	!Preferences().UiLocalFeatureSupported( KBrowserMinimalMultipleWindows ))
                                {
                                // Cancel history view, mini map, toolbar or any other active control on the current window 
                                BrCtlInterface().HandleCommandL( TBrCtlDefs::ECommandCancel + TBrCtlDefs::ECommandIdBase );
                                // there is already a window, so create a new one
                                CBrowserWindow *win = NULL; 
                                if(WindowMgr().CurrentWindow()) 
                                {
                                    if(iSameWinApp)
                                    {
                                        //We will be using same/already opened window if call is from external
                                        //application. So no new windows will be opened.
                                        win = WindowMgr().CurrentWindow();
                                        iWindowIdFromFromExternApp = win->WindowId();
                                    }
                                    if(!win)
                                    {
                                        //If not called from external app, then create window.
                                        win = WindowMgr().CreateWindowL( (WindowMgr().CurrentWindow()->WindowId()) ? WindowMgr().CurrentWindow()->WindowId() : 0, &KNullDesC );
                                    }
                                }
                                else
                                    win = WindowMgr().CreateWindowL( 0, &KNullDesC );
                                if (win != NULL)
                                    {
                                        
                                    CleanupStack::PushL( win );
                                    WindowMgr().SwitchWindowL( win->WindowId() );
                                    TRAP( err, FetchL(  *url,
                                                                KNullDesC,
                                                                KNullDesC,
                                                                accessPoint,
                                                                urlType ) );
                                    CleanupStack::Pop();  // win
                                    }
                            }
                            else
                                {
                                // Cancel history view, mini map, toolbar or any other active control on the current window 
                                BrCtlInterface().HandleCommandL( TBrCtlDefs::ECommandCancel + TBrCtlDefs::ECommandIdBase );
                                TRAP( err, FetchL(  *url,
                                                            KNullDesC,
                                                            KNullDesC,
                                                            accessPoint,
                                                            urlType ) );
                                }
                            }          
                    if (err==KErrNone)
                        {
                        SetLastActiveViewId(ViewToActivate);
                        }
                        }
                    CleanupStack::PopAndDestroy();  // url
                    }
                else if( typeId == KLaunchFeeds )
                    {
                    // Special facility to launch non-embedded into the feeds view
                    if( !iBrowserAlreadyRunning )
                    	{
                    	SetOverriddenLaunchContextId(EBrowserContextIdFeeds);
                    	SetLastActiveViewId( KUidBrowserFeedsFolderViewId );                    	
                    	}	
                    LaunchIntoFeedsL();          	
                    }         
                else
                    {
                    err = param1.Val( dataId );
                    if ( !err )
                        {
                        switch ( typeId )
                            {
                            case KBookmarkId:
                                {
                                // Bookmark typeid, uid
                                SetCalledFromAnotherApp( ETrue );
								
	                            if( IsPageLoaded() &&
                               	!(WindowMgr().CurrentWindow()->HasWMLContent(EFalse)) &&
                        		Preferences().UiLocalFeatureSupported( KBrowserMultipleWindows ) &&
                            	!Preferences().UiLocalFeatureSupported( KBrowserMinimalMultipleWindows ))
	                                {
	   	                            // Cancel history view, mini map, toolbar or any other active control on the current window 
	                                BrCtlInterface().HandleCommandL( TBrCtlDefs::ECommandCancel + TBrCtlDefs::ECommandIdBase );
	                                // there is already a window, so create a new one
	                                CBrowserWindow *win = WindowMgr().CreateWindowL( 0, &KNullDesC );
	                                if (win != NULL)
    	                                {
    	                                CleanupStack::PushL( win );
    	                                WindowMgr().SwitchWindowL( win->WindowId() );
    	                            	TRAP( err, FetchBookmarkL( dataId ) );
    	                                CleanupStack::Pop();  // win                                
    	                                }
	                                }
	                            else
	                            	{
    	                            // Cancel history view, mini map, toolbar or any other active control on the current window 
	                                BrCtlInterface().HandleCommandL( TBrCtlDefs::ECommandCancel + TBrCtlDefs::ECommandIdBase );
	                            	TRAP( err, FetchBookmarkL( dataId ) );
	                            	}    

                                if ( err )
                                    {
                                    // Unable to fetch bookmark
                                    TBrowserDialogs::InfoNoteL(
                                        R_BROWSER_INFO_NOTE, R_WML_UNABLE_TO_FETCH_BOOKMARK );
                                    }
	                            if (err==KErrNone)
    			                    {
                			        SetLastActiveViewId(ViewToActivate);
                        			}

                                break;
                                }
                            case KFolderId:
                                {
                                SetCalledFromAnotherApp( EFalse );
                                iIsForeground = IsForeground();
                                CloseContentViewL();
                                SetLastActiveViewId( KUidBrowserBookmarksViewId );
                                break;
                                }
                            default:
                                {
                                // Typeid was not readable
                                // TBD: error message!
                                break;
                                }
                            }
                        }
                    else
                        {
                        // Dataid was not readable
                        User::Leave( err );
                        }
                    }
                }
            else
                {
                // Sender Uid was not readable
                User::Leave( err );
                }
            if( !iSpecialSchemeinAddress )
            	{              
            	SetViewToBeActivatedIfNeededL( LastActiveViewId() );
            	}
            break;
            }

        default:
            {
            SendBrowserToBackground();
            break;
            }
        }
        CleanupStack::PopAndDestroy();  // command
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::WaitCVInit
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::WaitCVInit()
    {
    if( iParametrizedLaunchInProgress == 0 && iStartedUp)
        {
        iParametrizedLaunchInProgress = 1;
        iIdle->Cancel();
        iIdle->Start( TCallBack( StopDelay, this ) );
        iWait.Start();
        iParametrizedLaunchInProgress = 2;
        }
    }
void CBrowserAppUi::StopConnectionObserving()
    {
    LOG_ENTERFN("CBrowserAppUi::StopConnectionObserving");
    if ( iConnStageNotifier && iConnStageNotifier->IsActive() )
        {
        BROWSER_LOG( ( _L( " CBrowserAppUi::StopConnectionObserving Cancelling Observer" ) ) );
        iConnStageNotifier->Cancel();
        }
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::ConnNeededStatusL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::ConnNeededStatusL( TInt aErr )
    {
    LOG_ENTERFN("CBrowserAppUi::ConnNeededStatusL");
    UpdateSoftKeys();
    if (iSuppressAlwaysAsk)
        {
        // Always ask was temporarily disabled, after creating the network connection, enable it again
        iPreferences->SetAccessPointSelectionModeL(EAlwaysAsk);
        iSuppressAlwaysAsk = EFalse;
        }
    if ( aErr != KErrNone )
        {
        if( iCalledFromAnotherApp )
            {
            // This function call should be replaced by ExitBrowser(ETrue)
            // so that the blank content view is not displayed.
            // SendBrowserToBackground();
            }
        // if any error, return Cancel to Kimono.
        return;
        }

    BROWSER_LOG( ( _L( " CBrowserAppUi::ConnNeededStatusL First Stop Connection Observer" ) ) );
    StopConnectionObserving(); //Need to stop the connection observer first


    if ( !iConnStageNotifier->IsActive() )
        {
        BROWSER_LOG( ( _L( " CBrowserAppUi::ConnNeededStatusL Starting Connection Observer" ) ) );
        TName* connectionName = Connection().ConnectionNameL();
        CleanupStack::PushL( connectionName );

        iConnStageNotifier->StartNotificationL(connectionName, KLinkLayerClosed, this);

        CleanupStack::PopAndDestroy();  //connectionName
        }
    }


// -----------------------------------------------------------------------------
// CBrowserAppUi::SendBrowserToBackground
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::SendBrowserToBackground()
    {
    TApaTaskList taskList( CEikonEnv::Static()->WsSession() );

    // Browser might be embedded. 
    TUid appUid;
    if ( iEmbeddingApplicationUid != KNullUid )
        {
        appUid = iEmbeddingApplicationUid;
        }
    else 
        {
        appUid = KUidBrowserApplication;
        }
    TApaTask task = taskList.FindApp( appUid );
    task.SendToBackground();
    }


// -----------------------------------------------------------------------------
// CBrowserAppUi::CreateWindowInfoLC()
// -----------------------------------------------------------------------------
//
HBufC* CBrowserAppUi::CreateWindowInfoLC( const CBrowserWindow& aWindow )
    {
    HBufC* buf = aWindow.BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoTitle );
    if( !buf || !buf->Length() )
        {
        CleanupStack::PopAndDestroy( buf );
        buf = aWindow.BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoUrl );
        if( !buf )
            {

            CleanupStack::PopAndDestroy( buf );
            buf = KNullDesC().AllocLC();
            }
        }
    return buf;
    }
	
#ifdef BRDO_OCC_ENABLED_FF
// -----------------------------------------------------------------------------
// CBrowserContentView::SetRetryFlag
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::SetRetryFlag(TBool flag)
     {
     LOG_ENTERFN("CBrowserAppUi::SetRetryFlag");
     BROWSER_LOG( ( _L(" CBrowserAppUi::SetRetryFlag flag: %d"), flag ) );
     reConnectivityFlag = flag;
     }

// -----------------------------------------------------------------------------
// CBrowserContentView::GetRetryFlag
// -----------------------------------------------------------------------------
//
 TBool CBrowserAppUi::GetRetryFlag()
      {
      LOG_ENTERFN("CBrowserAppUi::GetRetryFlag");
      BROWSER_LOG( ( _L(" CBrowserAppUi::GetRetryFlag flag: %d"), reConnectivityFlag ) );
      return reConnectivityFlag;
      }
	  
// -----------------------------------------------------------------------------
// CBrowserContentView::RetryConnectivity
// -----------------------------------------------------------------------------
//
TInt CBrowserAppUi::RetryConnectivity(TAny* aCBrowserAppUi)
    {
    LOG_ENTERFN("CBrowserAppUi::RetryConnectivity");
    __ASSERT_DEBUG(aCBrowserAppUi, Util::Panic( Util::EUninitializedData ));
  
    TInt err = ((CBrowserAppUi*)aCBrowserAppUi)->RetryInternetConnection();
    
    BROWSER_LOG( ( _L(" CBrowserAppUi::RetryConnectivity Error: %d"), err ) );
    return err;
    }
	
TInt CBrowserAppUi::RetryInternetConnection()
    {
    LOG_ENTERFN("CBrowserAppUi::RetryInternetConnection");
    //First cancel the timer
    if ( iRetryConnectivity && iRetryConnectivity->IsActive() )
    {
        iRetryConnectivity->Cancel();
        BROWSER_LOG( ( _L( "CBrowserAppUi::RetryInternetConnection Timer Cancelled successfully " ) ) );
    }
    TInt err = KErrNone;
    if ( !iConnection->Connected() )
       {
       TRAP_IGNORE( err = iConnection->StartConnectionL( ETrue ) );
       }
    if( err == KErrNone )
       { 
       BROWSER_LOG( ( _L( "CBrowserAppUi::RetryInternetConnection Connection restablished successfully " ) ) );
       
       BROWSER_LOG( ( _L( "CBrowserAppUi::RetryInternetConnection UNSET retry flags " ) ) );
       TRAP_IGNORE( BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandUnSetRetryConnectivityFlag + (TInt)TBrCtlDefs::ECommandIdBase ) );
       SetRetryFlag(EFalse);
       
       TRAP_IGNORE(ConnNeededStatusL(err)); //Start the observer again
       TRAP_IGNORE( BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandRetryTransactions + (TInt)TBrCtlDefs::ECommandIdBase ) );
       //Let download manager knows about this new connection
       TRAP_IGNORE( BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandConnToDownloadManager + (TInt)TBrCtlDefs::ECommandIdBase ) );
       }
    else
        {
        BROWSER_LOG( ( _L( "CBrowserAppUi::RetryInternetConnection Connection failed " ) ) );
        BROWSER_LOG( ( _L( "CBrowserAppUi::RetryInternetConnection unset retry flags " ) ) );
        TRAP_IGNORE( BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandUnSetRetryConnectivityFlag + (TInt)TBrCtlDefs::ECommandIdBase ) );
        SetRetryFlag(EFalse);
        BROWSER_LOG( ( _L( "CBrowserAppUi::RetryInternetConnection clear queued transactions " ) ) );
        TRAP_IGNORE( BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandClearQuedTransactions + (TInt)TBrCtlDefs::ECommandIdBase ) );
        Display().StopProgressAnimationL(); //Stop Progress animation
        if ( Fetching() )
            {
            BROWSER_LOG( ( _L( "CBrowserAppUi::RetryInternetConnection cancel fetch " ) ) );
            CancelFetch();
            }
        iDialogsProvider->UploadProgressNoteL(0, 0, ETrue, (MBrowserDialogsProviderObserver *)this ); //Close the uploading dialog.
        iDialogsProvider->CancelAll(); //connection has been lost, so cancel the authentication dialog.
        }
    
    return err;
    }

void CBrowserAppUi::CheckOccConnectionStage()
    {
    LOG_ENTERFN("CBrowserAppUi::CheckOccConnectionStage");
    
    //Disconnect first
    BROWSER_LOG( ( _L( "CBrowserAppUi::CheckOccConnectionStage Disconnecting..." ) ) );
    iConnection->Disconnect();
    
    TNifProgressBuf buf = iConnStageNotifier->GetProgressBuffer();
    if( buf().iError == KErrDisconnected )
        {
        BROWSER_LOG( ( _L("CBrowserAppUi::CheckOccConnectionStage This is OCC roaming error : %d"), buf().iError ) );
        BROWSER_LOG( ( _L( "CBrowserAppUi::CheckOccConnectionStage Set retry flags " ) ) );
        TRAP_IGNORE( BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandSetRetryConnectivityFlag + (TInt)TBrCtlDefs::ECommandIdBase ) );
        SetRetryFlag(ETrue);    
        TRAP_IGNORE( BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandCancelQueuedTransactions + (TInt)TBrCtlDefs::ECommandIdBase ) );
        
        if( iRetryConnectivity && iRetryConnectivity->IsActive())
            {
            iRetryConnectivity->Cancel();
            }
        iRetryConnectivity->Start(KRetryConnectivityTimeout, 0,TCallBack(RetryConnectivity,this));
        }
    else
        {
        BROWSER_LOG( ( _L("CBrowserAppUi::CheckOccConnectionStage This is NOT OCC roaming error : %d"), buf().iError ) );
        Display().StopProgressAnimationL();
        if ( Fetching() )
            {
            CancelFetch();
            }
        iDialogsProvider->UploadProgressNoteL(0, 0, ETrue, (MBrowserDialogsProviderObserver *)this );
        iDialogsProvider->CancelAll();
        }  
    }
#endif

// -----------------------------------------------------------------------------
// CBrowserAppUi::ConnectionStageAchievedL()
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::ConnectionStageAchievedL()
    {
#ifdef BRDO_OCC_ENABLED_FF
    LOG_ENTERFN("CBrowserAppUi::ConnectionStageAchievedL");
    CheckOccConnectionStage();
#else
    // this function is called only when network is lost
    // because we set notifier for KAgentUnconnected only
    Display().StopProgressAnimationL();
	if ( Fetching() )
		{
                 CancelFetch();
		}
    //Close the uploading dialog.
    iDialogsProvider->UploadProgressNoteL(
	                0, 0, ETrue, (MBrowserDialogsProviderObserver *)this );
    //connection has been lost, so cancel the authentication dialog.
    iDialogsProvider->CancelAll();
    // SendDisconnectEventL();
    // not needed as by that point HTTPSession was already shutdown by executing disconnect menu option
    // will cause a crash when user tries to quickly reconnect right after disconnecting, as HTTP session
    // is starting to initialize, while this call is trying to close it.
#endif
    }


// -----------------------------------------------------------------------------
// CBrowserAppUi::UpdateSoftKeys
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::UpdateSoftKeys()
    {
	TRAP_IGNORE( UpdateCbaL() );
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::DeleteCookiesL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::DeleteCookiesL()
    {
    // stringPool is not needed to initialize
    RStringPool stringPool;
    RCookieManager cookieManager( stringPool );
    TInt num(0);

    User::LeaveIfError( cookieManager.Connect() );
    CleanupClosePushL( cookieManager );
    User::LeaveIfError( cookieManager.ClearCookies( num ) );
    CleanupStack::PopAndDestroy( &cookieManager );

    stringPool.Close();
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::LaunchHomePageL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::LaunchHomePageL()
    {
    LOG_ENTERFN( "CBrowserAppUi::LaunchHomePageL" );
    TWmlSettingsHomePage pgtype = iPreferences->HomePageType();
    TBool alwaysAskCase = (( Preferences().AccessPointSelectionMode() == EAlwaysAsk ) &&
                           ( EWmlSettingsHomePageAccessPoint == pgtype ));
    BROWSER_LOG( ( _L( "pgtype: %d, alwaysAsk: %d" ), pgtype, alwaysAskCase ) );

    if( ( pgtype == EWmlSettingsHomePageBookmarks ) || alwaysAskCase )
        // Bookmarks is selected as HomePage
        // has effect only for startup, otherwise 'Home' command is not shown
        {
        ContentView()->SetFullScreenOffL();
        // view change will be done somewhere else
        SetLastActiveViewId( KUidBrowserBookmarksViewId );
        SetViewToBeActivatedIfNeededL( LastActiveViewId() );
        return;
        }

    HBufC* buf = HBufC::NewLC( KMaxHomePgUrlLength );  // cleanupstack
    TPtr ptr( buf->Des() );
    TInt pgFound( KErrNotFound );

    pgFound = Preferences().HomePageUrlL( ptr );
    BROWSER_LOG( ( _L( "pgFound 1: %d" ), pgFound ) );


    if( pgFound == KErrNone )     // Home page url found, fetch the url
        {
        SetLastActiveViewId( KUidBrowserContentViewId );
        FetchL( ptr );
        }
    else  // Home page not found, or not defined, go to BookMarksView
        {
        TVwsViewId activeViewId;
        if( ( GetActiveViewId( activeViewId ) != KErrNone ) ||
            ( activeViewId.iViewUid == KUidBrowserContentViewId ) )
            {
            ContentView()->SetFullScreenOffL();
            }
        SetLastActiveViewId( KUidBrowserBookmarksViewId );
        SetViewToBeActivatedIfNeededL( LastActiveViewId() );
        }
    CleanupStack::PopAndDestroy( buf );
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::ClearTheCacheL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::ClearTheCacheL(TBool afterQuery, TBool aShowDoneNote)
    {
    TUint32 totalBytesFlushed = BrCtlInterface().ClearCache();
        if(afterQuery || totalBytesFlushed)
    	{

    	HBufC* resultText = NULL;
    	HBufC* tempText = NULL;
    	TBuf<12> totalText;

    	totalText.Num ((TInt)totalBytesFlushed);

    	resultText = StringLoader::LoadLC( R_TEXT_WML_CACHE_EMPTIED_NOTE);
    	//CleanupStack::PushL(resultText);
    	tempText = HBufC::NewLC( resultText->Length() + totalText.Length() );
    	//CleanupStack::PushL(tempText);

    	tempText->Des().AppendFormat( resultText->Des(), totalBytesFlushed );



    	if(aShowDoneNote)
    	    {
    	    TBrowserDialogs::InfoNoteL ( R_BROWSER_OK_NOTE, tempText->Des() );
    	    }
     	CleanupStack::PopAndDestroy(2); //resultText, tempText
    	}
    }

// ---------------------------------------------------------
// CBrowserAppUi::ClearFormAndPasswdDataL
// ---------------------------------------------------------
//
void CBrowserAppUi::ClearFormAndPasswdDataL(TBool aShowPromptAndComplete)
    {
    if (aShowPromptAndComplete)
        {
        CAknQueryDialog* query = CAknQueryDialog::NewL();
	    if ( query->ExecuteLD( R_BROWSER_CLEARFORMPASSWDDATA_QUERY ) )
            {
            // call brctl to clear form and password data
            BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandClearAutoFormFillData +
    		       						     (TInt)TBrCtlDefs::ECommandIdBase );

            BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandClearAutoFormFillPasswordData +
     		 								 (TInt)TBrCtlDefs::ECommandIdBase );

            TBrowserDialogs::InfoNoteL(
                R_BROWSER_OK_NOTE, R_QTN_BROWSER_NOTE_FORM_AND_PASSWD_DATA_CLEARED );
            }

        }
    else
        {
            // call brctl to clear form and password data
            BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandClearAutoFormFillData +
    		       						     (TInt)TBrCtlDefs::ECommandIdBase );

            BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandClearAutoFormFillPasswordData +
     		 								 (TInt)TBrCtlDefs::ECommandIdBase );

        }
    }


// ---------------------------------------------------------
// CBrowserAppUi::ClearHistoryWithPromptL
// ---------------------------------------------------------
//
void CBrowserAppUi::ClearHistoryWithPromptL()
    {
    CAknQueryDialog* query = CAknQueryDialog::NewL();

    if ( query->ExecuteLD( R_BROWSER_CLEARHISTORYDATA_QUERY ) )
        {
        ClearHistoryL();
        TBrowserDialogs::InfoNoteL( R_BROWSER_OK_NOTE,
                    R_QTN_BROWSER_NOTE_HISTORY_CLEARED );
        }
    }

// ---------------------------------------------------------
// CBrowserAppUi::ClearHistoryL
// ---------------------------------------------------------
//
void CBrowserAppUi::ClearHistoryL()
    {
	if (iWindowManager)
        {
        iWindowManager->SendCommandToAllWindowsL(
        (TInt)TBrCtlDefs::ECommandClearHistory + (TInt)TBrCtlDefs::ECommandIdBase);
        }
    else
        {
        BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandClearHistory +
                                         (TInt)TBrCtlDefs::ECommandIdBase );

        }

    if (!(Preferences().AdaptiveBookmarks()==EWmlSettingsAdaptiveBookmarksOff ))
        {
        // This call will ensure that the bookmarks are cleared when we are
        // outside the bookmarksview since the next call doesn't do anything in
        // that case due to the unset Container pointer.
		iRecentUrlStore->ClearData();

        // Below will serve for when we are called from BookmarksView to ensure
        // thorough clearing and view refresh after clearing adapt bookmarks
        iBookmarksView->HandleCommandL(EWmlCmdClearAdaptiveBookmarksNoPrompt);
        }

    UpdateCbaL();

    }

// ---------------------------------------------------------
// CBrowserAppUi::ClearAllPrivacyL
// ---------------------------------------------------------
//
void CBrowserAppUi::ClearAllPrivacyL()
    {

    CAknQueryDialog* query = CAknQueryDialog::NewL();
	if ( query->ExecuteLD( R_BROWSER_CLEARPRIVACYDATA_QUERY ) )
        {
        ClearTheCacheL(EFalse, EFalse);     // false -> no prompt or info note after op
        DeleteCookiesL();
        ClearHistoryL();
        ClearFormAndPasswdDataL(EFalse);    // no prompt or completed info note

        TBrowserDialogs::InfoNoteL(
            R_BROWSER_OK_NOTE, R_QTN_BROWSER_NOTE_CLEAR_ALL_PRIVACY_DONE );
        }
    }


// -----------------------------------------------------------------------------
// CBrowserAppUi::DisconnectL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::DisconnectL()
    {
//#pragma message("TODO: inform LoadObserver?")
    SendDisconnectEventL();
    Connection().Disconnect();

#ifdef __RSS_FEEDS
    iFeedsClientUtilities->DisconnectFeedsViewL();
    //notify feeds engine to close the connection
	iFeedsClientUtilities->DisconnectManualUpdateConnectionL();
#endif
    }


// -----------------------------------------------------------------------------
// CBrowserAppUi::BmOTABinSenderL()
// -----------------------------------------------------------------------------
//
MBmOTABinSender& CBrowserAppUi::BmOTABinSenderL()
    {
    if (!iSender)
    	{
		iLateSendUi->Cancel();
    	DoConstructSendUiL();
    	}
    return *iSender;
    }


// -----------------------------------------------------------------------------
// CBrowserAppUi::ContentView()
// -----------------------------------------------------------------------------
//
CBrowserContentView* CBrowserAppUi::ContentView() const
    {
    return (CBrowserContentView*)View( KUidBrowserContentViewId );
    }


// -----------------------------------------------------------------------------
// CBrowserAppUi::FetchL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::FetchL( const TDesC& aUrl, CBrowserLoadObserver::TBrowserLoadUrlType aUrlType )
    {
    TFavouritesWapAp ap;
    ap.SetDefault();
    FetchL( aUrl, KNullDesC, KNullDesC, ap, aUrlType );
    }

// TO DO: done by BaP
// -----------------------------------------------------------------------------
// CBrowserAppUi::CancelFetch
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::CancelFetch( TBool aIsUserInitiated /*= EFalse*/ )
    {
    LoadObserver().DoEndLoad( aIsUserInitiated );
    TRAP_IGNORE( BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandCancelFetch + (TInt)TBrCtlDefs::ECommandIdBase ) );
    SetContentDisplayed(ETrue);
    }


// ----------------------------------------------------------------------------------------
// CBrowserAppUi::HandleMessageL
//
// Changes the URL to another if WAP is running before Wap Url
// is started from Desk
// ----------------------------------------------------------------------------------------
//
MCoeMessageObserver::TMessageResponse CBrowserAppUi::HandleMessageL(
																	TUint32 aClientHandleOfTargetWindowGroup,
																	TUid aMessageUid,
																	const TDesC8& aMessageParameters )
    {
    iBrowserAlreadyRunning = ETrue;
    TApaTaskList taskList( CEikonEnv::Static()->WsSession() );
    TUid wapUid = KUidBrowserApplication;
    TApaTask task = taskList.FindApp( wapUid );
    task.BringToForeground();
    //Check for aMessageUid. If it is in array then set iSameWinApp = ETrue
    iSameWinApp = IsSameWinApp(aMessageUid); 
    if ( aMessageParameters.Compare( KLongZeroIdString ) )
        {
        ParseAndProcessParametersL( aMessageParameters );
        }
    return CAknViewAppUi::HandleMessageL(
		aClientHandleOfTargetWindowGroup,
        aMessageUid,
		aMessageParameters);
    }

// ----------------------------------------------------------------------------------
// CBrowserAppUi::UpdateNaviPaneL
// ----------------------------------------------------------------------------------
//
void CBrowserAppUi::UpdateNaviPaneL( TDesC& aStatusMsg )
    {
    ContentView()->UpdateNaviPaneL( aStatusMsg );
    }

// ----------------------------------------------------------------------------------
// CBrowserAppUi::StopDelay
// ----------------------------------------------------------------------------------
//
TInt CBrowserAppUi::StopDelay( TAny* aCBrowserAppUi )
    {
	__ASSERT_DEBUG(aCBrowserAppUi, Util::Panic( Util::EUninitializedData ));

    ((CBrowserAppUi*)aCBrowserAppUi)->DoStopDelay();
    return 0;
    }


// ----------------------------------------------------------------------------------
// CBrowserAppUi::DoStopDelay
// ----------------------------------------------------------------------------------
//
void CBrowserAppUi::DoStopDelay()
    {
    iWait.AsyncStop();
    }

// ----------------------------------------------------------------------------------
// CBrowserAppUi::DelayedSendUiConstructL
// ----------------------------------------------------------------------------------
//
/*static*/ TInt CBrowserAppUi::DelayedSendUiConstructL( TAny* aCBrowserAppUi )
	{
	__ASSERT_DEBUG(aCBrowserAppUi, Util::Panic( Util::EUninitializedData ));
	TRAP_IGNORE(
				((CBrowserAppUi*)aCBrowserAppUi)->DoConstructSendUiL();
				);
	return KErrNone;
	}


// ----------------------------------------------------------------------------------
// CBrowserAppUi::DoConstructSendUiL
// ----------------------------------------------------------------------------------
//
void CBrowserAppUi::DoConstructSendUiL()
	{
	__ASSERT_DEBUG(!iSender, Util::Panic( Util::EUnExpected ));
	iSender = CWmlBrowserBmOTABinSender::NewL();
	}

// -----------------------------------------------------------------------------------
// CBrowserAppUi::SetViewToReturnOnClose
// -----------------------------------------------------------------------------------
//
void CBrowserAppUi::SetViewToReturnOnClose( TUid const &aUid )
    {
    iViewToReturnOnClose.iUid = aUid.iUid;
    }

// -----------------------------------------------------------------------------------
// CBrowserAppUi::CloseContentViewL
// -----------------------------------------------------------------------------------
//
void CBrowserAppUi::CloseContentViewL()
    {
LOG_ENTERFN("CBrowserAppUi::CloseContentViewL");
	CBrowserAppDocument* doc = STATIC_CAST(CBrowserAppDocument*, Document());

    // Close should Exit asynchronously if called from another application 
    // & if not in feeds view
#ifdef __RSS_FEEDS
	if(CalledFromAnotherApp() && (!IsEmbeddedModeOn()) 
    	&& (GetPreviousViewFromViewHistory() != KUidBrowserFeedsFolderViewId)
    	&& (GetPreviousViewFromViewHistory() != KUidBrowserFeedsTopicViewId)
    	&& (GetPreviousViewFromViewHistory() != KUidBrowserFeedsFeedViewId ))
	    {
        SetExitInProgress( ETrue );
        BROWSER_LOG( ( _L( " iBrowserAsyncExit->Start()" ) ) );
        iBrowserAsyncExit->Start();
	    }
#else
	if(CalledFromAnotherApp() && !IsEmbeddedModeOn()) 
	    {
        SetExitInProgress( ETrue );
        BROWSER_LOG( ( _L( " iBrowserAsyncExit->Start()" ) ) );
        iBrowserAsyncExit->Start();
	    }
#endif // __RSS_FEEDS
   	    
	else
	    {	    
	if ( IsEmbeddedModeOn() &&
        doc->GetFolderToOpen() == KFavouritesRootUid )
		{
		ExitBrowser(ETrue);
		}
	else
		{
        if ( ContentView()->GetPreviousViewID() == KUidBrowserBookmarksViewId )
            {
            if ( GetBookmarksView()->GetAdaptiveBookmarksFolderWasActive() )
                {
                GetBookmarksView()->OpenAdaptiveBookmarksWhenActivated();
                }
            SetViewToBeActivatedIfNeededL( KUidBrowserBookmarksViewId );
            }
        else if ( IsEmbeddedModeOn())
        {
         if( !iExitInProgress )
          {
           ExitBrowser( ETrue );
          }
        }
        else
            {
               	if(CalledFromAnotherApp() && (!IsEmbeddedModeOn()))
               	    {
                    SetViewToBeActivatedIfNeededL( GetPreviousViewFromViewHistory());
  	       	        SetCalledFromAnotherApp(EFalse);
               	    SendBrowserToBackground();
               	    }                	
                else
                    {                  
                    SetViewToBeActivatedIfNeededL( iViewToReturnOnClose );
                    }
                }
            }
		}
	}

// -----------------------------------------------------------------------------------
// CBrowserAppUi::IsEmbeddedModeOn
// -----------------------------------------------------------------------------------
//
TBool CBrowserAppUi::IsEmbeddedModeOn() const
	{
	return iEikonEnv->StartedAsServerApp();
	}

// -----------------------------------------------------------------------------------
// CBrowserAppUi::HistoryLoadPrevious
// -----------------------------------------------------------------------------------
//
void CBrowserAppUi::HistoryLoadPrevious()
    {
    if( BrCtlInterface().NavigationAvailable( TBrCtlDefs::ENavigationBack ) )
        {
	    TRAP_IGNORE( BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandBack +
										    (TInt)TBrCtlDefs::ECommandIdBase ) );
        }
    }

// -----------------------------------------------------------------------------------
// CBrowserAppUi::HistoryLoadNext
// -----------------------------------------------------------------------------------
//
void CBrowserAppUi::HistoryLoadNext()
    {
    if( BrCtlInterface().NavigationAvailable( TBrCtlDefs::ENavigationForward ) )
        {
        LoadObserver().DoStartLoad( CBrowserLoadObserver::ELoadUrlTypeOther );
        BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandForward +
                                                (TInt)TBrCtlDefs::ECommandIdBase );
        }
    }


// ---------------------------------------------------------------------------
// CBrowserAppUi::IsShutdownRequested
// ---------------------------------------------------------------------------
//
TBool CBrowserAppUi::IsShutdownRequested() const
	{
    return iShutdownRequested;
	}

// ---------------------------------------------------------------------------
// CBrowserAppUi::IsEmbeddedInOperatorMenu
// ---------------------------------------------------------------------------
//
TBool CBrowserAppUi::IsEmbeddedInOperatorMenu() const
	{
	const TUid KUidOperatorMenuApp = { 0x10008D5E };
	const TUid KUidVideoServicesApp = { 0x10281893 }; 

	// If the embedding application is the Operator Menu or Video Services
	if ( iEmbeddingApplicationUid == KUidOperatorMenuApp || iEmbeddingApplicationUid == KUidVideoServicesApp )
		{
		return ETrue;
		}
	else
		{
		return EFalse;
		}
	}

// ---------------------------------------------------------------------------
// CBrowserAppUi::IsAppShutterActive
// ---------------------------------------------------------------------------
//
TBool CBrowserAppUi::IsAppShutterActive() const
	{
	CAknAppUi* shuttingApp = CAknEnv::AppWithShutterRunning();

	if ( shuttingApp )
		{
		return ETrue;
		}
	else
		{
		return EFalse;
		}
	}

// ---------------------------------------------------------------------------
// CBrowserAppUi::FetchHomePage
// --------------------------------------------------------------------------
//
void CBrowserAppUi::FetchHomePageL()
    {
LOG_ENTERFN( "CBrowserAppUi::FetchHomePageL" );
    if ( !Fetching() )
        {
        UpdateSoftKeys();

        // If the last active view is content view, return to the bookmark view on close.
        // Otherwise if the last active view is bookmark view and we return to the appropriate view on close.
        TUid lastViewId = LastActiveViewId();
        if( lastViewId == KUidBrowserContentViewId ||
            lastViewId == KUidBrowserNullViewId )
            {
            lastViewId = KUidBrowserBookmarksViewId;
            }
        SetViewToReturnOnClose( lastViewId );

        if( iLongZeroPressed )
            {
			BROWSER_LOG( ( _L( " LongZeroPressed." ) ) );

            // There should only be one special load observer at startup
            SpecialLoadObserver().SetLongZeroStartup( iLongZeroPressed );
            iLongZeroPressed = EFalse;
            HBufC* buf = HBufC::NewLC( KMaxHomePgUrlLength );
            TPtr ptr( buf->Des() );
            TBool homePgFound( KErrNotFound );

            // try to read Access Points homepage
            TUint defaultAp( KWmlNoDefaultAccessPoint );
            // 'always ask' feature doesn't let to read the AP
            if( Preferences().AccessPointSelectionMode() == EConnectionMethod )
                {
                defaultAp = Preferences().DefaultAccessPoint();
                }
            if ( defaultAp != KWmlNoDefaultAccessPoint ) // There is an access point defined
                {
                CApAccessPointItem* apItem = Preferences().AllPreferencesL().iDefaultAPDetails;
                if ( apItem )
                    {
                    const HBufC* defaultHP = apItem->ReadConstLongTextL( EApWapStartPage );
                    if ( defaultHP->Length() )
                        {
                        ptr.Zero();
                        ptr.Append( *defaultHP );
                        homePgFound = KErrNone;
                        }
                    }
                }
            // in case of error, read user defined home page from SD
            TInt err( KErrNone );
            if( homePgFound == KErrNotFound ) 
                {
                // do not let leaving
                if ((Preferences().HomePageType() == EWmlSettingsHomePageAddress)
                   ||(Preferences().HomePageType() == EWmlSettingsHomePageUseCurrent))
                	{                         
                	TRAP( err, homePgFound = Preferences().HomePageUrlL( ptr, ETrue ) );
                	}
                	}
            // we have a url to load
            if( err == KErrNone &&
                homePgFound == KErrNone &&
                ptr.Length() > 0 )
                {
                TRAP( err, FetchL( ptr, CBrowserLoadObserver::ELoadUrlTypeOther ) );
                // in case of any error switch back to BookmarksView
                if( err != KErrNone )
                    {
                    ContentView()->SetFullScreenOffL();
                    SetLastActiveViewId( KUidBrowserBookmarksViewId );
                    }
                }
            else
                {
                // we already switched to ContentView, so change the layout back
                ContentView()->SetFullScreenOffL();
                SetLastActiveViewId( KUidBrowserBookmarksViewId );
                }
            CleanupStack::PopAndDestroy( buf );
            }
        else
            {
            // it checks HomePageSettings
            LaunchHomePageL( /* EFalse */ );
            }
        }
    }

// ---------------------------------------------------------------------------
// CBrowserAppUi::StartPreferencesViewL
// ---------------------------------------------------------------------------
//
TBool CBrowserAppUi::StartPreferencesViewL( TUint aListItems )
    {
    TBool retVal( EFalse );
	TInt retUi( KErrNone );
    TInt err(KErrNone);

	//start the connection  dialog
	CCmApplicationSettingsUi* settings = CCmApplicationSettingsUi::NewL();
	CleanupStack::PushL( settings );
	TCmSettingSelection selection;
    // empty filter array because no filtering is wanted
    TBearerFilterArray filterArray;
	TRAP ( err, retUi = settings->RunApplicationSettingsL( selection, aListItems, filterArray ) );
	CleanupStack::PopAndDestroy(); // settings
    
	//if something has been selected
	if (retUi)	
	    {
	    //set the selection mode
        iPreferences->SetAccessPointSelectionModeL( 
                STATIC_CAST( TCmSettingSelectionMode, selection.iResult ) );

	    //based on the chosen connection type, store the connection identifier(iapid, snap id, always ask)
	    //in the preferences list
	    switch ( iPreferences->AccessPointSelectionMode() )
		    {			
		    case EConnectionMethod:
			    {
			    TUint32 id = iPreferences->DefaultAccessPoint();
                // CMManager gives us IAPid, need to translate to WAPid
			    if (selection.iId != 0)
			        {                   		
			        id = Util::WapIdFromIapIdL( *this, selection.iId ); 
			        }			    			    
			    iPreferences->SetDefaultAccessPointL( id );
                // Requested AP is preset for PushMtm
			    id = iPreferences->DefaultAccessPoint();
                if( id != KWmlNoDefaultAccessPoint )
                    {
    			    SetRequestedAP( id );
                    retVal = ETrue;
                    }
			    break;	
			    }

		    case EDestination:
			    {
			    TUint32 snapId = iPreferences->DefaultSnapId();
			    if (selection.iId != 0)
			        {                   		
			        snapId = selection.iId;
			        }			        
			    iPreferences->SetDefaultSnapId( snapId );
                if( snapId != KWmlNoDefaultSnapId )
                    {
                    retVal = ETrue;
                    }
			    break;	
			    }

		    case EAlwaysAsk:
		    default:
			    {
			    break;	
			    }
		    }			
	    }

    // alr: is return val needed?  should it be true or false in EAA and default?  check how it's used
    return retVal;
    }
// ---------------------------------------------------------------------------
// CBrowserAppUi::AhleConnection
// ---------------------------------------------------------------------------
//
CRecentUrlStore* CBrowserAppUi::RecentUrlStore()
    {
    return iRecentUrlStore;
    }

// ---------------------------------------------------------------------------
// CBrowserAppUi::SomeItemsNotSecure
// ---------------------------------------------------------------------------
//
TBool CBrowserAppUi::SomeItemsNotSecure() const
    {
    return !( LoadObserver().LoadStatus( CBrowserLoadObserver::ELoadStatusAllItemIsSecure ) );
    }

// ----------------------------------------------------
// CBrowserAppUi::IsProgressShown
// ----------------------------------------------------
//
TBool CBrowserAppUi::IsProgressShown() const
    {
    return iShowProgress;
    }

// ----------------------------------------------------
// CBrowserAppUi::SetProgressShown
// ----------------------------------------------------
//
void CBrowserAppUi::SetProgressShown( TBool aProgressShown )
    {
    iShowProgress = aProgressShown;
    }

// ----------------------------------------------------
// CBrowserAppUi::RecognizeUiFileL
// ----------------------------------------------------
//
TBool CBrowserAppUi::RecognizeUiFileL( RFile& aFile, TBuf<KMaxDataTypeLength>& aDataTypeDes  )
	{
	TBool recognized = EFalse;
	HBufC8* buffer = NULL;

	TFileName fileName;
	User::LeaveIfError(aFile.FullName(fileName));

    TInt fileSize;

	// Read the file from the beginning into a buffer
	User::LeaveIfError(aFile.Size(fileSize));
	buffer = HBufC8::NewLC(fileSize);
    TPtr8 bufferPtr(buffer->Des());
    TInt startPos = 0;
    User::LeaveIfError(aFile.Read(startPos, bufferPtr, fileSize));
    // move the seek position back to start
    User::LeaveIfError(aFile.Seek(ESeekStart, startPos));

    _LIT( KDotOPML, ".opml");
    _LIT( KDotXML, ".xml");

    // First try the name extension
    if ( fileName.Length() >= 3 )
        {
        TInt dotPos = fileName.LocateReverse( '.' );
        if ( dotPos != KErrNotFound )
            {
            TInt extLength = fileName.Length() - dotPos;
            HBufC* ext = fileName.Right( extLength ).AllocL();
            CleanupStack::PushL( ext );

            // if .opml return true
            if ( ext->CompareF( KDotOPML )  == 0)
            	{
				aDataTypeDes = KOPMLMimeType;
            	recognized = ETrue;
            	}
            else if (ext->CompareF( KDotXML ) == 0 )
                {
                _LIT8( dtd, "<opml" );
				if( bufferPtr.FindF( dtd ) != KErrNotFound )
					{
					aDataTypeDes =  KOPMLMimeType;
					recognized = ETrue;
					}
               	}
            CleanupStack::PopAndDestroy();	// ext
            }
        }

		if (!recognized)
			{
			// just check content of buffer
        	_LIT8( dtd, "<opml" );
			if( (bufferPtr.Length() > 5) &&
			    (bufferPtr.FindF( dtd ) != KErrNotFound ))
				{
				aDataTypeDes = KOPMLMimeType;
				recognized = ETrue;
				}
        	}

	CleanupStack::PopAndDestroy(); //buffer

	return recognized;
	}


// ---------------------------------------------------------
// CBrowserAppUi::ConfirmUiHandlingL
// ---------------------------------------------------------
//
TBool CBrowserAppUi::ConfirmUiHandlingL( TFileName& aFilename, TBuf<KMaxDataTypeLength>& aDataTypeDes  )
	{
	// switch on data type desc and handle file accordingly
	if(aDataTypeDes.Compare( KOPMLMimeType ) == 0)
		{
		// Display confirmation dialog before activating feeds view
		HBufC* message = StringLoader::LoadLC(R_FEEDS_QUERY_IMPORT_NOW, aFilename);
		HBufC* lsc_yes = StringLoader::LoadLC(R_FEEDS_IMPORT_NOW_YES);
		HBufC* rsc_no = StringLoader::LoadLC(R_FEEDS_IMPORT_NOW_NO);

		TBool ret(iDialogsProvider->DialogConfirmL( _L(""), *message, *lsc_yes, *rsc_no ));

		CleanupStack::PopAndDestroy(3); //message, lsc_yes, rsc_no

		if(ret)
			{
			return ETrue;
			}
		}

	return EFalse;
	}

// ---------------------------------------------------------
// CBrowserAppUi::CopyUiRFileToFileL
// ---------------------------------------------------------
//

void CBrowserAppUi::CopyUiRFileToFileL( RFile& aFile, const TDesC& aFilepath )
	{
	//
	// copy r-file to a temporary file
	//
	RFs tempRFs;
	RFile tempRFile;
	TInt fileSize;
	HBufC8* buffer = NULL;

	// Connect to file server
	User::LeaveIfError(tempRFs.Connect());
    CleanupClosePushL(tempRFs);

	// Create the filepath to copy to
	// Replace the file if it already exists
	User::LeaveIfError(tempRFile.Replace(tempRFs, aFilepath, EFileWrite));
	CleanupClosePushL(tempRFile);

	// Copy the file
	User::LeaveIfError(aFile.Size(fileSize));
	buffer = HBufC8::NewLC(fileSize);
    TPtr8 bufferPtr(buffer->Des());

	// Read from the start of the file
    User::LeaveIfError(aFile.Read(0, bufferPtr, fileSize));
	User::LeaveIfError(tempRFile.Write(bufferPtr, fileSize));

	tempRFile.Close();
	tempRFs.Close();

	// Cleanup Buffer and file copy stuff
	CleanupStack::PopAndDestroy(3); //buffer, tempRFile, tempRFs
	}

// ---------------------------------------------------------
// CBrowserAppUi::HandleUiFileL
// ---------------------------------------------------------
//
void CBrowserAppUi::HandleUiFileL( TFileName& aFilepath, TBuf<KMaxDataTypeLength>& aDataTypeDes )
	{
	// switch on data type desc and handle file accordingly
	if(aDataTypeDes.Compare(KOPMLMimeType) == 0)
		{
		// Activate Content view first in order to
		// get it's BrowserControl instance ready to go
		// (as opposed to Feeds' BC instance) to be used to render
		// feeds content in case the user wants to read
		// the feed in-page.
		SetLastActiveViewId( KUidBrowserContentViewId );
		SetViewToBeActivatedIfNeededL( LastActiveViewId() );
		ActivateLocalViewL( LastActiveViewId() );

		// Now activate bookmarks view
		SetLastActiveViewId( KUidBrowserBookmarksViewId );
		SetViewToBeActivatedIfNeededL( LastActiveViewId() );
		ActivateLocalViewL( LastActiveViewId() );
		HandleForegroundEventL( ETrue );

		// wait until the content view is completely ready
		WaitCVInit();

		// begin import, pass to feeds view
		iFeedsClientUtilities->BeginImportOPMLFeedsFileL(aFilepath);
		}
	}

// ---------------------------------------------------------
// CBrowserAppUi::HandleUiDownloadL
// ---------------------------------------------------------
//
void CBrowserAppUi::HandleUiDownloadL( TFileName& aFilepath, TBuf<KMaxDataTypeLength>& aDataTypeDes )
	{
	// switch on data type desc and handle file accordingly
	if(aDataTypeDes.Compare(KOPMLMimeType) == 0)
		{
		SetLastActiveViewId( KUidBrowserFeedsFolderViewId );
		// begin import
		iFeedsClientUtilities->BeginImportOPMLFeedsFileL(aFilepath);
        // switch back to FeedsView, so change layout
        ContentView()->SetFullScreenOffL();
		}
	}
	
// ---------------------------------------------------------
// CBrowserAppUi::LaunchIntoFeedsL
// ---------------------------------------------------------
//
void CBrowserAppUi::LaunchIntoFeedsL()
{
    if(iBrowserAlreadyRunning && (LastActiveViewId() == KUidBrowserContentViewId))
		{
		iFeedsClientUtilities->ShowFolderViewL(KUidBrowserContentViewId);         	
		}
	else
		{
		iFeedsClientUtilities->ShowFolderViewL();
		}
}
	
// ----------------------------------------------------
// CBrowserAppUi::CreateFullUrlLC
// ----------------------------------------------------
//
HBufC* CBrowserAppUi::CreateFullUrlLC(
        const TDesC& aUrl,
        const TDesC& aUsername,
        const TDesC& aPassword )
    {
    // username[:password]@server/document
    TInt lPw = aPassword.Length();
    TInt lUn = aUsername.Length();
    _LIT( KBrowserScheme, "://" );
    TInt index = aUrl.Find( KBrowserScheme );
    if( index == KErrNotFound )
    	{
        index = 0;  // insert at first position
    	}
    else
    	{
    	index += 3;  // insert after scheme
    	}

    HBufC *fullUrl = HBufC::NewLC( aUrl.Length() + lUn + lPw + 3 );
    fullUrl->Des().Copy( aUrl );
    if( lPw + lUn )
        {
        fullUrl->Des().Insert( index, _L( "@" ) );
        }
    if( lPw )
        {
        fullUrl->Des().Insert( index, aPassword );
        fullUrl->Des().Insert( index, _L( ":" ) );
        }
    if( lUn )
        {
        fullUrl->Des().Insert( index, aUsername );
        }
    // insert scheme???
    fullUrl->Des().ZeroTerminate();

    return fullUrl;
    }

// ---------------------------------------------------------
// CBrowserAppUi::HandleResourceChangeL( TInt aType )
// ---------------------------------------------------------
//
void CBrowserAppUi::HandleResourceChangeL( TInt aType )
    {
    LOG_ENTERFN("CBrowserAppUi::HandleResourceChangeL");
    LOG_WRITE_FORMAT(" aType: %d", aType);
    if( iExitInProgress )
        {
        BROWSER_LOG( ( _L(" iExitInProgress ETrue ") ) );
        return;
        }

    CAknViewAppUi::HandleResourceChangeL( aType );
    if ( aType == KEikDynamicLayoutVariantSwitch )
        {
        CBrowserViewBase* activeView = ActiveView( );
        if( activeView != NULL )
            {
            activeView->HandleClientRectChange();
            if ( ContentView() && (activeView != ContentView()) )
                {
                ContentView()->HandleClientRectChange();
                }
            }
        }
    if (aType == KEikMessageFadeAllWindows)
        {
        if (ContentView() && ContentView()->PenEnabled())
            {
            if (iCurrentView == KUidBrowserContentViewId)
                {
                ContentView()->Toolbar()->DisableToolbarL(ETrue);
                }
            }
        }

    if (aType == KEikMessageUnfadeWindows)
        {
        if (ContentView()&& ContentView()->PenEnabled() && !ExitInProgress())
            {
            if (iCurrentView == KUidBrowserContentViewId)
                {
                ContentView()->MakeZoomSliderVisibleL(EFalse);
                ContentView()->Toolbar()->DisableToolbarL(EFalse);
                }
            }
        }
    }

// ---------------------------------------------------------
// CBrowserAppUi::ChangeConnectionL()
// ---------------------------------------------------------
//
void CBrowserAppUi::ChangeConnectionL()
    {
    TConManChangeConn change;
    // This will contain the new IAP as well, but it needs to pass the old one
    TUint32 newAp = Util::IapIdFromWapIdL( *this, RequestedAp() );
    iConnection->ChangeIapL( change, newAp );
    if( EConManDoNothing == change )
        {
        // Do nothing
        }
    else
        {
        //cancel transaction
        SendDisconnectEventL();
        /* start all */
        if( EConManCloseAndStart == change )
            {
            //close connection
            iConnection->Disconnect();

#ifdef __RSS_FEEDS
            //notify feeds engine to close the connection
        	iFeedsClientUtilities->DisconnectManualUpdateConnectionL();
#endif
            }
        iConnection->SetRequestedAP( newAp );
        TInt err = iConnection->StartConnectionL( ETrue );
        if( err == KErrNone )
            {
            iConnection->ShowConnectionChangedDlg();
            }
        }
    }

// ---------------------------------------------------------
// CBrowserAppUi::HandleContentL()
// ---------------------------------------------------------
//
TBool CBrowserAppUi::HandleContentL( const TDesC& aFileName,
                                      const CAiwGenericParamList& aParamList,
                                      TBool& aContinue )
    {
    // Put the file name to the generic parameter list, if it is not there yet
    TInt index(0);
    aParamList.FindFirst( index, EGenericParamFile, EVariantTypeDesC );
    if ( index == KErrNotFound )
    	{
        TAiwVariant paramVariant( aFileName );
        TAiwGenericParam genericParam( EGenericParamFile, paramVariant );
        CAiwGenericParamList* paramList = (CAiwGenericParamList*)&aParamList;
        paramList->AppendL( genericParam );
    	}

	// Set EGenericParamAllowMove - we allow handlers to apply move instead
	// of copy for the file.
    aParamList.FindFirst( index, EGenericParamAllowMove, EVariantTypeAny );
    if ( index == KErrNotFound )
    	{
    	TBool allowMove( ETrue );
        TAiwVariant allowMoveVariant( allowMove );
        TAiwGenericParam genericParamAllowMove( EGenericParamAllowMove, allowMoveVariant );
        CAiwGenericParamList* paramList = (CAiwGenericParamList*)&aParamList;
        paramList->AppendL( genericParamAllowMove );
    	}

    BrowserLauncherService()->DownloadFinishedL( KErrNone, aParamList );

    // Wait until the client sends two TBools (contentHandled & continueBrowsing).
    iClientWantsToContinue = EFalse;
    iWasContentHandled = EFalse;

    iLauncherServiceWait.Start();

    // iClientWantsToContinue and iWasContentHandled contains the results
    aContinue = iClientWantsToContinue;
    return iWasContentHandled;
    }

// ---------------------------------------------------------
// CBrowserAppUi::DownloadedContentHandlerReserved1()
// ---------------------------------------------------------
//
TAny* CBrowserAppUi::DownloadedContentHandlerReserved1( TAny* /*aAnyParam*/ )
    {
    return 0;
    }

// ---------------------------------------------------------
// CBrowserAppUi::SetContentHandlingResult()
// ---------------------------------------------------------
//
void CBrowserAppUi::SetContentHandlingResult
    ( TBool aClientWantsToContinue, TBool aWasContentHandled )
    {
    iClientWantsToContinue = aClientWantsToContinue;
    iWasContentHandled = aWasContentHandled;

    // continue handling the content
    iLauncherServiceWait.AsyncStop();
    }

// ---------------------------------------------------------
// CBrowserAppUi::LoadSearchPageL()
// ---------------------------------------------------------
//
void CBrowserAppUi::LoadSearchPageL()
	{
    TPtrC url = iPreferences->SearchPageUrlL();

    // url undefined
    if ( !url.Length() )
        {
        HBufC* query = StringLoader::LoadLC(
                                    R_BROWSER_QUERY_DEF_SEARCHPAGE );
        // Ask if user want to define the search web setting now
        if (
            TBrowserDialogs::DialogQueryReqL( query->Des(),
                                                KNullDesC,
                                                KNullDesC )
            )
            {
            HBufC* prompt = StringLoader::LoadLC(
                                        R_BROWSER_PROMPT_SEARCH_URL );
            HBufC* retBuf = HBufC::NewLC( KMaxSearchPgUrlLength );
            TPtr retString = retBuf->Des();
            retString.Copy( KNullDesC );

            TInt result = TBrowserDialogs::DialogPromptReqL(
                                             prompt->Des(),
                                             &retString,
                                             ETrue,
                                             KMaxSearchPgUrlLength
                                             );

            if( result )  // URL was accepted
                {
                Preferences().SetSearchPageUrlL( retString );
                url.Set( iPreferences->SearchPageUrlL() );
                }
            CleanupStack::PopAndDestroy( 2 ); // retBuf, prompt
            }
        CleanupStack::PopAndDestroy( query );
        }

    FetchL( url );
	}

// ---------------------------------------------------------
// CBrowserAppUi::CloseWindowL()
// ---------------------------------------------------------
//
void CBrowserAppUi::CloseWindowL( TInt aWindowId )
    {
LOG_ENTERFN("AppUi::CloseWindowL");

	//There are only two cases for closing window. One is user initialized and the
	//other is script initialized. For both cases, the window should be deleted.
    TBool forceDelete( ETrue );
    // this variable is useful to know if the window has a parent window which needs to be activated on closing current window
    TBool parentPresent(EFalse);
    if ( WindowMgr().CurrentWindowQue() && WindowMgr().CurrentWindowQue()->iParent )
        {
        parentPresent = ETrue;
        }
    // delete the current window by default
    if( aWindowId == 0 )
        {
        aWindowId = WindowMgr().CurrentWindow()->WindowId();
        // scripts directly pass a real windowId,
        // so if user initiated the process, we really delete the window
        forceDelete = ETrue;
        }
    //If this window has been created from Search app
    if(aWindowId == iWindowIdFromFromExternApp)
    {
        //Make it false as window is going to close down
        iSameWinApp = EFalse;
    }
#ifdef __RSS_FEEDS
	// If we are closing a Feeds Full Story then go back to feeds
	TBool feedsWindow(EFalse);
	CBrowserWindow* win;
	TInt err = WindowMgr().Window(aWindowId, win);
	if (err == KErrNone)
		{
		feedsWindow = win->HasFeedsContent();
		win->SetHasFeedsContent(EFalse);
		}
#endif

	TInt winId = WindowMgr().DeleteWindowL( aWindowId, forceDelete );

#ifdef __RSS_FEEDS
	if (feedsWindow)
		{
		SetViewToBeActivatedIfNeededL( KUidBrowserFeedsFeedViewId );
		return;
		}
#endif

    if( winId > 0 )  // still remain a window to be activated
        {
   	    WindowMgr().SwitchWindowL( winId );
        if(CalledFromAnotherApp() && (!IsEmbeddedModeOn()) && !parentPresent)
   	        {
   	        SetCalledFromAnotherApp(EFalse);
   	        SendBrowserToBackground();
   	        }
   	    }
   	else  if( winId == 0) // last window was closed
	    {
        if(CalledFromAnotherApp() && (!IsEmbeddedModeOn()))
   	        {
#ifdef __RSS_FEEDS
   	        if ((GetPreviousViewFromViewHistory() == KUidBrowserFeedsFolderViewId) || 
   	            (GetPreviousViewFromViewHistory() == KUidBrowserFeedsTopicViewId) ||
   	            (GetPreviousViewFromViewHistory() == KUidBrowserFeedsFeedViewId))
   	            {
   	            SetViewToBeActivatedIfNeededL( GetPreviousViewFromViewHistory() );
   	            SetCalledFromAnotherApp(EFalse);
   	            SendBrowserToBackground();
   	            }
   	        else
                {
               	ExitBrowser(ETrue);
                }
#else
            ExitBrowser(ETrue);
#endif // __RSS_FEEDS
   	        }
        else
            {
        	ExitBrowser(ETrue);        
            }
    	}
    // else a background window was closed

    }

// ---------------------------------------------------------
// CBrowserAppUi::StartProgressAnimationL
// ---------------------------------------------------------
//
void CBrowserAppUi::StartProgressAnimationL()
    {
LOG_ENTERFN("AppUi::StartProgressAnimationL");
    }

// ---------------------------------------------------------
// CBrowserAppUi::StopProgressAnimationL
// ---------------------------------------------------------
//
void CBrowserAppUi::StopProgressAnimationL()
    {
LOG_ENTERFN("AppUi::StopProgressAnimationL");
    }

#ifdef __RSS_FEEDS
// ---------------------------------------------------------
// CBrowserAppUi::FeedsClientUtilities
// ---------------------------------------------------------
//
CFeedsClientUtilities& CBrowserAppUi::FeedsClientUtilities() const
	{
	return *iFeedsClientUtilities;
	}
#endif  // __RSS_FEEDS

// ---------------------------------------------------------
// CBrowserAppUi::OpenLinkInNewWindowL()
// ---------------------------------------------------------
//
void CBrowserAppUi::OpenLinkInNewWindowL( const CFavouritesItem& aBookmarkItem )
    {
    if( ( aBookmarkItem.IsItem() ) &&
        ( aBookmarkItem.Uid() != KFavouritesRSSItemUid ) )
        {
        // Check if we should/can create a new window
        // In most cases when the feature is not allowed option menu items have been dimmed.
        // In the case where a long key press on a bookmark happens we fall through to this function
        // and check the feature here.  If a new window is not allowed we can open in the current window
        if( IsPageLoaded() &&
        		Preferences().UiLocalFeatureSupported( KBrowserMultipleWindows ) &&
            	!Preferences().UiLocalFeatureSupported( KBrowserMinimalMultipleWindows ))
            {
            // there is already a window, so create a new one
            CBrowserWindow *win = WindowMgr().CreateWindowL( 0, &KNullDesC );
            if (win != NULL)
                {
                CleanupStack::PushL( win );
                WindowMgr().SwitchWindowL( win->WindowId() );
                FetchBookmarkL( aBookmarkItem );
                CleanupStack::Pop();  // win
                }
            }
        else
            {
            // no page is loaded yet, so do not create new window
            FetchBookmarkL( aBookmarkItem );
            }
        }
    }

// ---------------------------------------------------------
// CBrowserAppUi::SendDisconnectEventL
// ---------------------------------------------------------
//
void CBrowserAppUi::SendDisconnectEventL()
    {
    LOG_ENTERFN("CBrowserAppUi::SendDisconnectEventL");
    
    if(!iStartedUp)
        return;
        
    SpecialLoadObserver().CancelConnection();
    CArrayFixFlat<CWindowInfo*>* windows = iWindowManager->GetWindowInfoL( this );
    if( windows )
        {
        TInt i;

        for ( i = 0; i < windows->Count(); i++ )
            {
            CBrowserWindow* window = NULL;
            iWindowManager->Window( windows->At( i )->iWindowId, window );
            if( window )
                {
                TInt err;
           //     BROWSER_LOG( ( _L( " iConnection Connected" ) ) );
                TRAP( err, window->BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandCancelFetch + (TInt)TBrCtlDefs::ECommandIdBase ) );
           //     BROWSER_LOG( ( _L( " HandleCommandL:  %d"), err ) );
                }
            }
        
        CBrowserWindow* window = NULL;
        iWindowManager->Window( windows->At( 0 )->iWindowId, window );
                
        // delete the window items before deleting the array
        for( i=0; i<windows->Count(); ++i )
            delete windows->At( i );

        delete windows;
        }
    }
    /*

// ----------------------------------------------------
// CBrowserAppUi::IsPgNotFound
// ----------------------------------------------------
//
TBool CBrowserAppUi::IsPgNotFound() const
    {
    return iPgNotFound;
    }
*/
// ----------------------------------------------------
// CBrowserAppUi::HandleApplicationSpecificEventL
// ----------------------------------------------------
//
void CBrowserAppUi::HandleApplicationSpecificEventL(TInt aEventType, const TWsEvent& aWsEvent)
    {
    CAknAppUi::HandleApplicationSpecificEventL(aEventType, aWsEvent);

    /*
     * Note:  Even though we get these memory events from the system for handling OOM, and we pass them off
     * to the command handler, there is no code further down the line that actually handles them (it would 
     * normally be in BrCtl).  We totally ignore these events.  This is because the system has too high of an OOM threshold.
     * I.e. the system may only have 6m left and think it's out of memory, however, the browser can still render
     * many pages in only 6m.  So, these system events are ignored and the browser handles OOM with its own mechanism.
     * (See OOMStopper and OOMHandler)
     */
	if(aEventType == KAppOomMonitor_FreeRam )
		{
		iWindowManager->CloseAllWindowsExceptCurrent();
            BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandFreeMemory + (TInt)TBrCtlDefs::ECommandIdBase);
		    }
	else if(aEventType == KAppOomMonitor_MemoryGood)
		{
        BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandMemoryGood + (TInt)TBrCtlDefs::ECommandIdBase);
		}
    else if ( KAknFullOrPartialForegroundGained == aEventType )
        {
        if ( iViewToBeActivatedIfNeeded.iUid )
            {
            ActivateLocalViewL( iViewToBeActivatedIfNeeded );
            }
        else
            {
            // LastActiveViewId() might return with 0!
            TRAP_IGNORE( ActivateLocalViewL( LastActiveViewId() ) );
            }
        iViewToBeActivatedIfNeeded.iUid = 0;
        }
    }

// ----------------------------------------------------
// CBrowserAppUi::OfferApplicationSpecificKeyEventL
//
// Use this function to handle key events that are application wide and not specific to any view
// All views (containers) call this first and should return if key was consumed
// As a note for future reference the red end key shows up as iCode = EKeyNo...though you would
// think it would be EKeyEnd...
// ----------------------------------------------------
//
TKeyResponse CBrowserAppUi::OfferApplicationSpecificKeyEventL(const TKeyEvent& /*aKeyEvent*/,
															  TEventCode /*aType*/)
    {
    TKeyResponse result( EKeyWasNotConsumed );

    return result;
    }

// -----------------------------------------------------------------------------
// CBrowserAppUi::GetPreviousViewFromViewHistory
// -----------------------------------------------------------------------------
//
TUid CBrowserAppUi::GetPreviousViewFromViewHistory()
{
	return iPreviousView;
}

// -----------------------------------------------------------------------------
// CBrowserAppUi::SaveFocusedImageToGalleryL
// -----------------------------------------------------------------------------
//
void CBrowserAppUi::SaveFocusedImageToGalleryL()
    {
    TBrCtlImageCarrier* imageCarrier = BrCtlInterface().FocusedImageLC();
    CDocumentHandler* docHandler = CDocumentHandler::NewLC();
    TUriParser uriParser;
    User::LeaveIfError(uriParser.Parse(imageCarrier->Url()));
    TPtrC path;
    path.Set( uriParser.Extract( EUriPath ) );
    HBufC* fileName = NULL;
    if( path.Length() )
        {
        TInt slash = path.LocateReverse( '/' );
        if( slash != KErrNotFound && slash != path.Length() )
            {
            TPtrC temp( path.Right( path.Length() - slash - 1 ) );
            if( temp.Length() )
                {
                fileName = HBufC::NewLC(temp.Length() + 4); // leave room for edding extenstion if needed
                fileName->Des().Copy(temp);
                }
            }
        }
    HBufC8* dataType8 = NULL;
    dataType8 = HBufC8::NewLC(imageCarrier->ContentType().Length());
    dataType8->Des().Copy(imageCarrier->ContentType());
    TDataType dataType(*dataType8);
    if (fileName)
        {
        TPtr fileNamePtr(fileName->Des());
        docHandler->CheckFileNameExtension(fileNamePtr, dataType);
        docHandler->SaveL(imageCarrier->RawData(), dataType, *fileName, KEntryAttNormal);
        }
    else
        {
        docHandler->SaveL(imageCarrier->RawData(), dataType, KEntryAttNormal);
        }
    if (fileName)
        {
        CleanupStack::PopAndDestroy(2); // fileName, dataType8
        }
    else
        {
        CleanupStack::PopAndDestroy(dataType8); // dataType8
        }

    TFileName fName;
    User::LeaveIfError(docHandler->GetPath(fName));
    
    #ifdef BRDO_APP_GALLERY_SUPPORTED_FF
    
    CMGXFileManager* mgFileManager = MGXFileManagerFactory::NewFileManagerL(CEikonEnv::Static()->FsSession());
    CleanupStack::PushL(mgFileManager);
    if( fName.Length() > 0 )
        {
        TRAP_IGNORE( mgFileManager->UpdateL( fName ) );
        }
    else
        {
        TRAP_IGNORE( mgFileManager->UpdateL() );
        }
    CleanupStack::PopAndDestroy(1); // mgFileManager
    
    #endif
    
    CleanupStack::PopAndDestroy(1); // imageCarrier
    }

// ---------------------------------------------------------
// CBrowserAppUi::CheckFlashPresent
// ---------------------------------------------------------
//
TBool CBrowserAppUi::CheckFlashPresent()
    {
    RImplInfoPtrArray animPluginList;

    TRAP_IGNORE( REComSession::ListImplementationsL( KBrowserPluginInterfaceUid,
                                        animPluginList ) );


	_LIT8(KFlashMimeType,     "application/x-shockwave-flash");

    const TInt count = animPluginList.Count();
    TBool found = EFalse;
    for ( TInt i = 0; i < count; i++ )
        {
        CImplementationInformation* implInfo = animPluginList[i];
        if (( implInfo->DataType( ) ).Find( KFlashMimeType) != KErrNotFound)
            {
            found = ETrue;  //flash plugin is present
            break;
            }
        }
    animPluginList.ResetAndDestroy();

    return found;

    }

// ---------------------------------------------------------
// CBrowserAppUi::FlashPresent
// ---------------------------------------------------------
//
TBool CBrowserAppUi::FlashPresent()
    {
    return iFlashPresent;
    }

// ---------------------------------------------------------
// CBrowserAppUi::ShowNameAndVersionL
// ---------------------------------------------------------
//
void CBrowserAppUi::ShowNameAndVersionL()
{
    HBufC* header = StringLoader::LoadLC( R_BROWSER_QUERY_BROWSER_VERSION );
    HBufC* browserVersion = BrCtlInterface().VersionInfoLC(TBrCtlDefs::EBrowserVersion);

    // output browser version to dialog
    CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL( *browserVersion );
    CleanupStack::PushL(dlg);
    dlg->PrepareLC( R_BROWSER_PAGE_VERSION );
    CleanupStack::Pop(dlg);
    CAknPopupHeadingPane* hPane = dlg->QueryHeading();
    if ( hPane )
        {
        CleanupStack::PushL(hPane);
        hPane->SetTextL( *header );
	    CleanupStack::Pop(hPane);
        }
    dlg->RunLD();
	CleanupStack::PopAndDestroy(2); // browserVersion, header
}


// ---------------------------------------------------------
// CBrowserAppUi::IsLaunchHomePageDimmedL
// ---------------------------------------------------------
//
TBool CBrowserAppUi::IsLaunchHomePageDimmedL()
{
    
    // Set dimmed flag if shortcut key for launch home page is dimmed and was selected
    TBool dimmed = EFalse;
    TBool homepage = Preferences().UiLocalFeatureSupported( KBrowserUiHomePageSetting );
    if( homepage )
        {
        TWmlSettingsHomePage pgtype = Preferences().HomePageType();
        TBool alwaysAskCase = ((Preferences().AccessPointSelectionMode() == EAlwaysAsk ) &&
            ( EWmlSettingsHomePageAccessPoint == pgtype ));
        
        if (alwaysAskCase || (pgtype == EWmlSettingsHomePageBookmarks))
            {
            dimmed = ETrue;
            }
        else if (pgtype == EWmlSettingsHomePageAccessPoint)
        	{
        	// check to see if no home page defined on the access point
        	HBufC* buf = HBufC::NewLC( KMaxHomePgUrlLength );  // cleanupstack
    		TPtr ptr( buf->Des() );
    		TInt pgFound = Preferences().HomePageUrlL( ptr );
    		if( pgFound != KErrNone )
    			{
    			dimmed = ETrue;
    			}
    		CleanupStack::PopAndDestroy( buf );
        	}
        }
    else
        {
        dimmed = ETrue;
        }
        
    return(dimmed);
}

// ---------------------------------------------------------
// CBrowserAppUi::IsDisplayingMenuOrDialog
// ---------------------------------------------------------
//
TBool CBrowserAppUi::IsDisplayingMenuOrDialog()
{
	return CCoeAppUi::IsDisplayingMenuOrDialog();
}
// ---------------------------------------------------------
// CBrowserAppUi::HandleWsEventL
// ---------------------------------------------------------
void CBrowserAppUi::HandleWsEventL(const TWsEvent& aEvent, 
                                   CCoeControl* aDestination)
{
  /**
   * We need a special handling for pointer events to handle the situation 
   * when the pointer is crossing the boundaries of plugin window. Browser
   * scrolling id done using PointerMove events not a Drag. 
   * When PointerMoveBuffer is ready HandlePointerBufferReadyL() is called by
   * Symbian framework. By default if Pointer Down event wasn't issued on the 
   * current CoeControl Pointer Move events are not sent to this control.
   * The code below makes sure that when we entering the Content View window 
   * from pluging window we are still getting move events.
   * When we are leaving Content window we are generating fake Pointer Up 
   * event in order to on-demand scrollbars disapear properly. The later is a
   * hack and needs to be re-designed in such way that pluging will pass 
   * pointer event back to the browser.
   */
   if (aDestination != NULL)
       {
       TInt type=aEvent.Type();
       
       switch (type)
           {
           case EEventPointerExit:
               {

               break;
               }
               
           case EEventPointerEnter:
               {
               if (iCurrentView == KUidBrowserContentViewId)
                   {
                   CCoeControl* ctrl = &(BrCtlInterface());
                   ctrl->DrawableWindow()->DiscardPointerMoveBuffer(); 
                   const TInt count = ctrl->CountComponentControls();
                   for (TInt i = 0; i < count; i++) 
                       {
                       CCoeControl* c = ctrl->ComponentControl(i);
                       if (c) 
                          {
                          c->ClaimPointerGrab(EFalse);
                          }
                       }
                   ctrl->ClaimPointerGrab(EFalse);
                   }
               break;
               }
           
           }
       }
   CAknViewAppUi::HandleWsEventL(aEvent, aDestination);
}

// ---------------------------------------------------------
// CBrowserAppUi::HandleSystemEventL
// ---------------------------------------------------------
#if defined(__S60_50__)
void CBrowserAppUi::HandleSystemEventL(const TWsEvent& aEvent)
   {
   
   /**
    * We need a special handling for System events to handle the situation 
    * like when the shut down is done while download is going on.
    */
   
   switch (*(TApaSystemEvent*)(aEvent.EventData()))
     {
     case EApaSystemEventShutdown:    
       // do things here
        ExitBrowser ( IsAppShutterActive() );  
       break;
     default:
       break;
     }
   // call base class implementation
   CAknAppUi::HandleSystemEventL(aEvent);
   }
#endif  
// ---------------------------------------------------------
// CBrowserAppUi::StartFetchHomePageL
// ---------------------------------------------------------
void CBrowserAppUi::StartFetchHomePageL(void)
    {
    if(!iStartedUp)
        CompleteDelayedInit();
    
    // There's a homepage to be launched so start in content view
    SetLastActiveViewId(KUidBrowserContentViewId);      	    	          
   	//wait for contentview to initialize itself
    WaitCVInit();
    
    TInt error( KErrNone );
    TRAP( error, FetchHomePageL() );
    if( error != KErrNone )
        {
        CloseContentViewL();
        }                
    }
    
#ifdef BRDO_IAD_UPDATE_ENABLED_FF
// ---------------------------------------------------------
// CBrowserAppUi::CompleteIADUpdateCallback
// ---------------------------------------------------------
TInt CBrowserAppUi::CompleteIADUpdateCallback( TAny* aBrowserAppUi )
    {
    TRAP_IGNORE( ((CBrowserAppUi*)aBrowserAppUi)->CheckUpdatesL() )
    return KErrNone;
    }


// ---------------------------------------------------------
// CBrowserAppUi::CheckUpdateFileAvailable
// ---------------------------------------------------------
TBool CBrowserAppUi::CheckUpdateFileAvailable()
    {
    LOG_ENTERFN("CBrowserAppUi::CheckUpdateFileAvailable");
    TBuf<KMaxFileName> privatePath;
    TBuf<KMaxFileName> updateFileName;
    iFs.PrivatePath( privatePath );
    updateFileName.Copy( privatePath );
    updateFileName.Append( KUpdateFileName );
    RFile updatefile;   
    TInt err = updatefile.Open( iFs, updateFileName, EFileRead );
    if ( err == KErrNotFound ) 
        {
        LOG_WRITE( "CBrowserAppUi::CheckUpdateFileAvailable - update file not available" );
        return EFalse;
        }
    else
        {
        LOG_WRITE( "CBrowserAppUi::CheckUpdateFileAvailable - update file available" );
        updatefile.Close();
        }
    return ETrue;       
    }

// ---------------------------------------------------------
// CBrowserAppUi::WriteUpdateFile
// ---------------------------------------------------------
void CBrowserAppUi::WriteUpdateFile()
    {
    LOG_ENTERFN("CBrowserAppUi::WriteUpdateFile");
    TBuf<KMaxFileName> privatePath;
    TBuf<KMaxFileName> updateFileName;
    iFs.PrivatePath( privatePath );
    updateFileName.Copy( privatePath );
    updateFileName.Append( KUpdateFileName );
    RFile updatefile;
    TInt err = updatefile.Open( iFs, updateFileName, EFileWrite | EFileShareExclusive );
    if(err == KErrNotFound)
        {
        LOG_WRITE( "CBrowserAppUi::WriteUpdateFile - update file not available so create it" );
        err = updatefile.Create( iFs, updateFileName, EFileWrite | EFileShareExclusive );
        }
    //Get the current time
    TTime timenow;
    timenow.HomeTime();
    TInt64 time = timenow.Int64();
    TBuf8<50> data;
    data.AppendNum(time);
    LOG_WRITE( "CBrowserAppUi::WriteUpdateFile - write the current time in update file" );
    updatefile.Write(data);
    updatefile.Close();
    }

// ---------------------------------------------------------
// CBrowserAppUi::DeleteUpdateFile
// ---------------------------------------------------------
void CBrowserAppUi::DeleteUpdateFile()
    {
    TBuf<KMaxFileName> privatePath;
    TBuf<KMaxFileName> updateFileName;
    iFs.PrivatePath( privatePath );
    updateFileName.Copy( privatePath );
    updateFileName.Append( KUpdateFileName );
    iFs.Delete(updateFileName);  
    }

// ---------------------------------------------------------
// CBrowserAppUi::ReadUpdateFile
// ---------------------------------------------------------
TInt64 CBrowserAppUi::ReadUpdateFile()
    {
    TBuf<KMaxFileName> privatePath;
    TBuf<KMaxFileName> updateFileName;
    //Get the private path then append the filename
    iFs.PrivatePath( privatePath );
    updateFileName.Copy( privatePath );
    updateFileName.Append( KUpdateFileName );
    RFile updatefile;
    TInt err = updatefile.Open( iFs, updateFileName, EFileRead );
    TInt64 dataValue = 0;
    //If file is found
    if ( err != KErrNotFound ) 
        {
        TBuf8<50> readBuf;
        err = updatefile.Read(readBuf);
        updatefile.Close();
        if((err == KErrNone) && (readBuf.Length()>NULL))
            {
            //Convert from TBuf8 to TInt64
            TLex8 lex(readBuf);
            lex.Val(dataValue);
            }
        }        
    return dataValue;
    }
#endif

TBool CBrowserAppUi::IsSameWinApp(TUid aMessageUid)
{      
    TInt nElements = sizeof(mArrayOfExternalAppUid)/sizeof(TInt);
    for(TInt nIndex = 0;nIndex < nElements; nIndex++)
    {
        if(aMessageUid == TUid::Uid(mArrayOfExternalAppUid[nIndex]))
        {
        return ETrue; 
        }
    }
    return EFalse;
}
// End of File
