/*
* Copyright (c) 2002-2008 Nokia Corporation and/or its subsidiary(-ies).
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
#include <cdownloadmgruidownloadslist.h>
#include <cdownloadmgruidownloadmenu.h>
#include <FeatMgr.h>
#include <irmsgtypeuid.h>
#include <SenduiMtmUids.h>
#include <IrcMTM.h>
#include <EikMenuP.h>
#include <internetconnectionmanager.h>
#include <DocumentHandler.h>
#include <apmstd.h>
#include <aknnavi.h>
#include <EIKSPANE.H>
#include <aknnavide.h>
#include <bldvariant.hrh>
#include <LayoutMetaData.cdl.h> // For Layout_Meta_Data landscape/portrait status

#include <BrowserNG.rsg>
#include <StringLoader.h>

#include <aknviewappui.h>
#include <aknlists.h>
#include <akntitle.h>
#include <aknPopup.h>
#include <StringLoader.h>
#include <uri16.h>
#include <ApUtils.h>
#include <btmsgtypeuid.h>
#include "CommsModel.h"
#include <AknNaviLabel.h>
#include <FindItemMenu.h>
#include <EIKMENUB.H>
#include <avkon.hrh>

#include <AknInfoPopupNoteController.h>
#include <aknutils.h>
#include <AknStylusPopUpMenu.h>

#include <s32mem.h>
#include <Uri16.h>

#include "Browser.hrh"
#include "BrowserContentView.h"
#include "BrowserContentViewToolbar.h"
#include "BrowserContentViewContainer.h"
#include "BrowserContentViewZoomModeTimer.h"
#include "BrowserBookmarksView.h"
#include "CommonConstants.h"
#include "BrowserAppUi.h"
#include "BrowserGotoPane.h"
#include "Preferences.h"
#include "BrowserPreferences.h"
#include "Display.h"
#include <favouriteswapap.h>
#include "BrowserBookmarksModel.h"
#include "BrowserDialogs.h"
#include "BrowserUtil.h"
#include "BrowserBmOTABinSender.h"
#include <FINDITEMMENU.RSG>
#include "BrowserUiVariant.hrh"
#include "BrowserAdaptiveListPopup.h"
#include "BrowserApplication.h"
#include "logger.h"
#include <favouritesfile.h>
#include "BrowserWindowManager.h"
#include "BrowserWindow.h"
#include "BrowserPopupEngine.h"

#include <brctldialogsprovider.h>
#include <browserdialogsprovider.h>
#include <brctldefs.h>
#include <akntoolbar.h>
#ifdef RD_SCALABLE_UI_V2
#include <akntoolbarextension.h>
#endif
#include <akntouchpane.h>

#include "BrowserShortcutKeyMap.h"
_LIT( KSchemaIdentifier, "://" );
const TInt KSchemaIdentifierLength = 3;

// Time interval in milliseconds that status pane stays visible after download in fullscreen mode
const TInt KFullScreenStatusPaneTimeout( 3 * 1000000 ); // 3 seconds

const TInt KAutoFullScreenTimeout( 5 * 1000000 ); // 5 seconds

const TInt KAutoFullScreenIdleTimeout( 10 * 1000000 ); // 10 seconds
// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBrowserContentView::NewLC
// -----------------------------------------------------------------------------
//
CBrowserContentView* CBrowserContentView::NewLC( MApiProvider& aApiProvider,
                                                 TRect& aRect )
    {
    CBrowserContentView* view =
        new (ELeave) CBrowserContentView( aApiProvider );
    CleanupStack::PushL(view);
    view->ConstructL( aRect );

    return view;
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::CBrowserContentView
// -----------------------------------------------------------------------------
//
// Scroll indicators may become deprecated
CBrowserContentView::CBrowserContentView( MApiProvider& aApiProvider ) :
    CBrowserViewBase( aApiProvider ),
    iZoomMode(EFalse),
    iContentFullScreenMode( EFalse ),
    iIsPluginFullScreenMode( EFalse),
    iWasContentFullScreenMode( EFalse )
    {
    iFindItemIsInProgress = EFalse;
    iWasInFeedsView = EFalse;
    iPenEnabled = AknLayoutUtils::PenEnabled();
    iFullScreenBeforeEditModeEntry = EFalse;
    }


// -----------------------------------------------------------------------------
// CBrowserContentView::Id
// -----------------------------------------------------------------------------
//
TUid CBrowserContentView::Id() const
    {
    return KUidBrowserContentViewId;
    }


// -----------------------------------------------------------------------------
// CBrowserContentView::~CBrowserContentView
// -----------------------------------------------------------------------------
//
CBrowserContentView::~CBrowserContentView()
    {
    delete iBrowserContentViewToolbar;
    delete iNaviDecorator;
    delete iBookmarksModel;
    delete iContainer;
    delete iEnteredURL;
    delete iEnteredKeyword;
    delete iTextZoomLevelArray;
    delete iFontSizeArray;
    delete iZoomModeTimer;
    iNaviPane = NULL;
    if ( iToolBarInfoNote )
        {
        iToolBarInfoNote->HideInfoPopupNote();
        }
    delete iToolBarInfoNote;
    delete iStylusPopupMenu;
    delete iPeriodic;
    delete iAutoFSPeriodic;
    delete iIdlePeriodic;
    }


// -----------------------------------------------------------------------------
// CBrowserContentView::ConstructL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::ConstructL( TRect& aRect )
    {

    BaseConstructL( R_BROWSER_CONTENT_VIEW );

    ConstructMenuAndCbaEarlyL();

    iContainer = CBrowserContentViewContainer::NewL( this, ApiProvider() );

    iContainer->SetRect( aRect );
    iContainer->GotoPane()->SetGPObserver(this);
    iContainer->FindKeywordPane()->SetGPObserver(this);
    iEnteredKeyword = NULL;

    iEnteredURL = NULL;
    // get the StatusPane pointer
    if ( !iNaviPane )
        {
        CEikStatusPane *sp = ((CAknAppUi*)iEikonEnv->EikAppUi())->StatusPane();
        // Fetch pointer to the default navi pane control
        iNaviPane =
            (CAknNavigationControlContainer*)sp->ControlL(
                                            TUid::Uid(EEikStatusPaneUidNavi) );
        }

    // Array for zoom levels (..., 70%, 80%, 90%, 100%,... )
    iTextZoomLevelArray = new( ELeave )CArrayFixFlat<TInt> ( 1 );
    // Array for zoom font sizes ( 1, 2, ... )
    iFontSizeArray = new( ELeave )CArrayFixFlat<TInt> ( 1 );

    // Get the possibile zoom levels
    GetTextZoomLevelsL();
    iZoomModeTimer = CBrowserContentViewZoomModeTimer::NewL( this );

    iToolBarInfoNote = CAknInfoPopupNoteController::NewL();
    HBufC* str = StringLoader::LoadLC(R_BROWSER_TOOLTIP_TOOLBAR);
    iToolBarInfoNote->SetTextL(*str);
    CleanupStack::PopAndDestroy();
    iBrowserContentViewToolbar = CBrowserContentViewToolbar::NewL(this);
    if (iPenEnabled)
        {
        Toolbar()->SetToolbarObserver(this);
        Toolbar()->SetFocusing(EFalse);
        ShowToolbarOnViewActivation(ETrue);
        }
    iShortcutKeyMap = NULL;
    iHistoryAtBeginning = EFalse;
    iHistoryAtEnd = EFalse;
    iZoomSliderVisible = EFalse;
    iPeriodic = CPeriodic::NewL(CActive::EPriorityIdle);
    iAutoFSPeriodic = CPeriodic::NewL(CActive::EPriorityIdle);
    iIdlePeriodic = CPeriodic::NewL(CActive::EPriorityIdle);
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::HandleCommandL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::HandleCommandL( TInt aCommand )
    {
    // Disabling FullScreen for non-touch devices, before processing some dialog-based shortcut and toolbar actions
    if ( !iPenEnabled && iContentFullScreenMode && ( ( aCommand == EWmlCmdFindKeyword ) ||
                                                     ( aCommand == EWmlCmdHistory ) ||
                                                     ( aCommand == EWmlCmdGoToAddress ) ||
                                                     ( aCommand == EWmlCmdShowShortcutKeymap ) ||
                                                     ( aCommand == EWmlCmdShowMiniature )||
                                                     ( aCommand == EWmlCmdShowToolBar)) )
        {
        iWasContentFullScreenMode = iContentFullScreenMode;
        EnableFullScreenModeL( EFalse );
        }

#ifdef RD_SCALABLE_UI_V2
    // Close the extended toolbar whenever any item on it is selected
    if ( iPenEnabled && Toolbar()->ToolbarExtension()->IsShown() )
        {
        Toolbar()->ToolbarExtension()->SetShown( EFalse );
        StartAutoFullScreenTimer();
        }
#endif

#ifdef __RSS_FEEDS
    // Handle the subscribe to submenu.
    TInt cmd = aCommand - TBrCtlDefs::ECommandIdBase;

    if ((cmd >= TBrCtlDefs::ECommandIdSubscribeToBase) &&
            (cmd < TBrCtlDefs::ECommandIdPluginBase))
        {
            HandleSubscribeToL(cmd);
            return;
        }
#endif  // __RSS_FEEDS

    if ( ( aCommand != EWmlCmdZoomSliderShow ) && ZoomSliderVisible() )
        {
        MakeZoomSliderVisibleL( EFalse );
        }
    switch ( aCommand )
        {
        case EWmlCmdSetAsHomePage:
            {
            HBufC* url = ApiProvider().BrCtlInterface().PageInfoLC(TBrCtlDefs::EPageInfoUrl);
            if ( url && url->Length() && TBrowserDialogs::ConfirmQueryYesNoL(R_BROWSER_QUERY_SET_AS_HOME_PAGE))
                {
                ApiProvider().Preferences().SetHomePageUrlL(url->Des());
                ApiProvider().Preferences().SetHomePageTypeL( EWmlSettingsHomePageAddress );
                }
            CleanupStack::PopAndDestroy(); // url
            break;
            } 			      
        case EWmlCmdFavourites:
            {
            ApiProvider().SetViewToBeActivatedIfNeededL(
                KUidBrowserBookmarksViewId );
            break;
            }

        case EWmlCmdHistory:
            {
            ViewHistoryL();
            break;
            }

        case EWmlCmdHistoryBack:
            {
            ApiProvider().BrCtlInterface().HandleCommandL(
                                            (TInt)TBrCtlDefs::ECommandHistoryNavigateBack +
                                            (TInt)TBrCtlDefs::ECommandIdBase );
            UpdateTitleL( iApiProvider );
            break;
            }
        case EWmlCmdHistoryForward:
            {
            ApiProvider().BrCtlInterface().HandleCommandL(
                                            (TInt)TBrCtlDefs::ECommandHistoryNavigateForward +
                                            (TInt)TBrCtlDefs::ECommandIdBase );
            UpdateTitleL( iApiProvider );
            break;
            }
		// Set search pane active and then launch editor.	
        case EIsCmdSearchWeb:
            {
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF 
            CEikButtonGroupContainer* cba = Cba()->Current();
            CEikCba* eikCba = static_cast<CEikCba*>( cba->ButtonGroup() );
            if( eikCba )
                {
                eikCba->EnableItemSpecificSoftkey( EFalse );
                }
#endif
            iContainer->GotoPane()->SetSearchPaneActiveL();
            LaunchGotoAddressEditorL();
            break;
            }
        case EWmlCmdGoToAddress:
		case EWmlCmdGoToAddressAndSearch:
            {
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF            
            CEikButtonGroupContainer* cba = Cba()->Current();
            CEikCba* eikCba = static_cast<CEikCba*>( cba->ButtonGroup() );
            if( eikCba )
                {
                eikCba->EnableItemSpecificSoftkey( EFalse );
                }
#endif  
			iContainer->GotoPane()->SetGotoPaneActiveL();
            LaunchGotoAddressEditorL();
            break;
            }

        case EWmlCmdGotoPaneCancel:
            {
            CCoeEnv::Static()->AppUi()->RemoveFromStack( iContainer->GotoPane() );
            // Cancel editing and sets Goto Pane text back.
            if (iContainer->GotoPane()->PopupList() != NULL)
                {
                iContainer->GotoPane()->PopupList()->SetDirectoryModeL( ETrue );
                iContainer->GotoPane()->PopupList()->HidePopupL();
                }
            iContainer->ShutDownGotoURLEditorL();
            UpdateCbaL();
            if (iPenEnabled)
                {
                Toolbar()->SetDimmed(EFalse);
                Toolbar()->DrawNow();
                iBrowserContentViewToolbar->UpdateButtonsStateL();
                }
            if ( ApiProvider().Preferences().FullScreen() == EWmlSettingsFullScreenFullScreen )
                {
                MakeCbaVisible( EFalse );
                }
            if (iPenEnabled)
            	{
            	StartAutoFullScreenTimer();
            	}
        break;
        }

        case EWmlCmdGotoPaneGoTo:
            {
            HBufC* input = iContainer->GotoPane()->GetTextL();
	    CleanupStack::PushL( input );
	    if( iContainer->GotoPane()->GotoPaneActive() )
	        {
            if ((input) && (input->CompareF(KWWWString)) && input->Length() )
                {
                if (iPenEnabled)
                    {
                    Toolbar()->SetDimmed(EFalse);
                    Toolbar()->DrawNow();
                    }
                CCoeEnv::Static()->AppUi()->RemoveFromStack( iContainer->GotoPane() );
                // Cancel editing and sets Goto Pane text back.
                if (iContainer->GotoPane()->PopupList() != NULL)
                    {
                    iContainer->GotoPane()->PopupList()->SetDirectoryModeL( ETrue );
                    iContainer->GotoPane()->PopupList()->HidePopupL();
                    }
                GotoUrlInGotoPaneL();
                if ( ApiProvider().Preferences().FullScreen() == EWmlSettingsFullScreenFullScreen )
                    {
                    MakeCbaVisible( EFalse );
                    }
                }
	        }
	    else 
	        {
	        // Search Editor was active, So launch Search application with 
	        // search parameters and cancel editing of search and goto.
	        if ( (input) && input->Length()  )
               {
                if (iPenEnabled)
                  {
                  Toolbar()->SetDimmed(EFalse);
                  Toolbar()->DrawNow();
                  }
                      
                CCoeEnv::Static()->AppUi()->RemoveFromStack( iContainer->GotoPane() );
                if ( iContainer->GotoPane() )
                    {
                    iContainer->ShutDownGotoURLEditorL();
                    LaunchSearchApplicationL( *input );
                    }
                UpdateCbaL();
                UpdateFullScreenL();
               }
	        }
	        CleanupStack::PopAndDestroy( input );
			if (iPenEnabled)
              	{
               	StartAutoFullScreenTimer();
                }
            break;
            }

        case EWmlCmdGotoPaneSelect:
            {
            // Cancel editing and sets Goto Pane text back.
            if (iContainer->GotoPane()->PopupList() != NULL)
                {
                iContainer->GotoPane()->PopupList()->SetDirectoryModeL( ETrue );
                iContainer->GotoPane()->PopupList()->HidePopupL();
                }
            // set LSK to GOTO now
            UpdateCbaL();
            break;
            }

       //adaptive popuplist
        case EWmlCmdOpenFolder:
            {
            if (iContainer->GotoPane()->PopupList() != NULL)
                {
                iContainer->GotoPane()->PopupList()->SetDirectoryModeL( EFalse );
                }
            break;
            }

        case EAknSoftkeyCancel:
            {
            if ( isZoomMode() )
                {
                SaveCurrentZoomLevel(EFalse);
                //reset the zooming to the original settings
                SetZoomModeL( EFalse );
                //Exit in zooming, enable the cursor
                ApiProvider().BrCtlInterface().HandleCommandL(
                    (TInt)TBrCtlDefs::ECommandZoomSliderHide +
                    (TInt)TBrCtlDefs::ECommandIdBase );
                }

            if (iSynchRequestViewIsUp)
                {
                ApiProvider().BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandCancelFetch + (TInt)TBrCtlDefs::ECommandIdBase );
                }
            else
                {
                ApiProvider().BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandCancel + (TInt)TBrCtlDefs::ECommandIdBase );
                 }
            break;
            }

        case EWmlCmdSaveAsBookmark:
            {
            AddNewBookmarkL(EFalse);
            break;
            }

        // UI notifies the BrCtl which calls the DialogsProvider with a list of images
        case EWmlCmdShowImages:
            {
            ApiProvider().BrCtlInterface().HandleCommandL(
                                            (TInt)TBrCtlDefs::ECommandShowImages +
                                            (TInt)TBrCtlDefs::ECommandIdBase );
            break;
            }

        case EWmlCmdShowMiniature:
            {
            if (ApiProvider().Preferences().UiLocalFeatureSupported( KBrowserGraphicalPage ) &&
            !ApiProvider().WindowMgr().CurrentWindow()->HasWMLContent(ETrue))
                {
                ApiProvider().BrCtlInterface().HandleCommandL(
                                    (TInt)TBrCtlDefs::ECommandShowThumbnailView + (TInt)TBrCtlDefs::ECommandIdBase );
                }
            break;
            }

        // Download Manager UI Library shows a list of ongoing downloads
        case EWmlCmdDownloads:
            {
            ApiProvider().BrCtlInterface().HandleCommandL(
                                            (TInt)TBrCtlDefs::ECommandShowDownloads +
                                            (TInt)TBrCtlDefs::ECommandIdBase );
            break;
            }

        // Loads images on a page
        case EWmlCmdLoadImages:
            {
            ApiProvider().BrCtlInterface().HandleCommandL(
                                        (TInt)TBrCtlDefs::ECommandLoadImages +
                                        (TInt)TBrCtlDefs::ECommandIdBase );
            ApiProvider().WindowMgr().CurrentWindow()->SetImagesLoaded(ETrue);
            break;
            }

        // UI notifies the BrCtl which calls the DialogsProvider to show the toolbar
        case EWmlCmdShowToolBar:
            {
            if ( !PenEnabled() && iApiProvider.Preferences().ShowToolbarOnOff() &&
                 !iApiProvider.WindowMgr().CurrentWindow()->WMLMode() )
                {
                iToolBarInfoNote->ShowInfoPopupNote();
                ApiProvider().BrCtlInterface().HandleCommandL(
                                            (TInt)TBrCtlDefs::ECommandShowToolBar +
                                            (TInt)TBrCtlDefs::ECommandIdBase );
                }

            break;
            }
        //zoom in and set the cursor's position for non-touch
        case EWmlCmdZoomIn:
            {
            if ( ApiProvider().ContentDisplayed() && !ApiProvider().WindowMgr().CurrentWindow()->WMLMode() )
            {
                ApiProvider().BrCtlInterface().HandleCommandL(
                                (TInt)TBrCtlDefs::ECommandZoomIn +
                                (TInt)TBrCtlDefs::ECommandIdBase );
                ZoomImagesInL();
            }
            break;
            }

        //zoom out and set the cursor's position for non-touch
        case EWmlCmdZoomOut:
            {
            if ( ApiProvider().ContentDisplayed() && !ApiProvider().WindowMgr().CurrentWindow()->WMLMode() )
            {
                ApiProvider().BrCtlInterface().HandleCommandL(
                    (TInt)TBrCtlDefs::ECommandZoomOut +
                    (TInt)TBrCtlDefs::ECommandIdBase );
                ZoomImagesOutL();
            }
            break;
            }

        /* Zoom Mode currently disabled
        case EWmlCmdZoomMode:
            {
            // Disable the cursor: by using the zoom slider show cmd (temp)
            ApiProvider().BrCtlInterface().HandleCommandL(
                            (TInt)TBrCtlDefs::ECommandZoomSliderShow +
                            (TInt)TBrCtlDefs::ECommandIdBase );

            // Disable any activated objects (i.e. plugins, input boxes)
            ApiProvider().BrCtlInterface().HandleCommandL(
                            (TInt)TBrCtlDefs::ECommandCancel +
                            (TInt)TBrCtlDefs::ECommandIdBase );

            // Display softkeys if in full screen mode. i.e. go to normal screen
            if ( !iPenEnabled && iContentFullScreenMode )
                {
                iWasContentFullScreenMode = iContentFullScreenMode;
                EnableFullScreenModeL( EFalse );
                }

            // Enter zoom mode
            ZoomModeImagesL();
            break;
            }
        */

        case EWmlCmdZoomSliderShow:
            {
            // Toggle displaying the zoom slider, when the zoom button
            // on toolbar is selected
            MakeZoomSliderVisibleL( !iZoomSliderVisible );
            break;
            }
        case EWmlCmdZoomSliderHide:
            {
            // Currently not used
            MakeZoomSliderVisibleL( EFalse );
            break;
            }

        // Find keyword commands
        case EWmlCmdFindKeyword:
            {
            LaunchFindKeywordEditorL();
            break;
            }

        case EWmlCmdFindKeywordPaneFind :
            {
            break;
            }

        case EWmlCmdFindKeywordPaneClose :
            {
            CCoeEnv::Static()->AppUi()->RemoveFromStack( iContainer->FindKeywordPane() );
            // Cancel editing and sets Goto Pane text back.
            iContainer->ShutDownFindKeywordEditorL();
            UpdateCbaL();
            if (iPenEnabled)
                {
                iBrowserContentViewToolbar->UpdateButtonsStateL();
                }

            ApiProvider().BrCtlInterface().HandleCommandL(
                                        (TInt)TBrCtlDefs::ECommandClearFind +
                                        (TInt)TBrCtlDefs::ECommandIdBase );

            if ( ApiProvider().Preferences().FullScreen() == EWmlSettingsFullScreenFullScreen )
                {
                MakeCbaVisible( EFalse );
                }
            if (iPenEnabled)
	            {
	            StartAutoFullScreenTimer();
	            }
            break;
            }

        case EWmlCmdFindNext :
            {
            // Find next keyword
            FindKeywordL( NULL, ETrue );
            break;
            }
        case EWmlCmdFindPrevious :
            {
            // Find previous keyword
            FindKeywordL( NULL, EFalse );
            break;
            }

        case EAknSoftkeySelect:
            {
            if( iHistoryViewIsUp )
                {
                if ( !iPenEnabled && iWasContentFullScreenMode && !iContentFullScreenMode )
                        {
                        EnableFullScreenModeL( ETrue );
                        iWasContentFullScreenMode = EFalse;
                        }

                ApiProvider().BrCtlInterface().HandleCommandL(
                    (TInt)TBrCtlDefs::ECommandOpen + (TInt)TBrCtlDefs::ECommandIdBase );
                }

            // Save zoom factor and quit the zoom mode
            if ( isZoomMode() )
                {
                SaveCurrentZoomLevel(ETrue);
                // Disable the zoom mode
                SetZoomModeL( EFalse );
                // Return to full screen, if we were in full screen before
                if ( !iPenEnabled && iWasContentFullScreenMode && !iContentFullScreenMode )
                    {
                    EnableFullScreenModeL( ETrue );
                    iWasContentFullScreenMode = EFalse;
                    }
                // Exiting zoom mode, enable the cursor
                ApiProvider().BrCtlInterface().HandleCommandL(
                    (TInt)TBrCtlDefs::ECommandZoomSliderHide +
                    (TInt)TBrCtlDefs::ECommandIdBase );
                }
            break;
            }
        case EAknSoftkeyOk:
            {
            if( iThumbnailViewIsUp )
                {
                if ( !iPenEnabled && iWasContentFullScreenMode && !iContentFullScreenMode )
                    {
                    EnableFullScreenModeL( ETrue );
                    iWasContentFullScreenMode = EFalse;
                    }

                ApiProvider().BrCtlInterface().HandleCommandL(
                    (TInt)TBrCtlDefs::ECommandOpen + (TInt)TBrCtlDefs::ECommandIdBase );
                }
            break;
            }

        case EAknSoftkeyClose:
            {
            if( iWasInFeedsView)
                {
                // FeedsView library

                }
            else
                {
                if ( isZoomMode() )
                    {
                    SetZoomModeL( EFalse );
                    }
                AppUi()->HandleCommandL( EWmlCmdCloseWindow );
                }
            break;
            }

        case EWmlCmdSendAddressViaUnifiedMessage:
            {
            SendAddressL( );
            }
            break;

        case EWmlCmdBack:
            {
            if( iWasInFeedsView)
                {
                // FeedsView library

                }
            else if( iImageMapActive )
                {
                ApiProvider().BrCtlInterface().HandleCommandL(
                                        (TInt)TBrCtlDefs::ECommandBack +
                                        (TInt)TBrCtlDefs::ECommandIdBase );
                }
            else if( ApiProvider().BrCtlInterface().NavigationAvailable(
                        TBrCtlDefs::ENavigationBack ) )
                {
                AppUi()->HandleCommandL( aCommand );
                }
            else
                {
                AppUi()->HandleCommandL( EWmlCmdCloseWindow );
                }
            break;
            }

        case EWmlCmdOneStepBack:
            {
            if( ApiProvider().BrCtlInterface().NavigationAvailable(TBrCtlDefs::ENavigationBack ) )
                {
                AppUi()->HandleCommandL( aCommand );
                }
            break;
            }

        case EAknCmdHelp:
            {
            if ( HELP )
                {
                AppUi()->HandleCommandL( EAknCmdHelp );
                }
            break;
            }

        case EWmlCmdCancelFetch:
            {
            // give it to AppUi
            AppUi()->HandleCommandL( aCommand );
            break;
            }

        case EWmlCmdOpenLinkInNewWindow:
            {
            iApiProvider.BrCtlInterface().HandleCommandL(
                (TInt)TBrCtlDefs::ECommandOpenNewWindow + (TInt)TBrCtlDefs::ECommandIdBase );
            break;
            }

        case EWmlCmdOpenLink:
            {
            iApiProvider.BrCtlInterface().HandleCommandL(
                (TInt)TBrCtlDefs::ECommandOpen + (TInt)TBrCtlDefs::ECommandIdBase );
            break;
            }

        case EWmlCmdShowSubscribeList:
            {
            const RPointerArray<TBrCtlSubscribeTo>& subscribeToList = iApiProvider.BrCtlInterface().SubscribeToMenuItemsL();
            TInt len(subscribeToList.Count());
            if (len > 0)
                {
                CArrayFixFlat<TBrCtlSelectOptionData>* optList = new( ELeave ) CArrayFixFlat<TBrCtlSelectOptionData>(10);
                CleanupStack::PushL( optList );
                HBufC* title = StringLoader::LoadLC(R_BROWSER_SUBSCRIBE_LIST);
                TInt i;
                for(i = 0; i < len; i++)
                    {
                    if(subscribeToList[i]->Title().Length())
                        {
                        TBrCtlSelectOptionData t(subscribeToList[i]->Title(), EFalse, EFalse, EFalse);
                        optList->AppendL(t);
                        }
                    else
                        {
                        TBrCtlSelectOptionData t(subscribeToList[i]->Url(), EFalse, EFalse, EFalse);
                        optList->AppendL(t);
                        }
                    }
                TBool ret(ApiProvider().DialogsProvider().DialogSelectOptionL( *title, ESelectTypeNone, *optList));
                if( ret )
                    {
                    TInt i;
                    for (i = 0; i < optList->Count(); i++)
                        {
                        if ((*optList)[i].IsSelected())
                            {
                            HandleSubscribeToL(subscribeToList[i]->CommandID() - TBrCtlDefs::ECommandIdBase);
                            break;
                            }
                        }
                    }
                optList->Reset();
                CleanupStack::PopAndDestroy( 2 ); // optList, title
                }
            break;
            }

        case EWmlCmdHome:
            if (ApiProvider().Preferences().HomePageType() ==  EWmlSettingsHomePageAddress )
                {
                HBufC* url = HBufC::NewLC( KMaxHomePgUrlLength );
                TPtr ptr( url->Des() );
                User::LeaveIfError(ApiProvider().Preferences().HomePageUrlL( ptr ));
                ApiProvider().FetchL( ptr );
                CleanupStack::PopAndDestroy(); // url
                }
            else
                {
                HBufC* url = ApiProvider().BrCtlInterface().PageInfoLC(TBrCtlDefs::EPageInfoUrl);
                if ( url && url->Length() && TBrowserDialogs::ConfirmQueryYesNoL(R_BROWSER_QUERY_SET_AS_HOME_PAGE))
                    {
                    ApiProvider().Preferences().SetHomePageUrlL(url->Des());
                    ApiProvider().Preferences().SetHomePageTypeL( EWmlSettingsHomePageAddress );
                    }
                CleanupStack::PopAndDestroy(); // url
                }
            break;

        case EWmlCmdGo:
            break;

        case EWmlCmdConfigureShortcutKeymap:
            {
            ApiProvider().SetViewToBeActivatedIfNeededL( KUidBrowserSettingsViewId, KUidSettingsGotoShortcutsGroup.iUid );
            break;
            }

        case EWmlCmdShowShortcutKeymap:
            {
            ShowKeymap();
            break;
            }

        case EWmlCmdHideShortcutKeymap:
            {
            HideKeymap();
            break;
            }

        case EWmlCmdOpenFeedsFolder:
            // Launch into feeds view, telling it that we came from content view
            ApiProvider().FeedsClientUtilities().ShowFolderViewL(KUidBrowserContentViewId);
            break;

        case EWmlCmdEnterFullScreenBrowsing:
            {
            if ( iPenEnabled )
                {
                EnableFullScreenModeL( ETrue );
                }
            else
                {
                if ( iContentFullScreenMode )
                    {
                    EnableFullScreenModeL( EFalse );
                    }
                else
                    {
                    EnableFullScreenModeL( ETrue );
                    }
                }
            break;
            }

        case EWmlCmdExitFullScreenBrowsing:
            EnableFullScreenModeL( EFalse );
            break;

        default:
            {
            // DO element and toolbar commands are passed to BrCtl via AppUi
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }   // end of switch (aCommand)

    // Enabling FullScreen for non-touch devices, after processing some dialog-based shortcut and toolbar actions
    if ( !iPenEnabled && iWasContentFullScreenMode && !iContentFullScreenMode &&
                                                   ( ( aCommand == EWmlCmdFindKeywordPaneClose )  ||
                                                     ( aCommand == EWmlCmdHideShortcutKeymap )  ||
                                                     ( aCommand == EWmlCmdGotoPaneGoTo )  ||
                                                     ( aCommand == EWmlCmdGotoPaneCancel ) ||
                                                     ( aCommand == EAknSoftkeyCancel ) ) )
        {
        EnableFullScreenModeL( ETrue );
        iWasContentFullScreenMode = EFalse;
        }
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::ViewHistoryL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::ViewHistoryL()
    {
    ApiProvider().BrCtlInterface().HandleCommandL(
                                            (TInt)TBrCtlDefs::ECommandShowHistory +
                                                    (TInt)TBrCtlDefs::ECommandIdBase );
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::DoActivateL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::DoActivateL(
                                      const TVwsViewId& /*aPrevViewId*/,
                                      TUid /*aCustomMessageId*/,
                                      const TDesC8& /*aCustomMessage*/ )
    {
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
    CBrowserAppUi* ui;

    ui = STATIC_CAST( CBrowserAppUi*, AppUi() );
    iPreviousViewID = ApiProvider().LastActiveViewId();
    if ( ui->LastActiveViewId() == KUidBrowserNullViewId )
        {
        //No view has been yet set as startup view -> use bookmarks view
        ui->SetLastActiveViewId ( KUidBrowserBookmarksViewId );
        ui->SetViewToBeActivatedIfNeededL( ui->LastActiveViewId() );
        ApiProvider().SetLastActiveViewId ( ui->LastActiveViewId() );
        }
    else
        {
        UpdateFullScreenL();
        ApiProvider().SetLastActiveViewId(Id());
        }

    iContainer->ActivateL();

    AppUi()->AddToViewStackL( *this, iContainer );

    UpdateTitleL( ApiProvider() );

    ApiProvider().BrCtlInterface().MakeVisible(ETrue);
    ApiProvider().BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandGainFocus +
                                                    (TInt)TBrCtlDefs::ECommandIdBase);

    if( ui->Connection().Connected() &&
        !ui->Preferences().HttpSecurityWarningsStatSupressed() )
        {
        // Update security indicators
        if ( ui->SomeItemsNotSecure() )
            {
            ui->Display().UpdateSecureIndicatorL( EAknIndicatorStateOff );
            }
        else
            {
            ui->Display().UpdateSecureIndicatorL(
                                ui->LoadObserver().LoadStatus(
                                CBrowserLoadObserver::ELoadStatusSecurePage ) ?
                            EAknIndicatorStateOn : EAknIndicatorStateOff );
            }
        }
    UpdateCbaL();
    if(KeymapIsUp())
        {
        RedrawKeymap();
        }

	if (iPenEnabled)
		{
		StartAutoFullScreenTimer();
		}
    iContainer->SetRect(ClientRect());
PERFLOG_STOP_WRITE("ContentView::DoActivate")
    }


