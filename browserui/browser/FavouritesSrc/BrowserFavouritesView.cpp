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
*      Implementation of CBrowserFavouritesView.
*
*
*/


// INCLUDE FILES
#include <aknsfld.h>
#include <eikbtgpc.h>
#include <eikmenub.h>
#include <aknviewappui.h>
#include <Avkon.hrh>
#include <Uikon.hrh>
#include <eikmenup.h>
#include <AknForm.h>
#include <AknQueryDialog.h>
#include <BrowserNG.rsg>
#include <avkon.rsg>
#include <FavouritesLimits.h>
#include <FavouritesDb.h>
#include <FavouritesItem.h>
#include <FavouritesItemList.h>
#include <InternetConnectionManager.h>
#include <FeatMgr.h>
#include <centralrepository.h>
#include <nifman.h>

#include "BrowserUiPrivateCRKeys.h"
#include "BrowserUtil.h"
#include "BrowserDialogs.h"
#include "BrowserFavouritesView.h"
#include "BrowserFavouritesFolderSelector.h"
#include "BrowserFavouritesContainer.h"
#include "BrowserFavouritesListbox.h"
#include "BrowserFavouritesListboxState.h"
#include "BrowserFavouritesModel.h"
#include "Browser.hrh"
#include "BrowserAppUi.h"
#include "BrowserApplication.h"
#include "BrowserFavouritesIncrementalDelete.h"
#include "BrowserFavouritesIncrementalMove.h"
#include "BrowserUIVariant.hrh"
#include "BrowserWindowManager.h"
#include "CommandVisibility.h"
#include "CommonConstants.h"
#include "CommsModel.h"
#include "Display.h"
#include "Preferences.h"

#include "eikon.hrh"


// MACROS

/**
* Defining this macro shows "Unmark all", even if nothing is marked (except
* if there are no items at all).
*/
//#define SHOW_UNMARK_ALL_ALWAYS
/**
* Defining this macro shows "Mark all", even if all is marked (except
* if there are no items at all).
*/
//#define SHOW_MARK_ALL_ALWAYS

/// Cosmetic purpose macro (resource names are overly too long).
#define _R(str) R_BROWSER_FAVOURITES_ ## str

// CONSTANTS
/// Granularity of Uid lists used in managing multiple bookmarks.
LOCAL_C const TInt KGranularityUids = 4;

/// Estimated FFS overhead for deleting (in bytes).
LOCAL_C const TInt KEstimatedDeleteFfsOverhead = 16 * 1024;

// ==================== LOCAL FUNCTIONS ====================

/**
* Create a new empty list for Uids and push it on the cleanup stack.
* @return The constructed empty list.
*/
LOCAL_C CArrayFix<TInt>* NewUidListLC()
    {
    // Not inline - go for size, not speed.
    CArrayFixFlat<TInt>* list;
    list = new (ELeave) CArrayFixFlat<TInt> ( KGranularityUids );
    CleanupStack::PushL( list );
    return list;
    }

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserFavouritesView::~CBrowserFavouritesView
// ---------------------------------------------------------
//
CBrowserFavouritesView::~CBrowserFavouritesView()
    {
    delete iIncrementalOp;
    delete iContainer;
    delete iModel;
    delete iSavedListboxState;
    }

