/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Handle special load events such as network connection,
*                deal with non-http or non-html requests
*
*/



// INCLUDE FILES
#include "BrowserLoadObserver.h"
#include "ApiProvider.h"
#include "Display.h"
#include "BrowserContentView.h"
#include "CommonConstants.h"
#include "BrowserAppUi.h"
#include "logger.h"
#include "BrowserWindow.h"
#include "BrowserWindowManager.h"
#include "BrowserDialogsProviderProxy.h"

#include <FeatMgr.h>
#include <MConnection.h>
#include <BrowserDialogsProvider.h>

#ifdef I__LOG_EVENT_TIME
    // defines a local timer with name 'localTime'
    #define START_TIMER( localTime )  TTime localTime;  localTime.HomeTime();
    
    // count the elapsed time based on timer 'localTime'
    // and increment number of timers called ( numOfTimer )
    #define STOP_TIMER( localTime, numOfTimer ) \
        TTime locTime__a;  locTime__a.HomeTime(); \
        TInt64 updateTime = locTime__a.MicroSecondsFrom( localTime ).Int64(); \
        LOG_WRITE_FORMAT( "Update time: %d", updateTime ); \
        ++numOfTimer; \
        CBrowserLoadObserver::iTotalUpdateTime += updateTime;
#else  // I__LOG_EVENT_TIME
    // empty macros
    #define START_TIMER( a )
    #define STOP_TIMER( a, b )
#endif  // I__LOG_EVENT_TIME

// ---------------------------------------------------------
// CBrowserLoadObserver::NewL()
// ---------------------------------------------------------
//
CBrowserLoadObserver* CBrowserLoadObserver::NewL(
        MApiProvider& aApiProvider,
        CBrowserContentView& aContentView,
        CBrowserWindow& aWindow )
    {
    CBrowserLoadObserver* self = 
        new (ELeave) CBrowserLoadObserver( aApiProvider, aContentView, aWindow );
    CleanupStack::PushL( self );
    self->ConstructL( );
    CleanupStack::Pop();    // self
    return self;
    }

// ---------------------------------------------------------
// CBrowserLoadObserver::~CBrowserLoadObserver()
// ---------------------------------------------------------
//
CBrowserLoadObserver::~CBrowserLoadObserver()
    {
    	// iApiProvider, iContentView, iWindow not owned by CBRowserLoadObserver.
		// invalidate pointer for a cleaner/clearer destruction
        iApiProvider = NULL;
        iContentView = NULL;
		iWindow = NULL;
    }

