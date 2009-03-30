/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
*      Implementation of CBrowserInitialContainer.
*
*
*/


// INCLUDE FILES

#include <AknLists.h>
#include <BrowserNG.rsg>
#include <barsread.h>
#include <browser.mbg>
#include <AknsUtils.h>
#include <data_caging_path_literals.hrh>
#include "BrowserInitialContainer.h"
#include "CommonConstants.h"
#include "BrowserInitialView.h"
#include "BrowserAppUi.h"
#include "BrowserUIVariant.hrh"
#include "Display.h"
#include "BrowserGotoPane.h"


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserInitialContainer::CBrowserInitialContainer
// ---------------------------------------------------------
//
CBrowserInitialContainer::CBrowserInitialContainer( CBrowserInitialView *aView ) : iView( aView )
    {
    // 
    }
    
// ---------------------------------------------------------
// CBrowserInitialContainer::ConstructL
// ---------------------------------------------------------
//
void CBrowserInitialContainer::ConstructL(const TRect& aRect)
    {
    CreateWindowL();	

    // Set view title
	iView->ApiProvider().Display().SetTitleL( TitleResourceId() );
		

	// Create gotopane
    iGotoPane = CBrowserGotoPane::NewL( this );
    
    SetRect(aRect);	
    ActivateL();
    }

// ---------------------------------------------------------
// CBrowserInitialContainer::~CBrowserInitialContainer
// ---------------------------------------------------------
//
CBrowserInitialContainer::~CBrowserInitialContainer()
    {
    delete iGotoPane;
    }

// ---------------------------------------------------------
// CBrowserInitialContainer::SizeChanged
// ---------------------------------------------------------
//
void CBrowserInitialContainer::SizeChanged()
    {
   
	iGotoPane->HandleFindSizeChanged();	
    }

// ---------------------------------------------------------
// CBrowserInitialContainer::CountComponentControls
// ---------------------------------------------------------
//
TInt CBrowserInitialContainer::CountComponentControls() const
    {
    return 1; // return number of controls inside this container
    }

// ---------------------------------------------------------
// CBrowserInitialContainer::ComponentControl
// ---------------------------------------------------------
//
CCoeControl* CBrowserInitialContainer::ComponentControl( TInt aIndex ) const
    {
    switch ( aIndex )
        {
        case 0:
			return iGotoPane;
        default:
            return NULL;
        }
    }

// ---------------------------------------------------------
// CBrowserInitialContainer::Draw
// ---------------------------------------------------------
//
void CBrowserInitialContainer::Draw( const TRect& aRect ) const
    {
    CWindowGc& gc = SystemGc();
    
    // example code...
    gc.SetPenStyle( CGraphicsContext::ENullPen );
    gc.SetBrushColor( KRgbGray );
    gc.SetBrushStyle( CGraphicsContext::ESolidBrush );
    gc.DrawRect( aRect );
    }

// ---------------------------------------------------------
// CBrowserInitialContainer::HandleControlEventL
// ---------------------------------------------------------
//
void CBrowserInitialContainer::HandleControlEventL( CCoeControl* /*aControl*/,
                                                    TCoeEvent /*aEventType*/ )
    {
    // control event handler code here
    }

	

// ----------------------------------------------------------------------------
// CBrowserInitialContainer::OfferKeyEventL
// ----------------------------------------------------------------------------
//    
TKeyResponse CBrowserInitialContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
	{
    CBrowserAppUi* ui = CBrowserAppUi::Static();
    TKeyResponse result( EKeyWasNotConsumed );

	if (ui->OfferApplicationSpecificKeyEventL(aKeyEvent, aType) == EKeyWasConsumed)
		{
		return EKeyWasConsumed;
		}

    // If goto pane is visible, offer key events to it
	if ( iGotoPane->IsVisible() )
		{
		result = iGotoPane->OfferKeyEventL( aKeyEvent, aType );
		}

    return result;
	}

// ---------------------------------------------------------
// CBrowserInitialContainer::TitleResourceId
// ---------------------------------------------------------
//
TInt CBrowserInitialContainer::TitleResourceId()
    {
    return R_BROWSER_INITIAL_VIEW_TITLE;
    }			
    
//---------------------------------------------------------------------------
// CBrowserInitialContainer::ShutDownGotoURLEditorL
// ---------------------------------------------------------------------------
//
void CBrowserInitialContainer::ShutDownGotoURLEditorL()
    {
    iGotoPane->MakeVisible( EFalse );
    iGotoPane->SetFocus( EFalse );
    SetFocus( ETrue );
    }    
    
// End of File
