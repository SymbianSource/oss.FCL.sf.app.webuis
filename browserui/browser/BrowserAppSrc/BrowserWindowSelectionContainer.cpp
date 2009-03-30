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
*      Implementation of CBrowserWindowSelectionContainer.
*
*
*/



// INCLUDE FILES
#include "BrowserWindowSelectionContainer.h"
#include "BrowserWindowSelectionView.h"
#include "BrowserAppUi.h"
#include "Display.h"
#include "WindowInfoProvider.h"
#include "BrowserWindowManager.h"
#include "BrowserWindow.h"
#include <BrowserNG.rsg>
#include <aknnavi.h>
#include <akntabgrp.h>
#include <aknnavide.h> 
#include <akntabgrp.h>
#include <eikimage.h>
#include <barsread.h>
#include <BrCtlDefs.h>
#include <BrCtlInterface.h>
#include <BrCtlDefs.h>
#include <browser.mbg>
#include <gulicon.h>


// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::ConstructL
// ---------------------------------------------------------
//
void CBrowserWindowSelectionContainer::ConstructL(const TRect& aRect)
    {
    CreateWindowL();	
    // Tabgroup has been read from resource and it were pushed to the navi pane.
    // Get pointer to the navigation decorator with the ResourceDecorator() function.
    // Application owns the decorator and it has responsibility to delete the object.
    iNaviDecorator = iView->ApiProvider().Display().NaviPaneL()->CreateTabGroupL();
    iTabGroup = (CAknTabGroup*)iNaviDecorator->DecoratedControl();   
    iTabGroup->SetObserver(this);

    /*
    // Create tab icon  	
    iTabIcon = new (ELeave) CEikImage;
    TResourceReader rr;
    iCoeEnv->CreateResourceReaderLC( rr, R_BROWSER_ICON_PROP_FOLDER );
    iTabIcon->ConstructFromResourceL( rr );
    CleanupStack::PopAndDestroy(); // rr
    */
         
    SetRect(aRect);
    ActivateL();
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::CBrowserWindowSelectionContainer
// ---------------------------------------------------------
//    
CBrowserWindowSelectionContainer::CBrowserWindowSelectionContainer( CBrowserWindowSelectionView *aView ) : iView( aView )
    {
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::~CBrowserWindowSelectionContainer
// ---------------------------------------------------------
//    
CBrowserWindowSelectionContainer::~CBrowserWindowSelectionContainer()
    {
    delete iNaviDecorator;
    delete iThumbnail;
    }            

// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::ChangeTabL
// ---------------------------------------------------------
//        
void CBrowserWindowSelectionContainer::ChangeTabL( CArrayFixFlat<HBufC*>* aTabTexts, TInt aActivateTab )
    {
    // If there are any tabs
    if ( iTabGroup->TabCount() )
        {
        TInt tabCount = iTabGroup->TabCount();
        // Delete all tabs
        for ( TInt tabIndex = 0; tabIndex < tabCount; tabIndex++ )
            {
            // Delete the first tab until there is no more tab...
            iTabGroup->DeleteTabL( iTabGroup->TabIdFromIndex( 0 )  );
            }        
        }        
    // If only two tabs are needed, dont show more tabs.
    if ( aTabTexts->Count() == 2 )
        {
        GetTabGroup()->SetTabFixedWidthL( EAknTabWidthWithTwoLongTabs );        
        }
    // If more than two tabs are needed, maximum 3 tabs are visible
    else
        {
        GetTabGroup()->SetTabFixedWidthL( EAknTabWidthWithThreeLongTabs );                    
        }    
    // Add create the tabs
    for ( TInt tabIndex = 0; tabIndex < aTabTexts->Count(); tabIndex++ )
        {
        iTabGroup->AddTabL( tabIndex, (*aTabTexts)[tabIndex]->Des() /*, iTabIcon->Bitmap(), iTabIcon->Mask()*/ );
        }        
    // Set focus of the specified tab
    iTabGroup->SetActiveTabByIndex( aActivateTab );
    // Handle tab changes.
    iView->ApiProvider().Display().NaviPaneL()->PushL( *iNaviDecorator );
    
    // Draw the thumbnail by the window id.
    GetWindowThumbnailL( iView->GetWindowIdFromTabIndex( GetTabGroup()->ActiveTabIndex() ) );
    DrawNow();
    }
        
// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::SizeChanged
// ---------------------------------------------------------
//
void CBrowserWindowSelectionContainer::SizeChanged()
    {
    // TODO: Add here control resize code etc.
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::CountComponentControls
// ---------------------------------------------------------
//
TInt CBrowserWindowSelectionContainer::CountComponentControls() const
    {
    return 0; // return nbr of controls inside this container
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::ComponentControl
// ---------------------------------------------------------
//
CCoeControl* CBrowserWindowSelectionContainer::ComponentControl(TInt aIndex) const
    {
    switch ( aIndex )
        {
        default:
            return NULL;
        }
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::Draw
// ---------------------------------------------------------
//
void CBrowserWindowSelectionContainer::Draw(const TRect& aRect) const
    {
    CWindowGc& gc = SystemGc();
    
    // example code...
    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushColor( KRgbWhite );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.DrawRect( aRect );
        
    if ( iThumbnail )
        {
        // Put in center        
        // release
        TSize bitmapSize = iThumbnail->Bitmap()->SizeInPixels();        
        // test
        //TSize bitmapSize = iThumbnail->SizeInPixels();
        TInt x = ( aRect.Width()-bitmapSize.iWidth ) / 2;
        TInt y = ( aRect.Height()-bitmapSize.iHeight ) / 2;    
        // Draw thumbnail
        // release
        gc.BitBlt( TPoint(x,y), iThumbnail->Bitmap() );        
        // test
        // gc.BitBlt( TPoint(x,y), iThumbnail );                
        }
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::OfferKeyEventL
// ---------------------------------------------------------
//    
TKeyResponse CBrowserWindowSelectionContainer::OfferKeyEventL( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    // Handle if Ok key was pressed.
	if ( aType == EEventKey && aKeyEvent.iCode == EKeyOK )
	    {
        TInt windowId = iView->GetWindowIdFromTabIndex( GetTabGroup()->ActiveTabIndex() );
        iView->ApiProvider().WindowMgr().SwitchWindowL( windowId );
        //If the user switches window we reset the CalledFromAnotherApp status
        if(iView->ApiProvider().CalledFromAnotherApp())
            {
            iView->ApiProvider().SetCalledFromAnotherApp(EFalse);
            }
        iView->ApiProvider().SetViewToBeActivatedIfNeededL( iView->GetPreviousViewID() );	    
	    return EKeyWasConsumed;
	    }
    
    // Check if navipane has valid parameters.
    if ( !iTabGroup )
        {
        return EKeyWasNotConsumed;        
        }    
    if ( !iTabGroup->TabCount() )
        {
        return EKeyWasNotConsumed;        
        }        
    if ( !iNaviDecorator )
        {
        return EKeyWasNotConsumed;
        }
    // Get tab group
	CAknTabGroup* tabGroup = STATIC_CAST( CAknTabGroup*, iNaviDecorator->DecoratedControl() );
    // If tab group was not created succesfully.
    if ( !tabGroup )	
        {
        return EKeyWasNotConsumed;
        }        
    // If tab group has no tabs.
    if ( !tabGroup->TabCount() )
        {
        return EKeyWasNotConsumed;
        }
    // Forward events to tabgroup.
    const TKeyResponse result = tabGroup->OfferKeyEventL( aKeyEvent, aType );
    
    return result;
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::HandleControlEventL
// ---------------------------------------------------------
//
void CBrowserWindowSelectionContainer::HandleControlEventL(
    CCoeControl* /*aControl*/,TCoeEvent /*aEventType*/)
    {
    // TODO: Add your control event handler code here
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::GetWindowThumbnail
// ---------------------------------------------------------
//
void CBrowserWindowSelectionContainer::GetWindowThumbnailL( TInt aWindowId )
    {   
    // Get window url
    CBrowserWindow* window;
    iView->ApiProvider().WindowMgr().Window( aWindowId, window );
    HBufC* windowUrl = window->BrCtlInterface().PageInfoLC( TBrCtlDefs::EPageInfoUrl );
    
    // Get thumbnail
    // release
    delete iThumbnail;
    if (windowUrl)
    {
    	iThumbnail = window->BrCtlInterface().GetBitmapData( windowUrl->Des(), TBrCtlDefs::EBitmapThumbnail );
    }
    else
    {
       	iThumbnail = NULL;
    }
    // eof release
   	
       
    CleanupStack::PopAndDestroy(); // windowUrl               
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::HandlePointerEventL
// ---------------------------------------------------------
//
void CBrowserWindowSelectionContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
    if (aPointerEvent.iType == TPointerEvent::EButton1Down)
        {
        iView->HandleCommandL(EAknCmdOpen);
        }
    }

// ---------------------------------------------------------
// CBrowserWindowSelectionContainer::TabChangedL
// ---------------------------------------------------------
//
void CBrowserWindowSelectionContainer::TabChangedL(TInt /*aIndex*/)
    {
    iView->UpdateTitleL();
    iView->UpdateTumbnailL();
    }

// End of File  
