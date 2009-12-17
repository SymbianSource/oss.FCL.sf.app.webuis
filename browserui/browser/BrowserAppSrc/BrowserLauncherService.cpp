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


// INCLUDES
#include "BrowserLauncherService.h"
#include "browseroverriddensettings.h"
#include "LauncherServerCommands.hrh"
#include "BrowserAppDocument.h"
#include "BrowserAppUi.h"
#include "BrowserAsyncExit.h"
#include "logger.h"
#include <AiwGenericParam.h>
#include <s32mem.h>
#include "BrowserSpecialLoadObserver.h"
#include "mconnection.h"

// -----------------------------------------------------------------------------
// CBrowserLauncherService::~CBrowserLauncherService()
// -----------------------------------------------------------------------------
//
CBrowserLauncherService::~CBrowserLauncherService()
    {
    LOG_ENTERFN( "CBrowserLauncherService::~CBrowserLauncherService" );
    // The base class has no destructor, thus resources must be freed up here!
    delete iClientBuffer; iClientBuffer = 0;
    // Cancel outstanding asynchronous request
    if ( iMyAsyncMessage.Handle() !=0 )
        {
        iMyAsyncMessage.Complete( KErrCancel );
        }
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherService::ProcessMessageSyncL()
// -----------------------------------------------------------------------------
//
TInt CBrowserLauncherService::ProcessMessageSyncL( TUid /*aEmbeddingApplicationUid*/, 
												   TBrowserOverriddenSettings /*aSettings*/, 
												   TInt /*aFolderUid*/, 
												   TPtrC /*aSeamlessParam*/, 
												   TBool /*aIsContentHandlerRegistered*/ )
    {
    LOG_ENTERFN( "CBrowserLauncherService::ProcessMessageSyncL" );
    
    // Do stuff here...
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherService::ProcessMessageASyncL()
// -----------------------------------------------------------------------------
//
void CBrowserLauncherService::ProcessMessageASyncL( TUid aEmbeddingApplicationUid, 
													TBrowserOverriddenSettings aSettings, 
													TInt aFolderUid, 
													TPtrC aSeamlessParam, 
													TBool aIsContentHandlerRegistered, 
													TBool aIsOverridenSettings )
    {
    LOG_ENTERFN( "CBrowserLauncherService::ProcessMessageASyncL" );
    
    TBrowserOverriddenSettings* paSettings;
    TInt seamlessParamLength = aSeamlessParam.Length();
    HBufC8* param8 = HBufC8::NewLC( seamlessParamLength );
        
    // Set Browser`s document settings
    if ( aIsOverridenSettings )
    	{
    	paSettings = &aSettings;
    	iBrowserDocument->SetOverriddenSettings( paSettings );
    	}
    else
        {
        iBrowserDocument->SetOverriddenSettings( NULL );
        }
		
    // Set browser document		
    iBrowserDocument->SetFolderToOpen( aFolderUid );
    iBrowserDocument->SetIsContentHandlerRegistered( aIsContentHandlerRegistered );

    // Initialize browser
    iBrowserAppUi->SetEmbeddingAppliacationUid( aEmbeddingApplicationUid );
    iBrowserAppUi->InitBrowserL( );
    				
    // Load the specified URL
    param8->Des().Copy( aSeamlessParam );

    // If no command or URL was given open the browser with the specified bookmark
    if ( seamlessParamLength )
    	{
    	iBrowserAppUi->ParseAndProcessParametersL( *param8 );			
        iBrowserAppUi->SetViewToBeActivatedIfNeededL( iBrowserAppUi->LastActiveViewId() );
    	}
    else
    	{
    	iBrowserAppUi->ParseAndProcessParametersL( *param8, EFalse );
    	
        // startup content view to initialize fep/vkb
    	iBrowserAppUi->SetLastActiveViewId( KUidBrowserContentViewId );
    	iBrowserAppUi->SetViewToBeActivatedIfNeededL( iBrowserAppUi->LastActiveViewId() );
       	iBrowserAppUi->ActivateLocalViewL( iBrowserAppUi->LastActiveViewId() );			

    	// Now activate bookmarks view
    	iBrowserAppUi->SetLastActiveViewId( KUidBrowserBookmarksViewId );
       	iBrowserAppUi->SetViewToBeActivatedIfNeededL( iBrowserAppUi->LastActiveViewId() );
    	}
    iBrowserAppUi->ActivateLocalViewL( iBrowserAppUi->LastActiveViewId() );
    iBrowserAppUi->HandleForegroundEventL( ETrue );

	// perform special action for overridden context
	// could later use as flag for LSK,RSK.
	switch(aSettings.GetBrowserSetting(EBrowserOverSettingsContextId))
		{
			case EBrowserContextIdFeeds:
				iBrowserAppUi->SetOverriddenLaunchContextId(EBrowserContextIdFeeds);
				iBrowserAppUi->LaunchIntoFeedsL();	
				break;
			default:
				// do nothing
				break;
		
		}

    CleanupStack::PopAndDestroy(); // param8
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherService::ProcessMessageSyncBufferL()
// -----------------------------------------------------------------------------
//    
TInt CBrowserLauncherService::ProcessMessageSyncBufferL( )
    {
    LOG_ENTERFN( "CBrowserLauncherService::ProcessMessageSyncBufferL" );
    
    // Write iClientBuffer to  client`s address space.
    iMySyncMessage.WriteL( 0,iClientBuffer->Des() );    
    return KErrNone;
    }
	
// -----------------------------------------------------------------------------
// CBrowserLauncherService::ProcessBoolsL()
// -----------------------------------------------------------------------------
//    
void CBrowserLauncherService::ProcessBoolsL
    ( TBool aClientWantsToContinue, TBool aWasContentHandled )
    {
    LOG_ENTERFN( "CBrowserLauncherService::ProcessBoolsL" );
    
    iBrowserAppUi->SetContentHandlingResult( aClientWantsToContinue, aWasContentHandled );
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherService::DoBrowserExit()
// -----------------------------------------------------------------------------
//    
void CBrowserLauncherService::DoBrowserExit( )
    {
    LOG_ENTERFN( "CBrowserLauncherService::DoBrowserExitL" );
    
    if ( iBrowserAppUi )
        {
        if ( !iBrowserAppUi->ExitInProgress() )
            {
            if (iBrowserAppUi->SpecialLoadObserver().IsConnectionStarted()) // If Connection request is in processing
			    {  
			    iBrowserAppUi->Connection().Disconnect();                                             
			    }
	        else
			    {
			    iBrowserAppUi->iBrowserAsyncExit->Start();
			    }   
            }        
        iBrowserAppUi->SetBrowserLauncherService( NULL );
        }
    }
        
// -----------------------------------------------------------------------------
// CBrowserLauncherService::DownloadFinished()
// -----------------------------------------------------------------------------
//
void CBrowserLauncherService::DownloadFinishedL( TInt aReason, const CAiwGenericParamList& aParamList )
    {
    LOG_ENTERFN( "CBrowserLauncherService::DownloadFinishedL" );
    
    // This function is called by the Browser. If download is finished 
    // write the result to the clients buffer.

    // Get the size of the parameter list`s externalized size.
    TInt parameterListExternalizedSize = aParamList.Size();

    // Create the buffer to hold the externalized data.
    HBufC8* newClientBuffer = HBufC8::NewL( parameterListExternalizedSize );
    delete iClientBuffer; // delete the old buffer
    iClientBuffer = newClientBuffer;

    // Create Stream object, and write stream to descriptor iClientBuffer.
    TPtr8 piClientBuffer = iClientBuffer->Des();
    RDesWriteStream writeStream( piClientBuffer );

    // Externalize parameter list into descriptor iClientBuffer.	
    aParamList.ExternalizeL( writeStream );
    BROWSER_LOG( ( _L( " ExternalizeL OK" ) ) );

    // Close stream object.
    writeStream.CommitL();
    BROWSER_LOG( ( _L( " CommitL OK" ) ) );
    writeStream.Close();
    BROWSER_LOG( ( _L( " Close OK" ) ) );

    // Complete the client with the created buffer`s size.
    if ( aReason == KErrNone )
        {
        TPckg<TInt> bufferSize( iClientBuffer->Length() );
	    TRAPD(err, iMyAsyncMessage.WriteL( 1, bufferSize ));	
	    LOG_WRITE_FORMAT(" WriteL erro code : %d", err );
	    User::LeaveIfError(err);
        iMyAsyncMessage.Complete( KErrNone );
        }
    else
        {
        iMyAsyncMessage.Complete( aReason );
        }
    }

// -----------------------------------------------------------------------------
// CBrowserLauncherService::ConstructL()
// -----------------------------------------------------------------------------
//
void CBrowserLauncherService::ConstructL()
    {
    LOG_ENTERFN( "CBrowserLauncherService::ConstructL" );
    iBrowserAppUi->SetBrowserLauncherService( this );
    }

// End of File
