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
* Description:  Implementation of CPushMtmGcOperation.
*
*/



// INCLUDE FILES

#include "PushMtmGcOperation.h"
#include "PushMtmCliSrvPanic.h"
#include "PushMtmUtil.h"
#include "PushMtmLog.h"
#include <CSIPushMsgEntry.h>
#include <msvids.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMtmGcOperation::NewL
// ---------------------------------------------------------
//
CPushMtmGcOperation* CPushMtmGcOperation::NewL(
                                CMsvSession& aSession,
                                TMsvId aFolderId,
                                TRequestStatus& aObserverRequestStatus )
    {
    PUSHLOG_ENTERFN("CPushMtmGcOperation::NewL")
    
    CPushMtmGcOperation* self = new (ELeave) CPushMtmGcOperation
        ( aSession, aFolderId, aObserverRequestStatus );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop(); // self
    
    PUSHLOG_LEAVEFN("CPushMtmGcOperation::NewL")
    return self;
    }

// ---------------------------------------------------------
// CPushMtmGcOperation::~CPushMtmGcOperation
// ---------------------------------------------------------
//
CPushMtmGcOperation::~CPushMtmGcOperation()
    {
    PUSHLOG_ENTERFN("CPushMtmGcOperation::~CPushMtmGcOperation")
    
    Cancel();
    delete iEntrySel;
    delete iEntryWrapper;
    delete iUtil;
    
    PUSHLOG_LEAVEFN("CPushMtmGcOperation::~CPushMtmGcOperation")
    }

// ---------------------------------------------------------
// CPushMtmGcOperation::DoSyncL
// ---------------------------------------------------------
//
void CPushMtmGcOperation::DoSyncL()
    {
    PUSHLOG_ENTERFN("CPushMtmGcOperation::DoSyncL")
    
    InitL();
    
    for ( iCurrentIndex = 0; iCurrentIndex < iCount; ++iCurrentIndex )
        {
        // iCurrentIndex indicate the current entry in the selection.
        GcCurrentL();
        }

    // Release unnecessary resources.
    delete iEntrySel;
    iEntrySel = NULL;
    
    PUSHLOG_LEAVEFN("CPushMtmGcOperation::DoSyncL")
    }

// ---------------------------------------------------------
// CPushMtmGcOperation::CPushMtmGcOperation
// ---------------------------------------------------------
//
CPushMtmGcOperation::CPushMtmGcOperation( CMsvSession& aSession, 
                                          TMsvId aFolderId, 
                                          TRequestStatus& aObserverRequestStatus ) 
:   CPushMtmOperation( aSession, KMsvNullIndexEntryId, aObserverRequestStatus ), 
    iState( EInit ), iFolderId( aFolderId ), iCurrentIndex( 0 )
    {
    }

// ---------------------------------------------------------
// CPushMtmGcOperation::ConstructL
// ---------------------------------------------------------
//
void CPushMtmGcOperation::ConstructL()
    {
    // Do nothing here.
    }

// ---------------------------------------------------------
// CPushMtmGcOperation::InitL
// ---------------------------------------------------------
//
void CPushMtmGcOperation::InitL()
    {
    PUSHLOG_ENTERFN("CPushMtmGcOperation::InitL")
    
    if ( !iUtil )
        {
        iUtil = CPushMtmUtil::NewL( iMsvSession );
        }

    CMsvEntrySelection* folders = NULL;
    if ( iFolderId == KMsvNullIndexEntryId )
        {
        folders = iUtil->FindPushFoldersL();
        CleanupStack::PushL( folders );
        }
    else
        {
        folders = new (ELeave) CMsvEntrySelection;
        CleanupStack::PushL( folders );
        folders->AppendL( iFolderId );
        }

    // Get SIs from the given folder(s).
    delete iEntrySel;
    iEntrySel = NULL;
    iEntrySel = iUtil->FindMessagesL( *folders, KUidWapPushMsgSI );
    CleanupStack::PopAndDestroy(); // folders

    // Reset count.
    iCount = iEntrySel->Count();
    PUSHLOG_WRITE_FORMAT(" iCount: %d",iCount)
    // Reset index.
    iCurrentIndex = 0;

    // We need a wrapper only if there is entry to delete.
    if ( iCount && !iEntryWrapper )
        {
        iEntryWrapper = CSIPushMsgEntry::NewL();
        }
        
    PUSHLOG_LEAVEFN("CPushMtmGcOperation::InitL")
    }

