/*
* Copyright (c) 2002-2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  BrowserLauncher
*
*/



// INCLUDE FILES
#include "BrowserLauncher.h"
#include "BrowserLauncherExtension.h"
#include "logger.h"
#include "BrowserUiPrivateCRKeys.h"
#include "BrowserOverriddenSettings.h"
#include "Browser.hrh"

#include <FavouritesLimits.h> // Context Id:s
#include <eikappui.h>
#include <eikapp.h>
#include <eikenv.h>
#include <centralrepository.h>
#include <FavouritesDb.h>

// CONSTANTS
const TInt KBrowserAppUid = 0x10008D39;
const TInt KBrowserSeamlessParamLength = 20;
_LIT( KBookmarkStarterString, "1 " );

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBrowserLauncher::CBrowserLauncher
// -----------------------------------------------------------------------------
//
CBrowserLauncher::CBrowserLauncher()
    {
    iBrowserUid = TUid::Uid( KBrowserAppUid );
    }
    
// -----------------------------------------------------------------------------
// CBrowserLauncher::ConstructL
// -----------------------------------------------------------------------------
//
void CBrowserLauncher::ConstructL()
    {
    LOG_CREATE;
    LOG_ENTERFN( "CBrowserLauncher::ConstructL" );
    }

// -----------------------------------------------------------------------------
// CBrowserLauncher::NewLC
// -----------------------------------------------------------------------------
//
EXPORT_C 
CBrowserLauncher* CBrowserLauncher::NewLC()
    {
    LOG_ENTERFN( "CBrowserLauncher::NewLC" );
    CBrowserLauncher* self = new(ELeave) CBrowserLauncher();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CBrowserLauncher::NewL
// -----------------------------------------------------------------------------
//
EXPORT_C 
CBrowserLauncher* CBrowserLauncher::NewL()
    {
    LOG_ENTERFN( "CBrowserLauncher::NewL" );
    CBrowserLauncher* self = NewLC();
    CleanupStack::Pop();
    return self;
    }

// -----------------------------------------------------------------------------
// CBrowserLauncher::NewLC
// -----------------------------------------------------------------------------
//
EXPORT_C 
CBrowserLauncher* CBrowserLauncher::NewLC( TInt /*aOverrideBrowserUid*/ )
    {
    LOG_ENTERFN( "CBrowserLauncher::NewLC(TInt aOverrideBrowserUid) IS DEPRECATED" );
    CBrowserLauncher* self = new(ELeave) CBrowserLauncher();
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;	
	}
	
// -----------------------------------------------------------------------------
// CBrowserLauncher::NewL
// -----------------------------------------------------------------------------
//
EXPORT_C
CBrowserLauncher* CBrowserLauncher::NewL( TInt /*aOverrideBrowserUid*/ )
    {
    LOG_ENTERFN( "CBrowserLauncher::NewL(TInt aOverrideBrowserUid) IS DEPRECATED" );
    CBrowserLauncher* self = NewLC();
    CleanupStack::Pop();
    return self;	
    }
	
// Destructor
EXPORT_C 
CBrowserLauncher::~CBrowserLauncher()
    {
    LOG_ENTERFN( "CBrowserLauncher::~CBrowserLauncher" );
    Cancel();
    }

// ----------------------------------------------------------------------------
// CBrowserLauncher::LaunchBrowserEmbeddedL
// ----------------------------------------------------------------------------
//
EXPORT_C 
void CBrowserLauncher::LaunchBrowserEmbeddedL
                      ( const TDesC& aParams, 
                        MDownloadedContentHandler* aContentHandler, 
                        MAknServerAppExitObserver* aExitObserver, 
                        TBrowserOverriddenSettings* aSettings )
    {
    LaunchBrowserEmbeddedL
                      ( aParams, 
                        /*aContextId*/KErrNotFound, 
                        aContentHandler, 
                        aExitObserver, 
                        aSettings );
    }

// ----------------------------------------------------------------------------
// CBrowserLauncher::LaunchBrowserEmbeddedL
// ----------------------------------------------------------------------------
//
EXPORT_C 
void CBrowserLauncher::LaunchBrowserEmbeddedL
                      ( TInt aContextId, 
                        MDownloadedContentHandler* aContentHandler, 
                        MAknServerAppExitObserver* aExitObserver, 
                        TBrowserOverriddenSettings* aSettings )
    {
    LaunchBrowserEmbeddedL
                      ( /*aParams*/KNullDesC, 
                        aContextId, 
                        aContentHandler, 
                        aExitObserver, 
                        aSettings );
    }

// ----------------------------------------------------------------------------
// CBrowserLauncher::LaunchBrowserSyncEmbeddedL
// ----------------------------------------------------------------------------
//
EXPORT_C 
TInt CBrowserLauncher::LaunchBrowserSyncEmbeddedL
                      ( const TDesC& aParams, 
                        MDownloadedContentHandler* aContentHandler, 
                        TBrowserOverriddenSettings* aSettings )
    {
    LaunchBrowserEmbeddedL
                      ( aParams, 
                        /*aContextId*/KErrNotFound, 
                        aContentHandler, 
                        /*aExitObserver*/0, 
                        aSettings );

    return (iExtension->WaitBrowserToFinish());
    }

// ----------------------------------------------------------------------------
// CBrowserLauncher::LaunchBrowserSyncEmbeddedL
// ----------------------------------------------------------------------------
//
EXPORT_C 
TInt CBrowserLauncher::LaunchBrowserSyncEmbeddedL
                      ( TInt aContextId, 
                        MDownloadedContentHandler* aContentHandler, 
                        TBrowserOverriddenSettings* aSettings )
    {
    LaunchBrowserEmbeddedL
                      ( /*aParams*/KNullDesC, 
                        aContextId, 
                        aContentHandler, 
                        /*aExitObserver*/0, 
                        aSettings );

    return (iExtension->WaitBrowserToFinish());
    }

// ----------------------------------------------------------------------------
// CBrowserLauncher::Cancel
// ----------------------------------------------------------------------------
//
EXPORT_C 
void CBrowserLauncher::Cancel()
    {
    // Terminate the Browser
    delete iExtension;
    iExtension = 0;
    iFavouritesSess.Close();    
    }

// -----------------------------------------------------------------------------
// CBrowserLauncher::LaunchBrowserEmbeddedL
// If aParams is not empty, then it is used, otherwise aContextId is used.
// -----------------------------------------------------------------------------
//
void CBrowserLauncher::LaunchBrowserEmbeddedL
                      ( const TDesC& aParams, 
                        TInt aContextId, 
                        MDownloadedContentHandler* aContentHandler, 
                        MAknServerAppExitObserver* aExitObserver, 
                        TBrowserOverriddenSettings* aSettings )
    {
    LOG_ENTERFN( "CBrowserLauncher::LaunchBrowserEmbeddedL" );
    
    LOG_WRITE_FORMAT(" aParams: [%S]", &aParams);
    LOG_WRITE_FORMAT(" aContextId: %d", aContextId);
    LOG_WRITE_FORMAT(" aContentHandler: 0x%x", aContentHandler);
    LOG_WRITE_FORMAT(" aExitObserver: 0x%x", aExitObserver);
    LOG_WRITE_FORMAT(" aSettings: 0x%x", aSettings);

    TInt folderUid = 0;
    TInt folderPreferredUid = 0;
    
    // keep a local ptr to aSettings
    TBrowserOverriddenSettings* aOverrideSettings = aSettings;
    
    TUid embeddingAppUid = CEikonEnv::Static()->EikAppUi()->Application()->AppDllUid();
    LOG_WRITE_FORMAT(" embeddingAppUid: 0x%x", embeddingAppUid);
    
    iExtension = CBrowserLauncherExtension::NewL( *this );
    User::LeaveIfError( iFavouritesSess.Connect() );    
	
    if ( 0 == aParams.Length() )
        {       
        // SPECIAL folder UID handling
        folderUid = KFavouritesNullUid;
        TUint specialContextId = EBrowserContextIdNormal;
        
        switch(aContextId)
        	{
        		case KFavouritesFeedsContextId:
  					// For feeds, leave uid to Root as it is here.
  					// Upon launch, browser will then switch into the Feeds view
        			// upon opening.
        			specialContextId = EBrowserContextIdFeeds;
        			folderUid = KFavouritesRootUid;
        			folderPreferredUid = KFavouritesNullUid;
        			break;
        		default:
        			break;
        	}
        	
        // If we have now just set the folderUid, then
        // we need to tell browser of the context id if it's not done already
        if(folderUid != KFavouritesNullUid)
        	{
       		// if we're already given override settings and the specialContextId
       		// is not already given, set it now
       		if(aOverrideSettings)
       			{
       			if(aOverrideSettings->GetBrowserSetting(EBrowserOverSettingsContextId) != EBrowserContextIdUndefined)
       				{
       				aOverrideSettings->SetBrowserSetting(EBrowserOverSettingsContextId, specialContextId);
   					}
       			}
   			else
   				{
   					aOverrideSettings = new (ELeave) TBrowserOverriddenSettings;
   					CleanupStack::PushL( aOverrideSettings );
   					aOverrideSettings->SetBrowserSetting(EBrowserOverSettingsContextId, specialContextId);					
       			}
        	}      	
        if(folderUid == KFavouritesNullUid)
        	{        
	        // Resolve folder uid
	        folderUid = FolderByContextIdL( aContextId );
	        if ( folderUid == KFavouritesNullUid )
	            {
	                folderUid = KFavouritesRootUid;
	            }
	        folderPreferredUid = BookmarksIdByFolderIdL( folderUid );
	        }
        }
	LOG_WRITE_FORMAT("folderPreferredUid = 0x%x", folderPreferredUid );
	
	// Initialize client and connect to Browser application.
    iExtension->iLauncherClientService.InitializeL( iBrowserUid, *iExtension );
    
  	LOG_WRITE("After iLauncherClientService.InitializeL ");
    
    // Start listening to Browser application.
    iExtension->iLauncherClientService.ListenL( );
    LOG_WRITE("After iLauncherClientService.ListenL ");
    
    // Set handler of finished download.
    iExtension->iDownloadedContentHandler = aContentHandler;
    // Set handler of server exit.
    iExtension->iBrowserLauncherClientExitObserver = aExitObserver;

    HBufC* seamlessParam = 0;
    if ( !aParams.Length()  && folderPreferredUid  && EnhancedSeamlessLinkLocalFeaturesSupportedL() )
        {
        seamlessParam = HBufC::NewLC( KBrowserSeamlessParamLength );
        seamlessParam->Des().Copy( KBookmarkStarterString );
        seamlessParam->Des().AppendNum( folderPreferredUid );
        }

    // Pack aSettings,folderUid,aParams into a descriptor, and send the
    // data to Browser application.
    // Note: if seamlessParam is not empty, it must be used instead of aParams!
    HBufC8* bufferToSend = 0;    
    if ( seamlessParam )
        {
		// Create descriptor with seamless seamlessParam
        bufferToSend = TLauncherPackageUtils::PackLauncherDataL( embeddingAppUid, aOverrideSettings, folderUid, *seamlessParam, aContentHandler!=0 );
        }
    else
        {
		// Create descriptor with seamless aParams
        bufferToSend = TLauncherPackageUtils::PackLauncherDataL( embeddingAppUid, aOverrideSettings, folderUid, aParams, aContentHandler!=0 );
        }
    // Remember for this buffer in the Extension, because re-initialization.
    iExtension->SetPackedLauncherData( bufferToSend );
    
    // dbg
    // iExtension->iLauncherClientService.SendCommand( EServerExit );
    // iExtension->iLauncherClientService.SendSync( *bufferToSend );
    // dbg
    
    // Send parameters to Browser application.
    iExtension->iLauncherClientService.SendAsync( *bufferToSend, iExtension->iBufferSizeP );
    
    if ( seamlessParam )
        {
        CleanupStack::PopAndDestroy( seamlessParam ); // seamlessParam
        }

	// Cleanup overridden settings. If it was passed in as NULL, and the local
	// version is not, it means we allocated locally for it. Cleanup
	if(!aSettings && aOverrideSettings)
		{
			CleanupStack::PopAndDestroy(1); //aOverrideSettings
		}
    LOG_WRITE("LaunchBrowserEmbeddedL End ");
    }

// -----------------------------------------------------------------------------
// CBrowserLauncher::FolderByContextIdL
// -----------------------------------------------------------------------------
//
TInt CBrowserLauncher::FolderByContextIdL( TUint32 aContextId )
    {
    LOG_ENTERFN( "CBrowserLauncher::FolderByContextIdL" );

    TInt folder = KFavouritesNullUid;
    if ( aContextId != (TUint32)KFavouritesNullContextId )
        {
        CArrayFix<TInt>* uids = new (ELeave) CArrayFixFlat<TInt>( 1 );
        CleanupStack::PushL( uids );

        RFavouritesDb db;
        User::LeaveIfError( db.Open( iFavouritesSess, KBrowserBookmarks ) );
        CleanupClosePushL<RFavouritesDb>( db );
        User::LeaveIfError( db.GetUids( *uids, KFavouritesNullUid,
            CFavouritesItem::EFolder, NULL, aContextId ) );

        if( uids->Count() > 0 )
            {
            folder = uids->At( 0 );
            }
        CleanupStack::PopAndDestroy( 2 );   // db, uids
    }

    return folder;
    }

// -----------------------------------------------------------------------------
// CBrowserLauncher::BookmarksIdByFolderIdL
// -----------------------------------------------------------------------------
//
TInt CBrowserLauncher::BookmarksIdByFolderIdL( TUint32 aFolderId )
    {
    TInt bookmarkId = KFavouritesNullUid;
    if ( aFolderId != (TUint32)NULL )
        {

        RFavouritesDb db;
        User::LeaveIfError( db.Open( iFavouritesSess, KBrowserBookmarks ) );
        CleanupClosePushL<RFavouritesDb>( db );
        User::LeaveIfError( db.PreferredUid( aFolderId , bookmarkId ) );

        CleanupStack::PopAndDestroy( );   // db
        }
    return bookmarkId;
    }

// -----------------------------------------------------------------------------
// CBrowserLauncher::EnhancedSeamlessLinkLocalFeaturesSupportedL
// -----------------------------------------------------------------------------
//
TBool CBrowserLauncher::EnhancedSeamlessLinkLocalFeaturesSupportedL()
    {
    TBool supported = EFalse;
    TInt featureBitmask;

    CRepository* repository = CRepository::NewL( KCRUidBrowserUiLV );
    //Read local features bitmask
        if ( repository->Get( KBrowserUiCommonFlags, featureBitmask ) != KErrNone )
            {
            featureBitmask = 0;
            } 
        if  ( featureBitmask /*& KBrowserUiPreferredBookmarks*/  )
            {    
            supported = ETrue;
            }
    delete repository;

    return supported;
    }

// ---------------------------------------------------------
// TLauncherPackageUtils::PackLauncherDataL()
// ---------------------------------------------------------
//
HBufC8* TLauncherPackageUtils::PackLauncherDataL( const TUid aEmbeddingAppUid, 
												  const TBrowserOverriddenSettings* aSettings, 
												  TInt folderUid, 
												  const TDesC& seamlessParam, 
												  TBool aIsContentHandlerRegistered )
    {
    TBool isOverriddenSettings( EFalse );
    
    // Check if the overridden settings was set
    if ( aSettings )
    	{
    	isOverriddenSettings = ETrue;
    	}
    	
	// Create the package
	TPckgC<TUid> embeddingAppUidPckg( aEmbeddingAppUid );
    TPckgC<TBool> isOverriddenSettingsPckg( isOverriddenSettings );    
    TPckgC<TBrowserOverriddenSettings>* pSettingsPckg( 0 );    
    if ( aSettings )
    	{
    	// Put Overridden settings in package if it is not NULL
    	pSettingsPckg = new (ELeave) TPckgC<TBrowserOverriddenSettings>( *aSettings );
    	CleanupStack::PushL( pSettingsPckg );
    	}   
    TPckgC<TInt> folderUidPckg( folderUid );
    TPtrC8 seamlessParamPtr( (const TUint8*)seamlessParam.Ptr(), seamlessParam.Size() );
    TPckgC<TBool> isContentHandlerRegistered( aIsContentHandlerRegistered );

	// Get the size of all packages
	TInt totalSize;	
	if ( aSettings )
		{
	    totalSize = embeddingAppUidPckg.Size() + 
	    			isOverriddenSettingsPckg.Size() + 
	    			(*pSettingsPckg).Size() + 
	    			folderUidPckg.Size() + 
	    			seamlessParamPtr.Size() + 
	    			isContentHandlerRegistered.Size();
		}
	else
		{
	    totalSize = embeddingAppUidPckg.Size() + 
	     			isOverriddenSettingsPckg.Size() + 
	     			folderUidPckg.Size() + 
	     			seamlessParamPtr.Size() + 
	     			isContentHandlerRegistered.Size();
		}
		
	// Allocate buffer for packages
    HBufC8* buff = HBufC8::NewL( totalSize );
    
    // Pack the packages to buffer
    buff->Des().Copy( embeddingAppUidPckg );
	buff->Des().Append( isOverriddenSettingsPckg );
	if ( aSettings )
		{
		buff->Des().Append( *pSettingsPckg );
		}    	
    buff->Des().Append( folderUidPckg );
    buff->Des().Append( isContentHandlerRegistered );
    buff->Des().Append( seamlessParamPtr );
    
    if ( aSettings )
    	{
    	CleanupStack::PopAndDestroy( ); // pSettingsPckg
    	}    
    return buff;
    }
    
// ---------------------------------------------------------
// TLauncherPackageUtils::PackLauncherDataL()
// ---------------------------------------------------------
//
void TLauncherPackageUtils::UnPackLauncherDataL( TUid& aEmbeddingAppUid, TBrowserOverriddenSettings* aSettings, TInt& aFolderUid, TPtrC& aSeamlessParam, const TDesC8& aData, TBool& aIsContentHandlerRegistered, TBool& aIsOverriddenSettings )
    {
    TInt pos( 0 );

	// Get the first parameter aEmbeddingAppUid from IPC data
    TPckg<TUid> embeddingAppUid( aEmbeddingAppUid );
    embeddingAppUid.Copy( aData.Mid( pos, embeddingAppUid.Size() ) );    
    pos += embeddingAppUid.Size();

	// Get the parameter isOverriddenSettings from IPC data
    TPckg<TBool> isOveriddenSettings( aIsOverriddenSettings );
    isOveriddenSettings.Copy( aData.Mid( pos, isOveriddenSettings.Size() ) );    
    pos += isOveriddenSettings.Size();
    
    if ( aIsOverriddenSettings )
    	{
    	// Unpack the overridden settings
	    TPckg<TBrowserOverriddenSettings> settingsPckg( *aSettings );
	    settingsPckg.Copy( aData.Mid( pos, settingsPckg.Size() ) );
	    pos += settingsPckg.Size();    	
    	}
	else
		{
    	// No overridden setings were put in IPC data
		aSettings = NULL;
		}    	

	// Get second parameters from IPC data
    TPckg<TInt> folderUidPckg( aFolderUid );
    folderUidPckg.Copy( aData.Mid( pos, folderUidPckg.Size() ) );
    pos += folderUidPckg.Size();

	// Get third parameters from IPC data
    TPckg<TBool> isContentHandlerRegistered( aIsContentHandlerRegistered );
    isContentHandlerRegistered.Copy( aData.Mid( pos, isContentHandlerRegistered.Size() ) );    
    pos += isContentHandlerRegistered.Size();

    TPtrC8 remaining = aData.Mid( pos );
    aSeamlessParam.Set( (const TUint16*)remaining.Ptr(), remaining.Size()/2 );
    }
// End of file
