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
* Description:  Implementation of CPushMtmAutoFetchOperation.
*
*/



// INCLUDE FILES

#include "PushMtmAutoFetchOperation.h"
#include "PushMtmFetchOperation.h"
#include "PushContentHandlerPanic.h"
#include "PushMtmLog.h"

// CONSTANTS

/// Max retry.
LOCAL_D const TInt KMaxTry = 2;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMtmAutoFetchOperation::NewL
// ---------------------------------------------------------
//
CPushMtmAutoFetchOperation* CPushMtmAutoFetchOperation::NewL
            (
                const TDesC& aRequestedUrl, 
                TInt aTimeDelayInSec, 
                TRequestStatus& aObserverRequestStatus 
            )
    {
    CPushMtmAutoFetchOperation* op = 
        new (ELeave) CPushMtmAutoFetchOperation
            ( aTimeDelayInSec, aObserverRequestStatus );
    CleanupStack::PushL( op );
    op->ConstructL( aRequestedUrl );
    CleanupStack::Pop(); // op
    return op;
    }

// ---------------------------------------------------------
// CPushMtmAutoFetchOperation::~CPushMtmAutoFetchOperation
// ---------------------------------------------------------
//
CPushMtmAutoFetchOperation::~CPushMtmAutoFetchOperation()
    {
    Cancel();
    delete iFetchOp;
    delete iRequestedUrl;
	iTimer.Close();
    }

// ---------------------------------------------------------
// CPushMtmAutoFetchOperation::StartL
// ---------------------------------------------------------
//
void CPushMtmAutoFetchOperation::StartL()
    {
    PUSHLOG_ENTERFN("CPushMtmAutoFetchOperation::StartL")

    Cancel();

    iTry = 0;
    iState = EFetch;

	SetActive();
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );

    PUSHLOG_LEAVEFN("CPushMtmAutoFetchOperation::StartL")
	}

// ---------------------------------------------------------
// CPushMtmAutoFetchOperation::CPushMtmAutoFetchOperation
// ---------------------------------------------------------
//
CPushMtmAutoFetchOperation::CPushMtmAutoFetchOperation
                            
        ( 
            TInt aTimeDelayInSec, 
            TRequestStatus& aObserverRequestStatus 
        )
:   CActive( EPriorityStandard ), 
    iObserver( aObserverRequestStatus ),
    iState( EInit ), 
    iTimeDelayInSec( aTimeDelayInSec )
    {
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------
// CPushMtmAutoFetchOperation::ConstructL
// ---------------------------------------------------------
//
void CPushMtmAutoFetchOperation::ConstructL( const TDesC& aRequestedUrl )
    {
    PUSHLOG_ENTERFN("CPushMtmAutoFetchOperation::ConstructL")

    iRequestedUrl = HBufC::NewMaxL( aRequestedUrl.Length() );
    iRequestedUrl->Des().Copy( aRequestedUrl );

    User::LeaveIfError( iTimer.CreateLocal() );

    PUSHLOG_LEAVEFN("CPushMtmAutoFetchOperation::ConstructL")
    }

// ---------------------------------------------------------
// CPushMtmAutoFetchOperation::FetchL
// ---------------------------------------------------------
//
void CPushMtmAutoFetchOperation::FetchL()
    {
    PUSHLOG_ENTERFN("CPushMtmAutoFetchOperation::FetchL")

    delete iFetchOp;
    iFetchOp = NULL;
    iStatus = KRequestPending;
    SetActive();
    iFetchOp = CPushMtmFetchOperation::NewL( *iRequestedUrl, iStatus );
    PUSHLOG_WRITE(" Fetch op created")

    ++iTry; // Increase indicator.
    PUSHLOG_WRITE_FORMAT(" Try: %d",iTry)
    iFetchOp->StartL();
    iState = ECheck; // Next state.
    PUSHLOG_WRITE(" Fetch op started")

    PUSHLOG_LEAVEFN("CPushMtmAutoFetchOperation::FetchL")
    }

// ---------------------------------------------------------
// CPushMtmAutoFetchOperation::DoCancel
// ---------------------------------------------------------
//
void CPushMtmAutoFetchOperation::DoCancel()
    {
    if ( iFetchOp )
        {
        iFetchOp->Cancel();
        }

    iTimer.Cancel();

    TRequestStatus* status = &iObserver;
    User::RequestComplete( status, KErrCancel );
    }

// ---------------------------------------------------------
// CPushMtmAutoFetchOperation::RunL
// ---------------------------------------------------------
//
void CPushMtmAutoFetchOperation::RunL()
    {
    PUSHLOG_ENTERFN("CPushMtmAutoFetchOperation::RunL")

    __ASSERT_DEBUG( iState != EInit && iState != EDone, 
                    ContHandPanic( EPushContHandPanAutBadState ) );

    switch ( iState )
        {
        case EFetch:
            {
            PUSHLOG_WRITE(" EFetch")
            FetchL();
            break;
            }

        case ECheck:
            {
            PUSHLOG_WRITE_FORMAT2(" ECheck: %d,%d",iStatus.Int(),iTry)
            if ( iStatus.Int() != KErrNone && iTry < KMaxTry )
                {
                // Wait and Retry.
                iTimer.Cancel();
                iStatus = KRequestPending;
                SetActive();
                TTimeIntervalMicroSeconds32 delayInMSec = 
                                            iTimeDelayInSec * 1000000;
                iTimer.After( iStatus, delayInMSec );
                iState = EFetch;
                }
            else
                {
                // No error or no more trial allowed.
                iState = EDone;
                PUSHLOG_WRITE_FORMAT(" SignalObs: %d",iStatus.Int());
                TRequestStatus* status = &iObserver;
                User::RequestComplete( status, iStatus.Int() );
                }
            break;
            }

        default:
            {
            PUSHLOG_WRITE(" default")
            // JIC.
            iState = EDone;
            TRequestStatus* status = &iObserver;
            User::RequestComplete( status, KErrNone );
            break;
            }
        }

    PUSHLOG_LEAVEFN("CPushMtmAutoFetchOperation::RunL")
    }

// ---------------------------------------------------------
// CPushMtmAutoFetchOperation::RunError
// ---------------------------------------------------------
//
TInt CPushMtmAutoFetchOperation::RunError( TInt aError )
    {
    PUSHLOG_WRITE_FORMAT("CPushMtmAutoFetchOperation::RunError <%d>",aError);

    // Signal the observer that a leave has occured.
    TRequestStatus* status = &iObserver;
    User::RequestComplete( status, aError );

    return KErrNone;
    }

// End of file.
