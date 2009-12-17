/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
*      Implementation of CBrowserWindowSelectionView.
*
*
*/


// INCLUDE FILES
#include <aknviewappui.h>
#include <avkon.hrh>
#include <BrowserNG.rsg>
#include <FeatMgr.h>
#include <avkon.RSG>
#include <eikbtgpc.h>
#include <akntabgrp.h>
#include <aknnavi.h>
#include <brctlinterface.h>
#include <brctldefs.h>
#include "BrowserAppUi.h"
#include "CommonConstants.h"
#include "BrowserWindowSelectionView.h"
#include "BrowserWindowSelectionContainer.h" 
#include "Preferences.h"
#include "Display.h"
#include "BrowserWindowManager.h"
#include "BrowserWindow.h"

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------------------------
// CBrowserWindowSelectionView::NewLC
// ----------------------------------------------------------------------------
//
CBrowserWindowSelectionView* CBrowserWindowSelectionView::NewLC( MApiProvider& aApiProvider )
    {
    CBrowserWindowSelectionView* view = new (ELeave) CBrowserWindowSelectionView( aApiProvider );
    CleanupStack::PushL( view );
    view->ConstructL( );    
    return view;
    }
    
// ---------------------------------------------------------
// CBrowserWindowSelectionView::CBrowserWindowSelectionView
// ---------------------------------------------------------
//
CBrowserWindowSelectionView::CBrowserWindowSelectionView( MApiProvider& aApiProvider ):
CBrowserViewBase( aApiProvider )
    {
    }
        
// ---------------------------------------------------------
// CBrowserWindowSelectionView::ConstructL(const TRect& aRect)
// EPOC two-phased constructor
// ---------------------------------------------------------
//
void CBrowserWindowSelectionView::ConstructL()
    {
    BaseConstructL( R_BROWSERWINDOWSELECTION_VIEW );    
    iTitles = new (ELeave) CArrayFixFlat<HBufC*>( 2 );
    iWindowIds = new (ELeave) CArrayFixFlat<TInt>( 2 );
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionView::~CBrowserWindowSelectionView()
// destructor
// ---------------------------------------------------------
//
CBrowserWindowSelectionView::~CBrowserWindowSelectionView()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }

    delete iContainer;
    delete iWindowInfo;
    delete iTitles;
    delete iWindowIds;
    }

// ---------------------------------------------------------
// TUid CBrowserWindowSelectionView::Id()
//
// ---------------------------------------------------------
//
TUid CBrowserWindowSelectionView::Id() const
    {
    return KUidBrowserWindowSelectionViewId;
    }
    
// ---------------------------------------------------------------------------
// CBrowserWindowSelectionView::CommandSetResourceIdL
// ---------------------------------------------------------------------------
TInt CBrowserWindowSelectionView::CommandSetResourceIdL()
    {
    TInt commandSet = 0; // R_AVKON_SOFTKEYS_EMPTY;
    return commandSet;
    }  
    
