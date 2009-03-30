/*
* Copyright (c) 2005-2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Manages the feed related views and implements the FeedsViewBridge.
*
*/


#include <AknView.h>
#include <AknViewAppUi.h>
#include <BaUtils.h>
#include <BaDesca.h>
#include <BrCtlInterface.h>
#include <GulIcon.h>
#include <data_caging_path_literals.hrh>
#include <eikmenup.h>
#include <StringLoader.h>


#include <BrowserNG.rsg>
#include <BrCtlDialogsProvider.h>
#include <BrowserDialogsProvider.h>
#include <BrCtlDefs.h>
#include <InternetConnectionManager.h>
#include <FeatMgr.h>
#include <Uri16.h>

#include "Browser.hrh"
#include "BrowserAppUi.h"
#include "CommonConstants.h"
#include "BrowserWindow.h"
#include "BrowserWindowManager.h"
#include "BrowserUIVariant.hrh"
#include "Preferences.h"
#include "BrowserGotoPane.h"
#include "BrowserAdaptiveListPopup.h"
#include "FeedsFeedView.h"
#include "FeedsFolderView.h"
#include "BrowserDialogs.h"
#include "FeedsTopicView.h"
#include "BrowserUtil.h"
#include "BrowserSpecialLoadObserver.h"

#include "FeedsClientUtilities.h"

#include "BrowserBmOTABinSender.h"

// CONSTANTS
_LIT(KSupportedMimeTypes, "application/rss+xml;application/atom+xml;text/xml;application/xml");
const TInt KLastUpdateGranularity = 10;

