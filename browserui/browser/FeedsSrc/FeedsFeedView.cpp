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
* Description:  A view to browse a given feed.
*
*/


#include <aknviewappui.h>
#include <aknutils.h>
#include <AknToolbar.h>
#include <brctlinterface.h>
#include <eikbtgpc.h>
#include <eikmenup.h>
#include <hlplch.h>

#include "BrowserAppUi.h"
#include "CommonConstants.h"
#include "Browser.hrh"
#include <BrowserNG.rsg>
#include "BrowserUtil.h"
#include <feedattributes.h>
#include <folderattributes.h>
#include "FeedsFeedContainer.h"
#include "FeedsFeedView.h"

#include "Logger.h"

// -----------------------------------------------------------------------------
// CFeedsFeedView::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeedsFeedView* CFeedsFeedView::NewL( MApiProvider& aApiProvider, TRect& aRect )

	{
    CFeedsFeedView* self = new (ELeave) CFeedsFeedView(aApiProvider);
    
    CleanupStack::PushL(self);
    self->ConstructL(aRect);
    CleanupStack::Pop();

    return self;
	}


// -----------------------------------------------------------------------------
// CFeedsFeedView::CFeedsFeedView
//
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CFeedsFeedView::CFeedsFeedView(MApiProvider& aApiProvider)
: CBrowserViewBase( aApiProvider ),iPageScalerEnabled(EFalse),iPenEnabled(EFalse)
	{
    iPenEnabled = AknLayoutUtils::PenEnabled(); 
	}


// -----------------------------------------------------------------------------
// CFeedsFeedView::ConstructL
//
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFeedsFeedView::ConstructL(TRect& aRect)
    {
    // this is here in case content view wasn't activated yet on browser startup.
    iApiProvider.SetViewToBeActivatedIfNeededL( KUidBrowserContentViewId );
            
    BaseConstructL(R_FEEDS_FEED_VIEW);

	iContainer = CFeedsFeedContainer::NewL( this, ApiProvider() );
	iContainer->SetRect( aRect );
    iOrigRect.iTl = aRect.iTl;
    iOrigRect.iBr = aRect.iBr;
    iContainer->MakeVisible(EFalse);
    
    if(iPenEnabled)
        {
        Toolbar()->SetToolbarObserver(this);;        
        }    
    }


// -----------------------------------------------------------------------------
// CFeedsFeedView::~CFeedsFeedView
//
// Deconstructor.
// -----------------------------------------------------------------------------
//
CFeedsFeedView::~CFeedsFeedView()
    {
    DoDeactivate();

    delete iContainer;
    }

// -----------------------------------------------------------------------------
// CFeedsFeedView::Id
//
// Returns the id of the view.
// -----------------------------------------------------------------------------
//
TUid CFeedsFeedView::Id() const
    {
    return KUidBrowserFeedsFeedViewId;
    }

// -----------------------------------------------------------------------------
// CFeedsFeedView::HandleCommandL
//
// Processes commands.
// -----------------------------------------------------------------------------
//
void CFeedsFeedView::HandleCommandL(TInt aCommand)
    {   
    TBool  handled = EFalse;
    
    // Handle softkeys and other Symbian commands first.
    switch (aCommand)
        {
		case EAknSoftkeyBack:
			iApiProvider.FeedsClientUtilities().ShowTopicViewLocalL(iContainer->CurrentItem());
            handled = ETrue;
            break;
            
        case EAknSoftkeyNext:
            iContainer->ShowNextItemL();
            handled = ETrue;
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
        }
        
    if (handled)
        {
        return;
        }
     
    switch (aCommand)
        {
        case EFeedsBack:
		    iApiProvider.FeedsClientUtilities().ShowTopicViewLocalL(iContainer->CurrentItem());
            break;
            
        case EFeedsShowNextItem:
            iContainer->ShowNextItemL();
            break;

        case EFeedsShowPrevItem:
            iContainer->ShowPrevItemL();
            break;

		case EFeedsSeeFullStory:
            iContainer->iBrowserControl->HandleCommandL((TBrCtlDefs::ECommandIdBase + TBrCtlDefs::ECommandOpen));
            break;

        default:
            iApiProvider.FeedsClientUtilities().HandleCommandL(Id(),aCommand);
            break;
        }
    }


