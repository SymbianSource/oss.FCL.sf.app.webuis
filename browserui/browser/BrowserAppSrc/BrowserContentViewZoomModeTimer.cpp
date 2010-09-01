/*
* Copyright (c) 2008 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Timer for zoom mode.
*
*/


// INCLUDES
#include <e32base.h>

#include "BrowserContentView.h"
#include "BrowserContentViewZoomModeTimer.h"

// ---------------------------------------------------------------------------------------
// Default C++ constructor
// ---------------------------------------------------------------------------------------
//
CBrowserContentViewZoomModeTimer::CBrowserContentViewZoomModeTimer() 
    : CTimer( EPriorityUserInput )
    {
    }

// ---------------------------------------------------------------------------
// CBrowserContentViewZoomModeTimer::~CBrowserContentViewZoomModeTimer()
// ---------------------------------------------------------------------------
CBrowserContentViewZoomModeTimer::~CBrowserContentViewZoomModeTimer()
    {
    Cancel();
    }

// ---------------------------------------------------------------------------
// CBrowserContentViewZoomModeTimer::ConstructL()
// ---------------------------------------------------------------------------
void CBrowserContentViewZoomModeTimer::ConstructL( 
                                    CBrowserContentView* aBrowserContentView )
    {
    iBrowserContentView = aBrowserContentView;

    CTimer::ConstructL();
    CActiveScheduler::Add( this );
    }

//-----------------------------------------------------------------------------
// CBrowserContentViewZoomModeTimer::NewL()
//-----------------------------------------------------------------------------
//
CBrowserContentViewZoomModeTimer* CBrowserContentViewZoomModeTimer::NewL( 
                                    CBrowserContentView* aBrowserContentView )
    {
    CBrowserContentViewZoomModeTimer* self = 
                CBrowserContentViewZoomModeTimer::NewLC( aBrowserContentView );
    CleanupStack::Pop();
    return self;
    }

//-----------------------------------------------------------------------------
// CBrowserContentViewZoomModeTimer::NewLC()
//-----------------------------------------------------------------------------
//
CBrowserContentViewZoomModeTimer* CBrowserContentViewZoomModeTimer::NewLC(
                                    CBrowserContentView* aBrowserContentView )
    {
    CBrowserContentViewZoomModeTimer* self = 
                                new (ELeave) CBrowserContentViewZoomModeTimer();
    CleanupStack::PushL(self);
    self->ConstructL( aBrowserContentView );
    return self;
    }

// ----------------------------------------------------------------------------
// CBrowserContentViewZoomModeTimer::RunL()
// ----------------------------------------------------------------------------
void CBrowserContentViewZoomModeTimer::RunL()
    {
    if ( iBrowserContentView->isZoomMode() )
        {
        // If in zoom mode and timer expires, turn off zoom mode. Zoom indicator
        // should also be removed.
        iBrowserContentView->SetZoomModeL( EFalse );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserDownloadIndicatorTimer::Start()
// ----------------------------------------------------------------------------
void CBrowserContentViewZoomModeTimer::Start( TTimeIntervalMicroSeconds32 aPeriod )
    {
    CTimer::After( aPeriod );
    }

// End of File
