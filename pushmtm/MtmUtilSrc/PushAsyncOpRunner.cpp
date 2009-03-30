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
*      This file contains the member definitions of CPushAsyncOpRunner.
*      
*
*/



//  INCLUDES

#include "PushAsyncOpRunner.h"
#include "PushMtmUtilPanic.h"
#include <msvapi.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushAsyncOpRunner::StartL
// ---------------------------------------------------------
//
EXPORT_C void CPushAsyncOpRunner::StartL()
    {
    Cancel();
    TBool isObserving( EFalse );
    iReady = OnStartupL( isObserving );
    SetActive();
    if ( !isObserving )
        {
        InvokeRun();
        }
    }

// ---------------------------------------------------------
// CPushAsyncOpRunner::ProgressL
// ---------------------------------------------------------
//
EXPORT_C const TDesC8& CPushAsyncOpRunner::ProgressL()
    {
    if ( iOperation )
        {
        return iOperation->ProgressL();
        }
    else
        {
        return iDummyProgressBuf;
        }
    }

// ---------------------------------------------------------
// CPushAsyncOpRunner::CPushAsyncOpRunner
// ---------------------------------------------------------
//
EXPORT_C CPushAsyncOpRunner::CPushAsyncOpRunner( CMsvSession& aSession, 
                                                 const TMsvId aEntryId, 
                                                 TRequestStatus& aObserverStatus ) 
:   CPushMtmOperation( aSession, aEntryId, aObserverStatus ), 
    iReady( EFalse ), 
    iDummyProgressBuf( KNullDesC8 )
    {
    }

// ---------------------------------------------------------
// CPushAsyncOpRunner::~CPushAsyncOpRunner
// ---------------------------------------------------------
//
EXPORT_C CPushAsyncOpRunner::~CPushAsyncOpRunner()
    {
    Cancel();
    delete iOperation;
    }

// ---------------------------------------------------------
// CPushAsyncOpRunner::OnStartupL
// ---------------------------------------------------------
//
EXPORT_C TBool CPushAsyncOpRunner::OnStartupL( TBool& aIsObserving )
    {
    aIsObserving = EFalse;
    return ETrue;
    }

// ---------------------------------------------------------
// CPushAsyncOpRunner::OnRunLActionL
// ---------------------------------------------------------
//
EXPORT_C TBool CPushAsyncOpRunner::OnRunLActionL( TBool& aIsObserving )
    {
    aIsObserving = EFalse;
    return ETrue;
    }

// ---------------------------------------------------------
// CPushAsyncOpRunner::SetOperation
// ---------------------------------------------------------
//
EXPORT_C void CPushAsyncOpRunner::SetOperation( CMsvOperation* aOp )
    {
    delete iOperation;
    iOperation = NULL;
    iOperation = aOp;
    }

// ---------------------------------------------------------
// CPushAsyncOpRunner::SetOperation
// ---------------------------------------------------------
//
EXPORT_C void CPushAsyncOpRunner::DeleteOperation()
    {
    delete iOperation;
    iOperation = NULL;
    }

// ---------------------------------------------------------
// CPushAsyncOpRunner::Operation
// ---------------------------------------------------------
//
EXPORT_C CMsvOperation& CPushAsyncOpRunner::Operation()
    {
    __ASSERT_DEBUG( iOperation, UtilPanic( EPushMtmUtilPanNull ) );
    return *iOperation;
    }

// ---------------------------------------------------------
// CPushAsyncOpRunner::DoCancel
// ---------------------------------------------------------
//
EXPORT_C void CPushAsyncOpRunner::DoCancel()
    {
    if ( iOperation )
        {
        iOperation->Cancel();
        }
    CPushMtmOperation::DoCancel();
    }

// ---------------------------------------------------------
// CPushAsyncOpRunner::RunError
// ---------------------------------------------------------
//
EXPORT_C TInt CPushAsyncOpRunner::RunError( TInt aError )
    {
    if ( iOperation )
        {
        iOperation->Cancel();
        delete iOperation;
        iOperation = NULL;
        }
    return CPushMtmOperation::RunError( aError );
    }

// ---------------------------------------------------------
// CPushAsyncOpRunner::RunL
// ---------------------------------------------------------
//
EXPORT_C void CPushAsyncOpRunner::RunL()
    {
    if ( iReady )
        {
        SignalObserver( KErrNone );
        }
    else
        {
        TBool isObserving( EFalse );
        iReady = OnRunLActionL( isObserving );
        if ( iReady )
            {
            SignalObserver( KErrNone );
            }
        else
            {
            // Invoke RunL again and run OnRunLActionL in it again.
            SetActive();
            if ( !isObserving )
                {
                InvokeRun();
                }
            }
        }
    }

// End of file.
