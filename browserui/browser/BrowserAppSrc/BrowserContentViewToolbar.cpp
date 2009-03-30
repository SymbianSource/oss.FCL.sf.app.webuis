/*
* Copyright (c) 2007-2008 Nokia Corporation and/or its subsidiary(-ies).
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
*      Helper class that handles the toolbar
*      
*
*/


// INCLUDE FILES

#include "BrowserContentView.h"
#include "BrowserContentViewToolbar.h"
#include "BrowserContentViewContainer.h"
#include "BrowserWindowManager.h"
#include "BrowserWindow.h"
#include "ApiProvider.h"
#include "Preferences.h"
#include <AknToolbar.h>
#ifdef RD_SCALABLE_UI_V2 
#include <AknToolbarExtension.h>
#include "BrowserUiVariant.hrh"
#endif
#include <aknutils.h>
#include <BrctlInterface.h>
#include "BrowserAppUi.h"
#include <gulicon.h>
#include <browser.mbg>
#include <data_caging_path_literals.hrh> 
#include <aknsutils.h> 

// ================= MEMBER FUNCTIONS =========================================

// ----------------------------------------------------------------------------
// CBrowserContentViewToolbar::CBrowserContentViewToolbar
// ----------------------------------------------------------------------------
//
CBrowserContentViewToolbar::CBrowserContentViewToolbar(CBrowserContentView* aBrowserContentView)
    {
    iBrowserContentView = aBrowserContentView;
    }

// ----------------------------------------------------------------------------
// CBrowserContentViewToolbar::ConstructL
// ----------------------------------------------------------------------------
//
void CBrowserContentViewToolbar::ConstructL()
    {
#ifdef RD_SCALABLE_UI_V2
    // Do not move this code back to UpdateButtonsStateL.
    // Avkon does not allow hiding buttons when the extended toolbar is visible.
    TBool embeddedMode = iBrowserContentView->ApiProvider().IsEmbeddedModeOn();
    TBool embeddedOperatorMenu = CBrowserAppUi::Static()->IsEmbeddedInOperatorMenu();
    iBrowserContentView->Toolbar()->ToolbarExtension()->HideItemL( EWmlCmdShowSubscribeList, embeddedMode );
    iBrowserContentView->Toolbar()->ToolbarExtension()->HideItemL( EWmlCmdOpenFeedsFolder, embeddedMode );
    iBrowserContentView->Toolbar()->ToolbarExtension()->HideItemL( EWmlCmdFavourites, (embeddedMode || embeddedOperatorMenu) );
    iBrowserContentView->Toolbar()->ToolbarExtension()->HideItemL( EWmlCmdSaveAsBookmark, embeddedOperatorMenu );
    iBrowserContentView->Toolbar()->ToolbarExtension()->HideItemL( EWmlCmdLaunchHomePage, (embeddedMode || embeddedOperatorMenu) );
#endif
    }

