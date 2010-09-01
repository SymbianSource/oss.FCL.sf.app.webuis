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
* Description:  Implementation of CPushViewerDocument.
*
*/



// INCLUDE FILES

#include "PushViewerDoc.h"
#include <bldvariant.hrh>
#include "PushViewerMsgEdAppUi.h"
#include "PushMtmCommands.hrh"
#include "PushMtmDef.h"
#include <msvuids.h>
#include <MuiuMsvSingleOpWatcher.h>
#include <msvstd.h>
#include <mtmuibas.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushViewerDocument::NewL
// ---------------------------------------------------------
//
CPushViewerDocument* CPushViewerDocument::NewL( CEikApplication& aApp )
    {
    CPushViewerDocument* doc = new (ELeave) CPushViewerDocument( aApp );
    CleanupStack::PushL( doc );
    doc->ConstructL();
    CleanupStack::Pop();
    return doc;
    }
    
// ---------------------------------------------------------
// CPushViewerDocument::IssueLoadServiceL
// ---------------------------------------------------------
//
void CPushViewerDocument::IssueLoadServiceL()
    {
    CMsvSingleOpWatcher* opWatcher = CMsvSingleOpWatcher::NewLC( *this );

    TBuf8<1> dummyPar;
    CMsvEntrySelection* dummySel = new (ELeave) CMsvEntrySelection;
    CleanupStack::PushL( dummySel );

    CMsvOperation* operation = MtmUi().InvokeAsyncFunctionL
                                           ( EPushMtmCmdLoadService, 
                                             *dummySel, 
                                             opWatcher->iStatus, 
                                             dummyPar );

    CleanupStack::PopAndDestroy( dummySel ); // dummySel

    // Observe the operation.
    AddSingleOperationL( operation, opWatcher );
    CleanupStack::Pop( opWatcher ); // opWatcher
    }

// ---------------------------------------------------------
// CPushViewerDocument::IssueMessageInfoL
// ---------------------------------------------------------
//
void CPushViewerDocument::IssueMessageInfoL()
    {
    CMsvSingleOpWatcher* opWatcher = CMsvSingleOpWatcher::NewLC( *this );

    TMessageInfoParam par;
    par.iDontShowNotification = ETrue;
    TMessageInfoParamBuf parBuf( par );

    CMsvEntrySelection* dummySel = new (ELeave) CMsvEntrySelection;
    CleanupStack::PushL( dummySel );

    CMsvOperation* operation = MtmUi().InvokeAsyncFunctionL
                                       ( KMtmUiFunctionMessageInfo, 
                                         *dummySel, 
                                         opWatcher->iStatus, 
                                         parBuf );
    CleanupStack::PopAndDestroy( dummySel ); // dummySel

    // See MsgEditorDocument.cpp about the usage of the following 
    // method.
    AddSingleOperationL( operation, opWatcher );
    CleanupStack::Pop( opWatcher ); // opWatcher
    }

// ---------------------------------------------------------
// CPushViewerDocument::CPushViewerDocument
// ---------------------------------------------------------
//
CPushViewerDocument::CPushViewerDocument( CEikApplication& aApp )
:   CMsgEditorDocument( aApp )
    {
    }

// ---------------------------------------------------------
// CPushViewerDocument::CreateAppUiL
// ---------------------------------------------------------
//
CEikAppUi* CPushViewerDocument::CreateAppUiL()
    {
    // Create a MsgEditor based view.
    return new (ELeave) CPushViewerMsgEdAppUi;
    }

// ---------------------------------------------------------
// CPushViewerDocument::DefaultMsgFolder
// ---------------------------------------------------------
//
TMsvId CPushViewerDocument::DefaultMsgFolder() const
    {
    return KMsvDraftEntryIdValue;
    }

// ---------------------------------------------------------
// CPushViewerDocument::DefaultMsgService
// ---------------------------------------------------------
//
TMsvId CPushViewerDocument::DefaultMsgService() const
    {
    return KMsvLocalServiceIndexEntryId;
    }

// ---------------------------------------------------------
// CPushViewerDocument::CreateNewL
// ---------------------------------------------------------
//
TMsvId CPushViewerDocument::CreateNewL( TMsvId /*aService*/, 
                                        TMsvId /*aFolder*/ )
    {
    return 0;
    }

// ---------------------------------------------------------
// CPushViewerDocument::EntryChangedL
// ---------------------------------------------------------
//
void CPushViewerDocument::EntryChangedL()
    {
    }

// End of file.
