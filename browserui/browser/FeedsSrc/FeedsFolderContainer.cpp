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

#include <Browser_platform_variant.hrh>
#include <AknNavi.h>
#include <AknNaviDe.h>
#include <AknNaviLabel.h>
#include <akniconarray.h>
#include <aknlists.h>
#include <aknpopup.h>
#include <aknsconstants.h>
#include <aknsutils.h>
#include <AknToolbar.h>
#include <avkon.mbg>
#include <data_caging_path_literals.hrh>
#include <eikclbd.h>
#include <f32file.h>
#include <gulicon.h>
#include <aknconsts.h>
#include <StringLoader.h>

#ifdef __SERIES60_HELP
// Context-Sensitve Help File
#include <csxhelp/browser.hlp.hrh>
#include "BrowserApplication.h"
#endif // __SERIES60_HELP

#include "Browser.hrh"
#include <BrowserNG.rsg>
#include <browser.mbg>
#include <brctldialogsprovider.h>
#include <browserdialogsprovider.h>

#include <feedattributes.h>
#include <folderattributes.h>

#include <feedsentity.h>
#include "FeedsFolderContainer.h"
#include "FeedsFolderView.h"
#include "ApiProvider.h"
#include "Display.h"
#include "BrowserAppUi.h"
#include "BrowserDialogs.h"
#include "BrowserUtil.h"
#include <AknUtils.h>
// It's also the size of the icon array without favicon
const TInt KFirstFaviconIndex = 6;
const TInt KDateSize = 30;          // Size of Date strings
const TInt KTimeSize = 30;          // Size of Time strings


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeedsFolderContainer* CFeedsFolderContainer::NewL( CFeedsFolderView* aView,
            MApiProvider& aApiProvider, const TRect& aRect)

    {
    CFeedsFolderContainer* self = new (ELeave) CFeedsFolderContainer(aView, aApiProvider);
    
    CleanupStack::PushL(self);
    self->ConstructL(aRect);
    CleanupStack::Pop();

    return self;
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::CFeedsFolderContainer
//
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CFeedsFolderContainer::CFeedsFolderContainer(CFeedsFolderView* aView,
                             MApiProvider& aApiProvider ) :
    iView( aView ),
    iApiProvider( aApiProvider ),
    iOwnsListBoxIcons(ETrue)
    {
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::ConstructL
//
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::ConstructL(const TRect& aRect)
    {
    // Set up the control.
    CreateWindowL();
	SetMopParent( iView );
    InitContainerL(aRect);
    SetRect(aRect);
    ActivateL();
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::~CFeedsFolderContainer
//
// Deconstructor.
// -----------------------------------------------------------------------------
//
CFeedsFolderContainer::~CFeedsFolderContainer()
    {
    iTargetFolderList.Close();
    if (iOwnsListBoxIcons)
        {
        delete iListBoxIcons;
        }
        
    delete iListBox;
    delete iListBoxRows;
    if(iNaviDecorator)
       {
       delete iNaviDecorator;
       iNaviDecorator = NULL;
       }
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::OfferKeyEventL
//
// Handles key event.
// -----------------------------------------------------------------------------
//
TKeyResponse CFeedsFolderContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent, 
        TEventCode aType)
    {
    TKeyResponse isConsumed = EKeyWasConsumed;

    if (aType != EEventKey) 
        {
        return EKeyWasNotConsumed;
        }
        
    // handle # key press by marking current unmarked item or unmarking current marked item.        
    if ((aType == EEventKey) && (aKeyEvent.iScanCode == EStdKeyHash))
        {
        
        // Check if current item is marked
        const CArrayFix<TInt>* markedIndexes = MarkedItems();
        TInt i;
        TBool currentMarked = EFalse;
        
        // check to see if there are any items in the list, if not ignore the #
        if (iListBoxRows->Count() > 0)
        	{
	        for ( i = 0; i < MarkedItems()->Count(); i++ )
	            {
	            if ( CurrentItemIndex() == (*markedIndexes)[i] )
	                {
	                currentMarked = ETrue;
	                break;
	                }
	            }
	            
	        if (currentMarked)
	            {
	            HandleMarkL( EAknCmdUnmark );
	            }
	        else
	            {
	            HandleMarkL( EAknCmdMark );
	            }
	        UpdateCbaL();	                	
        	}

        return EKeyWasConsumed;
        }

    // If a move is active process the move.
    else if (iMoveActive && ((aKeyEvent.iCode == EKeyOK) || (aKeyEvent.iCode == EKeyEnter)))
        {
        PostHandleMoveL();

        iMoveActive = EFalse;
        UpdateCbaL();
        // Undim Toolbar
        DimToolbar(EFalse);
        
        return isConsumed;
        }

    // If the C-Key was pressed then delete the current item.
    else if (aKeyEvent.iCode == EKeyBackspace)
        {
        HandleDeleteL();
        return isConsumed;
        }
        
    isConsumed = EKeyWasNotConsumed;    
    // For handling Enter key in emulator / Keyboard ( Enter key should behave similar to MSK )
	if(EStdKeyEnter == aKeyEvent.iScanCode && EEventKey == aType && AknLayoutUtils::MSKEnabled())
		{
		CEikButtonGroupContainer* myCba = CEikButtonGroupContainer::Current();
		if(myCba != NULL)
			{
			TInt cmdId = myCba->ButtonGroup()->CommandId(CEikButtonGroupContainer::EMiddleSoftkeyPosition);
			if(EAknSoftkeyContextOptions  == cmdId)
				{
				iView->MenuBar()->TryDisplayContextMenuBarL();
				isConsumed = EKeyWasConsumed;
				}
			else if(iListBox->Model()->ItemTextArray()->MdcaCount() == 0)
				{
				iView->HandleCommandL(cmdId);
				isConsumed = EKeyWasConsumed;
				}
			}
		}
    // Otherwise let the listbox handle it.
    if(isConsumed != EKeyWasConsumed)
        {
        isConsumed = iListBox->OfferKeyEventL(aKeyEvent, aType);
        
        // Toolbar buttons should be updated when focus moves from feed to folder or vice versa
        if((aType == EEventKey) && ((aKeyEvent.iScanCode == EStdKeyUpArrow)||(aKeyEvent.iScanCode == EStdKeyDownArrow)) )
    		{
    		UpdateToolbarButtonsState();
      		}        
        }
    return isConsumed;
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::GetHelpContext
//
// Get help context for the control.
// -----------------------------------------------------------------------------
//
#ifdef __SERIES60_HELP
void CFeedsFolderContainer::GetHelpContext(TCoeHelpContext& aContext) const
    {
    // This must be the Browser's uid becasue the help texts are under Browser topics.
    aContext.iMajor = KUidBrowserApplication;
    aContext.iContext = KOSS_HLP_RSS_MAIN;
    }
#endif // __SERIES60_HELP


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::SizeChanged
//
// Called by framework when the view size is changed.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::SizeChanged()
    {
    iListBox->SetRect(Rect());
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleResourceChange
//
// Called by the framework when a display resource changes (i.e. skin or layout).
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleResourceChange(TInt aType)
    {
    CCoeControl::HandleResourceChange(aType);
    iListBox->HandleResourceChange(aType);
    
    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        TRect  rect;
        
        if (AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect))
            {
            SetRect(rect);
            }
        }
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::CountComponentControls
//
// Returns number of components.
// -----------------------------------------------------------------------------
//
TInt CFeedsFolderContainer::CountComponentControls() const
    {
    return 1;
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::ComponentControl
//
// Returns pointer to particular component.
// -----------------------------------------------------------------------------
//
CCoeControl* CFeedsFolderContainer::ComponentControl(TInt aIndex) const
    {
    switch (aIndex)
        {
        case 0:
            return iListBox;

        default:
            return NULL;
        }
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleListBoxEventL
//
// Processes key events from the listbox.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleListBoxEventL(CEikListBox* /*aListBox*/, 
        TListBoxEvent aEventType)
    {
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF    
    if (aEventType == MEikListBoxObserver::EEventEmptyListClicked)
        {
            return;
        }
#endif    
    // An item was selected.
    if ((aEventType == MEikListBoxObserver::EEventEnterKeyPressed) ||
        (aEventType == MEikListBoxObserver::EEventItemDoubleClicked)
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF		
		 || (aEventType == MEikListBoxObserver::EEventItemSingleClicked)
#endif		 
		 )
        {
        if ( iMoveActive )
            {
            HandleOkL();
            }
        else 
            {
            HandleSelectionL();
            }
        }
    // Toolbar buttons status should be changed when focus is moved from feed to folder or viceversa
    else if ( (aEventType == MEikListBoxObserver::EEventItemClicked)
#ifdef BRDO_TOUCH_ENABLED_FF
            ||(aEventType == MEikListBoxObserver::EEventFlickStopped)
#endif // BRDO_TOUCH_ENABLED_FF            
            )
      	{
		UpdateToolbarButtonsState();
      	}
    }

// -----------------------------------------------------------------------------
// CFeedsFolderContainer::SearchForFeed
//
// Recursive loop to find a feed
// -----------------------------------------------------------------------------
// 
TBool CFeedsFolderContainer::SearchForFeedL(const CFeedsEntity* aCurrent)
    {
    TBool isFeed = EFalse;
    
    __ASSERT_DEBUG( (aCurrent != NULL), Util::Panic( Util::EUninitializedData ));

    for (int i = 0; i < (aCurrent->GetChildren().Count()); i++)
        {      
        const CFeedsEntity* item = aCurrent->GetChildren()[i];

        if ( !(item->GetType() == EFolder ))
            {
            return ETrue;
            }
        else
            {
            // is folder empty?
            if ( item->GetChildren().Count() == 0 )
                {
                continue;
                }
            else
                {
                isFeed = SearchForFeedL(item); // recursive

                if ( isFeed )
                    {
                    return ETrue;
                    }
                }
            }

        }
    return EFalse;
    }

// -----------------------------------------------------------------------------
// CFeedsFolderContainer::UpdateToolbarButtons
//
// Updates the state of the toolbar buttons depending on the situation
// -----------------------------------------------------------------------------
//

void CFeedsFolderContainer::UpdateToolbarButtonsState()
    {
    TBool isFeed = EFalse;
    TBool feedInFocus = EFalse;
 

    if (iMoveActive || !iCurrentFolder)
        {
        return;        
        }

 
    TInt num = iCurrentFolder->GetChildren().Count();

    if ( iCurrentFolder->GetChildren().Count() == 0 )
        {
        iView->Toolbar()->SetItemDimmed(EFeedsUpdate, ETrue, ETrue);
        iView->Toolbar()->SetItemDimmed(EFeedsUpdateAll, ETrue, ETrue);
        iView->Toolbar()->SetItemDimmed(EFeedsOpen, ETrue, ETrue);             
        }
    else
        {
        TRAP_IGNORE(
        {
        isFeed = SearchForFeedL(iRootFolder);
        });

		// if feed is not in focus in the current view, then dim the 'update' button
        const CFeedsEntity*  fItem = NULL;
		// Get the current item.
		fItem = CurrentItem();
        if ( !(fItem->GetType() == EFolder ))
            {
        	feedInFocus = ETrue;
            }

        if ( isFeed && feedInFocus )
            {
            iView->Toolbar()->SetItemDimmed(EFeedsUpdate,IsMarkedItemFolder()?ETrue:EFalse, ETrue);
            iView->Toolbar()->SetItemDimmed(EFeedsUpdateAll, EFalse, ETrue);
            iView->Toolbar()->SetItemDimmed(EFeedsOpen, EFalse, ETrue);
            }
        else if ( isFeed && !feedInFocus )
            {
            iView->Toolbar()->SetItemDimmed(EFeedsUpdate, ETrue, ETrue);
            iView->Toolbar()->SetItemDimmed(EFeedsUpdateAll, EFalse, ETrue);
            iView->Toolbar()->SetItemDimmed(EFeedsOpen, EFalse, ETrue);
            }
        else
            {
            iView->Toolbar()->SetItemDimmed(EFeedsUpdate, ETrue, ETrue);
            iView->Toolbar()->SetItemDimmed(EFeedsUpdateAll, ETrue, ETrue);
            iView->Toolbar()->SetItemDimmed(EFeedsOpen, EFalse, ETrue);
            }
        }
    }



// -----------------------------------------------------------------------------
// CFeedsFolderContainer::UpdateFeedL
//
// Called when a feed's name and/or url is changed -- this is called after both
// IsValidFeedName and IsValidFeedUrl are called.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::UpdateFeedL(const TDesC& aName, const TDesC& aUrl, TInt aFreq)
    {
    UpdateCurrentItemL(&aName, &aUrl, aFreq);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::NewFeedL
//
// Called when a new feed is created -- this is called after both
// IsValidFeedName and IsValidFeedUrl are called.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::NewFeedL(const TDesC& aName, const TDesC& aUrl, TInt aFreq)
    {
    AddNewFeedL(aName, aUrl, aFreq);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::IsFeedNameValidL
//
// Validates the Feed's updated name.
// -----------------------------------------------------------------------------
//
TBool CFeedsFolderContainer::IsFeedNameValidL(const TDesC* aName, TBool aIsEditing)
    {
    TBool  valid = ETrue;
    TInt pos(0);
    
    // If aName is NULL show an info dialog and reject it.
    if (aName == NULL)
        {
        TBrowserDialogs::ErrorNoteL( R_FEEDS_TITLE_NEEDED );
        valid = EFalse;
        }

    //If aName contains single quote character, then show an info dialog and reject it
    else if ((pos = aName->Locate('\'')) != KErrNotFound)
        {
        TBrowserDialogs::ErrorNoteL( R_FEEDS_TITLE_SINGLE_QUOTE_NOT_ALLOWED );
        valid = EFalse;
        }

    // If the name is a duplicate show an info dialog and reject it.
    else
        {
        const CFeedsEntity*  otherItem;

        // Reject it if _some other_ item has the same name.
        if ((iRootFolder != NULL) && ((otherItem = iApiProvider.FeedsClientUtilities().Search(*aName,*iRootFolder)) != NULL))
            {            
            if (!((otherItem == CurrentItem()) && aIsEditing))
                {
                TBrowserDialogs::ErrorNoteL( R_FEEDS_NAME_ALREADY_IN_USE );
                valid = EFalse;
                }
            }
        }

    return valid;
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::IsFeedUrlValidL
//
// Validates the Feed's updated url.
// -----------------------------------------------------------------------------
//
TBool CFeedsFolderContainer::IsFeedUrlValidL(const TDesC* aUrl)
    {
    TBool  valid = ETrue;

    // If aUrl is NULL show an info dialog and reject it.
    if (aUrl == NULL)
        {
        valid = EFalse;
        }
        
    // Otherwise if the user didn't change the default url (i.e. "http://"
    // then reject it too.
    else
        {
    	HBufC*  defaultUrl = NULL;
	
        // Load the label
        defaultUrl = CCoeEnv::Static()->AllocReadResourceAsDes16L(R_FEEDS_NEW_FEED_URL_ITEM);
        if (defaultUrl->Compare(*aUrl) == 0)
            {
            valid = EFalse;
            }
            
        delete defaultUrl;
        }

    // Show the error.
    if (!valid)
        {
        TBrowserDialogs::ErrorNoteL( R_FEEDS_ADDRESS_NEEDED );
        }

    return valid;
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::UpdateFolderL
//
// Called when a folder's name is changed -- this is called 
// after IsValidFolderName is called.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::UpdateFolderL(const TDesC& aName)
    {
    UpdateCurrentItemL(&aName, NULL,0); 
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::NewFolderL
//
// Called when a new folder is created -- this is called 
// after IsValidFolderName is called.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::NewFolderL(const TDesC& aName)
    {
    AddNewFolderL(aName);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::IsFolderNameValidL
//
// Validates the folder's updated name.
// -----------------------------------------------------------------------------
//
TBool CFeedsFolderContainer::IsFolderNameValidL(const TDesC* aName, TBool aIsEditing)
    {
    // IsFeedNameValidL does the same thing...
    return IsFeedNameValidL(aName, aIsEditing);
    }


// -----------------------------------------------------------------------------
// CFolderView::RootFolderChangedL
//
// Called to notify the view the that root folder has changed.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::RootFolderChangedL(const CFeedsEntity& aRootFolder)
    {
    // Set the new root folder.
    iRootFolder = &aRootFolder;
    
    // Restore the previous current folder.
    SetCurrentFolder(iApiProvider.FeedsClientUtilities().Search(iCurrentFolderId,aRootFolder));
    
    // Update the list box to reflect the new root folder.
    UpdateListBoxL(CurrentItemIndex());
    
    // Update the Cba to reflect the new state.
    UpdateCbaL();
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::UpdateTitleL
//
// Update the view's title.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::UpdateTitleL()
    {

	TPtrC title;
    // Set the view's title.
    if (iCurrentFolder != NULL && iCurrentFolder != iRootFolder)
        {
        if (iCurrentFolder->GetStringValue(EFolderAttributeTitle,title) != KErrNotFound && title.Length() != 0)
            {
            iApiProvider.Display().SetTitleL(title);
            }
        else
            {
            iApiProvider.Display().SetTitleL(KNullDesC);
            }
        }
    else
        {
        iApiProvider.Display().SetTitleL(R_FEEDS_FOLDER_VIEW_TITLE);
        }
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::CurrentItem
//
// Returns the current folder-item.
// -----------------------------------------------------------------------------
//
CFeedsEntity* CFeedsFolderContainer::CurrentItem() const
    {
    CFeedsEntity*  item = NULL;
    TInt                index;

    index = iListBox->CurrentItemIndex();
    if (index >= 0)
        {
        item = iCurrentFolder->GetChildren()[index];
        }

    return item;
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::CurrentItemIndex
//
// Returns the index of the current folder-item.
// -----------------------------------------------------------------------------
//
TInt CFeedsFolderContainer::CurrentItemIndex() const
    {
    return iListBox->CurrentItemIndex();
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::UpdateCurrentItemL
//
// Updates the current folder-item.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::UpdateCurrentItemL(const TDesC* aName, const TDesC* aUrl, TInt aFreq)
    {
    TPtrC  name(KNullDesC);
    TPtrC  url(KNullDesC);

    if (aName != NULL)
        {
        name.Set(*aName);
        }
    if (aUrl != NULL)
        {
        url.Set(*aUrl);
        }

    // Update the folder item.
    CFeedsEntity *aCurrentItem = CurrentItem();
    __ASSERT_DEBUG( (aCurrentItem != NULL), Util::Panic( Util::EUninitializedData ) );
    iApiProvider.FeedsClientUtilities().ChangeFolderItemL(*(aCurrentItem), name, url, aFreq);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::AddNewFeedL
//
// Add a new feed.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::AddNewFeedL(const TDesC& aName, const TDesC& aUrl, TInt aFreq)
    {
    // Add the new feed.
    iApiProvider.FeedsClientUtilities().AddFolderItemL(aName, aUrl, EFalse, *iCurrentFolder, aFreq);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::AddNewFolderL
//
// Add a new folder.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::AddNewFolderL(const TDesC& aName)
    {
    // Add the new folder.
    iApiProvider.FeedsClientUtilities().AddFolderItemL(aName, KNullDesC, ETrue, *iCurrentFolder, 0);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::DeleteItemL
//
// Delete the current feed or folder
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::DeleteCurrentItemL()
    {
    RPointerArray<const CFeedsEntity>  markedItems(10);
    const CArrayFix<TInt>*            markedIndexes = NULL;
    const CFeedsEntity*                folder = NULL;

    CleanupClosePushL(markedItems);

    // Get the array of marked items.
    markedIndexes = MarkedItems();
    if ((markedIndexes == NULL) || (markedIndexes->Count() == 0))
        {
        iListBox->View()->SelectItemL(CurrentItemIndex());
        markedIndexes = MarkedItems();
        }

    // Copy the marked items into a temp array.
    for (TInt i = 0; i < markedIndexes->Count(); i++)
        {
        folder = iCurrentFolder->GetChildren()[(*markedIndexes)[i]];
        User::LeaveIfError(markedItems.Append(folder));
        }
    
    // Delete the items.
    iApiProvider.FeedsClientUtilities().DeleteFolderItemsL(markedItems);
        
    // Clean up
    CleanupStack::PopAndDestroy(/*markedItems*/);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HasMarkedItem
//
// Returns an array containing the indexes of the marked items.
// -----------------------------------------------------------------------------
//
const CArrayFix<TInt>* CFeedsFolderContainer::MarkedItems()
    {
    return iListBox->View()->SelectionIndexes();
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleShowParentFolderL
//
// Replaces the current folder with its parent.
// -----------------------------------------------------------------------------
//
TBool CFeedsFolderContainer::HandleShowParentFolderL(void)
    {
    TBool  handled = ETrue;

    if (iCurrentFolder && ( iCurrentFolder != iRootFolder))
        {
        const CFeedsEntity*  oldItem = NULL;

        // Refresh the list-box with the parent's contents.
        oldItem = iCurrentFolder;
        SetCurrentFolder(iCurrentFolder->GetParent());
        UpdateListBoxL();

        // Restore the old selection.
         __ASSERT_DEBUG( (oldItem != NULL), Util::Panic( Util::EUninitializedData ));
        TInt index = 0;
        for(TInt i=0;i<iCurrentFolder->GetChildren().Count();i++)
        {
        	if(iCurrentFolder->GetChildren()[i] == oldItem)
        	{
        		index = i;
        	}
        }
      	iListBox->SetCurrentItemIndex(index);
		iListBox->DrawDeferred();
        
        // Update the Cba to reflect the new state.
        UpdateCbaL();
        }
    else
        {
        handled = EFalse;
        }

    return handled;
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleSelectionL
//
// Processes key events from the listbox.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleSelectionL(void)
    {
    TInt                index;
    const CFeedsEntity*  item;

    index = iListBox->CurrentItemIndex();
    item = const_cast<CFeedsEntity*>(iCurrentFolder->GetChildren()[index]);
    User::LeaveIfNull((void*) item);

    // If a folder was selected - show it.
    if (item->GetType() == EFolder)
        {
        SetCurrentFolder(item);
        UpdateListBoxL();
        UpdateCbaL();
        UpdateToolbarButtonsState();
        }

    // Otherwise show the Feed.
    else 
        {
        ShowFeedL(*item);
        }
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleUpdateFeedL
//
// Handles updating the current Feed.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleUpdateFeedL(TBool aUpdateAll)
    {
    // Update the selected items if "Update" was selected.
    if (!aUpdateAll)
        {        
        const CArrayFix<TInt>*            markedIndexes = NULL;
        RPointerArray<const CFeedsEntity>  folderItems(10);

        CleanupClosePushL(folderItems);

        // Get the marked indexes.
        markedIndexes = MarkedItems();

        // If nothing is marked then use the current item.
        if ((markedIndexes == NULL) || (markedIndexes->Count() == 0))
            {
            folderItems.AppendL(iCurrentFolder->GetChildren()[
                    CurrentItemIndex()]);
            }

        // Otherwise copy the marked items into a temp array.
        else
            {            
            for (TInt i = 0; i < markedIndexes->Count(); i++)
                {
                folderItems.AppendL(iCurrentFolder->GetChildren()[
                        (*markedIndexes)[i]]);
                }
            }
            
        // Update the items.
        iApiProvider.FeedsClientUtilities().UpdateFolderItemsL(folderItems);
            
        // Clean up
        CleanupStack::PopAndDestroy(/*folderItems*/);
        }        

    // Otherwise update all of the items in the current FolderList.
    else
        {
        iApiProvider.FeedsClientUtilities().UpdateFolderItemsL();
        }
    }
    
    
// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleNewFeedL
//
// Handles the add-feed command.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleNewFeedL()
    {
    // Display the edit feed folder
    CFeedsEditFeedDialog*  dialog = NULL;

    dialog = CFeedsEditFeedDialog::NewL(*this, iView->AppUi());
    
    DimToolbar(ETrue);
    // Note: The result is handled in the MEditFeedDialogObserver methods.
    dialog->ExecuteLD(R_FEEDS_EDIT_FEED_DIALOG);
    dialog = NULL;
    DimToolbar(EFalse);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleNewFolderL
//
// Handles the add-folder command.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleNewFolderL()
    {
    // Display the edit feed folder
    CFeedsEditFolderDialog*  dialog = NULL;

    dialog = CFeedsEditFolderDialog::NewL(*this, iView->AppUi());
    
    DimToolbar(ETrue);
    // Note: The result is handled in the MEditFolderDialogObserver methods.
    dialog->ExecuteLD(R_FEEDS_EDIT_FOLDER_DIALOG);
    dialog = NULL;
    
    DimToolbar(EFalse);

    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleEditL
//
// Handles the edit command.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleEditL()
    {
    const CFeedsEntity*  item = NULL;

    // Get the current item.
    item = CurrentItem();
	TPtrC title;
	TPtrC url;
        TInt freq;

	if(item->GetType() == EFolder)
	   {
	   item->GetStringValue(EFolderAttributeTitle,title);
	   }
	else
	   {
	   item->GetStringValue(EFeedAttributeTitle,title);	
	   item->GetStringValue(EFeedAttributeLink,url);
       item->GetIntegerValue(EFeedAttributeAutoUpdateFreq,freq);
	   }

	__ASSERT_DEBUG( (item != NULL), Util::Panic( Util::EUninitializedData ));

    DimToolbar(ETrue);

    // Display the edit feed dialog
    if (!(item->GetType() == EFolder))
        {
        CFeedsEditFeedDialog*  dialog = NULL;

        dialog = CFeedsEditFeedDialog::NewL(*this, iView->AppUi(), title, url, freq);

	 	__ASSERT_DEBUG( (dialog != NULL), Util::Panic( Util::EUninitializedData ));
        
        // Note: The result is handled in the MEditFeedDialogObserver methods.
        dialog->ExecuteLD(R_FEEDS_EDIT_FEED_DIALOG);
        dialog = NULL;
        }
    
    // Display the edit folder dialog.
    else
        {
        CFeedsEditFolderDialog*  dialog = NULL;

        dialog = CFeedsEditFolderDialog::NewL(*this, iView->AppUi(), title);
        
	 	__ASSERT_DEBUG( (dialog != NULL), Util::Panic( Util::EUninitializedData ));

        // Note: The result is handled in the MEditFolderDialogObserver methods.
        dialog->ExecuteLD(R_FEEDS_EDIT_FOLDER_DIALOG);
        dialog = NULL;
        }
        
    DimToolbar(EFalse);
        
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleDelete
//
// Handles the delete command.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleDeleteL()
    {
    TBool                   deleteIt = EFalse;
    TBool                   clearOnCancel = EFalse;
    const CArrayFix<TInt>*  markedIndexes = NULL;

    // Return if there are no items in the folder
    if (CurrentItem() == NULL)
        {
        return;
        }

    // Get the marked items.
    markedIndexes = MarkedItems();
    
    // If none are marked then mark the current one.
    if ((markedIndexes == NULL) || (markedIndexes->Count() == 0))
        {
        iListBox->View()->SelectItemL(CurrentItemIndex());
        clearOnCancel = ETrue;
        markedIndexes = MarkedItems();
        }


    // Multiple items are being deleted.
    if (markedIndexes->Count() > 1)
        {
		if(TBrowserDialogs::ConfirmQueryYesNoL(R_FEEDS_DELETE_MULTIPLE_FEED))
            {
            deleteIt = ETrue;
            }
        }

    // Otherwise only a single item is being deleted.
    else
        {
        const CFeedsEntity*  item = NULL;
        
        // Get the item.
        item = iCurrentFolder->GetChildren()[((*markedIndexes)[0])];

		TPtrC title;
		if (item->GetType() == EFolder)
		    {
			item->GetStringValue(EFolderAttributeTitle,title);
	    	}
	    else
	        {
			item->GetStringValue(EFeedAttributeTitle,title);
	        }
    	if(TBrowserDialogs::ConfirmQueryYesNoL(R_FEEDS_DELETE_FEED, title))
            {
            deleteIt = ETrue;
            }
        }

    // Delete it
    if (deleteIt)
        {
        DeleteCurrentItemL();
        }
    else if (clearOnCancel)
        {
        iListBox->View()->ClearSelection();
        }        
    }
    
    
// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleMove
//
// Handles the move command.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleMoveL()
    {
    const CArrayFix<TInt>* markedItems = NULL;

    // Set the move flag.
    iMoveActive = ETrue;

    // If nothing was marked then mark the current item.
    markedItems = MarkedItems();
    if ((markedItems == NULL) || (markedItems->Count() == 0))
        {
        iListBox->View()->SelectItemL(CurrentItemIndex());
        }

    UpdateCbaL();
    // Dim Toolbar
    DimToolbar(ETrue);
    }
    
    
// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleMoveToFolderL
//
// Handles the move to folder command.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleMoveToFolderL()
    {
    CAknSingleGraphicPopupMenuStyleListBox*  listBox = NULL;
    CAknPopupList*                           popupList = NULL;
    const CFeedsEntity*                       targetFolder = NULL;

    // Construct the basic Pop-up
    MoveToFolderInitL(listBox, popupList);
    CleanupStack::PushL(listBox);

    // Show the pop-up.
    if (popupList->ExecuteLD())
        {
        TInt  selected;

        selected = listBox->CurrentItemIndex();

        if (selected >= 0)
            {
            targetFolder = iTargetFolderList[selected];
            }
        }

    CleanupStack::PopAndDestroy(listBox);

    // Moved the marked items into the targeted folder.
    if (targetFolder != NULL)
        {
        MoveToFolderMoveL(*targetFolder);
        }
    }
    
    
// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleMarkL
//
// Handles the mark-related commands.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleMarkL(TInt aCommand)
    {
    AknSelectionService::HandleMarkableListProcessCommandL(aCommand, iListBox);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleOkL
//
// Handles the ok commands.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleOkL()
    {
    // If a move is active process the move.
    if (iMoveActive)
        {
        PostHandleMoveL();

        iMoveActive = EFalse;
        UpdateCbaL();
        // Un-Dim Toolbar
        DimToolbar(EFalse);
        }
    }
    
    
// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleCancelL
//
// Handles the cancel commands.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleCancelL()
    {
    if (iMoveActive)
        {
        // Clear the move state and update the Cba.
        iMoveActive = EFalse;
		// Remove the marks.
        iListBox->View()->ClearSelection();
        UpdateCbaL();
        // Un-Dim Toolbar
        DimToolbar(EFalse);
        }
    }

// -----------------------------------------------------------------------------
// CFeedsFolderContainer::HandleExportL
//
// Handles the export commands.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::HandleExportL()
    {
    RPointerArray<const CFeedsEntity>  markedItems(10);
    const CArrayFix<TInt>*            markedIndexes = NULL;
    const CFeedsEntity*                folder = NULL;

    CleanupClosePushL(markedItems);

    // Get the array of marked items.
    markedIndexes = MarkedItems();
    if ((markedIndexes == NULL) || (markedIndexes->Count() == 0))
        {
        iListBox->View()->SelectItemL(CurrentItemIndex());
        markedIndexes = MarkedItems();
        }

    // Copy the marked items into an array.
    for (TInt i = 0; i < markedIndexes->Count(); i++)
        {
        folder = iCurrentFolder->GetChildren()[((*markedIndexes)[i])];
        User::LeaveIfError(markedItems.Append(folder));
        }
       
    _LIT(KOpmlExt, ".opml");
    HBufC* prompt = StringLoader::LoadLC( R_FEEDS_NAME_EXPORTED_FILE );
	TBuf<KBrowserMaxPrompt+1> retString;
	retString.Copy( KNullDesC );
	
	TInt result = TBrowserDialogs::DialogPromptReqL(
		prompt->Des(),
		&retString,
        EFalse,
		KFavouritesMaxBookmarkNameDefine );
	
	if( result )
		{
		// only append .opml if user has not already done so
		TInt dotPos = retString.LocateReverse( '.' );
		
		if ( dotPos != KErrNotFound )
			{
			// dot found, now check extension
            TInt extLength = retString.Length() - dotPos;
            HBufC* ext = retString.Right( extLength ).AllocL();
            CleanupStack::PushL( ext );

            // if not .opml append extension
            if ( ext->CompareF( KOpmlExt ) != 0)
            	{
				retString.Append(KOpmlExt);
            	}
			
			CleanupStack::PopAndDestroy(); // ext buffer
			}
		else
			{
			// no dot, definitely append			
			retString.Append(KOpmlExt);
			}

		// Pass folder array to feeds utilities to export
		iApiProvider.FeedsClientUtilities().ExportFeedsL(markedItems, retString);
		}
			
	CleanupStack::PopAndDestroy();  // prompt
		       
    // Clean up
    CleanupStack::PopAndDestroy(/*markedItems*/);
   
   	// Clear the selection
   	iListBox->View()->ClearSelection();
   	UpdateCbaL();
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::ShowFeedL
//
// Show the feed in the TopicView.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::ShowFeedL(const CFeedsEntity& aFolderItem)
    {
    iApiProvider.FeedsClientUtilities().ShowTopicViewL(aFolderItem);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::InitContainerL
//
// Inits the container.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::InitContainerL(const TRect& aRect)
    {
    // Init the basic list-box
    iListBox = new (ELeave) CAknDoubleGraphicStyleListBox;
    iListBox->ConstructL(this, EAknListBoxMarkableList);
    iListBox->SetContainerWindowL(*this);
    iListBox->SetRect(aRect.Size());
    iListBox->SetListBoxObserver(this);

    HBufC* emptyText = iCoeEnv->AllocReadResourceLC(R_FEEDS_NO_FEEDS);
    iListBox->View()->SetListEmptyTextL(*emptyText);
    CleanupStack::PopAndDestroy(emptyText);

    // Init the list-box's model.
    iListBoxRows = new (ELeave) CDesCArraySeg(10);
    iListBox->Model()->SetItemTextArray(iListBoxRows);
    iListBox->Model()->SetOwnershipType(ELbmDoesNotOwnItemArray);

    // Add scrollbars.
    iListBox->ActivateL();
    iListBox->CreateScrollBarFrameL(ETrue);
    iListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOn, CEikScrollBarFrame::EAuto);

    // Add the needed icons.
    InitIconArrayL();

    // Enable marquee.
	iListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);

    UpdateCbaL();
    }
    

// -----------------------------------------------------------------------------
// CFeedsFolderContainer::InitIconArrayL
//
// Inits the array of needed icons.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::InitIconArrayL()
    {
    _LIT(KDirAndFile,"z:browser.mbm");
    
    TParse*  fp = NULL;
    
    // Build the path to the feeds resource file.
    fp = new (ELeave) TParse();     
    CleanupStack::PushL(fp);
    
    User::LeaveIfError(fp->Set(KDirAndFile, &KDC_APP_BITMAP_DIR, NULL)); 
    TBuf<KMaxFileName> iconFile= fp->FullName();
    CleanupStack::PopAndDestroy(fp);

    // Create the icon array.    
    iListBoxIcons = new (ELeave) CAknIconArray( KFirstFaviconIndex );

    // Add the mark icon.
    AppendIconL(iListBoxIcons, KAknsIIDQgnIndiMarkedAdd, KAvkonBitmapFile(),
            EMbmAvkonQgn_indi_marked_add, EMbmAvkonQgn_indi_marked_add_mask);

    // Add the folder icon.
    AppendIconL(iListBoxIcons, KAknsIIDQgnPropFolderRss, iconFile,
            EMbmBrowserQgn_prop_folder_rss, EMbmBrowserQgn_prop_folder_rss_mask);

    // Add the folder error icon       
    AppendIconL(iListBoxIcons, KAknsIIDQgnPropFolderRssError, iconFile,
            EMbmBrowserQgn_prop_folder_rss_error, EMbmBrowserQgn_prop_folder_rss_error_mask);            

    // Add the feed icon for feed having unread count as 0.
    AppendIconL(iListBoxIcons, KAknsIIDQgnPropFileRss, iconFile,
            EMbmBrowserQgn_prop_file_rss, EMbmBrowserQgn_prop_file_rss_mask);

    // Add the feed icon for feed having unread count as non-0.
    AppendIconL(iListBoxIcons, KAknsIIDQgnPropFileRssNew, iconFile,
            EMbmBrowserQgn_prop_file_rss_new, EMbmBrowserQgn_prop_file_rss_new_mask);
   
    // Add the feed error icon
    AppendIconL(iListBoxIcons, KAknsIIDQgnPropFileRssError, iconFile,
            EMbmBrowserQgn_prop_file_rss_error, EMbmBrowserQgn_prop_file_rss_error_mask);            

    // Set the icons and cleanup
    iListBox->ItemDrawer()->ColumnData()->SetIconArray(iListBoxIcons);
    iOwnsListBoxIcons = EFalse;
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::AppendIconL
//
// Loads and appends an icon to the icon array.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::AppendIconL(CArrayPtr<CGulIcon>* aIcons, 
        const TAknsItemID& aID, const TDesC& aFilename, const TInt aFileBitmapId, 
        const TInt aFileMaskId)
    {
	CGulIcon*    newIcon;
	CFbsBitmap*  newIconBmp;
	CFbsBitmap*  newIconMaskBmp;

	__ASSERT_DEBUG( (aIcons != NULL), Util::Panic( Util::EUninitializedData ));


    // Create the bitmaps: the bitmaps are left on the cleanup stack.
	AknsUtils::CreateIconL(AknsUtils::SkinInstance(), aID, newIconBmp, 
            newIconMaskBmp, aFilename, aFileBitmapId, aFileMaskId);

    CleanupStack::PushL(newIconBmp);
    CleanupStack::PushL(newIconMaskBmp);

	newIcon = CGulIcon::NewL(newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(newIconMaskBmp);
    CleanupStack::Pop(newIconBmp);
    CleanupStack::PushL(newIcon);

	aIcons->AppendL(newIcon);
    CleanupStack::Pop(newIcon);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::UpdateListBoxL
//
// Sets the list-box's values.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::UpdateListBoxL(TInt aInitialSelection)
    {
    //0\t\1st Line of text\t2nd Line of text\t0
    TBuf<255>                     listBoxtxt;

    if (iCurrentFolder == NULL)
        {
        return;
        }

    // Clear the list.
    iListBoxRows->Reset();
    
    // Delete the old favicons.
    iListBoxIcons->Delete(KFirstFaviconIndex, iListBoxIcons->Count() - KFirstFaviconIndex);

	// Remove the marks.
    iListBox->View()->ClearSelection();

    if (iView->iContainer->IsVisible())
        {
        // Update the title.
        UpdateTitleL();
        }

    // Add the feeds.
    for (int i = 0; i < iCurrentFolder->GetChildren().Count(); i++)
        {
        const CFeedsEntity*  item = NULL;
        TInt iconIndex;
        
        item = iCurrentFolder->GetChildren()[i];
		TPtrC itemName;
		TInt statusCode;
		
		if (item->GetType() == EFolder)
		    {
        	item->GetStringValue(EFolderAttributeTitle,itemName);
	    	}
		else
		    {
        	item->GetStringValue(EFeedAttributeTitle,itemName);
		    }
        item->GetIntegerValue(EFolderAttributeStatus, statusCode);

        // Set the name.
        TInt len;

        len = itemName.Length();
        if (len > 253)
            {
            len = 253;
            }

        // Build the item's string.
        if (item->GetType() == EFolder)
            {
		   	   if (statusCode == KErrNone)
		   		   {
		           listBoxtxt.Zero();
		           listBoxtxt.AppendFormat(_L("%d\t%S\t"), EIconFolder, &itemName);
		           listBoxtxt.Append(KNullDesC());
		           }
		       else
		           {
		           listBoxtxt.Zero();
		           listBoxtxt.AppendFormat(_L("%d\t%S\t"), EIconErrorFolder, &itemName);
		           listBoxtxt.Append(KNullDesC());
		           } 
            }
        else
        	{
            if(statusCode == KErrNone)
                {
                CGulIcon*  favicon = NULL;
                iconIndex = EIconFeed;
                           
                // Convert the item's timestamp into a localized string.
                TBuf<KDateSize + KTimeSize + 1>  timestamp;
                TBuf<KTimeSize>  temp;    
                TTime  date;                       
        		TInt unreadCount;
        	    item->GetIntegerValue(EFolderAttributeUnreadCount,unreadCount);
                
                // Since the folder list isn't updated after it is fetched
                // the FeedsClientUtilities tracks the update times for feeds
                // the user visits in the current session.
                date = iApiProvider.FeedsClientUtilities().FeedUpdatedTime(*item);
                    
                // If the date isn't zero then format the date
                if (date.Int64() != 0)
                    {                
                    // Translate from UTC to local time.
                    TTime                 local;
                    TTime                 utc;
                    TTimeIntervalSeconds  delta;
                    
                    local.HomeTime();
                    utc.UniversalTime();
                    utc.SecondsFrom(local, delta);                
                    date -= delta;

                    //Set time
                    HBufC* timeFormat = iEikonEnv->AllocReadResourceLC( R_QTN_TIME_USUAL_WITH_ZERO );
                    date.FormatL( timestamp, *timeFormat );
                    CleanupStack::PopAndDestroy( timeFormat );//timeFormat
                    //
                    timestamp.Append(_L("  "));                    
                    //Set date
                    HBufC* dateFormat = iEikonEnv->AllocReadResourceLC( R_QTN_DATE_USUAL_WITH_ZERO );
                    date.FormatL( temp, *dateFormat );
                    CleanupStack::PopAndDestroy( dateFormat );//dateFormat
                    //
                    timestamp.Append(temp);
                    // Convert to locale specific numbers 
                    if (TBidiText::ERightToLeft == AknTextUtils::CurrentScriptDirectionality ())
                        {
                        AknTextUtils::DisplayTextLanguageSpecificNumberConversion(timestamp); 
                        }
                    }
                    
                // If there is a favicon for this url then add it to the icon list
                // set the icon's index.
        		TPtrC url;
		    	item->GetStringValue(EFeedAttributeLink,url);
                favicon = iApiProvider.FeedsClientUtilities().FaviconL(url);
                // favicon exist, no matter unread count is 0 or not, use favicon
                if (favicon != NULL)
                    {
                    iListBoxIcons->AppendL(favicon);
                    iconIndex = iListBoxIcons->Count() - 1;
                    }
                // no favicon, and unread count > 0, use unread icon
                else if (unreadCount > 0)
                    {
                    iconIndex = EIconFeedUnread;
                    }
                // else: no favicon, and unread count == 0, use read icon

                // Add the line.
                listBoxtxt.Zero();

                if (unreadCount > 0)
                    {
                    listBoxtxt.AppendFormat(_L("%d\t%S(%d)\t%S"), iconIndex, &itemName, unreadCount, &timestamp);
                    }
                else
                    {
                    listBoxtxt.AppendFormat(_L("%d\t%S\t%S"), iconIndex, &itemName, &timestamp);
                    }

                listBoxtxt.Append(KNullDesC());
                }
            else if(((statusCode >= 20400 )&& (statusCode < 20500 )) || (statusCode == KErrBadName))   
            	{
                iconIndex = EIconErrorFeed;
                HBufC* text = NULL;
                    
                text = iCoeEnv->AllocReadResourceL(R_FEEDS_INVALID_URL);
                TPtr txtPtr = text->Des();
                CleanupStack::PushL(text);
    		    listBoxtxt.Zero();
                listBoxtxt.AppendFormat(_L("%d\t%S\t%S"), iconIndex, &itemName, &txtPtr);
                listBoxtxt.Append(KNullDesC());
                CleanupStack::PopAndDestroy();
           		}
            else if( statusCode == KErrNotSupported)
            	{
                iconIndex = EIconErrorFeed;
                HBufC* text = NULL;
                    
                text = iCoeEnv->AllocReadResourceL(R_FEEDS_UNSUPPORTED_FEED);
                TPtr txtPtr = text->Des();
                CleanupStack::PushL(text);
    		    listBoxtxt.Zero();
                listBoxtxt.AppendFormat(_L("%d\t%S\t%S"), iconIndex, &itemName, &txtPtr);
                listBoxtxt.Append(KNullDesC());
                CleanupStack::PopAndDestroy();
           		} 
            else if ( statusCode == KErrTimedOut)
            	{
                iconIndex = EIconErrorFeed;
                HBufC* text = NULL;
                    
                text = iCoeEnv->AllocReadResourceL(R_FEEDS_NETWORK_ERROR);
                TPtr txtPtr = text->Des();
                CleanupStack::PushL(text);
    		    listBoxtxt.Zero();
                listBoxtxt.AppendFormat(_L("%d\t%S\t%S"), iconIndex, &itemName, &txtPtr);
                listBoxtxt.Append(KNullDesC());
                CleanupStack::PopAndDestroy();                
            	}
            else if ( statusCode == KErrCorrupt)
            	{
                iconIndex = EIconErrorFeed;
                HBufC* text = NULL;
                    
                text = iCoeEnv->AllocReadResourceL(R_FEEDS_MALFORMED_FEED);
                TPtr txtPtr = text->Des();
                CleanupStack::PushL(text);
    		    listBoxtxt.Zero();
                listBoxtxt.AppendFormat(_L("%d\t%S\t%S"), iconIndex, &itemName, &txtPtr);
                listBoxtxt.Append(KNullDesC());
                CleanupStack::PopAndDestroy();                
            	}
            else if(statusCode> 0)
            	{
                iconIndex = EIconErrorFeed;
                HBufC* text = NULL;
                    
                text = iCoeEnv->AllocReadResourceL(R_FEEDS_NETWORK_ERROR);
                TPtr txtPtr = text->Des();
                CleanupStack::PushL(text);
    		    listBoxtxt.Zero();
                listBoxtxt.AppendFormat(_L("%d\t%S\t%S"), iconIndex, &itemName, &txtPtr);
                listBoxtxt.Append(KNullDesC());
                CleanupStack::PopAndDestroy();                
            	}
            else
            	{
                iconIndex = EIconErrorFeed;
                HBufC* text = NULL;
                    
                text = iCoeEnv->AllocReadResourceL(R_FEEDS_GENERAL_ERROR_MAIN_PANE);
                TPtr txtPtr = text->Des();
                CleanupStack::PushL(text);
    		    listBoxtxt.Zero();
                listBoxtxt.AppendFormat(_L("%d\t%S\t%S"), iconIndex, &itemName, &txtPtr);
                listBoxtxt.Append(KNullDesC());            	
                CleanupStack::PopAndDestroy();                
            	}
        	}
       		
        // Add the row.
        iListBoxRows->AppendL(listBoxtxt);    
        }

    // Restore the selection -- the best it can that is.
    if (iCurrentFolder->GetChildren().Count() > 0)
        {
        if (aInitialSelection > (iCurrentFolder->GetChildren().Count() - 1))
            {
            aInitialSelection = iCurrentFolder->GetChildren().Count() - 1;
            }
        if (aInitialSelection < 0)
            {
            aInitialSelection = 0;
            }

        iListBox->SetCurrentItemIndex(aInitialSelection);
        }

    iListBox->HandleItemAdditionL();
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::MoveToFolderInitL
//
// Inits the move-to-folder pop-up.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::MoveToFolderInitL(CAknSingleGraphicPopupMenuStyleListBox*&
        aListBox, CAknPopupList*& aPopupList)
    {
    CDesCArraySeg*        listBoxRows = NULL;
    CTextListBoxModel*    listboxModel = NULL;
    HBufC*                title = NULL;
    CArrayPtr<CGulIcon>*  icons = NULL;

    // Construct the basic Pop-up
    aListBox = new (ELeave) CAknSingleGraphicPopupMenuStyleListBox();
    CleanupStack::PushL(aListBox);

	aPopupList = CAknPopupList::NewL(aListBox, 
            R_FEEDS_FOLDER_VIEW_CBA_MOVETOFOLDER_MOVECANCEL);
    CleanupStack::PushL(aPopupList);

    aListBox->ConstructL(aPopupList, EAknListBoxSelectionList | EAknListBoxLoopScrolling);

    // Set the title.
    title = CCoeEnv::Static()->AllocReadResourceLC(R_FEEDS_FOLDER_VIEW_MOVE_TO_PRMPT);
    aPopupList->SetTitleL(*title);
    CleanupStack::PopAndDestroy(title);

    // Construct the list-box's model.
    listBoxRows = new (ELeave) CDesCArraySeg(10);

    listboxModel = aListBox->Model();
    listboxModel->SetItemTextArray(listBoxRows);
    listboxModel->SetOwnershipType(ELbmOwnsItemArray);

    // Add scrollbars.
    aListBox->CreateScrollBarFrameL(ETrue);
    aListBox->ScrollBarFrame()->SetScrollBarVisibilityL(CEikScrollBarFrame::EOff, 
            CEikScrollBarFrame::EAuto);

    // Enable item marqueeing.
	aListBox->ItemDrawer()->ColumnData()->EnableMarqueeL(ETrue);
    
    // Add the folders to the list-box.
    MoveToFolderSetItemsL(*listBoxRows);
    aListBox->HandleItemAdditionL();

    // Set the icons
    icons = new (ELeave) CAknIconArray(1);
    CleanupStack::PushL(icons);

    // Add the folder icon.
    AppendIconL(icons, KAknsIIDQgnPropFolderSmall, KAvkonBitmapFile(),
            EMbmAvkonQgn_prop_folder_small, EMbmAvkonQgn_prop_folder_small_mask);

    // Set the icons and cleanup
    aListBox->ItemDrawer()->ColumnData()->SetIconArray(icons);
	CleanupStack::Pop(icons);

    CleanupStack::Pop(aPopupList);
    CleanupStack::Pop(aListBox);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::MoveToFolderSetItemsL
//
// Sets the move-to-folder list-box's values.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::MoveToFolderSetItemsL(CDesCArraySeg& aListBoxRows)
    {
    iTargetFolderList.Reset();
    //0\t\1st Line of text\t2nd Line of text\t0
    TBuf<255>  listBoxtxt;

    // If need be add an entry for the root-level.
    if (iCurrentFolder != iRootFolder)
        {
        TDesC*  text = NULL;

        text = CCoeEnv::Static()->AllocReadResourceLC(R_FEEDS_TEXT_FLDR_ROOT_LEVEL);

        listBoxtxt.Zero();
        listBoxtxt.AppendFormat(_L("0\t%S"), text);
        aListBoxRows.AppendL(listBoxtxt);
        iTargetFolderList.Append(iRootFolder);

        CleanupStack::PopAndDestroy(text);
        }

    if (iRootFolder)
        {
        AddFoldersToListL(*iRootFolder,aListBoxRows);
        }
    }

// Recursive loop to find all folders
void CFeedsFolderContainer::AddFoldersToListL(const CFeedsEntity& aFolder, CDesCArraySeg& aListBoxRows)
    {
    TBuf<255>  listBoxtxt;
    
    for (int i = 0; i < aFolder.GetChildren().Count(); i++)
        {
        const CFeedsEntity*  item = NULL;
        
        item = aFolder.GetChildren()[i];
        
                
        // Skip all feeds and current folder
        if (!(item->GetType()==EFolder) || iCurrentFolder == item)
            {
            if(iCurrentFolder == item)
        	    {
		        AddFoldersToListL(*item,aListBoxRows);// recursive            	
            	}
            continue;
            }

		TPtrC itemName;
		item->GetStringValue(EFolderAttributeTitle,itemName);

        // Set the name.
        TInt len;

        len = itemName.Length();
        if (len > 253)
            {
            len = 253;
            }

        listBoxtxt.Zero();
        listBoxtxt.AppendFormat(_L("0\t%S"), &itemName);
        aListBoxRows.AppendL(listBoxtxt);
        iTargetFolderList.Append(item);// save pointers to items to make selection easier
        
        AddFoldersToListL(*item,aListBoxRows);// recursive
        }
    
    }

// -----------------------------------------------------------------------------
// CFeedsFolderContainer::MoveToFolderMoveL
//
// Moves the marked items into the target-folder.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::MoveToFolderMoveL(const CFeedsEntity& aTargetFolder)
    {
    RPointerArray<const CFeedsEntity>  markedItems(10);
    RPointerArray<const CFeedsEntity>  removedItems(10);
    const RPointerArray<CFeedsEntity>&  folderItems = aTargetFolder.GetChildren();
    const CArrayFix<TInt>*            markedIndexes = NULL;
    const CFeedsEntity*                folder = NULL;

    CleanupClosePushL(markedItems);
    CleanupClosePushL(removedItems);
    // Get the array of marked items.
    markedIndexes = MarkedItems();
    if ((markedIndexes == NULL) || (markedIndexes->Count() == 0))
        {
        iListBox->View()->SelectItemL(CurrentItemIndex());
        markedIndexes = MarkedItems();
        }

    // Copy the marked items into a temp array.
    for (TInt i = 0; i < markedIndexes->Count(); i++)
        {
        folder = iCurrentFolder->GetChildren()[((*markedIndexes)[i])];
        // if the target folder already has some feed
        if(folderItems.Count() > 0)
            {
            TPtrC item;
            TPtrC url;
            folder->GetStringValue(EFeedAttributeTitle, item);
            const CFeedsEntity*  otherItem = iApiProvider.FeedsClientUtilities().Search(item, aTargetFolder);
            if(!(otherItem == CurrentItem())&& otherItem != NULL)
                {
                otherItem->GetStringValue(EFeedAttributeLink, url);
                iApiProvider.FeedsClientUtilities().AddFolderItemL(item,url,EFalse,aTargetFolder,0);
                User::LeaveIfError(removedItems.Append(folder));
                }
            else
                {
                // feed not present in target folder
                User::LeaveIfError(markedItems.Append(folder));
                }
            }
        // if the target folder is empty
        else
            {
            // Move the items.
            User::LeaveIfError(markedItems.Append(folder));
            }
        }
    if(removedItems.Count())
        {
        iApiProvider.FeedsClientUtilities().DeleteFolderItemsL(removedItems);
        }
    if(markedItems.Count())
        {
        // Move the items.
        iApiProvider.FeedsClientUtilities().MoveFolderItemsToL(markedItems, aTargetFolder);
        }
    // Clean up
    CleanupStack::PopAndDestroy(/*removedItems*/);
    CleanupStack::PopAndDestroy(/*markedItems*/);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::PostHandleMoveL
//
// Moves the current or marked items -- called after the move-cmd is confirmed.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::PostHandleMoveL()
    {
    const CArrayFix<TInt>*            markedIndexes = NULL;
    RPointerArray<const CFeedsEntity>  markedItems(10);
    TInt                              targetIndex;

    CleanupClosePushL(markedItems);

    // Get the marked indexes.
    markedIndexes = MarkedItems();

    // Return if there is nothing to do.
    if ((markedIndexes == NULL) || (markedIndexes->Count() == 0))
        {
        return;
        }

    // Get the target index.
    targetIndex = CurrentItemIndex();

    // Copy the marked items into a temp array.
    for (TInt i = 0; i < markedIndexes->Count(); i++)
        {
        User::LeaveIfError(markedItems.Append(
                iCurrentFolder->GetChildren()[((*markedIndexes)[i])]));
        }
    
    // Move the items.
    iApiProvider.FeedsClientUtilities().MoveFolderItemsL(markedItems, targetIndex);
        
    // Clean up
    CleanupStack::PopAndDestroy(/*markedItems*/);
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::UpdateCba
//
// Updates the options softkey to reflect the current state.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::UpdateCbaL()
    {
    TInt  commandSet;

    // Set default menu type and resource    
    iView->MenuBar()->SetMenuTitleResourceId( R_FEEDS_FOLDER_VIEW_MENUBAR);
    iView->MenuBar()->SetMenuType(CEikMenuBar::EMenuOptions);

    if ((MarkedItems() != NULL) && (MarkedItems()->Count() > 0))
        {
        // Change menu configuration to context-sensitive menu if any items are marked
        iView->MenuBar()->SetContextMenuTitleResourceId( R_FEEDS_FOLDER_MENU_BAR_CONTEXT_MENU);
        iView->MenuBar()->SetMenuType(CEikMenuBar::EMenuContext);
        iView->MenuBar()->SetMenuTitleResourceId( R_FEEDS_FOLDER_VIEW_MENUBAR);
        iView->MenuBar()->SetMenuType(CEikMenuBar::EMenuOptions);

        // normally, we use the back rsk unless we launched directly into feeds (in which case, exit)
		if ( !iApiProvider.BrowserWasLaunchedIntoFeeds() )
			{
            commandSet = R_FEEDS_FOLDER_VIEW_CBA_OPTIONS_BACK_CONTEXT_MENU;
			}
        else
            {
            commandSet = R_FEEDS_FOLDER_VIEW_CBA_OPTIONS_EXIT_CONTEXT_MENU;
            }
        }
    else if (iCurrentFolder && (iCurrentFolder->GetChildren().Count() > 0))
        {
        // normally, we use the back rsk unless we launched directly into feeds (in which case, exit)
		if ( !iApiProvider.BrowserWasLaunchedIntoFeeds() )
			{
            commandSet = R_FEEDS_FOLDER_VIEW_CBA_OPTIONS_BACK_OPEN;
			}
	    else
	        {
			if ( iCurrentFolder == iRootFolder )
	        	{
            	commandSet = R_FEEDS_FOLDER_VIEW_CBA_OPTIONS_EXIT_OPEN;
	        	}
	        else
	        	{
            	commandSet = R_FEEDS_FOLDER_VIEW_CBA_OPTIONS_BACK_OPEN;
	        	}
	        }
	        UpdateToolbarButtonsState();
        } 
    else
        {
        // normally, we use the back rsk unless we launched directly into feeds (in which case, exit)
		if ( !iApiProvider.BrowserWasLaunchedIntoFeeds() )
			{
            commandSet = R_FEEDS_FOLDER_VIEW_CBA_OPTIONS_BACK_ADDFEED;
			}
	    else
	        {
            commandSet = R_FEEDS_FOLDER_VIEW_CBA_OPTIONS_EXIT_ADDFEED;
	        }
        }

    if (iMoveActive)
        {
        commandSet = R_AVKON_SOFTKEYS_OK_CANCEL__OK;
        }

    iView->UpdateCbaL(commandSet);
    }        


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::SetCurrentFolder
//
// Changes the current folder.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::SetCurrentFolder(const CFeedsEntity* aFolder, 
        TBool aResetSelection)
    {
    iCurrentFolder = aFolder;
    
    if (aFolder != NULL)
        {
        iCurrentFolderId = aFolder->GetId();
        }
    else
        {        
        iCurrentFolderId = -1;
        }
        
    if (aResetSelection && (iListBox != NULL))
        {
        iListBox->SetCurrentItemIndex(0);
        }
    }

// -----------------------------------------------------------------------------
// CFeedsFolderContainer::CountFolderFolders
//
// Count the number of subfolders within a folder
//
// -----------------------------------------------------------------------------
void CFeedsFolderContainer::CountFolderFolders(const CFeedsEntity* aFolder, TInt& aFolderCount)
    {
    if (aFolder)
        {
        for(TInt index = 0; index < aFolder->GetChildren().Count(); index++)
            {
            if ((aFolder->GetChildren()[index])->GetType() == EFolder)
                {
                aFolderCount++;
                CountFolderFolders(aFolder->GetChildren()[index], aFolderCount);
                }
            }
        }
    }
    
// -----------------------------------------------------------------------------
// CFeedsFolderContainer::UnreadCountChangedL
//
// Calculate the unread count based on delta, return the calculated value.
// -----------------------------------------------------------------------------
//
TInt CFeedsFolderContainer::UnreadCountChangedL( TInt aUnreadCountDelta )
    {
    TInt     unreadCount = 0;
    TInt                index;
    CFeedsEntity*  item = NULL;

    index = iListBox->CurrentItemIndex();
    item = const_cast<CFeedsEntity*>(iCurrentFolder->GetChildren()[index]);
    User::LeaveIfNull((void*) item);

    // If it's a feed - update its unread count.
    if (item->GetType() != EFolder)
        {
        // calculate how many are unread for the current feed
        item->GetIntegerValue(EFolderAttributeUnreadCount, unreadCount );
        unreadCount += aUnreadCountDelta;
        if (unreadCount < 0)
            {
            unreadCount = 0;
            }
        CFeedsMap* map = CFeedsMap::NewL();
        map->SetIntegerAttribute(EFolderAttributeUnreadCount,unreadCount);
   
      //  item->ChangeValue(*map);
        delete map;
        }
    return unreadCount;
    }


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::SetUnreadCountL
//
// Set the unread count.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::SetUnreadCountL( TInt aUnreadCount )
{
    TInt                index;
    CFeedsEntity*  item = NULL;

    index = iListBox->CurrentItemIndex();
    item = const_cast<CFeedsEntity*>(iCurrentFolder->GetChildren()[index]);
    User::LeaveIfNull((void*) item);

    // If it's a feed - update its unread count.
    if (item->GetType() == EFalse)
        {
        CFeedsMap* map = CFeedsMap::NewL();
        map->SetIntegerAttribute(EFolderAttributeUnreadCount,aUnreadCount);
   
       // item->ChangeValue(*map);
        delete map;
        }
}


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::DynInitMarkUnmarkMenuPane
//
// Dynamically Initialize the Mark/Unmark sub-menu
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::DynInitMarkUnmarkMenuPane( CEikMenuPane *aMenuPane )
{
    AknSelectionService::HandleMarkableListDynInitMenuPane(R_AVKON_MENUPANE_MARKABLE_LIST_IMPLEMENTATION, aMenuPane, iListBox);
}


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::DimToolbar
//
// Dim or UnDim Toolbar
// if aDim is ETrue, then dim toolbar
// else undim toolbar
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::DimToolbar(TBool aDim)
{
    iView->Toolbar()->SetDimmed(aDim);
    iView->Toolbar()->DrawNow();
}


// -----------------------------------------------------------------------------
// CFeedsFolderContainer::IsMarkedItemFolder
//
// To Check Marked items contains Folder or Feed
// -----------------------------------------------------------------------------
//
TBool CFeedsFolderContainer::IsMarkedItemFolder()
{
    const CArrayFix<TInt>*  markedIndexes = NULL;

    // Get the marked items.
    markedIndexes = MarkedItems();
    
    // Check for Folder is marked or not
    for(TInt i=0 ; i< markedIndexes->Count(); i++)
    {
        if(iCurrentFolder->GetChildren()[(*markedIndexes)[i]] -> GetType() == EFolder)
        {
            return ETrue;
        }
        
    }
    return EFalse;
}

// -----------------------------------------------------------------------------
// CFeedsFolderContainer::ClearNavigationPane
//
// Clears the navigation pane.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::ClearNavigationPane()
    {
    if (iNaviDecorator)
        {
        delete iNaviDecorator;
        iNaviDecorator = NULL;
        }
    }

// -----------------------------------------------------------------------------
// CFeedsFolderContainer::UpdateNavigationPaneL
//
// Handles the changes needed to the Navigation Pane.
// -----------------------------------------------------------------------------
//
void CFeedsFolderContainer::UpdateNavigationPaneL()
    {
    CAknNavigationControlContainer*  naviPane = NULL;

    // Get the navigation sub-pane.
    CAknViewAppUi*  appUi;
    TUid            uid;
    TBuf<1>  string;

    string.Append(_L(" "));

    // Get the title sub-pane.
    appUi = static_cast<CAknViewAppUi*>(CCoeEnv::Static()->AppUi());

    uid.iUid = EEikStatusPaneUidNavi;

    CEikStatusPaneBase::TPaneCapabilities subPane = appUi->StatusPane()->
            PaneCapabilities(uid);

    // Set the title if the pane belongs to the app.
    if (subPane.IsPresent() && subPane.IsAppOwned())
        {
        naviPane = (CAknNavigationControlContainer*) appUi->StatusPane()->ControlL(uid);
        }
    else
        {
        User::Leave(KErrNotSupported);
        }

    // Ensure the NaviDecorator was created.
    if (!iNaviDecorator)
        {
        iNaviDecorator = naviPane->CreateNavigationLabelL( string );
        }


    // If not yet pushed, this will do the push; if already there, this brings
    // it to top and draws.
    naviPane->PushL(*iNaviDecorator);
    }