const TInt KWmlSettingsAutomaticUpdatingNotSet = 32767;

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeedsClientUtilities* CFeedsClientUtilities::NewL(CAknViewAppUi& aAppUI, MApiProvider& aApiProvider)
    {
    CFeedsClientUtilities* self = new (ELeave) CFeedsClientUtilities(aAppUI, aApiProvider);
    
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::CFeedsClientUtilities
//
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CFeedsClientUtilities::CFeedsClientUtilities(CAknViewAppUi& aAppUI, MApiProvider& aApiProvider):
		iAppUI(aAppUI), iApiProvider(aApiProvider), iIsUpdatingFeed(EFalse), iWaitDialog(0),
		iEnteredURL(0), iFolderView(0), iTopicView(0), iFeedView(0), 
        iIsConnected(EFalse), iItemIds(20), iItemStatus(20), iMimeTypes(0),
        iFeedUpdateTimeIds(KLastUpdateGranularity), iFeedUpdateTimeTimestamps(KLastUpdateGranularity),
        iPendingFolderItemTitle(NULL), iPendingFolderItemUrl(NULL), iExportFileName(NULL),
	    iSearchAgent(NULL),
    	iSearchOptList(NULL),
    	iFeedImportRequested(EFalse),
    	iRequestCanceled(EFalse),
        iIsWaitDialogDisplayed(EFalse), 
        iFeedsInterface(*this,0),
        iCanceledRequest(CTransaction::ENone)
	{
	}


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ConstructL
//
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::ConstructL()
    {
	iApiProvider.Preferences().AddObserverL( this );
	
	// Set up MimeTypes
	iMimeTypes = HBufC::NewL( KSupportedMimeTypes().Length() + 1 );
    TPtr ptr( iMimeTypes->Des() );
    ptr.Append( KSupportedMimeTypes() );
    ptr.ZeroTerminate();
    }

// -----------------------------------------------------------------------------
// CFolderItem::Search
//
// Searches for a FolderItem with the given name.  If "this"
// isn't a folder it only checks whether or not it matches
// the given name.  If "this" is a folder it also checks 
// all embedded items.
// -----------------------------------------------------------------------------
//
const CFeedsEntity* CFeedsClientUtilities::Search(const TDesC& aName,const CFeedsEntity& aFolder) const
    {
    TPtrC title;
    if (aFolder.GetType() == EFolder)
        {
        aFolder.GetStringValue(EFolderAttributeTitle,title);	
        }
    else
        {
        aFolder.GetStringValue(EFeedAttributeTitle,title);	
        }
    if (aName.CompareF(title) == 0)
        {
    	return &aFolder;
        }
    if (aFolder.GetType() == EFolder)
        {
    	for(TInt i=0;i<aFolder.GetChildren().Count();i++)
    	    {
    		const CFeedsEntity *item;
    		CFeedsEntity *child = aFolder.GetChildren()[i];
    		if((item = Search(aName,*child)) != NULL)
    		    {
    			return item;
    		    }
    	    }
        }

    // Otherwise no match was found.
    return NULL;
    }


// -----------------------------------------------------------------------------
// CFolderItem::Search
//
// Searches for a FolderItem with the given id.  If "this"
// isn't a folder it only checks whether or not it matches
// the given id.  If "this" is a folder it also checks 
// all embedded items.
// -----------------------------------------------------------------------------
//
const CFeedsEntity* CFeedsClientUtilities::Search(TInt aFolderItemId,const CFeedsEntity& aFolder) const
    {
    if (aFolderItemId == aFolder.GetId())
        {
    	return &aFolder;
        }
    if (aFolder.GetType() == EFolder)
        {
    	for(TInt i=0;i<aFolder.GetChildren().Count();i++)
    	    {
    		const CFeedsEntity *item;
    		CFeedsEntity *child = aFolder.GetChildren()[i];
    		if ((item = Search(aFolderItemId,*child)) != NULL)
    		    {
    			return item;
    		    }
    	    }
        }

    // Otherwise no match was found.
    return NULL;
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::~CFeedsClientUtilities
//
// Deconstructor.
// -----------------------------------------------------------------------------
//
CFeedsClientUtilities::~CFeedsClientUtilities()
    {
    DisconnectFromServer();
    
    iApiProvider.Preferences().RemoveObserver( this );
    
    delete iWaitDialog;   
    delete iMimeTypes;
    
    iFeedUpdateTimeIds.Close();
    iFeedUpdateTimeTimestamps.Close(); 
    
    delete iPendingFolderItemTitle;
    delete iPendingFolderItemUrl;
    delete iExportFileName;
    delete iEnteredURL;
    delete iSearchAgent;
    
    iItemIds.Close();
    iItemStatus.Close();
    iItemStatusOrig.Close();    

    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::RequestCompleted()
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::RequestCompleted(CTransaction* aTrans,TInt /*aEvent*/)
{
TInt status = aTrans->GetStatusCode();

switch (aTrans->Type())
    {
    	case CTransaction::EFetchRootFolderItem:
   	        FolderItemRequestCompleted(status, CTransaction::EFetchRootFolderItem);

    	    break;
    	
    	case CTransaction::EWatchForChanges:
    	    break;
        
    	case CTransaction::EExportOPML:
    	case CTransaction::EImportOPML:
    	    FolderItemRequestCompleted(status, aTrans->Type());
    	    break;
    	    
        case CTransaction::EAddFolderItem:
        case CTransaction::EDeleteFolderItem:
        case CTransaction::EChangeFolderItem:
        case CTransaction::EMoveFolderItem:
        case CTransaction::EMoveFolderItemTo:
        case CTransaction::EUpdateFolderItem:
    	    FolderItemRequestCompleted(status, aTrans->Type());
    	    break;

        case CTransaction::EChangeSettings:
            break;

    	case CTransaction::EFetchFeed:
   	        FeedRequestCompleted(status);
    	    break;
    	case CTransaction::EUpdateItemStatus:
            break;
        
        default:
            FolderItemRequestCompleted(status, aTrans->Type());
            break;
    }
}
    
// -----------------------------------------------------------------------------
// CFeedsClientUtilities::NetworkConnectionNeededL()
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::NetworkConnectionNeededL(
        TInt* aConnectionPtr, TInt& aSockSvrHandle,
        TBool& aNewConn, TApBearerType& aBearerType )
    {
    iApiProvider.SpecialLoadObserver().NetworkConnectionNeededL(
        aConnectionPtr, &aSockSvrHandle, &aNewConn, &aBearerType );
     //Wait dialog is shown only in the case of a new connection.
     if ( aNewConn )
         {
         iWaitDialog->ShowWaitDialogL(R_FEEDS_UPDATING_FEED);
         iIsWaitDialogDisplayed = ETrue;
         }
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::HandlePreferencesChangeL()
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::HandlePreferencesChangeL(
        const TPreferencesEvent aEvent,
        TPreferencesValues& aValues,
        TBrCtlDefs::TBrCtlSettings aSettingType )
    {
    // no specific TBrCtlDefs::TBrCtlSettings are defined for FeedsSettings
    // so we trigger for an Unknown setting
    if(   (EPreferencesItemChange == aEvent || EPreferencesDeactivate == aEvent)  &&
          (TBrCtlDefs::ESettingsUnknown == aSettingType ) )
        {
        SetPreferencesToFeedL( aValues );
        }
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::SetPreferencesToFeedL()
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::SetPreferencesToFeedL( const TPreferencesValues& aValues )
    {
    if ( iIsWaitDialogDisplayed )
        {
    	return;
        }    
    TFeedsServerSetting setting;

    // always set to 32767 in the case of feed level auto updating.
    //setting.iAutoUpdate = KWmlSettingsAutomaticUpdatingNotSet;
    setting.iAutoUpdate = EFalse;
    setting.iAutoUpdateFreq = KWmlSettingsAutomaticUpdatingNotSet;
  setting.iAutoUpdateWhileRoaming = aValues.iAutomaticUpdatingWhileRoaming;
    
    TUint32 autoUpdatingAP( 0 );
#ifndef __WINSCW__
    if (aValues.iAutomaticUpdatingAP != KWmlNoDefaultAccessPoint)
        {
        autoUpdatingAP = Util::IapIdFromWapIdL( iApiProvider, aValues.iAutomaticUpdatingAP );
        }
#endif //__WINSCW__
	setting.iAutoUpdateAP = autoUpdatingAP;

    SetFeedsServerSettingsL( setting );
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::DisconnectManualUpdateConnectionL()
//
// Disconnect connection provided by client for manual update.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::DisconnectManualUpdateConnectionL()
    {
    if ( iIsConnected )
        {
        // Ensure that the connection is available.
        ConnectToServerL(EFalse);

        // Pass the updated settings to the server.
        iFeedsInterface.DisconnectManualUpdateConnectionL();
        }
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::DisconnectFeedsViewL()
//
// Disconnect connection used by FeedsView.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::DisconnectFeedsViewL()
    {
    if( iFeedView )
        {
        iFeedView->DisconnectL();
        }
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::LoadUrlL()
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::LoadUrlL( const TDesC& aUrl )
    {
        
    if( iApiProvider.IsPageLoaded() &&
		iApiProvider.Preferences().UiLocalFeatureSupported( KBrowserMultipleWindows ) &&
    	!iApiProvider.Preferences().UiLocalFeatureSupported( KBrowserMinimalMultipleWindows ))
	    {	    
	    // there is already a window, so create a new one if not over the max number of windows allowed
	    if(iApiProvider.WindowMgr().WindowCount() == iApiProvider.WindowMgr().MaxWindowCount())
	    	{
	    	// Show warning to user
            TBrowserDialogs::ErrorNoteL( R_BROWSER_NOTE_MAX_WINDOWS );
		  	return;
	    	}

	    if(iApiProvider.WindowMgr().WindowCount() < iApiProvider.WindowMgr().MaxWindowCount())
	    	{
		    CBrowserWindow *win = iApiProvider.WindowMgr().CreateWindowL( 0, &KNullDesC );
		    if (win != NULL)
    		    {
    		    CleanupStack::PushL( win );
    		    iApiProvider.WindowMgr().SwitchWindowL( win->WindowId() );
    		    CleanupStack::Pop();  // win		        
    		    }
	    	}
	    }
	
	iApiProvider.WindowMgr().CurrentWindow()->SetHasFeedsContent(ETrue);
	iApiProvider.FetchL( aUrl );
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ShowFolderViewL
//
// Shows the folder view, loading the folder list from the FeedsServer if need be.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::ShowFolderViewL(TUid aCalledFromView /*= KUidBrowserBookmarksViewId*/)
    {
    
    if ( iIsWaitDialogDisplayed )
        {
    	return;
        }
   	// Record the view from which feeds was launched.
   	// If this function is not called with an argument,
   	// its default value "KUidBrowserBookmarksViewId" is used.
   	// This is specified in the header FeedsClientUtilities.h 
   	SetCalledFromView(aCalledFromView);    	 

    // Ensure that "this" is ready for uses.
    LazyInitL(EFalse);

    // Show wait dialog.
    iWaitDialog->ShowWaitDialogL(R_FEEDS_OPENING_FEED);
    iIsWaitDialogDisplayed = ETrue;
    
    // Set the view to show once the folder is ready.
    iNextViewId = KUidBrowserFeedsFolderViewId;
    iIsUpdatingFeed = EFalse;   

    // Get the root folder.
    FetchRootFolderL();
    iCurrentRequest = CTransaction::EFetchRootFolderItem;
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ShowTopicViewL
//
// Shows the topic view, loading the given feed associated with the given folder item.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::ShowTopicViewL(const CFeedsEntity& aFolderItem)
    {
    TTime timestamp;
    
    if ( iIsWaitDialogDisplayed )
        {
    	return;
        }    
        
    // Ensure that "this" is ready for uses.
    LazyInitL(EFalse);

    // Show wait dialog.
    timestamp = aFolderItem.GetTimeValue(EFeedAttributeTimestamp,timestamp);
    if ( timestamp.Int64() == 0 )
        {
    	if ( iApiProvider.Connection().Connected() )
        	{
    		iWaitDialog->ShowWaitDialogL(R_FEEDS_UPDATING_FEED);
        	}
        }
    else
        {
    	iWaitDialog->ShowWaitDialogL(R_FEEDS_OPENING_FEED);
        }
    iIsWaitDialogDisplayed = ETrue;   
    
    // Set the view to show once the feed is ready.
    iNextViewId = KUidBrowserFeedsTopicViewId;
    iIsUpdatingFeed = EFalse;
    
    // Fetch the feed.
    FetchFeedL(aFolderItem);
    iCurrentRequest = CTransaction::EFetchFeed;
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::SupportedMimeTypesL
//
// Returns the feed related mime-types supported by the bridge.  This makes 
// for a clean way to pass downloaded feeds content from the client to the FeedsServer 
// (via CFeedsClientUtilities::HandleFeedL).
// -----------------------------------------------------------------------------
//
TPtrC CFeedsClientUtilities::SupportedMimeTypesL()
    {
    return iMimeTypes->Des();
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::SubscribeToL
//
// Shows the folder view and subscribes to the given feed.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::SubscribeToL(const TDesC& aTitle, const TDesC& aUrl)
    {  
   	SetCalledFromView(KUidBrowserContentViewId); 

    // If the root folder is available then Add the feed.
    if (iFeedsInterface.RootFolder() != NULL)
        {        
        // Ensure that "this" is ready for uses.  
        LazyInitL( EFalse );
		CFeedsMap* temp = CFeedsMap::NewL();
		temp->SetStringAttribute(EFeedAttributeTitle,aTitle);
        temp->SetStringAttribute(EFeedAttributeLink,aUrl);
        temp->SetIntegerAttribute(EFeedAttributeAutoUpdateFreq,0);
        
        // Set the next view to show after the new add folder item is added.
        iNextViewId = KUidBrowserFeedsFolderViewId; 
        
        iFeedsInterface.AddL(EFeed,*temp, *(iFeedsInterface.RootFolder()));

		delete temp;
        // Ensure the Folder View shows the root-folder when AddFolderItemL completes.
        iFolderView->SetCurrentFolder(*(iFeedsInterface.RootFolder()), ETrue);
        }
        
    // Otherwise queue the folder item until the root folder is fetched (see above).
    else
        {
        // Ensure that "this" is ready for uses.  ETrue is passed to ensure that
        // the root folder will be fetched if it isn't already available.
        LazyInitL( ETrue );

        delete iPendingFolderItemTitle;
        iPendingFolderItemTitle = NULL;
        delete iPendingFolderItemUrl;
        iPendingFolderItemUrl = NULL;
        
        iPendingFolderItemTitle = aTitle.AllocL();
        TRAPD(err, iPendingFolderItemUrl = aUrl.AllocL());
        if (err != KErrNone)
            {
            delete iPendingFolderItemTitle;
            iPendingFolderItemTitle = NULL;
            
            User::Leave(err);
            }
        }
    }




// -----------------------------------------------------------------------------
// CFeedsClientUtilities::FeedsServerSettings
//
// Returns Feeds Server related settings.
// -----------------------------------------------------------------------------
//
TInt CFeedsClientUtilities::FeedsServerSettingsL(TFeedsServerSetting& aSetting)
    {
    // Ensure that the connection is available.
    ConnectToServerL(EFalse);

    return iFeedsInterface.GetSettingsL(aSetting);
    }
    

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::SetFeedsServerSettings
//
// Sets Feeds Server related settings.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::SetFeedsServerSettingsL(const TFeedsServerSetting& aNewSetting)
    {
    // Ensure that the connection is available.
    ConnectToServerL(EFalse);

    // Pass the updated settings to the server.
    iFeedsInterface.SetSettingsL(aNewSetting);
    }
    

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::SetBrowserControlSettingL
//
// Sets a Browser Control related settings.  These settings are directly passed to all 
// Browser Control instances used by the CFeedsViewBridge.  As such see the Browser
// Control documentation for infomation about the settings.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::SetBrowserControlSettingL(TUint aSetting, TUint aValue)
    {
    // Pass the setting to the Feed View's Browser Control.
    iApiProvider.BrCtlInterface().SetBrowserSettingL(aSetting, aValue);
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::FolderItemRequestCompleted
//
// Called by RequestHandlerCompleted when the root FolderItem is either ready 
// or an error occured.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::FolderItemRequestCompleted(TInt aStatus, CTransaction::TTransactionType aTransType/*=ENone*/)
    {
	TRAP_IGNORE( FolderItemRequestCompletedL(aStatus, aTransType) );
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::FolderItemRequestCompletedL
//
// Called by FolderItemRequestCompleted.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::FolderItemRequestCompletedL(TInt aStatus, CTransaction::TTransactionType aTransType/*=ENone*/)
    {
    if (aTransType == iCanceledRequest)
        {
        iCanceledRequest = CTransaction::ENone;
        return;
        }
    // Close the wait dialog.
    if (iWaitDialog)
        {
        iWaitDialog->Close();
        }
    iIsWaitDialogDisplayed = EFalse;
    	
    // Show the view if everything went ok.
    if (aStatus == KErrNone)
        {
        switch(aTransType)
        	{
        	case CTransaction::EExportOPML:
	        	//launch send ui
	        	SendOPMLFileL();
				break;

       		case CTransaction::EImportOPML:
				// Delete temp file here if it exists
				if(iTempOPMLImportFilePath.Length() > 0)
					{
					RFs tempRFs;
					if (tempRFs.Connect() != KErrNone)
						{
						break;
						}
					CleanupClosePushL(tempRFs);
					tempRFs.Delete( iTempOPMLImportFilePath );
					tempRFs.Close();
					CleanupStack::PopAndDestroy(); // cleanup tempRFs and reset temp file import path
					iTempOPMLImportFilePath = _L("");
					}
									
		        // Again show folder view to trigger a redraw
				ShowFolderViewL();	
				break;

        	default:
        		// if an import OPML was requested, do that now
        	    if(iFeedImportRequested)
        			{
        			// clear flag
        			iFeedImportRequested = EFalse;

        			// show wait dialog
					iWaitDialog->ShowWaitDialogL(R_FEEDS_WAIT_IMPORTING_FEEDS);
				    iIsWaitDialogDisplayed = ETrue;
					
        	
					if(iTempOPMLImportFilePath.Length())
						{
						iFeedsInterface.ImportOPMLL( iTempOPMLImportFilePath );
						}
            iCurrentRequest = CTransaction::EImportOPML;
        			}       	
        	
		        // Set the updated folder
		        iFolderView->RootFolderChangedL(*(iFeedsInterface.RootFolder()));
		        
		        // Show it.
		        if (iNextViewId == KUidBrowserFeedsFolderViewId)
		            {            
		            ShowFolderViewLocalL();
		            }
		        
		        // If the user tried to subscribe to a feed before it was connected
		        // to the server then add the item now.
		        if (iPendingFolderItemTitle != NULL)
		            {
		            TRAPD(err, SubscribeToL(*iPendingFolderItemTitle, *iPendingFolderItemUrl));
		            if (err == KErrNone)
		                {
		                delete iPendingFolderItemTitle;
		                iPendingFolderItemTitle = NULL;
		                delete iPendingFolderItemUrl;
		                iPendingFolderItemUrl = NULL;                
		                }
		            // Don't reset next-view; iNextViewId is EFolderViewId after above SubscribeToL() call
		                
		            // Otherwise try to add the folder item next time.
		            }
		        else if(iNextViewId == KUidBrowserFeedsFolderViewId)
		            {
		            // Reset next-view.
		            iNextViewId = KUidBrowserNullViewId;
		            }

		        // Reset the "last updated" cache now that a new folder list is available.
		        ResetFeedUpdateTime();        
        	}
        }
    // Otherwise show an error.
    else
        {
        // If the server terminated, reconnect to it.
        if (aStatus == KErrServerTerminated)
            {
            DisconnectFromServer();
            ConnectToServerL();
           
            }
        
        ShowServerError(aStatus, aTransType);
        
        // This happens when user subscribe to a feed from content view
        // If it failed, we want to show the content view.
        if (aStatus == KErrAlreadyExists)
            {
            // Set content view as the last view id.
            iApiProvider.SetLastActiveViewId( KUidBrowserContentViewId );
    
            // Show the view.
            // We already started switching to the feeds folder view and shut off full screen
            // This will force the DoActivate on the ContentView and set back to full screen
            iApiProvider.SetViewToBeActivatedIfNeededL( KUidBrowserFeedsFolderViewId );
            iApiProvider.SetViewToBeActivatedIfNeededL( KUidBrowserContentViewId );
            }

        // Reset next-view.
        iNextViewId = KUidBrowserNullViewId;
        }        
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::FeedRequestCompleted
//
// Called when the asynchronous request is complete.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::FeedRequestCompleted(TInt aEvent)
    {
    TRAP_IGNORE( FeedRequestCompletedL(aEvent) );
    }
    
// -----------------------------------------------------------------------------
// CFeedsClientUtilities::FeedRequestCompletedL
//
// Called when the asynchronous request is complete.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::FeedRequestCompletedL(TInt aStatus)
    {
    if (CTransaction::EFetchFeed == iCanceledRequest)
        {
        iCanceledRequest = CTransaction::ENone;
        return;
        }    
    // Close the wait dialog.
    iWaitDialog->Close();
    iIsWaitDialogDisplayed = EFalse;

    // Show the view if everything went ok.
    if (aStatus == KErrNone)
        {
        // Extract out the intial item status -- later on, iItemStatusOrig is 
        // used to determine which item status' change.
        RArray<TInt>  ignore(20);
        
        iItemIds.Reset();
        iItemStatus.Reset();
        iItemStatusOrig.Reset();
        
        CleanupClosePushL(ignore);
        ItemStatusL(iItemIds, iItemStatus, *(iFeedsInterface.Feed()));
        ItemStatusL(ignore, iItemStatusOrig, *(iFeedsInterface.Feed()));
        CleanupStack::PopAndDestroy(/*ignore*/);
        
        // update folder view for unread count
        TInt unreadCount = 0;
        for (TInt i = 0; i < iItemStatus.Count(); i++)
            {
            if ( iItemStatus[i] == EItemStatusUnread || iItemStatus[i] == EItemStatusNew )
                {
                unreadCount++;
                }
            }
        iFolderView->SetUnreadCountL( unreadCount );               

        // The method was called because of a updated feed.  In general, update
        // the current view to reflect the updated feed.
        if (iIsUpdatingFeed)
            {
            if (iApiProvider.LastActiveViewId() == KUidBrowserFeedsTopicViewId)
            	{
				iTopicView->SetCurrentFeedL(*(CurrentFeed()), 0);            	
                }
            }
        
        // Otherwise, this is a newly requested feed, so show the next
        // view now that it is available.
        else
            {            
            if (iNextViewId == KUidBrowserFeedsFeedViewId)
                {
                ShowFeedViewLocalL(0);
                }
            else if (iNextViewId == KUidBrowserFeedsTopicViewId)
            	{
            	ShowTopicViewLocalL(0);
                }
            }
            
        // Update the feed's "last updated" value in the cache.
        UpdateFeedUpdatedTimeL(*(iFeedsInterface.Feed()));
        }
    
    // Otherwise show an error.
    else
        {
        // If the server terminated, reconnect to it.
        if (aStatus == KErrServerTerminated)
            {
            DisconnectFromServer();
            ConnectToServerL();
            
            }
            
        ShowServerError(aStatus);
        }

    // Reset updating feed.
    iIsUpdatingFeed = EFalse;

    // Reset next-view.
    iNextViewId = KUidBrowserNullViewId;
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::DialogDismissedL
//
// Called when the user presses the cancel button.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::DialogDismissedL()
    {
    // Cancel all activities that can be cancelled.
    iIsWaitDialogDisplayed = EFalse;

    
    // If there is an ongoing search by the search agent, make sure
    // it's cancelled and search agent destroyed  
    if(iSearchAgent != NULL)
        {
        iSearchAgent->CancelSearch();
        delete(iSearchAgent);
        iSearchAgent = NULL;
        }
    else
        {
        iCanceledRequest = iCurrentRequest;
        iFeedsInterface.CancelAllL();
        }    
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::FetchRootFolderL
//
// Get the root folder from the Feeds server.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::FetchRootFolderL()
    {
    iFeedsInterface.GetRootFolderL();
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::AddFolderItemL
//
// Add a new folder item.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::AddFolderItemL(const TDesC& aTitle, const TDesC& aUrl,
        TBool aIsFolder, const CFeedsEntity& aParent, TInt aFreq)
    {
    
		CFeedsMap* temp = CFeedsMap::NewL();
		
		temp->SetStringAttribute(EFeedAttributeTitle,aTitle);
		temp->SetStringAttribute(EFeedAttributeLink,aUrl);	
        temp->SetIntegerAttribute(EFeedAttributeAutoUpdateFreq,aFreq);

        // Set the next view to show after the new add folder item is added.
        iNextViewId = KUidBrowserFeedsFolderViewId;
    	
        iFeedsInterface.AddL(aIsFolder?EFolder:EFeed,*temp, aParent);

		delete temp;
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ChangeFolderItemL
//
// Change the folder item.  If this is a folder then KNullDesC should be passed 
// to the aUrl argument.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::ChangeFolderItemL(CFeedsEntity& aFolderItem, 
                const TDesC& aTitle, const TDesC& aUrl,TInt aFreq)
    {
	CFeedsMap* temp = CFeedsMap::NewL();
	temp->SetStringAttribute(EFeedAttributeTitle,aTitle);
    temp->SetStringAttribute(EFeedAttributeLink,aUrl);
    temp->SetIntegerAttribute(EFeedAttributeAutoUpdateFreq,aFreq);
    // Set the next view to show after the new add folder item is added.
    iNextViewId = KUidBrowserFeedsFolderViewId;
    	
    aFolderItem.ChangeValueL(*temp);
    delete temp;
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::DeleteFolderItemsL
//
// Delete the folder items. 
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::DeleteFolderItemsL(RPointerArray<const CFeedsEntity>& aFolderItems)
    {
    iFeedsInterface.DeleteL(aFolderItems);
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::MoveFolderItemsToL
//
// Move the folder items to a different parent. 
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::MoveFolderItemsToL(RPointerArray<const CFeedsEntity>& aFolderItems,
        const CFeedsEntity& aParent)
    {
    iFeedsInterface.MoveToFolderL(aFolderItems, aParent);
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::MoveFolderItemsL
//
// Move the folder item to a different index. 
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::MoveFolderItemsL(RPointerArray<const CFeedsEntity>& aFolderItems,
        TInt aIndex)
    {
    iFeedsInterface.MoveL(aFolderItems, aIndex);
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::FetchFeedL
//
// Get the given feed from the Feeds server.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::FetchFeedL(const CFeedsEntity& aFeedEntity, TBool aForceUpdate, 
        TBool aNoCache)
    {
    // Fetch the new feed.
    iFeedsInterface.FetchL(aFeedEntity, aForceUpdate, aNoCache);
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::UpdateFeedL
//
// Updates the feed with the given id.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::UpdateFeedL(const CFeedsEntity& aFeedEntity)
    {
    if(iIsWaitDialogDisplayed)
    {
    	return;
    }
    // Show wait dialog iff connection is already established.
    if(iApiProvider.Connection().Connected())
    {
        iWaitDialog->ShowWaitDialogL(R_FEEDS_UPDATING_FEED);
        iIsWaitDialogDisplayed = ETrue;
    }    
    
    iIsUpdatingFeed = ETrue;

    // Fetch the feed.
    FetchFeedL(aFeedEntity, ETrue);
    iCurrentRequest = CTransaction::EFetchFeed;
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::UpdateFolderItemsL
//
// Updates the given feeds in the background.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::UpdateFolderItemsL(const RPointerArray<const CFeedsEntity>& 
        aFolderItems)
    {
    if (iIsWaitDialogDisplayed)
        {
    	return;
        }    
    // Show wait dialog iff connection is already established.
    if (iApiProvider.Connection().Connected())
        {
        iWaitDialog->ShowWaitDialogL(R_FEEDS_UPDATING_FEED);
        iIsWaitDialogDisplayed = ETrue;
        }    

    // Fetch the feeds.
    iFeedsInterface.UpdateL(aFolderItems);
    iCurrentRequest = CTransaction::EUpdateFolderItem;
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::UpdateFolderItemsL
//
// Updates all of feeds in the background.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::UpdateFolderItemsL()
    {
    if (iIsWaitDialogDisplayed)
        {
    	return;
        } 
       
    // Show wait dialog iff connection is already established.
    if (iApiProvider.Connection().Connected())
        {
        iWaitDialog->ShowWaitDialogL(R_FEEDS_UPDATING_ALL_WAIT_DIALOG);
        iIsWaitDialogDisplayed = ETrue;
        }    

    // Fetch the feeds.
    RPointerArray<const CFeedsEntity> aFolderItem;
    aFolderItem.Append((iFeedsInterface.RootFolder()));
    
    iFeedsInterface.UpdateL(aFolderItem);
    iCurrentRequest = CTransaction::EUpdateFolderItem;
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ImportFeeds
//
// Import feeds from OPML file
// -----------------------------------------------------------------------------
//

void CFeedsClientUtilities::ImportFeedsL()
	{
    if (iIsWaitDialogDisplayed)
        {
    	return;
        }    
	
	// construct search agent
	if(NULL != iSearchAgent)
		{
		delete(iSearchAgent);
		iSearchAgent = NULL;
		}
	iSearchAgent = CFeedsFileSearchAgent::NewL(*this);

    iWaitDialog->ShowWaitDialogL(R_FEEDS_WAIT_SEARCHING_FOR_FEEDS);
    iIsWaitDialogDisplayed = ETrue;

	// Start search
	if(NULL != iSearchAgent)
		{
		iSearchAgent->StartSearchingL();
		}
	}

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::FeedsFileSearchCompleteL
//
// Implemented from the MFeedsFileSearchAgentCallback mixin, this
// callback is called when the search for files in the filesystem has completed
// -----------------------------------------------------------------------------
//

void CFeedsClientUtilities::FeedsFileSearchCompleteL(TInt aCount)
	{
	TFileEntry * fileEntry;
	TInt i;

	iWaitDialog->Close();
	iIsWaitDialogDisplayed = EFalse;
	
	// If nothing is found, indicate that, cleanup and quit
	if(aCount == 0)
		{
	    HBufC* note = StringLoader::LoadLC(R_FEEDS_NO_FEEDS_FOUND_ON_DEVICE);
        iApiProvider.DialogsProvider().DialogAlertL(_L(""),*note);       
        CleanupStack::PopAndDestroy( note );
		}
	else
		{
		//
		// construct results dialog box to show search results
		//
		if(NULL != iSearchOptList)
			{
			delete(iSearchOptList);
			iSearchOptList = NULL;
			}
		
		iSearchOptList = new( ELeave ) CArrayFixFlat<TBrCtlSelectOptionData>(aCount);
                
	    HBufC* title = StringLoader::LoadLC(R_FEEDS_POPUP_TITLE_FEEDS_FILES_FOUND);	    

		// loop through the search results
        for(i = 0; i < aCount; i++)
	        {
	        if(iSearchAgent)
	        	{
					// grab file found from the search agent results and add it 
					// as an option
	        		fileEntry = iSearchAgent->GetFileEntry(i);
	        		if(NULL != fileEntry)
	        			{ 
			            TBrCtlSelectOptionData t(fileEntry->iEntry.iName, EFalse, EFalse, EFalse);
        		    	iSearchOptList->AppendL(t);
    	       			}
	        	}
	        }
	    
        TBool ret(iApiProvider.DialogsProvider().DialogSelectOptionL( *title,
        	ESelectTypeNone, *iSearchOptList));
        	
		CleanupStack::PopAndDestroy(title);
		
		//
		// If the user selects an option, import it
		//		
    	if ( ret )
        	{
       		for( i = 0; i < aCount; i++)
            	{
            	if( (*iSearchOptList)[i].IsSelected() )
                	{
	                if(iSearchAgent)
			        	{
	        			fileEntry = iSearchAgent->GetFileEntry(i);
	        			if(NULL != fileEntry)
	        				{
	        				BeginImportOPMLFeedsFileL(fileEntry->iPath);
	        				}
			        	}
			        	break;
                	}
            	}
        	}
	
		}

		// destroy the search agent
		delete(iSearchAgent);
		iSearchAgent = NULL;
	}

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::BeginImportOPMLFeedsFileL
//
// Given a path, save it in the class, show the root folder view and set
// a flag to indicate we're importing. When the root folder is done opening,
// then begin the actual import
// -----------------------------------------------------------------------------
//

void CFeedsClientUtilities::BeginImportOPMLFeedsFileL( TFileName& aFilepath )
	{
		// save path
		iTempOPMLImportFilePath = aFilepath;

		// Switch to feeds view, setting the flag will import 
		// from the tempOPMLImportFilePath when the view is ready
		iFeedImportRequested = ETrue;		
		ShowFolderViewL();	
	
	}

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ExportFeeds
//
// Export feeds to OPML file
// -----------------------------------------------------------------------------
//

void CFeedsClientUtilities::ExportFeedsL(RPointerArray<const CFeedsEntity>& aFolderItems, const TDesC &aExportFileName)
	{
    if (iIsWaitDialogDisplayed)
        {
    	return;
        }    

	iWaitDialog->ShowWaitDialogL(R_FEEDS_WAIT_PROCESSING);
	iIsWaitDialogDisplayed = ETrue;
	
	if (iExportFileName)
		{
    	delete iExportFileName;
    	iExportFileName = NULL;
		}
	
	iExportFileName = aExportFileName.AllocL();
    iFeedsInterface.ExportFoldersL(aFolderItems, aExportFileName);
    iCurrentRequest = CTransaction::EExportOPML;
	}

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ShowFolderViewLocalL
//
// Shows the folder view.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::ShowFolderViewLocalL()
    {
    // Set this as the last view id -- this is used in ShowLastViewL.
    iApiProvider.SetLastActiveViewId(KUidBrowserFeedsFolderViewId);
    
    // Show the view.
    iApiProvider.SetViewToBeActivatedIfNeededL(KUidBrowserFeedsFolderViewId);    
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ShowTopicViewLocalL
//
// Shows the topic view.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::ShowTopicViewLocalL(TInt aInitialItem)
    {
    // Set this as the last view id -- this is used in ShowLastViewL.
    iApiProvider.SetLastActiveViewId(KUidBrowserFeedsTopicViewId);
    
    // Set the inital item.
    if (iTopicView == NULL)
		{
		TRect rect(iAppUI.ClientRect());
		iTopicView = CFeedsTopicView::NewL( iApiProvider, rect );
		iAppUI.AddViewL( iTopicView ); // transfer ownership to CAknViewAppUi
		}

    iTopicView->SetInitialItem(aInitialItem);

    // Show the view.
    iApiProvider.SetViewToBeActivatedIfNeededL(KUidBrowserFeedsTopicViewId);    
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ShowFeedViewLocalL
//
// Shows the feed view.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::ShowFeedViewLocalL(TInt aInitialItem)
    {
    // Set this as the last view id -- this is used in ShowLastViewL.
    iApiProvider.SetLastActiveViewId(KUidBrowserFeedsFeedViewId);
    
    // Set the inital item.
    if (iFeedView == NULL)
		{
		TRect rect(iAppUI.ClientRect());
		iFeedView = CFeedsFeedView::NewL( iApiProvider, rect );
		iAppUI.AddViewL( iFeedView ); // transfer ownership to CAknViewAppUi
		}

    iFeedView->SetInitialItem(aInitialItem);

    // Show the view.
    iApiProvider.SetViewToBeActivatedIfNeededL(KUidBrowserFeedsFeedViewId);    
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::CurrentRootFolder
//
// Returns the current root folder.
// -----------------------------------------------------------------------------
//
const CFeedsEntity* CFeedsClientUtilities::CurrentRootFolder() 
    {
    return iFeedsInterface.RootFolder();
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::CurrentFeed
//
// Returns the current feed.
// -----------------------------------------------------------------------------
//
CFeedsEntity* CFeedsClientUtilities::CurrentFeed() 
    {
    return iFeedsInterface.Feed();
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::AddItemL
//
// Add a menu item to the given menu.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::AddItemL(CEikMenuPane& aMenuPane, TInt aCommand, 
        TInt aTitleId)
    {
    CEikMenuPaneItem::SData  item;
    HBufC*                   buf = NULL;
    
    buf = StringLoader::LoadLC(aTitleId);    
    item.iText.Copy(*buf);
    CleanupStack::PopAndDestroy(buf);
    buf = NULL;
    
    item.iCommandId = aCommand;
    item.iFlags = 0;
    item.iCascadeId = 0;
    aMenuPane.AddMenuItemL(item);
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::AddCascadeL
//
// Add a sub-menu to the given menu.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::AddCascadeL(CEikMenuPane& aMenuPane, TInt aCommand, 
        TInt aTitleId, TInt aCascade)
    {
    CEikMenuPaneItem::SData  item;
    HBufC*                   buf = NULL;
    
    buf = StringLoader::LoadLC(aTitleId);    
    item.iText.Copy(*buf);
    CleanupStack::PopAndDestroy(buf);
    buf = NULL;
    
    item.iCommandId = aCommand;
    item.iFlags = 0;
    item.iCascadeId = aCascade;
    aMenuPane.AddMenuItemL(item);
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::SetItemStatus
//
// Sets the item's status (read/unread/new).
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::SetItemStatusL(CFeedsEntity* aItem, TFeedItemStatus aStatus)
    {
    TInt	ret;
	CFeedsMap* temp = CFeedsMap::NewL();
	temp->SetIntegerAttribute(EItemAttributeStatus,aStatus);
    ret = aItem->ChangeValueL(*temp);
	delete temp;

    if (ret != KErrNone)
        {
        ShowServerError(ret);
        }
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ItemStatus
//
// Returns the item's status (read/unread/new).
// -----------------------------------------------------------------------------
//
TFeedItemStatus CFeedsClientUtilities::ItemStatus(TInt aItemId)
    {
    TInt         pos;
    TFeedItemStatus  status = EItemStatusUndefined;
    
    if ((pos = iItemIds.Find(aItemId)) != KErrNotFound)
        {        
        status = iItemStatus[pos];
        }
    else
        {
        // TODO: panic
        }
        
    return status;
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ItemStatusWriteToServerL
//
// Writes the item status out to the server.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::ItemStatusWriteToServerL()
    {
    RArray<TInt>         ids(20);
    RArray<TFeedItemStatus>  status(20);
    TInt     unreadCountDelta = 0;
    TInt     unreadCount = 0;
    
    if (iItemStatus.Count() == 0)
        {
        return;
        }
        
    CleanupClosePushL(ids);
    CleanupClosePushL(status);
    
    // Build new status arrays that only contains items that changed.
    for (TInt i = 0; i < iItemStatus.Count(); i++)
        {
        if (iItemStatus[i] != iItemStatusOrig[i])
            {
            // update orig, since the old orig doesn't hold true any more
            // important to do so: 
            // because this method will be called whenever deactivateView of FeedsView is called
            iItemStatusOrig[i] = iItemStatus[i];

            ids.AppendL(iItemIds[i]);
            status.AppendL(iItemStatus[i]);

            // In current UI, status can only change from New -> Read, or Unread -> Read
            if ( iItemStatus[i] == EItemStatusRead )
                {
                unreadCountDelta--;
                }
            }
        }    

    // update folder view
    unreadCount = iFolderView->UnreadCountChangedL( unreadCountDelta );

    // Write the item status out to the server.
    iFeedsInterface.UpdateFeedItemStatusL(ids, status, unreadCount);
    CleanupStack::PopAndDestroy(/*status*/);
    CleanupStack::PopAndDestroy(/*ids*/);
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::FaviconL
//
// Returns the Favicon to the given url or NULL if one isn't found.
// -----------------------------------------------------------------------------
//
CGulIcon* CFeedsClientUtilities::FaviconL(const TDesC& aUrl)
    {
    CGulIcon*    icon = NULL;
    
    // TODO: Do this if access to the database is too slow.
        // First search the local cache.
        
        // If not found extract it from the Feeds View's Browser Control and add
        // it to the local cache.

    // Get the favicon from the Browser Control.
	icon = iApiProvider.WindowMgr().CurrentWindow()->BrCtlInterface().GetBitmapData(aUrl, TBrCtlDefs::EBitmapFavicon);    
    if (icon != NULL)
        {
        icon->SetMask(NULL);
        }
    
    return icon;
    }
    

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::LazyInitL
//
// Ensures that the views and the connection to the FeedsServer are ready.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::LazyInitL(TBool aGetRootFolder)
    {   
    // Create Views
	if(iFolderView == NULL)
		{
	    TRect rect(iAppUI.ClientRect());
	    iFolderView = CFeedsFolderView::NewL( iApiProvider, rect );
	    iAppUI.AddViewL( iFolderView ); // transfer ownership to CAknViewAppUi
		}
		
    // Create the WaitDialog
    if (iWaitDialog == NULL)
        {        
        iWaitDialog = CFeedsWaitDialog::NewL(*this);
        }
    
    // Connect to the server.
    ConnectToServerL(aGetRootFolder);
    }
    
    
// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ShowServerError
//
// Show a server error.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::ShowServerError(TInt aStatus, CTransaction::TTransactionType aRequestType/*=ENone*/)
    {
    TInt labelId;

    // Determine which label to use.
    switch(aStatus)
        {
        case KErrNoMemory:
            labelId = R_FEEDS_OUT_OF_MEMORY;
            break;
		// Few errors wont be shown as pop up dialog as they will appear in list box main pane
        case KErrCorrupt:
            // A pop up error note should be displayed if import is requested,
            // else error will be displayed in main pane itself
            if (aRequestType == CTransaction::EImportOPML) 
                {
                labelId = R_FEEDS_MALFORMED_FEED_ERROR;
                break;	
                }
		case KErrNotSupported:
		case KErrTimedOut:
	   	case  KErrBadName:
        	return;
			
        case -KErrNotSupported:
            // Show the special http not supported on WINSCW error
            TRAP_IGNORE(TBrowserDialogs::InfoNoteL(R_BROWSER_INFO_NOTE, R_FEEDS_HTTP_UNSUPPORTED_WINSCW));
            return;

		case KErrAlreadyExists:
            labelId = R_FEEDS_NAME_ALREADY_IN_USE;
			break;			

        	
        case KErrArgument:
            // A pop up error note should be displayed if import is requested,
            // else error will be displayed in main pane itself
            if (aRequestType == CTransaction::EImportOPML)
                {
               	labelId = R_FEEDS_MALFORMED_FEED_ERROR;
                break;
                }
        default:
			if (aStatus > 0) // All network errors
                {
                // Error will shown in listbox main pane
                return;
                }

            // Otherwise.
            else
                {
                labelId = R_FEEDS_GENERAL_ERROR;
                }
            break;
        }

    // Show the error dialog.
	if (aStatus != KErrCancel)
		{
		TRAP_IGNORE(TBrowserDialogs::InfoNoteL(R_BROWSER_INFO_NOTE, labelId));
		}
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ConnectToServerL
//
// Connect to the server.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::ConnectToServerL(TBool aGetRootFolder)
    {
    if (!iIsConnected)
        {
        User::LeaveIfError(iFeedsInterface.Connect());
        iIsConnected = ETrue;
        }        
    
    if (aGetRootFolder)
        {
        // Set this to no-view so the folder view isn't shown after it's fetched.
        iNextViewId = KUidBrowserNullViewId;
        
        FetchRootFolderL();
        }
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::DisconnectFromServer
//
// Disconnect from the server.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::DisconnectFromServer()
    {
    if( !iIsConnected )
        {
        //Nothing connected, return, this is causing crash.
        return; 
        }
    iFeedsInterface.Close();

    iIsConnected = EFalse;
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::FeedUpdatedTime
//
// Since the folder list isn't updated after it is fetched the FeedsClientUtilities 
// tracks the last update times for feeds the user visits in the current session.
// This method returns the "last updated" timestamp of the given item.
// -----------------------------------------------------------------------------
//
TTime CFeedsClientUtilities::FeedUpdatedTime(const CFeedsEntity& aItem)
    {
    TTime  timestamp;
    TInt   pos;
    
    
    // Search the cache for the feed's url.
    if ((pos = iFeedUpdateTimeIds.Find(aItem.GetId())) != KErrNotFound)
        {
        timestamp = iFeedUpdateTimeTimestamps[pos];
        }
        
    // Otherwise return the item's timestamp.
    else
        {
        aItem.GetTimeValue(EFeedAttributeTimestamp,timestamp);
        }
        
    // Return the cached value.
    return timestamp;
    }
    
    
// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ResetFeedUpdateTime
//
// Resets the "last updated" cache.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::ResetFeedUpdateTime()
    {
    // Reset the arrays.
    iFeedUpdateTimeIds.Reset();
    iFeedUpdateTimeTimestamps.Reset();
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::UpdateFeedUpdatedTimeL
//
// Update the feed's "last updated" value in the cache.
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::UpdateFeedUpdatedTimeL(const CFeedsEntity& aFeed)
    {    
    TInt  pos;
    TInt  err;
    
    // TODO: Use the feed's id instead of the url.  This would require the
    //       packed folder to store the feed id as well.
    
    // Search the cache for the feed's url and if found then update the timestamp.
    if ((pos = iFeedUpdateTimeIds.Find(aFeed.GetId())) != KErrNotFound)
        {
        	TTime time;
        	aFeed.GetTimeValue(EFeedAttributeTimestamp,time);
        	iFeedUpdateTimeTimestamps[pos] = time;
        }

    // Otherwise add a new slot.  
    else
        {
        User::LeaveIfError(iFeedUpdateTimeIds.Append(aFeed.GetId()));
       	TTime time;
       	aFeed.GetTimeValue(EFeedAttributeTimestamp,time);

        err = iFeedUpdateTimeTimestamps.Append(time);
        
        // Ensure the arrays don't get out of sync if the second append fails.
        if (err != KErrNone)
            {
            iFeedUpdateTimeIds.Remove(iFeedUpdateTimeIds.Count() - 1);
            User::LeaveIfError(err);
            }
        }
    }


// -----------------------------------------------------------------------------
// CFeedsClientUtilities::InitMenuItemL()
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::InitMenuItemL( CEikMenuPane* aMenuPane,
                                             TInt aCommandId,
                                             TInt aResourceId, 
                                             TInt aCascadeId,
                                             TInt aFlags )
    {
    CEikMenuPaneItem::SData item;
    item.iCommandId = aCommandId;
    item.iFlags = aFlags;
    item.iCascadeId = aCascadeId;
    HBufC* buf = StringLoader::LoadLC( aResourceId );
    item.iText.Copy( *buf );
    CleanupStack::PopAndDestroy( buf );	// buf
    buf = NULL;

    aMenuPane->AddMenuItemL( item );    
    }
    
// -----------------------------------------------------------------------------
// CFeedsClientUtilities::SendOPMLFileL()
// -----------------------------------------------------------------------------
//    
void CFeedsClientUtilities::SendOPMLFileL( )
    {
    RFs                 rfs;
    
    _LIT(KPath, "C:\\system\\temp\\");
    TBuf<KMaxFileName>  path(KPath);
    
	User::LeaveIfError(rfs.Connect());
	CleanupClosePushL(rfs);

	path.Append(*iExportFileName);
	
	MBmOTABinSender& sender = iApiProvider.BmOTABinSenderL();

	sender.ResetAndDestroy();
    sender.SendOPMLFileL(path);
    
    CleanupStack::PopAndDestroy(/*rfs*/);
    }
    
// -----------------------------------------------------------------------------
// CFeedsClientUtilities::HandleCommandL()
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::HandleCommandL( TUid aViewId, TInt aCommand )
    {
     switch (aCommand)
        {           
        case EWmlCmdBackToPage:
            {
            iApiProvider.SetViewToReturnOnClose( aViewId );
            iApiProvider.SetViewToBeActivatedIfNeededL( KUidBrowserContentViewId );
            break;
            }

        case EFeedsImport:
        	ImportFeedsL();
        	break;
        
        default:
		    // pass common commands to app ui
		    iAppUI.HandleCommandL( aCommand );
        }
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::DynInitMenuPaneL()
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::DynInitMenuPaneL(
        TInt aResourceId,
        CEikMenuPane* aMenuPane )
    {
    
    // Option menu items common to all three feeds views
    if ((aResourceId == R_FEEDS_FOLDER_VIEW_MENU) ||
        (aResourceId == R_FEEDS_FEED_VIEW_MENU) ||
        (aResourceId == R_FEEDS_TOPIC_VIEW_MENU))
        {      
        // browser prefs
        InitMenuItemL( aMenuPane, EWmlCmdPreferences, R_WMLBROWSER_SETTINGS_TITLE );
        
        // Help
	InitMenuItemL( aMenuPane, EAknCmdHelp, R_BROWSER_MENU_ITEM_HELP, R_FEEDS_HELP_SUBMENU );

        // exit
        InitMenuItemL( aMenuPane, EWmlCmdUserExit, R_BROWSER_MENU_ITEM_EXIT );
        }
   }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::SetCalledFromView()
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::SetCalledFromView(TUid aViewId)
    {
    iCalledFromView = aViewId;
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::CalledFromView()
// -----------------------------------------------------------------------------
//
TUid CFeedsClientUtilities::CalledFromView()
    {
    return iCalledFromView;
    }

// -----------------------------------------------------------------------------
// CFeedsClientUtilities::ItemStatusL
//
// Returns the INITIAL status of each of the items in the feed.
// The caller can then modify the values and call UpdateFeedItemStatusL
// to request the feeds server to update the feed's item status. 
// -----------------------------------------------------------------------------
//
void CFeedsClientUtilities::ItemStatusL(RArray<TInt>& aItemIds, 
                RArray<TFeedItemStatus>& aItemStatus, const CFeedsEntity& aFeed) const
    {
    TInt status;

    aItemIds.Reset();
    aItemStatus.Reset();

    for(TInt index=0 ; index< aFeed.GetChildren().Count() ; index++)
        {
        CFeedsEntity *feedEntity = aFeed.GetChildren()[index];

        feedEntity->GetIntegerValue(EItemAttributeStatus,status);

        User::LeaveIfError(aItemIds.Append(feedEntity->GetId()));
        User::LeaveIfError(aItemStatus.Append((TFeedItemStatus)status));
        }
    }
