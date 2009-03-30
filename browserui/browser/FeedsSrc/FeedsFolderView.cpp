/*
* Copyright (c) 2005-2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  A view to browse a user's list of feeds.
*
*/



#include <avkon.rsg>

#include <aknviewappui.h>
#include <aknutils.h>
#include <AknToolbar.h>
#include <eikmenup.h>
#include <hlplch.h>
#include <s32mem.h>

#include "Browser.hrh"
#include <BrowserNG.rsg>

#include "BrowserAppUi.h"
#include "CommonConstants.h"
#include <FeedAttributes.h>
#include <FolderAttributes.h>
#include "FeedsFolderContainer.h"
#include "FeedsFolderView.h"
#include "Display.h"


// -----------------------------------------------------------------------------
// CFeedsFolderView::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeedsFolderView* CFeedsFolderView::NewL( MApiProvider& aApiProvider, TRect& aRect )
	{
    CFeedsFolderView* self = new (ELeave) CFeedsFolderView(aApiProvider);
    
    CleanupStack::PushL(self);
    self->ConstructL(aRect);
    CleanupStack::Pop();

    return self;
	}


// -----------------------------------------------------------------------------
// CFeedsFolderView::CFeedsFolderView
//
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CFeedsFolderView::CFeedsFolderView(MApiProvider& aApiProvider):
CBrowserViewBase( aApiProvider ),iPenEnabled(EFalse)
	{
    iPenEnabled = AknLayoutUtils::PenEnabled(); 
	}


// -----------------------------------------------------------------------------
// CFeedsFolderView::ConstructL
//
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFeedsFolderView::ConstructL(TRect& aRect)
    {
    BaseConstructL(R_FEEDS_FOLDER_VIEW);
    
    iContainer = CFeedsFolderContainer::NewL( this, ApiProvider(), aRect );
    iContainer->MakeVisible(EFalse);
    
    if(iPenEnabled)
        {
        Toolbar()->SetToolbarObserver(this);        
        }
    }


// -----------------------------------------------------------------------------
// CFeedsFolderView::~CFeedsFolderView
//
// Deconstructor.
// -----------------------------------------------------------------------------
//
CFeedsFolderView::~CFeedsFolderView()
    {
    DoDeactivate();

    delete iContainer;
    }

// -----------------------------------------------------------------------------
// CFeedsFolderView::Id
//
// Returns the id of the view.
// -----------------------------------------------------------------------------
//
TUid CFeedsFolderView::Id() const
    {
    return KUidBrowserFeedsFolderViewId;
    }

