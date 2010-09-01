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
*
*/



// INCLUDE FILES

#include "BrowserFaviconScaler.h"

#include <fbs.h>
#include <BitmapTransforms.h>

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBrowserFaviconScaler::CBrowserFaviconScaler
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CBrowserFaviconScaler::CBrowserFaviconScaler(MBrowserFaviconScalerCallback& aCallback)
    : CActive(CActive::EPriorityIdle-1), iCallback(&aCallback)
    {
    CActiveScheduler::Add(this);
    }

// -----------------------------------------------------------------------------
// CBrowserFaviconScaler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CBrowserFaviconScaler::ConstructL()
    {
    iScaler = CBitmapScaler::NewL();
    }

// -----------------------------------------------------------------------------
// CBrowserFaviconScaler::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CBrowserFaviconScaler* CBrowserFaviconScaler::NewL(MBrowserFaviconScalerCallback& aCallback)
    {
    CBrowserFaviconScaler* self = new( ELeave ) CBrowserFaviconScaler(aCallback);

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }


// Destructor
CBrowserFaviconScaler::~CBrowserFaviconScaler()
    {
    Cancel();
    delete iScaler;
    //delete iResultBitmap;
    }


// -----------------------------------------------------------------------------
// CBrowserFaviconScaler::StartScalingL
//
//
// -----------------------------------------------------------------------------
//
void CBrowserFaviconScaler::StartScalingL(CFbsBitmap& aSource, const TRect& aTargetRect, TBool aIsMask)
    {
    // cancel outstanding request
    Cancel();

    // create target bitmap
    iResultBitmap = new (ELeave) CFbsBitmap();
    TDisplayMode dispMode = (aIsMask) ? EGray256 : aSource.DisplayMode();
    User::LeaveIfError(iResultBitmap->Create(aTargetRect.Size(), dispMode));
    
    // start scaling, async
    iTargetRect = aTargetRect;
    iScaler->Scale(&iStatus, aSource, *iResultBitmap, EFalse);

    SetActive();
    }

// -----------------------------------------------------------------------------
// CPageOverview::DeleteResultBitmap
//
//
// -----------------------------------------------------------------------------
//
void CBrowserFaviconScaler::DeleteResultBitmap()
    {
    delete iResultBitmap;
    iResultBitmap = 0;
    }


// -----------------------------------------------------------------------------
// CPageOverview::DoCancel
//
//
// -----------------------------------------------------------------------------
//
void CBrowserFaviconScaler::DoCancel()
    {
    iScaler->Cancel();
    DeleteResultBitmap();
    }


// -----------------------------------------------------------------------------
// CPageOverview::RunL
//
//
// -----------------------------------------------------------------------------
//
void CBrowserFaviconScaler::RunL()
    {
    iCallback->ScalingCompletedL(iResultBitmap, iTargetRect);
    // if the callback called StartScalingL(), we must not delete the bitmap
    if (!IsActive())
        {
        //DeleteResultBitmap();
        }
    }


// ========================== OTHER EXPORTED FUNCTIONS =========================


//  End of File
