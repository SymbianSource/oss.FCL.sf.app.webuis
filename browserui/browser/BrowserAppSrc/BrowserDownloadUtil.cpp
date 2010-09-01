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
* Description:  Download handling utilities.
*
*/


// INCLUDE FILES

#include "BrowserDownloadUtil.h"
#include "logger.h"
#include "ApiProvider.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CAsyncDownloadsListExecuter::CAsyncDownloadsListExecuter
// ---------------------------------------------------------
//
CAsyncDownloadsListExecuter::CAsyncDownloadsListExecuter( MApiProvider& aApiProvider )
:   CActive( EPriorityStandard ), 
    iApiProvider( aApiProvider )
    {
LOG_ENTERFN("CAsyncDownloadsListExecuter::CAsyncDownloadsListExecuter");
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------
// CAsyncDownloadsListExecuter::~CAsyncDownloadsListExecuter
// ---------------------------------------------------------
//
CAsyncDownloadsListExecuter::~CAsyncDownloadsListExecuter()
    {
LOG_ENTERFN("CAsyncDownloadsListExecuter::~CAsyncDownloadsListExecuter");
    Cancel();
    }

// ---------------------------------------------------------
// CAsyncDownloadsListExecuter::Start
// ---------------------------------------------------------
//
void CAsyncDownloadsListExecuter::Start()
    {
LOG_ENTERFN("CAsyncDownloadsListExecuter::Start");
    if ( !IsActive() )
        {
        BROWSER_LOG( ( _L( " Already active" ) ) );
        SetActive();
        TRequestStatus* s = &iStatus;
        User::RequestComplete( s, KErrNone );
        }
    }

// ---------------------------------------------------------
// CAsyncDownloadsListExecuter::DoCancel
// ---------------------------------------------------------
//
void CAsyncDownloadsListExecuter::DoCancel()
	{
LOG_ENTERFN("CAsyncDownloadsListExecuter::DoCancel");
	}

// ---------------------------------------------------------
// CAsyncDownloadsListExecuter::RunL
// ---------------------------------------------------------
//
void CAsyncDownloadsListExecuter::RunL()
	{
LOG_ENTERFN("CAsyncDownloadsListExecuter::RunL");
	// open the downloads list
    iApiProvider.BrCtlInterface().HandleCommandL( 
                                  (TInt)TBrCtlDefs::ECommandShowDownloads + 
                                  (TInt)TBrCtlDefs::ECommandIdBase );
    // The list of downloads is now closed.
	}

// ---------------------------------------------------------
// CAsyncDownloadsListExecuter::RunError
// ---------------------------------------------------------
//
#ifdef I__BROWSER_LOG_ENABLED
TInt CAsyncDownloadsListExecuter::RunError( TInt aError )
	{
LOG_ENTERFN("CAsyncDownloadsListExecuter::RunError");
	BROWSER_LOG( ( _L( " aError: %d" ), aError ) );
	return 0;
	}
#else
TInt CAsyncDownloadsListExecuter::RunError( TInt /*aError*/ )
	{
	return 0;
	}
#endif

// End of File
