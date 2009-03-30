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
* Description:  Ui Mtm class definition
*
*/



// INCLUDE FILES

#include <bldvariant.hrh>
#include "PushMtmUi.h"
#include "PushMtmUiPanic.h"
#include "PushMtmUiDef.h"
#include "PushViewerDef.h"
#include "PushMtmCommands.hrh"
#include "PushMtmSettings.h"
#include "PushSettingsDialog.h"
#include "PushMessageInfoOp.h"
#include "PushLoadServiceOp.h"
#ifdef __SERIES60_PUSH_SP
#include "PushViewServiceOp.h"
#endif // __SERIES60_PUSH_SP
#include "PushViewMessageOp.h"
#include "PushMtmLog.h"
#include "PushMtmDef.h"
#include <PushEntry.h>
#include <msvids.h>
#include <msvuids.h>
#include <msvapi.h>
#include <mtmuidef.hrh>
#include <MTMExtendedCapabilities.hrh>
#include <MuiuMsgEditorLauncher.h>
#include <data_caging_path_literals.hrh>
#include <f32file.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushMtmUi::NewL
// ---------------------------------------------------------
//
EXPORT_C CPushMtmUi* CPushMtmUi::NewL( CBaseMtm& aBaseMtm, 
                                       CRegisteredMtmDll& aRegisteredMtmDll )
    {
    CPushMtmUi* ui = new (ELeave) CPushMtmUi( aBaseMtm, aRegisteredMtmDll );
    CleanupStack::PushL( ui );
    ui->ConstructL();
    CleanupStack::Pop();    // ui
    return ui;
    }

// ---------------------------------------------------------
// CPushMtmUi::~CPushMtmUi
// ---------------------------------------------------------
//
CPushMtmUi::~CPushMtmUi()
    {
    }

// ---------------------------------------------------------
// CPushMtmUi::CPushMtmUi
// ---------------------------------------------------------
//
CPushMtmUi::CPushMtmUi( CBaseMtm& aBaseMtm, 
                        CRegisteredMtmDll& aRegisteredMtmDll )
:   CBaseMtmUi( aBaseMtm, aRegisteredMtmDll )
    {
    }

// ---------------------------------------------------------
// CPushMtmUi::SetProgressSuccess
// ---------------------------------------------------------
//
void CPushMtmUi::SetProgressSuccess
( TPckgBuf<TMsvLocalOperationProgress>& aProgress, TMsvId aId )
    {
    aProgress().iTotalNumberOfEntries = 1;
    aProgress().iNumberCompleted = 1;
    aProgress().iId = aId;
    }

// ---------------------------------------------------------
// CPushMtmUi::CompletedOperationL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::CompletedOperationL
( TRequestStatus& aObserverStatus )
    {
    TPckgBuf<TMsvLocalOperationProgress> progress;
    SetProgressSuccess( progress, BaseMtm().Entry().Entry().Id() );
    CMsvCompletedOperation* operation = 
        CMsvCompletedOperation::NewL( Session(), Type(), progress, 
        KMsvLocalServiceIndexEntryId, aObserverStatus );

    return operation;
    }

// ---------------------------------------------------------
// CPushMtmUi::EnsureServiceEntryL
// ---------------------------------------------------------
//
void CPushMtmUi::EnsureServiceEntryL() const
    {
    PUSHLOG_ENTERFN("CPushMtmUi::EnsureServiceEntryL")

    // Get root entry.
    CMsvEntry* rootMsvEntry = Session().GetEntryL( KMsvRootIndexEntryId );
    CleanupStack::PushL( rootMsvEntry );

    // The Push MTM service entry is invisible. Set selection ordering
    // to find the invisible entries also.
    TMsvSelectionOrdering msvSelectionOrdering = rootMsvEntry->SortType();;
    msvSelectionOrdering.SetShowInvisibleEntries( ETrue );
    rootMsvEntry->SetSortTypeL( msvSelectionOrdering );

    // The service entry is the child entry of the root entry.
    // KUidMtmWapPush is used to identify the mtm type of the service entry!
    CMsvEntrySelection* msvEntrySelection = 
        rootMsvEntry->ChildrenWithMtmL( KUidMtmWapPush );
    CleanupStack::PushL( msvEntrySelection );

    const TInt serviceEntryCount = msvEntrySelection->Count();
    PUSHLOG_WRITE_FORMAT(" serviceEntryCount <%d>",serviceEntryCount)
    __ASSERT_DEBUG( serviceEntryCount <= 1, 
		            UiPanic( EPushMtmUiPanTooManyServiceEntries ) );
    // If there is no push service entry in Message Server, create one.
    // If there is more than one, then delete the unnecessary entries.
    if ( serviceEntryCount == 0 )
        {
    	TMsvEntry newEntryData;
	    newEntryData.iType = KUidMsvServiceEntry;
	    newEntryData.iMtm = KUidMtmWapPush;
        newEntryData.SetVisible( EFalse );
        rootMsvEntry->CreateL( newEntryData );
        }
    else if ( 1 < serviceEntryCount )
        {
        for ( TInt i = 1; i < serviceEntryCount; ++i ) // i starts from 1.
            {
            rootMsvEntry->DeleteL( msvEntrySelection->At(i) );
            }
        }

    CleanupStack::PopAndDestroy( 2, rootMsvEntry );
                               // msvEntrySelection, rootMsvEntry

    PUSHLOG_LEAVEFN("CPushMtmUi::EnsureServiceEntryL")
    }

