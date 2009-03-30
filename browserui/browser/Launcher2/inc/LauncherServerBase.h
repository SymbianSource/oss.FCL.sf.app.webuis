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
* Description:  Header file for CLauncherServerBase
*
*
*/


#ifndef __LAUNCHERSERVERBASE_H__
#define __LAUNCHERSERVERBASE_H__

// INCLUDE FILES
#include <e32base.h>
#include <AknServerApp.h>

// FORWARD DECLARATION
class TBrowserOverriddenSettings;
class CBrowserAppDocument;
class CBrowserAppUi;

// CLASS DECLARATION
class CLauncherServerBase : public CAknAppServiceBase
{
public:

    /**
    * Default constructor
    */
    IMPORT_C CLauncherServerBase();

    /**
    * Handle incoming IPC messages.
    * @param aMessage Object which encapsulates a client request.
    */
    IMPORT_C void ServiceL( const RMessage2& aMessage );
    
    /**
    * Pointer to the Browser`s Ui.
    */    
    CBrowserAppUi* iBrowserAppUi; // not owned     

private:

    /**
    * Handle syncronous messages. Parse the recieved message, 
    * and call ProcessMessageSyncL
    * @param aMessage Object which encapsulates a client request.
    */
    void HandleMessageSyncL( /*const RMessage2& aMessage*/ );

    /**
    * Handle asyncronous messages. Parse the recieved message, 
    * and call ProcessMessageASyncL.
    * @param aMessage Object which encapsulates a client request.
    */    
    void HandleMessageASyncL( /*const RMessage2& aMessage*/ );

    /**
    *
    * @param aMessage Object which encapsulates a client request.
    */    
    void HandleMessageSyncBufferL( /*const RMessage2& aMessage*/ );

    /**
    *
    * @param aMessage Object which encapsulates a client request.
    */    	
    void HandleMessageSyncBoolsL( /*const RMessage2& aMessage*/ );

    /**
    *
    * @param aMessage Object which encapsulates a client request.
    */    	
    void HandleBrowserExit( /*const RMessage2& aMessage*/ );
    

    /**
    * Process incoming messages syncronously.
    * @param aEmbeddingApplicationUid Embedding application`s UID	
    * @param aSettings Overridden user preferences.
    * @param aFolderUid Uid of the folder that the Browser should open.
    * @param aSeamlessParam 
    * @param aIsContentHandlerRegistered True if a content handler is registered
    * @return The result of processing. ( same as SendRecieve() )
    */ 
    virtual TInt ProcessMessageSyncL( TUid aEmbeddingApplicationUid, 
    								  TBrowserOverriddenSettings aSettings, 
    								  TInt aFolderUid, TPtrC aSeamlessParam, 
    								  TBool aIsContentHandlerRegistered ) = 0;

    /**
    * Process incoming messages asyncronously.
    * @param aEmbeddingApplicationUid Embedding application`s UID
    * @param aSettings Overridden user preferences.
    * @param aFolderUid Uid of the folder that the Browser should open.
    * @param aSeamlessParam
    * @param aIsContentHandlerRegistered True if a content handler is registered    
    * @param isOverridenSettings True if the client sended an overidden setting
    */
    virtual void ProcessMessageASyncL( TUid aEmbeddingApplicationUid, 
    								   TBrowserOverriddenSettings aSettings, 
    								   TInt aFolderUid, 
    								   TPtrC aSeamlessParam, 
    								   TBool aIsContentHandlerRegistered, 
    								   TBool aIsOverridenSettings ) = 0;

    /**
    * Process incoming messages asyncronously.
    * @param aMessage Object which encapsulates a client request.
    * @param aSettings Overridden user preferences.
    * @param aFolderUid Uid of the folder that the Browser should open.
    * @param aSeamlessParam  
    * @param aParam The index of the parameter, in SendRecieve() function`s
    * 		 parameters, which is the buffer on client side.
    */
    virtual TInt ProcessMessageSyncBufferL( ) = 0;

    /**
    * Process TBool parameters.
    */
    virtual void ProcessBoolsL( TBool aClientWantsToContinue, TBool aWasContentHandled ) = 0;

    /**
    * Process BrowserExit.
    */
    virtual void DoBrowserExit( ) = 0;       

protected:

    /**
    * Buffer which will be copied to the client`s address
    * space on completing asyncronous requests.
    */
    HBufC8* iClientBuffer;

    /**
    * Pointer to the Browser`s document.
    */    
    CBrowserAppDocument* iBrowserDocument; // not owned

    /**
    * Object which encapsulates a copy of a client request.
    */	
    RMessage2 iMySyncMessage; // One slot for synchronous requests.
    RMessage2 iMyAsyncMessage; // One slot for asynchronous requests.

/* Capability check will come later...
public:
	CLauncherServerBase( TCapability aClientReqs );

private:
	CPolicyServer::TCustomResult SecurityCheckL ( const RMessage2& aMsg, 
												  TInt& aAction, 
												  TSecurityInfo& aMissing );
	TCapability iClientReqs;
*/
};

#endif // __LAUNCHERSERVERBASE_H__

// End of File