// -----------------------------------------------------------------------------
// CFeedsFolderView::HandleCommandL
//
// Processes commands.
// -----------------------------------------------------------------------------
//
void CFeedsFolderView::HandleCommandL(TInt aCommand)
    {   
    TBool  handled = EFalse;
    
    // Handle softkeys and some standard commands first.
    switch (aCommand)
        {
		case EAknSoftkeyBack:
            if (!iContainer->HandleShowParentFolderL())
            	{
            	// If we are in embedded mode (perhaps someone imported a feed from the messaging app)
            	// then we want to close the browser vs. returning to the bookmark view
            	if (iApiProvider.IsEmbeddedModeOn())
            		{
					AppUi()->HandleCommandL( EWmlCmdCloseBrowser );
            		}
              	// Go back to content view only if we came from there. Note that ContentView can be 
               	// on the history if another application launched a URL.                 
                else if(((ApiProvider().GetPreviousViewFromViewHistory() == KUidBrowserContentViewId) 
                	&& (ApiProvider().FeedsClientUtilities().CalledFromView() == KUidBrowserContentViewId))
                	||(ApiProvider().GetPreviousViewFromViewHistory() == KUidBrowserFeedsTopicViewId)
                	&& (ApiProvider().FeedsClientUtilities().CalledFromView() != KUidBrowserBookmarksViewId))
                	{
                	 	iApiProvider.SetViewToBeActivatedIfNeededL( KUidBrowserContentViewId );	
                	}
            	else
            		{
                  	iApiProvider.SetViewToBeActivatedIfNeededL( KUidBrowserBookmarksViewId );
            		}
                }
            handled = ETrue;
            break;
        case EAknSoftkeyOk:
            iContainer->HandleOkL();
            handled = ETrue;
            break;

        case EAknSoftkeyCancel:
            iContainer->HandleCancelL();
            handled = ETrue;
            break;

		case EAknCmdMark:    	        
		case EAknCmdUnmark:    	        
		case EAknMarkAll:    	        
		case EAknUnmarkAll:
            iContainer->HandleMarkL(aCommand);
            handled = ETrue;
          	iContainer->UpdateCbaL();
            iContainer->UpdateToolbarButtonsState();
            break;
            
#ifdef __SERIES60_HELP
        case EAknCmdHelp:
            {
            iApiProvider.SetLastActiveViewId(Id());
            HlpLauncher::LaunchHelpApplicationL(iEikonEnv->WsSession(), 
                    AppUi()->AppHelpContextL());
            handled = ETrue;
            break;
            }
#endif //__SERIES60_HELP 


        case EEikCmdExit:
        case EAknSoftkeyExit:
            AppUi()->HandleCommandL(aCommand);
            handled = ETrue;
            break;
        }
        
    if (handled)
        {
        return;
        }
        
    // Handle menu commands.
    switch (aCommand)
        {
        case EFeedsOpen:
        case EFeedsActivate:
            iContainer->HandleSelectionL();
            break;

		case EFeedsUpdate:    
		    iContainer->HandleUpdateFeedL();
            break;

		case EFeedsUpdateAll:
		    iContainer->HandleUpdateFeedL(ETrue);
            break;

		case EFeedsNewFeed: 	        
            iContainer->HandleNewFeedL();
            iContainer->UpdateToolbarButtonsState();
            break;
		
		case EFeedsNewFolder:    	        
            iContainer->HandleNewFolderL();
            iContainer->UpdateToolbarButtonsState();
            break;

		case EFeedsEdit:
		case EFeedsRename:
            iContainer->HandleEditL();
            iContainer->UpdateToolbarButtonsState();
            break;

		case EFeedsDelete:
            iContainer->HandleDeleteL();
            break;

		case EFeedsMove:    	        
            iContainer->HandleMoveL();
            break;

		case EFeedsMoveToFolder:    	        
            iContainer->HandleMoveToFolderL();
            break;

		case EFeedsExport:
			iContainer->HandleExportL();
			break;
			
        case EWmlCmdDownloads:            
            ApiProvider().BrCtlInterface().HandleCommandL(
                							(TInt)TBrCtlDefs::ECommandShowDownloads +
                							(TInt)TBrCtlDefs::ECommandIdBase );
            break;

        default:
            iApiProvider.FeedsClientUtilities().HandleCommandL(Id(),aCommand);
            break;
        }        
    }


