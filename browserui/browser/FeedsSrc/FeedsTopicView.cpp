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
* Description:  A view to browse a feed's topics.
*
*/



#include <aknviewappui.h>
#include <aknutils.h>
#include <AknToolbar.h>
#include <eikbtgpc.h>
#include <eikmenup.h>
#include <hlplch.h>

#include "Browser.hrh"
#include <BrowserNG.rsg>
#include <feedattributes.h>
#include <folderattributes.h>
#include "BrowserAppUi.h"
#include "CommonConstants.h"

#include "FeedsTopicView.h"
#include "FeedsTopicContainer.h"


// -----------------------------------------------------------------------------
// CFeedsTopicView::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeedsTopicView* CFeedsTopicView::NewL(	MApiProvider& aApiProvider,	TRect& aRect )
	{
    CFeedsTopicView* self = new (ELeave) CFeedsTopicView(aApiProvider);
    
    CleanupStack::PushL(self);
    self->ConstructL(aRect);
    CleanupStack::Pop();

    return self;
	}


// -----------------------------------------------------------------------------
// CFeedsTopicView::CFeedsTopicView
//
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CFeedsTopicView::CFeedsTopicView(MApiProvider& aApiProvider):
	 CBrowserViewBase( aApiProvider ),iPenEnabled(EFalse)
	{
    iPenEnabled = AknLayoutUtils::PenEnabled();
	}


// -----------------------------------------------------------------------------
// CFeedsTopicView::ConstructL
//
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFeedsTopicView::ConstructL(TRect& aRect)
    {
    BaseConstructL(R_FEEDS_TOPIC_VIEW);
    
    iContainer = CFeedsTopicContainer::NewL( this, ApiProvider(), aRect );
    iContainer->MakeVisible(EFalse);
        
    if(iPenEnabled)
        {
        Toolbar()->SetToolbarObserver(this);;        
        }
    }


// -----------------------------------------------------------------------------
// CFeedsTopicView::~CFeedsTopicView
//
// Deconstructor.
// -----------------------------------------------------------------------------
//
CFeedsTopicView::~CFeedsTopicView()
    {
    DoDeactivate();

    delete iContainer;
    }

// -----------------------------------------------------------------------------
// CFeedsTopicView::Id
//
// Returns the id of the view.
// -----------------------------------------------------------------------------
//
TUid CFeedsTopicView::Id() const
    {
    return KUidBrowserFeedsTopicViewId;
    }