// ---------------------------------------------------------
// CBrowserWindowSelectionView::HandleCommandL()
// ---------------------------------------------------------
//
void CBrowserWindowSelectionView::HandleCommandL(TInt aCommand)
    {   
    switch ( aCommand )
        {
        case EAknCmdOpen:
            {
            TInt windowId = GetWindowIdFromTabIndex( iContainer->GetTabGroup()->ActiveTabIndex() );
            ApiProvider().WindowMgr().SwitchWindowL( windowId );
            //If the user switches window we reset the CalledFromAnotherApp status
        	if(ApiProvider().CalledFromAnotherApp())
            	{
            	ApiProvider().SetCalledFromAnotherApp(EFalse);
            	}
            ApiProvider().SetViewToBeActivatedIfNeededL( GetPreviousViewID() );
            break;                
            }
        case EWmlCmdCloseWindow:
            {            
            TInt windowId = GetWindowIdFromTabIndex( iContainer->GetTabGroup()->ActiveTabIndex() );
            TInt newWindowId = ApiProvider().WindowMgr().DeleteWindowL( windowId, EFalse );
            
            // Check if there is only one remaining window,
            // after closing the window.
            delete iWindowInfo;
            iWindowInfo = NULL;
            iWindowInfo = ApiProvider().WindowMgr().GetWindowInfoL( this );
            if ( iWindowInfo->Count() < 2 )
                {
                ApiProvider().SetViewToBeActivatedIfNeededL( KUidBrowserContentViewId );
                ApiProvider().WindowMgr().SwitchWindowL( newWindowId );                
                }
            break;                
            }            
        case EWmlCmdCloseAllWindowButThis:
            {
            iWindowEventsDisabled = ETrue;
            // Get current window id.
            TInt windowId = GetWindowIdFromTabIndex( iContainer->GetTabGroup()->ActiveTabIndex() );
            // Get all windows id.
            delete iWindowInfo;
            iWindowInfo = NULL;
            iWindowInfo = ApiProvider().WindowMgr().GetWindowInfoL( this );
            
            // Delete all windows except this.            
            for ( TInt i = 0; i < iWindowInfo->Count(); i++ )
                {                
                if ( (*iWindowInfo)[i]->iWindowId != windowId )
                    {
                    ApiProvider().WindowMgr().DeleteWindowL( (*iWindowInfo)[i]->iWindowId, ETrue );                    
                    }                    
                }
            // Activate curent window.
            ApiProvider().SetViewToBeActivatedIfNeededL( KUidBrowserContentViewId );
            ApiProvider().WindowMgr().SwitchWindowL( windowId );
            iWindowEventsDisabled = EFalse;            
            break;                
            }            
        case EWmlCmdWindowSelectionCancel:
            {
            ApiProvider().SetViewToBeActivatedIfNeededL( KUidBrowserContentViewId );
            break;
            }
        default:
            {
            break;
            }
        }
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CBrowserWindowSelectionView::HandleClientRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionView::DoActivateL(...)
// 
// ---------------------------------------------------------
//
void CBrowserWindowSelectionView::DoActivateL( const TVwsViewId& aPrevViewId,TUid /*aCustomMessageId*/,
                                               const TDesC8& /*aCustomMessage*/)
    {    
    StatusPane()->SwitchLayoutL( R_AVKON_STATUS_PANE_LAYOUT_USUAL );
    StatusPane()->MakeVisible( ETrue );

    iPreviousViewID = ApiProvider().LastActiveViewId( );
    ApiProvider().SetLastActiveViewId( Id() );
    iLastViewId = aPrevViewId;
    if (!iContainer)
        {
        iContainer = new (ELeave) CBrowserWindowSelectionContainer( this );
        iContainer->SetMopParent(this);
        iContainer->ConstructL( ClientRect() );
        AppUi()->AddToStackL( *this, iContainer );
        UpdateTabsL();
        }
    ApiProvider().WindowMgr().AddObserverL( this );        
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionView::DoDeactivate()
// ---------------------------------------------------------
//
void CBrowserWindowSelectionView::DoDeactivate()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }
    
    delete iContainer;
    iContainer = NULL;
    ApiProvider().WindowMgr().RemoveObserver( this );    
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionView::CreateWindowInfoLC()
// ---------------------------------------------------------
//    
HBufC* CBrowserWindowSelectionView::CreateWindowInfoLC( const CBrowserWindow& aWindow )
    {
    HBufC* buf = aWindow.BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoTitle );
    if( !buf || !buf->Length() )
        {
        CleanupStack::PopAndDestroy( buf );
        buf = aWindow.BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoUrl );
        }
    return buf;
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionView::UpdateTabs()
// ---------------------------------------------------------
//        
void CBrowserWindowSelectionView::UpdateTabsL()
    {
    delete iWindowInfo;
    iWindowInfo = NULL;
    iWindowInfo = ApiProvider().WindowMgr().GetWindowInfoL( this );

    iTitles->Reset();
    iWindowIds->Reset();
    TInt activeTab = 0;
    for ( TInt i = 0; i < iWindowInfo->Count(); i++ )
        {
        iTitles->AppendL( CONST_CAST( HBufC*, (*iWindowInfo)[i]->iWindowText ) );
        iWindowIds->AppendL( (*iWindowInfo)[i]->iWindowId );
        if ( (*iWindowInfo)[i]->iCurrent )
            {
            activeTab = i;
            HBufC* title = CONST_CAST( HBufC*, (*iWindowInfo)[i]->iWindowText );
            if ( title )
                {
                ApiProvider().Display().SetTitleL( title->Des() );
                }                
            }
        }
    iContainer->ChangeTabL( iTitles, activeTab );
    }
    
    
// ---------------------------------------------------------
// CBrowserWindowSelectionView::UpdateTabs()
// ---------------------------------------------------------
//        
void CBrowserWindowSelectionView::UpdateTumbnailL()
    {
    TInt windowId = GetWindowIdFromTabIndex( iContainer->GetTabGroup()->ActiveTabIndex() );
    iContainer->GetWindowThumbnailL( windowId );
    iContainer->DrawNow();
    }
    
// ---------------------------------------------------------
// CBrowserWindowSelectionView::UpdateTitleL()
// ---------------------------------------------------------
//        
void CBrowserWindowSelectionView::UpdateTitleL( )
    {    
    HBufC* title = GetWindowTitleFromTabIndex( iContainer->GetTabGroup()->ActiveTabIndex() );
    ApiProvider().Display().SetTitleL( (*title).Des( ) );
    }
    
// -----------------------------------------------------------------------------
// CBrowserWindowSelectionView::GetWindowIdFromTabIndex
// -----------------------------------------------------------------------------
//
TInt CBrowserWindowSelectionView::GetWindowIdFromTabIndex( TInt aActiveTabIndex )
    {
    TInt windowId = 0;
    for ( TInt i = 0; i < iWindowIds->Count(); i++ )
        {
        if ( i ==  aActiveTabIndex ) 
            {
            windowId = (*iWindowInfo)[i]->iWindowId;
            }
        }
    return windowId;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowSelectionView::GetWindowIdFromTabIndex
// -----------------------------------------------------------------------------
//
HBufC* CBrowserWindowSelectionView::GetWindowTitleFromTabIndex( TInt aActiveTabIndex )
    {
    HBufC*  windowText = NULL;
    for ( TInt i = 0; i < iWindowInfo->Count(); i++ )
        {        
        if ( i ==  aActiveTabIndex ) 
            {
            windowText = CONST_CAST( HBufC*, (*iWindowInfo)[i]->iWindowText );
            }
        }
    return windowText;
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionView::WindowEventHandlerL
// ---------------------------------------------------------
//
void CBrowserWindowSelectionView::WindowEventHandlerL( TWindowEvent aEvent, TInt /*aWindowId*/ )
    {
    if ( !iWindowEventsDisabled )
        {            
        if ( aEvent == EWindowClose || aEvent == EWindowOpen )
            {
            delete iWindowInfo;
            iWindowInfo = NULL;
            iWindowInfo = ApiProvider().WindowMgr().GetWindowInfoL( this );
            // Update tabs when there are more than one windows open.
            if ( iWindowInfo->Count() > 1 )
                {        
                UpdateTabsL();
                }            
            }            
        if ( aEvent == EWindowLoadStart )
            {
            iEikonEnv->InfoMsg( _L("load start") );
            }            
        if ( aEvent == EWindowLoadStop )
            {
            iEikonEnv->InfoMsg( _L("load stop") );
            }            
        }
    }
        
// End of File
