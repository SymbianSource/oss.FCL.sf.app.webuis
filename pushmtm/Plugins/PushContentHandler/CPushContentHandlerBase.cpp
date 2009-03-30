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
* Description:  Implementation of CPushContentHandlerBase.
*
*/



// INCLUDE FILES

#include "CPushContentHandlerBase.h"
#include "PushMtmUtil.h"
#include "PushMtmSettings.h"
#include "PushInitiatorList.h"
#include "PushInitiator.h"
#include "PushMtmCommands.hrh"
#include "PushMtmLog.h"
#include "PushMtmUiDef.h"
#include "StringResourceReader.h"
#include "PushContentHandlerPanic.h"
#include <PushEntry.h>
#include <PushMessage.h>
#include <msvstd.h>
#include <mtclreg.h>
#include <mtclbase.h>
#include <data_caging_path_literals.hrh>
#include <f32file.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushContentHandlerBase::CPushContentHandlerBase
// ---------------------------------------------------------
//
CPushContentHandlerBase::CPushContentHandlerBase()
:   CContentHandlerBase()
	{
	}

// ---------------------------------------------------------
// CPushContentHandlerBase::ConstructL
// ---------------------------------------------------------
//
void CPushContentHandlerBase::ConstructL()
	{
    iMsvSession = CMsvSession::OpenSyncL( *this );
    PUSHLOG_WRITE("CPushContentHandlerBase iMsvSession OK")
	iWapPushUtils = CPushMtmUtil::NewL( *iMsvSession );
    PUSHLOG_WRITE("CPushContentHandlerBase iWapPushUtils OK")
    iMtmSettings = CPushMtmSettings::NewL();
    PUSHLOG_WRITE("CPushContentHandlerBase iMtmSettings OK")

    // Add resource file reader.
    TParse* fileParser = new (ELeave) TParse;
    CleanupStack::PushL( fileParser );
    fileParser->Set( KPushMtmUiResourceFileAndDrive, &KDC_MTM_RESOURCE_DIR, NULL ); 
    iStrRscReader = new (ELeave) CStringResourceReader
                    ( iMsvSession->FileSession(), fileParser->FullName() );
    CleanupStack::PopAndDestroy( fileParser ); // fileParser
    fileParser = NULL;
    PUSHLOG_WRITE("CPushContentHandlerBase iStrRscReader OK")

    CActiveScheduler::Add( this );
	}

// ---------------------------------------------------------
// CPushContentHandlerBase::~CPushContentHandlerBase
// ---------------------------------------------------------
//
CPushContentHandlerBase::~CPushContentHandlerBase()
	{
    PUSHLOG_ENTERFN("CPushContentHandlerBase::~CPushContentHandlerBase")

    // Call Cancel() in derived classes!
    delete iStrRscReader; // Must be deleted before iMsvSession.
    delete iMtmSettings; // It has to be deleted before iMsvSession, because 
    // it uses iMsvSession->FileSession().
	delete iWapPushUtils;
    delete iMsvSession;

    PUSHLOG_LEAVEFN("CPushContentHandlerBase::~CPushContentHandlerBase")
	}

// ---------------------------------------------------------
// CPushContentHandlerBase::DoCollectGarbageL
// ---------------------------------------------------------
//
void CPushContentHandlerBase::DoCollectGarbageL()
	{
    PUSHLOG_ENTERFN("CPushContentHandlerBase::DoCollectGarbageL")

    CClientMtmRegistry* clientReg = CClientMtmRegistry::NewL( *iMsvSession );
    CleanupStack::PushL( clientReg );
    PUSHLOG_WRITE("CPushContentHandlerBase clientReg OK")
    CBaseMtm* pushMtm = clientReg->NewMtmL( KUidMtmWapPush );
    CleanupStack::PushL( pushMtm );
    PUSHLOG_WRITE("CPushContentHandlerBase pushMtm OK")
    CMsvEntrySelection* dummySel = new (ELeave) CMsvEntrySelection;
    CleanupStack::PushL( dummySel );
    TBuf8<1> dummyPar;

    pushMtm->InvokeSyncFunctionL( EPushMtmCmdCollectGarbage, 
                                  *dummySel, dummyPar );

    CleanupStack::PopAndDestroy( 3, clientReg );
                                // dummySel, pushMtm, clientReg

    PUSHLOG_LEAVEFN("CPushContentHandlerBase::DoCollectGarbageL")
    }

// ---------------------------------------------------------
// CPushContentHandlerBase::FilterPushMsgL
// ---------------------------------------------------------
//
TBool CPushContentHandlerBase::FilterPushMsgL()
    {
    PUSHLOG_ENTERFN("CPushContentHandlerBase::FilterPushMsgL")

	TBool msgAccepted = ETrue;

    if ( iMtmSettings->ServiceReception() == EFalse )
        {
        // Discard message.
        msgAccepted = EFalse;
        PUSHLOG_WRITE(" ServiceReception OFF")
        }

    PUSHLOG_WRITE_FORMAT(" msgAccepted <%d>",msgAccepted)
    PUSHLOG_LEAVEFN("CPushContentHandlerBase::FilterPushMsgL")
    return msgAccepted;
    }

// ---------------------------------------------------------
// CPushContentHandlerBase::HandleSessionEventL
// ---------------------------------------------------------
//
void CPushContentHandlerBase::HandleSessionEventL
    ( 
        TMsvSessionEvent /*aEvent*/, 
        TAny* /*aArg1*/, 
        TAny* /*aArg2*/, 
        TAny* /*aArg3*/ 
    )
    {
    }
