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
*     Multiple Windows' WindowManager.
*
*
*/


// INCLUDE FILES
#include "BrowserWindowManager.h"
#include "BrowserWindow.h"
#include "BrowserWindowQueue.h"
#include "BrowserContentView.h"
#include "WindowInfoProvider.h"
#include "Logger.h"
#include "Preferences.h"
#include "Display.h"
#include "ApiProvider.h"
#include <brctldefs.h>
#include "BrowserUtil.h"
#include <BrowserNG.rsg>
#include <StringLoader.h>
#include <AknQueryDialog.h>


// -----------------------------------------------------------------------------
// CBrowserWindowQue::LastItem()
// -----------------------------------------------------------------------------
//
CBrowserWindowQue* CBrowserWindowQue::LastItem() const
    {
    // remove 'const' modifier from 'this' in a const member function
    CBrowserWindowQue* a = CONST_CAST( CBrowserWindowQue*, this );
    for( ; a->iNext; a=a->iNext)
        ;
    return a;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowQue::~CBrowserWindowQue()
// -----------------------------------------------------------------------------
//
CBrowserWindowQue::~CBrowserWindowQue()
    {
LOG_ENTERFN("WindowQue::~WindowQue");
    if( this->iPrev )
        {
		BROWSER_LOG( ( _L( "iPrev" ) ) );
        this->iPrev->iNext = this->iNext;
        }
    if( this->iNext )
        {
		BROWSER_LOG( ( _L( "iNext" ) ) );
        this->iNext->iPrev = this->iPrev;
        }
    delete iWindow;
    iWindow = NULL;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowQue::AppendL()
// -----------------------------------------------------------------------------
//
void CBrowserWindowQue::AppendL( CBrowserWindowQue* aItem )
    {
	__ASSERT_DEBUG( (aItem != NULL), Util::Panic( Util::EUninitializedData ));
    aItem->iPrev = LastItem();
    aItem->iPrev->iNext = aItem;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowQue::Next()
// -----------------------------------------------------------------------------
//
CBrowserWindowQue* CBrowserWindowQue::Next() const
    {
    CBrowserWindowQue* b = this->iNext;
    while( b && (b->iStatus & CBrowserWindowQue::EWindowDeleted ) )
        {
        b = b->iNext;
        }
    return b;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowQue::Prev()
// -----------------------------------------------------------------------------
//
CBrowserWindowQue* CBrowserWindowQue::Prev() const
    {
    CBrowserWindowQue* b = this->iPrev;
    while( b && (b->iStatus & CBrowserWindowQue::EWindowDeleted ) )
        {
        b = b->iPrev;
        }
    return b;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::
// -----------------------------------------------------------------------------
// initialization of static member variable
TInt CBrowserWindowManager::iWindowIdGenerator( 0 );

// -----------------------------------------------------------------------------
// CBrowserWindowManager::NewLC()
// -----------------------------------------------------------------------------
//
CBrowserWindowManager* CBrowserWindowManager::NewLC(
        MApiProvider& aApiProvider,
        CBrowserContentView& aContentView,
        TInt aMaxWindowCount )
    {
    CBrowserWindowManager* self = new (ELeave)
        CBrowserWindowManager( aApiProvider, aContentView, aMaxWindowCount );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::NewL()
// -----------------------------------------------------------------------------
//
CBrowserWindowManager* CBrowserWindowManager::NewL(
        MApiProvider& aApiProvider,
        CBrowserContentView& aContentView,
        TInt aMaxWindowCount )
    {
    CBrowserWindowManager* self = CBrowserWindowManager::NewLC(
        aApiProvider, aContentView, aMaxWindowCount );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::~CBrowserWindowManager()
// -----------------------------------------------------------------------------
//
CBrowserWindowManager::~CBrowserWindowManager( )
    {
LOG_ENTERFN("WindowManager::~WindowManager");
    // Preferences must exist!
    if(iApiProvider!=NULL)
    	{
    	iApiProvider->Preferences().RemoveObserver( this );
    	}
    RemoveObserver(iContentView);
    CBrowserWindowQue *window = iWindowQueue;
    CBrowserWindowQue *temp = NULL;


    TBool isStandAlone( !iApiProvider->IsEmbeddedModeOn() );
    if ( ( isStandAlone && !iUserExit ) || ( !isStandAlone ) )
        {
        TRAP_IGNORE( window->iWindow->BrCtlInterface().HandleCommandL(
            (TInt)TBrCtlDefs::ECommandSaveLaunchParams + (TInt)TBrCtlDefs::ECommandIdBase ) );
        }

    for(; window; )
        {
        temp = window;
        window = window->iNext;
        DeleteOneWindowL( temp->iWindow->WindowId() );
        /*delete temp; 
        temp = NULL;*/
        }
        RemoveDeletedWindowsL();
    delete iObservers;
    iObservers = NULL;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::WindowCount()
// -----------------------------------------------------------------------------
//
TInt CBrowserWindowManager::WindowCount() const
    {
    return iWindowCount - iDeletedWindowCount;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::CBrowserWindowManager()
// -----------------------------------------------------------------------------
//
CBrowserWindowManager::CBrowserWindowManager(
        MApiProvider& aApiProvider,
        CBrowserContentView& aContentView,
        TInt aMaxWindowCount ) :
    iApiProvider( &aApiProvider ),
    iContentView( &aContentView),
    iMaxWindowCount( aMaxWindowCount ),
    iWindowCount( 0 ),
    iDeletedWindowCount( 0 ),
    iWindowQueue( NULL ),
    iCurrentWindow( NULL ),
    iUserExit( EFalse ),
    iIsContentExist( EFalse )
    {
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::ConstructL()
// -----------------------------------------------------------------------------
//
void CBrowserWindowManager::ConstructL( )
    {
    iApiProvider->Preferences().AddObserverL( this );
    CreateWindowL( 0, &KNullDesC );
    iCurrentWindow = iWindowQueue;
    // don't draw anything, just activate it
    iCurrentWindow->iWindow->ActivateL( EFalse );
    iObservers = new ( ELeave ) CArrayPtrFlat< MWindowObserver >( 1 );
    AddObserverL(iContentView);
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::Window()
// -----------------------------------------------------------------------------
//
TInt CBrowserWindowManager::Window(
        TInt aWindowId,
        CBrowserWindow*& aWindow )
    {
    RemoveDeletedWindowsL();
    CBrowserWindowQue *windowQue = NULL;
    TInt error( Window( aWindowId, windowQue, iWindowQueue ) );
    if( windowQue )
        {
        aWindow = windowQue->iWindow;
        }
    return error;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::FindWindowL()
// -----------------------------------------------------------------------------
//
CBrowserWindow* CBrowserWindowManager::FindWindowL(
        const TDesC& aTargetName )
    {
LOG_ENTERFN("WindowManager::FindWindowL( Target )");
    RemoveDeletedWindowsL();
    CBrowserWindow* result = NULL;
    CBrowserWindow* window = NULL;
    CBrowserWindowQue *windowQue = iWindowQueue;
    for( ; windowQue; windowQue = windowQue->Next() )
        {
        window = windowQue->iWindow;
        if( window->TargetName()->Compare( aTargetName ) == 0 )
            {
            result = window;
            }
        }
    return result;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::CurrentWindow()
// -----------------------------------------------------------------------------
//
CBrowserWindow* CBrowserWindowManager::CurrentWindow() const
    {
    if( iCurrentWindow )
        {
        return iCurrentWindow->iWindow;
        }
    else
        {
        return NULL;
        }
    }
	
// -----------------------------------------------------------------------------
// CBrowserWindowManager::CurrentWindowQue()
// -----------------------------------------------------------------------------
//
CBrowserWindowQue* CBrowserWindowManager::CurrentWindowQue() const
    {
    if( iCurrentWindow )
        {
        return iCurrentWindow;
        }
    else
        {
        return NULL;
        }
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::GetWindowInfoL()
// -----------------------------------------------------------------------------
//
CArrayFixFlat<CWindowInfo*>* CBrowserWindowManager::GetWindowInfoL(
        MWindowInfoProvider* aWindowInfo )
    {
	__ASSERT_DEBUG( (aWindowInfo != NULL), Util::Panic( Util::EUninitializedData ));

    RemoveDeletedWindowsL();
    CBrowserWindowQue* window = iWindowQueue;
    CArrayFixFlat<CWindowInfo*>* windowInfoList = new (ELeave)
        CArrayFixFlat<CWindowInfo*>( 3 /* KGranularityMedium */ );
    CleanupStack::PushL( windowInfoList );
    for( ; window; window = window->Next() )
        {
        // create window info text by client
        HBufC* buf = aWindowInfo->CreateWindowInfoLC( *( window->iWindow ) );

        if (!buf)
        {
	  		CleanupStack::PopAndDestroy( buf );
     		buf = KNullDesC().AllocLC();
        }

        // create window info( text, id, current )
        CWindowInfo* windowInfo = new ( ELeave ) CWindowInfo(
          	buf, window->iWindow->WindowId(), iCurrentWindow == window );

        CleanupStack::Pop( buf );

        // append to list
        windowInfoList->AppendL( windowInfo );

        }

    CleanupStack::Pop( windowInfoList );
    return windowInfoList;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::AddObserver()
// -----------------------------------------------------------------------------
//
void CBrowserWindowManager::AddObserverL( MWindowObserver* aObserver )
    {
    LOG_ENTERFN("CBrowserWindowManager::AddObserver");
    if ( iObservers )
        {
        iObservers->AppendL( aObserver );
        }
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::RemoveObserver()
// -----------------------------------------------------------------------------
//
void CBrowserWindowManager::RemoveObserver( MWindowObserver* aObserver )
    {
    LOG_ENTERFN("CBrowserWindowManager::RemoveObserver");
    TInt i( 0 );
    TInt count = iObservers->Count();
    for ( i = 0; i < count; i++ )
        {
        if ( iObservers->At( i ) == aObserver )
            {
            iObservers->Delete( i );
            break;
            }
        }
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::NotifyObservers()
// -----------------------------------------------------------------------------
//
void CBrowserWindowManager::NotifyObserversL( TWindowEvent aEvent, TInt aWindowId )
    {
    LOG_ENTERFN("CBrowserWindowManager::NotifyObservers");
    if ( iObservers )
        {
        TInt i;
        TInt count = iObservers->Count();

        if ( count )
            {
            for ( i = 0; i < count; i++ )
                {
                iObservers->At( i )->WindowEventHandlerL( aEvent, aWindowId );
                }
            }
        }
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::CreateWindowL()
// -----------------------------------------------------------------------------
//
CBrowserWindow* CBrowserWindowManager::CreateWindowL(
        TInt aParentId,
        const TDesC* aTargetName )
    {
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
LOG_ENTERFN("WindowManager::CreateWindowL");
    RemoveDeletedWindowsL();
    if( WindowCount() >= iMaxWindowCount )
        {
        HBufC* maxWinsReached = StringLoader::LoadLC(R_BROWSER_NOTE_MAX_WINDOWS);
	  	ApiProvider()->DialogsProvider().DialogNoteL( *maxWinsReached );
	  	CleanupStack::PopAndDestroy( maxWinsReached );
        return NULL;
        }
    CBrowserWindow* window = NULL;
    // search for already existing window
    Window( aParentId, aTargetName, window, iWindowQueue );  // error is ignored
    if( window )
        {

        return window;
        }
    window = CBrowserWindow::NewLC( ++iWindowIdGenerator, aTargetName, this );
BROWSER_LOG( ( _L( "Window is created, ID: %d, windowCount: %d" ),
    window->WindowId(), WindowCount ) );
    // create a WindowQue instance initiated by the new window
    CBrowserWindowQue *windowQue = new (ELeave) CBrowserWindowQue( window );
    CleanupStack::PushL( windowQue );
    // and set window's parent
    TInt error2( Window( aParentId, windowQue->iParent, iWindowQueue ) );
BROWSER_LOG( ( _L( "error2: %d" ), error2 ) );
    // append window to the list
    if( iWindowQueue )
        {
        BROWSER_LOG( ( _L( "window queue not empty" ) ) );
        iWindowQueue->AppendL( windowQue );
        }
    else
        {
        BROWSER_LOG( ( _L( "window queue empty" ) ) );
        iWindowQueue = windowQue;
        }
    CleanupStack::Pop( windowQue );

    // window is created and append to the list successfully
    ++iWindowCount;

    CleanupStack::Pop( window );
    NotifyObserversL( EWindowOpen, window->WindowId() );

    // window is activated by the client
PERFLOG_STOP_WRITE("****CreateWindowL***")
    return window;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::DeleteWindowL()
// -----------------------------------------------------------------------------
//
TInt CBrowserWindowManager::DeleteWindowL(
        TInt aWindowId,
        TBool aUserInitiated )
    {
LOG_ENTERFN("WindowManager::DeleteWindowL");
    TInt windowToBeActivated( KErrNotFound );
    // find the window
    CBrowserWindowQue* windowQue = NULL;
    Window( aWindowId, windowQue, iWindowQueue );
    if( windowQue )
        {
        // make the old window absolutely inactive
        windowQue->iWindow->BrCtlInterface().HandleCommandL(
	        (TInt)TBrCtlDefs::ECommandClearHistory + (TInt)TBrCtlDefs::ECommandIdBase );
		// Changes were made to ClearHistory that cause it to keep one (current) page
		// In order to reuse a window we need to clear the history again after the first new page
		// is loaded.
		windowQue->iWindow->SetFirstPage(ETrue);


        windowQue->iWindow->BrCtlInterface().HandleCommandL(
            (TInt)TBrCtlDefs::ECommandLoseFocus + (TInt)TBrCtlDefs::ECommandIdBase );

        if(windowQue->iWindow->HasWMLContent(EFalse))
            {
            windowQue->iWindow->BrCtlInterface().HandleCommandL(
                (TInt)TBrCtlDefs::ECommandUnloadWMLEngine + (TInt)TBrCtlDefs::ECommandIdBase );
            }

        // On 3.1 CancelFetch activates the content view for some reason
        // this section is temporarily flagged out until further
        // investigation.
        windowQue->iWindow->BrCtlInterface().HandleCommandL(
           (TInt)TBrCtlDefs::ECommandCancelFetch + (TInt)TBrCtlDefs::ECommandIdBase );

        // if the topmost window is deleted
        // try to find a new one to be activated
        if( iCurrentWindow == windowQue )
            {
            CBrowserWindowQue* next = windowQue->Next();
            CBrowserWindowQue* prev = windowQue->Prev();
            if( next )
                {
                windowToBeActivated = next->iWindow->WindowId();
                iCurrentWindow = next;
                }
            else if( prev )
                {
                windowToBeActivated = prev->iWindow->WindowId();
                iCurrentWindow = prev;
                }
            else
                {
                windowToBeActivated = 0;
                }
            }
        // else a background window was deleted

        // last window handling
        if( WindowCount() == 1 )
            {
            // here we already set the windowToBeActivated to 0
            // means CloseContentView or Exit in AppUi

            // clear all content of Window (images, scripts)

//            windowQue->iWindow->BrCtlInterface().HandleCommandL(
//                (TInt)TBrCtlDefs::ECommand + (TInt)TBrCtlDefs::ECommandIdBase );
            windowQue->iWindow->BrCtlInterface().HandleCommandL(
                (TInt)TBrCtlDefs::ECommandUnloadWMLEngine + (TInt)TBrCtlDefs::ECommandIdBase );

            //Remove the plugins windows. This is a fix for plugins still plays in the background
            //while the page is closed.
            windowQue->iWindow->BrCtlInterface().HandleCommandL(
                 (TInt)TBrCtlDefs::ECommandUnLoadPluginWindows + (TInt)TBrCtlDefs::ECommandIdBase );

            iIsContentExist = EFalse;
            windowQue->iWindow->SetHasWMLContent(EFalse);
            windowQue->iWindow->SetCurrWinHasWMLContent(EFalse);


            }
        // Update MW Indicator
        else if ( WindowCount() == 2 )
            {
            // sets the window status
            windowQue->iStatus |= CBrowserWindowQue::EWindowDeleted;
            windowQue->iWindow->DeactivateL();
            // maintain deleted windows' count
            ++iDeletedWindowCount;

            if( aUserInitiated )
                {
                DeleteOneWindowL( aWindowId );
                }
            }
        else
            {
            // sets the window status
            windowQue->iStatus |= CBrowserWindowQue::EWindowDeleted;
            windowQue->iWindow->DeactivateL();
            // maintain deleted windows' count
            ++iDeletedWindowCount;

            if( aUserInitiated )
                {
                DeleteOneWindowL( aWindowId );
                }
            }
        }
    NotifyObserversL( EWindowClose, aWindowId );

    return windowToBeActivated;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::SwitchWindowL()
// -----------------------------------------------------------------------------
//
TInt CBrowserWindowManager::SwitchWindowL( TInt aWindowId, TBool aDraw )
    {
LOG_ENTERFN("WindowManager::SwitchWindowL");
    CBrowserWindowQue* windowQue = NULL;
    TBool wdnSwitching = EFalse;
    TInt error( Window( aWindowId, windowQue, iWindowQueue ) );
BROWSER_LOG( ( _L( "error: %d" ), error ) );
    if( windowQue )
        {
		BROWSER_LOG( ( _L( "Switching..." ) ) );
        if( windowQue != iCurrentWindow )
            {
            wdnSwitching = ETrue;
            iCurrentWindow->iWindow->DeactivateL();
            iCurrentWindow = windowQue;
            }
        //If the history view is up, don't activate the content view unless it is switching the window. Otherwise,
        //it will overdraw the history view and it will cause serious problems when there
        //is a background redirection
        if(!iCurrentWindow->iWindow->IsWindowActive() && (!iContentView->IsHistoryViewUp() || wdnSwitching))
           {
           iCurrentWindow->iWindow->ActivateL( aDraw );
           iContentView->UpdateCbaL();
           }
        }
    return error;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::Window()
// -----------------------------------------------------------------------------
// search for a window identified by its Id
TInt CBrowserWindowManager::Window(
        TInt aWindowId,
        CBrowserWindowQue*& aWindowQue,
        CBrowserWindowQue* aStartingItem ) const
    {
LOG_ENTERFN("WindowManager::Window( Id )");
    TInt error( KErrNotFound );
    CBrowserWindowQue *windowQue = aStartingItem;
    for( ; (error != KErrNone) && (windowQue); windowQue = windowQue->iNext )
        {
        if( windowQue->iWindow->WindowId() == aWindowId )
            {
            error = KErrNone;
            aWindowQue = windowQue;
            }
        }
    return error;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::Window()
// -----------------------------------------------------------------------------
// search for a window identified by TargetName and its Parent
TInt CBrowserWindowManager::Window(
        TInt aParentId,
        const TDesC* aTargetName,
        CBrowserWindow*& aWindow,
        CBrowserWindowQue* aStartingItem ) const
    {
LOG_ENTERFN("WindowManager::Window( Parent, Target )");
    TInt error( KErrNotFound );
    if( aTargetName && aTargetName->Length() )  // sanity check
        {
        CBrowserWindow* window = NULL;
        CBrowserWindowQue *windowQue = aStartingItem;
        CBrowserWindowQue* parent = NULL;
        for( ; (error != KErrNone) && (windowQue); windowQue = windowQue->iNext )
            {
            window = windowQue->iWindow;
            parent = windowQue->iParent;
            if( parent &&
                ( parent->iWindow->WindowId() == aParentId ) &&
                ( window->TargetName()->Compare( *aTargetName ) == 0 ) )
                {
                error = KErrNone;
                aWindow = window;
                }
            }
        }
    return error;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::DeleteOneWindowL()
// -----------------------------------------------------------------------------
//
TInt CBrowserWindowManager::DeleteOneWindowL( TInt aWindowId )
    {
    // Last window cannot be deleted, this is not called then.
LOG_ENTERFN("WindowManager::DeleteOneWindowL");
    CBrowserWindowQue* windowQue = NULL;
    // find window, error ignored
    Window( aWindowId, windowQue, iWindowQueue );
    if( windowQue )
        {
        // set WindowQueue's first item if required
        if( iWindowQueue == windowQue )
            {
            iWindowQueue = windowQue->iNext;
            }

        // clear children's parent pointer
        CBrowserWindowQue* queue = iWindowQueue;
        for( ; queue; queue = queue->iNext )
            {
            if( queue->iParent == windowQue )
                {
                queue->iParent = NULL;
                }
            }

        // decrease WindowCount
        --iWindowCount;

        // maintain deleted status
        if ( windowQue->iStatus & CBrowserWindowQue::EWindowDeleted )
            {
            __ASSERT_DEBUG( iDeletedWindowCount,
                Util::Panic( Util::EUnExpected ));
            --iDeletedWindowCount;
            }
        // delete window via its windowQue container
        delete windowQue;
        }
    return KErrNone;
    }

// ----------------------------------------------------------------------------
// CBrowserWindowManager::RemoveDeletedWindowsL()
// ----------------------------------------------------------------------------
//
void CBrowserWindowManager::RemoveDeletedWindowsL()
    {
    CBrowserWindowQue *window = iWindowQueue;
    CBrowserWindowQue *temp = NULL;
    // walk through all the windows
    for( ; window; )
        {
        temp = window;
        window = window->iNext;
        // if it is set as deleted
        if( temp->iStatus & CBrowserWindowQue::EWindowDeleted )
            {
            DeleteOneWindowL( temp->iWindow->WindowId() );
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserWindowManager::HandlePreferencesChangeL()
// ----------------------------------------------------------------------------
//
void CBrowserWindowManager::HandlePreferencesChangeL(
        const TPreferencesEvent aEvent,
        TPreferencesValues& aValues,
        TBrCtlDefs::TBrCtlSettings aSettingType )
    {
LOG_ENTERFN("WindowManager::HandlePreferencesChangeL");
BROWSER_LOG( ( _L( "Preferences event: %d" ), aEvent ) );
    switch( aEvent )
        {
        case EPreferencesActivate:
            {
            // Topmost Window ONLY
            if ( iCurrentWindow )
                {
                iCurrentWindow->iWindow->HandlePreferencesChangeL(
                    aEvent, aValues, aSettingType );
                }
            break;
            }
        case EPreferencesDeactivate:
        case EPreferencesItemChange:
            {
            // All windows
            CBrowserWindowQue *windowQue = iWindowQueue;
            for( ; windowQue; windowQue = windowQue->Next() )
                {
                windowQue->iWindow->HandlePreferencesChangeL(
                    aEvent, aValues, aSettingType );
                }
            break;
            }
        default:
            // don't do anything
            break;
        }
    }

// ----------------------------------------------------------------------------
// CBrowserWindowManager::SendCommandToAllWindowsL()
// ----------------------------------------------------------------------------
//
void CBrowserWindowManager::SendCommandToAllWindowsL(
        TInt aCommand )
    {
LOG_ENTERFN("WindowManager::SendCommandToAllWindowsL");
BROWSER_LOG( ( _L( "Command: %d" ), aCommand ) );

    CBrowserWindowQue *windowQue = iWindowQueue;
    for( ; windowQue; windowQue = windowQue->Next() )
        {
        windowQue->iWindow->BrCtlInterface().HandleCommandL(aCommand);
        }
    }

// ----------------------------------------------------------------------------
// CBrowserWindowManager::SetCurrentWindowViewState()
// ----------------------------------------------------------------------------
//
void CBrowserWindowManager::SetCurrentWindowViewState(TBrCtlDefs::TBrCtlState aViewState, TInt aValue)
	{
LOG_ENTERFN("WindowManager::SetCurrentWindowViewState");
BROWSER_LOG( ( _L( "State: %d" ), aViewState ) );

	TBool val(aValue > 0);
	if (aViewState == TBrCtlDefs::EStateWmlView)
		{
		CurrentWindow()->SetWMLMode(val); // set current page has wml (true or false)
		if (val)
			{
			CurrentWindow()->SetHasWMLContent(ETrue); // at least 1 page in window has/had wml
			CurrentWindow()->SetCurrWinHasWMLContent(ETrue); // current page has wml content

			}
		else
			{
			CurrentWindow()->SetCurrWinHasWMLContent(EFalse);// current page isn't wml
			}
		}
	}

// ----------------------------------------------------------------------------
// CBrowserWindowManager::CloseAllWindowsExceptCurrent()
// ----------------------------------------------------------------------------
//
void CBrowserWindowManager::CloseAllWindowsExceptCurrent()
    {
    LOG_ENTERFN("WindowManager::CloseAllWindowsExceptCurrent");
    CBrowserWindowQue* window = iWindowQueue;
    for( ; window; window = window->Next() )
        {
        if(iCurrentWindow != window)
            {
            DeleteWindowL(window->iWindow->WindowId(), EFalse);
            }
        }
    }
// End of file