// ----------------------------------------------------------------------------
// CBrowserContentViewToolbar::NewL
// ----------------------------------------------------------------------------
//
CBrowserContentViewToolbar* CBrowserContentViewToolbar::NewL(CBrowserContentView* aBrowserContentView)
    {
    CBrowserContentViewToolbar *self = new CBrowserContentViewToolbar(aBrowserContentView);
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------------------------
// CBrowserContentViewToolbar::~CBrowserContentViewToolbar
// ----------------------------------------------------------------------------
//
CBrowserContentViewToolbar::~CBrowserContentViewToolbar()
    {
    }

// ----------------------------------------------------------------------------
// CBrowserContentViewToolbar::UpdateButtonsStateBasicL
// ----------------------------------------------------------------------------
//
void CBrowserContentViewToolbar::UpdateButtonsStateL()
    {
    TBool historyView = iBrowserContentView->IsHistoryViewUp();
    TBool findKeywordPane = iBrowserContentView->Container()->FindKeywordPane()->IsVisible();
    TBool pageOverview = iBrowserContentView->IsMiniatureViewUp();
    TBool zoomSlider = iBrowserContentView->ZoomSliderVisible();
    TBool contentDisplayed = iBrowserContentView->ApiProvider().ContentDisplayed();
    TBool wmlMode = iBrowserContentView->ApiProvider().WindowMgr().CurrentWindow()->WMLMode();
    TBool DimHomePage = iBrowserContentView->ApiProvider().IsLaunchHomePageDimmedL();

    
    // Extended Toolbar
    iBrowserContentView->Toolbar()->SetItemDimmed( EWmlCmdToolbarExtensionContentView, 
            (pageOverview), ETrue );
    iBrowserContentView->Toolbar()->HideItem( EWmlCmdToolbarExtensionContentView, 
            (historyView || findKeywordPane), ETrue );
    // Goto Url
    iBrowserContentView->Toolbar()->SetItemDimmed( EWmlCmdGoToAddress, 
            (pageOverview), ETrue );
    iBrowserContentView->Toolbar()->HideItem( EWmlCmdGoToAddress, 
            (historyView || findKeywordPane), ETrue );

    // Zoom Slider button
    iBrowserContentView->Toolbar()->SetItemDimmed( EWmlCmdZoomSliderShow, 
                ( !contentDisplayed || pageOverview || wmlMode ), ETrue );

    // History buttons
    iBrowserContentView->Toolbar()->HideItem( EWmlCmdHistoryBack, (!historyView), ETrue );
    iBrowserContentView->Toolbar()->HideItem( EWmlCmdHistoryForward, (!historyView), ETrue );
    
    if ( historyView )
        {
           iBrowserContentView->Toolbar()->SetItemDimmed( EWmlCmdHistoryForward, EFalse, ETrue );                        
           iBrowserContentView->Toolbar()->SetItemDimmed( EWmlCmdHistoryBack, EFalse, ETrue );                            
        if(iBrowserContentView->IsHistoryAtEnd())
            {
            iBrowserContentView->Toolbar()->SetItemDimmed( EWmlCmdHistoryForward, ETrue, ETrue );            
            }
        if (iBrowserContentView->IsHistoryAtBeginning())
            {
            iBrowserContentView->Toolbar()->SetItemDimmed( EWmlCmdHistoryBack, ETrue, ETrue );        
            }        
        }
    
    // Find keyword buttons     
    iBrowserContentView->Toolbar()->HideItem( EWmlCmdFindNext, (!findKeywordPane), ETrue ); 
    iBrowserContentView->Toolbar()->HideItem( EWmlCmdFindPrevious, (!findKeywordPane), ETrue );   

    iBrowserContentView->Toolbar()->SetItemDimmed( EWmlNoCmd, ETrue, ETrue );
    iBrowserContentView->Toolbar()->HideItem( EWmlNoCmd, (!(historyView || findKeywordPane)), ETrue );

    // Extended Toolbar
#ifdef RD_SCALABLE_UI_V2
    CBrCtlInterface& brctl = iBrowserContentView->ApiProvider().BrCtlInterface();
    TBool subscribeToItems(EFalse);
    TRAPD(err, const RPointerArray<TBrCtlSubscribeTo>& items = brctl.SubscribeToMenuItemsL();
                subscribeToItems = (err == KErrNone && items.Count() > 0));
    TBool noMultiWin = ( !iBrowserContentView->ApiProvider().Preferences().UiLocalFeatureSupported( KBrowserMultipleWindows ) || 
    	    iBrowserContentView->ApiProvider().IsEmbeddedModeOn() || 
    	    iBrowserContentView->ApiProvider().WindowMgr().WindowCount() < 2);
              
    iBrowserContentView->Toolbar()->ToolbarExtension()->SetItemDimmed( EWmlCmdFindKeyword, wmlMode );
    iBrowserContentView->Toolbar()->ToolbarExtension()->SetItemDimmed( EWmlCmdShowMiniature, wmlMode );
    iBrowserContentView->Toolbar()->ToolbarExtension()->SetItemDimmed( EWmlCmdShowSubscribeList, 
        (!subscribeToItems || wmlMode) );
    iBrowserContentView->Toolbar()->ToolbarExtension()->SetItemDimmed( EWmlCmdSwitchWindow, noMultiWin );
    iBrowserContentView->Toolbar()->ToolbarExtension()->SetItemDimmed( EWmlCmdLaunchHomePage, DimHomePage );
#endif
    }
  
//  End of File
