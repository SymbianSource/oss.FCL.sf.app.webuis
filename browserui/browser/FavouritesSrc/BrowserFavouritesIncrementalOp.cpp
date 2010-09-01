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
* Description: 
*      Implementation of CWmlBrowserFavouritesIncrementalOp.
*      
*
*/


// INCLUDE FILES

#include <AknWaitDialog.h>
#include <BrowserNG.rsg>
#include <eikenv.h>
#include "BrowserFavouritesIncrementalOp.h"
#include "BrowserFavouritesModel.h"
#include "Browser.hrh"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalOp::~CBrowserFavouritesIncrementalOp
// ---------------------------------------------------------
//
CBrowserFavouritesIncrementalOp::~CBrowserFavouritesIncrementalOp()
    {
    // Base class cancels.
    delete iWaitNote;                   // Safety code.
    if ( iWait && iWait->IsStarted() )  // Safety code.
        {
        iWait->AsyncStop();
        }
    delete iWait;
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalOp::ExecuteL
// ---------------------------------------------------------
//
void CBrowserFavouritesIncrementalOp::ExecuteL()
    {
    CreateWaitNoteLC();
    iWaitNote->SetCallback( this );
    iWaitNote->RunLD();
    Call();         // Schedule first step.
    iWait->Start(); // Wait for completion.
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalOp::CBrowserFavouritesIncrementalOp
// ---------------------------------------------------------
//
CBrowserFavouritesIncrementalOp::CBrowserFavouritesIncrementalOp
    (
    CBrowserFavouritesModel& aModel,
    TInt aPriority /*=CActive::EPriorityStandard*/
    )
: CAsyncOneShot( aPriority ), iModel( &aModel )
    {
    // Base class adds this to the Active Scheduler.
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalOp::ConstructL
// ---------------------------------------------------------
//
void CBrowserFavouritesIncrementalOp::ConstructL()
    {
    iWait = new (ELeave) CActiveSchedulerWait();
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalOp::DoneL
// ---------------------------------------------------------
//
void CBrowserFavouritesIncrementalOp::DoneL()
    {
    // Default is do nothing.
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalOp::RunL
// ---------------------------------------------------------
//
void CBrowserFavouritesIncrementalOp::RunL()
    {
    if ( StepL() )
        {
        // More steps to go.
        Call();
        }
    else
        {
        // This call deletes the dialog and NULL-s ptr. When dialog goes down,
        // DialogDismisedL will be called, and we call DoneL() and stop
        // waiting there.
        iWaitNote->ProcessFinishedL();
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalOp::DoCancel
// ---------------------------------------------------------
//
void CBrowserFavouritesIncrementalOp::DoCancel()
    {
    delete iWaitNote;
    iWaitNote = NULL;
    iWait->AsyncStop();
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalOp::RunError
// ---------------------------------------------------------
//
TInt CBrowserFavouritesIncrementalOp::RunError( TInt aError )
    {
    // Own part of error processing: clean up the wait note and terminate
    // the waiting. Then propagate the error up to base classes.
    delete iWaitNote;
    iWaitNote = NULL;
    iWait->AsyncStop();
    return aError;
    }

// ---------------------------------------------------------
// CBrowserFavouritesIncrementalOp::DialogDismissedL
// ---------------------------------------------------------
//
void CBrowserFavouritesIncrementalOp::DialogDismissedL( TInt /*aButtonId*/ )
    {
    DoneL();
    iWait->AsyncStop();
    }
// End of File
