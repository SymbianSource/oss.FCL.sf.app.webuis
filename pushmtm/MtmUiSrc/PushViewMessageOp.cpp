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
* Description:  Implementation of CPushViewMessageOp.
*
*/



// INCLUDE FILES

#include "PushViewMessageOp.h"
#include "PushMtmUiDef.h"
#include "PushMtmUiPanic.h"
#include "PushMtmLog.h"
#include "PushMtmUtil.h"
#include <CSIPushMsgEntry.h>
#include <eikenv.h>
#include <aknappui.h>
#include <uikon.hrh>
#include <apmstd.h>
#include <DocumentHandler.h>
//#include <PushMtmUi.rsg>
#include <AknNoteWrappers.h>
#include <data_caging_path_literals.hrh>
#include <f32file.h>

// CONSTANTS

_LIT8( KTextPlainString, "text/plain" );

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushLoadServiceOp::NewL
// ---------------------------------------------------------
//
CPushViewMessageOp* CPushViewMessageOp::NewL( CMsvSession& aSession, 
                                              TMsvId aEntryId, 
                                              TRequestStatus& aObserverStatus ) 
    {
    PUSHLOG_ENTERFN("CPushViewMessageOp::NewL")

    CPushViewMessageOp* self = 
        new (ELeave) CPushViewMessageOp( aSession, aEntryId, aObserverStatus );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );

    PUSHLOG_LEAVEFN("CPushViewMessageOp::NewL")
    return self;
    }

// ---------------------------------------------------------
// CPushViewMessageOp::~CPushViewMessageOp
// ---------------------------------------------------------
//
CPushViewMessageOp::~CPushViewMessageOp()
    {
    PUSHLOG_ENTERFN("CPushViewMessageOp::~CPushViewMessageOp")

    Cancel();
    delete iDocHandler;

    PUSHLOG_LEAVEFN("CPushViewMessageOp::~CPushViewMessageOp")
    }

// ---------------------------------------------------------
// CPushViewMessageOp::StartL
// ---------------------------------------------------------
//
void CPushViewMessageOp::StartL()
    {
    PUSHLOG_WRITE("CPushViewMessageOp StartL")

    Cancel();

    // Mark service read.
    iMtmUtil->MarkServiceUnreadL( iEntryId, EFalse );

    CPushMtmUiOperation::StartL();
    }

// ---------------------------------------------------------
// CPushViewMessageOp::CPushViewMessageOp
// ---------------------------------------------------------
//
CPushViewMessageOp::CPushViewMessageOp( CMsvSession& aSession, 
                                        TMsvId aEntryId, 
                                        TRequestStatus& aObserverStatus ) 
:   CPushMtmUiOperation( aSession, aEntryId, aObserverStatus ),
    iIsCancelled( EFalse )
    {
    PUSHLOG_WRITE("CPushViewMessageOp constructing")
    }

// ---------------------------------------------------------
// CPushViewMessageOp::ConstructL
// ---------------------------------------------------------
//
void CPushViewMessageOp::ConstructL() 
    {
    CPushMtmUiOperation::ConstructL();
    }

// ---------------------------------------------------------
// CPushViewMessageOp::RunL
// ---------------------------------------------------------
//
void CPushViewMessageOp::RunL()
    {
    PUSHLOG_ENTERFN("CPushViewMessageOp::RunL");

    if ( iDocHandler )
        {
        // Content handler is running; we just need to close it, 
        // NotifyExit has already been called.
        delete iDocHandler;
        iDocHandler = NULL;
        CancelObserveEntryEvent();

        // Ready.
        SignalObserver( KErrNone );
        }
    else
        {
        // Lauch the content handler.
        TMsvEntry tEntry;
        TMsvId service;
        User::LeaveIfError( iMsvSession.GetEntry( iEntryId, service, tEntry ) );

        __ASSERT_ALWAYS( tEntry.iMtm == KUidMtmWapPush, 
                         UiPanic( EPushMtmUiPanBadMtm ) );

        CSIPushMsgEntry* si = NULL;
        const TInt32 bioType( tEntry.iBioType );

        if ( bioType == KUidWapPushMsgSI.iUid )
            {
            si = CSIPushMsgEntry::NewL();
            }
        else
            {
            __ASSERT_DEBUG( EFalse, UiPanic( EPushMtmUiPanBadBioType ) );
            User::Leave( KErrNotSupported );
            }

        CleanupStack::PushL( si );
        si->RetrieveL( iMsvSession, iEntryId );

        PUSHLOG_WRITE("CPushViewMessageOp SI retrieved")

        __ASSERT_DEBUG( !iDocHandler, UiPanic( EPushMtmUiPanAlreadyInitialized ) );
        iDocHandler = CDocumentHandler::NewL( CEikonEnv::Static()->Process() );

        iDocHandler->SetExitObserver( this );       // (1)
        const TPtrC text = si->Text();
        // Copy the text to an 8-bit buffer:
        HBufC8* text8 = HBufC8::NewMaxLC( text.Length() );
        text8->Des().Copy( text );
        TDataType dataType( KTextPlainString );

        CAiwGenericParamList& paramList = iDocHandler->InParamListL();
        RFile tempFile;  
        iDocHandler->SaveTempFileL
            ( *text8, dataType, KNullDesC, tempFile );
        PUSHLOG_WRITE(" SaveTempFileL OK");
        CleanupClosePushL( tempFile );
        iDocHandler->OpenFileEmbeddedL( tempFile, dataType, paramList );
        PUSHLOG_WRITE(" OpenFileEmbeddedL OK");
        CleanupStack::PopAndDestroy( &tempFile ); // tempFile
        PUSHLOG_WRITE(" PopAndDestroy( &tempFile ) OK");

        CleanupStack::PopAndDestroy( 2, si ); // text8, si
        PUSHLOG_WRITE(" PopAndDestroy( 2, si ) OK");

        // Signal the observer when the viewer is terminated.
        // Until then we are observeing the message 
        // (delete & replacement notification).
        //ObserveEntryEventL();                       // (2)
        // Now we are observing two things: the viewer (1) and the entry (2).

        iStatus = KRequestPending;
        SetActive();
        }

    PUSHLOG_LEAVEFN("CPushViewMessageOp::RunL")
    }