// ---------------------------------------------------------------------------
// CBrowserFavouritesView::CommandSetResourceIdL
// ---------------------------------------------------------------------------
TInt CBrowserFavouritesView::CommandSetResourceIdL()
    {
    // for 5.0, BrowserBookmarksView takes over cba-assignment duties
    // this function should not be called. We now route though BrowserBookmarksView.
    return R_AVKON_SOFTKEYS_EMPTY;
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::HandleCommandL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::HandleCommandL( TInt aCommand )
    {
    switch ( aCommand )
        {
        case EWmlCmdBackFromFolder:
            {
            if ( ApiProvider().IsEmbeddedModeOn() )
                {
                AppUi()->HandleCommandL( EWmlCmdCloseBrowser );
                }
            else
                {
                CloseCurrentFolderL();
                }
            break;
            }

        case EWmlCmdOpenFolder:
            {
            OpenCurrentFolderL();
            break;
            }

        case EWmlCmdOpenMarkedFolder:
            {
            OpenMarkedFolderL();
            break;
            }

        case EWmlCmdFavourites:
            {
            ApiProvider().SetViewToBeActivatedIfNeededL( KUidBrowserBookmarksViewId );
            break;
            }

        case EWmlCmdMark:
        case EWmlCmdUnmark:
        case EWmlCmdMarkAll:
        case EWmlCmdUnmarkAll:
            {
            iContainer->Listbox()->HandleMarkableListCommandL( aCommand );
            iContainer->HandleCursorChangedL( iContainer->Listbox() );
            UpdateCbaL();
            break;
            }

        case EWmlCmdDelete:
            {
            DeleteMarkedItemsL();
            break;
            }

        case EWmlCmdNewFolder:
            {
            (void)AddNewFolderL();
            break;
            }

        case EWmlCmdMoveToFolder:
            {
            MoveMarkedItemsL();
            break;
            }


        case EWmlCmdRename:
            {
            RenameCurrentItemL();
            break;
            }

        case EWmlCmdSetPreferredBookmark:
            {
            SetPreferedCurrentItemL();
            break;
            }

        case EWmlCmdSetUnPreferredBookmark:
            {
            SetUnPreferedCurrentItemL();
            break;
            }

        case EWmlCmdPreferences:
            {
            iSaveStateOnDeactivate = ETrue;
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        default :
            {
            AppUi()->HandleCommandL( aCommand );
            break;
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserFavouritesView::CBrowserFavouritesView
// ----------------------------------------------------------------------------
//
CBrowserFavouritesView::CBrowserFavouritesView( MApiProvider& aApiProvider,
                                                TInt aInitialFolderId )
        : CBrowserViewBase( aApiProvider ),
          iPreferredHighlightUid( KFavouritesNullUid ),
          iShowOkOptions( EFalse ),
          iCurrentFolder( aInitialFolderId ),
          iIsActivated( EFalse ),
          iLastSelection( 0 ),
          iUpdatePending( EFalse ),
          iRefresh( ETrue )
    {
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::ConstructL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::ConstructL( TInt aViewResourceId )
    {
    BaseConstructL( aViewResourceId );
    iModel = CreateModelL();
    iInitialFolder = iCurrentFolder;
    // Ap model cannot be created without opening it; so that's not done here.
    // DoActivateL / DoDecativateL does that.
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::GetItemsLC
// ---------------------------------------------------------
//
CFavouritesItemList* CBrowserFavouritesView::GetItemsLC( TInt aFolder )
    {
    CFavouritesItemList* items = new (ELeave) CFavouritesItemList();
    CleanupStack::PushL( items );
    iModel->Database().GetAll( *items, aFolder );
    iModel->SortL( *items );
    return items;
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::GetSeamlessFolderResourceID
// ---------------------------------------------------------
//
TInt CBrowserFavouritesView::GetSeamlessFolderResourceID( TInt aContextID )
    {
    TInt resId = 0;

    switch( aContextID ) // Check if the item is a seamless link folder
        {
        case KFavouritesApplicationContextId:
               {
            resId = R_BROWSERBM_FOLDER_DOWNLOAD_APPLICATIONS;
            break;
            }
        case KFavouritesImageContextId:
        {
            resId = R_BROWSERBM_FOLDER_DOWNLOAD_IMAGES;
            break;
            }
        case KFavouritesAudioContextId:
            {
            resId = R_BROWSERBM_FOLDER_DOWNLOAD_TONES;
            break;
            }
        case KFavouritesVideoContextId:
            {
            resId = R_BROWSERBM_FOLDER_DOWNLOAD_VIDEOS;
            break;
            }
        case KFavouritesSkinContextId:
            {
            resId = R_BROWSERBM_FOLDER_DOWNLOAD_SKINS;
            break;
            }
        case KFavouritesMusicContextId:
            {
            resId = R_BROWSERBM_FOLDER_DOWNLOAD_MUSIC;
            break;
            }
        case KFavouritesServiceContextId:
            {
            resId = R_IS_RECOMMENDATIONS;
            break;
            }
        default:
            {
            break; // not a seamless link folder.
            }
        }

        return resId;
    }


// ---------------------------------------------------------
// CBrowserFavouritesView::GetSeamlessFolderTitleResourceID
// ---------------------------------------------------------
//
TInt CBrowserFavouritesView::GetSeamlessFolderTitleResourceID( TInt aContextID )
    {
    TInt resId = 0;

    switch( aContextID ) // Check if the item is a seamless link folder
        {
        case KFavouritesApplicationContextId:
               {
            resId = R_BROWSERBM_FOLDER_TITLE_DOWNLOAD_APPLICATIONS;
            break;
            }
        case KFavouritesImageContextId:
        {
            resId = R_BROWSERBM_FOLDER_TITLE_DOWNLOAD_IMAGES;
            break;
            }
        case KFavouritesAudioContextId:
            {
            resId = R_BROWSERBM_FOLDER_TITLE_DOWNLOAD_TONES;
            break;
            }
        case KFavouritesVideoContextId:
            {
            resId = R_BROWSERBM_FOLDER_TITLE_DOWNLOAD_VIDEOS;
            break;
            }
        case KFavouritesSkinContextId:
            {
            resId = R_BROWSERBM_FOLDER_TITLE_DOWNLOAD_SKINS;
            break;
            }
        case KFavouritesMusicContextId:
            {
            resId = R_BROWSERBM_FOLDER_TITLE_DOWNLOAD_MUSIC;
            break;
            }
        default:
            {
            break; // not a seamless link folder.
            }
        }

        return resId;
      }


// ---------------------------------------------------------
// CBrowserFavouritesView::IsSeamlessFolder
// ---------------------------------------------------------
//
TInt CBrowserFavouritesView::IsSeamlessFolder( TInt aContextID )
     {
     return GetSeamlessFolderResourceID( aContextID ) != 0 ? ETrue : EFalse;
     }

// ---------------------------------------------------------
// CBrowserFavouritesView::HighlightPreferredL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::HighlightPreferredL()
    {
    if (( iContainer->Listbox()->HighlightUidNow( iPreferredHighlightUid ) ) &&
        ( iPreferredHighlightUid != KFavouritesNullUid ))
        {
        // Have a preferred uid...
        // Successfully highlighted; clear preferred now.
        iPreferredHighlightUid = KFavouritesNullUid;
        iContainer->HandleCursorChangedL( iContainer->Listbox() );
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::RefreshL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::RefreshL( TBool aDbErrorNote /*=EFalse*/ )
    {
    // Do the refresh only if activated; otherwise ignore.
    // (This is a sanity-check like functionality, but is really used.
    // Bookmarks View observes changes in active and default AP-s, even if
    // not activated (and tries to refresh); but this "activated-check" is
    // made here, not in Bookmarks View, because it makes things more safe and
    // "future-proof".)
    if ( iIsActivated )
        {
        iUpdatePending = ETrue;
        if ( !iIncrementalOp )
            {
            // During an incremental operation we should never do the update
            // and refill the listbox because some of the operations holds
            // pointers to listbox items. Update is pending then.
            if ( iModel->BeginL( /*aWrite=*/EFalse, aDbErrorNote ) ==
                    KErrNone )
                {
                TInt folderExists;
                iModel->Database().FolderExists( iCurrentFolder, folderExists );
                if ( folderExists )
                    {
                    // Current folder is still OK. Refresh.
                    FillListboxL( iCurrentFolder, /*aKeepState=*/ETrue );
                    }
                else
                    {
                    // Cannot access current folder (maybe deleted).
                    // Go to root.
                    OpenFolderL( KFavouritesRootUid );
                    }
                iModel->CommitL();
                iUpdatePending = EFalse;
                }
            // iUpdatePending stays ETrue if unsuccessful.
            }
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::DoActivateL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::DoActivateL(
                        const TVwsViewId& /*aPrevViewId*/,
                        TUid /*aCustomMessageId*/,
                        const TDesC8& /*aCustomMessage*/ )
    {
    iPreviousViewID = ApiProvider().LastActiveViewId( );
    ApiProvider().SetLastActiveViewId( Id() );
    ApiProvider().CommsModel().AddObserverL( *this );
    iContainer = CreateContainerL();
    iModel->AddObserverL( *this );
    AppUi()->AddToViewStackL( *this, iContainer );

    ApiProvider().StartProgressAnimationL();

    if ( iModel->BeginL( /*aWrite=*/EFalse, /*aDbErrorNote=*/ETrue )
                                                                == KErrNone )
        {
        // Disable redraw to avoid flickering.
        // (OpenFolderL and the listbox's SetStateL both
        // wants to redraw the listbox. Do it once only.)
        TBool redrawDisabled =
            iContainer->Listbox()->View()->RedrawDisabled();
        iContainer->Listbox()->View()->SetDisableRedraw( ETrue );
        // Open folder first.
        OpenFolderL( iCurrentFolder );
        iModel->CommitL();

        // Now set back redraw and draw the listbox.
        if ( (iLastSelection >=0 ))
            {
            if ( iLastSelection > iContainer->Listbox()->BottomItemIndex() )
                {
                iLastSelection = 0;
                }
            iContainer->Listbox()->SetCurrentItemIndex( iLastSelection );
            UpdateGotoPaneL();
            }

        iContainer->Listbox()->View()->SetDisableRedraw( redrawDisabled );
        iContainer->Listbox()->DrawNow();
        }
    else
        {
        // No folder is opened; update CBA and Navi Pane now.
        if ( iLastSelection >=0 )
            {
            iContainer->Listbox()->SetCurrentItemIndex( iLastSelection );
            UpdateGotoPaneL();
            }
        UpdateCbaL();
        UpdateNaviPaneL();
        }

    if (iSavedListboxState)
        {
        iContainer->Listbox()->SetStateL(*iSavedListboxState);
        }
    iIsActivated = ETrue;
    }

// ----------------------------------------------------------------------------
// CBrowserFavouritesView::ResetStateInDeActivation
// ----------------------------------------------------------------------------
//
void CBrowserFavouritesView::ResetStateInDeActivation()
    {
    //Set iLastSelection to zero if there will be a folder change
    if ( iCurrentFolder != iInitialFolder )
        {
        iLastSelection = 0;
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::DoDeactivate
// ---------------------------------------------------------
//
void CBrowserFavouritesView::DoDeactivate()
    {
    iIsActivated = EFalse;
    TRAP_IGNORE(ApiProvider().StopProgressAnimationL());
    if ( iContainer )
        {
        iLastSelection = iContainer->Listbox()->CurrentItemIndex();
        }

    if ( !iSaveStateOnDeactivate )
        {
        ResetStateInDeActivation();  // entering root directory
        iSaveStateOnDeactivate = EFalse;
        }
    else
        {
        CBrowserFavouritesListboxState *temp = NULL;
        TInt err( KErrNone );
        TInt tempUid( KFavouritesNullUid );

        temp = new CBrowserFavouritesListboxState;
        if ( temp!=NULL )
            {
            delete iSavedListboxState;
            iSavedListboxState = temp;
            // folder might be empty, no CurrentItem at all
            if ( iContainer->Listbox()->CurrentItem() )
                {
                tempUid = iContainer->Listbox()->CurrentItem()->Uid();
                }
            iSavedListboxState->iHighlightUid = tempUid;
            TRAP( err, {
                iSavedListboxState->iMarkUids = iContainer->Listbox()->MarkedUidsLC();
                CleanupStack::Pop();    // Uid list; ownership is now in the state.
                } )
            }
        if ( !err )
            {
            iSavedListboxState->iTopItemUid = tempUid;
            }
        else
            {
            delete iSavedListboxState;
            }
        }

    if ( !ApiProvider().ExitInProgress() )
        {
        ApiProvider().CommsModel().RemoveObserver( *this );
        }
    iModel->RemoveObserver( *this );

    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        delete iContainer;
        iContainer = NULL;
        }
    iModel->CloseDb();
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::HandleClientRectChange
// ---------------------------------------------------------
//
void CBrowserFavouritesView::HandleClientRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::DynInitMenuPaneL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::DynInitMenuPaneL
( TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    if ( iContainer && iContainer->Listbox() )
        {
        TBrowserFavouritesSelectionState state =
            iContainer->Listbox()->SelectionStateL();
        TCommandVisibility::TIndex index =
            TCommandVisibility::Index( state, iShowOkOptions );
        DynInitMenuPaneL( aResourceId, aMenuPane, index, state );
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::HandleForegroundEventL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::HandleForegroundEventL( TBool aForeground )
    {
    // If another thread made changes to database, sometimes we could not
    // refresh when the notifcation comes in (most likely the other thread
    // closes the database and that performs an automatic compact(?)).
    // When coming foreground, check for missed updates.
    // Also, when coming to foreground Refresh if Skin has changed
    if ( aForeground && (iUpdatePending || iContainer->Listbox()->IsSkinUpdated()))
        {
        RefreshL();
        if (iContainer->Listbox()->IsSkinUpdated())
            {
            iContainer->Listbox()->SetSkinUpdated(EFalse);
            }
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::HandleListBoxEventL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::HandleListBoxEventL
( CEikListBox* aListBox, MEikListBoxObserver::TListBoxEvent aEventType )
    {
    if ( aListBox == iContainer->Listbox() )
        {
        switch ( aEventType )
            {
            case MEikListBoxObserver::EEventEnterKeyPressed:
            case MEikListBoxObserver::EEventItemDoubleClicked:
                {
                if ( iContainer->Listbox()->CurrentItem() )
                    {
                    if ( iContainer->Listbox()->CurrentItem()->IsFolder() )
                        {
                        OpenCurrentFolderL();
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
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::CloseCurrentFolderL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::CloseCurrentFolderL()
    {
    if ( iCurrentFolder != KFavouritesRootUid )
        {
        // Folders are one level deep. Closing any folder takes us to root.
        if ( iModel->BeginL( /*aWrite=*/EFalse, /*aDbErrorNote=*/ETrue )
                                                                == KErrNone )
            {
            if ( iContainer && iContainer->Listbox() )
                {
                // After closing, the highlight should be
                // on the folder just being closed.
                iPreferredHighlightUid = iCurrentFolder;
                OpenFolderL( KFavouritesRootUid );
                }
            iModel->CommitL();
            }
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::OpenCurrentFolderL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::OpenCurrentFolderL()
    {
    if ( iContainer && iContainer->Listbox() )
        {
        const CFavouritesItem* folder =
            iContainer->Listbox()->CurrentItem();
        if ( folder && folder->IsFolder() )
            {
            if ( iModel->BeginL( /*aWrite=*/EFalse, /*aDbErrorNote=*/ETrue )
                                                                == KErrNone )
                {
                OpenFolderL( folder->Uid() );
                iModel->CommitL();
                }
            }
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::OpenMarkedFolderL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::OpenMarkedFolderL()
    {
    if ( iContainer && iContainer->Listbox() )
        {
        const CFavouritesItem* folder =
            iContainer->Listbox()->MarkedItemL();
        if ( folder && folder->IsFolder() )
            {
            if ( iModel->BeginL( /*aWrite=*/EFalse, /*aDbErrorNote=*/ETrue )
                                                                == KErrNone )
                {
                OpenFolderL( folder->Uid() );
                iModel->CommitL();
                }
            }
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::DeleteMarkedItemsL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::DeleteMarkedItemsL()
    {
    CArrayPtr<const CFavouritesItem>* items =
        iContainer->Listbox()->SelectedItemsLC();
    // Permormance optimization: keep a separate list for folders.
    // (Make the "non-empty folder" check only for folders; this speeds up
    // things considerably when many items are selected.)
    CArrayFix<TInt>* uids = NewUidListLC();
    CArrayFix<TInt>* folderUids = NewUidListLC();
    TInt i;
    TInt count = items->Count();
    const CFavouritesItem* item = NULL;

    for ( i = 0; i < count; i++ )
        {
        item = items->At( i );
        uids->AppendL( item->Uid() );
        if ( item->IsFolder() )
            {
            folderUids->AppendL( item->Uid() );
            }
        }
    // Cannot delete the "items" pointer list, as other objects were pushed
    // after it. But DO NOT USE it after this point! The call to ConfirmDeleteL
    // allows other active objects to run, and the listbox may be updated.
    // If listbox is updated, all pointers in "items" point to deleted objects.

    if ( ConfirmDeleteL( *uids, *folderUids ) )
        {

        if (count ==1)
            {
            CFavouritesItemList* allItems = GetItemsLC( KFavouritesRootUid );
            item = items->At(0);
            iPreferredHighlightUid = item->Uid();
            TInt index = allItems->UidToIndex(iPreferredHighlightUid);
            index++;
            if (allItems->IndexToUid(index) != NULL)
                {
                iPreferredHighlightUid = allItems->IndexToUid(index);
                }
            else
                {
                iPreferredHighlightUid = allItems->IndexToUid(index-2);
                }
            CleanupStack::PopAndDestroy(); //allItems
            }


        CArrayFix<TInt>* notDeletedUids = NewUidListLC();

        // OOD handling. If disk space is low, we try to get some spare.

        CRepository* repository = CRepository::NewL( KCRUidBrowser );

        TBool diskLow = Util::FFSSpaceBelowCriticalLevelL
                ( /*aShowErrorNote=*/EFalse, KEstimatedDeleteFfsOverhead );
        if ( diskLow )
            {
            // Using disk space requests is not supported by CenRep.
            delete repository;
            }

        if ( !iModel->BeginL
                (
                /*aWrite=*/ETrue,
                /*aDbErrorNote=*/ETrue,
                /*aLffsCheck=*/EFalse
                )
            )
            {
            __ASSERT_DEBUG( !iIncrementalOp, Util::Panic
                ( Util::EIncrementalOperationAlreadyRunning ) );
            // Seems we have to give lower priority to the delete operation,
            // or else the wait note does not show up.
            iIncrementalOp = CBrowserFavouritesIncrementalDelete::NewL
                (
                *iModel,
                *uids,
                *notDeletedUids,
                CActive::EPriorityStandard - 1
                );
            iIncrementalOp->ExecuteL();
            delete iIncrementalOp;
            iIncrementalOp = NULL;
            iModel->CommitL();

            if ( diskLow )
                {
                // Compact database now.
                iModel->Database().Compact();
                // Make a manual refresh. When we get notification about the
                // commit, database is most likely busy with compact, and
                // the refresh is missed.
                RefreshL();
                }

            // Everything that can be deleted, is deleted by now.
            // Let the user know the outcome.
            DisplayResultOfDeleteL( *uids, *notDeletedUids );
            // Refresh the view only in embedded mode because in standalone mode Refresh is called by
            // HandleFavouritesModelChangeL causing the view to refresh twice and lose the cursor position
            if ( ApiProvider().IsEmbeddedModeOn() )
                {
                RefreshL();
                }
            }

        for ( TInt ii = 0; ii< uids->Count(); ii++ )
            {
            TInt prefUid;
            iModel->Database().PreferredUid( iCurrentFolder, prefUid );
            if ( prefUid  == uids->At( ii ) )
                {
                iModel->Database().SetPreferredUid( iCurrentFolder,NULL );
                break;
                }
            }

        CleanupStack::PopAndDestroy();  // notDeletedUids
        }
    CleanupStack::PopAndDestroy( 3 );   // folderUids, uids, items
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::AddNewFolderL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesView::AddNewFolderL()
    {
    TInt uid = KFavouritesNullUid;
    // Make an item and fill it with default values.
    CFavouritesItem* folder = CFavouritesItem::NewLC();
    folder->SetType( CFavouritesItem::EFolder );
    folder->SetParentFolder( KFavouritesRootUid );
    iModel->SetNameToDefaultL( *folder );
    TInt err = iModel->MakeUniqueNameL( *folder );
    if ( !err )
        {
        // We have the folder with a default unique name.
        // Let the user rename it.
        if ( iModel->RenameQueryL( *folder, ETrue ))
            {
            // Rename accepted.
            err = iModel->AddL( *folder, /*aDbErrorNote=*/ETrue,
                CBrowserFavouritesModel::EAskIfRename );
            if ( !err )
                {
                // Succesfully created. Set highlight to new folder (by setting
                // preferred highlight uid - we have not see the new folder
                // yet, until db notification kicks in.
                uid = iPreferredHighlightUid = folder->Uid();
                }
            }
        }
    CleanupStack::PopAndDestroy();  // folder
    return uid;
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::MoveMarkedItemsL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::MoveMarkedItemsL()
    {
    TInt targetFolder = SelectTargetFolderL( iCurrentFolder );

    // If we have the target folder, do the move.
    if ( targetFolder != KFavouritesNullUid )
        {
        CArrayFix<TInt>* uids = iContainer->Listbox()->SelectedUidsLC();
        CArrayFix<TInt>* unmovableUids = NewUidListLC();
        CArrayFix<TInt>* conflictingNameUids = NewUidListLC();
        if ( !iModel->BeginL( /*aWrite=*/ETrue, /*aDbErrorNote=*/ETrue ) )
            {
            __ASSERT_DEBUG( !iIncrementalOp, Util::Panic
                ( Util::EIncrementalOperationAlreadyRunning ) );
            // Seems we have to give lower priority to the move operation,
            // or else the wait note does not show up.
            iIncrementalOp = CBrowserFavouritesIncrementalMove::NewL
                (
                *iModel,
                *uids,
                *unmovableUids,
                *conflictingNameUids,
                targetFolder,
                CActive::EPriorityStandard - 1
                );
            iIncrementalOp->ExecuteL();
            delete iIncrementalOp;
            iIncrementalOp = NULL;
            iModel->CommitL();
            // Everything that can be moved, is moved by now.
            // If some items could not be moved, see why.
            HandleMoveResultL
                ( targetFolder, *uids, *unmovableUids, *conflictingNameUids );

            for ( TInt ii = 0; ii< uids->Count(); ii++ )
                {
                TInt prefUid;
                iModel->Database().PreferredUid( iCurrentFolder, prefUid );
                if ( prefUid  == uids->At( ii ) )
                    {
                    iModel->Database().SetPreferredUid( iCurrentFolder,NULL );
                    break;
                    }
                }

            CBrowserBookmarksOrder* bmOrder  = CBrowserBookmarksOrder::NewLC();
            CArrayFixFlat<TInt>* orderArray = new (ELeave) CArrayFixFlat<TInt>(KGranularityHigh);
            CleanupStack::PushL( orderArray );
            if ( Model().Database().GetData( CurrentFolder() ,*bmOrder ) == KErrNone)
                {
                if ( bmOrder->GetBookMarksOrder().Count() )
                    {
                    orderArray->AppendL( &( bmOrder->GetBookMarksOrder()[0] ), bmOrder->GetBookMarksOrder().Count());
                    TInt swapTo = 0;
                    TInt swappedItem;
                    for ( TInt i=0; i<orderArray->Count();i++ )
                        {
                        TInt swapFrom=0;
                        while (swapFrom<uids->Count() && ( (*uids)[swapFrom] != ( *orderArray )[i]) )
                            {
                            swapFrom++;
                            }
                        if (swapFrom<uids->Count())
                            {
                            if ((swapFrom != swapTo) && (swapTo < uids->Count() ) )
                                {
                                swappedItem = ( *uids )[swapFrom];
                                ( *uids )[swapFrom] = ( *uids )[swapTo];
                                ( *uids )[swapTo] = swappedItem;
                                }
                            swapTo++;
                            }
                        }
                    }
                }

            orderArray->Reset();
            if ( Model().Database().GetData( targetFolder ,*bmOrder ) == KErrNone)
                {
                if ( bmOrder->GetBookMarksOrder().Count() )
                    {
                    orderArray->AppendL( &( bmOrder->GetBookMarksOrder()[0] ), bmOrder->GetBookMarksOrder().Count());
                    if ( orderArray->Count() )
                        {
                        for ( TInt i = 0; i < uids->Count(); i++ )
                            {
                            for (TInt j = 0; j < orderArray->Count(); j++ )
                                {
                                if ( orderArray->At( j ) == uids->At( i ))
                                    {
                                    orderArray->Delete( j );
                                    }
                                }
                             Model().AddUidToLastPlaceL(uids->At( i ), orderArray, bmOrder);
                             }
                        bmOrder->SetBookMarksOrderL( *orderArray );
                        Model().Database().SetData( targetFolder , *bmOrder );
                        }
                    }
                }

            CleanupStack::PopAndDestroy(  ); //orderArray
            CleanupStack::PopAndDestroy(  ); //bmOrder
            }
        CleanupStack::PopAndDestroy( 3 );   // conflictingNameUids,
                                            // unmovableUids, uids
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::RenameCurrentItemL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::RenameCurrentItemL()
    {
    if ( iContainer && iContainer->Listbox() )
        {
        const CFavouritesItem* listboxItem =
            iContainer->Listbox()->CurrentItem();
        if ( listboxItem )
            {
            CFavouritesItem* item = CFavouritesItem::NewLC();
            *item = *listboxItem;
            if ( iModel->RenameQueryL( *item, EFalse ) )
                {
                iModel->UpdateL
                    (
                    *item,
                    listboxItem->Uid(),
                    /*aDbErrorNote=*/ETrue,
                    CBrowserFavouritesModel::EAskIfRename
                    );

                iPreferredHighlightUid = item->Uid();

                // Show infonote "Unable to rename" if read-only item
                if ( listboxItem->IsReadOnly() )
                    {
                    TBrowserDialogs::InfoNoteL
                                                (
                                                R_BROWSER_INFO_NOTE,
                                                _R(TEXT_PRMPT_CANT_RENAME),
                                                listboxItem->Name()
                                                );
                    }
                }
            CleanupStack::PopAndDestroy();  // item
            }
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::SetRootFolderForAdaptiveBookmarks
// ---------------------------------------------------------
//
void CBrowserFavouritesView::SetRootFolderForAdaptiveBookmarks()
    {
    iCurrentFolder = KFavouritesRootUid;
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::OpenFolderL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::OpenFolderL( TInt aFolder )
    {

    CFavouritesItem* folder = CFavouritesItem::NewLC();
    if ( iModel->Database().Get( aFolder, *folder ) == KErrNone )
        {
        iContainer->Listbox()->GetDefaultData().iPreferedId = NULL;
        iContainer->Listbox()->GetDefaultData().iInSeamlessFolder = EFalse;
        // Set title.
        if ( aFolder == KFavouritesRootUid )
            {
            UpdateNaviPaneL(); // remove NaviPane before setting title - SetTitleL redraws
            ApiProvider().Display().SetTitleL( RootTitleResourceId() );
            }
        else
            {
            // Next get localized title text if the folder is a seamless link folder.
            TInt resId = CBrowserFavouritesView::GetSeamlessFolderTitleResourceID(
                    folder->ContextId() );
            HBufC* name;

            if (resId)
                {
                iContainer->Listbox()->GetDefaultData().iInSeamlessFolder = ETrue;
                TInt prefUid;
                iModel->Database().PreferredUid( aFolder, prefUid );
                iContainer->Listbox()->GetDefaultData().iPreferedId = prefUid;
                name = iCoeEnv->AllocReadResourceLC( resId );
                folder->SetNameL(name->Des()); // set the localized name
                CleanupStack::PopAndDestroy(); // name
                }

            ApiProvider().Display().SetTitleL( folder->Name() );
            }

        iCurrentFolder = aFolder;
        FillListboxL( aFolder, /*aKeepState=*/EFalse );
        Container()->Listbox()->ClearSelection();
        UpdateCbaL();
        UpdateNaviPaneL();
        }
    CleanupStack::PopAndDestroy();  // folder
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::OpenNextFolderL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::OpenNextFolderL( TBool aForward )
    {
    __ASSERT_DEBUG( iCurrentFolder != KFavouritesRootUid,
        Util::Panic( Util::ERootLevel ) );

    if ( iModel->BeginL( /*aWrite=*/EFalse, /*aDbErrorNote=*/ETrue )
                                                                == KErrNone )
        {
        TInt nextFolder = iModel->NextFolderL( iCurrentFolder, aForward );
        if ( nextFolder != KFavouritesNullUid )
            {
            OpenFolderL( nextFolder );
            }
        iModel->CommitL();
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::FillListboxL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::FillListboxL( TInt aFolder, TBool aKeepState )
    {
    // During an incremental operation we should never update the listbox
    // because some of them holds pointers to listbox items.
    //
    // If we have an incremental operation, we should never get here:
    // 1) Any change notification (faves db, ap db, preferences) handling is
    //    skipped in RefreshL, where all change handling comes
    //    together.
    // 2) All incremental operations have an empty CBA, so we cannot get here
    //    by user input either.
    // But to make this safe from potential errors (e.g. user activity due to
    // some error in wait notes, CBA, key handling etc., and also from bugs in
    // this code), we double-check here.
    // In release build do nothing (avoid the crash), in debug build panic.
    if ( iIncrementalOp )
        {
#ifdef _DEBUG
        Util::Panic( Util::ECannotUpdateDataInUse );
#else
        return;
#endif
        }

    CBrowserFavouritesListbox* listbox = iContainer->Listbox();

    // Disable redraw to avoid flickering.
    TBool redrawDisabled = listbox->View()->RedrawDisabled();
    listbox->View()->SetDisableRedraw( ETrue );

    // Change the data.
    CFavouritesItemList* items = GetItemsLC( aFolder );

    // Next take localized item names for seamless links.
    TInt contextId;
    TInt resId = 0;
    HBufC* name;
 
    TBool browserService = ApiProvider().Preferences().ServiceFeature();
    TBool firstBoot = ApiProvider().Preferences().GetIntValue( KBrowserFirstBoot );
    TInt serviceUid = KErrNotFound;
    
    for(int i=0; i<items->Count(); i++)
        {
	    if (!items->At(i)->IsHidden())
            {
            contextId = items->At(i)->ContextId();
        
            if ( firstBoot && browserService )
                {
                //This is the first boot and we need to move Service top of the bookmarks.
                //Here we save uid for Service item.
                if ( contextId == KFavouritesServiceContextId )
                    {
                    serviceUid = items->IndexToUid( i );
                    }
                }
        
            resId = CBrowserFavouritesView::GetSeamlessFolderResourceID( 
        			contextId );
            if(resId)
                {
                name = iCoeEnv->AllocReadResourceLC( resId );
                items->At(i)->SetNameL(name->Des());
                CleanupStack::PopAndDestroy(); // name
                }
            }
        else
            {
            items->Delete(i);
            i--;
            }
        }
    
    if ( firstBoot && browserService && serviceUid != KErrNotFound )
        {
        //get current order array
        CBrowserBookmarksOrder* currentOrder = CBrowserBookmarksOrder::NewLC();
        CArrayFixFlat<TInt>* orderArray = new (ELeave) CArrayFixFlat<TInt>( KGranularityHigh );
        CleanupStack::PushL( orderArray );

        iModel->Database().GetData( KFavouritesRootUid, *currentOrder );
        orderArray->AppendL( &( currentOrder->GetBookMarksOrder()[0] ), currentOrder->GetBookMarksOrder().Count());
        
        //First sort UI. Move Service
        CFavouritesItem* serviceCopy = CFavouritesItem::NewLC();
        const CFavouritesItem* serviceItem = items->ItemByUid( serviceUid );
        if ( serviceItem )
            {
            *serviceCopy = *serviceItem;
            items->Delete( serviceUid );
            items->InsertL( 0, serviceCopy ); //ownership transfered
            CleanupStack::Pop( serviceCopy );
            }
        else
            {
            User::Leave( KErrNotFound );
            }

        //Next change order in db. Move Service
        orderArray->InsertL( 0, orderArray->At( serviceUid ) );
        orderArray->Delete( serviceUid+1 );
        
        //save changes to db
        currentOrder->SetBookMarksOrderL( *orderArray );
        iModel->Database().SetData( KFavouritesRootUid, *currentOrder );
        
        CleanupStack::PopAndDestroy( 2 ); //orderArray, currentOrder
        //service moved to top of the bookmark list. We can set firstboot value to false
        // so now we can set firstboot value to false.
        CRepository* repository = CRepository::NewLC( KCRUidBrowser );
        User::LeaveIfError( repository->Set(KBrowserFirstBoot, EFalse) );
        CleanupStack::PopAndDestroy( repository );
        }

    CleanupStack::Pop();    // items: passing ownership to listbox.
    iContainer->Listbox()->SetDataL
        ( items, /*ApiProvider().CommsModel(),*/ aKeepState );
    CAknColumnListBoxView *aknview = STATIC_CAST(CAknColumnListBoxView*, iContainer->Listbox()->View() );

    if ( !items->Count() )
        {
        iContainer->Listbox()->UpdateFilterL();  // ...and tell to listbox.
        aknview->SetFindEmptyListState(EFalse);
        }
    else
        {
        aknview->SetFindEmptyListState(ETrue);
        }

    iContainer->SizeChanged();  // Needed to show/hide Find Pane!
    iCurrentFolder = aFolder;

    HighlightPreferredL();
    iContainer->HandleCursorChangedL( listbox );
    listbox->View()->SetDisableRedraw( redrawDisabled );
    listbox->DrawNow();

    UpdateCbaL();
    UpdateNaviPaneL();
    }

// ----------------------------------------------------------------------------
// CBrowserFavouritesView::ConfirmDeleteL
// ----------------------------------------------------------------------------
//
TBool CBrowserFavouritesView::ConfirmDeleteL
( CArrayFix<TInt>& aUids, CArrayFix<TInt>& aFolderUids )
    {
    CBrowserFavouritesModel::TDbQuery anyNonEmptyFolders;
    TInt ret = EFalse;

    anyNonEmptyFolders = iModel->AnyNonEmptyFoldersL( aFolderUids );
    switch ( anyNonEmptyFolders )
        {
        case CBrowserFavouritesModel::EYes:
            {
            // At least one non-empty folder is selected.
            ret = TBrowserDialogs::ConfirmQueryYesNoL(
                                            _R(TEXT_DEL_FULL_FLDRS_QUERY) );
            break;
            }

        case CBrowserFavouritesModel::ENo:
            {
            // No non-empty folders are selected.
            TInt count = aUids.Count();
            switch ( count )
                {
                case 0:
                    {
                    // Empty selection (???);
                    ret = EFalse;
                    break;
                    }

                case 1:
                    {
                    // One item is selected (item or empty folder).
                    // Get its name.
                    const CFavouritesItem* item = iContainer->Listbox()->
                        ItemByUid( aUids.At( 0 ) );
                    if ( item )
                        {
                        ret = TBrowserDialogs::ConfirmQueryYesNoL(
                                    _R(TEXT_QUERY_COMMON_CONF_DELETE), item->Name() );
                        }
                    else
                        {
                        // Could not get item (maybe listbox has been updated).
                        ret = EFalse;
                        }
                    break;
                    }

                default:
                    {
                    // More items are selected (items or/and empty folders).
                    ret = TBrowserDialogs::ConfirmQueryYesNoL(
                                                _R(TEXT_DEL_ITEMS_QUERY), count );
                    break;
                    }

                }
            break;
            }

        case CBrowserFavouritesModel::EError:
        default:
            {
            // Could not access database.
            ret = EFalse;
            break;
            }
        }

    return ret;
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::DisplayResultOfDeleteL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::DisplayResultOfDeleteL
( const CArrayFix<TInt>& /*aUids*/, const CArrayFix<TInt>& aNotDeletedUids )
    {
    switch ( aNotDeletedUids.Count() )
        {
        case 0:
            {
            // All items deleted successfully.
            // Display nothing.
            break;
            }
        case 1:
            {
            // One item is not deleted.
            const CFavouritesItem* item = iContainer->Listbox()->
                ItemByUid( aNotDeletedUids.At( 0 ) );
            if ( item )
                {
                TBrowserDialogs::InfoNoteL
                    (
                    R_BROWSER_INFO_NOTE,
                    _R(TEXT_CANT_DELETE_ITEM),
                    item->Name()
                    );
                }
            // else
            //   this item is not found in the listbox (so it does not exist,
            //   most likely got deleted somehow) - keep quiet.
            break;
            }
        default:
            {
            // More items are not deleted.
            TBrowserDialogs::InfoNoteL
                                    (
                                    R_BROWSER_INFO_NOTE,
                                    _R(TEXT_CANT_DELETE_ITEMS),
                                    aNotDeletedUids.Count()
                                    );
            break;
            }
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::SelectTargetFolderL
// ---------------------------------------------------------
//
TInt CBrowserFavouritesView::SelectTargetFolderL( TInt aExcludeFolder )
    {
    TInt targetFolder = KFavouritesNullUid;

    switch ( iModel->AnyFoldersL() )
        {
        case CBrowserFavouritesModel::EYes:
            {
            // Select from existing folders.
            CBrowserFavouritesFolderSelector* selector =
                CBrowserFavouritesFolderSelector::NewL
                    ( *iModel, ApiProvider(), aExcludeFolder );
            targetFolder = selector->ExecuteLD();
            break;
            }
        case CBrowserFavouritesModel::ENo:
            {
            break;
            }
        case CBrowserFavouritesModel::EError:
        default:
            {
            // Some error; quit.
            break;
            }
        }
    return targetFolder;
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::HandleMoveResultL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::HandleMoveResultL
        (
        TInt aFolder,
        const CArrayFix<TInt>& /*aUids*/,
        const CArrayFix<TInt>& aUnmovableUids,
        const CArrayFix<TInt>& aConflictingNameUids
        )
    {
    // See if some items has to be renamed to do the move.
    if ( aConflictingNameUids.Count() )
        {
        if ( aConflictingNameUids.Count() == 1 )
            {
            // One item needs to be renamed if the user wants to move them.
            // Let the user know and ask if replace.
            const CFavouritesItem* item = iContainer->Listbox()->
                ItemByUid( aConflictingNameUids.At( 0 ) );
            if ( item )
                {
                TBrowserDialogs::InfoNoteL
                                    (
                                    R_BROWSER_INFO_NOTE,
                                    _R(TEXT_FLDR_NAME_ALREADY_USED),
                                    item->Name()
                                    );
                RenameAndMoveItemsL( aConflictingNameUids, aFolder );
                }
            }
        else
            {
            // Some items needs to be renamed if the user wants to move them.
            // Let the user know and ask for each if replace.
            TBrowserDialogs::InfoNoteL(  R_BROWSER_INFO_NOTE,
                                        _R(TEXT_FLDR_NAMES_IN_USE_RENAME) );
            RenameAndMoveItemsL( aConflictingNameUids, aFolder );
            }
        }

    // Check unmovable items.
    if ( aUnmovableUids.Count() )
        {
        if ( aUnmovableUids.Count() == 1 )
            {
            // One item is not moved.
            const CFavouritesItem* item = iContainer->Listbox()->
                ItemByUid( aUnmovableUids.At( 0 ) );
            if ( item )
                {
                TBrowserDialogs::InfoNoteL
                    (
                    R_BROWSER_INFO_NOTE,
                    _R(TEXT_ITEM_CANNOT_BE_MOVED),
                    item->Name()
                    );
                }
            }
        else
            {
            // More items could not be moved.
            TBrowserDialogs::InfoNoteL
                (
                R_BROWSER_INFO_NOTE,
                _R(TEXT_SOME_ITEMS_CANT_MOVE),
                aUnmovableUids.Count()
                );
            }
        }
    // else
    //   All is well: all moved succesfully. Display nothing.
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::RenameAndMoveItemsL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::RenameAndMoveItemsL
( const CArrayFix<TInt>& aUids, TInt aFolder )
    {
    TInt err = KErrNone;
    TInt i;
    TInt uid;
    TInt count = aUids.Count();
    TInt itemErr;

    CFavouritesItem* item = CFavouritesItem::NewLC();

    for ( i = 0; i < count && !err; i++ )
        {
        // Get each item in turn from db (ignore if any of them is not found).
        // Let the user rename each. Quit on the first Cancel from the user.
        // All database manipulating methods, which are called here, retry on
        // failure, so if we ever get an error value, that means that the user
        // cancelled something.
        uid = aUids.At( i );
        err = iModel->BeginL( /*aWrite=*/ETrue, /*aDbErrorNote=*/ETrue );
        if ( !err )
            {
            // Check if we have the item.
            itemErr = iModel->Database().Get( uid, *item );
            iModel->CommitL();
            if ( !itemErr )
                {
                // We have this item. Ask the user to rename it.
                if ( iModel->RenameQueryL( *item, EFalse) )
                    {
                    // The user renamed the item: try to update and go on.
                    item->SetParentFolder( aFolder );
                    err = iModel->UpdateL( *item, uid, /*aDbErrorNote=*/ETrue,
                        CBrowserFavouritesModel::EAskIfRename );
                    }
                else
                    {
                    // Cancelled. Quit.
                    err = KErrCancel;
                    }
                }
            }
        }
    CleanupStack::PopAndDestroy();  // item
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::UpdateNaviPaneL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::UpdateNaviPaneL()
    {
    if ( iCurrentFolder == KFavouritesRootUid )
        {
        // In root folder show the tabs.
        iContainer->ShowRootNaviPane();
        }
    else
        {
        // In other folder, show "1/4" style text.
        TInt folderIndex;
        TInt folderCount;
        TInt err;
        err = iModel->FolderInfoL( iCurrentFolder, folderCount, folderIndex );
        if ( !err )
            {
            iContainer->ShowFolderNaviPaneL( folderIndex, folderCount );
            }
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::OfferKeyEventL
// ---------------------------------------------------------
//
TKeyResponse CBrowserFavouritesView::OfferKeyEventL
( const TKeyEvent& aKeyEvent, TEventCode /*aType*/ )
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
            if ( iCurrentFolder != KFavouritesRootUid )
                {
                // We are in some folder. Move between subfolders in the
                // leftward direction.
                if ( !ApiProvider().IsEmbeddedModeOn() )
                    {
                    OpenNextFolderL( /*aForward=*/EFalse );
                    }
                //return EKeyWasConsumed anyway, since in embedded mode user can access only the launch folder
                result = EKeyWasConsumed;
                }
            break;
            }

        case EKeyRightArrow:       // East
            {
            if ( iCurrentFolder != KFavouritesRootUid )
                {
                // We are in some folder. Move between subfolders in the
                // rightward direction.
                if ( !ApiProvider().IsEmbeddedModeOn() )
                    {
                    OpenNextFolderL( /*aForward=*/ETrue );
                    }
                //return EKeyWasConsumed anyway, since in embedded mode user can access only the launch folder
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
            if ( state.AnyDeletable() )
                {
                // We don't editing (not in Goto Pane),
                // and have something to delete. In this case, Clear key
                // deletes the selection (or highlighted items).
                if ( ((iContainer->Listbox()->CurrentItem()->ContextId() == NULL ) &&
                    (iContainer->Listbox()->CurrentItem()->Uid() != KFavouritesAdaptiveItemsFolderUid) ||
                    state.AnyMarked() )
                  )
                    {
                    DeleteMarkedItemsL();
                    }
                result = EKeyWasConsumed;
                }
            break;
            }

        case EKeyOK:
            // OK key is handled through MSK handling
            result = EKeyWasConsumed;
            break;

        case EKeyEnter:
            {
            if ( !(aKeyEvent.iModifiers & EModifierShift) )
                {
                TBrowserFavouritesSelectionState state =
                    iContainer->Listbox()->SelectionStateL();
                if ( state.IsEmpty() || state.AnyMarked() )
                    {
                    // Selection key (alone) pressed when there are marked
                    // items or the list is empty. Bring up context sensitive
                    // (OK-Options) menu.
                    MenuBar( )->
                        SetMenuTitleResourceId( OkOptionsMenuResourceId() );
                    iShowOkOptions = ETrue;
                    // Call to menu activation is trapped - to make sure that
                    // iShowOkOptions and bad menu doesn't stick in.
                    TRAP_IGNORE( ProcessCommandL( EAknSoftkeyOptions ) );
                    iShowOkOptions = EFalse;
                    MenuBar( )->
                        SetMenuTitleResourceId( OptionsMenuResourceId() );

                    // Update CBA to include context menu MSK icon
                    UpdateCbaL();

                    result = EKeyWasConsumed;
                    }
                }
            break;
            }

        default:
            {
            break;
            }
        }

    return result;
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::HandleFavouritesModelChangeL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::HandleFavouritesModelChangeL()
    {
    // Change in favourites database.
    if ( iRefresh )
        {
        RefreshL();
        }
    iRefresh = ETrue;
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::HandleCommsModelChangeL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::HandleCommsModelChangeL()
    {
    // Change in AP-s (CommsDb).
    RefreshL( /*aDbErrorNote=*/EFalse );
    }

void CBrowserFavouritesView::OpenFixedFolderL(TInt aUid)
    {
    OpenFolderL(aUid);
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::SetPreferedCurrentItemL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::SetPreferedCurrentItemL()
    {
    //  check the current item is exist
    if ( iContainer->Listbox()->CurrentItem() )
        {
        iContainer->Listbox()->GetDefaultData().iPreferedId =
            iContainer->Listbox()->CurrentItem()->Uid();
        iModel->Database().SetPreferredUid(
            iCurrentFolder,iContainer->Listbox()->CurrentItem()->Uid() );
        iRefresh = ETrue;
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::SetUnPreferedCurrentItemL
// ---------------------------------------------------------
//
void CBrowserFavouritesView::SetUnPreferedCurrentItemL()
    {
    //  check the current item is exist
    if ( iContainer->Listbox()->CurrentItem() )
        {
        iContainer->Listbox()->GetDefaultData().iPreferedId = NULL;
        iModel->Database().SetPreferredUid( iCurrentFolder,NULL );
        iRefresh = ETrue;
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesView::ConfigContextMenu
// ---------------------------------------------------------
//
void CBrowserFavouritesView::ConfigContextMenu()
    {
    // Config Context Sensitive Menu Resource and Type when items are marked

    MenuBar()->SetContextMenuTitleResourceId( OkOptionsMenuResourceId());
    MenuBar()->SetMenuType(CEikMenuBar::EMenuContext);
    MenuBar()->SetMenuTitleResourceId( OptionsMenuResourceId());
    MenuBar()->SetMenuType(CEikMenuBar::EMenuOptions);  // adds task swapper menu item first
    }

// End of File