// -----------------------------------------------------------------------------
// CFeedsFolderView::DoActivateL
//
// Called when the view is activated.
// -----------------------------------------------------------------------------
//
void CFeedsFolderView::DoActivateL(const TVwsViewId& /*aPrevViewId*/, 
        TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
    {
   	StatusPane()->SwitchLayoutL( R_AVKON_STATUS_PANE_LAYOUT_USUAL );
  	StatusPane()->MakeVisible( ETrue );
	ApiProvider().Display().ClearMessageInfo();

    if (!iContainer)
        {
		iContainer = CFeedsFolderContainer::NewL( this, ApiProvider(), ClientRect() );

        // Set the root folder.
        if (iRootFolder != NULL)
            {
            iContainer->RootFolderChangedL(*iRootFolder);
            }
        }

    if (!iContainerOnStack)
        {
        AppUi()->AddToViewStackL(*this, iContainer);
        iContainer->SetRect(ClientRect());
        iContainer->MakeVisible(ETrue);
        iContainerOnStack = ETrue;
        // resize screen after calling SetRect.  This way looks better
        iContainer->HandleResourceChange(KEikDynamicLayoutVariantSwitch);
        }

	iApiProvider.SetLastActiveViewId(Id());
    iContainer->UpdateTitleL();
    iContainer->UpdateListBoxL(iInitialItem);

	iContainer->UpdateCbaL();	

    }


// -----------------------------------------------------------------------------
// CFeedsFolderView::DoDeactivate
//
// Called when the view is deactivated.
// -----------------------------------------------------------------------------
//
void CFeedsFolderView::DoDeactivate()
    {
    if (iContainerOnStack)
        {
        AppUi()->RemoveFromViewStack(*this, iContainer);
        iContainer->MakeVisible(EFalse);
        iContainer->ClearNavigationPane();
        iContainerOnStack = EFalse;
        
        iInitialItem = iContainer->CurrentItemIndex();
        }
	}


// -----------------------------------------------------------------------------
// CFeedsFolderView::DynInitMenuPaneL
//
// Disables unrelated menu options.
// -----------------------------------------------------------------------------
//
void CFeedsFolderView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    const CFeedsEntity*      item = NULL;
    
    // Get the current item.
    item = iContainer->CurrentItem();

    // Handle the main menu.
    if (aResourceId == R_FEEDS_FOLDER_VIEW_MENU)
        {
        // Dynamically build the menu.
        		
        // Downloads (only if download menu not empty (ie, ongoing downloads)
        if (ApiProvider().BrCtlInterface().BrowserSettingL( TBrCtlDefs::ESettingsNumOfDownloads ))
            {
            iApiProvider.FeedsClientUtilities().AddItemL( *aMenuPane, EWmlCmdDownloads, R_FEEDS_OPTIONS_GO_DOWNLOADS);
            }

        iApiProvider.FeedsClientUtilities().AddCascadeL(*aMenuPane,EFeedsOptionsGoto,
            R_FEEDS_OPTIONS_GOTO, R_FEEDS_OPTIONS_GOTO_SUBMENU);

        iApiProvider.FeedsClientUtilities().AddCascadeL(*aMenuPane,EFeedsOptionsFeedsActions,
            R_FEEDS_OPTIONS_FEEDSACTIONS, R_FEEDS_OPTIONS_FEEDSACTIONS_SUBMENU);

        iApiProvider.FeedsClientUtilities().AddCascadeL(*aMenuPane,EFeedsOptionsEdit,
            R_FEEDS_OPTIONS_EDIT, R_FEEDS_OPTIONS_EDIT_SUBMENU);
      
        // these items only visible for non-empty folder
        if (item != NULL)
            {            
            // Mark/Unmark submenu - only for non-empty folder
            iApiProvider.FeedsClientUtilities().AddCascadeL(*aMenuPane,EFeedsOptionsMarkUnmark,
                R_FEEDS_OPTIONS_MARKUNMARK, R_FEEDS_OPTIONS_MARKUNMARK_SUBMENU);
            }

        // ie, privacy submenu
        //iApiProvider.FeedsClientUtilities().AddCascadeL(*aMenuPane,EFeedsOptionsClear,
        //    R_FEEDS_OPTIONS_CLEAR, R_FEEDS_OPTIONS_CLEAR_SUBMENU);
        
        iApiProvider.FeedsClientUtilities().AddCascadeL(*aMenuPane,EFeedsOptionsClear,
            R_FEEDS_OPTIONS_CLEAR, R_CLEAR_SUBMENU);
        }
     else if(aResourceId == R_FEEDS_OPTIONS_GOTO_SUBMENU)
        {     
        // Back to Page (if page loaded)
        if ( iApiProvider.IsPageLoaded() )
            {
            iApiProvider.FeedsClientUtilities().AddItemL( *aMenuPane, EWmlCmdBackToPage, R_BROWSER_MENU_ITEM_BACK_TO_PAGE );
            }
        
        // Bookmarks view (if browser wasn't launched directly into feeds)
        if ( !iApiProvider.BrowserWasLaunchedIntoFeeds() )
		    {
    	    iApiProvider.FeedsClientUtilities().AddItemL( *aMenuPane, EWmlCmdFavourites, R_BROWSER_MENU_ITEM_FAVOURITES );
		    } 
        }
     else if(aResourceId == R_FEEDS_OPTIONS_FEEDSACTIONS_SUBMENU)
        {
        if(iContainer &&
           iContainer->iCurrentFolder &&
           iContainer->iCurrentFolder->GetChildren().Count() > 0)
            {        
            if (!(item->GetType() == EFolder) && !iContainer->IsMarkedItemFolder())
                {
                // Update (only if feed has focus)
                iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EFeedsUpdate, R_FEEDS_UPDATE);
                }
        
            // Update All (only for non-empty views/folders)
            if(iContainer->SearchForFeedL(iRootFolder))
                {
                iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EFeedsUpdateAll, R_FEEDS_UPDATE_ALL);    
                }
  
        
            // Send (only for non-empty views/folders)
	    	iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EFeedsExport, R_OPTIONS_EXPORT_FEEDS);
            }
        
        // Import Feeds
		iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EFeedsImport, R_OPTIONS_IMPORT_FEEDS);

        // Create Feed
        iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EFeedsNewFeed, R_FEEDS_NEW_FEED);        
        }
    else if(aResourceId == R_FEEDS_OPTIONS_EDIT_SUBMENU)
        {        
        if (item != NULL)
        	{
        	TInt markedcount = (iContainer && iContainer->MarkedItems()) ? iContainer->MarkedItems()->Count() : 0;

	        // "Edit Feed" or "Rename Folder" depending upon which has focus
	        if (markedcount < 1)// don't allow edit/rename if feed(s)/folder(s) are marked
                {
                if (item->GetType() == EFolder)
    	            {
    	            iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EFeedsRename, R_FLDR_RENAME);
    	            }
    	        else
    	            {
    	            iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EFeedsEdit, R_FEEDS_EDIT);
    	            }
                }
	            
	        // Delete (only if editable item has focus)
	        iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EFeedsDelete, R_FEEDS_DELETE);
	            
	        // move
	        if ( iContainer &&
	             iContainer->iCurrentFolder &&
             iContainer->iCurrentFolder->GetChildren().Count() >= 2 )
	            {
	            iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EFeedsMove, R_OPTIONS_ORG_MOVE);
	            }
	        
	        // move to folder only if a feed has focus and there are folders available
        if (!(item->GetType() == EFolder))
	            {            
                TInt folderCount = 0;
                iContainer->CountFolderFolders(iContainer->iRootFolder, folderCount);
                
                if(iContainer && (folderCount> 0) && !iContainer->IsMarkedItemFolder())
	                {   
	                iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EFeedsMoveToFolder, R_OPTIONS_ORG_MOVE_TO_FOLDER);
	                }            
	            }
        	}
                
        // Create Folder
        iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EFeedsNewFolder, R_OPTIONS_ORG_NEW_FOLDER);
        }
    else if(aResourceId == R_FEEDS_OPTIONS_MARKUNMARK_SUBMENU)
        {
        // Mark (or unmark)
        if(IsCurrentItemMarked())
            {
            iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EAknCmdUnmark, R_OPTIONS_LIST_UNMARK_ONE);            
            }
        else
            {
            iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EAknCmdMark, R_OPTIONS_LIST_MARK_ONE);  
            }
        
        // Mark All
        iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EAknMarkAll, R_OPTIONS_LIST_MARK_ALL);        
        
        // Unmark ALL
        if (AnyMarkedItems())
            {
            iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EAknUnmarkAll, R_OPTIONS_LIST_UNMARK_ALL);            
            }
        }
    else if(aResourceId == R_FEEDS_OPTIONS_CLEAR_SUBMENU)
        {
        
        
        }     
    iApiProvider.FeedsClientUtilities().DynInitMenuPaneL(aResourceId, aMenuPane );
    }


