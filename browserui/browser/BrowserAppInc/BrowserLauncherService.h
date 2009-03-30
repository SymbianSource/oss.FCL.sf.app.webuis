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
*      Implementation of BrowserLauncherService
*
*
*/


#ifndef BROWSERLAUNCHERSERVICE_H
#define BROWSERLAUNCHERSERVICE_H

// INCLUDE FILES 
#include "LauncherServerBase.h"

// FORWARD DECLARATIONS
class TBrowserOverriddenSettings;
class CAiwGenericParamList;

// CLASS DECLARATION
class CBrowserLauncherService: public CLauncherServerBase
{
public:

    /**
    * Destructor.
    */
    virtual ~CBrowserLauncherService();   

private: 
	/**
	* Process incoming messages syncronously.
    * @param aEmbeddingApplicationUid Embedding application`s UID	
    * @param aSettings Overridden user preferences.
    * @param aFolderUid Uid of the folder that the Browser should open.
    * @param aSeamlessParam 
    * @param aIsContentHandlerRegistered True if a content handler is registered
	* @return The result of processing. ( same as SendRecieve() )
	*/ 
	// from CLauncherServerBase
	TInt ProcessMessageSyncL( TUid aEmbeddingApplicationUid, 
							  TBrowserOverriddenSettings aSettings, 
							  TInt aFolderUid, 
							  TPtrC aSeamlessParam, 
							  TBool aIsContentHandlerRegistered );
	
	/**
	* Process incoming messages asyncronously.
    * @param aEmbeddingApplicationUid Embedding application`s UID
    * @param aSettings Overridden user preferences.
	* @param aFolderUid Uid of the folder that the Browser should open.
    * @param aSeamlessParam
    * @param aIsContentHandlerRegistered True if a content handler is registered    
    * @param isOverridenSettings True if the client sended an overidden setting
	*/
	// from CLauncherServerBase
    void ProcessMessageASyncL( TUid aEmbeddingApplicationUid, 
    						   TBrowserOverriddenSettings aSettings, 
    						   TInt aFolderUid, 
    						   TPtrC aSeamlessParam, 
    						   TBool aIsContentHandlerRegistered, 
    						   TBool isOverridenSettings );

	/**
	* Write iClientBuffer member to  client`s address space.
	* @return The error code of the process.
	*/    
    TInt ProcessMessageSyncBufferL( );
    
        /**
        * Process TBool parameters.
        */
        void ProcessBoolsL( TBool aClientWantsToContinue, TBool aWasContentHandled );

    /**
    * Closes the Browser.
    */
    void DoBrowserExit( );
    
public:
    
	/**
	* Notifies client about finished download.
	* @param aReason Result of the downloading
	*/    
    void DownloadFinishedL( TInt aReason, const CAiwGenericParamList& aParamList );
    
public:	
	/**
	* Second phase constructor.
	*/
    void ConstructL();   
};

#endif	// CBrowserAppServer_H