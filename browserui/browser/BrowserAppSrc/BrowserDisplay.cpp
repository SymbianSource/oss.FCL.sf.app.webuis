/*
* Copyright (c) 2002-2006 Nokia Corporation and/or its subsidiary(-ies).
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
*      Access to display components
*
*
*/


// INCLUDE FILES
// System includes
#include <aknappui.h>
#include <aknnavi.h>
#include <aknnavide.h>
#include <akntabgrp.h>
#include <akntitle.h>
#include <eikenv.h>
#include <eikspane.h>
#include <ApEngineConsts.h>
#include <avkon.hrh>
#include <bldvariant.hrh>
#include <FeatMgr.h>
#include <akneditstateindicator.h>
#include <aknindicatorcontainer.h>
#include <AknStatuspaneUtils.h>
#include <Browser_platform_variant.hrh>

#include <wlaninternalpskeys.h> // For WLAN state checking

#include <LayoutMetaData.cdl.h> // For Layout_Meta_Data landscape/portrait status

// User includes
#include "ApiProvider.h"
#include "BrowserProgressIndicator.h"
#include "BrowserDisplay.h"
#include "BrowserUtil.h"
#include "Preferences.h"
#include "CommonConstants.h"
#include "BrowserWindow.h"
#include "BrowserWindowManager.h"
#include "Logger.h"

#include "WlanObserver.h"

#include <BrowserNG.rsg>

// TYPES

struct TIndicatorContainer {
    TInt locName;   // indicator's local name
    TInt aknName;   // indicator's avkon name
    TInt indState;  // indicator's value
};

// cosmetic macros
#define _AKN_IND( a ) EAknNaviPaneEditorIndicator ## a
#define _LOC_IND( a ) ELocalIndicator ## a
#define _IND_STATE( a ) KLocalIndicators[ _LOC_IND( a ) ].indState
#define _SET_INDIC( a ) SetIndicatorState( \
    TUid::Uid( _AKN_IND( a ) ), _IND_STATE( a ) )
#define _SET_INDIC2( a, b ) SetIndicatorState( \
    TUid::Uid( _AKN_IND( a ) ), _IND_STATE( a ), b )

// indicators used in this module
enum {
    _LOC_IND( SecuredConnection ),
    _LOC_IND( MessageInfo ),
    _LOC_IND( ProgressBar ),
    // _LOC_IND( WaitBar ),  //
    // _LOC_IND( T9 ),  //
    // _LOC_IND( UpperCase ),  //
    // _LOC_IND( LowerCase ),  //
    // _LOC_IND( AudioAttached ),  //
    // _LOC_IND( Objects ),  //
    // _LOC_IND( WmlWaitGlobe ),
    _LOC_IND( Gprs ),
    _LOC_IND( WlanAvailable ),
    _LOC_IND( WlanActive ),
    _LOC_IND( WlanActiveSecure ),
    _LOC_IND( FileSize ),
    // _LOC_IND( MessageLength ),  //
    _LOC_IND( WmlWindows ),
    _LOC_IND( WmlWindowsText ),
    _LOC_IND( LastItem )
};

// CONSTANTS
_LIT( KBrowserSpaceChar, " " );