// ---------------------------------------------------------
// CPushMtmUi::CreateL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::CreateL(
        const TMsvEntry& /*aEntry*/,
        CMsvEntry& /*aParent*/,
        TRequestStatus& /*aStatus*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUi::OpenL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::OpenL( TRequestStatus& aStatus )
    {
    __ASSERT_DEBUG( BaseMtm().Entry().Entry().iMtm == Type(), 
		            UiPanic( EPushMtmUiPanBadMtm ) );

    const TUid type( BaseMtm().Entry().Entry().iType );

    CMsvOperation* operation = NULL;

    if ( type == KUidMsvMessageEntry )
        {
        operation = ViewL( aStatus );
        }
    else
        {
        User::Leave( KErrNotSupported );
        }

    return operation;
    }

// ---------------------------------------------------------
// CPushMtmUi::CloseL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::CloseL( TRequestStatus& aStatus )
    {
    __ASSERT_DEBUG( BaseMtm().Entry().Entry().iType == KUidMsvMessageEntry, 
                    UiPanic( EPushMtmUiPanBadType ) );
    return CompletedOperationL( aStatus );
    }

// ---------------------------------------------------------
// CPushMtmUi::EditL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::EditL( TRequestStatus& aStatus )
    {
    __ASSERT_DEBUG( BaseMtm().Entry().Entry().iMtm == Type(), 
		UiPanic( EPushMtmUiPanBadMtm ) );

    if ( BaseMtm().Entry().Entry().iType != KUidMsvServiceEntry )
        {
        // All push messages are read only ones.
        User::Leave( KErrNotSupported );
        }

    CPushSettingsDialog* dlg = new (ELeave) CPushSettingsDialog( Session() );
    dlg->ExecuteLD();
    return CompletedOperationL( aStatus );
    }

// ---------------------------------------------------------
// CPushMtmUi::ViewL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::ViewL( TRequestStatus& aStatus )
    {
    const TMsvEntry tEntry( BaseMtm().Entry().Entry() );

    __ASSERT_DEBUG( tEntry.iMtm == Type(), UiPanic( EPushMtmUiPanBadMtm ) );
    __ASSERT_DEBUG( tEntry.iType == KUidMsvMessageEntry, 
                                           UiPanic( EPushMtmUiPanBadType ) );

    if ( tEntry.iType != KUidMsvMessageEntry )
        {
        User::Leave( KErrNotSupported );
        }

    CMsvOperation* operation = NULL;

    if ( 
         tEntry.iBioType == KUidWapPushMsgSI.iUid 
#ifdef __SERIES60_PUSH_SL
         || tEntry.iBioType == KUidWapPushMsgSL.iUid
#endif // __SERIES60_PUSH_SL
       )
        {
        TEditorParameters editorParameters;
        editorParameters.iId = tEntry.Id();
        editorParameters.iFlags = EMsgLaunchEditorThenWait | 
                                  EMsgReadOnly | 
                                  EMsgLaunchEditorEmbedded;

        // Get the application name.
        HBufC* appFileName = 
            MsvUiEditorUtilities::ResolveAppFileNameL( KUidPushViewerApp );
        CleanupStack::PushL( appFileName );

        operation = MsgEditorLauncher::LaunchEditorApplicationL
                                                        ( BaseMtm().Session(), 
                                                          tEntry.iMtm, 
                                                          aStatus, 
                                                          editorParameters, 
                                                          *appFileName );

        CleanupStack::PopAndDestroy( appFileName ); // appFileName
        }

#ifdef __SERIES60_PUSH_SP

    else if ( tEntry.iBioType == KUidWapPushMsgUnknown.iUid )
        {
        // Don't launch the PushViewer, but the embedded viewer.
        // It's unnecessary to call 
        // BaseMtm().SwitchCurrentEntryL( tEntry.Id() );
        TBuf8<1> dummyPar;
        CMsvEntrySelection* dummySel = new (ELeave) CMsvEntrySelection;
        CleanupStack::PushL( dummySel );

        operation = InvokeAsyncFunctionL( EPushMtmCmdViewService, 
                                          *dummySel, 
                                          aStatus, 
                                          dummyPar );

        CleanupStack::PopAndDestroy( dummySel ); // dummySel
        }

#endif // __SERIES60_PUSH_SP

    else
        {
        __ASSERT_DEBUG( EFalse, UiPanic( EPushMtmUiPanBadBioType ) );
        User::Leave( KErrNotSupported );
        }

    return operation;
    }

// ---------------------------------------------------------
// CPushMtmUi::OpenL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::OpenL
( TRequestStatus& /*aStatus*/, const CMsvEntrySelection& /*aSelection*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUi::CloseL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::CloseL
( TRequestStatus& /*aStatus*/, const CMsvEntrySelection& /*aSelection*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUi::EditL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::EditL
( TRequestStatus& /*aStatus*/, const CMsvEntrySelection& /*aSelection*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUi::ViewL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::ViewL
( TRequestStatus& /*aStatus*/, const CMsvEntrySelection& /*aSelection*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUi::CancelL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::CancelL
( TRequestStatus& /*aStatus*/, const CMsvEntrySelection& /*aSelection*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUi::DeleteFromL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::DeleteFromL
( const CMsvEntrySelection& /*aSelection*/, TRequestStatus& /*aStatus*/ )
	{
    User::Leave( KErrNotSupported );
    return NULL;
	}