// -----------------------------------------------------------------------------
// CFeedsFeedView::DoActivateL
//
// Called when the view is activated.
// -----------------------------------------------------------------------------
//
void CFeedsFeedView::DoActivateL(const TVwsViewId& /*aPrevViewId*/, 
        TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
    {
    // fix bug RFON-7FJS2Z: need to activate status pane going back from full story page to feeds view
   	StatusPane()->SwitchLayoutL( R_AVKON_STATUS_PANE_LAYOUT_USUAL );
  	StatusPane()->MakeVisible( ETrue );
    //Take Button group pointer
    CEikButtonGroupContainer* pButtonGroupContainer = CEikButtonGroupContainer::Current();
    if (!iContainerOnStack)
        {
        AppUi()->AddToViewStackL(*this, iContainer);        
        //Make button group invisible in order to let container have correct Rect.
        //Patch is based on comments received from AVKON
        if(pButtonGroupContainer) pButtonGroupContainer->MakeVisible(EFalse);
        iContainer->SetRect(iOrigRect);
        if(iContainer->iBrowserControl->BrowserSettingL(TBrCtlDefs::ESettingsPageOverview))
            {
            TRAP_IGNORE(iContainer->iBrowserControl->SetBrowserSettingL(TBrCtlDefs::ESettingsPageOverview, EFalse));
            iPageScalerEnabled = ETrue;
            }
        TRAP_IGNORE(iContainer->iBrowserControl->HandleCommandL( 
                (TInt)TBrCtlDefs::ECommandIdBase + (TInt)TBrCtlDefs::ECommandGainFocus));
        
        iContainer->MakeVisible(ETrue);        
        //Again make Button group visible
        if(pButtonGroupContainer) pButtonGroupContainer->MakeVisible(ETrue);
        iContainerOnStack = ETrue;
        // resize screen after calling SetRect.  This way looks better
        iContainer->HandleResourceChange(KEikDynamicLayoutVariantSwitch);
        }

    // Set the current feed.
    iContainer->SetCurrentFeedL(*(iApiProvider.FeedsClientUtilities().CurrentFeed()), iInitialItem);
    
    iApiProvider.SetLastActiveViewId(Id());
    UpdateToolbarButtonsState();
    }


// -----------------------------------------------------------------------------
// CFeedsFeedView::DoDeactivate
//
// Called when the view is deactivated.
// -----------------------------------------------------------------------------
//
void CFeedsFeedView::DoDeactivate()
    {
    if (iContainerOnStack)
        {
        iContainer->ClearNavigationPane();        
        
        if(iPageScalerEnabled)
            {
            TRAP_IGNORE(iContainer->iBrowserControl->SetBrowserSettingL(TBrCtlDefs::ESettingsPageOverview, ETrue));
            iPageScalerEnabled = EFalse;
            }
        TRAP_IGNORE(iContainer->iBrowserControl->HandleCommandL( 
                (TInt)TBrCtlDefs::ECommandIdBase + (TInt)TBrCtlDefs::ECommandLoseFocus));
        
        AppUi()->RemoveFromViewStack(*this, iContainer);
        iContainer->MakeVisible(EFalse);
        
        iInitialItem = iContainer->CurrentItem();
        iContainerOnStack = EFalse;
        
        // when exit from feeds view, viewMgr might be deleted already
        if ( !ApiProvider().ExitInProgress() )
            {
            TRAP_IGNORE(iApiProvider.FeedsClientUtilities().ItemStatusWriteToServerL()); 
            }
        }
	}


// -----------------------------------------------------------------------------
// CFeedsFeedView::DynInitMenuPaneL
//
// Disables unrelated menu options.
// -----------------------------------------------------------------------------
//
void CFeedsFeedView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    __ASSERT_DEBUG( (aMenuPane != NULL), Util::Panic( Util::EUninitializedData ));
    
    // Handle the main menu.
    if (aResourceId == R_FEEDS_FEED_VIEW_MENU)
        {
        // Back to Page (if page loaded)
        if ( iApiProvider.IsPageLoaded() )
            {
            iApiProvider.FeedsClientUtilities().AddItemL( *aMenuPane, EWmlCmdBackToPage, R_BROWSER_MENU_ITEM_BACK_TO_PAGE );
            }
        }
        
    iApiProvider.FeedsClientUtilities().DynInitMenuPaneL(aResourceId, aMenuPane );
    }

// -----------------------------------------------------------------------------
// CFeedsFeedView::SetInitialItem
//
// Sets the initial item once the view is activated.
// -----------------------------------------------------------------------------
//
void CFeedsFeedView::SetInitialItem(TInt aItemIndex)
    {
    iInitialItem = aItemIndex;
    }

// -----------------------------------------------------------------------------
// CFeedsTopicView::UpdateToolbarButtonsState
//
// Updates the state of the toolbar buttons depending on the situation
// -----------------------------------------------------------------------------
//

void CFeedsFeedView::UpdateToolbarButtonsState()
    {
    if ( iContainer->ItemCount()  > 1 )
        {
        Toolbar()->SetItemDimmed(EFeedsShowPrevItem, EFalse, ETrue);
        Toolbar()->SetItemDimmed(EFeedsShowNextItem, EFalse, ETrue);     
        }
    else
        {
        Toolbar()->SetItemDimmed(EFeedsShowPrevItem, ETrue, ETrue);
        Toolbar()->SetItemDimmed(EFeedsShowNextItem, ETrue, ETrue);		
        }
    }
    
// ---------------------------------------------------------------------------
// CFeedsFeedView::CommandSetResourceIdL
// ---------------------------------------------------------------------------
TInt CFeedsFeedView::CommandSetResourceIdL()
    {
    // It is never called, but this function have to be implemented
    return KWmlEmptyResourceId;
    }
    
// ---------------------------------------------------------
// CFeedsFeedView::HandleClientRectChange
// ---------------------------------------------------------
//
void CFeedsFeedView::HandleClientRectChange()
    {
    if( iContainer )
        {
        iContainer->MakeVisible(EFalse);
        iContainer->SetRect(ClientRect());
        iContainer->MakeVisible(ETrue);        
        }
    }

// -----------------------------------------------------------------------------
// CFeedsFeedView::DisconnectL
// -----------------------------------------------------------------------------
//
void CFeedsFeedView::DisconnectL()
    {
    if( iContainer )
        {
        iContainer->iBrowserControl->HandleCommandL( TBrCtlDefs::ECommandDisconnect + TBrCtlDefs::ECommandIdBase );
        }
    }
