/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Blocks dialog requests to the BrowserDialogsProvider if a
*               window is in the background. When the window comes to the 
*               foreground, it will be displayed.
*
*
*/

#include "BrowserDialogsProviderBlocker.h"
#include "BrowserWindowFocusNotifier.h"

// ----------------------------------------------------------------------------
//	static CBrowserDialogsProviderBlocker* 
//                                  CBrowserDialogsProviderBlocker::NewLC
// ----------------------------------------------------------------------------
//    
CBrowserDialogsProviderBlocker* CBrowserDialogsProviderBlocker::NewLC(
                            CBrowserWindowFocusNotifier& aWinFocusNotifier )
    {
    CBrowserDialogsProviderBlocker* self = new (ELeave) 
                CBrowserDialogsProviderBlocker( aWinFocusNotifier );
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// ----------------------------------------------------------------------------
//	CBrowserDialogsProviderBlocker::CBrowserDialogsProviderBlocker()
// ----------------------------------------------------------------------------
//    
CBrowserDialogsProviderBlocker::CBrowserDialogsProviderBlocker(
                            CBrowserWindowFocusNotifier& aWinFocusNotifier )
 
    : CActive( CActive::EPriorityStandard ),
    iWinFocusNotifier( aWinFocusNotifier )
    {
    CActiveScheduler::Add( this );
    }

// ---------------------------------------------------------------------------
//	CBrowserDialogsProviderBlocker::~CBrowserDialogsProviderBlocker()
//  Destroy the object and release all memory objects
// ---------------------------------------------------------------------------
//    
CBrowserDialogsProviderBlocker::~CBrowserDialogsProviderBlocker()
    {
    Cancel();
    }

// ---------------------------------------------------------------------------
//	void CBrowserDialogsProviderBlocker::ConstructL()
//  Two-pase constructor
// ---------------------------------------------------------------------------
//    
void CBrowserDialogsProviderBlocker::ConstructL()
    {
    }

// ---------------------------------------------------------------------------
//	void CBrowserDialogsProviderBlocker::Start()
//  Complete an asynchronous request.
// ---------------------------------------------------------------------------
//    
void CBrowserDialogsProviderBlocker::Start()
    {
    // Register the AO status
    iStatus = KRequestPending;
    iWinFocusNotifier.Add( &iStatus );
    SetActive();
    
    // Asynch wait start
    iWait.Start();
    }

// ---------------------------------------------------------------------------
//	void CBrowserDialogsProviderBlocker::DoCancel()
//  Cancel any outstanding requests.
// ---------------------------------------------------------------------------
//    
void CBrowserDialogsProviderBlocker::DoCancel()
    {
    // The AO can continue
    iWait.AsyncStop();
    }

// ---------------------------------------------------------------------------
//	void CBrowserDialogsProviderBlocker::RunL()
//  Handles object`s request completion event.
// ---------------------------------------------------------------------------
//    
void CBrowserDialogsProviderBlocker::RunL()
    {
    // The AO can continue
    iWait.AsyncStop();  
    }

// End of file
