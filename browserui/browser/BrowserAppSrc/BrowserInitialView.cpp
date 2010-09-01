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
*      Implementation of CBrowserInitialView.
*
*
*/


// INCLUDE FILES
#include <avkon.hrh>
#include <BrowserNG.rsg>
#include <FeatMgr.h>
#include <ApUtils.h>
#include "BrowserInitialView.h"
#include "BrowserInitialContainer.h"
#include "BrowserBookmarksView.h"
#include "BrowserAppUi.h"
#include "CommonConstants.h"
#include "Browser.hrh"

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------------------------------
// CBrowserInitialView::NewLC
// ----------------------------------------------------------------------------
//
CBrowserInitialView* CBrowserInitialView::NewLC( MApiProvider& aApiProvider)
    {
    CBrowserInitialView* view = new (ELeave) CBrowserInitialView( aApiProvider );
    CleanupStack::PushL( view );
    view->ConstructL( );
    return view;
    }

// ---------------------------------------------------------
// CBrowserInitialView::ConstructL(const TRect& aRect)
// ---------------------------------------------------------
//
void CBrowserInitialView::ConstructL()
    {
    BaseConstructL( R_BROWSERINITIAL_VIEW );
    }

// ---------------------------------------------------------
// CBrowserInitialView::CBrowserInitialView()
// ---------------------------------------------------------
//
CBrowserInitialView::CBrowserInitialView( MApiProvider& aApiProvider ):
CBrowserViewBase( aApiProvider )
    {
    }

// ---------------------------------------------------------
// CBrowserInitialView::~CBrowserInitialView()
// destructor
// ---------------------------------------------------------
//
CBrowserInitialView::~CBrowserInitialView()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }

    delete iContainer;
    delete iEnteredURL;
    }

// ---------------------------------------------------------
// TUid CBrowserInitialView::Id()
// ---------------------------------------------------------
//
TUid CBrowserInitialView::Id() const
    {
    return KUidBrowserInitialViewId;
    }

// ---------------------------------------------------------
// CBrowserInitialView::HandleCommandL
// ---------------------------------------------------------
//
void CBrowserInitialView::HandleCommandL(TInt aCommand)
    {
    AppUi()->HandleCommandL( aCommand );
    }

// ---------------------------------------------------------
// CBrowserInitialView::HandleClientRectChange()
// ---------------------------------------------------------
//
void CBrowserInitialView::HandleClientRectChange()
    {
    if ( iContainer )
        {
        iContainer->SetRect( ClientRect() );
        }
    }

// ---------------------------------------------------------------------------
// CBrowserInitialView::CommandSetResourceIdL
// ---------------------------------------------------------------------------
TInt CBrowserInitialView::CommandSetResourceIdL()
    {
    return R_BROWSERINITIAL_CBA_OPTIONS_EXIT;
    }

// ---------------------------------------------------------
// CBrowserInitialView::DoActivateL
// ---------------------------------------------------------
//
void CBrowserInitialView::DoActivateL( const TVwsViewId& /*aPrevViewId*/,
                                       TUid /*aCustomMessageId*/,
                                       const TDesC8& /*aCustomMessage*/ )
    {
    iPreviousViewID = ApiProvider().LastActiveViewId();
 	ApiProvider().SetLastActiveViewId( Id() );
    if (!iContainer)
        {
        iContainer = new (ELeave) CBrowserInitialContainer( this );
        iContainer->SetMopParent(this);
        iContainer->ConstructL( ClientRect() );
        AppUi()->AddToViewStackL( *this, iContainer );
        iContainer->GotoPane()->SetGPObserver( this );
        }
    UpdateCbaL();
   }

// ---------------------------------------------------------
// CBrowserInitialView::DynInitMenuPaneL
// ---------------------------------------------------------
//
void CBrowserInitialView::DynInitMenuPaneL( TInt /*aResourceId*/, CEikMenuPane* /*aMenuPane*/ )
    {
    }

// ---------------------------------------------------------
// CBrowserInitialView::DoDeactivate()
// ---------------------------------------------------------
//
void CBrowserInitialView::DoDeactivate()
    {
    if ( iContainer )
        {
        AppUi()->RemoveFromViewStack( *this, iContainer );
        }

    delete iContainer;
    iContainer = NULL;
    }

// ---------------------------------------------------------
// CBrowserInitialView::LaunchGotoAddressEditorL
// ---------------------------------------------------------
//
void CBrowserInitialView::LaunchGotoAddressEditorL()
    {
    }

void CBrowserInitialView::HandleGotoPaneEventL( CBrowserGotoPane* /*aGotoPane*/, TEvent /*aEvent*/ )
    {
    //
    }

// ---------------------------------------------------------
// CBrowserInitialView::GotoUrlInGotoPaneL
// ---------------------------------------------------------
//
void CBrowserInitialView::GotoUrlInGotoPaneL()
    {
    }

// ---------------------------------------------------------
// CBrowserInitialView::UpdateGotoPaneL
// ---------------------------------------------------------
//
void CBrowserInitialView::UpdateGotoPaneL()
    {
    }
// End of File