// -----------------------------------------------------------------------------
// CBrowserContentView::UpdateFullScreenL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::UpdateFullScreenL()
    {
    TVwsViewId activeViewId;
    if ( AppUi()->GetActiveViewId( activeViewId ) == KErrNone )
        {
        if ( activeViewId.iViewUid == KUidBrowserContentViewId )
            {
            TBool sizeChangedCba = EFalse;
            TBool sizeChangedSP = EFalse;
            TBool resIdChanged = EFalse;
            TInt resId = StatusPane()->CurrentLayoutResId();

            if ( Layout_Meta_Data::IsLandscapeOrientation() )
                {
                resIdChanged = resId != R_AVKON_STATUS_PANE_LAYOUT_USUAL;
                StatusPane()->SwitchLayoutL(R_AVKON_STATUS_PANE_LAYOUT_USUAL);
                }
            else //Portrait
                {
                resIdChanged = resId != R_AVKON_STATUS_PANE_LAYOUT_SMALL;
                StatusPane()->SwitchLayoutL(R_AVKON_STATUS_PANE_LAYOUT_SMALL);
                }

            if ( iContentFullScreenMode )
                {
                sizeChangedCba = Cba()->IsVisible();
                Cba()->MakeVisible( EFalse );
                UpdateCbaL();

                if (!ApiProvider().Fetching())
                    {
                    sizeChangedSP = StatusPane()->IsVisible();
                    StatusPane()->MakeVisible( EFalse );
                    }
                else
                    {
                    sizeChangedSP = !StatusPane()->IsVisible();
                    ShowFsStatusPane(ETrue);
                    }
                }
            else
                {
                sizeChangedCba = !Cba()->IsVisible();
                Cba()->MakeVisible( ETrue );
                sizeChangedSP = !StatusPane()->IsVisible();
                StatusPane()->MakeVisible( ETrue );
                }

            ApiProvider().Display().FSPaneOnL( );
            ApiProvider().Display().SetGPRSIndicatorOnL();
            UpdateTitleL(ApiProvider());
            ApiProvider().Display().RestoreTitleL();
            StatusPane()->ApplyCurrentSettingsL();
            if ( resIdChanged || sizeChangedCba || sizeChangedSP  )
                {
                iContainer->SetRect( ClientRect() );
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::SetFullScreenOffL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::SetFullScreenOffL()
    {
    TVwsViewId activeViewId;
    if ( AppUi()->GetActiveViewId( activeViewId ) == KErrNone )
        {
        if ( activeViewId.iViewUid == KUidBrowserContentViewId )
            {
            Cba()->MakeVisible( ETrue );
            UpdateCbaL();
            if ( Layout_Meta_Data::IsLandscapeOrientation() )
	            StatusPane()->SwitchLayoutL(R_AVKON_STATUS_PANE_LAYOUT_USUAL);
            StatusPane()->ApplyCurrentSettingsL();
            StatusPane()->MakeVisible( ETrue );
            iContainer->SetRect( ClientRect() );
            }
        }
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::SetZoomLevelL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::SetZoomLevelL()
    {
        // Get the current index of the array.
        iCurrentZoomLevel = ApiProvider().BrCtlInterface().
        BrowserSettingL(TBrCtlDefs::ESettingsCurrentZoomLevelIndex);
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::DoDeactivate
// -----------------------------------------------------------------------------
//
void CBrowserContentView::DoDeactivate()
    {
    if ( !ApiProvider().ExitInProgress() )
        {
        TBrCtlDefs::TBrCtlElementType focusedElementType =
                ApiProvider().BrCtlInterface().FocusedElementType();
        if (focusedElementType == TBrCtlDefs::EElementActivatedInputBox)
            {
            ApiProvider().BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandAccept +
                                 (TInt)TBrCtlDefs::ECommandIdBase );
            }
        TRAP_IGNORE( ApiProvider().Display().UpdateSecureIndicatorL(
            EAknIndicatorStateOff ));
        iContainer->SetFocus( EFalse );

        TRAP_IGNORE( ApiProvider().BrCtlInterface().HandleCommandL(
            (TInt)TBrCtlDefs::ECommandLoseFocus +
            (TInt)TBrCtlDefs::ECommandIdBase ) );
        ApiProvider().BrCtlInterface().MakeVisible(EFalse);
        }

    AppUi()->RemoveFromViewStack( *this, iContainer );
    }

// ----------------------------------------------------------------------------
// CBrowserContentView::CommandSetResourceIdL
// ----------------------------------------------------------------------------
//
TInt CBrowserContentView::CommandSetResourceIdL()
    {
    LOG_ENTERFN("ContentView::CommandSetResourceIdL");

    // default case for normal screen
    TInt result( R_BROWSER_DEFAULT_BUTTONS );

    // default case for full screen
    if ( iContentFullScreenMode )
        {
        result = R_BROWSER_SOFTKEYS_EXIT_FULL_SCREEN;
        }
    if ( isZoomMode() )
        {
        return R_BROWSER_DEFAULT_BUTTONS_AT_ZOOM_MODE_ON;
        }
    if( KeymapIsUp() )
        {
        return R_BROWSER_SOFTKEYS_CONFIGURE_HIDE;
        }
    if( iHistoryViewIsUp )
        {
        return R_BROWSER_SOFTKEYS_SELECT_CANCEL_SELECT;
        }
    if( iPluginPlayerIsUp )
        {
        return R_BROWSER_SOFTKEYS_CANCEL;
        }
    if( iSmartTextViewIsUp )
        {
        return R_INPUT_ELEMENT_BUTTONS;
        }
    if( iThumbnailViewIsUp )
        {
        return R_BROWSER_DEFAULT_BUTTONS_AT_THUMBNAIL_VIEW_ON;
        }
    if( iImageMapActive )
        {
        return R_BROWSER_DEFAULT_BUTTONS;
        }
    if( iSynchRequestViewIsUp )
        {
        if ( !iContentFullScreenMode )
            {
            result = R_BROWSER_SOFTKEYS_CANCEL;
            }
        return result;
        }

    if( iContainer->GotoPane()->IsVisible() )
        // when goto pane is up there is no focusable element or active fetching
        // process in place
        {
        if( iContainer->GotoPane()->SearchPaneActive() )
            {
            result = R_BROWSER_BOOKMARKS_CBA_SEARCH_PANE_SEARCH_CANCEL;
            }
        else
            {
            //check wheter there is an active popuplist
            if( (iContainer->GotoPane()->PopupList() != NULL) &&
                       ( iContainer->GotoPane()->PopupList()->IsOpenDirToShow() ))
                {
                result = R_BROWSER_BOOKMARKS_CBA_GOTO_PANE_OPENDIR_CANCEL;
                }
            else if(iContainer->GotoPane()->PopupList() &&
                    iContainer->GotoPane()->PopupList()->IsPoppedUp() &&
                    !iPenEnabled)
                {
                // LSK Select is only for non-touch devices
                result =  R_BROWSER_BOOKMARKS_CBA_GOTO_PANE_SELECT_CANCEL;
                }
            else
                {
                result =  R_BROWSER_BOOKMARKS_CBA_GOTO_PANE_GOTO_CANCEL;
                }
            }
        }
    else if ( iContainer->FindKeywordPane()->IsVisible() )
        {
        result = R_BROWSER_CBA_FIND_KEYWORD_PANE_OPTION_CLOSE;
        }
    else if ( ApiProvider().Fetching() )
        {
        if ( !iContentFullScreenMode )
            {
            // Enable Options menu during download can be done here
            // otherwise use defualt Full Screen buttons
            result = R_BROWSER_OPTIONS_MENU_DURING_DOWNLOAD;
            }
        }
    else
        {
        TBrCtlDefs::TBrCtlElementType elementtype =
            ApiProvider().BrCtlInterface().FocusedElementType();
        if( elementtype == TBrCtlDefs::EElementActivatedObjectBox )
            {
            if ( !iContentFullScreenMode )
                {
                result = R_INPUT_ELEMENT_BUTTONS;
                }
            }
        else if ( !ApiProvider().BrCtlInterface().NavigationAvailable(
                TBrCtlDefs::ENavigationBack ) )
            // at the beginning of the  history list
            {
            if ( !iContentFullScreenMode )
                {
                // Options + Close
                result = R_BROWSER_DEFAULT_BUTTONS_AT_BEGINNING_OF_HISTORY;
                }
            }
        }
BROWSER_LOG( ( _L(" ContentView's buttons:%d"), result ) );
    return result;
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::ProcessCommandL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::ProcessCommandL(TInt aCommand)
    {
    CBrowserViewBase::ProcessCommandL( aCommand );
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::DynInitMenuPaneL
// -----------------------------------------------------------------------------
void CBrowserContentView::DynInitMenuPaneL( TInt aResourceId,
                                            CEikMenuPane* aMenuPane )
    {
    LOG_ENTERFN("CBrowserContentView::DynInitMenuPaneL");
    __ASSERT_DEBUG(aMenuPane, Util::Panic(Util::EUninitializedData));

    if ( aResourceId == R_MENU_PANE )
        {
        // web feeds
        #ifndef __RSS_FEEDS
        aMenuPane->SetItemDimmed( EWmlCmdShowSubscribeList, ETrue );
        #else
        const RPointerArray<TBrCtlSubscribeTo>& items = ApiProvider().BrCtlInterface().SubscribeToMenuItemsL();
        if ( items.Count() == 0 )
            {
            aMenuPane->SetItemDimmed( EWmlCmdShowSubscribeList, ETrue );
            }
        #endif  // __RSS_FEEDS

        // downloads
        aMenuPane->SetItemDimmed( EWmlCmdDownloads,
            !ApiProvider().BrCtlInterface().BrowserSettingL( TBrCtlDefs::ESettingsNumOfDownloads ) );

        // If we have a touch device, check to see if the zoom slider is up, if so disable
        if ( PenEnabled() && ZoomSliderVisible() )
            {
            MakeZoomSliderVisibleL( EFalse );
            }

        // find
        aMenuPane->SetItemDimmed( EWmlCmdFindNext, ETrue );
        aMenuPane->SetItemDimmed( EWmlCmdFindPrevious, ETrue );

/*
        // help menu sub-menu - depending if Independent Application Update is available
        if (BRDO_BROWSER_UPDATE_UI_FF)
           {
           aMenuPane->SetItemDimmed( EAknCmdHelp, ETrue );
           }
        else
           {
           aMenuPane->SetItemDimmed( EWmlCmdHelpMenu, ETrue );
           }
*/
        if ( iContainer->FindKeywordPane()->IsVisible() )
            {
            CEikMenuPaneItem::SData menuFindNext =
                aMenuPane->ItemData( EWmlCmdFindNext );
            CEikMenuPaneItem::SData menuFindPrevious =
                aMenuPane->ItemData( EWmlCmdFindPrevious );

            // Delete all menu items
            aMenuPane->DeleteBetweenMenuItems( 0,
            aMenuPane->NumberOfItemsInPane() - 1 );

            aMenuPane->AddMenuItemL( menuFindNext );
            aMenuPane->AddMenuItemL( menuFindPrevious );
            aMenuPane->SetItemDimmed( EWmlCmdFindNext, EFalse );
            aMenuPane->SetItemDimmed( EWmlCmdFindPrevious, EFalse );
            return;
            }

        // wml option menu items
        if ( ApiProvider().BrCtlInterface().WMLOptionMenuItemsL()->Count() < 1 )
            {
            // there isn't DO element, or only the first one with PREV exist
            aMenuPane->SetItemDimmed( EWmlCmdServiceOptions, ETrue );
            }

        // find window
        if (ApiProvider().WindowMgr().CurrentWindow()->WMLMode())
            {
            aMenuPane->SetItemDimmed(EWmlCmdFindKeyword, ETrue);
            }

        // shortcut key map
        if (PenEnabled() || ApiProvider().IsEmbeddedModeOn())
            {
            aMenuPane->SetItemDimmed( EWmlCmdShowShortcutKeymap, ETrue);
            }

        // BrCtl adds menu items to UI options menu list
        ApiProvider().BrCtlInterface().AddOptionMenuItemsL( *aMenuPane, aResourceId );
        }
    else if ( aResourceId == R_DO_ELEMENTS )
        {
        RPointerArray<TBrCtlWmlServiceOption>* wmlElements;
        wmlElements = ApiProvider().BrCtlInterface().WMLOptionMenuItemsL();

        for ( TInt i=0; i<wmlElements->Count(); i++ )
            {
            TBrCtlWmlServiceOption* option = (*wmlElements)[i];
            if ( option != NULL)
                {
                CEikMenuPaneItem::SData item;
                item.iText.Copy( option->Text() );
                item.iCommandId = option->ElemID();
                item.iFlags = 0;
                item.iCascadeId = 0;
                aMenuPane->InsertMenuItemL(item, 0);
                }
            }
        }
    else if ( aResourceId == R_GOTO_SUBMENU )
        {
        // back to page
        aMenuPane->SetItemDimmed( EWmlCmdBackToPage, ETrue );

        // bookmarks/webfeeds
        if( ApiProvider().IsEmbeddedModeOn() )
            {
            aMenuPane->SetItemDimmed( EWmlCmdFavourites, ETrue );
            aMenuPane->SetItemDimmed( EWmlCmdOpenFeedsFolder, ETrue );
            }

        // home
        aMenuPane->SetItemDimmed( EWmlCmdLaunchHomePage, ApiProvider().IsLaunchHomePageDimmedL() );
        //search 
         if ( ! ApiProvider().Preferences().SearchFeature() )
             {
             aMenuPane->SetItemDimmed( EIsCmdSearchWeb, ETrue );
             }
        }
    else if ( aResourceId == R_PAGEACTIONS_SUBMENU )
        {
        // bookmark
        if ( ApiProvider().IsEmbeddedModeOn() && CBrowserAppUi::Static()->IsEmbeddedInOperatorMenu() )
            {
            aMenuPane->SetItemDimmed( EWmlCmdSaveAsBookmark, ETrue );
            }
        HBufC *pageUrl = ApiProvider().BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoUrl );
        if( ( pageUrl == NULL ) || ( pageUrl->Length() == 0 ) )
            {
            aMenuPane->SetItemDimmed( EWmlCmdSaveAsBookmark, ETrue );
            }
        CleanupStack::PopAndDestroy( pageUrl );

        // send
        aMenuPane->SetItemDimmed( EWmlCmdSendBookmarkViaUnifiedMessage, ETrue );

        if (ApiProvider().IsEmbeddedModeOn())
            {
            aMenuPane->SetItemDimmed( EWmlCmdSendAddressViaUnifiedMessage, ETrue );
            }

        // set as home page
        //aMenuPane->SetItemDimmed( EWmlCmdSetAsHomePage, ETrue);


        // pop-up blocking
        if ( ApiProvider().IsEmbeddedModeOn() || ApiProvider().WindowMgr().CurrentWindow()->WMLMode())
            {
            aMenuPane->SetItemDimmed( EWmlCmdBlockPopups, ETrue );
            aMenuPane->SetItemDimmed( EWmlCmdAllowPopups, ETrue );
            }
        else
            {
            aMenuPane->SetItemDimmed( EWmlCmdBlockPopups, EFalse );
            aMenuPane->SetItemDimmed( EWmlCmdAllowPopups, EFalse );

            if ( ApiProvider().Preferences().PopupBlocking())
                {
                HBufC* url = ApiProvider().WindowMgr().CurrentWindow()->BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoUrl );
                TBool dimAllow = EFalse;
                if( url )
                    {
                    // 'Allow Popups' needed if the current URL is not on the whitelist
                    TRAPD( errWhite, dimAllow = ApiProvider().PopupEngine().IsUrlOnWhiteListL( *url ););
                    // Error handling
                    if ( !errWhite )
                        {
                        aMenuPane->SetItemDimmed( EWmlCmdBlockPopups, !dimAllow );
                        aMenuPane->SetItemDimmed( EWmlCmdAllowPopups, dimAllow );
                        }
                    }
                // else no url (strange case), so no popup specific menuitem
                CleanupStack::PopAndDestroy( url );
                }
                else
                  {
                  //not shown any popupblocks related menu items when it's setting noblock
                  aMenuPane->SetItemDimmed( EWmlCmdBlockPopups, ETrue );
                  aMenuPane->SetItemDimmed( EWmlCmdAllowPopups, ETrue );
                  }
            }
        }
    else if ( aResourceId == R_VIEW_SUBMENU )
        {
        // toolbar
        if ( !PenEnabled() && iApiProvider.Preferences().ShowToolbarOnOff() )
            {
            aMenuPane->SetItemDimmed( EWmlCmdShowToolBar, ApiProvider().WindowMgr().CurrentWindow()->WMLMode() );
            }
        else
            {
            aMenuPane->SetItemDimmed( EWmlCmdShowToolBar, ETrue);
            }

        // disable fullscreen mode for touch only -- which has auto fullscreen now
        if (iPenEnabled)
            {
            aMenuPane->SetItemDimmed(EWmlCmdEnterFullScreenBrowsing, ETrue);
            }

        // rotate
        if (!ApiProvider().Preferences().RotateDisplay() )
            {
            aMenuPane->SetItemDimmed( EWmlCmdRotateDisplay, ETrue );
            }

        // page overview
        if ( !ApiProvider().Preferences().UiLocalFeatureSupported( KBrowserGraphicalPage ) ||
             ApiProvider().WindowMgr().CurrentWindow()->WMLMode() )
            {
            aMenuPane->SetItemDimmed( EWmlCmdShowMiniature, ETrue);
            }

        // show images
        if( !ApiProvider().BrCtlInterface().ImageCountL() )
            {
            aMenuPane->SetItemDimmed( EWmlCmdShowImages, ETrue );
            }

        // load images
        if (( ApiProvider().Preferences().AutoLoadContent() == EWmlSettingsAutoloadImagesNoFlash) ||
              (ApiProvider().Preferences().AutoLoadContent() == EWmlSettingsAutoloadAll) ||
              ApiProvider().WindowMgr().CurrentWindow()->HasLoadedImages() )
            {
            aMenuPane->SetItemDimmed( EWmlCmdLoadImages, ETrue );
            }

        // window
        if ( !ApiProvider().Preferences().UiLocalFeatureSupported( KBrowserMultipleWindows ) ||
             ApiProvider().IsEmbeddedModeOn() ||
             ApiProvider().WindowMgr().WindowCount() < 2)
            {
            aMenuPane->SetItemDimmed( EWmlCmdSwitchWindow, ETrue );
            }
        }
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::CheckForEmptyWindowsMenuL
// Checks for the existence of Windows Submenu Items return true if no submenu items exist
// -----------------------------------------------------------------------------
//
TBool CBrowserContentView::CheckForEmptyWindowsMenuL(TWindowsMenuItemsDimCheck* aWindowMenuItems)
    {

    __ASSERT_DEBUG( (aWindowMenuItems != NULL), Util::Panic( Util::EUninitializedData ));

    // 'OpenLinkInNewWindow is not available for 'non-anchor' elements or
    // if current page has wml content.
    //TBrCtlDefs::TBrCtlElementType elementtype =
    //    ApiProvider().BrCtlInterface().FocusedElementType();

    // The commented part below enables the "Open link in new window" option.
    // Currently the option is permanently dimmed
    // UI change request AHUN-6U3NT4, S60 bug AHUN-6UYT6N
    aWindowMenuItems->dimOpenInNewWin =  ETrue; /*( (elementtype != TBrCtlDefs::EElementAnchor) ||
                         (ApiProvider().WindowMgr().CurrentWindow()->HasWMLContent(EFalse)) ); */


    // Multiple Windows Supported
    if ( ApiProvider().Preferences().UiLocalFeatureSupported(
                                                    KBrowserMultipleWindows ) )
        {
        TInt winCount = ApiProvider().WindowMgr().WindowCount();

        // 'Close Window' & Switch Window needed if are 2+ open windows
        if ( winCount > 1 )
            {
            aWindowMenuItems->dimCloseWin = EFalse;
            // don't allow window switching if current window has wml.  must close this window first
            aWindowMenuItems->dimSwitchWin = ApiProvider().WindowMgr().CurrentWindow()->HasWMLContent(EFalse /*is any page wml?*/);//EFalse;
            }

        // Pop-up blocking is enabled
        if ( ApiProvider().Preferences().PopupBlocking() )
            {
            //---------------------------------------------------------------------
            // Popup Blocking Menu items
            HBufC* url = ApiProvider().WindowMgr().CurrentWindow()->
                BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoUrl );
            if( url )
                {
                // 'Allow Popups' needed if the current URL is not on the whitelist
                TRAPD( errWhite,
                aWindowMenuItems->dimAllowPopups = ApiProvider().PopupEngine().IsUrlOnWhiteListL( *url );
                            );
                // Error handling
                if ( errWhite )
                    {
                    // Better to have the menu item than not
                    aWindowMenuItems->dimAllowPopups = aWindowMenuItems->dimBlockPopups = EFalse;
                    }
                else
                    {
                    aWindowMenuItems->dimBlockPopups = !aWindowMenuItems->dimAllowPopups;
                    }
                }
            // else no url (strange case), so no popup specific menuitem

            CleanupStack::PopAndDestroy( url );
            }
        //=========================================================================
        }
    return (aWindowMenuItems->dimOpenInNewWin &&
            aWindowMenuItems->dimSwitchWin &&
            aWindowMenuItems->dimCloseWin &&
            aWindowMenuItems->dimAllowPopups &&
            aWindowMenuItems->dimBlockPopups);
    }


// -----------------------------------------------------------------------------
// CBrowserContentView::DimMultipleWindowsMenuItems
// Dims the items of the windows submenu according to aWindowMenuItems
// -----------------------------------------------------------------------------
//
void CBrowserContentView::DimMultipleWindowsMenuItems( CEikMenuPane& aMenuPane, TWindowsMenuItemsDimCheck aWindowMenuItems )
    {
    aMenuPane.SetItemDimmed( EWmlCmdOpenLinkInNewWindow, aWindowMenuItems.dimOpenInNewWin );
    aMenuPane.SetItemDimmed( EWmlCmdSwitchWindow,  aWindowMenuItems.dimSwitchWin );
    aMenuPane.SetItemDimmed( EWmlCmdCloseWindow,   aWindowMenuItems.dimCloseWin );
    aMenuPane.SetItemDimmed( EWmlCmdAllowPopups,   aWindowMenuItems.dimAllowPopups );
    aMenuPane.SetItemDimmed( EWmlCmdBlockPopups,   aWindowMenuItems.dimBlockPopups );
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::OptionListInit
// Callback - we should initialize the list of DO elements
// -----------------------------------------------------------------------------
//
void CBrowserContentView::OptionListInitL()
    {
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::HandleGotoPaneEventL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::HandleGotoPaneEventL(
                                               CBrowserGotoPane* /*aGotoPane*/,
                                               MGotoPaneObserver::TEvent aEvent )
    {
    switch ( aEvent )
        {
        case MGotoPaneObserver::EEventEnterKeyPressed:
            {
             if( !MenuBar()->MenuPane()->IsVisible() )
                {
                if (iContainer->GotoPane()->PopupList() != NULL)
                    {
                    if ( iContainer->GotoPane()->PopupList()->IsOpenDirToShow() )
                        {
                        HandleCommandL( EWmlCmdOpenFolder );
                        }
                    else
                        {
                        HandleCommandL( EWmlCmdGotoPaneGoTo );
                        }
                    }
                }
            break;
            }

        default:
            {
            break;
            }
        }
    }


// -----------------------------------------------------------------------------
// CBrowserContentView::OnScreenPosition
// -----------------------------------------------------------------------------
//
TPoint CBrowserContentView::OnScreenPosition()
    {
    TRect rect = AppUi()->ApplicationRect();
    TPoint point (rect.iTl);
    if (iPenEnabled)
        {
        if (AppUi()->TouchPane())
            {
            TRect touchRect = AppUi()->TouchPane()->Rect();
            if (touchRect.iTl == rect.iTl)
                {
                if (touchRect.Width() > touchRect.Height()) // Horizontal, on top
                    {
                    point = TPoint(rect.iTl.iX, touchRect.iBr.iY);
                    }
                else // Vertical, on the left
                    {
                    point = TPoint(touchRect.iBr.iX, rect.iTl.iY);
                    }
                }
            }
        }
    return point;
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::GotoUrlInGotoPaneL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::GotoUrlInGotoPaneL()
    {
    if ( iContainer->GotoPane() )
        {
        delete iEnteredURL;
        iEnteredURL = NULL;
        iEnteredURL = iContainer->GotoPane()->GetTextL(); // the url
        if( iEnteredURL )
            {
            iContainer->ShutDownGotoURLEditorL();

            TFavouritesWapAp accessPoint;
            // if current ap exists then use it. other ways use default ap.
            if( ApiProvider().Connection().CurrentAPId() )
                {
                TUint apId = ApiProvider().Connection().CurrentAPId();
                apId = Util::WapIdFromIapIdL( ApiProvider(), apId );
                accessPoint.SetApId( apId );
                }
            else
                {
                accessPoint.SetApId(
                ApiProvider().Preferences().DefaultAccessPoint() );
                }
            UpdateCbaL();
            UpdateFullScreenL();

            ApiProvider().FetchL(   iEnteredURL->Des(),
                                    KNullDesC,
                                    KNullDesC,
                                    accessPoint,
                                    CBrowserLoadObserver::ELoadUrlTypeOther );

            }
        else  // no URL entered
            {
            iContainer->ShutDownGotoURLEditorL();
            UpdateCbaL();
            UpdateFullScreenL();
            }
        }
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::UpdateGotoPaneL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::UpdateGotoPaneL()
    {
    // Reset goto pane content
    if( AUTOCOMP ) //ask the feature manager
        {
        //2.1 does not put http:// there
        iContainer->GotoPane()->SetTextL( KNullDesC );
        }
    else
        {
        //2.0 does need http://
        iContainer->GotoPane()->SetTextL( KHttpString );
        }
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::LaunchGotoAddressEditorL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::LaunchGotoAddressEditorL()
    {
    // Cancel editing and sets Goto Pane text back.
    if (iContainer->GotoPane()->PopupList() != NULL)
        {
        iContainer->GotoPane()->PopupList()->SetDirectoryModeL( ETrue );
        iContainer->GotoPane()->PopupList()->HidePopupL();
        }

    if ( ApiProvider().Preferences().FullScreen() == EWmlSettingsFullScreenFullScreen )
        {
        MakeCbaVisible( ETrue );
        }

    CCoeEnv::Static()->AppUi()->RemoveFromStack( iContainer->GotoPane() );
    CCoeEnv::Static()->AppUi()->AddToStackL( iContainer->GotoPane(), ECoeStackPriorityMenu );
    iContainer->GotoPane()->MakeVisible( ETrue );
    if ( iEnteredURL && iEnteredURL->Length() )
       {
            iContainer->GotoPane()->SetTextL(*iEnteredURL);
            iContainer->GotoPane()->SelectAllL();
       }
    else
        {
            // nor iEnteredURL; use default name.
            // set "http://www." per Browser UI spec
            iContainer->GotoPane()->SetTextL(KWWWString  );
        }
    iContainer->GotoPane()->SetFocus( ETrue );
    UpdateCbaL();

    if (iPenEnabled)
        {
        Toolbar()->SetDimmed(ETrue);
        Toolbar()->DrawNow();
        }

    }


// -----------------------------------------------------------------------------
// CBrowserContentView::LaunchFindKeywordEditorL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::LaunchFindKeywordEditorL()
    {
    // Cancel editing and sets Goto Pane text back.
    if (iContainer->FindKeywordPane()->PopupList() != NULL)
        {
        iContainer->FindKeywordPane()->PopupList()->SetDirectoryModeL( ETrue );
        iContainer->FindKeywordPane()->PopupList()->HidePopupL();
        }

    if ( ApiProvider().Preferences().FullScreen() == EWmlSettingsFullScreenFullScreen )
        {
        MakeCbaVisible( ETrue );
        }

    CCoeEnv::Static()->AppUi()->RemoveFromStack( iContainer->FindKeywordPane() );
    CCoeEnv::Static()->AppUi()->AddToStackL( iContainer->FindKeywordPane(), ECoeStackPriorityMenu );
    iContainer->FindKeywordPane()->MakeVisible( ETrue );
    iContainer->FindKeywordPane()->SetFocus( ETrue );
    UpdateCbaL();
    if (iPenEnabled)
        {
#ifdef RD_SCALABLE_UI_V2
        Toolbar()->ToolbarExtension()->SetShown(EFalse);
#endif
        iBrowserContentViewToolbar->UpdateButtonsStateL();
        }
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::UpdateTitleL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::UpdateTitleL( MApiProvider& aApiProvider )
    {
    HBufC* title = aApiProvider.BrCtlInterface().PageInfoLC(
        TBrCtlDefs::EPageInfoTitle );
    if ( title && title->Length() )
        {
        aApiProvider.Display().SetTitleL( title->Des() );
        }
    else
        {

        TInt offset = 0;
        HBufC* titleUrl = aApiProvider.BrCtlInterface().PageInfoLC(
            TBrCtlDefs::EPageInfoUrl );
        if( titleUrl == NULL )  // is this really possible?
            {
            CleanupStack::PopAndDestroy( titleUrl );
            titleUrl = KNullDesC().AllocLC();
            }
        offset = titleUrl->Find( KSchemaIdentifier );
        if(offset != KErrNotFound )
            {
            titleUrl->Des().Delete(0,offset + KSchemaIdentifierLength);
            }
        aApiProvider.Display().SetTitleL( titleUrl->Des() );
        CleanupStack::PopAndDestroy( );  // titleUrl
        }

    CleanupStack::PopAndDestroy(); // title
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::AddNewBookmarkL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::AddNewBookmarkL( TBool aAsLastVisited, HBufC* aUrl )
    {
    TInt pop = 0;
    HBufC* cardTitle = NULL;
    TPtrC cardTitlePtr( KNullDesC );

    // Make a bookmark model.
    iBookmarksModel = CBrowserBookmarksModel::NewL(ApiProvider());

    // Create a bookmark item.
    CFavouritesItem* item = CFavouritesItem::NewLC();

    HBufC* theUrl = NULL;
    if (aUrl == NULL)
        {
        // Set URL.
        theUrl = ApiProvider().BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoUrl );
        if( !theUrl || !theUrl->Length() )
            {
            CleanupStack::PopAndDestroy( theUrl );
            theUrl = KNullDesC().AllocLC();
            }
        pop++;
        // Get the title
        cardTitle = ApiProvider().BrCtlInterface().PageInfoLC(TBrCtlDefs::EPageInfoTitle );
        pop++;
        }
    else
        {
        theUrl = aUrl;
        }

    if ( cardTitle )
        {
        cardTitle->Des().Trim();
        cardTitlePtr.Set( cardTitle->Des() );
        }

    if ( cardTitlePtr.Length() )
        {
        // Current card has non-empty title, use that as bookmark name.
        item->SetNameL( cardTitlePtr.Left(KFavouritesMaxName) );
        }
    else
        {
        // Get the URL
        HBufC* url = theUrl->AllocLC();
        pop++;

        if ( url && url->Length() )
            {
            TUriParser16 urlParser;
            urlParser.Parse(*url);
            url->Des().SetLength( 0 );
            url->Des().Append( urlParser.Extract( EUriHost) );
            url->Des().Append( urlParser.Extract( EUriPath ) );
            item->SetNameL( url->Left(KFavouritesMaxName) );
            }
        else
            {
            // No title, nor url; use default name.
            iBookmarksModel->SetNameToDefaultL( *item );
            }
        }

    // set url, username, password
    Util::RetreiveUsernameAndPasswordFromUrlL( *theUrl, *item );
    CleanupStack::PopAndDestroy(pop);  // url, cardTitle, theUrl

    item->SetParentFolder( KFavouritesRootUid );

    // Set WAP AP.
    TFavouritesWapAp ap;    // default AP
    item->SetWapAp( ap );

    if ( aAsLastVisited )
        {
        iBookmarksModel->SetLastVisitedL( *item, EFalse,
        CBrowserFavouritesModel::EAutoRename );
        }

    else
        {
        HBufC* prompt = StringLoader::LoadLC( R_WML_ENTER_BOOKMARK_NAME );
        TBuf<KBrowserMaxPrompt+1> retString;
        retString.Copy( item->Name() );

        TBool queryName = ETrue;
        while( queryName )
            {
            TInt result = TBrowserDialogs::DialogPromptReqL(
                prompt->Des(),
                &retString,
                EFalse,
                KFavouritesMaxBookmarkNameDefine );
            if( result )
                {
                item->SetNameL(retString.Left(KFavouritesMaxName));
                TInt err = iBookmarksModel->AddL(
                    *item,
                    ETrue,
                    CBrowserFavouritesModel::EDontRename );
                if ( !err )
                    {
                    TBrowserDialogs::InfoNoteL(
                                R_BROWSER_OK_NOTE,
                                iBookmarksModel->StringResourceId
                                ( *item, CBrowserFavouritesModel::ESaved ) );
                    queryName = EFalse;
                    AddBMUidToLastPlaceToCurrentListL( item->Uid() );
                    
                    // To retain FavIcon in iconDatabase, we need to reference icon bitmap reference at least once
                    HBufC* url = ApiProvider().BrCtlInterface().PageInfoLC(TBrCtlDefs::EPageInfoUrl);
                    CGulIcon *icon = ApiProvider().BrCtlInterface().GetBitmapData(*url, TBrCtlDefs::EBitmapFavicon);
                    CleanupStack::PopAndDestroy(); //url
                    delete icon;
                    }
                else if( KErrAlreadyExists == err )
                    {
                    TBrowserDialogs::InfoNoteL(
                            R_BROWSER_INFO_NOTE, R_WML_NAME_ALREADY_IN_USE );
                    }
                else
                    {
                    TBrowserDialogs::ErrorNoteL(
                        iBookmarksModel->StringResourceId(
                        *item,
                        CBrowserFavouritesModel::ENotSaved ) );

                    }
                }
            else
                queryName = EFalse;
            }
        CleanupStack::PopAndDestroy();  // prompt
        }

    CleanupStack::PopAndDestroy();   // item

    delete iBookmarksModel;
    iBookmarksModel = NULL;
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::AddBMUidToLastPlaceToCurrentListL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::AddBMUidToLastPlaceToCurrentListL(const TInt aUid )
    {
    CArrayFixFlat<TInt>* orderArray = new (ELeave) CArrayFixFlat<TInt>(KGranularityHigh);
    CleanupStack::PushL( orderArray );

    CBrowserBookmarksOrder* currentOrder = CBrowserBookmarksOrder::NewLC();
    if ( iBookmarksModel->Database().GetData( KFavouritesRootUid ,*currentOrder ) == KErrNone)
        {
        if ( currentOrder->GetBookMarksOrder().Count() > 0 )
            {
            orderArray->AppendL( &( currentOrder->GetBookMarksOrder()[0] ),
                currentOrder->GetBookMarksOrder().Count() );
            }
        }
    iBookmarksModel->AddUidToLastPlaceL(aUid, orderArray, currentOrder );
    currentOrder->SetBookMarksOrderL( *orderArray );
    iBookmarksModel->Database().SetData( KFavouritesRootUid ,*currentOrder );
    CleanupStack::PopAndDestroy( 2 );// currentOrder, orderArray
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::SendAddressL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::SendAddressL( )
    {
    HBufC* url = ApiProvider().BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoUrl );
    if( url == NULL )
        {
        CleanupStack::PopAndDestroy( url );  // remove previous
        url = KNullDesC().AllocLC();
        }

#ifndef _BOOKMARK_SENT_ASCII
    if( url->Length() <= KMaxUrlLenghtInOTA )
#endif  // _BOOKMARK_SENT_ASCII
        {
        HBufC* title = ApiProvider().BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoTitle );
        if( title == NULL )
            {
            CleanupStack::PopAndDestroy( title );  // remove previous
            title = KNullDesC().AllocLC();
            }

        //CBrowserAppUi* ui = reinterpret_cast<CBrowserAppUi*>(AppUi());
        MBmOTABinSender& sender = ApiProvider().BmOTABinSenderL();

        sender.ResetAndDestroy();
        sender.AppendL( url->Des(), title->Des() );
        sender.SendAddressL( );
        CleanupStack::PopAndDestroy( title );  // title
        }
#ifndef _BOOKMARK_SENT_ASCII
    else
        {
#pragma message ( __FILE__ ": Length of bookmark addr cannot exceed 255bytes!")
        _LIT( KErrMsg, "URL address of bookmark is too long. It cannot be sent.");

        TBrowserDialogs::ErrorNoteL( KErrMsg );
        }
#endif  // _BOOKMARK_SENT_ASCII
    CleanupStack::PopAndDestroy( url );  // url
    }


// -----------------------------------------------------------------------------
// CBrowserContentView::UpdateOptionMenuItemLabelL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::UpdateOptionMenuItemLabelL(CEikMenuPane* aMenuPane, const TInt aCommandId, const TInt aResourceId)
    {

     __ASSERT_DEBUG( (aMenuPane != NULL), Util::Panic( Util::EUninitializedData ));

    HBufC* browserLabel = CCoeEnv::Static()->AllocReadResourceLC(aResourceId);
    aMenuPane->SetItemTextL(aCommandId, *browserLabel);
    CleanupStack::PopAndDestroy();
    }


// -----------------------------------------------------------------------------
// CBrowserContentView::DoSearchL
// Do search for specified items in current page
// -----------------------------------------------------------------------------
//
void CBrowserContentView::DoSearchL(
                            CFindItemEngine::TFindItemSearchCase aSearchCase )
    {
    HBufC* findText = NULL;
    findText = ApiProvider().BrCtlInterface().PageInfoLC(
                                                TBrCtlDefs::EPageInfoContent );
    if( findText == NULL )
        {
        CleanupStack::PopAndDestroy( findText );
        return;
        }

    if( findText->Length() != 0 )
        {

        TPtr findTextDes = findText->Des();

        for( int i = 0; i < findText->Length(); ++i )
            {
            if( (*findText)[i] == TUint16( 0x0a ) )
                {
                findTextDes[i] = TUint16( CEditableText::EParagraphDelimiter );
                }

            else if( (*findText)[i] == TUint16( CEditableText::ETabCharacter ) )
                {
                findTextDes[i] = TUint16( CEditableText::ESpace );
                }
            else if( (*findText)[i] == TUint16( CEditableText::ELineBreak ) )
                {
                findTextDes[i] = TUint16( CEditableText::EParagraphDelimiter );
                }
            }
        }

    SetFullScreenOffL();
    CFindItemDialog* dialog =  CFindItemDialog::NewL( *findText, aSearchCase );
    dialog->ExecuteLD();
    UpdateFullScreenL();
    UpdateTitleL( ApiProvider() );

    CleanupStack::PopAndDestroy( findText );
    }


// -----------------------------------------------------------------------------
// CBrowserContentView::SetLastVisitedBookmarkL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::SetLastVisitedBookmarkL()
    {
    AddNewBookmarkL( ETrue );
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::UpdateNaviPaneL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::UpdateNaviPaneL( TDesC& aStatusMsg )
    {
    //Set text that will be shown in navipane
    iStatusMsg.Des().Copy(aStatusMsg.Left(KStatusMessageMaxLength));//Might be longer than iStatusMsg
    SetNavipaneViewL();
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::SetNavipaneViewL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::SetNavipaneViewL()
    {
    TVwsViewId activeViewId;
    if ( AppUi()->GetActiveViewId( activeViewId ) == KErrNone )
        {
        if ( (activeViewId.iViewUid == KUidBrowserContentViewId) )
            {
            if (!iNaviDecorator)
                {
                delete iNaviDecorator;
                iNaviDecorator = NULL;

                if( iNaviPane )
                    {
                    if (iStatusMsg.Length())
                        {
                        iNaviDecorator = iNaviPane->CreateNavigationLabelL(
                            iStatusMsg );
                        }
                    else
                        {
                        iNaviDecorator = iNaviPane->CreateNavigationLabelL(_L(""));
                        }
                    }
                iNaviPane->PushL( *iNaviDecorator );
                }
            else
                {
                CAknNaviLabel* naviLabel = STATIC_CAST( CAknNaviLabel*,
                    iNaviDecorator->DecoratedControl());
                naviLabel->SetTextL(iStatusMsg);
                // make sure at top and redraw
                iNaviPane->PushL( *iNaviDecorator );
                }

            iNaviDecorator->MakeScrollButtonVisible(ETrue);
            }
        }
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::FindItemIsInProgress
// -----------------------------------------------------------------------------
//
TBool CBrowserContentView::FindItemIsInProgress()
    {
    return iFindItemIsInProgress;
    }

// ---------------------------------------------------------------------------
//  CBrowserContentView::HandleStatusPaneSizeChange
// ---------------------------------------------------------------------------
//
void CBrowserContentView::HandleStatusPaneSizeChange()
    {
    iContainer->SetRect(ClientRect());
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::StateChanged
// ---------------------------------------------------------------------------
//
void CBrowserContentView::StateChanged(
        TBrCtlDefs::TBrCtlState aState,
        TInt aValue )
    {
LOG_ENTERFN(" CBrowserContentView::StateChanged" );
LOG_WRITE_FORMAT(" state: %d", aState );
LOG_WRITE_FORMAT(" value: %d", aValue );
    switch(aState)
        {
        case TBrCtlDefs::EStateImageMapView:
            {
            iImageMapActive = (TBool) aValue;
            TRAP_IGNORE( UpdateCbaL() );
            break;
            }
        case TBrCtlDefs::EStateHistoryView:
            {
            iHistoryViewIsUp = (TBool) aValue;
            iHistoryAtBeginning = EFalse;
            iHistoryAtEnd = EFalse;
            if ( iPenEnabled )
                {
                TRAP_IGNORE( iBrowserContentViewToolbar->UpdateButtonsStateL() );
                }
            TRAP_IGNORE( UpdateCbaL() );
            TRAP_IGNORE( UpdateTitleL( ApiProvider() ) );
			if (iPenEnabled)
				{
	            StartAutoFullScreenTimer();
				}
            break;
            }
        case TBrCtlDefs::EStateThumbnailView:
            {
            iThumbnailViewIsUp = (TBool) aValue;
            if ( iPenEnabled )
                {
                TRAP_IGNORE( iBrowserContentViewToolbar->UpdateButtonsStateL() );
                }
            TRAP_IGNORE( UpdateCbaL() );
            TRAP_IGNORE( UpdateTitleL( ApiProvider() ) );
			if (iPenEnabled)
				{
	            StartAutoFullScreenTimer();
				}
            break;
            }
        case TBrCtlDefs::EStatePluginPlayer:
            {
            iPluginPlayerIsUp = (TBool) aValue;
            TRAP_IGNORE( UpdateCbaL() );
            TRAP_IGNORE( UpdateTitleL( ApiProvider() ) );
            break;
            }
        case TBrCtlDefs::EStateSmartTextView:
            {
            iSmartTextViewIsUp = (TBool) aValue;
            TRAP_IGNORE( UpdateCbaL() );
            TRAP_IGNORE( UpdateTitleL( ApiProvider() ) );
            break;
            }
        case TBrCtlDefs::EStateToolBarMode:
            {
            if(!aValue)
                {
                TRAP_IGNORE( UpdateCbaL() );
                }
            break;
            }
        case TBrCtlDefs::EStateScriptLog:
            {
            TRAP_IGNORE( ApiProvider().Preferences().SetScriptLogL( aValue ) );
            break;
            }
        case TBrCtlDefs::EStateZoomSliderMode:
            {
            if ( aValue )
                {
                iZoomSliderVisible = ETrue;
                }
            else
                {
                iZoomSliderVisible = EFalse;
				if (iPenEnabled)
					{
	    	        StartAutoFullScreenTimer();
					}
                }
            // Inform Toolbar of this fact
            if ( iPenEnabled )
                {
                TRAP_IGNORE( iBrowserContentViewToolbar->UpdateButtonsStateL() );
                }
            break;
            }
        case TBrCtlDefs::EStateFullscreenBrowsing:
            {
            if (!iPenEnabled && IsEditMode())
                {
                if (!aValue)
                    { // exit fullscreen so remember what to go back to after edit mode is exited
                    iFullScreenBeforeEditModeEntry = iContentFullScreenMode;
                    TRAP_IGNORE( EnableFullScreenModeL( EFalse ) );
                    }
                else if (aValue && iFullScreenBeforeEditModeEntry)
                    {
                    TRAP_IGNORE( EnableFullScreenModeL( ETrue ) );
                    }
                }
            else if (!aValue)
                {
                    TRAP_IGNORE( EnableFullScreenModeL( EFalse ) );
                }
            break;
            }
        case TBrCtlDefs::EStateHistoryBeginning:
            {
            if(aValue)
                {
                iHistoryAtBeginning = ETrue;
                }
            else
                {
                iHistoryAtBeginning = EFalse;
                }
            if (iPenEnabled)
                {
                TRAP_IGNORE( iBrowserContentViewToolbar->UpdateButtonsStateL() );
                }
            TRAP_IGNORE( UpdateTitleL( ApiProvider() ) );
            break;
            }
        case TBrCtlDefs::EStateHistoryEnd:
            {
            if(aValue)
                {
                iHistoryAtEnd = ETrue;
                }
            else
                {
                iHistoryAtEnd = EFalse;
                }
            if (iPenEnabled)
                {
                TRAP_IGNORE( iBrowserContentViewToolbar->UpdateButtonsStateL() );
                }
            TRAP_IGNORE( UpdateTitleL( ApiProvider() ) );
            break;
            }
        case TBrCtlDefs::EStateSynchRequestMode:
            {
            iSynchRequestViewIsUp = (TBool) aValue;
            if (iPenEnabled)
                {
                TRAP_IGNORE( iBrowserContentViewToolbar->UpdateButtonsStateL() );
                }
            TRAP_IGNORE( UpdateCbaL() );
            TRAP_IGNORE( UpdateTitleL( ApiProvider() ) );
            break;
            }
        case TBrCtlDefs::EStatePluginFullScreen:
            HandlePluginFullScreen((TBool) aValue);
            break;
        default:
            break;
        }

    ApiProvider().WindowMgr().SetCurrentWindowViewState(aState, aValue);
    if (iPenEnabled && ApiProvider().WindowMgr().CurrentWindow()->WMLMode())
    	{
        EnableFullScreenModeL( EFalse );
    	}

    }

// -------------------------------------------------------------------
// CBrowserContentView::HandleCommandL
// ---------------------------------------------------------------------------
//
void CBrowserContentView::HandleCommandL(
    TBrCtlDefs::TBrCtlClientCommands aCommand,
    const CArrayFix<TPtrC>& /*aAttributesNames*/,
    const CArrayFix<TPtrC>& aAttributeValues)
    {
    switch(aCommand)
        {
        case TBrCtlDefs::EClientCommandLaunchFindKeyword:
            {
            // Find Keyword not initiated from options menu so don't set menupane to visible.
            LaunchFindKeywordEditorL();
            MenuBar()->MenuPane()->MakeVisible(EFalse);
            break;
            }
        case TBrCtlDefs::EClientCommandGotoWebAddress:
            {
            HandleCommandL(EWmlCmdGoToAddress);
            break;
            }

        case TBrCtlDefs::EClientCommandSaveAsBookmark:
            {
            HandleCommandL(EWmlCmdSaveAsBookmark);
            break;
            }

        case TBrCtlDefs::EClientCommandManageBookmarks:
            {
            HandleCommandL(EWmlCmdFavourites);
            break;
            }

        case TBrCtlDefs::EClientCommandZoomIn:
            {
            ZoomImagesInL();
            break;
            }

        case TBrCtlDefs::EClientCommandZoomOut:
            {
            ZoomImagesOutL();
            break;
            }

        case TBrCtlDefs::EClientCommandZoomMode:
            {
            ZoomModeImagesL();
            break;
            }

        case TBrCtlDefs::EClientCommandGoToHompage:
            {
            HandleCommandL(EWmlCmdLaunchHomePage);
            break;
            }
        case TBrCtlDefs::EClientCommandRotateScreen:
            {
            HandleCommandL(EWmlCmdRotateDisplay);
            break;
            }
        case TBrCtlDefs::EClientCommandSavePage:
            {
            HandleCommandL(EWmlCmdSavePage);
            break;
            }
        case TBrCtlDefs::EClientCommandSwitchWindow:
            {
            HandleCommandL(EWmlCmdSwitchWindow);
            break;
            }
        case TBrCtlDefs::EClientCommandShowHelp:
            {
            HandleCommandL(EAknCmdHelp);
            break;
            }

        case TBrCtlDefs::EClientCommandToolbarConfigure:
            {
            // open settings view in toolbar group
            AppUi()->HandleCommandL( EWmlCmdPreferencesToolbar );
            break;
            }

        case TBrCtlDefs::EClientCommandToolbarShowKeymap:
            {
            ShowKeymap();
            break;
            }

        case TBrCtlDefs::EClientCommandSubscribeToFeeds:
            {
#ifdef __RSS_FEEDS
            const TPtrC16  name(aAttributeValues.At(0));
            const TPtrC16  url(aAttributeValues.At(1));

            ApiProvider().SetLastActiveViewId( KUidBrowserFeedsFolderViewId );
            ApiProvider().FeedsClientUtilities().SubscribeToL( name, url );
            // switch back to FeedsView, so change layout
            SetFullScreenOffL();
#endif
            break;
            }

        case TBrCtlDefs::EClientCommandShowContextMenu:
            {
            // Get ElementType
            const TPtrC elTypePtr(aAttributeValues.At(0));
            TLex lex(elTypePtr);
            TInt elType;
            TInt resourceId = 0;
            if (lex.Val(elType) == KErrNone)
                {
                switch (elType)
                    {
                    case TBrCtlDefs::EElementNone:
                        resourceId = R_BROWSER_EMPTY_SPACE_STYLUS_POPUP_MENU;
                        break;
                    case TBrCtlDefs::EElementAnchor:
                    case TBrCtlDefs::EElementTelAnchor:
                    case TBrCtlDefs::EElementMailtoAnchor:
                        resourceId = R_BROWSER_ANCHOR_STYLUS_POPUP_MENU;
                        break;
                    case TBrCtlDefs::EElementSmartLinkTel:
                        resourceId = R_BROWSER_PHONE_NUMBER_STYLUS_POPUP_MENU;
                        break;
                    case TBrCtlDefs::EElementSmartLinkEmail:
                        resourceId = R_BROWSER_EMAIL_ADDRESS_STYLUS_POPUP_MENU;
                        break;
                    case TBrCtlDefs::EElementImageBox:
                        resourceId = R_BROWSER_IMAGE_STYLUS_POPUP_MENU;
                        break;
                    case TBrCtlDefs::EElementBrokenImage:
                        if (ApiProvider().Preferences().AutoLoadContent() == EWmlSettingsAutoloadText)
                            {
                            if (ApiProvider().FlashPresent())
                                {
                                resourceId = R_BROWSER_IMAGE_PLACEHOLDER_STYLUS_POPUP_MENU_WITH_FLASH;
                                }
                            else
                                {
                                resourceId = R_BROWSER_IMAGE_PLACEHOLDER_STYLUS_POPUP_MENU_NO_FLASH;
                                }
                            }
                        break;
                    case TBrCtlDefs::EElementAreaBox:
                        resourceId = R_BROWSER_IMAGEMAP_STYLUS_POPUP_MENU;
                        break;
                    default:
                        break;
                    }
                if (resourceId != 0)
                    {
                    TPoint point;
                    // Get point X
                    const TPtrC xPtr(aAttributeValues.At(1));
                    lex.Assign(xPtr);
                    lex.Val(point.iX);
                    // Get point Y
                    const TPtrC yPtr(aAttributeValues.At(2));
                    lex.Assign(yPtr);
                    lex.Val(point.iY);

                    delete iStylusPopupMenu;
                    iStylusPopupMenu = NULL;
                    iStylusPopupMenu = CAknStylusPopUpMenu::NewL( this, point);
                    TResourceReader rr;
                    CCoeEnv::Static()->CreateResourceReaderLC(rr, resourceId);
                    iStylusPopupMenu->ConstructFromResourceL( rr );
                    CleanupStack::PopAndDestroy(); // resource reader
                    iStylusPopupMenu->SetPosition( point);
                    iStylusPopupMenu->ShowMenu();
                    }
                }
            break;
            }

        case TBrCtlDefs::EClientCommandToolbarSettings:
            {
            // Launch Settings View
            HandleCommandL(EWmlCmdPreferences);
            break;
            }

        case TBrCtlDefs::EClientCommandFullScreen:
            {
            HandleCommandL(EWmlCmdEnterFullScreenBrowsing);
            break;
            }

        default:
            break;
        }
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::HandleSubscribeToL
// Handles a selection from the subscribe to sub-menu.
// -----------------------------------------------------------------------------
//
void CBrowserContentView::HandleSubscribeToL( TInt aCommand )
    {
#ifndef __RSS_FEEDS
    (void) aCommand;
#else  // __RSS_FEEDS
    TInt index;

    // Get the corresponding feed info.
    const RPointerArray<TBrCtlSubscribeTo>& items = ApiProvider().
            BrCtlInterface().SubscribeToMenuItemsL();

    index = aCommand - TBrCtlDefs::ECommandIdSubscribeToBase;
    if ((index >= 0) && (index < items.Count()))
        {
        const TPtrC16  name(items[index]->Title());
        const TPtrC16  url(items[index]->Url());

        ApiProvider().SetLastActiveViewId( KUidBrowserFeedsFolderViewId );
        ApiProvider().FeedsClientUtilities().SubscribeToL( name, url );
        // switch back to FeedsView, so change layout
        SetFullScreenOffL();
        }
#endif  // __RSS_FEEDS
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::HandleSubscribeToUrlL
// Handles subscribing to a feed from a ur
// -----------------------------------------------------------------------------
//
void CBrowserContentView::HandleSubscribeToWithUrlL( TPtrC aUrl )
    {
#ifndef __RSS_FEEDS
    (void) aUrl;
#else  // __RSS_FEEDS

    ApiProvider().SetLastActiveViewId( KUidBrowserFeedsFolderViewId );

    //Get the 'Title' info by comparing aUrl with the url's populated in array 'items'
    const RPointerArray<TBrCtlSubscribeTo>& items = ApiProvider().BrCtlInterface().SubscribeToMenuItemsL();
    TInt index = 0;
    TInt itemCount = items.Count();

    while (itemCount != 0 && index < itemCount)
        {
        if (!(aUrl.Compare(items[index]->Url())))
            {
            break;
            }
        else
            {
            index++;
            }
        }


    if (itemCount == 0 || index >= itemCount )
        {
        ApiProvider().FeedsClientUtilities().SubscribeToL( KNullDesC, aUrl );
        }
    else
        {
        const TPtrC16  name(items[index]->Title());
        ApiProvider().FeedsClientUtilities().SubscribeToL(name , aUrl );
        }

#endif  // __RSS_FEEDS
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::DynInitToolbarL
// ---------------------------------------------------------------------------
//
void CBrowserContentView::DynInitToolbarL( TInt /*aResourceId*/, CAknToolbar* /*aToolbar*/ )
    {
    // If we have a touch device, check to see if the zoom slider is up,
    // if displayed, close it, so the extended toolbar shows without the
    // zoomslider displaying on top of it.
    if ( PenEnabled() && ZoomSliderVisible() )
        {
        MakeZoomSliderVisibleL( EFalse );
        }

    if ( iPenEnabled )
        {
        iBrowserContentViewToolbar->UpdateButtonsStateL() ;
        }
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::WindowEventHandlerL
// ---------------------------------------------------------------------------
//
void CBrowserContentView::WindowEventHandlerL( TWindowEvent /*aEvent*/, TInt aWindowId )
    {
    if ( aWindowId == ApiProvider().WindowMgr().CurrentWindow()->WindowId() )
        {
        if ( iPenEnabled )
            {
            iBrowserContentViewToolbar->UpdateButtonsStateL();
            }
        }
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::IsHistoryViewUp
// ---------------------------------------------------------------------------
//
TBool CBrowserContentView::IsHistoryViewUp()
    {
    return iHistoryViewIsUp;
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::IsHistoryAtBeginning
// ---------------------------------------------------------------------------
//
TBool CBrowserContentView::IsHistoryAtBeginning()
    {
    return iHistoryAtBeginning;
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::IsHistoryAtEnd
// ---------------------------------------------------------------------------
//
TBool CBrowserContentView::IsHistoryAtEnd()
    {
    return iHistoryAtEnd;
    }


// ---------------------------------------------------------------------------
// CBrowserContentView::FindKeywordL
// ---------------------------------------------------------------------------
//
void CBrowserContentView::FindKeywordL( HBufC* aFindString, TBool aFindDirection )
    {
    TInt findResponse = 0;

    if ( aFindString )
        {
        // Find string
        delete iEnteredKeyword;
        iEnteredKeyword = NULL;
        iEnteredKeyword = aFindString;
        findResponse = ApiProvider().BrCtlInterface().FindKeyword( iEnteredKeyword->Des() );
        }
    else
        {
        // Search next/previous
        findResponse = ApiProvider().BrCtlInterface().FindKeywordAgain( aFindDirection );
        }

    HBufC* toolTipText = NULL;

    switch ( findResponse )
        {
        case TBrCtlDefs::EFindNoMatches:
            toolTipText = StringLoader::LoadLC( R_QTN_BROWSER_KEYWORD_FIND_NO_MATCHES );
            break;
        case TBrCtlDefs::EFindWrapAround:
            toolTipText = StringLoader::LoadLC( R_QTN_BROWSER_KEYWORD_FIND_WRAPAROUND );
            break;
        case TBrCtlDefs::EFindAllMatches:
            toolTipText = StringLoader::LoadLC( R_QTN_BROWSER_KEYWORD_FIND_ALL_CONTENT_SEARCHED );
            break;
        }

    // Show the result of the search.
    if ( toolTipText )
        {
        ApiProvider().DialogsProvider().ShowTooltipL( *toolTipText );
        CleanupStack::PopAndDestroy( toolTipText );
        }

    }

// ---------------------------------------------------------------------------
// CBrowserContentView::HandleClientRectChange
// ---------------------------------------------------------------------------
//
void CBrowserContentView::HandleClientRectChange()
    {
    if( iContainer )
        {
        iContainer->HandleResourceChange(KEikDynamicLayoutVariantSwitch);
        UpdateFullScreenL();
        if(KeymapIsUp())
            {
            RedrawKeymap();
            }
        }
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::SetZoomModeL
// ---------------------------------------------------------------------------
//
void CBrowserContentView::SetZoomModeL( TBool aZoomMode )
    {
    // if ( !ApiProvider().Fetching() )
    if (ApiProvider().ContentDisplayed())
        {
        iZoomMode = aZoomMode;
        TRAP_IGNORE( UpdateCbaL() );
        if ( aZoomMode )
            {
            // Show the current zoom level in tooltip
            SetZoomLevelTitleTextL( R_BROWSER_ZOOM_LEVEL);
            //MakeZoomSliderVisibleL( ETrue );
            }
        }
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::MakeZoomSliderVisibleL
// Informs Browser Control if the Zoom Slider should be visible or not.
// ---------------------------------------------------------------------------
//
void CBrowserContentView::MakeZoomSliderVisibleL( TBool aVisible )
    {
    if ( aVisible && !iZoomSliderVisible )
        {
        // If the page is being fetched then don't show the zoom slider
        if ( ApiProvider().ContentDisplayed())
            {
            ApiProvider().BrCtlInterface().HandleCommandL(
                                    (TInt)TBrCtlDefs::ECommandZoomSliderShow +
                                    (TInt)TBrCtlDefs::ECommandIdBase );
            iZoomSliderVisible = ETrue;
            }
        }
    // Set the zoom slider to invisible only if the toggle is false,
    // and the zoom slider used to be visible
    else if (!aVisible && iZoomSliderVisible )
        {
        ApiProvider().BrCtlInterface().HandleCommandL(
                                    (TInt)TBrCtlDefs::ECommandZoomSliderHide +
                                    (TInt)TBrCtlDefs::ECommandIdBase );
        iZoomSliderVisible = EFalse;
        }

        if ( iPenEnabled )
            {
            iBrowserContentViewToolbar->UpdateButtonsStateL();
            }
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::ZoomImagesInL
// ---------------------------------------------------------------------------
//
void CBrowserContentView::ZoomImagesInL( TInt aDuration )
    {
        if ( ApiProvider().ContentDisplayed() && !ApiProvider().WindowMgr().CurrentWindow()->WMLMode() )
        {
            iZoomLevelArray = ApiProvider().BrCtlInterface().ZoomLevels();

            // Change zoom level
            ZoomImagesL( 1, 0, iZoomLevelArray->Count()-2, aDuration );
        }
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::ZoomImagesOutL
// ---------------------------------------------------------------------------
//
void CBrowserContentView::ZoomImagesOutL( TInt aDuration )
    {
        if ( ApiProvider().ContentDisplayed() && !ApiProvider().WindowMgr().CurrentWindow()->WMLMode() )
        {
        iZoomLevelArray = ApiProvider().BrCtlInterface().ZoomLevels();

        // Change zoom level
        ZoomImagesL( -1, 1, iZoomLevelArray->Count()-1, aDuration );
        }
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::ZoomModeImagesL
// ---------------------------------------------------------------------------
//
void CBrowserContentView::ZoomModeImagesL()
    {
    if ( ApiProvider().ContentDisplayed() && !ApiProvider().WindowMgr().CurrentWindow()->WMLMode() )
        {
        // Retrieve the current zooming index value
        iCurrentZoomLevel = ApiProvider().BrCtlInterface().BrowserSettingL(
                                TBrCtlDefs::ESettingsCurrentZoomLevelIndex );
        iSavedZoomLevel = iCurrentZoomLevel;

        // Toggle current zoom mode. Zoom Mode allows the user to change
        // the zoom level using the navikey or HW zoom key
        SetZoomModeL( ETrue );
        }
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::ZoomImagesL
// ---------------------------------------------------------------------------
//
void CBrowserContentView::ZoomImagesL( TInt aDirection, TUint aLow,
                                       TUint aHigh, TInt aDuration )
    {
    // Retrieve the current zooming index value, it takes effect right away
    iCurrentZoomLevel = ApiProvider().BrCtlInterface().BrowserSettingL(
                            TBrCtlDefs::ESettingsCurrentZoomLevelIndex );
    TInt currentZoomIndex = FindCurrentZoomIndex(iCurrentZoomLevel);

    // Zoom can be increment, decrement or a direct change to a specific value.
    if ( currentZoomIndex >= aLow && currentZoomIndex <= aHigh )
        {
        // Set current zoom level.
        currentZoomIndex += aDirection;
        iCurrentZoomLevel = (*iZoomLevelArray)[currentZoomIndex];

        // Set actual zoom level.
        ApiProvider().BrCtlInterface().SetBrowserSettingL(
            TBrCtlDefs::ESettingsCurrentZoomLevelIndex, iCurrentZoomLevel );

        // Retrieve the current zooming index value, because each page has its
        // minimum zoom level, if smaller than it, engine will ignore
        // so we have to retrieve the correct zoom level
        iCurrentZoomLevel = ApiProvider().BrCtlInterface().BrowserSettingL(
                                TBrCtlDefs::ESettingsCurrentZoomLevelIndex );
        }

    // Display the zoom indicator
    if ( currentZoomIndex <= 0 )
        {
        // Display minimum zoom indicator, it will dismiss after aDuration
        SetZoomLevelTitleTextL( R_BROWSER_ZOOM_LEVEL_MIN, aDuration );
        }
    else if ( currentZoomIndex >= iZoomLevelArray->Count()-1 )
        {
        TInt arrayCount =  iZoomLevelArray->Count()-1;
        // Display maximum zoom indicator, it will dismiss after aDuration
        SetZoomLevelTitleTextL( R_BROWSER_ZOOM_LEVEL_MAX, aDuration );
        }
    else
        {
        // Display zoom percentage indicator, it will dismiss after aDuration
        // We turned off displaying zoom indicator for each level
        // SetZoomLevelTitleTextL( R_BROWSER_ZOOM_LEVEL, aDuration );
        }
    }

// ---------------------------------------------------------------------------
// SaveCurrentZoomLevel
// ---------------------------------------------------------------------------
void CBrowserContentView::SaveCurrentZoomLevel(TBool saveZoom)
{
    if (saveZoom)
        {
        //save the current zoom level by clicking softkey to confirm
        ApiProvider().BrCtlInterface().SetBrowserSettingL(
            TBrCtlDefs::ESettingsCurrentZoomLevelIndex, iCurrentZoomLevel );
        }
    else
        {
        iCurrentZoomLevel = iSavedZoomLevel;
        ApiProvider().BrCtlInterface().SetBrowserSettingL(
            TBrCtlDefs::ESettingsCurrentZoomLevelIndex, iCurrentZoomLevel );
        }
}

// ---------------------------------------------------------------------------
// FindCurrentZoomIndex
// ---------------------------------------------------------------------------
TInt CBrowserContentView::FindCurrentZoomIndex(TInt aCurrentZoomLevel)
{
	TInt index = -1;
	for ( index = 0; index<iZoomLevelArray->Count()-1; index++)
	{
	   if (aCurrentZoomLevel <=(*iZoomLevelArray)[index] )
		{
			break;
		}
	}
	return index;
}

// ---------------------------------------------------------------------------
// CBrowserContentView::SetZoomLevelTitleTextL
// ---------------------------------------------------------------------------
//
void CBrowserContentView::SetZoomLevelTitleTextL( TInt aResourceId, TInt aDuration )
    {
    HBufC* zoomLevelFormat = NULL;

    if ( aResourceId == R_BROWSER_ZOOM_LEVEL_MIN ||
         aResourceId == R_BROWSER_ZOOM_LEVEL_MAX )
        {
        // Display the zoom max or min string
        zoomLevelFormat = StringLoader::LoadLC( aResourceId );

        // Show zoom indicator for aDuration seconds
        ApiProvider().WindowMgr().CurrentWindow()->ChangeTitlePaneTextUntilL( zoomLevelFormat->Des(), aDuration );

        CleanupStack::PopAndDestroy( ); // zoomLevelFormat
        }
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::GetTextZoomLevelsL
// ---------------------------------------------------------------------------
//
void CBrowserContentView::GetTextZoomLevelsL( )
    {
    // Creating the text level
    // Levels must be equal to TFontSize, with step percent 25% or 50%
    // depending on chinese build.
    // This is a temporary solution until make sure if engine
    // provides text zoom levels. So it is currently hardcoded.
    //------
    // China build has only three font sizes:
    // 0, 2, 4 (all small, normal, all large)
    TInt fontSize = 0;
    if ( !AVKONAPAC )
        {
        // font sizes 0,1,2,3,4
        for ( TInt level = 50; level <= 150; level = level + 25 )
            {
            iTextZoomLevelArray->AppendL( level );
            iFontSizeArray->AppendL( fontSize );
            fontSize++;
            }
        }
    else
        {
        // font sizes 0,2,4
        for ( TInt level = 50; level <= 150; level = level + 50 )
            {
            iTextZoomLevelArray->AppendL( level );
            iFontSizeArray->AppendL( fontSize );
            fontSize = fontSize + 2;
            }
        }

    // Get initial text zoom level index, bt comparing
    // the preferences value, with the one in iFontSizeArray,
    // and get the index (i)
    TBool found = EFalse;
    for ( TInt i = 0; i < iFontSizeArray->Count(); i++ )
        {
        if ( ApiProvider().Preferences().FontSize() == (*iFontSizeArray)[i] )
            {
            iCurrentTextZoomLevelIndex = i;
            found = ETrue;
            }
        }
    // If the initial font size value is not found, then the cenrep
    // default is not right, by default initial cenrep font size value
    // should be 0,2,4. So general, and chinese values are the same.
    // Because if for example default cenrep value of font size is 1,
    // then that is not supported in chinese build.
    // General font size values are: 0,1,2,3,4
    // Chinese font size values are: 0,2,4
    if ( !found )
        {
        User::Leave( KErrGeneral );
        }
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::KeymapIsUp
// ---------------------------------------------------------------------------
//
TBool CBrowserContentView::KeymapIsUp()
    {
    return (iShortcutKeyMap != NULL);
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::isZoomMode
// ---------------------------------------------------------------------------
//
TBool CBrowserContentView::isZoomMode()
    {
    // Zoom Mode currently disabled
    // return iZoomMode;
    return EFalse;
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::ShowKeymap
// ---------------------------------------------------------------------------
//
void CBrowserContentView::ShowKeymap()
    {
    iShortcutKeyMap = CBrowserShortcutKeyMap::NewL(iContainer, iApiProvider);
    UpdateCbaL();
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::HideKeymap
// ---------------------------------------------------------------------------
//
void CBrowserContentView::HideKeymap()
    {
    delete(iShortcutKeyMap);
    iShortcutKeyMap = NULL;
    TRAP_IGNORE(UpdateCbaL());
    TRAP_IGNORE(UpdateFullScreenL());
    }

// ---------------------------------------------------------------------------
// CBrowserContentView::RedrawKeymap
// ---------------------------------------------------------------------------
//
void CBrowserContentView::RedrawKeymap()
    {
    HideKeymap();
    ShowKeymap();
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::EnableFullScreenModeL
// -----------------------------------------------------------------------------
//
void CBrowserContentView::EnableFullScreenModeL( TBool aEnableFullScreen )
    {
    TInt command( KErrNotFound );
    if ( aEnableFullScreen )
        {
        if (iPenEnabled)
            {
            Toolbar()->SetToolbarVisibility( EFalse, EFalse );
            Toolbar()->MakeVisible( EFalse );
            }
        iContentFullScreenMode = ETrue;
        UpdateFullScreenL();
        command = TBrCtlDefs::ECommandEnterFullscreenBrowsing;
        }
    else
        {
        if (iPenEnabled)
            {
            Toolbar()->SetToolbarVisibility( ETrue, EFalse );
            }
        iContentFullScreenMode = EFalse;
        SetFullScreenOffL();
        command = TBrCtlDefs::ECommandLeaveFullscreenBrowsing;
        if (iPenEnabled)
        	{
        	StartAutoFullScreenIdleTimer();
        	}
        }
    ApiProvider().BrCtlInterface().HandleCommandL(
        command + (TInt)TBrCtlDefs::ECommandIdBase );
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::HandlePluginFullScreen
// -----------------------------------------------------------------------------
//
void CBrowserContentView::HandlePluginFullScreen(TBool aFullScreen)
{
  iIsPluginFullScreenMode = aFullScreen;
  if (aFullScreen) {
  	if (AppUi()->Orientation() != CAknAppUiBase::EAppUiOrientationLandscape) {
      iOrientation = AppUi()->Orientation();
      TRAP_IGNORE(AppUi()->SetOrientationL(CAknAppUiBase::EAppUiOrientationLandscape));
    }
    if (iPenEnabled) {
      Toolbar()->SetToolbarVisibility( EFalse, EFalse );
    }
    iContentFullScreenMode = ETrue;
    UpdateFullScreenL();
  }
  else {
    TRAP_IGNORE(AppUi()->SetOrientationL(iOrientation));
    if (iPenEnabled) {
      Toolbar()->SetToolbarVisibility( ETrue, EFalse );
    }
    iContentFullScreenMode = EFalse;
    SetFullScreenOffL();
  }

}

// -----------------------------------------------------------------------------
// CBrowserContentView::ShowFsStatusPane
// -----------------------------------------------------------------------------
//
void CBrowserContentView::ShowFsStatusPane(TBool aShow)
    {

    if (aShow)
        {
#ifdef RD_SCALABLE_UI_V2
        if ( Layout_Meta_Data::IsLandscapeOrientation() )
           StatusPane()->SwitchLayoutL(R_AVKON_WIDESCREEN_PANE_LAYOUT_USUAL_FLAT_NO_SOFTKEYS);
#endif
        StatusPane()->MakeVisible( ETrue );
        iContainer->SetRect(ClientRect());
        // Remove any timer that has already been set
        if ( iPeriodic )
            {
            iPeriodic->Cancel();
            }
        }
    else
        {
        // Kick off timer for 3 seconds before hiding status pane again
        if ( iPeriodic )
            {
                iPeriodic->Cancel();
            }
        iPeriodic->Start(KFullScreenStatusPaneTimeout, 0,TCallBack(CallHideFsStatusPane,this));
        }
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::CallHideFsStatusPane
// -----------------------------------------------------------------------------
TInt CBrowserContentView::CallHideFsStatusPane(TAny* aCBrowserContentView)
    {
    __ASSERT_DEBUG(aCBrowserContentView, Util::Panic( Util::EUninitializedData ));
    TRAP_IGNORE(
                ((CBrowserContentView*)aCBrowserContentView)->HideFsStatusPane();
                );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::HideFsStatusPane
// -----------------------------------------------------------------------------
void CBrowserContentView::HideFsStatusPane()
    {
    iPeriodic->Cancel();
    if (iContentFullScreenMode && ApiProvider().LastActiveViewId() == KUidBrowserContentViewId )
        {
        StatusPane()->MakeVisible( EFalse );
        iContainer->SetRect(ClientRect());
        }
    }
// -----------------------------------------------------------------------------
// CBrowserContentView::StartAutoFullScreenTimer
// -----------------------------------------------------------------------------
//
void CBrowserContentView::StartAutoFullScreenTimer()
	{
    SuspendAutoFullScreenIdleTimer();
    if ( iAutoFSPeriodic )
        {
            iAutoFSPeriodic->Cancel();
        }
    iAutoFSPeriodic->Start(KAutoFullScreenTimeout, 0,TCallBack(CallActivateAutoFullScreen,this));

	}

// -----------------------------------------------------------------------------
// CBrowserContentView::StartAutoFullScreenIdleTimer
// -----------------------------------------------------------------------------
//
void CBrowserContentView::StartAutoFullScreenIdleTimer()
	{
    if ( iIdlePeriodic )
        {
            iIdlePeriodic->Cancel();
        }
    iIdlePeriodic->Start(KAutoFullScreenIdleTimeout, 0,TCallBack(CallActivateAutoFullScreen,this));

	}

// -----------------------------------------------------------------------------
// CBrowserContentView::SuspendAutoFullScreenTimer
// -----------------------------------------------------------------------------
//
void CBrowserContentView::SuspendAutoFullScreenTimer()
	{
    if ( iAutoFSPeriodic )
        {
            iAutoFSPeriodic->Cancel();
        }
	}

// -----------------------------------------------------------------------------
// CBrowserContentView::SuspendAutoFullScreenIdleTimer
// -----------------------------------------------------------------------------
//
void CBrowserContentView::SuspendAutoFullScreenIdleTimer()
	{
    if ( iIdlePeriodic )
        {
            iIdlePeriodic->Cancel();
        }
	}


// -----------------------------------------------------------------------------
// CBrowserContentView::CallSwitchToAutoFullScreen
// -----------------------------------------------------------------------------
//
TInt CBrowserContentView::CallActivateAutoFullScreen(TAny* aCBrowserContentView)
    {
    __ASSERT_DEBUG(aCBrowserContentView, Util::Panic( Util::EUninitializedData ));
    TRAP_IGNORE(
                ((CBrowserContentView*)aCBrowserContentView)->ActivateAutoFullScreenMode();
                );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::ActivateAutoFullScreen
// -----------------------------------------------------------------------------
void CBrowserContentView::ActivateAutoFullScreenMode()
    {
    if ( iAutoFSPeriodic )
        {
            iAutoFSPeriodic->Cancel();
        }
    if ( iIdlePeriodic )
        {
            iIdlePeriodic->Cancel();
        }

    if (  ApiProvider().LastActiveViewId() == KUidBrowserContentViewId )
    	{
    	if( IsForeground()
    		&& ApiProvider().LoadObserver().ContentDisplayed()
    		&& !iZoomSliderVisible
    		&& !iHistoryViewIsUp
    		&& !iContainer->GotoPane()->IsVisible()
			&& !iContainer->FindKeywordPane()->IsVisible()
#ifdef RD_SCALABLE_UI_V2
			&& !Toolbar()->ToolbarExtension()->IsShown()
#endif
		 	&& !iThumbnailViewIsUp
		 	&& !iOptionsMenuActive
		 	&& !ApiProvider().IsDisplayingMenuOrDialog()
			&& !ApiProvider().WindowMgr().CurrentWindow()->WMLMode()
			&& !iIsPluginFullScreenMode)
    		{
			EnableFullScreenModeL( ETrue);
			}
		else
			{
			if (iIsPluginFullScreenMode || !IsForeground())
				{
				//Hide the FullscreenExit Button
					ApiProvider().BrCtlInterface().HandleCommandL(
						TBrCtlDefs::ECommandLeaveFullscreenBrowsing
						+ (TInt)TBrCtlDefs::ECommandIdBase );
				}
			StartAutoFullScreenTimer();
			}
    	}
    }

// -----------------------------------------------------------------------------
// CBrowserContentView::SetEmphasis
// -----------------------------------------------------------------------------

void CBrowserContentView::SetEmphasis(CCoeControl* aMenuControl, TBool aEmphasis)
	{
	if (iPenEnabled)
		{
		if(aEmphasis)
			{
			iOptionsMenuActive = ETrue;
			SuspendAutoFullScreenTimer();
			}
		else
			{
			iOptionsMenuActive = EFalse;
			StartAutoFullScreenTimer();
			}
		}
	CAknView::SetEmphasis(aMenuControl, aEmphasis);
	}

// -----------------------------------------------------------------------------
// CBrowserContentView::IsEditMode
// -----------------------------------------------------------------------------
TBool CBrowserContentView::IsEditMode()
	{
	TBrCtlDefs::TBrCtlElementType focusedElementType =
                ApiProvider().BrCtlInterface().FocusedElementType();

	TBool retVal = ((focusedElementType == TBrCtlDefs:: EElementActivatedInputBox) ||
                    (focusedElementType == TBrCtlDefs:: EElementInputBox) ||
                    (focusedElementType == TBrCtlDefs:: EElementTextAreaBox));
	return  (retVal);
	}

// End of File