// -----------------------------------------------------------------------------
// CFeedsFolderView::UpdateCbaL
//
// Updates the options softkey to reflect the command set.
// -----------------------------------------------------------------------------
//
void CFeedsFolderView::UpdateCbaL(TInt aCommandSet)
    {
    if (Cba())
        {
        Cba()->SetCommandSetL(aCommandSet);
        Cba()->DrawDeferred();
        }
    }

// -----------------------------------------------------------------------------
// CFeedsFolderView::RootFolderChangedL
//
// Called to notify the view the that root folder has changed.
// -----------------------------------------------------------------------------
//
void CFeedsFolderView::RootFolderChangedL(const CFeedsEntity& aRootFolder)
    {
    iRootFolder = &aRootFolder;
    
    if (iContainer != NULL)
        {        
        iContainer->RootFolderChangedL(aRootFolder);
        iContainer->UpdateToolbarButtonsState();        
        }
    }


// -----------------------------------------------------------------------------
// CFeedsFolderView::SetCurrentFolder
//
// Changes the current folder.
// -----------------------------------------------------------------------------
//
void CFeedsFolderView::SetCurrentFolder(const CFeedsEntity& aFolder, TBool aResetSelection)
    {
    if (iContainer != NULL)
        {        
        iContainer->SetCurrentFolder(&aFolder, aResetSelection);
        }
        
    if (aResetSelection)
        {
        iInitialItem = 0;
        }
    }
    
