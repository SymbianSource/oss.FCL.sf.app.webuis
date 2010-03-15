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
* Description:  Implementation of CBrowserBookmarksView.
*
*/

// INCLUDE FILES
#include <cdownloadmgruidownloadslist.h>
#include <cdownloadmgruidownloadmenu.h>
#include <eikmenup.h>
#include <aknsfld.h>
#include <smut.h>
#include <IrcMTM.h>

#ifndef AVERELL2_BLUETOOTH_MISSING
    #include <btmsgtypeuid.h>
#endif

#include <irmsgtypeuid.h>
#include <favouritesdb.h>

#include <BrowserNG.rsg>

#include <favouritesitem.h>
#include <favouritesitemlist.h>
#include <ApEngineConsts.h>
#include <ApListItem.h>
#include <ApListItemList.h>
#include <StringLoader.h>
#include <FeatMgr.h>
#include <Uri16.h>
#include <akntoolbar.h>
#ifdef RD_SCALABLE_UI_V2
#include <akntoolbarextension.h>
#endif


#include "BrowserDialogs.h"
#include "BrowserBookmarksView.h"
#include "BrowserBookmarksContainer.h"
#include "BrowserBookmarksModel.h"
#include "BrowserFavouritesListbox.h"
#include "BrowserBookmarksEditDialog.h"
#include "BrowserBookmarksGotoPane.h"
#include "CommonConstants.h"
#include "BrowserUtil.h"
#include "BrowserAppUi.h"
#include "Browser.hrh"
#include "CommandVisibility.h"
#include "BrowserBmOTABinSender.h"
#include "CommsModel.h"
#include <mconnection.h>
#include "BrowserUiVariant.hrh"
#include "BrowserWindowManager.h"
#include "BrowserWindow.h"
#include "BrowserDownloadUtil.h"
#include "Display.h"
#include "logger.h"

#include "eikon.hrh"


// CONSTANTS
const TInt KAdaptiveBookmarksFirstPositionInBMView = 0;
const TInt KAdaptiveBookmarksSecondPositionInBMView = 1;