// ---------------------------------------------------------
// CPushMtmUi::UnDeleteFromL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::UnDeleteFromL
( const CMsvEntrySelection& /*aSelection*/, TRequestStatus& /*aStatus*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUi::DeleteServiceL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::DeleteServiceL
( const TMsvEntry& /*aService*/, TRequestStatus& /*aStatus*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUi::ReplyL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::ReplyL(
        TMsvId /*aDestination*/,
        TMsvPartList /*aPartlist*/,
        TRequestStatus& /*aCompletionStatus*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUi::ForwardL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::ForwardL(
        TMsvId /*aDestination*/,
        TMsvPartList /*aPartlist*/,
        TRequestStatus& /*aCompletionStatus*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUi::CopyToL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::CopyToL
( const CMsvEntrySelection& /*aSelection*/, TRequestStatus& /*aStatus*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }


// ---------------------------------------------------------
// CPushMtmUi::MoveToL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::MoveToL
( const CMsvEntrySelection& /*aSelection*/, TRequestStatus& /*aStatus*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }


// ---------------------------------------------------------
// CPushMtmUi::CopyFromL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::CopyFromL(
        const CMsvEntrySelection& /*aSelection*/,
        TMsvId /*aTargetId*/,
        TRequestStatus& /*aStatus*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUi::MoveFromL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::MoveFromL(
        const CMsvEntrySelection& /*aSelection*/,
        TMsvId /*aTargetId*/,
        TRequestStatus& /*aStatus*/ )
    {
    User::Leave( KErrNotSupported );
    return NULL;
    }

// ---------------------------------------------------------
// CPushMtmUi::QueryCapability
// ---------------------------------------------------------
//
TInt CPushMtmUi::QueryCapability( TUid aCapability, TInt& aResponse )
    {
    TInt ret( KErrNone );

    switch ( aCapability.iUid )
        {
		case KUidMsvMtmUiQueryMessagingInitialisation:
		case KUidMsvMtmQueryFactorySettings:
		case KUidMsvMtmQueryMessageInfo:
			{
			aResponse = ETrue;
			break;
			}
        default:
            {
            ret = BaseMtm().QueryCapability( aCapability, aResponse );
            break;
            }
        }

    return ret;
    }