// ---------------------------------------------------------------------------
// CFeedsFolderView::CommandSetResourceIdL
// ---------------------------------------------------------------------------
TInt CFeedsFolderView::CommandSetResourceIdL()
    {
    // It is never called, but this function have to be implemented
    return KWmlEmptyResourceId;
    }
    
// ---------------------------------------------------------
// CFeedsFolderView::HandleClientRectChange
// ---------------------------------------------------------
//
void CFeedsFolderView::HandleClientRectChange()
	{
	if( iContainer )
	    {
        iContainer->SetRect( ClientRect() );
        }
	}

// -----------------------------------------------------------------------------
// CFeedsFolderView::UnreadCountChangedL
//
// Calculate the unread count based on delta, return the calculated value.
// -----------------------------------------------------------------------------
//
TInt CFeedsFolderView::UnreadCountChangedL( TInt aUnreadCountDelta )
{
    TInt unreadCount = 0;

    if (iContainer != NULL)
        {        
        unreadCount = iContainer->UnreadCountChangedL( aUnreadCountDelta );
        }
        
    return unreadCount;
}

// -----------------------------------------------------------------------------
// CFeedsFolderView::SetUnreadCount
//
// Set the unread count.
// -----------------------------------------------------------------------------
//
void CFeedsFolderView::SetUnreadCountL( TInt aUnreadCount )
{
    if (iContainer != NULL)
        {        
        iContainer->SetUnreadCountL( aUnreadCount );
        }
}


// -----------------------------------------------------------------------------
// CFeedsFolderView::AnyFoldersMarked
//
// returns ETrue if any Feeds folders are marked otherwise returns EFalse
// -----------------------------------------------------------------------------
//
TBool CFeedsFolderView::AnyFoldersMarked()
{
    const CArrayFix<TInt>*  markedIndexes = iContainer->MarkedItems();
    const CFeedsEntity*      markedItem = NULL;
    TInt i;

    for ( i = 0; i < markedIndexes->Count(); i++ )
        {
        // Get the item.
        markedItem = iContainer->iCurrentFolder->GetChildren()[(*markedIndexes)[i]];
        if ( markedItem != NULL && markedItem->GetType() == EFolder )
            {  
            return ETrue;
            }
        }
        
    return EFalse;        
}


// -----------------------------------------------------------------------------
// CFeedsFolderView::AnyMarkedItems
//
// returns ETrue anything is marked otherwise returns EFalse
// -----------------------------------------------------------------------------
//
TBool CFeedsFolderView::AnyMarkedItems()
{
    const CArrayFix<TInt>*  markedIndexes = iContainer->MarkedItems();

    if (markedIndexes->Count() > 0 )
        {
        return ETrue;
        }
    else
        {
        return EFalse; 
        }
}

// -----------------------------------------------------------------------------
// CFeedsFolderView::IsItemMarked
//
// returns ETrue if item is marked, otherwise returns EFalse
// -----------------------------------------------------------------------------
//

TBool CFeedsFolderView::IsCurrentItemMarked()
{
    // Get the current item's index
    TInt currIndex = iContainer->CurrentItemIndex();
    const CArrayFix<TInt>*  markedIndexes = iContainer->MarkedItems();

    if (markedIndexes->Count() > 0)
        {
        TKeyArrayFix key(0, ECmpTInt);
        TInt pos = 0;
        TInt retVal = markedIndexes->Find(currIndex,key,pos);
        if( retVal == KErrNone)
            {
            return ETrue;
            }
        }
    return EFalse;
}
