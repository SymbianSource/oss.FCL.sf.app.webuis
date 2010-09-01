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
* Description:  Implementation of CPushMtmOperation.
*
*/



// INCLUDE FILES

#include "PushMtmOperation.h"
#include <msvids.h>
#include <PushEntry.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMtmOperation::~CPushMtmOperation
// ---------------------------------------------------------
//
EXPORT_C CPushMtmOperation::~CPushMtmOperation()
    {
    Cancel();
    delete iCEntry;
    }

// ---------------------------------------------------------
// CPushMtmOperation::StartL
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmOperation::StartL()
    {
    // Enable restart.
    Cancel();
	SetActive();
    InvokeRun();
    }

// ---------------------------------------------------------
// CPushMtmOperation::ProgressL
// ---------------------------------------------------------
//
EXPORT_C const TDesC8& CPushMtmOperation::ProgressL()
    {
    return iProgressPckg;
    }

// ---------------------------------------------------------
// CPushMtmOperation::CPushMtmOperation
// ---------------------------------------------------------
//
EXPORT_C CPushMtmOperation::CPushMtmOperation( CMsvSession& aSession, 
                                               TMsvId aId, 
                                               TRequestStatus& aObserverRequestStatus ) 
:   CMsvOperation( aSession, EPriorityStandard, aObserverRequestStatus ), 
    iEntryId( aId ), 
    iProgressPckg( iProgress )
    {
    iProgress.Reset();
    // The Push MTM has no services.
	iService = KMsvLocalServiceIndexEntryId;
    // The operation belongs to the Push MTM.
	iMtm = KUidMtmWapPush;
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------
// CPushMtmOperation::InvokeRun
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmOperation::InvokeRun()
    {
    TRequestStatus* status = &iStatus;
    User::RequestComplete( status, KErrNone );
    }

// ---------------------------------------------------------
// CPushMtmOperation::SignalObserver
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmOperation::SignalObserver( TInt aCompletionCode )
    {
    TRequestStatus* status = &iObserverRequestStatus;
    User::RequestComplete( status, aCompletionCode );
    }

// ---------------------------------------------------------
// CPushMtmOperation::ObserveEntryEventL
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmOperation::ObserveEntryEventL()
    {
    if ( iCEntry == NULL )
        {
        iCEntry = iMsvSession.GetEntryL( iEntryId );
        }
    iCEntry->AddObserverL( *this );
    }

// ---------------------------------------------------------
// CPushMtmOperation::CancelObserveEntryEvent
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmOperation::CancelObserveEntryEvent()
    {
    if ( iCEntry )
        {
        iCEntry->RemoveObserver( *this );
        }
    }

// ---------------------------------------------------------
// CPushMtmOperation::RunL
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmOperation::RunL()
    {
    SignalObserver( KErrNone );
    }

// ---------------------------------------------------------
// CPushMtmOperation::DoCancel
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmOperation::DoCancel()
    {
    CancelObserveEntryEvent();
    SignalObserver( KErrCancel );
    }

// ---------------------------------------------------------
// CPushMtmOperation::RunError
// ---------------------------------------------------------
//
EXPORT_C TInt CPushMtmOperation::RunError( TInt aError )
    {
    CancelObserveEntryEvent();
    if ( aError )
        {
        // Signal the observer that a leave has occured.
        SignalObserver( aError );
        }
    return KErrNone;
    }

// ---------------------------------------------------------
// CPushMtmOperation::HandleEntryEventL
// ---------------------------------------------------------
//
EXPORT_C void CPushMtmOperation::HandleEntryEventL( TMsvEntryEvent /*aEvent*/, 
                           TAny* /*aArg1*/, TAny* /*aArg2*/, TAny* /*aArg3*/ )
    {
    }

// End of file.
