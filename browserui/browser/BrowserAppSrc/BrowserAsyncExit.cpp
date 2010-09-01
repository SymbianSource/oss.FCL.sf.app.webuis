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
*  CBrowserAsyncExit class can exit the Browser in async mode. It calls the CBrowserAppUi`s
*  ExitBrowser() method, when the object completes the request.
*  The class helps avoiding to send  event.iCode  EKeyApplication1; event
*  to the embedded browser to exit.
*
*
*/


#include "BrowserAsyncExit.h"
#include "ApiProvider.h"
#include "Logger.h"


// ---------------------------------------------------------------------------
// staticCBrowserAsyncExit* CBrowserAsyncExit::NewL( )
// Create a CBrowserAsyncExit object. Leaves on failure.
// ---------------------------------------------------------------------------
//
CBrowserAsyncExit* CBrowserAsyncExit::NewL( MApiProvider* aApiProvider )
    {
    LOG_ENTERFN("CBrowserAsyncExit::NewL");    
    CBrowserAsyncExit* self = NewLC( aApiProvider );
    CleanupStack::Pop(self);
    return self;
    }

// ---------------------------------------------------------------------------
//	static CBrowserAsyncExit* CBrowserAsyncExit::NewLC( )
//  Create a CBrowserAsyncExit object. Leaves on failure.
// ---------------------------------------------------------------------------
//    
CBrowserAsyncExit* CBrowserAsyncExit::NewLC( MApiProvider* aApiProvider )
    {
    LOG_ENTERFN("CBrowserAsyncExit::NewLC");        
    CBrowserAsyncExit* self = new (ELeave) CBrowserAsyncExit( aApiProvider );
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// ---------------------------------------------------------------------------
//	CBrowserAsyncExit::CBrowserAsyncExit( ):CActive(CActive::EPriorityStandard)
//  Constructs this object
// ---------------------------------------------------------------------------
//    
CBrowserAsyncExit::CBrowserAsyncExit( MApiProvider* aApiProvider ):CActive(CActive::EPriorityStandard)
    {
    LOG_ENTERFN("CBrowserAsyncExit::CBrowserAsyncExit");            
    CActiveScheduler::Add( this );
    iApiProvider = aApiProvider;
    }

// ---------------------------------------------------------------------------
//	CBrowserAsyncExit::~CBrowserAsyncExit()
//  Destroy the object and release all memory objects
// ---------------------------------------------------------------------------
//    
CBrowserAsyncExit::~CBrowserAsyncExit()
    {
    LOG_ENTERFN("CBrowserAsyncExit::~CBrowserAsyncExit");                
    Cancel();
    }

// ---------------------------------------------------------------------------
//	void CBrowserAsyncExit::ConstructL()
//  Two-pase constructor
// ---------------------------------------------------------------------------
//    
void CBrowserAsyncExit::ConstructL()
    {  
    }

// ---------------------------------------------------------------------------
//	void CBrowserAsyncExit::Start()
//  Complete an asynchronous request.
// ---------------------------------------------------------------------------
//    
void CBrowserAsyncExit::Start()
    {
	if ( !IsActive() )
		{
		LOG_ENTERFN("CBrowserAsyncExit::Start");  
		TRequestStatus* status = &iStatus;
		SetActive();    
		User::RequestComplete( status, KErrNone );
		}
    }

// ---------------------------------------------------------------------------
//	void CBrowserAsyncExit::DoCancel()
//  Cancel any outstanding requests.
// ---------------------------------------------------------------------------
//    
void CBrowserAsyncExit::DoCancel()
    {
    LOG_ENTERFN("CBrowserAsyncExit::DoCancel");         
    }

// ---------------------------------------------------------------------------
//	void CBrowserAsyncExit::RunL()
//  Handles object`s request completion event.
// ---------------------------------------------------------------------------
//    
void CBrowserAsyncExit::RunL()
    {
    LOG_ENTERFN("CBrowserAsyncExit::RunL");     
    iApiProvider->ExitBrowser( EFalse ); // Exit the browser.
    }

// End of file
