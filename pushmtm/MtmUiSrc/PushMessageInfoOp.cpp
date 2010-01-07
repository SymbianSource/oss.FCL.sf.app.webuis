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
* Description:  Implementation of CPushMessageInfoOp.
*
*/

// INCLUDE FILES

#include "PushMessageInfoOp.h"
#include "PushMessageInfoDialog.h"
#include "PushMtmUtil.h"
#include "PushMtmUiDef.h"
#include "PushMtmUiPanic.h"
#include "PushMtmLog.h"
#include <AknNoteWrappers.h>
#include <PushMtmUi.rsg>
#include <data_caging_path_literals.hrh>
#include <f32file.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMessageInfoOp::CPushMessageInfoOp
// ---------------------------------------------------------
//
CPushMessageInfoOp::CPushMessageInfoOp( CMsvSession& aSession, 
                                        TMsvId aEntryId, 
                                        TBool aDontShowNotification, 
                                        TRequestStatus& aObserverStatus ) 
:   CPushMtmUiOperation( aSession, aEntryId, aObserverStatus ), 
    iDontShowNotification( aDontShowNotification ), 
    iObserverCompleted( EFalse )
    {
    }

// ---------------------------------------------------------
// CPushMessageInfoOp::~CPushMessageInfoOp
// ---------------------------------------------------------
//
CPushMessageInfoOp::~CPushMessageInfoOp()
    {
    Cancel();
    delete iDialog;
    if ( iDeletedFlag )
        {
		*iDeletedFlag = ETrue;
        }
    }

// ---------------------------------------------------------
// CPushMessageInfoOp::StartL
// ---------------------------------------------------------
//
void CPushMessageInfoOp::StartL()
    {
    Cancel();
    iObserverCompleted = EFalse;
    CPushMtmOperation::StartL();
    }

// ---------------------------------------------------------
// CPushMessageInfoOp::RunL
// ---------------------------------------------------------
//
void CPushMessageInfoOp::RunL()
    {
    __ASSERT_DEBUG( !iDialog, UiPanic( EPushMtmUiPanAlreadyInitialized ) );

    if ( iObserverCompleted )
        {
        return;
        }

    TBool deleted( EFalse );
    iDeletedFlag = &deleted;

    // Time to set up entry observation (delete & replacement case).
    ObserveEntryEventL();

    iDialog = new (ELeave) CPushMessageInfoDialog;
    // Workaround for Cancel(): SetActive() is called.
    // CMsvSingleOpWatcher::DoCancel() will wait for this observed object to 
    // complete that. iDialog->ExecuteLD() allows other RunL's to run, so 
    // that it may happen that CMsvSingleOpWatcher::DoCancel() is called 
    // when iDialog->ExecuteLD() is running. It means that this object must 
    // be active in that time in order DoCancel() will be called.
    iStatus = KRequestPending;
    SetActive();
    iDialog->ExecuteLD( iMsvSession, iEntryId );

    // Ready.
    if ( !deleted )
        {
        iDialog = NULL;
        iDeletedFlag = NULL;
        // Consider what happens if this object is cancelled in ExecuteLD!
        // In this case the observer is already completed (DoCancel), so 
        // avoid completeing it twice. iObserverCompleted indicates it.
        if ( !iObserverCompleted )
            {
            SignalObserver( KErrNone );
            iObserverCompleted = ETrue;
            // Due to SetActive() call RunL again, but it will do nothing 
            // (see above).
            InvokeRun();
            }
        }
    }

// ---------------------------------------------------------
// CPushMessageInfoOp::DoCancel
// ---------------------------------------------------------
//
void CPushMessageInfoOp::DoCancel()
    {
    PUSHLOG_WRITE("CPushMessageInfoOp::DoCancel");
    CancelObserveEntryEvent();

    if ( iDialog )
        {
        delete iDialog;
        iDialog = NULL;
        // Due to SetActive() in RunL we must call InvokeRun(): the dialog 
        // does not completes this operation.
        InvokeRun();
        }
    if ( !iObserverCompleted )
        {
        SignalObserver( KErrCancel );
        iObserverCompleted = ETrue;
        }
    }

// ---------------------------------------------------------
// CPushMessageInfoOp::RunError
// ---------------------------------------------------------
//
TInt CPushMessageInfoOp::RunError( TInt aError )
    {
    PUSHLOG_WRITE_FORMAT("CPushMessageInfoOp::RunError: %d",aError);
    CancelObserveEntryEvent();

    if ( !iObserverCompleted )
        {
        SignalObserver( aError );
        iObserverCompleted = ETrue;
        }
    return aError;
    }

// ---------------------------------------------------------
// CPushMessageInfoOp::HandleEntryEventL
// ---------------------------------------------------------
//
void CPushMessageInfoOp::HandleEntryEventL( TMsvEntryEvent aEvent, 
                         TAny* /*aArg1*/, TAny* /*aArg2*/, TAny* /*aArg3*/ )
    {
    PUSHLOG_ENTERFN("CPushMessageInfoOp::HandleEntryEventL");

    if ( aEvent == EMsvEntryChanged )
        {
        PUSHLOG_WRITE(" Changed");
        HandleEntryChangeL();
        }
    else if ( aEvent == EMsvEntryDeleted )
        {
        PUSHLOG_WRITE(" Deleted");
        NotifyAndCancelL( R_PUSHLS_EXPIRED_NOTE );
        }
    else
        {
        // Nothing to do.
        }

    PUSHLOG_LEAVEFN("CPushMessageInfoOp::HandleEntryEventL");
    }

// ---------------------------------------------------------
// CPushMessageInfoOp::HandleEntryChangeL
// ---------------------------------------------------------
//
void CPushMessageInfoOp::HandleEntryChangeL()
    {
    PUSHLOG_ENTERFN("CPushMessageInfoOp::HandleEntryChangeL");

    // We have to act only in case of such changes where the content 
    // of the message changes, not only the unread/read flag.
    // The content handlers and the Push subsystem behave so that 
    // the content is changed only when the entry becomes 'unread', 
    // except one case where the CH changes the content, but the 
    // message becomes 'read' (SL-execute-high). In this case 
    // a flag indicates that the content was changed.

    // Get an up-to-date entry and check the necessary flags:
    TMsvEntry tEntry;
    TMsvId service;
    User::LeaveIfError( iMsvSession.GetEntry( iEntryId, service, tEntry ) );
    TBool isChangeToUnread = tEntry.Unread();
    TBool contentChangedFlagSet = 
        CPushMtmUtil::Attrs( tEntry ) & EPushMtmReadButContentChanged;

    if ( !isChangeToUnread && !contentChangedFlagSet )
        {
        // Nothing to do. Somebody has just marked it 'read'.
        PUSHLOG_WRITE(" Content not changed");
        }
    else
        {
        NotifyAndCancelL( R_PUSHMISC_INFO_REPLACED );
        }

    PUSHLOG_LEAVEFN("CPushMessageInfoOp::HandleEntryChangeL")
    }

// ---------------------------------------------------------
// CPushMessageInfoOp::NotifyAndCancelL
// ---------------------------------------------------------
//
void CPushMessageInfoOp::NotifyAndCancelL( TInt aResId )
    {
    PUSHLOG_ENTERFN("CPushMessageInfoOp::NotifyAndCancelL");

    if ( iDontShowNotification )
        {
        // Don't show.
        }
    else
        {
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
        }

    // Close the dialog anyway.
    Cancel();

    PUSHLOG_LEAVEFN("CPushMessageInfoOp::NotifyAndCancelL");
    }

// End of file.

