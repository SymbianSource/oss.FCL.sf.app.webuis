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
*      Implementation of CLauncherClientService
*
*
*/


// INCLUDES
#include "LauncherClientService.h"
#include "LauncherReceiveHandler.h"
#include "logger.h"

// -----------------------------------------------------------------------------
// RLauncherClientService::ServiceUid()
// -----------------------------------------------------------------------------
//
TUid RLauncherClientService::ServiceUid() const
    {
    return TUid::Uid( 0x01010101 );
    }

// -----------------------------------------------------------------------------
// RLauncherClientService::InitializeL()
// -----------------------------------------------------------------------------
//
void RLauncherClientService::InitializeL
    ( TUid aServerAppUid, MLaunchedServerObserver& aServerObserver )
    {
    LOG_ENTERFN( "RLauncherClientService::InitializeL" );
    
    iActiveHandler = CLauncherReceiveHandler::NewL( &aServerObserver, *this );
    CActiveScheduler::Add( iActiveHandler );

    ConnectChainedAppL( aServerAppUid );
    iServerExitMonitor = CApaServerAppExitMonitor::NewL( *this, 
                                                         aServerObserver, 
                                                         CActive::EPriorityStandard );
    }

// -----------------------------------------------------------------------------
// RLauncherClientService::ListenL()
// -----------------------------------------------------------------------------
//
void RLauncherClientService::ListenL()
    {
    LOG_ENTERFN( "RLauncherClientService::ListenL" );
    
    if ( iActiveHandler )
        {
        __ASSERT_DEBUG(!iActiveHandler->IsActive(), User::Panic(_L("Already Listening..."),1));
        iActiveHandler->StartListen();
        }
    else
        {
        User::Leave( KErrNotFound );
        }
    }

// -----------------------------------------------------------------------------
// RLauncherClientService::StopListening()
// -----------------------------------------------------------------------------
//
void RLauncherClientService::StopListening()
    {
    LOG_ENTERFN( "RLauncherClientService::StopListening" );
    
    // tell receive handler to cancel receive and stop listening
    iActiveHandler->Cancel();    
    }

// -----------------------------------------------------------------------------
// RLauncherClientService::SendData()
// -----------------------------------------------------------------------------
//
TInt RLauncherClientService::SendSync(  TDesC8& aBufferToSend )
    {
    LOG_ENTERFN( "RLauncherClientService::SendSync" );
    return SendReceive( ELauncherSendSync, TIpcArgs( &aBufferToSend ) );
    }

// -----------------------------------------------------------------------------
// RLauncherClientService::SendAsync()
// -----------------------------------------------------------------------------
//
void RLauncherClientService::SendAsync( TDesC8& aBufferToSend, TDes8& aBufferSize )
    {
    LOG_ENTERFN( "RLauncherClientService::SendAsync" );
    // If no previous request was done, which has not finished,
    // then request is ok, send it...
    SendReceive( ELauncherSendASync, TIpcArgs( &aBufferToSend, &aBufferSize ), 
                          iActiveHandler->iStatus );
    }

// -----------------------------------------------------------------------------
// RLauncherClientService::SendAsync()
// -----------------------------------------------------------------------------
//
void RLauncherClientService::SendAsync()
    {
    LOG_ENTERFN( "RLauncherClientService::SendAsyncListen" );
    // If no previous request was done, which has not finished,
    // then request is ok, send it...
    SendReceive( ELauncherSendASyncListen, iActiveHandler->iStatus );
    }


// -----------------------------------------------------------------------------
// RLauncherClientService::SendAsync()
// -----------------------------------------------------------------------------
//
void RLauncherClientService::SendAsync(TDes8& aBufferSize )
    {
    LOG_ENTERFN( "RLauncherClientService::SendAsyncListen" );
    // If no previous request was done, which has not finished,
    // then request is ok, send it...
    
    
    TPckg<TInt> dummyParam(0);
    
    SendReceive( ELauncherSendASyncListen, TIpcArgs( &dummyParam, &aBufferSize ),iActiveHandler->iStatus );
    }

// -----------------------------------------------------------------------------
// RLauncherClientService::SendSyncBuffer()
// -----------------------------------------------------------------------------
//
TInt RLauncherClientService::SendSyncBuffer( TDes8& aReceiveBuffer )
    {
    LOG_ENTERFN( "RLauncherClientService::SendSyncBuffer" );
    // Else reset the state of the request, and send the buffer with
    // the size, set by the SendAsync() function 
    // So the server can write into it.
    return SendReceive( ELauncherSendSyncBuffer, TIpcArgs( &aReceiveBuffer ) );
    }

// -----------------------------------------------------------------------------
// RLauncherClientService::SendSyncBools()
// -----------------------------------------------------------------------------
//
TInt RLauncherClientService::SendSyncBools
    ( TBool aClientWantsToContinue, TBool aWasContentHandled )
    {
    LOG_ENTERFN( "RLauncherClientService::SendSyncBools" );
    // Else reset the state of the request, and send the buffer with
    // the size, set by the SendAsync() function 
    // So the server can write into it.
    return SendReceive( ELauncherSendSyncBools, 
                                     TIpcArgs( aClientWantsToContinue, aWasContentHandled ) );
    }
	
// -----------------------------------------------------------------------------
// RLauncherClientService::SendCommand()
// -----------------------------------------------------------------------------
//
TInt RLauncherClientService::SendCommand( TInt aCommand )
    {
    LOG_ENTERFN( "RLauncherClientService::SendCommand" );
    LOG_WRITE_FORMAT(" aCommand: %d", aCommand);
    return SendReceive( aCommand, TIpcArgs() );
    }

// -----------------------------------------------------------------------------
// RLauncherClientService::CancelReceive()
// -----------------------------------------------------------------------------
//
void RLauncherClientService::CancelReceive()
    {
    LOG_ENTERFN( "RLauncherClientService::CancelReceive" );
    SendReceive( ELauncherCancelReceive, TIpcArgs( 0 ) );
    }

// -----------------------------------------------------------------------------
// RLauncherClientService::Close()
// -----------------------------------------------------------------------------
//
void RLauncherClientService::Close()
    {
    LOG_ENTERFN( "RLauncherClientService::Close" );
    
    delete iActiveHandler;
    iActiveHandler = 0;
    LOG_WRITE(" iActiveHandler OK");

    delete iServerExitMonitor;
    iServerExitMonitor = 0;
    LOG_WRITE(" iServerExitMonitor OK");

    REikAppServiceBase::Close();
    LOG_WRITE(" ServiceBase::Close OK");
    }
    
// End of File
