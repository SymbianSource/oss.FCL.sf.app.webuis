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
*      Implementation of CLauncherReceiveHandler
*
*
*/


// INCLUDES
#include "LauncherReceiveHandler.h"
#include "logger.h"

// -----------------------------------------------------------------------------
// CLauncherReceiveHandler::CLauncherReceiveHandler()
// -----------------------------------------------------------------------------
//
CLauncherReceiveHandler::CLauncherReceiveHandler( MLaunchedServerObserver* aServerObserver, RLauncherClientService& aSession )
: CActive(CActive::EPriorityStandard), iSession ( aSession )
    {
    iServerObserver = aServerObserver;        
    }

// -----------------------------------------------------------------------------
// CLauncherReceiveHandler::~CLauncherReceiveHandler()
// -----------------------------------------------------------------------------
//
CLauncherReceiveHandler::~CLauncherReceiveHandler()
    {
    LOG_ENTERFN( "CLauncherReceiveHandler::~CLauncherReceiveHandler" );
    Cancel();
    }

// -----------------------------------------------------------------------------
// CLauncherReceiveHandler::NewL()
// -----------------------------------------------------------------------------
//
CLauncherReceiveHandler* CLauncherReceiveHandler::NewL( MLaunchedServerObserver* aServerObserver, RLauncherClientService& aSession )
    {
    LOG_ENTERFN( "CLauncherReceiveHandler::NewL" );
    CLauncherReceiveHandler* self = NewLC( aServerObserver, aSession );
    CleanupStack::Pop(self);
    return self;
    }

// -----------------------------------------------------------------------------
// CLauncherReceiveHandler::NewLC()
// -----------------------------------------------------------------------------
//
CLauncherReceiveHandler* CLauncherReceiveHandler::NewLC( MLaunchedServerObserver* aServerObserver, RLauncherClientService& aSession )
    {
    LOG_ENTERFN( "CLauncherReceiveHandler::NewLC" );
    CLauncherReceiveHandler* self = new (ELeave) CLauncherReceiveHandler( aServerObserver, aSession );
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CLauncherReceiveHandler::ConstructL()
// -----------------------------------------------------------------------------
//
void CLauncherReceiveHandler::ConstructL()
    {
    }

// -----------------------------------------------------------------------------
// CLauncherReceiveHandler::StartListen()
// -----------------------------------------------------------------------------
//
void CLauncherReceiveHandler::StartListen()
    {
    LOG_ENTERFN( "CLauncherReceiveHandler::StartListen" );
    iStatus = KRequestPending;
    SetActive();
    }
    
// -----------------------------------------------------------------------------
// CLauncherReceiveHandler::DoCancel()
// -----------------------------------------------------------------------------
//
void CLauncherReceiveHandler::DoCancel()
    {
    LOG_ENTERFN( "CLauncherReceiveHandler::DoCancel" );
    iSession.CancelReceive();
    }

// -----------------------------------------------------------------------------
// CLauncherReceiveHandler::RunL()
// -----------------------------------------------------------------------------
//
void CLauncherReceiveHandler::RunL()
    {
    LOG_ENTERFN( "CLauncherReceiveHandler::RunL" );
    // Handle and re-initialize only if no error occured.
    if ( iStatus.Int() == KErrNone )
        {
        iServerObserver->HandleReceivedDataL( iStatus );
        iSession.ListenL();
        }
    }
    
// End of File