// ---------------------------------------------------------
// CPushViewMessageOp::DoCancel
// ---------------------------------------------------------
//
void CPushViewMessageOp::DoCancel()
    {
    PUSHLOG_ENTERFN("CPushViewMessageOp::DoCancel");

    iIsCancelled = ETrue;

    CancelObserveEntryEvent();
    // If there is DocHandler, then it means that we are active and 
    // waiting for NotifyExit to be called.
    if ( iDocHandler )
        {
        // Terminate the embedded viewer. TODO
        // Complete us.
        InvokeRun();
        }

    SignalObserver( KErrCancel );

    PUSHLOG_LEAVEFN("CPushViewMessageOp::DoCancel");
    }

// ---------------------------------------------------------
// CPushViewMessageOp::RunError
// ---------------------------------------------------------
//
TInt CPushViewMessageOp::RunError( TInt aError )
    {
    PUSHLOG_WRITE_FORMAT("CPushViewMessageOp::RunError: %d",aError);

    CancelObserveEntryEvent();

    delete iDocHandler;
    iDocHandler = NULL;

    TRAP_IGNORE( ShowGlobalErrorNoteL( aError ) );
    return CPushMtmUiOperation::RunError( aError );
    }

// ---------------------------------------------------------
// CPushViewMessageOp::HandleEntryEventL
// ---------------------------------------------------------
//
void CPushViewMessageOp::HandleEntryEventL( TMsvEntryEvent aEvent, 
                         TAny* /*aArg1*/, TAny* /*aArg2*/, TAny* /*aArg3*/ )
    {
    PUSHLOG_ENTERFN("CPushViewMessageOp::HandleEntryEventL");

    if ( aEvent == EMsvEntryChanged )
        {
        PUSHLOG_WRITE(" Changed");
        // We don't have to bother with such case that the 
        // content changes, but the entry is saved as 'read' in 
        // this case, because this operation handles only SI messages.
        //TODO NotifyAndCancelL( R_PUSHMISC_INFO_REPLACED );
        }
    else if ( aEvent == EMsvEntryDeleted )
        {
        PUSHLOG_WRITE(" Deleted");
        //TODO NotifyAndCancelL( R_PUSHLS_EXPIRED_NOTE );
        }
    else
        {
        // Nothing to do.
        }

    PUSHLOG_LEAVEFN("CPushViewMessageOp::HandleEntryEventL");
    }

// ---------------------------------------------------------
// CPushViewMessageOp::NotifyAndCancelL
// ---------------------------------------------------------
//
void CPushViewMessageOp::NotifyAndCancelL( TInt aResId )
    {
    PUSHLOG_ENTERFN("CPushViewMessageOp::NotifyAndCancelL");

    // Add resource file.
    TParse* fileParser = new (ELeave) TParse;
    CleanupStack::PushL( fileParser );
    fileParser->Set( KPushMtmUiResourceFileAndDrive, &KDC_MTM_RESOURCE_DIR, NULL ); 
    AssureResourceL( fileParser->FullName() );
    CleanupStack::PopAndDestroy( fileParser ); // fileParser
    fileParser = NULL;

    // Show note about the message deletion.
    HBufC* noteText = iCoeEnv.AllocReadResourceLC( aResId );
    CAknInformationNote* note = new (ELeave) CAknInformationNote( ETrue );
    note->ExecuteLD( *noteText );
    CleanupStack::PopAndDestroy( noteText ); // noteText

    // Close the dialog.
    Cancel();

    PUSHLOG_LEAVEFN("CPushViewMessageOp::NotifyAndCancelL");
    }

// -----------------------------------------------------------------------------
// CPushViewMessageOp::HandleServerAppExit
// -----------------------------------------------------------------------------
//
void CPushViewMessageOp::HandleServerAppExit( TInt LOG_ONLY( aReason ) )
    {
    PUSHLOG_WRITE_FORMAT("CPushViewMessageOp::HandleServerAppExit: %d",aReason);

    if ( !iIsCancelled )
        {
        // Ready. The application was terminated normally.
        InvokeRun();
        }
    }

// End of file.