// order of the indicators in this structure MUST match the enum definition above!
LOCAL_D TIndicatorContainer KLocalIndicators[] =
{
    { _LOC_IND( SecuredConnection ), _AKN_IND( SecuredConnection ), EAknIndicatorStateOff },
    { _LOC_IND( MessageInfo ),       _AKN_IND( MessageInfo ),       EAknIndicatorStateOff },
    { _LOC_IND( ProgressBar ),       _AKN_IND( ProgressBar ),       EAknIndicatorStateOff },
    // { _LOC_IND( WaitBar ),           _AKN_IND( WaitBar ),           EAknIndicatorStateOff },
    // { _LOC_IND( T9 ),                _AKN_IND( T9 ),                EAknIndicatorStateOff },
    // { _LOC_IND( UpperCase ),         _AKN_IND( UpperCase ),         EAknIndicatorStateOff },
    // { _LOC_IND( LowerCase ),         _AKN_IND( LowerCase ),         EAknIndicatorStateOff },
    // { _LOC_IND( AudioAttached ),     _AKN_IND( AudioAttached ),     EAknIndicatorStateOff },
    // { _LOC_IND( Objects ),           _AKN_IND( Objects ),           EAknIndicatorStateOff },
    // { _LOC_IND( WmlWaitGlobe ),      _AKN_IND( WmlWaitGlobe ),      EAknIndicatorStateOff },
    { _LOC_IND( Gprs ),              _AKN_IND( Gprs ),              EAknIndicatorStateOn },
    { _LOC_IND( WlanAvailable ),     _AKN_IND( WlanAvailable ),     EAknIndicatorStateOff },
    { _LOC_IND( WlanActive ),        _AKN_IND( WlanActive ),        EAknIndicatorStateOff },
    { _LOC_IND( WlanActiveSecure ),  _AKN_IND( WlanActiveSecure ),  EAknIndicatorStateOff },
    { _LOC_IND( FileSize ),          _AKN_IND( FileSize ),          EAknIndicatorStateOff },
    // { _LOC_IND( MessageLength ),     _AKN_IND( MessageLength ),     EAknIndicatorStateOff },
    { _LOC_IND( WmlWindows ),        _AKN_IND( WmlWindows ),        EAknIndicatorStateOff },
    { _LOC_IND( WmlWindowsText ),    _AKN_IND( WmlWindowsText ),    EAknIndicatorStateOff },
    { _LOC_IND( LastItem ),          0,                             0 }
};

// ================= MEMBER FUNCTIONS =======================

// C++ default constructor can NOT contain any code, that
// might leave.
//
CBrowserDisplay::CBrowserDisplay( MApiProvider& aApiProvider, CBrowserWindow& aWindow )
: iApiProvider( aApiProvider ), iWindow( &aWindow )
	{
	}

// ---------------------------------------------------------------------------
// CBrowserDisplay::ConstructL()
// ---------------------------------------------------------------------------
//
void CBrowserDisplay::ConstructL()
	{
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
	iProgressIndicator = CBrowserProgressIndicator::NewL( iApiProvider );
    iEditorIndicatorContainer = NULL;
    iTitle = NULL;
PERFLOG_STOP_WRITE("BrProgressIndicator")
    InitIndicatorsL();  //for full screen

#ifdef _DEBUG
    // check the indicator array's integrity
    TInt i = 0;
    while( KLocalIndicators[i].locName != _LOC_IND( LastItem ) )
        {
        __ASSERT_DEBUG( KLocalIndicators[i].locName == i,
            Util::Panic( Util::EUnExpected ) );
        ++i;
        }
#endif  // _DEBUG
	}

// ---------------------------------------------------------------------------
// CBrowserDisplay::NewL()
// ---------------------------------------------------------------------------
//
CBrowserDisplay* CBrowserDisplay::NewL( MApiProvider& aApiProvider, CBrowserWindow& aWindow )
	{
	CBrowserDisplay* self = new (ELeave) CBrowserDisplay( aApiProvider, aWindow );

	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop();	// self

	return self;
	}

// ---------------------------------------------------------------------------
// CBrowserDisplay::~CBrowserDisplay()
// ---------------------------------------------------------------------------
//
CBrowserDisplay::~CBrowserDisplay()
	{
	delete iProgressIndicator;
    delete iEditorIndicatorContainer;
    delete iTitle;
    delete iWlanObserver;
	}

// ---------------------------------------------------------------------------
// CBrowserDisplay::StatusPane()
// ---------------------------------------------------------------------------
//
CEikStatusPane* CBrowserDisplay::StatusPane() const
    {
	return STATIC_CAST( CAknAppUi*, CEikonEnv::Static()->EikAppUi() )->
                                                                StatusPane();
    }

// ---------------------------------------------------------------------------
// CBrowserDisplay::NaviPaneL()
// ---------------------------------------------------------------------------
//
CAknNavigationControlContainer* CBrowserDisplay::NaviPaneL() const
    {
    CEikStatusPane* sp = StatusPane();
    User::LeaveIfNull( sp );
    return STATIC_CAST( CAknNavigationControlContainer*,
					sp->ControlL( TUid::Uid( EEikStatusPaneUidNavi ) ) );
    }