// -----------------------------------------------------------------------------
// CFeedsTopicView::HandleCommandL
//
// Processes commands.
// -----------------------------------------------------------------------------
//
void CFeedsTopicView::HandleCommandL(TInt aCommand)
    {   
    TBool  handled = EFalse;
        
    // Handle softkeys and other Symbian commands first.
    switch (aCommand)
        {
		case EAknSoftkeyBack:
		    iApiProvider.FeedsClientUtilities().ShowFolderViewLocalL();
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
        
    // Handle menu commands.
    switch (aCommand)
        {
		case EFeedsOpen:
    		if ((iApiProvider.FeedsClientUtilities().CurrentFeed()->GetChildren().Count()) > 0)
        		{
                iContainer->HandleOpenL();     
        		}
            break;

		case EFeedsUpdate:
		    iContainer->HandleUpdateFeedL();
            break;

		case EFeedsBackToPage:
            iApiProvider.FeedsClientUtilities().HandleCommandL(Id(), EWmlCmdBackToPage );
			break;

        default:
            iApiProvider.FeedsClientUtilities().HandleCommandL(Id(),aCommand);
            break;
        }
    }


// -----------------------------------------------------------------------------
// CFeedsTopicView::DoActivateL
//
// Called when the view is activated.
// -----------------------------------------------------------------------------
//
void CFeedsTopicView::DoActivateL(const TVwsViewId& /*aPrevViewId*/, 
        TUid /*aCustomMessageId*/, const TDesC8& /*aCustomMessage*/)
    {

    // If need be, add the container to the control stack.
    if (!iContainerOnStack)
        {
        AppUi()->AddToViewStackL(*this, iContainer);
        iContainer->SetRect(ClientRect());
        iContainer->MakeVisible(ETrue);
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
// CFeedsTopicView::DoDeactivate
//
// Called when the view is deactivated.
// -----------------------------------------------------------------------------
//
void CFeedsTopicView::DoDeactivate()
    {
    if (iContainerOnStack)
        {
        AppUi()->RemoveFromViewStack(*this, iContainer);
        iContainer->MakeVisible(EFalse);
        iContainer->ClearNavigationPane();
        iContainerOnStack = EFalse;

        iInitialItem = iContainer->CurrentIndex();
        }
	}


// -----------------------------------------------------------------------------
// CFeedsTopicView::DynInitMenuPaneL
//
// Disables unrelated menu options.
// -----------------------------------------------------------------------------
//
void CFeedsTopicView::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {


    if (aResourceId == R_FEEDS_TOPIC_VIEW_MENU)
        {
        // Refresh (same as update?)
        iApiProvider.FeedsClientUtilities().AddItemL(*aMenuPane, EFeedsUpdate, R_FEEDS_UPDATE);

        // Back to Page (if page loaded)
        if ( iApiProvider.IsPageLoaded() )
            {
            iApiProvider.FeedsClientUtilities().AddItemL( *aMenuPane, EWmlCmdBackToPage, R_BROWSER_MENU_ITEM_BACK_TO_PAGE );
            }
        }      
        
    iApiProvider.FeedsClientUtilities().DynInitMenuPaneL(aResourceId, aMenuPane );
    }


// -----------------------------------------------------------------------------
// CFeedsTopicView::SetCurrentFeedL
//
// Sets the current feed
// -----------------------------------------------------------------------------
//
void CFeedsTopicView::SetCurrentFeedL(const CFeedsEntity& aFeed, TInt aIndex)
    {
    iContainer->SetCurrentFeedL(aFeed, aIndex);
    }
    
    
// -----------------------------------------------------------------------------
// CFeedsTopicView::SetInitialItem
//
// Sets the initial item once the view is activated.
// -----------------------------------------------------------------------------
//
void CFeedsTopicView::SetInitialItem(TInt aItemIndex)
    {
    iInitialItem = aItemIndex;
    }

// -----------------------------------------------------------------------------
// CFeedsTopicView::UpdateToolbarButtonsState
//
// Updates the state of the toolbar buttons depending on the situation
// -----------------------------------------------------------------------------
//

void CFeedsTopicView::UpdateToolbarButtonsState()
    {
    if (iApiProvider.IsPageLoaded())
        {
        Toolbar()->SetItemDimmed(EFeedsBackToPage, EFalse, ETrue);     
        }
    else
        {
        Toolbar()->SetItemDimmed(EFeedsBackToPage, ETrue, ETrue);            
        }
    if ((iApiProvider.FeedsClientUtilities().CurrentFeed()->GetChildren().Count()) > 0)
        {
        Toolbar()->SetItemDimmed(EFeedsOpen, EFalse, ETrue);     
        }
    else
        {
        Toolbar()->SetItemDimmed(EFeedsOpen, ETrue, ETrue);            
        }
    }

// ---------------------------------------------------------------------------
// CFeedsTopicView::CommandSetResourceIdL
// ---------------------------------------------------------------------------
TInt CFeedsTopicView::CommandSetResourceIdL()
    {
    // It is never called, but this function have to be implemented
    return KWmlEmptyResourceId;
    }
    
// ---------------------------------------------------------
// CFeedsTopicView::HandleClientRectChange
// ---------------------------------------------------------
//
void CFeedsTopicView::HandleClientRectChange()
	{
	if( iContainer )
	    {
        iContainer->SetRect( ClientRect() );
        }
	}