// ----------------------------------------------------------------------------
// CBrowserLoadObserver::HandleBrowserLoadEventL()
// ----------------------------------------------------------------------------
//
void CBrowserLoadObserver::HandleBrowserLoadEventL(
        TBrCtlDefs::TBrCtlLoadEvent aLoadEvent,
        TUint aSize,
        TUint16 aTransactionId)
    {
LOG_ENTERFN("CBrowserLoadObserver::HandleBrowserLoadEventL");
LOG_WRITE_FORMAT(" LoadEvent: %d", aLoadEvent );
LOG_WRITE_FORMAT("      Size: %d", aSize );
LOG_WRITE_FORMAT("      TrId: %d", aTransactionId );
LOG_WRITE_FORMAT(" LoadState: %d", iLoadState );
LOG_WRITE_FORMAT("  LoadType: %d", iLoadUrlType );
	if( aLoadEvent == TBrCtlDefs::EEventNewContentDisplayed )
		{
		iWindow->ResetPageOverviewLocalSettingL();
		iWindow->SetImagesLoaded(EFalse);		
        } 
    switch( iLoadUrlType )
        {
        default:
            {
            HandleLoadEventOtherL( aLoadEvent, aSize, aTransactionId );
            break;
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserLoadObserver::HandleLoadEventOtherL()
// ----------------------------------------------------------------------------
//
void CBrowserLoadObserver::HandleLoadEventOtherL(
        TBrCtlDefs::TBrCtlLoadEvent aLoadEvent,
        TUint aSize,
        TUint16 aTransactionId )
    {
#define STATECHECK( a ) { if( (iLoadState & a) == 0 ) break; }
    
    TInt err( KErrNone );
    switch( aLoadEvent )
        {
        case TBrCtlDefs::EEventLoadError:
            {
            SetStatus( ELoadStatusMainError );
            break;
            }
        case TBrCtlDefs::EEventEnteringSecurePage:
            {
            SetStatus( ELoadStatusSecurePage );
            SetStatus( ELoadStatusAllItemIsSecure );
            UpdateSecureIndicatorL();
            break;
            }
        case TBrCtlDefs::EEventSomeItemsNotSecure:
            {
            ClearStatus( ELoadStatusAllItemIsSecure );
            UpdateSecureIndicatorL();
            break;
            }
        case TBrCtlDefs::EEventSecureItemInNonSecurePage:
            {
            SetStatus( ELoadStatusSecureItemNonSecurePage );
            UpdateSecureIndicatorL();
            break;
            }
        case TBrCtlDefs::EEventExitingSecurePage:
		case TBrCtlDefs::EEventSubmittingToNonSecurePage:
            {
            ClearStatus( ELoadStatusSecurePage );
            ClearStatus( ELoadStatusAllItemIsSecure );
            SetStatus ( ELoadStatusSecurePageVisited );
            UpdateSecureIndicatorL();
            break;
            }
        case TBrCtlDefs::EEventTitleAvailable:
            {
            SetStatus( ELoadStatusTitleAvailable );
            NewTitleAvailableL();
            break;
            }
        case TBrCtlDefs::EEventNewContentStart:
            {
#ifdef I__LOG_EVENT_TIME
            iStartDownloadTime.HomeTime();
            iTotalUpdateTime = 0;
            iNumberOfUpdates = 0;
#endif  // I__LOG_EVENT_TIME
            StateChange( ELoadStateResponseInProgress );
            iApiProvider->SetProgressShown( ETrue );
            iApiProvider->Display().StartProgressAnimationL();
            
            // in case we're in bookmarks view and a background page load is in
            // progress, don't update the softkeys
            if( iApiProvider->IsForeGround() && InBrowserContentView() )
                {
                if( CBrowserAppUi::Static()->ActiveView() )
                    {
                    CBrowserAppUi::Static()->ActiveView()->UpdateCbaL();
                    }
                }
            iApiProvider->WindowMgr().NotifyObserversL( EWindowLoadStart, iWindow->WindowId() );
            break;
            }
        case TBrCtlDefs::EEventUrlLoadingStart:
            {
            STATECHECK( ELoadStateResponseInProgress )
            iApiProvider->Display().StartProgressAnimationL();

            // If the load is not initiated from the bookmarks view (ie. engine initiated
            // via some timer on a page like cnn.com) then don't change view to content view
            if (iBrowserInitLoad)
                {
                iApiProvider->SetViewToBeActivatedIfNeededL(KUidBrowserContentViewId);
                iBrowserInitLoad = EFalse;
                }

            // add transaction to ProgressBar
            iApiProvider->Display().AddTransActIdL( aTransactionId );    
            
            // Display the status pane, while loading
            if ( InBrowserContentView() && iContentView->FullScreenMode() )
                {
                iContentView->ShowFsStatusPane(ETrue);
                }
            break;
            }
        // first content chunk arrived
        case TBrCtlDefs::EEventNewUrlContentArrived:
            {
            STATECHECK( ELoadStateResponseInProgress )
            SetStatus( ELoadStatusFirstChunkArrived );
            // set MaxData for this transaction
            iApiProvider->Display().AddProgressDataL(
                aTransactionId, 0, aSize );
            break;
            }
        // additional content chunk arrived
        case TBrCtlDefs::EEventMoreUrlContentArrived:
            {
            STATECHECK( ELoadStateResponseInProgress )
            START_TIMER( t1 );
            // set RecvdData for this transaction
            iApiProvider->Display().AddProgressDataL(
                aTransactionId, aSize, 0 );
            if( iApiProvider->IsForeGround() )
                {
                iApiProvider->Display().NotifyProgress();
                }
            STOP_TIMER( t1, iNumberOfUpdates );
            break;
            }
        // content is processed, new fetch is allowed.
        // some more event may still remain
        case TBrCtlDefs::EEventContentFinished:
            {
            StateChange( ELoadStateIdle );
            if( !ContentDisplayed() )
                {
                SetContentDisplayed( ETrue );
                }

            if( !iApiProvider->ExitInProgress() && 
                iApiProvider->IsForeGround() )
                {
                iApiProvider->Display().StopProgressAnimationL();
                }
            User::ResetInactivityTime();
            
            if( LoadUrlType() == ELoadUrlTypeEmbeddedBrowserWithUrl &&
                !LoadStatus( ELoadStatusFirstChunkArrived ) && 
                !iApiProvider->ExitInProgress() )
                {
                // Go back to bookmarks and close the content view
                iApiProvider->SetViewToReturnOnClose( 
                                                KUidBrowserBookmarksViewId );
                iApiProvider->CloseContentViewL();
                }
            // No content to be shown, go back to where we came from
            else if ( !iRestoreContentFlag  )
                {
                CBrowserAppUi::Static()->ActivateLocalViewL( 
                                            iApiProvider->LastActiveViewId() );
                if( iApiProvider->IsForeGround() )
                    {
                    if  ( CBrowserAppUi::Static()->ActiveView() )
                        {
                        CBrowserAppUi::Static()->ActiveView()->UpdateCbaL();
                        }
                    }
                }                        
            else
                {
                ContentArrivedL();
                
                // in case we're in bookmarks view and a background page load is in
                // progress, don't update the softkeys
                if( iApiProvider->IsForeGround() && InBrowserContentView() )
                    {
                    if  ( CBrowserAppUi::Static()->ActiveView() )
                        {
                        CBrowserAppUi::Static()->ActiveView()->UpdateCbaL();
                        }
                    }
                }
#ifdef I__LOG_EVENT_TIME
            TTime endDownloadTime;
            endDownloadTime.HomeTime();
            TInt64 dlTime = endDownloadTime.MicroSecondsFrom(iStartDownloadTime).Int64();
            LOG_WRITE_FORMAT( "Total download time: %d", dlTime );
            LOG_WRITE_FORMAT( "Total update time: %d", iTotalUpdateTime );
            LOG_WRITE_FORMAT( "Total number of updates: %d", iNumberOfUpdates );
            if( iNumberOfUpdates )
                {
                LOG_WRITE_FORMAT( "Average update time: %d", 
                    iTotalUpdateTime / iNumberOfUpdates );
                }
            if( dlTime )
                {
                LOG_WRITE_FORMAT( "Total update time (%% of download time): %d",
                    iTotalUpdateTime / (dlTime / 100) );
                }
#endif  // I__LOG_EVENT_TIME
            break;
            }
        // first chunk displayed, no parameter
        case TBrCtlDefs::EEventNewContentDisplayed:
            {
            iApiProvider->WindowMgr().SetContentExist( ETrue );
            SetStatus( ELoadStatusFirstChunkDisplayed );
            SetRestoreContentFlag( ETrue );
            ContentArrivedL();
            if( !ContentDisplayed() )
                {
                SetContentDisplayed( ETrue );
                }
            break;
            }
        // additional chunk displayed, no parameter
        case TBrCtlDefs::EEventMoreContentDisplayed:
            {
            SetStatus( ELoadStatusContentDisplayed );
            SetRestoreContentFlag( ETrue );
            ContentArrivedL();
            break;
            }
        // last chunk arrived
        case TBrCtlDefs::EEventUrlLoadingFinished:
            {
            iApiProvider->Display().TransActIdComplete( aTransactionId );
			SetRestoreContentFlag( ETrue );
            ContentArrivedL();
			TRAP( err, iApiProvider->LogAccessToRecentUrlL( iWindow->BrCtlInterface() ) );            
            break;
            }
        case TBrCtlDefs::EEventLoadFinished:
            {
            if( !iApiProvider->ExitInProgress() &&
                iApiProvider->IsForeGround() )
                {
                iApiProvider->Display().StopProgressAnimationL();

                // Turn off status pane, SK, and Cba 
                // If in content view, set to fullscreen after download complete
                if ( InBrowserContentView() && iContentView->FullScreenMode() )
                    {
                    iContentView->ShowFsStatusPane(EFalse);
                    }
                }
            iApiProvider->WindowMgr().NotifyObserversL( EWindowLoadStop, iWindow->WindowId() );
            break;
            }
        case TBrCtlDefs::EEventAuthenticationFailed:
            {
            // don't add url to Adaptive Bookmarks
            ClearStatus( ELoadStatusFirstChunkArrived );
            break;
            }
        // Large file upload events
		case TBrCtlDefs::EEventUploadStart:
			{
			iMaxUploadContent = aSize;
            iWindow->DialogsProviderProxy().UploadProgressNoteL( 
                iMaxUploadContent, 0, EFalse, this );
			break;
			}
		case TBrCtlDefs::EEventUploadIncrement:
			{
            iWindow->DialogsProviderProxy().UploadProgressNoteL( 
                iMaxUploadContent, aSize, EFalse, this );
			break;
         	}
		case TBrCtlDefs::EEventUploadFinished:
			{
            iWindow->DialogsProviderProxy().UploadProgressNoteL( 
                iMaxUploadContent, aSize, ETrue, this ); 
			break;         
			}
        default:
            break;
        }
#undef STATECHECK
    }

// ----------------------------------------------------------------------------
// CBrowserLoadObserver::CBrowserLoadObserver()
// ----------------------------------------------------------------------------
//
CBrowserLoadObserver::CBrowserLoadObserver(
        MApiProvider& aApiProvider,
        CBrowserContentView& aContentView,
        CBrowserWindow& aWindow ) :
    iApiProvider( &aApiProvider ),
    iContentView( &aContentView ),
    iWindow( &aWindow ),
    iLoadState( ELoadStateIdle ),
    iLoadUrlType( ELoadUrlTypeOther ),
    iStatus( 0 )
    {
    }

// ----------------------------------------------------------------------------
// CBrowserLoadObserver::ConstructL()
// ----------------------------------------------------------------------------
//
void CBrowserLoadObserver::ConstructL()
    {
    }

// ----------------------------------------------------------------------------
// CBrowserLoadObserver::DoStartLoad()
// ----------------------------------------------------------------------------
//
void CBrowserLoadObserver::DoStartLoad(
        TBrowserLoadUrlType aLoadUrlType )
    {
/*
LOG_WRITE("-------------------")
LOG_WRITE_FORMAT(" UrlType: %d ", aLoadUrlType )
*/
    // __ASSERT_DEBUG instead of condition?
    if( iLoadState == ELoadStateIdle )
        {                
        if (LoadStatus(ELoadStatusSecurePageVisited))
            {
            ClearStatus();  
            SetStatus(ELoadStatusSecurePageVisited);  
            }
        else 
            {
            ClearStatus();            
            }        
        
        iLoadUrlType = aLoadUrlType;
        iRestoreContentFlag = EFalse;
        }
        
    iBrowserInitLoad = ETrue;
    }

// ----------------------------------------------------------------------------
// CBrowserLoadObserver::DoEndLoad()
// ----------------------------------------------------------------------------
//
void CBrowserLoadObserver::DoEndLoad(
        TBool aIsUserInitiated )
    {
// LOG_WRITE( "Cancelling.")
    if( aIsUserInitiated)
        {
        // wait for the remaining load events
        StateChange( ELoadStateLoadDone );
        }
    else  // don't wait for anything
        {
        StateChange( ELoadStateIdle );
        }

    // first arrives ContentFinished and then UrlLoadingFinished!
    // what to do with status?
    // updatesoftkeys()  done in appui

    CBrowserViewBase* view = CBrowserAppUi::Static()->ActiveView();
    if( view ) // just to be sure
        {
        TVwsViewId activeViewId = view->ViewId();
        if( activeViewId.iViewUid == KUidBrowserBookmarksViewId)
            {
            SetRestoreContentFlag( EFalse );
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserLoadObserver::NewTitleAvailableL()
// ----------------------------------------------------------------------------
//
void CBrowserLoadObserver::NewTitleAvailableL()
    {
    if( iWindow->IsWindowActive() )
        {
        CBrowserViewBase* view = CBrowserAppUi::Static()->ActiveView();        
         if( view ) // just to be sure
              {
              TVwsViewId activeViewId = view->ViewId();
              if( activeViewId.iViewUid == KUidBrowserContentViewId )
                  {
                  iContentView->UpdateTitleL( *iApiProvider );
                  }
              }
        }
    SetRestoreContentFlag( ETrue );
    }
    
// ----------------------------------------------------------------------------
// CBrowserLoadObserver::InBrowserContentView()
// ----------------------------------------------------------------------------
//
TBool CBrowserLoadObserver::InBrowserContentView()
    {
    CBrowserViewBase* view = CBrowserAppUi::Static()->ActiveView();        
    if( view ) // just to be sure
        {
        TVwsViewId activeViewId = view->ViewId();
        return ( activeViewId.iViewUid == KUidBrowserContentViewId );
        }
        
    return EFalse;
    }


// ----------------------------------------------------------------------------
// CBrowserLoadObserver::ContentArrivedL()
// ----------------------------------------------------------------------------
//
void CBrowserLoadObserver::ContentArrivedL()
    {
    if( iApiProvider->Connection().Connected()
        && iApiProvider->Preferences().HttpSecurityWarningsStatSupressed() )
        {
        TInt secureUpdate = EAknIndicatorStateOff;
        if( LoadStatus( ELoadStatusSecurePage ) )
            {
            if( LoadStatus( ELoadStatusAllItemIsSecure ) )
                {
                secureUpdate = EAknIndicatorStateOn;
                }
            }
        iApiProvider->Display().UpdateSecureIndicatorL( secureUpdate );
        }
    }

//-----------------------------------------------------------------------------
// CBrowserLoadObserver::ReportDialogEvent
//-----------------------------------------------------------------------------
// Handles dialog provider events
void CBrowserLoadObserver::ReportDialogEventL(
        TInt aType,
        TInt aFlags )
    {
    switch( aType )
        {
        case MBrowserDialogsProviderObserver::ENotifyError:
            // aFlags contains error code
            {
            // If card not in deck error, go to first card of deck
            SetRestoreContentFlag( aFlags == KBrsrWmlbrowserCardNotInDeck );
            break;
            }
        case MBrowserDialogsProviderObserver::ENotifyHttpError:
            // aFlags contains error code
            {
            SetRestoreContentFlag( EFalse );
            break;
            }
        case MBrowserDialogsProviderObserver::EUserAuthentication:
            {
            SetRestoreContentFlag( aFlags ); // False == Cancelled
            break;
            }
        case MBrowserDialogsProviderObserver::EConfirm:
            // aFlags contains Cancel status
            {
            // if confirmation query was cancelled, step back to idle
            if( !aFlags )
                {
                DoEndLoad( EFalse );
                }
            SetRestoreContentFlag( !aFlags );
            break;
            }
        case MBrowserDialogsProviderObserver::EUploadProgress:
            {
            // Cancel fetching - dialog is cancelled
            if ( aFlags == KErrCancel )
                {
                iWindow->BrCtlInterface().HandleCommandL( 
                    (TInt)TBrCtlDefs::ECommandCancelFetch +
                    (TInt)TBrCtlDefs::ECommandIdBase );
                }
            break;
            }
        default:
            break;
        }
    }

//-----------------------------------------------------------------------------
// CBrowserLoadObserver::UpdateSecureIndicatorL
//-----------------------------------------------------------------------------
//
void CBrowserLoadObserver::UpdateSecureIndicatorL()
    {
    TBool status = LoadStatus( ELoadStatusAllItemIsSecure );
    iApiProvider->Display().UpdateSecureIndicatorL( 
        status &&
        !iApiProvider->Preferences().HttpSecurityWarningsStatSupressed() );
    }

//-----------------------------------------------------------------------------
// CBrowserLoadObserver::LoadUrlType
//-----------------------------------------------------------------------------
//
CBrowserLoadObserver::TBrowserLoadUrlType CBrowserLoadObserver::LoadUrlType() const
    {
    return iLoadUrlType;
    }

//-----------------------------------------------------------------------------
// CBrowserLoadObserver::LoadState
//-----------------------------------------------------------------------------
//
CBrowserLoadObserver::TBrowserLoadState CBrowserLoadObserver::LoadState() const
    {
    return iLoadState;
    }
//-----------------------------------------------------------------------------
// CBrowserLoadObserver::StateChange
//-----------------------------------------------------------------------------
//
void CBrowserLoadObserver::StateChange( TBrowserLoadState aNextState )
    {
    if( ELoadStateIdle == iLoadState &&
        iLoadState != aNextState )
       {
          iNewContentDisplayed = EFalse;
          iApiProvider->WindowMgr().NotifyObserversL( EWindowCntDisplayed, iWindow->WindowId() );
       }
    iLoadState = aNextState;
    }

//-----------------------------------------------------------------------------
// CBrowserLoadObserver::SetContentDisplayed
//-----------------------------------------------------------------------------
//

void CBrowserLoadObserver::SetContentDisplayed( TBool aValue )
    { 
    iNewContentDisplayed = aValue;
    TRAP_IGNORE( iApiProvider->WindowMgr().NotifyObserversL( EWindowCntDisplayed, iWindow->WindowId()));
    }


// End of file