// ---------------------------------------------------------
// CBrowserDisplay::SetTitleL()
// ---------------------------------------------------------
//
void CBrowserDisplay::SetTitleL( const TDesC& aTitle )
    {
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
    HBufC* temp = aTitle.AllocL();
    delete iTitle;
    iTitle = temp;
    CAknIndicatorContainer* indicContainerEditing = SelectIndicContainerEditing();
    // Checking to see if we are in contentview so that title is updated for other views like
    // Bookmarks and Settings. In landscape mode title could be updated all the time regardless of progress bar. 
    if ( !Layout_Meta_Data::IsLandscapeOrientation() && 
           (indicContainerEditing) && (ApiProvider().LastActiveViewId() == KUidBrowserContentViewId ))
        {
		//Under full screen mode, the progress bar is at the bottom and the title should be updated
		//all the time.
		if (indicContainerEditing->IndicatorState( 
            TUid::Uid(EAknNaviPaneEditorIndicatorProgressBar) ) != EAknIndicatorStateOn)
            {
            RestoreTitleL();
            }
		}
    else
        {
        RestoreTitleL();
        }
PERFLOG_STOP_WRITE("BrDsply:SetTitle")
    }

// ---------------------------------------------------------
// CBrowserDisplay::ClearMessageInfo()
// ---------------------------------------------------------
//
void CBrowserDisplay::ClearMessageInfo()
    {
    
    if( iEditorIndicatorContainer != NULL )
        {
        CAknIndicatorContainer* indicContainerEditing = SelectIndicContainerEditing();
        CAknIndicatorContainer* indicContainerOwn = SelectIndicContainerOwn();

        _IND_STATE( MessageInfo ) = EAknIndicatorStateOff;
        indicContainerEditing->_SET_INDIC( MessageInfo );
        indicContainerOwn->_SET_INDIC( MessageInfo );
        }
    }
