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
* Description:  Implementation of CPushViewServiceOp.
*
*/



// INCLUDE FILES

#include "PushViewServiceOp.h"
#include "PushMtmUiPanic.h"
#include "PushMtmLog.h"
#include <PushEntry.h>
#include <CUnknownPushMsgEntry.h>
#include <eikenv.h>
#include <apmstd.h>
#include <DocumentHandler.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushViewServiceOp::CPushViewServiceOp
// ---------------------------------------------------------
//
CPushViewServiceOp::CPushViewServiceOp( CMsvSession& aSession, 
                                        TMsvId aEntryId, 
                                        TRequestStatus& aObserverStatus ) 
:   CPushMtmUiOperation( aSession, aEntryId, aObserverStatus )
    {
    PUSHLOG_WRITE("CPushViewServiceOp constructing")
    }

// ---------------------------------------------------------
// CPushViewServiceOp::~CPushViewServiceOp
// ---------------------------------------------------------
//
CPushViewServiceOp::~CPushViewServiceOp()
    {
    Cancel();
    delete iDocHandler;
    }

// ---------------------------------------------------------
// CPushViewServiceOp::StartL
// ---------------------------------------------------------
//
void CPushViewServiceOp::StartL()
    {
    PUSHLOG_ENTERFN("CPushViewServiceOp::~CPushViewServiceOp")

    Cancel();
    CPushMtmOperation::StartL();

    PUSHLOG_LEAVEFN("CPushViewServiceOp::~CPushViewServiceOp")
    }

// ---------------------------------------------------------
// CPushViewServiceOp::RunL
// ---------------------------------------------------------
//
void CPushViewServiceOp::RunL()
    {
    PUSHLOG_ENTERFN("CPushViewServiceOp::RunL")

    TMsvEntry tEntry;
    TMsvId service;
    User::LeaveIfError( iMsvSession.GetEntry( iEntryId, service, tEntry ) );

    __ASSERT_ALWAYS( tEntry.iMtm == KUidMtmWapPush, 
                     UiPanic( EPushMtmUiPanBadMtm ) );

    CUnknownPushMsgEntry* context = NULL;
    const TInt32 bioType( tEntry.iBioType );

    if ( bioType == KUidWapPushMsgUnknown.iUid )
        {
        context = CUnknownPushMsgEntry::NewL();
        }
    else
        {
        __ASSERT_DEBUG( EFalse, UiPanic( EPushMtmUiPanBadBioType ) );
        User::Leave( KErrNotSupported );
        }

    CleanupStack::PushL( context );
    context->RetrieveL( iMsvSession, iEntryId );

    const TPtrC  contentType = context->ContentType();
    const TPtrC8 content = context->MessageData();

    __ASSERT_DEBUG( !iDocHandler, UiPanic( EPushMtmUiPanAlreadyInitialized ) );
    iDocHandler = CDocumentHandler::NewL( CEikonEnv::Static()->Process() );

    iDocHandler->SetExitObserver( this );
    HBufC8* contentType8 = HBufC8::NewMaxLC( contentType.Length() );
    contentType8->Des().Copy( contentType );
    TDataType dataType( *contentType8 );

    CAiwGenericParamList& paramList = iDocHandler->InParamListL();
    RFile tempFile;  
    iDocHandler->SaveTempFileL
        ( content, dataType, KNullDesC, tempFile );
    PUSHLOG_WRITE(" SaveTempFileL OK");
    CleanupClosePushL( tempFile );
    iDocHandler->OpenFileEmbeddedL( tempFile, dataType, paramList );
    PUSHLOG_WRITE(" OpenFileEmbeddedL OK");
    CleanupStack::PopAndDestroy( &tempFile ); // tempFile
    PUSHLOG_WRITE(" PopAndDestroy( &tempFile ) OK");

    CleanupStack::PopAndDestroy( 2, context ); // contentType8, context

    // Signal the observer when the viewer is terminated.

    PUSHLOG_LEAVEFN("CPushViewServiceOp::RunL")
    }

// ---------------------------------------------------------
// CPushViewServiceOp::DoCancel
// ---------------------------------------------------------
//
void CPushViewServiceOp::DoCancel()
    {
    PUSHLOG_WRITE("CPushViewServiceOp::DoCancel")
    SignalObserver( KErrCancel );
    }

// ---------------------------------------------------------
// CPushViewServiceOp::RunError
// ---------------------------------------------------------
//
TInt CPushViewServiceOp::RunError( TInt aError )
    {
    PUSHLOG_WRITE_FORMAT("CPushViewServiceOp::RunError <%d>",aError)
    TRAP_IGNORE( ShowGlobalErrorNoteL( aError ) );
    return CPushMtmOperation::RunError( aError );
    }

// -----------------------------------------------------------------------------
// CPushViewServiceOp::HandleServerAppExit
// -----------------------------------------------------------------------------
//
void CPushViewServiceOp::HandleServerAppExit( TInt aReason )
    {
    PUSHLOG_WRITE_FORMAT("CPushViewServiceOp::HandleServerAppExit <%d>",aReason)
    aReason = 0; // Just to avoid warnings.
    // Ready.
    SignalObserver( KErrNone );
    }

// End of file.