const TInt KBrowserDesCArrayGranularity = 8;

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::NewLC
// ----------------------------------------------------------------------------
//
CBrowserBookmarksView* CBrowserBookmarksView::NewLC(
        MApiProvider& aApiProvider,
        CRecentUrlStore& aRecentUrlStore,
        TInt aInitialFolderUid )
    {
    CBrowserBookmarksView* view = new (ELeave) CBrowserBookmarksView( aApiProvider,
                        aRecentUrlStore,
                        aInitialFolderUid );
    CleanupStack::PushL( view );

    view->ConstructL( R_BROWSER_BOOKMARKS_VIEW );

    return view;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::~CBrowserBookmarksView
// ----------------------------------------------------------------------------
//
CBrowserBookmarksView::~CBrowserBookmarksView()
    {
LOG_ENTERFN("CBrowserBookmarksView::~CBrowserBookmarksView");
    delete iDownloadsListExecuter;
    iDownloadsListExecuter = 0;

    if ( !ApiProvider().ExitInProgress() )
        {
        // Remove observers only in destructor. See comment in ConstructL why.
        ApiProvider().Preferences().RemoveObserver( this );
        ApiProvider().Connection().UnsetApChangeObserver();
        }
    CBrowserBookmarksContainer* container = TheContainer();
    if ( container )
        {
        container->GotoPane()->SetGPObserver( NULL );
        if( ApiProvider().Preferences().SearchFeature() )
            container->SearchPane()->SetGPObserver( NULL );
        }
    delete iDomainFolderName;
    delete iItemsToMove;
    delete iCurrentOrder;
    delete iEnteredURL;
    iEnteredURL = NULL;
BROWSER_LOG( ( _L("delete iEnteredUrl 1") ) );
    iCursorPos = -1;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::UpdateGotoPaneL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::UpdateGotoPaneL()
    {
LOG_ENTERFN("BookmarksView::UpdateGotoPaneL");
    // Simulate cursor movement; that will cancel editing (if editing)
    // and refresh Goto Pane.
    if ( Container() )
        {
        if ( iEnteredURL )
            {
            BROWSER_LOG( ( _L("iEnteredUrl exists") ) );
            if ( iCursorPos >=0 )  // update pane only once!
                {
                TheContainer()->GotoPane()->SetTextL(   iEnteredURL->Des(),
                                                        EFalse,
                                                        EFalse );

                CEikEdwin* editor = STATIC_CAST( CEikEdwin*,
                        TheContainer()->GotoPane()->ComponentControl( 1 ) );
                editor->SetCursorPosL( iCursorPos, EFalse );
                iCursorPos = -1;
                TheContainer()->GotoPane()->SetFocus( ETrue );
                }
            }
        else
            {
            Container()->HandleCursorChangedL( Container()->Listbox() );
            }
        }
    }

// --------------------------------------------------------------------------------
// CBrowserBookmarksView::DeleteItems
// --------------------------------------------------------------------------------
//
void CBrowserBookmarksView::DeleteItems( TInt aCommand )
    {

    if (iHandleDeleteInProgress)
        return;

    iHandleDeleteInProgress = ETrue;

    const CFavouritesItem* item = Container()->Listbox()->CurrentItem();

    if (!item)
        {
        return;
        }

    if ( item->ParentFolder() == KFavouritesAdaptiveItemsFolderUid )
        {
        TBool confirm;
        CArrayPtr<const CFavouritesItem>* items =  Container()->Listbox()->SelectedItemsLC();

        if ( items->Count() == 1 )
            {
            confirm = TBrowserDialogs::ConfirmQueryYesNoL( R_BROWSER_FAVOURITES_TEXT_QUERY_COMMON_CONF_DELETE, (*items)[0]->Name());
            }
        else if (items->Count() > 1)
            {
            confirm = TBrowserDialogs::ConfirmQueryYesNoL( R_BROWSER_FAVOURITES_TEXT_DEL_ITEMS_QUERY, items->Count());
            }
        else
            {
            confirm = EFalse;
            }

        if (confirm)
            {
            for ( TInt i = 0; i < items->Count(); i++ )
                {
                iRecentUrlStore.DeleteData(items->At( i )->Url());
                }
            Container()->Listbox()->View()->ClearSelection();
            iSaveBMOrder = ETrue;
            RefreshL();
            }

        CleanupStack::PopAndDestroy();//items
        }
    else
        {
        iSaveBMOrder = ETrue;
        CBrowserFavouritesView::HandleCommandL( aCommand );
        }

    iHandleDeleteInProgress = EFalse;

    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::HandleCommandL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::HandleCommandL( TInt aCommand )
    {
LOG_ENTERFN("BookmarksView::HandleCommandL");
    //
    // FIRST SWITCH Generic CBA commands to original commands
    // use ProcessCommandL to route through akn to make sure
    // command gets routed to the right place
    //
    //

    // Assume generic command unless default case, below.
    TBool genericCmdHandled = ETrue;

    switch ( aCommand )
        {
        case EBrowserBookmarksCmdCancel:
            {
            if ( iManualItemMovingGoingOn )
                {
                ProcessCommandL(EWmlCmdMoveCancel);
                }
			else if( TheContainer() 
			        && ( TheContainer()->GotoPane()->IsEditing() 
			        || TheContainer()->SearchPane()->IsEditing()))
                {
                ProcessCommandL(EWmlCmdGotoPaneCancel);
                }
            break;
            }
        case EBrowserBookmarksCmdOpen:
            {
            const CFavouritesItem* item = NULL;
            if (Container())
            {
                item = Container()->Listbox()->CurrentItem();
            }

            if ( item && item->IsFolder() )
                {
                ProcessCommandL(EWmlCmdOpenFolder);
                }
            else
                {
                ProcessCommandL(EWmlCmdDownload);
                }

            break;
            }
        case EBrowserBookmarksCmdBack:
            {
            // default remap is to say back from folder
            TInt cmd = EWmlCmdBackFromFolder;

            // unless we have these conditions
            if ( !iInAdaptiveBookmarksFolder )
                {
                if ( iCurrentFolder == KFavouritesRootUid )
                    {
                    if ( ApiProvider().IsPageLoaded() || ApiProvider().Fetching() )
                        {
                        cmd = EWmlCmdBackToPage;
                        }
                    }
                }
        ProcessCommandL(cmd);
        break;
            }
        default:
            {
            genericCmdHandled = EFalse;
            break;
            }
        }

    if (genericCmdHandled)
        {
        return;
        }

    //
    // EXISTING (regular) command set
    //
    if ( TheContainer() )
    {
        switch ( aCommand )
        {
        // CBA commands.
        case EWmlCmdGotoPaneGoTo:
            {
            if( TheContainer()->GotoPaneActive() )
                {
                if (TheContainer()->GotoPane()->Editor()->TextLength()!= 0)
                    {
                    // Cancel editing and sets Goto Pane text back.
                    if (TheContainer()->GotoPane()->PopupList() != NULL)
                        {
                        TheContainer()->GotoPane()->PopupList()->SetDirectoryModeL( ETrue );
                        TheContainer()->GotoPane()->PopupList()->HidePopupL();
                        }
                    GotoUrlInGotoPaneL();
                    }
                }
            else // it was from searchpane 
                {
                if (TheContainer()->SearchPane()->Editor()->TextLength()!= 0)
                    {
                    // Launch Search application with search parameters
                    // and cancel editing of search and goto.
                    // Dim Toolbar buttons
                    DimToolbarButtons(EFalse);
                    HBufC* searchString = TheContainer()->SearchPane()->GetTextL();
                    CleanupStack::PushL( searchString );
                    if( searchString )
                        {
                        UpdateCbaL();
                        // Set GoTo/Search Inactive
                        TheContainer()->SetGotoInactiveL();
                        LaunchSearchApplicationL( *searchString );
                        }
                    CleanupStack::PopAndDestroy( searchString );
                    }
                // set LSK to GOTO now
                UpdateCbaL();
                }
            break;
            }
        case EWmlCmdGotoPaneSelect:
            {
            if (TheContainer()->GotoPane()->Editor()->TextLength()!= 0)
                {
                // Cancel editing and sets Goto Pane text back.
                if (TheContainer()->GotoPane()->PopupList() != NULL)
                    {
                    TheContainer()->GotoPane()->PopupList()->SetDirectoryModeL( ETrue );
                    TheContainer()->GotoPane()->PopupList()->HidePopupL();
                    }
                // set LSK to GOTO now
                UpdateCbaL();
                }
            break;
            }


        case EWmlCmdNewFolder:
            {
            TInt uid = CBrowserFavouritesView::AddNewFolderL( );

            AddUidToLastPlaceToCurrentListL(uid);
            iRefresh = EFalse;
            Model().Database().SetData( CurrentFolder() , *iCurrentOrder );
            break;
            }
        case EWmlCmdMove:
            {
            StartMoveItemsL();
            break;
            }
        case EWmlCmdMoveOk:
            {
            MoveItemsL();
            break;
            }
        case EWmlCmdMoveCancel:
            {
            CancelMoveItemsL();
            break;
            }

        case EWmlCmdMoveToFolder:
            {
            iSaveBMOrder = ETrue;
            CBrowserFavouritesView::HandleCommandL(aCommand);
            break;
            }

        case EWmlCmdGotoPaneCancel:
            {
            // Cancel editing and sets Goto Pane text back.
            if (TheContainer()->GotoPane()->PopupList() != NULL)
                {
                TheContainer()->GotoPane()->PopupList()->SetDirectoryModeL( ETrue );
                TheContainer()->GotoPane()->PopupList()->HidePopupL();
                }
            delete iEnteredURL;
            iEnteredURL = NULL;
            BROWSER_LOG( ( _L("delete iEnteredUrl 2") ) );
            iCursorPos = -1;

            TheContainer()->SetGotoInactiveL();

            DimToolbarButtons(EFalse);

            break;
            }

        // Menu commands.
        case EWmlCmdFavourites:
            {
            if ( iInAdaptiveBookmarksFolder )
                {
                ExitAdaptiveBookmarks();
                }
            ApiProvider().SetViewToBeActivatedIfNeededL( KUidBrowserBookmarksViewId );
            break;
            }

        case EWmlCmdBackToPage:
            {
            ApiProvider().SetViewToReturnOnClose( KUidBrowserBookmarksViewId );
            ApiProvider().SetViewToBeActivatedIfNeededL( KUidBrowserContentViewId );
            break;
            }

        case EWmlCmdDownload:
        case EWmlCmdDownloadPage:
            {
            ActivateCurrentBookmarkL();
            break;
            }

#ifdef __RSS_FEEDS
        case EWmlCmdOpenFeedsFolder:
            {
            ApiProvider().FeedsClientUtilities().ShowFolderViewL();
            break;
            }
#endif  // __RSS_FEEDS


        case EWmlCmdLaunchHomePage:
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }

        case EWmlCmdOpenItem:
            {
            // re-handle the open item command depending on whether a folder or an
            // item is selected.
            if ( Container() && Container()->Listbox() )
                {
                const CFavouritesItem* item = Container()->Listbox()->CurrentItem();
                if ( item && item->IsFolder() )
                    {
                    HandleCommandL(EWmlCmdOpenFolder);
                    }
                else
                    {
                    HandleCommandL(EWmlCmdDownload);
                    }
                }
            break;
            }

        case EWmlCmdAddBookmark:
            {
            AddNewBookmarkL();
            break;
            }

        case EWmlCmdGoToAddressAndSearch:    
        case EWmlCmdGoToAddress: // MSK for Recent Url page
        case EWmlCmdSwitchToGotoActive:
            {
            DimToolbarButtons(ETrue);
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF            
            CEikButtonGroupContainer* cba = Cba()->Current();
            CEikCba* eikCba = static_cast<CEikCba*>( cba->ButtonGroup() );
            if( eikCba )
                {
                eikCba->EnableItemSpecificSoftkey( EFalse );
                }
#endif            
            TheContainer()->SetGotoActiveL();
            break;
            }

        case EIsCmdSearchWeb:
        	{
        	DimToolbarButtons(ETrue);
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF 
            CEikButtonGroupContainer* cba = Cba()->Current();
            CEikCba* eikCba = static_cast<CEikCba*>( cba->ButtonGroup() );
            if( eikCba )
                {
                eikCba->EnableItemSpecificSoftkey( EFalse );
                }
#endif
        	TheContainer()->SetSearchActiveL();
        	break;
        	}

        case EWmlCmdEditBookmark:
            {
            EditCurrentBookmarkL();
            break;
            }

        case EWmlCmdSetAsHomePage:
            {
            if ( Container() && Container()->Listbox() )
                {
                const CFavouritesItem* item =
                                        Container()->Listbox()->CurrentItem();
                if ( item && item->IsItem() &&
                    TBrowserDialogs::ConfirmQueryYesNoL(
                                    R_QTN_BROWSERBM_QUERY_SET_AS_HOME_PAGE ) )
                    {
                    ApiProvider().Preferences().SetHomePageUrlL( item->Url() );
                    ApiProvider().Preferences().SetHomePageTypeL( EWmlSettingsHomePageAddress );
                    }
                }
            break;
            }

        case EWmlCmdPreferences:
            {
            CBrowserFavouritesView::HandleCommandL( aCommand );
            break;
            }

        case EWmlCmdCopyToBookmarks:
            {
            CArrayPtr<const CFavouritesItem>* items =
            Container()->Listbox()->SelectedItemsLC();
            if (items->Count())
                {
                // Set WAP AP.
                CFavouritesItem* item;
                for (TInt i=0;i<items->Count();i++)
                    {
                    item = CFavouritesItem::NewL();
                    *item = *( items->At( i ) );
                    CleanupStack::PushL( item );
                    item->SetParentFolder( KFavouritesRootUid );
                    Model().AddL( *item, EFalse, CBrowserFavouritesModel::EAutoRename );
                    AddUidToLastPlaceToCurrentListL( item->Uid() );
                    CleanupStack::PopAndDestroy();//item
                    }
                iRefresh = EFalse;
                Model().Database().SetData( CurrentFolder() , *iCurrentOrder );
                Container()->Listbox()->View()->ClearSelection();
                if (items->Count()==1)
                    {
                    TBrowserDialogs::InfoNoteL
                        (
                        R_BROWSER_INFO_NOTE,
                        R_BROWSER_ADAPTIVE_BOOKMARKS_ADAPTIVE_BOOKMARK_COPIED
                        );
                    }
                 else
                    {
                    TBrowserDialogs::InfoNoteL(
                        R_BROWSER_INFO_NOTE,
                        R_BROWSER_ADAPTIVE_BOOKMARKS_ADAPTIVE_BOOKMARKS_COPIED,
                        items->Count() );
                    }
                }
            CleanupStack::PopAndDestroy();//items
            break;
            }

        case EWmlCmdRename:
            {
            // NO rename on menu
                {
                CBrowserFavouritesView::HandleCommandL( aCommand );
                }
            break;
            }

        case EWmlCmdDelete:
            {
      DeleteItems( aCommand );
      break;
      } // case


        case EWmlCmdClearAdaptiveBookmarks:
            {
            if ( TBrowserDialogs::ConfirmQueryYesNoL(
                    R_BROWSER_ADAPTIVE_BOOKMARKS_DELETE_ALL_ADAPTIVE_BOOKMARKS) )
                {
                ClearAdaptiveBookmarksL();
                }
            break;
            }

        case EWmlCmdClearAdaptiveBookmarksNoPrompt:
            {
            ClearAdaptiveBookmarksL();
            break;
            }

        case EWmlCmdOpenFolder:
            {
            if ( TheContainer()->GotoPane()->PopupList() )
                {
                if ( TheContainer()->GotoPane()->PopupList()->IsPoppedUp() )
                    {
                    TheContainer()->GotoPane()->PopupList()->SetDirectoryModeL(
                                                                    EFalse );
                    break; // This case is supposed to fall through into the next case in certain
                           // conditions.
                    }
                }
            }

        case EWmlCmdOpenMarkedFolder:
            {
            SetEmptyTextL();
            if (Container()->Listbox()->CurrentItem()->Uid() ==
                                            KFavouritesAdaptiveItemsFolderUid )
                {
                iAdaptiveBookmarksFolderSelected = ETrue;
                iInAdaptiveBookmarksFolder = ETrue;
                SetEmptyTextL();
                RefreshL();
                }
            else
                {
                CBrowserFavouritesView::HandleCommandL( aCommand );
                UpdateToolbarButtonsState();
                }
            break;
            }

        case EWmlCmdBackFromFolder:
            {
            Container()->Listbox()->ClearSelection();
            if ( iDomainFolderName )
                {
                HBufC* domainFolderNameToHighlight;
                domainFolderNameToHighlight = HBufC::NewL(
                                            iDomainFolderName->Length() );
                CleanupStack::PushL( domainFolderNameToHighlight );

                domainFolderNameToHighlight->Des().Append( *iDomainFolderName );
                delete iDomainFolderName;
                iDomainFolderName = NULL;

                CFavouritesItemList* items =
                            GetItemsLC( KFavouritesAdaptiveItemsFolderUid );
                TInt indexToHighlight = 0;

                for ( TInt i = 0; i < items->Count(); i++ )
                    {
                    if ( (*items)[i]->Name() == (*domainFolderNameToHighlight) )
                        {
                        indexToHighlight = i;
                        }
                    }

                CleanupStack::PopAndDestroy();  // items
                CleanupStack::PopAndDestroy();  // domainFolderNameToHighlight

                HBufC* title;
                title=iCoeEnv->AllocReadResourceLC(
                                        R_BROWSER_ADAPTIVE_BOOKMARKS_TITLE );
                ApiProvider().Display().SetTitleL( *title );
                CleanupStack::PopAndDestroy();  // title
                RefreshL();
                Container()->Listbox()->SetCurrentItemIndexAndDraw(
                                                            indexToHighlight );
                }
            else
                {
                if ( iInAdaptiveBookmarksFolder )
                    {
                    iAdaptiveBookmarksFolderSelected = EFalse;
                    iInAdaptiveBookmarksFolder=EFalse;
                    SetEmptyTextL();
                    HBufC* title;
                    title=iCoeEnv->AllocReadResourceLC(
                                        TheContainer()->TitleResourceId() );
                    UpdateNaviPaneL();
                    ApiProvider().Display().SetTitleL( *title );
                    iPreferredHighlightUid=KFavouritesAdaptiveItemsFolderUid;
                    CleanupStack::PopAndDestroy();  // title
                    RefreshL();
                    }
                else
                    {
                    CBrowserFavouritesView::HandleCommandL( aCommand );
                    }
                }
            // Back from a folder may require change in toolbar button state
            UpdateToolbarButtonsState();
            break;
            }

        case EWmlCmdSendBookmarkViaUnifiedMessage:
            {
            // Both case can be handled alike - SendBookmarksL
            // uses SelectedItemsLC - this always what we want (if there is
            // something marked, those; if no marked, the highlighted one).
            SendBookmarksL( );
            break;
            }


#ifdef __SERIES60_HELP
        case EAknCmdHelp:
            {
            STATIC_CAST(CBrowserAppUi*, AppUi())->HandleCommandL(EAknCmdHelp);
            break;
            }
#endif //__SERIES60_HELP

        case EWmlCmdDownloads:
            {
            ApiProvider().BrCtlInterface().HandleCommandL(
                                            (TInt)TBrCtlDefs::ECommandShowDownloads +
                                            (TInt)TBrCtlDefs::ECommandIdBase );
            break;
            }

        case EWmlCmdDownloadPageNewWindow:
            {
            const CFavouritesItem* item = Container()->Listbox()->CurrentItem();
            if ( item )
                {
                CBrowserAppUi::Static()->OpenLinkInNewWindowL( *item );
                }
            break;
            }

        default:
            {
            if (aCommand == EWmlCmdDelete &&
                  (Container()->Listbox()->CurrentItem()->ParentFolder()==KFavouritesAdaptiveItemsFolderUid))
               {
               DeleteItems( aCommand );
               break;
               }
            CBrowserFavouritesView::HandleCommandL( aCommand );
            break;
            }
        }
    }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::SetEmptyTextL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::SetEmptyTextL()
    {
    HBufC* buf;
    if (iInAdaptiveBookmarksFolder)
        {
        HBufC* title;
        if (iDomainFolderName != NULL)
            {
            ApiProvider().Display().SetTitleL( *iDomainFolderName );
            }
        else
            {
            title=iCoeEnv->AllocReadResourceLC( R_BROWSER_ADAPTIVE_BOOKMARKS_TITLE );
            ApiProvider().Display().SetTitleL( *title );
            CleanupStack::PopAndDestroy();  // title
            }
        buf = iCoeEnv->AllocReadResourceLC( R_BROWSER_ADAPTIVE_BOOKMARKS_NO_ADAPTIVE_BOOKMARKS );
        }
    else
        {
        buf = iCoeEnv->AllocReadResourceLC( TheContainer()->ListboxEmptyTextResourceId() );
        }
    TheContainer()->Listbox()->View()->SetListEmptyTextL( *buf );
    CleanupStack::PopAndDestroy();  // buf
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::Id
// ----------------------------------------------------------------------------
//
TUid CBrowserBookmarksView::Id() const
    {
    return KUidBrowserBookmarksViewId;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::CommandSetResourceIdL
// ----------------------------------------------------------------------------
//
TInt CBrowserBookmarksView::CommandSetResourceIdL()
    {
    // massive re-work for 5.0, to dynamically assign CBA
    return(R_BROWSER_BUTTONS_CBA_DYNAMIC);
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::CommandSetResourceDynL
// ----------------------------------------------------------------------------
//

void CBrowserBookmarksView::CommandSetResourceDynL(TSKPair& aLsk, TSKPair& aRsk, TSKPair& aMsk)
{
    //
    // Get gotoPane pointer and appropriate Container pointer.
    //
    // These are passed in to the LSK, RSK, MSK dynamic setting functions
    // and used to help chose the appropriate softkeys
    //
    CBrowserBookmarksGotoPane* gotoPanePtr = NULL;
    CBrowserBookmarksContainer* theContainer = TheContainer();

    // if the container doesn't exist, leave gotoPanePtr at NULL
    if (theContainer)
        {
        gotoPanePtr = theContainer->GotoPane();
        }

    // set LSK, RSK, MSK respectively
    SetLSKDynL(aLsk, gotoPanePtr);
    SetRSKDynL(aRsk, gotoPanePtr);
    SetMSKDynL(aMsk, aLsk, theContainer, gotoPanePtr);

    //
    // Finally, if the contextual menu command was assigned to the MSK,
    // we need to set up the menu now
    //
    if ( aMsk.id() == EAknSoftkeyContextOptions )
    {
        ConfigContextMenu();
    }
}

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::SetLSKDynL
// ----------------------------------------------------------------------------
//

void CBrowserBookmarksView::SetLSKDynL(TSKPair& aLsk,
    CBrowserBookmarksGotoPane* aGotoPane)
{
    CBrowserBookmarksContainer* theContainer = TheContainer();
    CBrowserBookmarksGotoPane* searchPane = NULL;
    if( theContainer )
        {
        searchPane = theContainer->SearchPane();
        }
    // Default lsk is option
    aLsk.setPair(EAknSoftkeyOptions, R_BROWSER_BOOKMARKS_DYN_SK_TEXT_SOFTKEY_OPTION);

    if ( iManualItemMovingGoingOn )
        {
        aLsk.setPair(EWmlCmdMoveOk, R_BROWSER_BOOKMARKS_DYN_SK_TEXT_SOFTKEY_OK);
        }
    else if ( aGotoPane && aGotoPane->IsEditing() )
        {
        // default for goto is go
        aLsk.setPair(EWmlCmdGotoPaneGoTo, R_BROWSER_BOOKMARKS_DYN_SK_QTN_WML_SOFTK_GO);

        // special goto cases
        if ( ApiProvider().Fetching() )
            {
            // No assignment. Set as default
            aLsk.setPair(EBrowserBookmarksCmdSoftkeyEmpty, R_BROWSER_BOOKMARKS_DYN_SK_TEXT_SOFTKEY_EMPTY);
            }
        else if (aGotoPane->PopupList() &&
                aGotoPane->PopupList()->IsOpenDirToShow() )
            {
            aLsk.setPair(EBrowserBookmarksCmdOpen, R_BROWSER_BOOKMARKS_DYN_SK_TEXT_SOFTKEY_OPEN);
            }
        else if (aGotoPane->PopupList() &&
                aGotoPane->PopupList()->IsPoppedUp() &&
                !iPenEnabled)
            {
            // LSK Select is only for non-touch devices
            aLsk.setPair(EWmlCmdGotoPaneSelect, R_BROWSER_BOOKMARKS_DYN_SK_TEXT_SOFTKEY_SELECT);
            }
		}
	else if(  searchPane && searchPane->IsEditing()  )
		{
        aLsk.setPair(EWmlCmdGotoPaneGoTo, R_BROWSER_BOOKMARKS_DYN_SK_QTN_IS_SOFTK_SEARCH);
	    }
}

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::SetRSKDynL
// ----------------------------------------------------------------------------
//

void CBrowserBookmarksView::SetRSKDynL(TSKPair& aRsk,
    CBrowserBookmarksGotoPane* aGotoPane)
{
    CBrowserBookmarksContainer* theContainer = TheContainer();
    CBrowserBookmarksGotoPane* searchPane = NULL;
    if( theContainer )
        {
        searchPane = theContainer->SearchPane();
        }
    if ( iManualItemMovingGoingOn )
        {
        aRsk.setPair(EBrowserBookmarksCmdCancel, R_BROWSER_BOOKMARKS_DYN_SK_TEXT_SOFTKEY_CANCEL);
        }
	else if( (aGotoPane && aGotoPane->IsEditing()) || (searchPane && searchPane->IsEditing()) )
        {
        // default for goto is cancel
        aRsk.setPair(EBrowserBookmarksCmdCancel, R_BROWSER_BOOKMARKS_DYN_SK_TEXT_SOFTKEY_CANCEL);
        }
    else
        {
        // default case for everything else is Back
        aRsk.setPair(EBrowserBookmarksCmdBack, R_BROWSER_BOOKMARKS_DYN_SK_TEXT_SOFTKEY_BACK);

        // OR, it could be exit under these conditions
        if ( !iInAdaptiveBookmarksFolder && !ApiProvider().IsPageLoaded() && !ApiProvider().Fetching() )
            {
            if ( iCurrentFolder == KFavouritesRootUid )
                {
                aRsk.setPair(EAknSoftkeyExit, R_BROWSER_BOOKMARKS_DYN_SK_TEXT_SOFTKEY_EXIT);
                }
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::SetMSKDynL
// ----------------------------------------------------------------------------
//

void CBrowserBookmarksView::SetMSKDynL(TSKPair& aMsk, const TSKPair aLsk,
    CBrowserFavouritesContainer* aContainer,
    CBrowserBookmarksGotoPane* aGotoPane)
    {
    // setup any marked items flag for convenience
    // also verify that container is defined
    TBool anyMarkedItems = EFalse;
    if ( aContainer)
        {
        anyMarkedItems = aContainer->Listbox()->SelectionStateL().AnyMarked();
        }
    else
        {
        return;
        }

    //
    // Usually, MSK is context menu if marked items.
    // The exception is if moving is in progress (and marked items).. then its the dot (in place of OK).
    // Otherwise, the Open Command is thrown.
    //
    if ( anyMarkedItems )
        {
        if ( iManualItemMovingGoingOn )
            {
            aMsk.setPair(EWmlCmdMoveOk, R_BROWSER_BOOKMARKS_DYN_SK_TEXT_SOFTKEY_OK);
            }
        else
            {
            aMsk.setPair(EAknSoftkeyContextOptions, R_BROWSER_BOOKMARKS_DYN_SK_TEXT_SOFTKEY_OPTION);
            }
        }
    else
        {
        aMsk.setPair(EBrowserBookmarksCmdOpen, R_BROWSER_BOOKMARKS_DYN_SK_TEXT_SOFTKEY_OPEN);
        }

	CBrowserBookmarksContainer* theContainer = TheContainer();
    CBrowserBookmarksGotoPane* searchPane = NULL;
	if( theContainer )
	    {
	    searchPane = theContainer->SearchPane();
	    }
    //
    // UNDER these special conditions, the msk is set differently:
    //
    if ( aGotoPane && aGotoPane->IsEditing() )
	    {
		// follow whatever the lsk is set to
		aMsk = aLsk;
	    }
	else if(  searchPane && searchPane->IsEditing() )
        {
        // follow whatever the lsk is set to
        aMsk = aLsk;
        }
    else
        {
        if ( iInAdaptiveBookmarksFolder )
            {
            if ( aContainer->Listbox()->UnfilteredNumberOfItems() <= 0 )
                {
                aMsk.setPair( EWmlCmdGoToAddress, R_BROWSER_BOOKMARKS_DYN_SK_QTN_BROWSER_MSK_GOTO );
                }
            }
        else
            {
            if ( (reinterpret_cast <CBrowserAppUi*>(AppUi())->IsEmbeddedModeOn()) ||
                iCurrentFolder == KFavouritesRootUid )
                {
                if ( aContainer->Listbox()->UnfilteredNumberOfItems() <= 0 )
                    {
                    aMsk.setPair( EWmlCmdAddBookmark, R_BROWSER_BOOKMARKS_DYN_SK_QTN_MSK_ADD );
                    }
                }
            else
                {
                if ( aContainer->Listbox()->UnfilteredNumberOfItems() <= 0 )
                    {
                    aMsk.setPair( EWmlCmdAddBookmark, R_BROWSER_BOOKMARKS_DYN_SK_QTN_MSK_ADD );
                    }
                }
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::HandleBookmarksGotoPaneEventL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::HandleBookmarksGotoPaneEventL
        (
        CBrowserBookmarksGotoPane*
#ifdef _DEBUG
        aGotoPane  // used only for debugging purposes
#endif
        ,
        MBookmarksGotoPaneObserver::TEvent aEvent
        )
    {
#ifdef _DEBUG
    __ASSERT_DEBUG( aGotoPane == TheContainer()->GotoPane() ||
            aGotoPane == TheContainer()->SearchPane(), \
        Util::Panic( Util::EFavouritesInternal ) );
#endif

    switch ( aEvent )
        {
        case MBookmarksGotoPaneObserver::EEventEnterKeyPressed:
            {
            HandleCommandL( EWmlCmdGotoPaneGoTo );
            break;
            }

        case MBookmarksGotoPaneObserver::EEventEditingModeChanged:
            {
            UpdateCbaL();
            break;
            }

        default:
            {
            break;
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::CBrowserBookmarksView
// ----------------------------------------------------------------------------
//
CBrowserBookmarksView::CBrowserBookmarksView (  MApiProvider& aApiProvider,
                                                CRecentUrlStore& aRecentUrlStore,
                                                TInt aInitialFolderUid )
    : CBrowserFavouritesView( aApiProvider, aInitialFolderUid ),
        iFetchInit( EFalse ),

        iRecentUrlStore (aRecentUrlStore)



    {
    // Initially start with default AP as Start Page AP.
    iCurrentDefaultAp = iStartPageApUid =
                        ApiProvider().Preferences().DefaultAccessPoint();
    iPenEnabled = AknLayoutUtils::PenEnabled();
    iHandleDeleteInProgress = EFalse;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::ConstructL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::ConstructL( TInt aViewResourceId )
    {
LOG_ENTERFN("BookmarksView::ConstructL");
    CBrowserFavouritesView::ConstructL( aViewResourceId );
    // Begin observing changes in active AP and default AP. Note that we
    // set/unset observers here, not in DoActivateL/DoDeactivate: we do the
    // observing even if the view is not active. This is needed, because
    // preferences and the active AP can be changed from other views also.
    // If notification kicks in when the view is not active, display RefreshL
    // is, of course, suppressed (in base class).
    ApiProvider().Preferences().AddObserverL( this );
    ApiProvider().Connection().SetApChangeObserver( *this );
    iOpenAdaptiveBookmarksWhenActivated = EFalse;
    iAdaptiveBookmarksFolderSelected = EFalse;
    iInAdaptiveBookmarksFolder=EFalse;
    iAdaptiveBookmarksCurrentSetting =
                            ApiProvider().Preferences().AdaptiveBookmarks();
    iEnteredURL = NULL;
BROWSER_LOG( ( _L("delete iEnteredUrl 3") ) );
    iCursorPos = -1;

    // The following code gets all bookmarks and saves them back in the correct
    // order in Favourites db. It was important to add this piece of code here
    // in the constructor so that the first time when a bookmark is added
    // through content view the correct index is used to insert the bookmark.
    // TSW Error report id# MLEN-6Z8HMM

    iSaveBMOrder = ETrue;

    if ( Model().BeginL( /*aWrite=*/ETrue, /*aDbErrorNote*/ EFalse ) ==
            KErrNone )
        {
        CFavouritesItemList* items = GetItemsLC( KFavouritesRootUid );
        CleanupStack::PopAndDestroy();
        Model().CommitL();
        }
       
    //Since the webcore will be reading the bookmark information in background
    //thread, its important to refresh when the thread notifies the fresh data.
    //Call to GetItemsLC above, which inturn calls ManualBMSortL will set iRefresh to false
    //Make it true so that latest FavIcon db info is shown     
    iRefresh = ETrue;

    if (iPenEnabled)
        {
        Toolbar()->SetToolbarObserver(this);
        }
    if ( ApiProvider().Preferences().SearchFeature() )
        {
        Toolbar()->HideItem( EWmlCmdGoToAddress, ETrue, ETrue );
        }
    else
        {
        Toolbar()->HideItem( EWmlCmdGoToAddressAndSearch, ETrue, EFalse );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::HandleListBoxEventL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::HandleListBoxEventL(
                                CEikListBox* aListBox,
                                MEikListBoxObserver::TListBoxEvent aEventType )
    {
    if ( aListBox == Container()->Listbox() )
        {
        switch ( aEventType )
            {
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF
            case MEikListBoxObserver::EEventEmptyListClicked:
                return;
#endif
            case MEikListBoxObserver::EEventEnterKeyPressed:
            case MEikListBoxObserver::EEventItemDoubleClicked:
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF
            case MEikListBoxObserver::EEventItemSingleClicked:
#endif                
                {
                if ( iManualItemMovingGoingOn )
                    {
                    MoveItemsL();
                    return;
                    }
                else if ( Container()->Listbox()->CurrentItem() )
                    {
                    if ( Container()->Listbox()->CurrentItem()->IsItem())
                        {
                        ActivateCurrentBookmarkL();
                        return;
                        }
                    else
                        {
                        if (Container()->Listbox()->CurrentItem()->Uid()==KFavouritesAdaptiveItemsFolderUid)
                            {
                            iAdaptiveBookmarksFolderSelected = ETrue;
                            iInAdaptiveBookmarksFolder=ETrue;
                            SetEmptyTextL();
                            RefreshL();

                            iPreviousViewID = KUidBrowserBookmarksViewId;
                            return;
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

    if ( TheContainer()->Listbox()->CurrentItem()->IsFolder() )
        {
        SetEmptyTextL();
        }

    CBrowserFavouritesView::HandleListBoxEventL( aListBox, aEventType );
    UpdateToolbarButtonsState();
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::DynInitMenuPaneL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::DynInitMenuPaneL
                                (
                                TInt aResourceId,
                                CEikMenuPane* aMenuPane,
                                TCommandVisibility::TIndex /*aIndex */,
                                const TBrowserFavouritesSelectionState& aState
                                )
    {
    LOG_ENTERFN("CBrowserBookmarksView::DynInitMenuPaneL");

    __ASSERT_DEBUG( (aMenuPane != NULL), Util::Panic( Util::EUninitializedData ));

    const CFavouritesItem* item =  TheContainer()->Listbox()->CurrentItem();

    switch ( aResourceId )
        {
        case R_BROWSER_BOOKMARKS_MENU_PANE:
            {
            // downloads
            aMenuPane->SetItemDimmed( EWmlCmdDownloads, !ApiProvider().BrCtlInterface().BrowserSettingL( TBrCtlDefs::ESettingsNumOfDownloads ) );

            // edit
            if ( !item || (iInAdaptiveBookmarksFolder && aState.IsEmpty() ))
                {
                aMenuPane->SetItemDimmed( EWmlCmdManageBookmarks, ETrue );
                }
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF
            // add bookmark
            if ( iInAdaptiveBookmarksFolder )
                {
                aMenuPane->SetItemDimmed( EWmlCmdAddBookmark, ETrue );
                }

            // create folder
            if ( iCurrentFolder != KFavouritesRootUid || iInAdaptiveBookmarksFolder )
                {
                aMenuPane->SetItemDimmed( EWmlCmdNewFolder, ETrue );
                }
#endif    

            if ( iInAdaptiveBookmarksFolder && aState.IsEmpty() )
                {
                aMenuPane->SetItemDimmed( EWmlCmdBmActions, ETrue );
                }
#ifndef BRDO_SINGLE_CLICK_ENABLED_FF
            if (!item)
                {
                aMenuPane->SetItemDimmed( EWmlCmdMarkUnmark, ETrue );
                }
#endif				
/*
            if (BRDO_BROWSER_UPDATE_UI_FF)
               {
               aMenuPane->SetItemDimmed( EAknCmdHelp, ETrue );
               }
            else
               {
               aMenuPane->SetItemDimmed( EWmlCmdHelpMenu, ETrue );
               }
*/
            break;
            }
        case R_GOTO_SUBMENU:
            {
            // back to page
            if ( !ApiProvider().IsPageLoaded() && !ApiProvider().Fetching())
                {
                aMenuPane->SetItemDimmed( EWmlCmdBackToPage, ETrue );
                }

            // bookmarks
            aMenuPane->SetItemDimmed( EWmlCmdFavourites, ETrue );

            // web feeds
            if (ApiProvider().IsEmbeddedModeOn())
                {
                aMenuPane->SetItemDimmed( EWmlCmdOpenFeedsFolder, ETrue );
                }

            // history
            aMenuPane->SetItemDimmed( EWmlCmdHistory, ETrue );

            // home
            aMenuPane->SetItemDimmed( EWmlCmdLaunchHomePage, ETrue );
            //search 
            if ( ! ApiProvider().Preferences().SearchFeature() )
                {
                aMenuPane->SetItemDimmed( EIsCmdSearchWeb, ETrue );
                }
            break;
            }
        case R_BMACTIONS_SUBMENU:
            {
            // send
            aMenuPane->SetItemDimmed( EWmlCmdSendAddressViaUnifiedMessage, ETrue );

            if (!item)
                {
                aMenuPane->SetItemDimmed( EWmlCmdSendBookmarkViaUnifiedMessage, ETrue );
                }
            else if ( (item && (item->IsFolder() ||
                 item->Uid() == KFavouritesAdaptiveItemsFolderUid ||
                 item->ContextId())))
                {
                aMenuPane->SetItemDimmed( EWmlCmdSendBookmarkViaUnifiedMessage, ETrue );
                }

            // copy to bookmarks
            if ( !iInAdaptiveBookmarksFolder )
                {
                aMenuPane->SetItemDimmed( EWmlCmdCopyToBookmarks, ETrue );
                }

            // set as homepage
            if ( ApiProvider().Preferences().UiLocalFeatureSupported( KBrowserUiHomePageSetting ) )
                {
                TBool dimSaveAsHomePage = EFalse;

                if ( (item && (item->IsFolder() ||
                      item->Uid() == KFavouritesAdaptiveItemsFolderUid ||
                      item->ContextId())))
                    {
                    dimSaveAsHomePage = ETrue;
                    }
                else
                    {
                    dimSaveAsHomePage = ApiProvider().IsEmbeddedModeOn() || !aState.CurrentIsItem()
                     || ( aState.MarkedCount() > 1 );
                    }

                aMenuPane->SetItemDimmed( EWmlCmdSetAsHomePage, dimSaveAsHomePage );
                }
#ifndef BRDO_SINGLE_CLICK_ENABLED_FF
            // add bookmark
            if ( iInAdaptiveBookmarksFolder )
                {
                aMenuPane->SetItemDimmed( EWmlCmdAddBookmark, ETrue );
                }
#endif            
            // set preferred/unpreferred
            if ( !iContainer->Listbox()->CurrentItem() )
                {
                aMenuPane->SetItemDimmed( EWmlCmdSetPreferredBookmark, ETrue );
                aMenuPane->SetItemDimmed( EWmlCmdSetUnPreferredBookmark, ETrue );
                }
            else
                {
                if ( ( iCurrentFolder == KFavouritesRootUid )||
                    !iContainer->Listbox()->GetDefaultData().iInSeamlessFolder ||
                        ( iContainer->Listbox()->GetDefaultData().iPreferedId ==
                                    iContainer->Listbox()->CurrentItem()->Uid() )
                  )
                    {
                    aMenuPane->SetItemDimmed( EWmlCmdSetPreferredBookmark, ETrue );
                    }

                if ( ( iCurrentFolder == KFavouritesRootUid ) ||
                    !iContainer->Listbox()->GetDefaultData().iInSeamlessFolder ||
                    !( iContainer->Listbox()->GetDefaultData().iPreferedId ==
                                iContainer->Listbox()->CurrentItem()->Uid() )
                   )
                    {
                    aMenuPane->SetItemDimmed( EWmlCmdSetUnPreferredBookmark,
                                                                    ETrue );
                    }
                }

            break;
            }
        case R_BOOKMARKS_EDIT_SUBMENU:
            {
            // edit bookmark
            if (!item || aState.MarkedCount() > 0)
                {
                aMenuPane->SetItemDimmed( EWmlCmdEditBookmark, ETrue );
                }
            else if ( (item && (item->IsFolder() ||
                 item->Uid() == KFavouritesAdaptiveItemsFolderUid ||
                 item->ContextId() || aState.CurrentIsSpecial() || iInAdaptiveBookmarksFolder)))
                {
                aMenuPane->SetItemDimmed( EWmlCmdEditBookmark, ETrue );
                }

            // rename

            // Can't rename recent urls and seamless folders - can only rename basic folders
            if ( item->IsFolder() &&
                (item->Uid() != KFavouritesAdaptiveItemsFolderUid) &&
                (item->ContextId() == NULL)  )
                {
                // more item is marked -> dim rename
                if ( aState.MarkedCount() > 0 )
                    {
                    aMenuPane->SetItemDimmed( EWmlCmdRename, ETrue );
                    }
                }
            else
                {
                aMenuPane->SetItemDimmed( EWmlCmdRename, ETrue );
                }


            // delete
            if (!item)
                {
                aMenuPane->SetItemDimmed( EWmlCmdDelete, ETrue );
                }
            else if ( item->Uid() == KFavouritesAdaptiveItemsFolderUid ||
                 item->ContextId() ||
                 !aState.AnyDeletable() )
                {
                aMenuPane->SetItemDimmed( EWmlCmdDelete, ETrue );
                }

            // move
            if ( iInAdaptiveBookmarksFolder || (TheContainer()->Listbox()->UnfilteredNumberOfItems() < 2 ) )
                {
                aMenuPane->SetItemDimmed( EWmlCmdMove, ETrue );
                }

            // move to folder
            if ( !aState.AnyPlainItem() || (iCurrentFolder == KFavouritesRootUid &&
                 iContainer && iContainer->Listbox() && !iContainer->Listbox()->AnyFoldersL()) )
                {
                aMenuPane->SetItemDimmed( EWmlCmdMoveToFolder, ETrue );
                }
#ifndef BRDO_SINGLE_CLICK_ENABLED_FF
            // create folder
            if ( iCurrentFolder != KFavouritesRootUid || iInAdaptiveBookmarksFolder )
                {
                aMenuPane->SetItemDimmed( EWmlCmdNewFolder, ETrue );
                }
#endif            

            break;
            }
        case R_BROWSER_FAVOURITES_MENU_PANE_MARK_UNMARK:
            {
            // For items and folders, show "Mark" or "Unmark".
            aMenuPane->SetItemDimmed ( aState.CurrentIsMarked() ? EWmlCmdMark : EWmlCmdUnmark, ETrue );
    #ifndef SHOW_UNMARK_ALL_ALWAYS

            // We must show "Unmark all", even if nothing is marked.
            //
            // The case when there are no items at all, is not handled here:
            // in that case we don't have "Edit list" menu.
            if ( aState.NoneIsMarked() )
                {
                aMenuPane->SetItemDimmed( EWmlCmdUnmarkAll, ETrue );
                }
    #endif  // SHOW_UNMARK_ALL_ALWAYS

    #ifndef SHOW_MARK_ALL_ALWAYS
            // As for "mark all", consider items only.
            if ( (aState.iVisibleItemCount == aState.iMarkedItemCount) && (aState.iVisibleFolderCount == aState.iMarkedFolderCount) )
                {
                //Checking for both items and folders
                aMenuPane->SetItemDimmed( EWmlCmdMarkAll, ETrue );
                }
           else if( (aState.iVisibleItemCount == 0) && (aState.iVisibleFolderCount < 2))//Only one folder is present and no items
                {
                aMenuPane->SetItemDimmed( EWmlCmdMarkAll, ETrue );
                }
    #endif  // SHOW_MARK_ALL_ALWAYS
            break;
            }
        case R_BROWSER_BOOKMARKS_MENU_PANE_OK:
            {
            // Bookmark-specific handling.
            if ( aState.IsEmpty() || (TheContainer()->Listbox()->UnfilteredNumberOfItems() < 2)||iInAdaptiveBookmarksFolder)
                {
                aMenuPane->SetItemDimmed( EWmlCmdMove, ETrue );
                }
            else
                {
                aMenuPane->SetItemDimmed( EWmlCmdMove, EFalse );
                }
               
            //Disable MoveToFolder option if you are in RecentlyVisitedUrl folder
            if( iInAdaptiveBookmarksFolder )
                {
                aMenuPane->SetItemDimmed( EWmlCmdMoveToFolder, ETrue );
                // These options are not required when user press MSK
                aMenuPane->SetItemDimmed( EWmlCmdAddBookmark, ETrue );
                aMenuPane->SetItemDimmed( EWmlCmdCopyToBookmarks, ETrue );
                aMenuPane->SetItemDimmed( EWmlCmdSwitchToGotoActive, ETrue );
                aMenuPane->SetItemDimmed( EWmlCmdNewFolder, ETrue );
                }
                
            if( iContainer && iContainer->Listbox() && !iContainer->Listbox()->AnyFoldersL() )
                {
                aMenuPane->SetItemDimmed( EWmlCmdMoveToFolder, ETrue );
                }
            const CFavouritesItem* item =  TheContainer()->Listbox()->CurrentItem();
            if  ( ( item ) &&
                  ( ( item->Uid() == KFavouritesAdaptiveItemsFolderUid ) ||
                    ( item->ContextId() != NULL ) )
                )
                {
                // We can't delete adaptive bookmarks folder,
                //   or seamless folders
                aMenuPane->SetItemDimmed( EWmlCmdDelete, ETrue );
                aMenuPane->SetItemDimmed( EWmlCmdMoveToFolder, ETrue );
                }
                
            //Enable CopyToBookmarks option if you are in RecentlyVisitedUrl folder
            if( iInAdaptiveBookmarksFolder )
            	{
            	aMenuPane->SetItemDimmed( EWmlCmdCopyToBookmarks, EFalse );
            	}
            //judge the selected items include RecentlyVisitedUrl folder or not, if include set "copy to bookmarks" dim
            CArrayPtr<const CFavouritesItem>* items = Container()->Listbox()->SelectedItemsLC();
            if (items->Count())
                {
                for (TInt i=0;i<items->Count();i++)
				    {
				    if((*items)[i]->Uid() == KFavouritesAdaptiveItemsFolderUid )
				        {
			            aMenuPane->SetItemDimmed( EWmlCmdCopyToBookmarks, ETrue );
			            }
			        }
                }
            CleanupStack::PopAndDestroy();//items
            break;
            }
        default:
            {
            break;
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::RootTitleResourceId
// ----------------------------------------------------------------------------
//
TInt CBrowserBookmarksView::RootTitleResourceId() const
    {
    if ( iOpenAdaptiveBookmarksWhenActivated )
        {
        return R_BROWSER_ADAPTIVE_BOOKMARKS_TITLE;
        }
    else
        {
        return R_BROWSER_OPTION_BOOKMARKS;
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::OptionsMenuResourceId
// ----------------------------------------------------------------------------
//
TInt CBrowserBookmarksView::OptionsMenuResourceId() const
    {
    return ( R_BROWSER_BOOKMARKS_MENU_BAR );
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::OkOptionsMenuResourceId
// ----------------------------------------------------------------------------
//
TInt CBrowserBookmarksView::OkOptionsMenuResourceId() const
    {
    return R_BROWSER_BOOKMARKS_MENU_BAR_OK;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::CreateModelL
// ----------------------------------------------------------------------------
//
CBrowserFavouritesModel* CBrowserBookmarksView::CreateModelL()
    {
    return CBrowserBookmarksModel::NewL(ApiProvider());
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::CreateContainerL
// ----------------------------------------------------------------------------

CBrowserFavouritesContainer* CBrowserBookmarksView::CreateContainerL()
    {
    CBrowserBookmarksContainer* container =
        CBrowserBookmarksContainer::NewL( ClientRect(), *this );
    container->GotoPane()->SetGPObserver( this );
    if(  ApiProvider().Preferences().SearchFeature() )
        container->SearchPane()->SetGPObserver( this );
    return container;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::MoveItemsL
// ----------------------------------------------------------------------------
void CBrowserBookmarksView::MoveItemsL()
    {
    
    // Get the Uid of currently highlighted bookmark item from listbox.
    // NOTE: Listbox indexes is based on "visible" items in list box, our order
    // array has visible and invisible items, we have to use Uid to find items.
    const CFavouritesItem* toItem = Container()->Listbox()->CurrentItem();
    TInt toUid(NULL);
    if ( toItem ) {
        toUid = toItem->Uid();
        if (toUid == (*iItemsToMove)[0])
            { // Moving to same location so change nothing, cleanup, & exit
            CancelMoveItemsL();
            return;
            }
    }    
    
	// Get a copy of the ordered array, it may contain visible and hidden items.
    CArrayFixFlat<TInt>* orderArray =
                            new (ELeave) CArrayFixFlat<TInt>(KGranularityHigh);
    CleanupStack::PushL( orderArray );
    
    orderArray->AppendL( & ( iCurrentOrder->GetBookMarksOrder()[0] ),
                             iCurrentOrder->GetBookMarksOrder().Count() );
    
    if ( toUid == NULL ) {
        toUid = (*orderArray)[orderArray->Count()-1] ;
    }


	// Create a sorted "bookmarks to be moved" array
    CArrayFixFlat<TInt>* sortedItemsToMove =
                            new (ELeave) CArrayFixFlat<TInt>(KGranularityHigh);
    CleanupStack::PushL( sortedItemsToMove );


	// Walk our copy of the ordered bookmark array and
	// 1. Delete the bookmark items to be moved from the ordered array
	// 2. Sort the bookmark items to be moved to match the visible order of the list
    TInt i( 0 );	// index into order bookmark list
    TInt j( 0 );	// index into sorted iItemsToMove
    for ( i=0; i < orderArray->Count(); i++ ) {
    	TInt orderArrayUid = (*orderArray)[i];
        
        // Find bookmark to be moved, sort and remove
        for ( j=0; j < iItemsToMove->Count(); j++ ) {
	        if ( (*iItemsToMove)[j] == orderArrayUid ) {
				sortedItemsToMove->AppendL( (*iItemsToMove)[j] );
				orderArray->Delete( i );
				i--;
				break;
	        }
        }

		// Stop sorting, if we're done
		if ( iItemsToMove->Count() == sortedItemsToMove->Count() ) {
			break;
		}
    }

	// Find the highlighted bookmark item and insert the newly ordered
	// "bookmark items to be moved" in there
    for ( i=0; i < orderArray->Count(); i++ ) {
    	if ( toUid == (*orderArray)[ i ] ) {
			for ( j=0; j < sortedItemsToMove->Count(); j++ ) {
				orderArray->InsertL( i+j, (*sortedItemsToMove)[j] );
			}
			break;
    	}
    }

    iCurrentOrder->SetBookMarksOrderL( *orderArray );
    Model().Database().SetData( CurrentFolder(), *iCurrentOrder );

    iManualItemMovingGoingOn = EFalse;
    Container()->Listbox()->ClearSelection();
    iPreferredHighlightUid = (*sortedItemsToMove)[0];

	// Delete our arrays sortedItemsToMove, orderArray
    CleanupStack::PopAndDestroy( 2 );
    delete iItemsToMove;
    iItemsToMove = NULL;

    UpdateCbaL();
    // UnDim Toolbar buttons
    DimToolbarButtons(EFalse);

    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::CancelMoveItems
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::CancelMoveItemsL()
    {
    iManualItemMovingGoingOn = EFalse;
    HandleCommandL(EWmlCmdUnmarkAll);
    UpdateCbaL();
    // UnDim Toolbar buttons
    DimToolbarButtons(EFalse);
    iPreferredHighlightUid = ( *iItemsToMove )[0];
    delete iItemsToMove;
    iItemsToMove = NULL;
    RefreshL();
    }


// ----------------------------------------------------------------------------
// CBrowserBookmarksView::StartMoveItems
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::StartMoveItemsL()
    {
    iManualItemMovingGoingOn = ETrue;

    TInt moveFrom = ( *Container()->Listbox()->SelectedRealIndexesLC() )[0];
    CleanupStack::PopAndDestroy(); //SelectedRealIndexesLC
    TInt count = Container()->Listbox()->SelectedRealIndexesLC()->Count();
    CleanupStack::PopAndDestroy(); //SelectedRealIndexesLC
    if ( ( count == 1 )
        && ( Container()->Listbox()->CurrentItemRealIndex() == moveFrom ) )
        {
        HandleCommandL(EWmlCmdMark);
        }


    if (iItemsToMove)
        {
        delete iItemsToMove;
        iItemsToMove = NULL;
        }
    iItemsToMove = new (ELeave) CArrayFixFlat<TInt>( KGranularityHigh );

    CArrayPtr<const CFavouritesItem>* items =
                                    Container()->Listbox()->SelectedItemsLC();

    for ( TInt i = 0; i < items->Count(); i++ )
        {
        iItemsToMove->AppendL( (*(*items)[i]).Uid() );
        }
    CleanupStack::PopAndDestroy(items);
    UpdateCbaL();
    // Dim Toolbar buttons
    DimToolbarButtons(ETrue);
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::ManualBMSortL
// ----------------------------------------------------------------------------
//
TBool CBrowserBookmarksView::ManualBMSortL( TInt aFolder,
                                            CFavouritesItemList* aItems )
    {
     __ASSERT_DEBUG( (aItems != NULL), Util::Panic( Util::EUninitializedData ));

    delete iCurrentOrder;
    iCurrentOrder = NULL;
    iCurrentOrder = CBrowserBookmarksOrder::NewLC();
    CleanupStack::Pop();
    if ( aItems->Count() )
    {
    CArrayFixFlat<TInt>* orderArray = new (ELeave) CArrayFixFlat<TInt>(KGranularityHigh);
    CleanupStack::PushL(orderArray);

    if ( !Model().Database().GetData( aFolder ,*iCurrentOrder ) )
        {
        if ( iCurrentOrder->GetBookMarksOrder().Count() > 0 )
            {
            orderArray->AppendL( & ( iCurrentOrder->GetBookMarksOrder()[0] ),
                                    iCurrentOrder->GetBookMarksOrder().Count());

            if ( aFolder == KFavouritesRootUid )
                {
                TInt i = 0;

                while (  i < orderArray->Count() &&
                            !( (*orderArray)[i] == KFavouritesStartPageUid ))
                    {
                    i++;
                    }

                if ( i == orderArray->Count() )
                    {
                    if ( iStartPagePresent )
                        {
                        orderArray->InsertL( 0, KFavouritesStartPageUid );
                        iSaveBMOrder = ETrue;
                        }
                    }
                else
                    {
                    if ( !iStartPagePresent )
                        {
                        orderArray->Delete(i);
                        iSaveBMOrder = ETrue;
                        }
                    }
                if ( iInsertAdaptiveBookmarksFolder )
                    {
                    iInsertAdaptiveBookmarksFolder = EFalse;
                    i = 0;

                    while (  i < orderArray->Count() &&
                                !( (*orderArray)[i] ==
                                            KFavouritesAdaptiveItemsFolderUid ))
                        {
                        i++;
                        }

                    if ( i < orderArray->Count() )
                        {
                        orderArray->Delete(i);
                        }
                    if ( orderArray->Count() )
                        {
                        if ( (*orderArray)[0] == KFavouritesStartPageUid )
                            {
                            orderArray->InsertL( 1,
                                        KFavouritesAdaptiveItemsFolderUid );
                            }
                        else
                            {
                            orderArray->InsertL( 0,
                                        KFavouritesAdaptiveItemsFolderUid );
                            }
                        }
                    }
                else
                    if ( (ADAPTIVEBOOKMARKS) &&
                        ( ApiProvider().Preferences().AdaptiveBookmarks() ==
                                            EWmlSettingsAdaptiveBookmarksOn ) )
                        {
                        i = 0;

                        while (  i < orderArray->Count() &&
                                        !( (*orderArray)[i] ==
                                            KFavouritesAdaptiveItemsFolderUid ))
                            {
                            i++;
                            }

                        if ( i == orderArray->Count() != 0 )
                            {
                                if ( (*orderArray)[0] == KFavouritesStartPageUid )
                                    {
                                    orderArray->InsertL( 1,
                                        KFavouritesAdaptiveItemsFolderUid );
                                    }
                                else
                                    {
                                    orderArray->InsertL( 0,
                                        KFavouritesAdaptiveItemsFolderUid );
                                    }
                            }
                        }

                //add new items to the beginning of the list, if there is any
                for ( TInt j = 0; j < aItems->Count(); j++ )
                    {
                    i = 0;
                    while ( i < orderArray->Count() &&
                                !( (*orderArray)[i] == aItems->At(j)->Uid() ))
                        {
                        i++;
                        }

                    if ( i == orderArray->Count() &&
                        !(aItems->At(j)->IsFactoryItem() ) &&
                        !(aItems->At(j)->Uid() ==
                                    KFavouritesAdaptiveItemsFolderUid) &&
                        !(aItems->At(j)->Uid() == KFavouritesStartPageUid) )
                        {
                        Model().AddUidToLastPlaceL( aItems->At(j)->Uid(),
                                                    orderArray, iCurrentOrder );
                        }
                    }
                }
            Model().ManualSortL(aFolder, iCurrentOrder, orderArray, aItems);
            orderArray->Reset();
            }
        }

    orderArray->Reset();

    if ( aItems->Count() )
        {
        for ( TInt i = 0; i < aItems->Count(); i++ )
            {
            orderArray->AppendL( (*aItems)[i]->Uid() );
            }
        iCurrentOrder->SetBookMarksOrderL( *orderArray );
        }
    if ( iSaveBMOrder )
        {
        iSaveBMOrder = EFalse;
        Model().Database().SetData( aFolder, *iCurrentOrder );
        }
    CleanupStack::PopAndDestroy( orderArray );
    }
    return ETrue;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::GetItemsL
// ----------------------------------------------------------------------------
//
CFavouritesItemList* CBrowserBookmarksView::GetItemsLC( TInt aFolder )
    {
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
    CFavouritesItemList* items;
    if ( iInAdaptiveBookmarksFolder )
        {
        items= new (ELeave) CFavouritesItemList();
        CleanupStack::PushL(items);//1
        CDesCArrayFlat* aditems = new ( ELeave )
                                CDesCArrayFlat( KBrowserDesCArrayGranularity );
        aditems->Reset();
        CleanupStack::PushL(aditems);//2

        CDesCArrayFlat* adnames = new ( ELeave )
                                CDesCArrayFlat( KBrowserDesCArrayGranularity );
        adnames->Reset();
        CleanupStack::PushL(adnames);//3

        CFavouritesItem* adaptiveItem;

        if ( !iRecentUrlStore.GetData( *aditems, *adnames) )
            {
            for (TInt i=0;i<aditems->MdcaCount(); i++)
                {
                adaptiveItem= CFavouritesItem::NewL();
                CleanupStack::PushL(adaptiveItem);//4
                adaptiveItem->SetUrlL( aditems->MdcaPoint(i) );
                adaptiveItem->SetNameL(adnames->MdcaPoint(i).Left(KFavouritesMaxName));
                adaptiveItem->SetType(CFavouritesItem::EItem);
                adaptiveItem->SetParentFolder(KFavouritesAdaptiveItemsFolderUid);
                items->AppendL(adaptiveItem);
                CleanupStack::Pop();    // adaptiveItem
                }
            }
        CleanupStack::PopAndDestroy( 2 );// aditems, adnames
        }
    else
        {
        items = new (ELeave) CFavouritesItemList();
        CleanupStack::PushL( items );
        Model().Database().GetAll( *items, aFolder );
        TInt aBMPosition = KAdaptiveBookmarksFirstPositionInBMView; // Adaptive BM folder is first if there is no startpage
        if ( aFolder == KFavouritesRootUid )
            {
            // Create Start Page item to be displayed in root folder (if exist).
            // (Start Page does not really exist in the database.)
            CFavouritesItem* startPage = CreateStartPageBookmarkL();
            if ( startPage )
                {
                if (!iStartPagePresent)
                    {
                    iStartPagePresent = ETrue;
                    iSaveBMOrder = ETrue;
                    }
                CleanupStack::PushL( startPage );   // ownersip is here.
                items->InsertL( 0, startPage );
                CleanupStack::Pop();    // startPage: owner is the list now.
                aBMPosition=KAdaptiveBookmarksSecondPositionInBMView; // Adaptive BM folder is second if there is a startpage item
                }
            else
                {
                if (iStartPagePresent)
                    {
                    iStartPagePresent = EFalse;
                    iSaveBMOrder = ETrue;
                    }
                }
            }

        if ( aFolder == KFavouritesRootUid )
            {
            // Create Adaptive Items Folder item to be displayed in root folder (if exist).
            // (Start Page does not really exist in the database.)
            if ( ( ADAPTIVEBOOKMARKS ) &&
                    ( ApiProvider().Preferences().AdaptiveBookmarks() ==
                                        EWmlSettingsAdaptiveBookmarksOn ))
                {
                CFavouritesItem* adaptiveItemsFolder =
                            Model().Database().CreateAdaptiveItemsFolderL();
                CleanupStack::PushL(adaptiveItemsFolder);
                HBufC* folderName=StringLoader::LoadLC(
                                    R_BROWSER_ADAPTIVE_BOOKMARKS_FOLDER);
                adaptiveItemsFolder->SetNameL(*folderName);

                CleanupStack::PopAndDestroy();//folderName
                items->InsertL( aBMPosition , adaptiveItemsFolder );
                CleanupStack::Pop();    // adaptiveItemsFolder: owner is the list now.
                }
            }
        ManualBMSortL( aFolder, items );
        }
PERFLOG_STOP_WRITE("\t\t\t\t BM View GetItemsLC" );
    return items;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::RefreshL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::RefreshL( TBool aDbErrorNote )
    {
    // Same as base class method, except that refreshing Goto Pane is
    // disallowed when fetching is in progress.
    // Base class call is TRAP-ped to make sure that frozen state of the Goto
    // Pane does not stay in due to a leave; but after removing the frozen
    // state, we trow up the catched leave again (if any).
    TBool fetching = ApiProvider().Fetching();

    if ( (fetching && TheContainer() && TheContainer()->GotoPane() )
        || ( TheContainer() && TheContainer()->GotoPane() && TheContainer()->GotoPane()->IsEditing() ))
        {
        TheContainer()->GotoPane()->Freeze( ETrue );
        }

    TRAPD( err, CBrowserFavouritesView::RefreshL( aDbErrorNote ) );


    if ( TheContainer() && TheContainer()->GotoPane() )
        {
        TheContainer()->GotoPane()->Freeze( EFalse );
        }
    User::LeaveIfError( err );
    // Refresh (i.e. bookmark creation in an empty folder) may cause the need
    // for toolbar button state to change.
    UpdateToolbarButtonsState();
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::DoActivateL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::DoActivateL (
                                        const TVwsViewId& aPrevViewId,
                                        TUid aCustomMessageId,
                                        const TDesC8& aCustomMessage
                                        )
    {
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
    LOG_ENTERFN("CBrowserBookmarksView::DoActivateL");
    LOG_WRITE_FORMAT(" aCustomMessageId: %d", aCustomMessageId);

    StatusPane()->SwitchLayoutL( R_AVKON_STATUS_PANE_LAYOUT_USUAL );
    StatusPane()->MakeVisible( ETrue );
    ApiProvider().Display().ClearMessageInfo();

    iAdaptiveBookmarksFolderWasActive = EFalse;

    if ( iOpenAdaptiveBookmarksWhenActivated )
        {
        iInAdaptiveBookmarksFolder = ETrue;
        }
    CBrowserFavouritesView::DoActivateL
                            ( aPrevViewId, aCustomMessageId, aCustomMessage );
    ApiProvider().BrCtlInterface().AddLoadEventObserverL( this );

    if ( iPreferencesChanged )
        {
        iPreferencesChanged=EFalse;
        if ( iInAdaptiveBookmarksFolder )
            {
            SetEmptyTextL();
            RefreshL();
            }
        }

    if ( aCustomMessageId == KUidCustomMsgDownloadsList )
        {
        LOG_WRITE(" KUidCustomMsgDownloadsList");
        // open the downloads list asynchronously not to block DoActivateL!
        if ( iDownloadsListExecuter == 0 )
            {
            iDownloadsListExecuter = new (ELeave) CAsyncDownloadsListExecuter( ApiProvider() );
            }
        iDownloadsListExecuter->Start();
        }
    if ( iOpenAdaptiveBookmarksWhenActivated )
        {
        iInAdaptiveBookmarksFolder = ETrue;
        SetEmptyTextL();
        RefreshL();
        iOpenAdaptiveBookmarksWhenActivated = EFalse;
        }

    UpdateToolbarButtonsState();

PERFLOG_STOP_WRITE("BMView DoActivate")
//  LOG_LEAVEFN("CBrowserBookmarksView::DoActivateL");
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::DoDeactivate
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::DoDeactivate()
    {
    if ( !iSaveStateOnDeactivate )
        {
        ExitAdaptiveBookmarks();
        }
    if ( !ApiProvider().ExitInProgress() )
        {
        ApiProvider().BrCtlInterface().RemoveLoadEventObserver( this );
        }
    CBrowserFavouritesView::DoDeactivate();
    iManualItemMovingGoingOn = EFalse;
    // UnDim Toolbar buttons
    DimToolbarButtons(EFalse);
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::GotoUrlInGotoPaneL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::GotoUrlInGotoPaneL()
    {
LOG_ENTERFN("BookmarksView::GotoUrlInGotoPaneL");
    CBrowserBookmarksContainer* container = TheContainer();
    if ( container->GotoPane()->IsEditing() )
        {
        // Dim Toolbar buttons
        DimToolbarButtons(EFalse);

        delete iEnteredURL;
        iEnteredURL = NULL;
        BROWSER_LOG( ( _L("delete iEnteredUrl 4") ) );

        iEnteredURL = container->GotoPane()->GetTextL();
        if ( iEnteredURL )
            {
            iTrId = 0;
            CEikEdwin* editor = STATIC_CAST(CEikEdwin*, container->GotoPane()->ComponentControl(1));
            iCursorPos = editor->CursorPos();

            UpdateCbaL();
            // Cancel editing, but leave entered text in place.
            container->GotoPane()->CancelEditingL();

            ApiProvider().FetchL( iEnteredURL->Des(), CBrowserLoadObserver::ELoadUrlTypeOther );
            }
        }
    }
// ----------------------------------------------------------------------------
// CBrowserBookmarksView::ExitAdaptiveBookmarks()
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::ExitAdaptiveBookmarks()
    {
    if (iInAdaptiveBookmarksFolder)
        {
        iAdaptiveBookmarksFolderWasActive = ETrue;
        iInAdaptiveBookmarksFolder=EFalse;
        delete iDomainFolderName;
        iDomainFolderName=NULL;
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::DimToolbarButtons()
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::DimToolbarButtons(TBool aDimButtons)
    {
    Toolbar()->SetItemDimmed( EWmlCmdAddBookmark, aDimButtons , ETrue );
    Toolbar()->SetItemDimmed( EWmlCmdGoToAddress, aDimButtons , ETrue );
    Toolbar()->SetItemDimmed( EWmlCmdDelete, aDimButtons , ETrue );
    if (!aDimButtons)
        {
        // when undimming buttons we may not want them all back on
        UpdateToolbarButtonsState();
        }
    Toolbar()->DrawNow();
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::ActivateCurrentBookmarkL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::ActivateCurrentBookmarkL()
    {
    if ( Container() && Container()->Listbox() )
        {
        const CFavouritesItem* item = Container()->Listbox()->CurrentItem();
        if ( item && item->IsItem() )
            {
            ApiProvider().FetchBookmarkL( *item );
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::AddNewBookmarkL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::AddNewBookmarkL()
    {
    // Make an item and fill it with default values.
    CFavouritesItem* item = CFavouritesItem::NewLC();
    item->SetType( CFavouritesItem::EItem );
    item->SetParentFolder( CurrentFolder() );
    item->SetUrlL( KHttpString );
    Model().SetNameToDefaultL( *item );
    TInt err = Model().MakeUniqueNameL( *item );
    if ( !err )
        {
        // to prevent faded statuspane at the begining of dialog activation
        CEikStatusPane* sp = STATIC_CAST( CAknAppUi*,
                                  CEikonEnv::Static()->EikAppUi() )
                                    ->StatusPane();
        sp->DrawNow();

        // Dim Toolbar buttons
        DimToolbarButtons(ETrue);

        // We have the item with a default unique name. Let the user edit it.
        CBrowserBookmarkEditDialog* dialog =
            CBrowserBookmarkEditDialog::NewL
                (
                *item,
                Model(),
                //ApiProvider().CommsModel()
                ApiProvider()
                );
        dialog->ExecuteLD( R_BROWSER_BOOKMARKS_DIALOG_EDIT );

        // Un-Dim Toolbar buttons
        DimToolbarButtons(EFalse);

        // If the bookmark was added to the database, highlight it.
        // Unlike in AddNewFolderL, adding the item to the database
        // and setting highlight uid (next line of code) runs in the RunL
        // method of different active objects. So the database notification
        // may have came in or may not by now.
        // This way of doing the highlight (setting a preferred and calling
        // HighlightPreferred works in both cases.
        iPreferredHighlightUid = item->Uid();
        // If not added, iPreferredHighlightUid is now KFavouritesNullUid,
        // so the following does nothing.
        if ( iPreferredHighlightUid != KFavouritesNullUid )
            {
            // Highlight newly added item.
            HighlightPreferredL();
            }
        }
    AddUidToLastPlaceToCurrentListL( item->Uid() );
    iRefresh = EFalse;
    Model().Database().SetData( CurrentFolder() , *iCurrentOrder );
    CleanupStack::PopAndDestroy();  // item
    Container()->Listbox()->ClearSelection();
    }


// ----------------------------------------------------------------------------
// CBrowserBookmarksView::AddUidToLastPlaceToCurrentListL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::AddUidToLastPlaceToCurrentListL( TInt aUid )
    {
    if ( iCurrentOrder )
        {
        CArrayFixFlat<TInt>* orderArray = new (ELeave) CArrayFixFlat<TInt>(KGranularityHigh);
        CleanupStack::PushL( orderArray );

        if (iCurrentOrder->GetBookMarksOrder().Count())
            {
            orderArray->AppendL( &( iCurrentOrder->GetBookMarksOrder()[0] ),
                iCurrentOrder->GetBookMarksOrder().Count());
            }

        Model().AddUidToLastPlaceL(aUid, orderArray, iCurrentOrder);

        iCurrentOrder->SetBookMarksOrderL( *orderArray );
        CleanupStack::PopAndDestroy( orderArray );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarks::EditCurrentBookmarkL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::EditCurrentBookmarkL()
    {
    if ( Container() && Container()->Listbox() )
        {
        const CFavouritesItem* listboxItem =
            Container()->Listbox()->CurrentItem();
        if ( listboxItem && listboxItem->IsItem() )
            {
            // to prevent faded statuspane at the begining of dialog activation
            CEikStatusPane* sp = STATIC_CAST( CAknAppUi*,
                                  CEikonEnv::Static()->EikAppUi() )
                                    ->StatusPane();
            sp->DrawNow();

            // Dim Toolbar buttons
            DimToolbarButtons(ETrue);

            CFavouritesItem* item = CFavouritesItem::NewLC();
            *item = *listboxItem;
            CBrowserBookmarkEditDialog* dialog =
                CBrowserBookmarkEditDialog::NewL
                    (
                    *item,
                    Model(),
                    ApiProvider(),
                    listboxItem->Uid()
                    );
            dialog->ExecuteLD( R_BROWSER_BOOKMARKS_DIALOG_EDIT );
            CleanupStack::PopAndDestroy();  // item

            // unDim Toolbar buttons
            DimToolbarButtons(EFalse);

            iPreferredHighlightUid = item->Uid();

            // Force a refresh of database changes.
            iRefresh = ETrue;
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::SendBookmarksL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::SendBookmarksL( )
    {
    CArrayPtr<const CFavouritesItem>* items =
                                    Container()->Listbox()->SelectedItemsLC();

    MBmOTABinSender& sender = ApiProvider().BmOTABinSenderL();

    sender.ResetAndDestroy();

    TBool result( ETrue );
    TInt index( 0 );
    TInt count( items->Count() );

#ifndef _BOOKMARK_SENT_ASCII
    for ( index = 0; index < count; ++index )
        {
        if ( ( *items )[ index ]->Url().Length() > KMaxUrlLenghtInOTA )
            {
#pragma message ( __FILE__ ": TODO: Length of bookmark address cannot exceed 255bytes!")
            result = EFalse;
            }
        }
#endif  // _BOOKMARK_SENT_ASCII

    if ( result )
        {
        for ( index = 0; index < count; ++index )
            {
            sender.AppendL( ( *items )[ index ]->Url(),
                              ( *items )[ index ]->Name() );
            }
        }

    // to save memory
    CleanupStack::PopAndDestroy(); // items;

    if ( result )
        {
        sender.SendAddressL( );
        }

    Container()->Listbox()->SetCurrentItemIndexAndDraw( Container()->Listbox()->CurrentItemIndex());
    }


// ----------------------------------------------------------------------------
// CBrowserBookmarksView::TheContainer
// ----------------------------------------------------------------------------
//
CBrowserBookmarksContainer* CBrowserBookmarksView::TheContainer()
    {
    return STATIC_CAST( CBrowserBookmarksContainer*, Container() );
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::ApChangedL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::ApChangedL( TUint32 aNewAPId )
    {
    if ( aNewAPId != iStartPageApUid )
        {
        // New AP has been taken into use. Set Start Page to that.
        iStartPageApUid = aNewAPId;
        // Redraw view. Note that this does nothing if view is not activated.
        RefreshL( EFalse );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::HandlePreferencesChangeL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::HandlePreferencesChangeL(
                                const TPreferencesEvent /*aEvent*/,
                                TPreferencesValues& /*aValues*/,
                                TBrCtlDefs::TBrCtlSettings /*aSettingType*/ )
    {
    // adaptive bookmarks setting changed
    if ( iAdaptiveBookmarksCurrentSetting !=
                            ApiProvider().Preferences().AdaptiveBookmarks() )
        {
        iAdaptiveBookmarksCurrentSetting =
                                ApiProvider().Preferences().AdaptiveBookmarks();

        if ( iAdaptiveBookmarksCurrentSetting !=
                                            EWmlSettingsAdaptiveBookmarksOn )
            {
            if ( iPreferredHighlightUid == KFavouritesAdaptiveItemsFolderUid )
                {
                Container()->Listbox()->View()->SetCurrentItemIndex( 0 );
                }

            ExitAdaptiveBookmarks();

            if ( iAdaptiveBookmarksCurrentSetting ==
                                            EWmlSettingsAdaptiveBookmarksOff )
                {
                if ( TBrowserDialogs::ConfirmQueryYesNoL(
                    R_BROWSER_ADAPTIVE_BOOKMARKS_DELETE_ALL_ADAPTIVE_BOOKMARKS ) )
                    {
                    iRecentUrlStore.ClearData();
                    }
                }
            }
        else
            {
            iInsertAdaptiveBookmarksFolder = ETrue;
            iSaveBMOrder = ETrue;
            }
        }

    TUint32 defaultAp = ApiProvider().Preferences().DefaultAccessPoint();

    if ( defaultAp != iCurrentDefaultAp )
        {
        // Default AP changed. Set Start Page bookmark to that.
        iCurrentDefaultAp = iStartPageApUid = defaultAp;
        // Redraw view. Note that this does nothing if view is not activated.
        RefreshL( EFalse );
        }

    iPreferencesChanged=ETrue;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::CreateStartPageBookmarkL
// ----------------------------------------------------------------------------
//
CFavouritesItem* CBrowserBookmarksView::CreateStartPageBookmarkL()
    {
LOG_ENTERFN("CBrowserBookmarksView::CreateStartPageBookmarkL");
    // no HomePage if Type is Bookmarks
    // or 'Always ask' feature is selected
    TWmlSettingsHomePage pgtype = ApiProvider().Preferences().HomePageType();
    TBool accessPointSelectionMode(
        (ApiProvider().Preferences().AccessPointSelectionMode() == EAlwaysAsk) &&
        ( EWmlSettingsHomePageAccessPoint == pgtype ) );
LOG_WRITE_FORMAT(" pgType: %d", pgtype );
LOG_WRITE_FORMAT(" selection mode: %d", accessPointSelectionMode );
    if ( accessPointSelectionMode || (pgtype == EWmlSettingsHomePageBookmarks) )
        {
        return NULL;
        }

    CFavouritesItem* startPage = NULL;
    TUint defaultAp = ApiProvider().Preferences().DefaultAccessPoint();
LOG_WRITE_FORMAT(" defaultAp: %d", defaultAp );
    if ( defaultAp != KWmlNoDefaultAccessPoint ) // There is an access point defined
        {
        // Get pointer to Start Page AP (or NULL if no such);
        /*
        Modified by MOG
        --------------------- Performance  Changes ----------------------
         remove dependency from CommsModel becasue the cached list may not be pre
        CApListItem* apItem =
            ApiProvider().CommsModel().AccessPoints()->ItemForUid( defaultAp );
        ---------------------- Performance Changes ----------------------
        */
        CApAccessPointItem* apItem = iApiProvider.Preferences().AllPreferencesL().iDefaultAPDetails;
        if ( apItem )
            {
            const HBufC* defaultHP = apItem->ReadConstLongTextL( EApWapStartPage );
            if ( defaultHP->Length() )
                {
                // Have AP and the AP has Start Page address. Hurray.
                startPage = Model().Database().CreateStartPageItemL();
                CleanupStack::PushL( startPage );
                TBuf<KFavouritesMaxName> name;
                apItem->ReadTextL(EApWapAccessPointName, name);
                startPage->SetNameL( name );
                startPage->SetUrlL( *defaultHP );
                TFavouritesWapAp ap;
                ap.SetApId( defaultAp );
                startPage->SetWapAp( ap );
                CleanupStack::Pop();    // startPage: owner is the caller.
                }
            }
        }
//LOG_LEAVEFN("CBrowserBookmarksView::CreateStartPageBookmarkL");
    return startPage;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::OfferKeyEventL
// ----------------------------------------------------------------------------
//
TKeyResponse CBrowserBookmarksView::OfferKeyEventL(
                                const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    // This method is called back from the container, to process the keypresses
    // that cannot be handled by the container. Those are (currently) the
    // left/right arrow keypresses, since the container does not know anything
    // about what folder is open, which knowledge is required to process
    // arrow presses. So we do that here.
    // Any other key processing is still in the container.
    TKeyResponse result = EKeyWasNotConsumed;
    switch ( aKeyEvent.iCode )
        {

        case EKeyLeftArrow:       // West
            {
            if ( ( CurrentFolder() != KFavouritesRootUid ) || ( iInAdaptiveBookmarksFolder ) )
                {
                // We are in some folder. Move between subfolders in the leftwards direction.
                if ( !ApiProvider().IsEmbeddedModeOn() && !iManualItemMovingGoingOn )
                    {
                    OpenNextFolderL( /*aForward=*/EFalse );
                    }
                result = EKeyWasConsumed;
                }
            break;
            }

        case EKeyRightArrow:       // East
            {
            if ( ( CurrentFolder() != KFavouritesRootUid ) || ( iInAdaptiveBookmarksFolder ) )
                {
                // We are in some folder. Move between subfolders in the rightwards direction.
                if ( !ApiProvider().IsEmbeddedModeOn() && !iManualItemMovingGoingOn )
                    {
                    OpenNextFolderL( /*aForward=*/ETrue );
                    }
                result = EKeyWasConsumed;
                }
            break;
            }

        case EKeyRightUpArrow:    // Northeast
        case EStdKeyDevice11:     //   : Extra KeyEvent supports diagonal event simulator wedge
        case EKeyRightDownArrow:  // Southeast
        case EStdKeyDevice12:     //   : Extra KeyEvent supports diagonal event simulator wedge
        case EKeyLeftDownArrow:   // Southwest
        case EStdKeyDevice13:     //   : Extra KeyEvent supports diagonal event simulator wedge
        case EKeyLeftUpArrow:     // Northwest
        case EStdKeyDevice10:     //   : Extra KeyEvent supports diagonal event simulator wedge
            {                     // Here, "eat" all of the diagonals so they have no effect
            result = EKeyWasConsumed;
            break;
            }

        case EKeyBackspace:
        case EKeyDelete:
            {
            TBrowserFavouritesSelectionState state =
                iContainer->Listbox()->SelectionStateL();
            const CFavouritesItem* item = Container()->Listbox()->CurrentItem();

            if  ( ( state.AnyDeletable() ) &&
                  // We can't delete adaptive bookmarks folder or seamless folders'
                  ( item ) &&
                  ( item->Uid() != KFavouritesAdaptiveItemsFolderUid ) &&
                  ( item->ContextId() == NULL )
                )
                {
                // Clear key deletes the selection (or highlighted items).
                if (iInAdaptiveBookmarksFolder)
                    {
                    HandleCommandL(EWmlCmdDelete);
                    result = EKeyWasConsumed;
                    }
                else if (iManualItemMovingGoingOn)
                    // Manual Move in progress -- disable delete key
                    {
                    result = EKeyWasConsumed;
                    }
                else
                    {
                    result =CBrowserFavouritesView::OfferKeyEventL(aKeyEvent,aType);
                    }

                }
            break;
            }

        case EKeyOK:
            // OK key is handled through MSK handling
            result = EKeyWasConsumed;
            break;

        case EKeyEnter:
            {
            if ( iManualItemMovingGoingOn )
                {
                MoveItemsL();
                result = EKeyWasConsumed;
                }
            else
                {
                result = CBrowserFavouritesView::OfferKeyEventL(aKeyEvent,aType);
                }
            break;
            }

        default:
            {
            result =CBrowserFavouritesView::OfferKeyEventL(aKeyEvent,aType);
            break;
            }
        }

    return result;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::OpenNextFolderL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::OpenNextFolderL( TBool aForward )
    {
    if ( !Model().BeginL( EFalse, ETrue ) )
        {
        TInt nextFolder;
        if ( iInAdaptiveBookmarksFolder )
            {
            // If in domain then forget about it
            if (iDomainFolderName)
                {
                delete iDomainFolderName;
                iDomainFolderName = NULL;
                }

            nextFolder = Model().NextFolderL( KFavouritesAdaptiveItemsFolderUid,
                                                                    aForward );
            if ( nextFolder != KFavouritesNullUid )
                {
                iInAdaptiveBookmarksFolder = EFalse;
                }
            }
        else
            {
            nextFolder = Model().NextFolderL( CurrentFolder(), aForward );
            }

        if ( nextFolder != KFavouritesNullUid )
            {
            if ( nextFolder != KFavouritesAdaptiveItemsFolderUid )
                {
                SetEmptyTextL();
                OpenFolderL( nextFolder );
                }
            }
        Model().CommitL();
        if ( nextFolder == KFavouritesAdaptiveItemsFolderUid )
            {
            iInAdaptiveBookmarksFolder=ETrue;
            SetEmptyTextL();
            SetRootFolderForAdaptiveBookmarks();
            SetEmptyTextL();
            RefreshL();
            }
        }
    // Changing folder may require change in toolbar button state
    UpdateToolbarButtonsState();
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksView::UpdateNaviPaneL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksView::UpdateNaviPaneL()
    {
    if (iInAdaptiveBookmarksFolder)
        {
        // In other folder, show "1/4" style text.
        TInt folderIndex;
        TInt folderCount;
        TInt err;
        err = Model().FolderInfoL( KFavouritesAdaptiveItemsFolderUid,
                                                    folderCount, folderIndex );
        if ( !err )
            {
                TheContainer()->ShowFolderNaviPaneL( folderIndex, folderCount );
            }
        }
    else
        {
        CBrowserFavouritesView::UpdateNaviPaneL();
        }
    }

// ---------------------------------------------------------------------------
// CBrowserBookmarksView::HandleBrowserLoadEventL
// ---------------------------------------------------------------------------
//
void CBrowserBookmarksView::HandleBrowserLoadEventL(
        TBrCtlDefs::TBrCtlLoadEvent aLoadEvent,
        TUint /* aSize */,
        TUint16 aTransactionId )
    {
LOG_ENTERFN("BookmarksView::HandleBrowserLoadEventL");
    // only GotoPane is interested in LoadEvents!
    if ( Container() )
        {
        if ( aLoadEvent != TBrCtlDefs::EEventThumbnailAvailable)
        {
            // If in bookmarks view, a background page load is in
            // progress, just update the soft keys when page load is finished.
            // (ie. RSK would be "Stop" while page loading but change to "Exit"
            // when page load finished)
            if ( aLoadEvent == TBrCtlDefs::EEventLoadFinished )
                {
                UpdateCbaL();
                }
        }
        // save transaction id
        if ( aLoadEvent == TBrCtlDefs::EEventUrlLoadingStart && iTrId == 0 )
            {
            iTrId = aTransactionId;
            }
        // first content chunk arrived
        else if ( aLoadEvent == TBrCtlDefs::EEventNewUrlContentArrived &&
            iTrId == aTransactionId )
            {
            if ( iEnteredURL )
                {
                delete iEnteredURL;
                iEnteredURL = NULL;
                BROWSER_LOG( ( _L("delete iEnteredUrl 5") ) );
                }
            iCursorPos = -1;
            }
        }
    }


// ---------------------------------------------------------------------------
// CBrowserBookmarksView::ClearAdaptiveBookmarksL
// ---------------------------------------------------------------------------
//
void CBrowserBookmarksView::ClearAdaptiveBookmarksL()
    {
    iRecentUrlStore.ClearData();
    Container()->Listbox()->View()->ClearSelection();
    RefreshL();
    }

// ---------------------------------------------------------------------------
// CBrowserBookmarksView::UpdateToolbarButtonsState
// ---------------------------------------------------------------------------
//
void CBrowserBookmarksView::UpdateToolbarButtonsState()
    {
    if ( Container() && Container()->Listbox() )
        {
        const CFavouritesItem* item = Container()->Listbox()->CurrentItem();

        // Button: EWmlCmdDelete
        // Default case: un-dim the delete Button
        // Case: Not an item so dim delete button
        // Special Case: dim button for adaptive folder and
        // seamless links
        TBrowserFavouritesSelectionState state = iContainer->Listbox()->SelectionStateL();

        TBool needToDimDeleteBtn = !item
                                || ( item->Uid() == KFavouritesAdaptiveItemsFolderUid )
                                || ( item->ContextId() != NULL )
                                || (!state.AnyDeletable());

        Toolbar()->SetItemDimmed( EWmlCmdDelete, needToDimDeleteBtn, ETrue );
        //set EWmlCmdAddBookmark dim in RecentlyVisitedUrl Folder
        Toolbar()->SetItemDimmed( EWmlCmdAddBookmark, iInAdaptiveBookmarksFolder, ETrue);
        if(iManualItemMovingGoingOn)
            {
            DimToolbarButtons(ETrue);
            }      
        }
    }

void CBrowserBookmarksView::HandleForegroundEventL( TBool aForeground )
    {
    // make sure we call base class function before proceeding
    CBrowserFavouritesView::HandleForegroundEventL(aForeground);
    if ( aForeground )
        {
        if ( TheContainer()->GotoPaneActive() && TheContainer()->GotoPane()->PopupList()->IsPoppedUp() )
            {
            // The pop up messes the screen when changing layouts
            // So make sure to redraw the pane while coming to foreground 
            TheContainer()->GotoPane()->PopupList()->HandleResourceChange(KEikDynamicLayoutVariantSwitch);
            }
        }
    }
// End of File