// ---------------------------------------------------------
// CPushMtmGcOperation::GcCurrentL
// ---------------------------------------------------------
//
void CPushMtmGcOperation::GcCurrentL()
    {
    PUSHLOG_ENTERFN("CPushMtmGcOperation::GcCurrentL")
    
    PUSHLOG_WRITE_FORMAT(" iCurrentIndex: %d",iCurrentIndex)
    TMsvId entryId( iEntrySel->At(iCurrentIndex) );

#ifdef _DEBUG
    TMsvEntry tEntry;
    TMsvId dummyService;
    User::LeaveIfError
        ( iMsvSession.GetEntry( entryId, dummyService, tEntry ) );

    __ASSERT_DEBUG( tEntry.iMtm == KUidMtmWapPush && 
                    tEntry.iBioType == KUidWapPushMsgSI.iUid, 
                    CliSrvPanic( EPushMtmCliSrvPanBadType ) );
#endif // _DEBUG

    iEntryWrapper->RetrieveL( iMsvSession, entryId );
    // Check expiration if expiration time was set.
    if ( iEntryWrapper->Expires() != Time::NullTTime() )
        {
	    TTime today;
	    today.UniversalTime();
        if ( iEntryWrapper->Expires() < today )
		    {
		    PUSHLOG_WRITE(" Expired")
            // The message has expired. Delete it.
            CMsvEntry* cParent = 
                iMsvSession.GetEntryL( iEntryWrapper->Entry().Parent() );
            CleanupStack::PushL( cParent );
            // Delete the message.
            cParent->DeleteL( entryId );
            CleanupStack::PopAndDestroy(); // cParent
            }
        }
        
    PUSHLOG_LEAVEFN("CPushMtmGcOperation::GcCurrentL")
    }

// ---------------------------------------------------------
// CPushMtmGcOperation::RunL
// ---------------------------------------------------------
//
void CPushMtmGcOperation::RunL()
    {
    PUSHLOG_ENTERFN("CPushMtmGcOperation::RunL")
    
    if ( iState == EInit )
        {
        InitL();
        if ( !iCount )
            {
            // Ready.
            SignalObserver( KErrNone );
            }
        else
            {
            iState = EGarbageCollecting;
            SetActive();
            InvokeRun();
            }
        }
    else if ( iState == EGarbageCollecting )
        {
        __ASSERT_DEBUG( iCount, 
            CliSrvPanic( EPushMtmCliSrvPanEmptySelection ) );
        // iCurrentIndex indicate the current entry in the selection.
        GcCurrentL();
        if ( ++iCurrentIndex < iCount )
            {
            // Delete next.
            SetActive();
            InvokeRun();
            }
        else
            {
            // No more to delete.
            SignalObserver( KErrNone );
            }
        }
    else
        {
        __ASSERT_DEBUG( EFalse, 
            CliSrvPanic( EPushMtmCliSrvPanCommandNotSupported ) );
        }
        
    PUSHLOG_LEAVEFN("CPushMtmGcOperation::RunL")
    }

// ---------------------------------------------------------
// CPushMtmGcOperation::DoCancel
// ---------------------------------------------------------
//
void CPushMtmGcOperation::DoCancel()
    {
    PUSHLOG_ENTERFN("CPushMtmGcOperation::DoCancel")
    
    CPushMtmOperation::DoCancel();
    
    PUSHLOG_LEAVEFN("CPushMtmGcOperation::DoCancel")
    }

// ---------------------------------------------------------
// CPushMtmGcOperation::RunError
// ---------------------------------------------------------
//
TInt CPushMtmGcOperation::RunError( TInt aError )
    {
    PUSHLOG_ENTERFN("CPushMtmGcOperation::RunError")
    PUSHLOG_WRITE_FORMAT(" aError: %d",aError)
    
    TBool doContinue( EFalse );

    PUSHLOG_WRITE_FORMAT(" iState: %d",iState)
    if ( iState == EInit )
        {
        // We cannot continue.
        }
    else if ( iState == EGarbageCollecting )
        {
        if ( aError == KErrNotFound )
            {
            // Try to continue with next.
            if ( ++iCurrentIndex < iCount )
                {
                // Delete next.
                doContinue = ETrue;
                SetActive();
                InvokeRun();
                }
            }
        }
    else
        {
        __ASSERT_DEBUG( EFalse, 
            CliSrvPanic( EPushMtmCliSrvPanCommandNotSupported ) );
        }

    TInt ret = KErrNone;
    if ( doContinue )
        {
        ret = KErrNone;
        }
    else
        {
        // Report error.
        ret = CPushMtmOperation::RunError( aError );
        }

    PUSHLOG_LEAVEFN("CPushMtmGcOperation::RunError")
    return ret;
    }

// End of file.
