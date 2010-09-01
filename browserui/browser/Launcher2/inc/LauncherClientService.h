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
* Description:  Header file for RLauncherClientService
*
*
*/


#ifndef __LAUNCHERCLIENTSERVICE_H
#define __LAUNCHERCLIENTSERVICE_H

// INCLUDE FILES 
#include <e32std.h>
#include <e32base.h>
#include <AknServerApp.h>
#include "LauncherServerCommands.hrh"

// FORWARD DECLARATIONS
class CLauncherReceiveHandler;

// CLASS DECLARATION
// Observer class, observes server application events.
class MLaunchedServerObserver: public MAknServerAppExitObserver
    {
public:

    /**
    * Notifies the client, that implements this class, about 
    * the server sent data.
    * @param aStatus The status of the result of the completion.
    */
    virtual void HandleReceivedDataL( TRequestStatus aStatus ) = 0;

    };

// CLASS DECLARATION
class RLauncherClientService : public RAknAppServiceBase
    {
public:	
    /**
    * Returns the ID of this service.
    * @return The UID of the service.
    */	
    TUid ServiceUid() const;

    /**
    * Initializes this object. Call this before using this object.
    * @param aServerAppUid The UID of the server application.
    * @param aServerObserver Observes the server application.
    */
    void InitializeL( TUid aServerAppUid, MLaunchedServerObserver& aServerObserver );

    /**
    * Starts listening to the server application, so the observer
    * will be notified about server application events.
    */    
    void ListenL();

    /**
    * Stops listening to the server application, so the observer
    * will not be notified about server application events.
    */    
    void StopListening();
    
    /**
    * Destroy the object and release all memory objects
    */    
    void Close();

    /**
    * Send data to process by the server application syncronously.
    * @param aBufferToSend The data to send.
    * @return The completion code of the processed data.
    */
    TInt SendSync( TDesC8& aBufferToSend );

    /**
    * Send data to process by the server application asyncronously.
    * @param aBufferToSend Data to send.
    * @param aBufferSize The size of the buffer which the client should
    *                                 allocate, to receive the data from server,
    *                                 before calling SendSyncBuffer() with the
    *                                 allocated descriptor. aBufferSize is defined
    *                                 by the server.
    */
    void SendAsync( TDesC8& aBufferToSend,TDes8& aBufferSize );

    /**
    * Send data to process by the server application asyncronously.
    * @param aBufferToSend Data to send.
    * @param aBufferSize The size of the buffer which the client should
    * 					 allocate, to receive the data from server,
    *                    before calling SendSyncBuffer() with the
    *                    allocated descriptor. aBufferSize is defined
    *                    by the server.
    */
    void SendAsync();

	void SendAsync(TDes8& aBufferSize );
    /**
    * Send data to process by the server application syncronously.
    * @param aReceiveBuffer Buffer, which will be filled by the 
    *                       server application.
    */
    TInt SendSyncBuffer( TDes8 &aReceiveBuffer );

    TInt SendSyncBools( TBool aClientWantsToContinue, TBool aWasContentHandled );

    /**
    * Send a command to execute by the server application. Server 
    * commands can be found in ServerCommands.hrh.
    * Possibile commands: EOpenDownloadedContent, EServerExit.
    * @param aCommand Command to send.
    * @return The result of the processed command data.
    */	
    TInt SendCommand( TInt aCommand );
	
private:

    /**
    * Cancel receiving data.
    */
    void CancelReceive();

public:

    /*
    * Observes server application exit. Owned!
    */
    CApaServerAppExitMonitor* iServerExitMonitor;

    friend class CLauncherReceiveHandler;    

private:

    /*
    * Active object that is notified when receiving data ayncronously from
    * server application. Internal use only. Owned!
    */
    CLauncherReceiveHandler* iActiveHandler;
};
	
#endif // __LAUNCHER_H

// End of File