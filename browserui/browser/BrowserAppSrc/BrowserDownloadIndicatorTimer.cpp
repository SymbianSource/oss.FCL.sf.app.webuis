/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Timer for showing initial download animation in Full Screen Pane of the browser.
*
*/


// INCLUDES
#include <e32base.h>
#include <e32std.h>
#include "BrowserDownloadIndicatorTimer.h"
#include "BrowserDownloadIndicatorTimerEvent.h"


// ---------------------------------------------------------------------------------------
// Default C++ constructor
// ---------------------------------------------------------------------------------------
//
CBrowserDownloadIndicatorTimer::CBrowserDownloadIndicatorTimer() 
    : CTimer(EPriorityNormal)
    {
    }

// ---------------------------------------------------------------------------
// CBrowserDownloadIndicatorTimer::~CBrowserDownloadIndicatorTimer()
// ---------------------------------------------------------------------------
CBrowserDownloadIndicatorTimer::~CBrowserDownloadIndicatorTimer()
    {
    Cancel();
    }

// ---------------------------------------------------------------------------
// CBrowserDownloadIndicatorTimer::ConstructL()
// ---------------------------------------------------------------------------
void CBrowserDownloadIndicatorTimer::ConstructL( 
                    MBrowserDownloadIndicatorTimerEvent& aProgressIndicator )
    {
    iProgressIndicator = &aProgressIndicator;
   
    CTimer::ConstructL();
    CActiveScheduler::Add( this );
    }

//-----------------------------------------------------------------------------
// CBrowserDownloadIndicatorTimer::NewLC()
//-----------------------------------------------------------------------------
//
CBrowserDownloadIndicatorTimer* CBrowserDownloadIndicatorTimer::NewLC(
                    MBrowserDownloadIndicatorTimerEvent& aProgressIndicator )
    {
    CBrowserDownloadIndicatorTimer* self = 
                                new (ELeave) CBrowserDownloadIndicatorTimer;
    CleanupStack::PushL(self);
    self->ConstructL(aProgressIndicator);
    return self;
    }

//-----------------------------------------------------------------------------
// CBrowserDownloadIndicatorTimer::NewL()
//-----------------------------------------------------------------------------
//
CBrowserDownloadIndicatorTimer* CBrowserDownloadIndicatorTimer::NewL(
                    MBrowserDownloadIndicatorTimerEvent& aProgressIndicator)
    {
    CBrowserDownloadIndicatorTimer* self = 
                CBrowserDownloadIndicatorTimer::NewLC( aProgressIndicator );
    CleanupStack::Pop();
    return self;
    }

// ----------------------------------------------------------------------------
// CBrowserDownloadIndicatorTimer::RunL()
// ----------------------------------------------------------------------------
void CBrowserDownloadIndicatorTimer::RunL()
    {
    if (iStatus == KErrNone)
       {
       iProgressIndicator->SetBrowserDownloadIndicatorStateOff();
       }
    }


// ----------------------------------------------------------------------------
// CBrowserDownloadIndicatorTimer::Start()
// ----------------------------------------------------------------------------
void CBrowserDownloadIndicatorTimer::Start(TTimeIntervalMicroSeconds32 aPeriod)
    {
    CTimer::After( aPeriod );
    }

// End of File