// ---------------------------------------------------------
// CBrowserDisplay::RestoreTitleL()
// ---------------------------------------------------------
//
void CBrowserDisplay::RestoreTitleL()
    {
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
    if ( (iTitle !=NULL) )
        {

        if(( iApiProvider.LastActiveViewId() == KUidBrowserContentViewId )
        	&& ( iEditorIndicatorContainer != NULL )
        	&& (!(AknStatuspaneUtils::StaconPaneActive())) )
            {
            CAknIndicatorContainer* indicContainerEditing = SelectIndicContainerEditing();
            CAknIndicatorContainer* indicContainerOwn = SelectIndicContainerOwn();

            _IND_STATE( FileSize ) = EAknIndicatorStateOff;
            indicContainerEditing->_SET_INDIC( FileSize );
            indicContainerOwn->_SET_INDIC( FileSize );
            indicContainerOwn->SetIndicatorValueL( 
                TUid::Uid( _AKN_IND( FileSize ) ), KNullDesC() );

            if ( Layout_Meta_Data::IsLandscapeOrientation() )
                { 
                // Lanscape mode - Title in title pane not navipane/messageinfo
                ClearMessageInfo();
                }
            else
                { // Portrait mode - Title in navipane/messageinfo
                _IND_STATE( MessageInfo ) = EAknIndicatorStateOn;
                indicContainerEditing->_SET_INDIC( MessageInfo );
                indicContainerOwn->_SET_INDIC( MessageInfo );
                indicContainerEditing->SetIndicatorValueL(
                    TUid::Uid( _AKN_IND( MessageInfo ) ), *iTitle);
                indicContainerOwn->SetIndicatorValueL(
                    TUid::Uid( _AKN_IND( MessageInfo ) ), *iTitle);
                }

            CEikStatusPane* sp = STATIC_CAST( CAknAppUi*,
                CEikonEnv::Static()->EikAppUi() )->StatusPane();
            CAknTitlePane* title = STATIC_CAST( CAknTitlePane*,
                sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
            title->SetTextL( *iTitle );
            }
        else
            {
            if( ( iEditorIndicatorContainer != NULL )   
                && ( AknStatuspaneUtils::StaconPaneActive() ) )
               {
               CAknIndicatorContainer* indicContainerEditing = SelectIndicContainerEditing();
               CAknIndicatorContainer* indicContainerOwn = SelectIndicContainerOwn();
                //turn off other indicator, because both are visible in landscape mode
               _IND_STATE( MessageInfo ) = EAknIndicatorStateOff;
               indicContainerEditing->_SET_INDIC2( MessageInfo, ETrue );
               indicContainerOwn->_SET_INDIC2( MessageInfo, ETrue );
               }

            // Set title to be page title
            CEikStatusPane* sp = STATIC_CAST( CAknAppUi*,
                CEikonEnv::Static()->EikAppUi() )->StatusPane();
            CAknTitlePane* title = STATIC_CAST( CAknTitlePane*,
               sp->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
            title->SetTextL( *iTitle );
            }

#ifndef __WINSCW__ 
        // Wireless Lan Indicator 
        TInt wlanStateValue;
        StartWlanObserverL();  // if needed
        if ( iWlanObserver && iWlanObserver->GetCurrentState(wlanStateValue) )
            {
            UpdateWlanIndicator(wlanStateValue);
            }
#endif // __WINSCW__

        // Multiple Windows Indicator
        TInt winCount = iApiProvider.WindowMgr().WindowCount();
        TBool showMWIndic = (( winCount > 1 ) && 
                            (ApiProvider().LastActiveViewId() == KUidBrowserContentViewId )) ?
                             ETrue : EFalse;
        UpdateMultipleWindowsIndicatorL( showMWIndic, winCount );
        }
PERFLOG_STOP_WRITE("BrDsply:RestoreTitle")
    }

// ---------------------------------------------------------
// CBrowserDisplay::SetTitleL()
// ---------------------------------------------------------
//
void CBrowserDisplay::SetTitleL( TInt aResourceId )
	{
	HBufC* title = CEikonEnv::Static()->AllocReadResourceLC( aResourceId );
    SetTitleL( *title );
    CleanupStack::PopAndDestroy();   // title
    }
	
// ---------------------------------------------------------
// CBrowserDisplay::StartProgressAnimationL()
// ---------------------------------------------------------
//
void CBrowserDisplay::StartProgressAnimationL()
	{
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
LOG_ENTERFN( "Display::StartProgressAnimationL" );
BROWSER_LOG( ( _L("myWindow: %d, active: %d"),
        iWindow->WindowId(), iWindow->IsWindowActive() ) );
    if ( iApiProvider.IsProgressShown() )
        {
        // ProgressIndicator doesn't have pointer to its Window or parent Display
        // so check validity here
        if( iWindow->IsWindowActive() )
            {
            iProgressIndicator->StartL();
            }
        // start globe animation
        iApiProvider.StartProgressAnimationL();
        }
PERFLOG_STOP_WRITE("BrDsply: StartPrgAnim")
    }

// ---------------------------------------------------------
// CBrowserDisplay::StopProgressAnimationL()
// ---------------------------------------------------------
//
void CBrowserDisplay::StopProgressAnimationL()
    {
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
LOG_ENTERFN( "Display::StopProgressAnimationL" );
BROWSER_LOG( ( _L("myWindow: %d"), iWindow->WindowId() ) );
    if ( iApiProvider.IsProgressShown() )
        {
        // always stop spinning globe in FavouritesView
        // most of the cases View::DoDeactivateL() does it
        // other: e.g. Cancel load in BookmarksView
        iApiProvider.StopProgressAnimationL();

        if ( !ApiProvider().Fetching() )
            {
            if ( iEditorIndicatorContainer != NULL )
                {
                CAknIndicatorContainer* indicContainerEditing = SelectIndicContainerEditing();
                CAknIndicatorContainer* indicContainerOwn = SelectIndicContainerOwn();

                _IND_STATE( ProgressBar ) = EAknIndicatorStateOff;
                indicContainerEditing->_SET_INDIC( ProgressBar );
                indicContainerOwn->_SET_INDIC( ProgressBar );

                if( iTitle )
                    {
                    RestoreTitleL();
                    }
                iApiProvider.SetProgressShown( EFalse );
                }
            iProgressIndicator->StopL();
            }
        }
    PERFLOG_STOP_WRITE("BrDsply: StopPrgAnim")
    }

// ---------------------------------------------------------
// CBrowserDisplay::SelectIndicContainerEditing()
// ---------------------------------------------------------
//
CAknIndicatorContainer* CBrowserDisplay::SelectIndicContainerEditing()
    {
    CAknIndicatorContainer* indicContainer1 = CAknEnv::Static()->
        EditingStateIndicator()->IndicatorContainer();
    return indicContainer1;
    }

// ---------------------------------------------------------
// CBrowserDisplay::SelectIndicContainerOwn()
// ---------------------------------------------------------
//
CAknIndicatorContainer* CBrowserDisplay::SelectIndicContainerOwn()
    {
    CAknIndicatorContainer* indicContainer2 = STATIC_CAST(
        CAknIndicatorContainer*, iEditorIndicatorContainer->DecoratedControl() );
    return indicContainer2;
    }

// ---------------------------------------------------------
// CBrowserDisplay::InitIndicatorsL()
// ---------------------------------------------------------
//
void CBrowserDisplay::InitIndicatorsL()
    {
    PERFLOG_LOCAL_INIT
    PERFLOG_STOPWATCH_START

    if (iEditorIndicatorContainer)
    	{
        return;	
    	}

    if ((iEditorIndicatorContainer == NULL))
        {
        PERFLOG_LOCAL_INIT
        PERFLOG_STOPWATCH_START
        iEditorIndicatorContainer = NaviPaneL()->CreateEditorIndicatorContainerL();
        PERFLOG_STOP_WRITE(" -> BrDsply: EditorContainer")
        }
        
    PERFLOG_STOPWATCH_START
        CAknIndicatorContainer* indicContainer = SelectIndicContainerOwn();

        _IND_STATE( SecuredConnection ) = EAknIndicatorStateOff;
        _IND_STATE( MessageInfo )       = EAknIndicatorStateOff;
        _IND_STATE( ProgressBar )       = EAknIndicatorStateOff;
        // _IND_STATE( WaitBar )           = EAknIndicatorStateOff;
        // _IND_STATE( T9 )                = EAknIndicatorStateOff;
        // _IND_STATE( UpperCase )         = EAknIndicatorStateOff;
        // _IND_STATE( LowerCase )         = EAknIndicatorStateOff;
        // _IND_STATE( AudioAttached )     = EAknIndicatorStateOff;
        // _IND_STATE( Objects )           = EAknIndicatorStateOff;
        // _IND_STATE( WmlWaitGlobe )      = EAknIndicatorStateOff;
        _IND_STATE( Gprs )              = EAknIndicatorStateOn;

        _IND_STATE( WlanAvailable )     = EAknIndicatorStateOff;
        _IND_STATE( WlanActive )        = EAknIndicatorStateOff;
        _IND_STATE( WlanActiveSecure )  = EAknIndicatorStateOff;

        _IND_STATE( FileSize )          = EAknIndicatorStateOff;
        // _IND_STATE( MessageLength )     = EAknIndicatorStateOff;
        _IND_STATE( WmlWindows )        = EAknIndicatorStateOff;
        _IND_STATE( WmlWindowsText )    = EAknIndicatorStateOff;

        indicContainer->_SET_INDIC( SecuredConnection );
        indicContainer->_SET_INDIC( MessageInfo );
        indicContainer->_SET_INDIC( ProgressBar );
        // indicContainer->_SET_INDIC( WaitBar );
        // indicContainer->_SET_INDIC( T9 );
        // indicContainer->_SET_INDIC( UpperCase );
        // indicContainer->_SET_INDIC( LowerCase );
        // indicContainer->_SET_INDIC( AudioAttached );
        // indicContainer->_SET_INDIC( Objects );
        // indicContainer->_SET_INDIC( WmlWaitGlobe );
        indicContainer->_SET_INDIC( Gprs );

        indicContainer->_SET_INDIC( WlanAvailable );
        indicContainer->_SET_INDIC( WlanActive );
        indicContainer->_SET_INDIC( WlanActiveSecure );

        indicContainer->_SET_INDIC( FileSize );
        // indicContainer->_SET_INDIC( MessageLength );
        indicContainer->_SET_INDIC( WmlWindows );
        indicContainer->_SET_INDIC( WmlWindowsText );
    PERFLOG_STOP_WRITE( "Display:InitIndicators")
    }

// ---------------------------------------------------------
// CBrowserDisplay::UpdateSecureIndicatorL()
// ---------------------------------------------------------
//
void CBrowserDisplay::UpdateSecureIndicatorL( const TInt aState )
    {
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
    if( !iApiProvider.ExitInProgress() )
        {
        SetFullScreenSecureIndicatorL(aState);
        NaviPaneL()->DrawDeferred();
        }
PERFLOG_STOP_WRITE( "Display:UpdateSecureIndicator")
    }

// ---------------------------------------------------------
// CBrowserDisplay::UpdateFSDownloadInitialIndicator()
// ---------------------------------------------------------
//
void CBrowserDisplay::UpdateFSDownloadInitialIndicator( const TBool aState )
	{
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
    if( !iEditorIndicatorContainer ) 
    	{
    	InitIndicatorsL();
    	}
        
    if ( iApiProvider.IsProgressShown() && ( iEditorIndicatorContainer != NULL ) )
        {
        CAknIndicatorContainer* indicContainerEditing = SelectIndicContainerEditing();
        CAknIndicatorContainer* indicContainerOwn = SelectIndicContainerOwn();
        if (aState)
            { 
            ClearMessageInfo();

            _IND_STATE( FileSize ) = EAknIndicatorStateOff;
            indicContainerEditing->_SET_INDIC( FileSize );
            indicContainerOwn->_SET_INDIC( FileSize );
            }
        }
PERFLOG_STOP_WRITE("BrDsply: GlobeAnimation init")
    }

// ---------------------------------------------------------
// CBrowserDisplay::SetGPRSIndicatorOnL()
// ---------------------------------------------------------
//
void CBrowserDisplay::SetGPRSIndicatorOnL()
	{
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
    if ( iEditorIndicatorContainer != NULL )
        {
        // Set GPRS indicator on, it will update itself
        CAknIndicatorContainer* indicContainerEditing = SelectIndicContainerEditing();
        CAknIndicatorContainer* indicContainerOwn = SelectIndicContainerOwn();
        _IND_STATE( Gprs ) = EAknIndicatorStateOn;
        indicContainerEditing->_SET_INDIC( Gprs );
        indicContainerOwn->_SET_INDIC( Gprs );
        }
PERFLOG_STOP_WRITE("BrDsply: Gprsidic on")
    }

// ---------------------------------------------------------
// CBrowserDisplay::UpdateWlanIndicator()
// ---------------------------------------------------------
//
void CBrowserDisplay::UpdateWlanIndicator( const TInt aWlanValue )
    {
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START    

    if ( iEditorIndicatorContainer != NULL )
        {
        CAknIndicatorContainer* indicContainerEditing = SelectIndicContainerEditing();
        CAknIndicatorContainer* indicContainerOwn = SelectIndicContainerOwn();
        
        _IND_STATE( WlanAvailable ) = EAknIndicatorStateOff;
        indicContainerEditing->_SET_INDIC( WlanAvailable );
        indicContainerOwn->_SET_INDIC( WlanAvailable );     

        _IND_STATE( WlanActive ) = EAknIndicatorStateOff;
        indicContainerEditing->_SET_INDIC( WlanActive );
        indicContainerOwn->_SET_INDIC( WlanActive );     

        _IND_STATE( WlanActiveSecure ) = EAknIndicatorStateOff;
        indicContainerEditing->_SET_INDIC( WlanActiveSecure );
        indicContainerOwn->_SET_INDIC( WlanActiveSecure );     

        switch (aWlanValue)
            {
            case EPSWlanIndicatorNone:
                {
                // do nothing.
                break;
                }
            
            case EPSWlanIndicatorAvailable:
                {
                _IND_STATE( WlanAvailable ) = EAknIndicatorStateOn;
                indicContainerEditing->_SET_INDIC( WlanAvailable );
                indicContainerOwn->_SET_INDIC( WlanAvailable );     
                break;
                }           
       
            case EPSWlanIndicatorActive:
                {
                _IND_STATE( WlanActive ) = EAknIndicatorStateOn;
                indicContainerEditing->_SET_INDIC( WlanActive );
                indicContainerOwn->_SET_INDIC( WlanActive );     
                break;
                }
            
            case EPSWlanIndicatorActiveSecure:
                {

                _IND_STATE( WlanActiveSecure ) = EAknIndicatorStateOn;
                indicContainerEditing->_SET_INDIC( WlanActiveSecure );
                indicContainerOwn->_SET_INDIC( WlanActiveSecure );     
                break;
                }
            
            default:
                {
                // do nothing.
                break;
                }
            }
        }
PERFLOG_STOP_WRITE("BrDsply: UpdateWlanIndicator")
    }
    
// ---------------------------------------------------------
// CBrowserDisplay::StartWlanObserverL()
// ---------------------------------------------------------
//
void CBrowserDisplay::StartWlanObserverL()
	{
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START

    if (iWlanObserver == NULL)
        {
        iWlanObserver = new ( ELeave ) CWlanObserver( iApiProvider );
        }
        
PERFLOG_STOP_WRITE("BrDsply: Wlan Observer Started")
    }    

// ---------------------------------------------------------
// CBrowserDisplay::SetFullScreenSecureIndicatorL()
// ---------------------------------------------------------
//
void CBrowserDisplay::SetFullScreenSecureIndicatorL(const TBool aState)
	{
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
	_IND_STATE( SecuredConnection ) = aState;
    if ( iEditorIndicatorContainer != NULL && iWindow->IsWindowActive() )
        {
        CAknIndicatorContainer* indicContainerEditing = SelectIndicContainerEditing();
        CAknIndicatorContainer* indicContainerOwn = SelectIndicContainerOwn();
        _IND_STATE( SecuredConnection ) = aState ?
            EAknIndicatorStateOn : EAknIndicatorStateOff;
        indicContainerEditing->_SET_INDIC( SecuredConnection );
        indicContainerOwn->_SET_INDIC( SecuredConnection );
	    }
PERFLOG_STOP_WRITE("BrDsply: Secure indic upd")
    }

// ---------------------------------------------------------
// CBrowserDisplay::AddTransActIdL()
// ---------------------------------------------------------
//
void CBrowserDisplay::AddTransActIdL( TUint16 aId ) const
    {
    iProgressIndicator->AddTransActIdL( aId );
    }

// ---------------------------------------------------------
// CBrowserDisplay::AddProgressDataL()
// ---------------------------------------------------------
//
void CBrowserDisplay::AddProgressDataL(
        TUint16 aId, TUint32 aRecvdData, TUint32 aMaxData ) const
    {
    iProgressIndicator->AddProgressDataL( aId, aRecvdData, aMaxData );
    }

// ---------------------------------------------------------
// CBrowserDisplay::TransActIdComplete()
// ---------------------------------------------------------
//
void CBrowserDisplay::TransActIdComplete( TUint16 aId ) const
    {
    TRAP_IGNORE( iProgressIndicator->TransActIdCompleteL( aId ) );
    }

// ---------------------------------------------------------
// CBrowserDisplay::StartFSWaitIndicator()
// ---------------------------------------------------------
//
void CBrowserDisplay::StartFSWaitIndicator()
    {
    if( iEditorIndicatorContainer != NULL &&
        iWindow->IsWindowActive() )
        {
        CAknIndicatorContainer* indicContainerEditing = SelectIndicContainerEditing();
        CAknIndicatorContainer* indicContainerOwn = SelectIndicContainerOwn();
        }
    }

// ---------------------------------------------------------
// CBrowserDisplay::UpdateFSProgressIndicator()
// ---------------------------------------------------------
//
void CBrowserDisplay::UpdateFSProgressIndicator( const TInt aMaxData, 
                                                 const TInt aReceivedData )
    {
PERFLOG_LOCAL_INIT
PERFLOG_STOPWATCH_START
LOG_ENTERFN("UpdateFSProgressIndicator");
BROWSER_LOG( ( _L( "maxData: %d, recData: %d" ), aMaxData, aReceivedData ) );
    if ( iApiProvider.IsProgressShown() && 
       ( iEditorIndicatorContainer != NULL ) &&
         iWindow->IsWindowActive() )
        {
        CAknIndicatorContainer* indicContainerEditing = SelectIndicContainerEditing();
        CAknIndicatorContainer* indicContainerOwn = SelectIndicContainerOwn();

        if( indicContainerEditing->IndicatorState( 
                TUid::Uid( EAknNaviPaneEditorIndicatorProgressBar ) )
                != EAknIndicatorStateOn ||
            indicContainerOwn->IndicatorState( 
                TUid::Uid( EAknNaviPaneEditorIndicatorProgressBar ) )
                != EAknIndicatorStateOn
            )
            {
            _IND_STATE( ProgressBar ) = EAknIndicatorStateOn;
            indicContainerEditing->_SET_INDIC2( ProgressBar, ETrue );
            indicContainerOwn->_SET_INDIC2( ProgressBar, ETrue );
            }

        indicContainerEditing->SetIndicatorValue( 
            TUid::Uid( EAknNaviPaneEditorIndicatorProgressBar ), 
            aReceivedData, aMaxData );
        indicContainerOwn->SetIndicatorValue( 
            TUid::Uid( EAknNaviPaneEditorIndicatorProgressBar ), 
            aReceivedData, aMaxData );
        }
PERFLOG_STOP_WRITE("BrDsply: Prg indic upd")
    }

// ---------------------------------------------------------
// CBrowserDisplay::UpdateFSProgressDataL()
// ---------------------------------------------------------
//
void CBrowserDisplay::UpdateFSProgressDataL( const TDesC16& aReceivedDataText )
    {
    PERFLOG_LOCAL_INIT
	PERFLOG_STOPWATCH_START
    if ( iApiProvider.IsProgressShown() &&( iEditorIndicatorContainer != NULL ) )
        {
        CAknIndicatorContainer* indicContainerEditing = 
                                                SelectIndicContainerEditing();
        CAknIndicatorContainer* indicContainerOwn = SelectIndicContainerOwn();

        if( indicContainerEditing->IndicatorState( 
                TUid::Uid( EAknNaviPaneEditorIndicatorMessageInfo ) )
                != EAknIndicatorStateOff )
            {
            ClearMessageInfo();
            }

        if( indicContainerEditing->IndicatorState( 
                TUid::Uid( EAknNaviPaneEditorIndicatorFileSize ) )
                != EAknIndicatorStateOn ||
            indicContainerOwn->IndicatorState( 
                TUid::Uid( EAknNaviPaneEditorIndicatorFileSize ) )
                != EAknIndicatorStateOn )
            {
            _IND_STATE( FileSize ) = EAknIndicatorStateOn;
            indicContainerEditing->_SET_INDIC( FileSize );
            indicContainerOwn->_SET_INDIC( FileSize );
            }

        indicContainerEditing->SetIndicatorValueL( 
            TUid::Uid( _AKN_IND( FileSize ) ), aReceivedDataText );
        indicContainerOwn->SetIndicatorValueL( 
            TUid::Uid( _AKN_IND( FileSize ) ), aReceivedDataText );

        NaviPaneL()->PushL( *iEditorIndicatorContainer );
        }
    PERFLOG_STOP_WRITE("BrDsply: Prg data upd")
    }


// ---------------------------------------------------------
// CBrowserDisplay::FSPaneOnL()
// ---------------------------------------------------------
//
void CBrowserDisplay::FSPaneOnL()
    {
    InitIndicatorsL();
    NaviPaneL()->PushL(*iEditorIndicatorContainer);
    }

// ---------------------------------------------------------
// CBrowserDisplay::NotifyProgress()
// ---------------------------------------------------------
//
void CBrowserDisplay::NotifyProgress()
    {
    if ( iProgressIndicator != NULL && iWindow->IsWindowActive() )
        {
        iProgressIndicator->NotifyProgress();
        }
    }

// ----------------------------------------------------------------------------
// CBrowserDisplay::UpdateMultipleWindowsIndicatorL()
// ----------------------------------------------------------------------------
//
void CBrowserDisplay::UpdateMultipleWindowsIndicatorL( 
                                                TBool aState, TInt aWinCount )
    {
    LOG_ENTERFN("CBrowserDisplay::UpdateMultipleWindowsIndicatorL");

    if ( iEditorIndicatorContainer != NULL )
        {
        CAknIndicatorContainer* indicContainerEditing = 
                                                SelectIndicContainerEditing();
        CAknIndicatorContainer* indicContainerOwn = SelectIndicContainerOwn();

        // Show or hide MW Icon
        _IND_STATE( WmlWindows ) = aState ? 
            EAknIndicatorStateOn : EAknIndicatorStateOff;
        indicContainerEditing->_SET_INDIC( WmlWindows );
        indicContainerOwn->_SET_INDIC( WmlWindows );

        // Show or hide text indicator
        _IND_STATE( WmlWindowsText ) = aState ? 
            EAknIndicatorStateOn : EAknIndicatorStateOff;
        indicContainerEditing->_SET_INDIC( WmlWindowsText );
        indicContainerOwn->_SET_INDIC( WmlWindowsText );

        if ( aState )
            {
            // MW Indicator is made of an indicator icon and a number (num wins)

            // Display Number of windows open in text indicator
            HBufC* numWins = HBufC::NewLC( 3 );
            numWins->Des().AppendNum( aWinCount );
            numWins->Des().Append( KBrowserSpaceChar );
            indicContainerEditing->SetIndicatorValueL(
                TUid::Uid( _AKN_IND( WmlWindowsText ) ), *numWins );
            indicContainerOwn->SetIndicatorValueL(
                TUid::Uid( _AKN_IND( WmlWindowsText ) ), *numWins );
            CleanupStack::PopAndDestroy( numWins );
            }
	    }
    }

// End of File
