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
* Description:  View class for playing media in full screen
*
*/


// INCLUDE FILES
#include <coecntrl.h>

#include "BavpViewFullScreen.h"
#include "BavpControllerVideo.h"
#include "BavpLogger.h"

// ================= MEMBER FUNCTIONS =======================

// -----------------------------------------------------------------------------
// CBavpViewFullScreen::CBavpViewFullScreen
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CBavpViewFullScreen::CBavpViewFullScreen(CBavpControllerVideo* aController)
					:iNormalController(aController)
    {
    }

// -----------------------------------------------------------------------------
// CBavpViewFullScreen::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CBavpViewFullScreen::ConstructL()
    {
    CreateWindowL();
    SetExtent( TPoint(), TSize() );
    ActivateL();
    SetPointerCapture( ETrue );
    ClaimPointerGrab( ETrue );
    }

// -----------------------------------------------------------------------------
// CBavpViewFullScreen::HandlePointerEventL
// Handles pointer events, simulate a KeyDown event to revert to normal screen
// -----------------------------------------------------------------------------
void CBavpViewFullScreen::HandlePointerEventL(const TPointerEvent& aPointerEvent)
{
	Log( EFalse, _L("CBavpViewFullScreen::HandlePointerEventL") );
	switch (aPointerEvent.iType)
	{
		case TPointerEvent::EButton1Down:
			{
			//simulate a keyup event
			TKeyEvent  aKeyEvent;
			aKeyEvent.iCode = 0;
			aKeyEvent.iModifiers = 0;
			aKeyEvent.iRepeats = 0;
			aKeyEvent.iScanCode = EStdKeyDownArrow;
			//convert back to normal screen
			iNormalController->HandleKeysL(aKeyEvent, EEventKeyDown);
			break;
			}
		default:
			break;
	}


}
// -----------------------------------------------------------------------------
// CBavpViewFullScreen::NewL()
// Two-phased constructor.
// -----------------------------------------------------------------------------
CBavpViewFullScreen* CBavpViewFullScreen::NewL(CBavpControllerVideo* aController)
    {
    CBavpViewFullScreen* self = new (ELeave) CBavpViewFullScreen(aController);

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
	}


// -----------------------------------------------------------------------------
// CBavpViewFullScreen::~CBavpViewFullScreen
// Destructor
// -----------------------------------------------------------------------------
CBavpViewFullScreen::~CBavpViewFullScreen()
    {
    // ignore error
	//release pointer control
	SetPointerCapture(EFalse);
    }

// -----------------------------------------------------------------------------
// CBavpViewFullScreen::Draw
// This method is called by CCoeControl::Draw
// -----------------------------------------------------------------------------
void CBavpViewFullScreen::Draw( const TRect& aRect ) const
    {
    if ( Window().DisplayMode() == EColor16MA )
        {
        CWindowGc& gc = SystemGc();
        gc.SetDrawMode( CGraphicsContext::EDrawModeWriteAlpha );
        gc.SetBrushColor( TRgb::Color16MA(0) );
        gc.Clear( aRect );
        }
    }

// End of File