// ---------------------------------------------------------
// CPushMtmUi::InvokeSyncFunctionL
// ---------------------------------------------------------
//
void CPushMtmUi::InvokeSyncFunctionL( TInt aFunctionId, 
                                      const CMsvEntrySelection& /*aSelection*/, 
                                      TDes8& /*aParameter*/ )
    {
    PUSHLOG_WRITE_FORMAT
        ("CPushMtmUi::InvokeSyncFunctionL <%d>",aFunctionId)

    if ( aFunctionId == KMtmUiMessagingInitialisation || 
         aFunctionId == KMtmUiFunctionRestoreFactorySettings )
        {
        // Restore the settings...
        CPushMtmSettings* pushMtmSettings = CPushMtmSettings::NewLC();
        pushMtmSettings->RestoreFactorySettingsL();
        CleanupStack::PopAndDestroy( pushMtmSettings ); // pushMtmSettings
        // ...and ensure that a service entry exists with which 
        // the settings can be reached from a messaging application.
        EnsureServiceEntryL();
        }
    else
        {
        User::Leave( KErrNotSupported );
        }
    }

// ---------------------------------------------------------
// CPushMtmUi::InvokeAsyncFunctionL
// ---------------------------------------------------------
//
CMsvOperation* CPushMtmUi::InvokeAsyncFunctionL
                           ( TInt aFunctionId, 
                             const CMsvEntrySelection& aSelection, 
                             TRequestStatus& aCompletionStatus, 
                             TDes8& aParameter )
    {
    PUSHLOG_WRITE_FORMAT
        ("CPushMtmUi::InvokeAsyncFunctionL <%d>",aFunctionId)

    CMsvOperation* operation = NULL;

	switch ( aFunctionId )
		{
        case KMtmUiFunctionMessageInfo:
            {
            TBool dontShowNotification = EFalse;
            if ( 0 < aParameter.Length() )
                {
                TMessageInfoParam par;
                TMessageInfoParamPckg parPckg( par );
                parPckg.Set( aParameter );
                dontShowNotification = par.iDontShowNotification;
                }

            CPushMessageInfoOp* op = new (ELeave) CPushMessageInfoOp
                                                  ( Session(), 
                                                  BaseMtm().Entry().EntryId(), 
                                                  dontShowNotification, 
                                                  aCompletionStatus );
            CleanupStack::PushL( op );
            op->StartL();
            operation = op;
            CleanupStack::Pop( op ); // op
            break;
            }

        case EPushMtmCmdLoadService:
            {
            CPushLoadServiceOp* op = CPushLoadServiceOp::NewL
                                                  ( Session(), 
                                                  BaseMtm().Entry().EntryId(), 
                                                  aCompletionStatus );
            CleanupStack::PushL( op );
            op->StartL();
            operation = op;
            CleanupStack::Pop( op ); // op
            break;
            }

#ifdef __SERIES60_PUSH_SP

        case EPushMtmCmdViewService:
            {
            CPushViewServiceOp* op = new (ELeave) CPushViewServiceOp
                                                  ( Session(), 
                                                  BaseMtm().Entry().EntryId(), 
                                                  aCompletionStatus );
            CleanupStack::PushL( op );
            op->StartL();
            operation = op;
            CleanupStack::Pop( op ); // op
            break;
            }

#endif // __SERIES60_PUSH_SP

        case EPushMtmCmdViewMessage:
            {
            CPushViewMessageOp* op = CPushViewMessageOp::NewL
                                                  ( Session(), 
                                                  BaseMtm().Entry().EntryId(), 
                                                  aCompletionStatus );
            CleanupStack::PushL( op );
            op->StartL();
            operation = op;
            CleanupStack::Pop( op ); // op
            break;
            }

        default:
            {
            operation = CBaseMtmUi::InvokeAsyncFunctionL
                ( aFunctionId, aSelection, aCompletionStatus, aParameter );
            break;
            }
        }

    return operation;
    }

// ---------------------------------------------------------
// CPushMtmUi::GetResourceFileName
// ---------------------------------------------------------
//
void CPushMtmUi::GetResourceFileName( TFileName& aFileName ) const
    {
    // Extract the file name and extension from the constant
    TParsePtrC fileParser( KPushMtmUiResourceFileAndDrive );
    aFileName = fileParser.NameAndExt();
    }

// End of File
