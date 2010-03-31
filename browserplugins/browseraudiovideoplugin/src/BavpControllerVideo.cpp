/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Controller class for handling browser requests to play video
*
*/


// INCLUDE FILES
#include <mmf/common/mmferrors.h>
#include <aknclearer.h>
#include <AknUtils.h>

#include "BavpPlugin.h"
#include "BavpLogger.h"
#include "BavpControllerVideo.h"
#include "BavpView.h"
#include "BavpViewFullScreen.h"
#include <mmf/common/mmfcontrollerframework.h> 
#include <MMFScalingCustomCommandConstants.h>

using namespace RT_GestureHelper;

// CONSTANTS
// One second represented in microseconds
const TInt KBavpOneSecond = 1000000;

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBavpControllerVideo::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
CBavpControllerVideo* CBavpControllerVideo::NewL( MBavpView* aView,
                                                  TUint aAccessPtId,
                                                  TBavpMediaType aMediaType,
                                                  const TDesC& aFileName )
    {
    CBavpControllerVideo* self = new( ELeave ) CBavpControllerVideo( aView, aAccessPtId );

    Log( EFalse, _L("CBavpControllerVideo::NewL this="), (TInt)self );

    CleanupStack::PushL( self );
    self->ConstructL( aMediaType, aFileName );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::~CBavpControllerVideo()
// Destructor
// -----------------------------------------------------------------------------
CBavpControllerVideo::~CBavpControllerVideo()
    {
    Log( EFalse, _L("CBavpControllerVideo::~CBavpControllerVideo this="), (TInt)this );

    // If currently playing, call Stop before you close and delete
    if ( iCurrentState == EBavpPlaying && iVideoPlayer)
        {
        iVideoPlayer->Stop();
        }

    // Close the video plugin, and delete it
    if ( iVideoPlayer )
        {
        iVideoPlayer->Close();
#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF
        iVideoPlayer->RemoveDisplayWindow(iBavpView->WindowInst());
#endif
        delete iVideoPlayer;
        }

    // CIdle AO used for display update
    if ( iDisplayUpdater )
        {
        delete iDisplayUpdater;
        }

  if ( iBackLightUpdater )
      {
      iBackLightUpdater->Cancel();
      delete iBackLightUpdater;
      }
    // Used for displaying content in full screen
    if ( iBavpViewFullScreen )
        {
        delete iBavpViewFullScreen;
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::CBavpControllerVideo
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CBavpControllerVideo::CBavpControllerVideo( MBavpView* aView, TUint aAccessPtId )
    : CBavpController( aView, aAccessPtId ), iActiveWindow(NULL)
    {
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CBavpControllerVideo::ConstructL( TBavpMediaType aMediaType,
                                       const TDesC& aFileName )
    {
    // Construct the BavpController base class
    BaseConstructL( aMediaType, aFileName );

    // Create CIdle (AO) to update the display
    iDisplayUpdater = CIdle::NewL( CActive::EPriorityIdle );
    iBackLightUpdater=CHeartbeat::NewL(CActive::EPriorityStandard);
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::PrepareCompleteL
// Handle the leaving methods needed by MvpuoPrepareComplete
// -----------------------------------------------------------------------------
void CBavpControllerVideo::PrepareCompleteL()
    {
    // Resolution
    TSize size;
    iVideoPlayer->VideoFrameSizeL(size);
    iClipInfo->iResolutionHeight = size.iHeight;
    iClipInfo->iResolutionWidth = size.iWidth;

    // Bitrate
    iClipInfo->iBitrate = iVideoPlayer->VideoBitRateL() +
                          iVideoPlayer->AudioBitRateL();

    TInt iMetaCount = iVideoPlayer->NumberOfMetaDataEntriesL();
    CMMFMetaDataEntry* metaData = NULL;

    for ( TInt i = 0; i < iMetaCount; i++ )
        {
        metaData = iVideoPlayer->MetaDataEntryL( i );

        Log( EFalse, _L("metaData index="), i );
        Log( EFalse, metaData->Name() );
        Log( EFalse, metaData->Value() );

        // Check if seekable
        if ( !metaData->Name().CompareF( KBavpSeekable ) )
            {
            if ( !metaData->Value().CompareF( KBavpFalse ) )
                {
                iClipInfo->iSeekable = EFalse;
                }
            }

        // Check if live stream
        if ( !metaData->Name().CompareF( KBavpLiveStream ) )
            {
            if ( !metaData->Value().CompareF( KBavpTrue ) )
                {
                // Live stream is not seekable
                iClipInfo->iLiveStream = ETrue;
                iClipInfo->iSeekable = EFalse;
                }
            }

        delete metaData;
        }   // end of for

    // Set up the max volume
    iClipInfo->iMaxVolume = iVideoPlayer->MaxVolume();
    iPlayerMaxVolume = iClipInfo->iMaxVolume;

    // Duration
    iClipInfo->iDurationInSeconds = iVideoPlayer->DurationL();

    // Audio track
    iClipInfo->iAudioTrack = iVideoPlayer->AudioEnabledL();

    // Video track
    iClipInfo->iVideoTrack = iVideoPlayer->VideoBitRateL();

    if ( IsClipSeekable() )
        {
        iVideoPlayer->SetPositionL( iClipInfo->iPosition );
        }
#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF
	TRect rect( iBavpView->CoeControl().Rect() );
    CEikonEnv* eikon = CEikonEnv::Static();
    RWsSession& ws = eikon->WsSession();
    CWsScreenDevice* screenDevice = eikon->ScreenDevice();
    // Clip rect manipulation.
    // Calculate the rect for display, including title and status pane
    TRect clipRect = GetClipRect( rect );
    TRAPD(errAdd, iVideoPlayer->AddDisplayWindowL(CCoeEnv::Static()->WsSession(),
                                                    *screenDevice,
                                                    iBavpView->WindowInst(),
                                                            rect,
                                                            clipRect));
    TRAPD(errScale, iVideoPlayer->SetAutoScaleL(iBavpView->WindowInst(), EAutoScaleBestFit));
    if( ( errAdd != KErrNone ) && (iActiveWindow != &iBavpView->WindowInst()) )
        {
        iActiveWindow = &iBavpView->WindowInst();
        }
    Log( EFalse, _L("CBavpControllerVideo::InitVideoPlayerL() AddDisplayWindowL %d"), errAdd );
    Log( EFalse, _L("CBavpControllerVideo::InitVideoPlayerL() SetAutoScaleL %d"), errScale );
#endif
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::InitVideoPlayer
// The non-leaving version that calls the leaving version. Closes the current
// player and prepares the MMF to play the video
// -----------------------------------------------------------------------------
void CBavpControllerVideo::InitVideoPlayer()
    {
    TRAP_IGNORE( InitVideoPlayerL() );
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::InitVideoPlayerL
// Closes the current player and prepares the MMF to play the video
// -----------------------------------------------------------------------------
void CBavpControllerVideo::InitVideoPlayerL()
    {
    Log( EFalse, _L("CBavpControllerVideo::InitVideoPlayerL()") );

    // If videoplayer is already opened, close and delete it
    if ( iVideoPlayer )
        {
        iVideoPlayer->Close();
#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF
        iVideoPlayer->RemoveDisplayWindow(iBavpView->WindowInst());
#endif
        delete iVideoPlayer;
        iVideoPlayer = NULL;
        }

    // Get the rect to display
    TRect rect( iBavpView->CoeControl().Rect() );
    TPoint pt = iBavpView->WindowInst().AbsPosition();
    rect.Move( pt.iX, pt.iY );

    CEikonEnv* eikon = CEikonEnv::Static();
    RWsSession& ws = eikon->WsSession();
    CWsScreenDevice* screenDevice = eikon->ScreenDevice();

    // Clip rect manipulation.
    // Calculate the rect for display, including title and status pane
    TRect clipRect = GetClipRect( rect );

    Log( EFalse, _L("InitVideoPlayerL() - calling CVideoPlayerUtility::NewL") );

    // Initialize the videoplayer
#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF
	iVideoPlayer = CVideoPlayerUtility2::NewL( *this, EMdaPriorityNormal,
                                              EMdaPriorityPreferenceNone);
#else
    iVideoPlayer = CVideoPlayerUtility::NewL( *this, EMdaPriorityNormal,
                                              EMdaPriorityPreferenceNone,
                                              ws, *screenDevice,
                                              iBavpView->WindowInst(), rect,
                                              clipRect);
#endif

    // Register for loading notification
    iVideoPlayer->RegisterForVideoLoadingNotification( *this );

    switch ( iClipInfo->iMediaType )
        {
        case ELocalVideoFile:
            Log( EFalse, _L("InitVideoPlayerL() - calling OpenVideoFile") );
            OpenVideoFileL();
            break;

        case ELocalRamFile:
        case EUrl:
        case ELocalSdpFile:
#if defined(BRDO_ASX_FILE)
		case ELocalAsxFile:
#endif // BRDO_ASX_FILE
            Log( EFalse, _L("InitVideoPlayerL() - calling OpenVideoUrlL") );
            OpenVideoUrlL();
            break;

        // Not supported
        case ELocalAudioPlaylist:
        default:
            User::Leave( KErrNotSupported );
            break;
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::UnInitVideoPlayer
// Closes the player and delete the instance
// -----------------------------------------------------------------------------
void CBavpControllerVideo::UnInitVideoPlayer()
    {
    Log( EFalse, _L("CBavpControllerVideo::UnInitVideoPlayer()") );

    if ( iVideoPlayer )
        {
        iVideoPlayer->Close();
#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF
        iVideoPlayer->RemoveDisplayWindow(iBavpView->WindowInst());
#endif
        delete iVideoPlayer;
        iVideoPlayer = NULL;
        }

    iCurrentState = EBavpNone;
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::RotateScreen90
// Rotate the screen if needed
// -----------------------------------------------------------------------------
void CBavpControllerVideo::RotateScreen90()
    {
    Log( EFalse, _L("CBavpControllerVideo::RotateScreen90()") );

    TRect screenRect;
    AknLayoutUtils::LayoutMetricsRect( AknLayoutUtils::EScreen, screenRect );

    // Rotate only if screen rect height is greater than its width
    if ( screenRect.Height() > screenRect.Width() )
        {
#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF
		TRAP_IGNORE( iVideoPlayer->SetRotationL(iBavpView->WindowInst(), EVideoRotationClockwise90 ) );
#else
		TRAP_IGNORE( iVideoPlayer->SetRotationL( EVideoRotationClockwise90 ) );
#endif

        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::RevertToFullScreenL
// Set full screen display mode
// -----------------------------------------------------------------------------
void CBavpControllerVideo::RevertToFullScreenL()
    {
    Log( EFalse, _L("CBavpControllerVideo::RevertToFullScreenL()"));
    CBavpPluginEcomMain* npm = (CBavpPluginEcomMain*)Dll::Tls();
    bool fullscreen = true;

    //This method is not supported when using CVideoPlayerUtility2, and will 
    //always leave with KErrNotSupported. instead of variating for new player 
    //we are ignoring the Leave
	TRAP_IGNORE( iVideoPlayer->StopDirectScreenAccessL() );


    iClipInfo->iInFullScreenMode = ETrue;

    npm->Funcs()->setvalue(iBavpView->bavPlugin()->getNPP(), 
                           NPPVpluginFullScreenBool, 
                           static_cast<void*>(&fullscreen));
    iBavpView->ControllerStateChangedL();
    
    CEikonEnv* eikon = CEikonEnv::Static();
    RWsSession& ws = eikon->WsSession();
    CWsScreenDevice* screenDevice = eikon->ScreenDevice();
    
    iNormalScreenRect = iBavpView->CoeControl().Rect();
    TPoint pt = iBavpView->WindowInst().AbsPosition();
    iNormalScreenRect.Move(pt.iX, pt.iY);
    TRect screenRect = eikon->EikAppUi()->ApplicationRect();
    
    CCoeControl* parentView = iBavpView->CoeControl().Parent();
    parentView->SetRect(screenRect);
    iBavpView->CoeControl().SetExtent(TPoint(0,0), screenRect.Size());

#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF

    iVideoPlayer->RemoveDisplayWindow(iBavpView->WindowInst());
    TRAPD(errAdd, iVideoPlayer->AddDisplayWindowL(ws,*screenDevice,iBavpView->WindowInst(), screenRect, screenRect));
    TRAPD(errScale, iVideoPlayer->SetAutoScaleL(iBavpView->WindowInst(), EAutoScaleBestFit));
    Log( EFalse, _L("CBavpControllerVideo::RevertToFullScreenL() errAdd %d"), errAdd);
    Log( EFalse, _L("CBavpControllerVideo::RevertToFullScreenL() errScale %d"), errScale);
#else
	iVideoPlayer->SetDisplayWindowL( ws, *screenDevice,
                                     iBavpView->WindowInst(),
                                     screenRect, screenRect );
                                         
    RotateScreen90();
#endif
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::RevertToNormalScreenL
// Set back to normal display mode
// -----------------------------------------------------------------------------
void CBavpControllerVideo::RevertToNormalScreenL()
    {
     Log( EFalse, _L("CBavpControllerVideo::RevertToNormalScreenL()") );
    CEikonEnv* eikon = CEikonEnv::Static();
    RWsSession& ws = eikon->WsSession();
    CWsScreenDevice* screenDevice = eikon->ScreenDevice();
    CBavpPluginEcomMain* npm = (CBavpPluginEcomMain*)Dll::Tls();
    bool fullscreen = false;
    
    //This method is not supported when using CVideoPlayerUtility2, and will 
    //always leave with KErrNotSupported. instead of variating for new player 
    //we are ignoring the Leave
    TRAP_IGNORE( iVideoPlayer->StopDirectScreenAccessL() );
    
    npm->Funcs()->setvalue(iBavpView->bavPlugin()->getNPP(), 
                               NPPVpluginFullScreenBool, 
                               static_cast<void*>(&fullscreen));

    CCoeControl* parentView = iBavpView->CoeControl().Parent();
    parentView->SetRect(iNormalScreenRect);
    iBavpView->CoeControl().SetExtent(TPoint(0,0), iNormalScreenRect.Size());

    iClipInfo->iInFullScreenMode = EFalse;
    RefreshRectCoordinatesL();
#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF
	iVideoPlayer->SetRotationL(iBavpView->WindowInst(), EVideoRotationNone );
#else
	iVideoPlayer->SetRotationL( EVideoRotationNone );
#endif

    iBavpView->ControllerStateChangedL();
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::ToggleScreen
// Start a call back for the display screen switches (Full Screen to
// Normal Screen)
// -----------------------------------------------------------------------------
void CBavpControllerVideo::ToggleScreen()
    {
    if ( iDisplayUpdater->IsActive() )
        {
        iDisplayUpdater->Cancel();
        }

    iDisplayUpdater->Start( TCallBack( CBavpControllerVideo::WaitForScreenSwitch,
                                       this ) );
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::RefreshRectCoordinatesL
// Refresh the coordinates of rect for display
// -----------------------------------------------------------------------------
void CBavpControllerVideo::RefreshRectCoordinatesL()
    {
    Log( EFalse, _L("CBavpControllerVideo::RefreshRectCoordinatesL()") );

    if ( IsClipFullScreen() ) {
        return;
    }
    if ( iVideoPlayer )
        {

#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF
        UpdateWindowSize();
#else
        TRect rect( iBavpView->CoeControl().Rect() );

        TRect wr(iBavpView->WindowRect()); //can have negative coordinates 
                                           //if video scrolled out of viewport
        CEikonEnv* eikon = CEikonEnv::Static();
        RWsSession& ws = eikon->WsSession();
        CWsScreenDevice* screenDevice = eikon->ScreenDevice();

		// Windows' absolute position, relative to the current screen
        TPoint pt = iBavpView->WindowInst().AbsPosition();
        rect.Move( pt.iX, pt.iY );
        wr.Move( pt.iX, pt.iY );

	 	// Reset clipRect
        TRect clipRect = rect;
        clipRect = GetClipRect( rect );

        Log( EFalse, _L("\nPosition: == clipRect to DAS:  ==\n") );
        Log( EFalse, _L("\nPosition:: clipRect.x = \n"), clipRect.iTl.iX );
        Log( EFalse, _L("\nPosition:: clipRect.y = \n"), clipRect.iTl.iY );
        Log( EFalse, _L("\nPosition:2: clipRect.x = \n"), clipRect.iBr.iX );
        Log( EFalse, _L("\nPosition:2: clipRect.y = \n"), clipRect.iBr.iY );

		TRAP_IGNORE
		    (
              iVideoPlayer->SetDisplayWindowL( ws, *screenDevice,
            							     iBavpView->WindowInst(),
            							     wr, clipRect );
		    );

#endif
        if( iCurrentState == EBavpPaused || iCurrentState == EBavpStopped || iCurrentState == EBavpPlayComplete )
            {
            iVideoPlayer->RefreshFrameL();
            }

        } //end of iVideoPlayer
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::WaitForScreenSwitch
// Switches between normal screen and full screen
// -----------------------------------------------------------------------------
TInt CBavpControllerVideo::WaitForScreenSwitch( TAny* aThisData )
    {
    TRAP_IGNORE( WaitForScreenSwitchL( aThisData ) );
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::WaitForScreenSwitch
// Switches between normal screen and full screen
// -----------------------------------------------------------------------------
void CBavpControllerVideo::WaitForScreenSwitchL( TAny* aThisData )
    {
    Log( EFalse, _L("CBavpControllerVideo::WaitForScreenSwitchL()") );

    // Use static cast
    CBavpControllerVideo* thisData = STATIC_CAST(CBavpControllerVideo*, aThisData);

    // Switch from Normal screen mode to Full Screen mode
    if ( !thisData->IsClipFullScreen() )
        {
        thisData->RevertToFullScreenL();
     
        }
    else if ( thisData->IsClipFullScreen() )
        {
        // Switch from Full Screen mode to Normal Screen mode
        thisData->RevertToNormalScreenL();
     
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::DoFastForwardL
// Fast forwards the video
// -----------------------------------------------------------------------------
TInt CBavpControllerVideo::DoFastForwardL( TAny* aThisData )
    {
    CBavpControllerVideo* thisData = STATIC_CAST( CBavpControllerVideo*, aThisData );
    TTimeIntervalMicroSeconds skipPosition( 0 );
    TTimeIntervalMicroSeconds currentPosition( thisData->iVideoPlayer->PositionL() );

    Log( EFalse, _L("CBavpControllerVideo::DoFastForwardL() - currentPosition = "),
         currentPosition.Int64() );

    // If we're not at the end of clip, skip forward a few positions
    if ( currentPosition < thisData->iClipInfo->iDurationInSeconds )
        {
        skipPosition = currentPosition.Int64() + KSkipToPosition;

        // If skip pass end of clip, set to the end of clip
        if ( skipPosition > thisData->iClipInfo->iDurationInSeconds )
            {
            skipPosition = thisData->iClipInfo->iDurationInSeconds;
            }

        Log( EFalse, _L("CBavpControllerVideo::DoFastForwardL() - skipPosition = "),
             skipPosition.Int64() );

        // Set the position
        thisData->iVideoPlayer->SetPositionL( skipPosition );

        // Refresh the frame
        thisData->iVideoPlayer->RefreshFrameL();
        thisData->iClipInfo->iPosition = skipPosition;
        }
    else
        {
        // Fast Forward reached the end of the clip, cancel the fast forward
        thisData->iPositionUpdater->Cancel();
        thisData->iClipInfo->iPosition = thisData->iClipInfo->iDurationInSeconds;
        // Set the state to complete since it reached the end of the clip
        thisData->iCurrentState = EBavpFastForwardComplete;
        Log( EFalse, _L("CBavpControllerVideo::DoFastForwardL() - End of clip") );
        }

    return ETrue;
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::DoRewindL
// Rewinds the video
// -----------------------------------------------------------------------------
TInt CBavpControllerVideo::DoRewindL( TAny* aThisData )
    {
    CBavpControllerVideo* thisData = STATIC_CAST( CBavpControllerVideo*, aThisData );
    TTimeIntervalMicroSeconds skipPosition( 0 );
    TTimeIntervalMicroSeconds currentPosition( thisData->iVideoPlayer->PositionL() );

    Log( EFalse, _L("CBavpControllerVideo::DoFastForwardL() - currentPosition = "),
         currentPosition.Int64() );

    // We're not at the beginning of the clip, rewind a few positions
    if ( currentPosition > 0 )
        {
        skipPosition = currentPosition.Int64() - KSkipToPosition;

        // If skip pass beginning of clip, set to the beginning of clip
        if ( skipPosition < 0 )
            {
            skipPosition = 0;
            }

        Log( EFalse, _L("CBavpControllerVideo::RewindL() - skipPosition = "),
             skipPosition.Int64() );

        // Set the position
        thisData->iVideoPlayer->SetPositionL( skipPosition );

        // Refresh the frame
        thisData->iVideoPlayer->RefreshFrameL();
        thisData->iClipInfo->iPosition = skipPosition;
        }
    else
        {
        // Rewind reached the beginning of the clip, cancel the rewind
        thisData->iPositionUpdater->Cancel();
        thisData->iClipInfo->iPosition = 0;
        // Set the state to complete since it reached beginning of clip
        thisData->iCurrentState = EBavpRewindComplete;
        Log( EFalse, _L("CBavpControllerVideo::RewindL() - beginning of clip") );
        }

    return ETrue;
    }

// --------------------------------------------------------------------------
// CBavpControllerVideo::HandleError
// Handle error codes. We can receive errors from the MMF Video Player
// and its MMF callbacks (mvpuoPlayComplete, etc).
// The MMF errors are for handling content issues, such as decoding content
// and network access.
// NOTES:
// MMF errors start at -12000, see /epoc32/include/mmf/common/MMFErrors.h
// MMF also returns -1 (KErrNotFound) and few other system-wide errors
// -----------------------------------------------------------------------------
void CBavpControllerVideo::HandleError( TInt aError )
    {
    Log( EFalse, _L("In CBavpControllerVideo::HandleError(): err="), aError );

    switch ( aError )
        {
        case KErrSessionClosed: // When we stop live streaming
            if ( iCurrentState == EBavpBuffering ||
                 iCurrentState == EBavpStopped )
                {
                Log( EFalse, _L("HandleError: iCurrentState=Buffering or Stopped and err=KErrSessionClosed") );
                // Controller needs to be re-initialized, whenever
                // live streaming is "Stopped"
                // iCurrentState = EBavpNone, set by UnInitVideoPlayer
                UnInitVideoPlayer();
                // Set state to stopped, because we "stop" live content
                iCurrentState = EBavpStopped;
                }
            else if ( iCurrentState == EBavpPlaying ||
                      iCurrentState == EBavpPaused )
                {
                Log( EFalse, _L("HandleError:iCurrentState=Playing or Paused and err=KErrSessionClosed") );
                // If the window is not on top, it might be a video or phone
                // call interrupt
                TRAP_IGNORE
                    (
                    if ( !iBavpView->IsTopWindowL() )
                        {
                        InitVideoPlayer();
                        }
                    )   // end of TRAP_IGNORE
                }
            else
                {
                }
            break;

        case KErrDisconnected:
            // If the window is not on top, it might be a video or phone
            // call interrupt
            Log( EFalse, _L("HandleError: err=KErrDisconnected") );
            TRAP_IGNORE
                (
                if ( !iBavpView->IsTopWindowL() )
                    {
                    InitVideoPlayer();
                    }
                )   // end of TRAP_IGNORE
            break;

        case KErrMMVideoDevice:
        case KErrMMAudioDevice:
            HandleMultiInstanceError();
            break;

        // Errors from MMF (i.e. MvpuoPrepareComplete, MvpuoPlayComplete...)
        case KErrNotFound:
        case KErrNotSupported:
        case KErrCorrupt:
        case KErrInUse:
        case KErrNoMemory:
        case KErrAbort:
        case KErrArgument:
        case KErrCouldNotConnect:
        case KErrTimedOut:
        case KErrHardwareNotAvailable:
        case KErrOverflow:
        case KErrMMNotEnoughBandwidth:
        case KErrMMNetworkRead:
        case KErrMMNetworkWrite:
        case KErrMMServerNotSupported:
        case KErrMMUDPReceive:
        case KErrMMInvalidProtocol:
        case KErrMMInvalidURL:
        case KErrMMProxyServer:
        case KErrMMDecoder:
        case KErrMMPartialPlayback:
            // Close the player and delete player instance.
            // iCurrentState = EBavpNone, set by UnInitVideoPlayer.
            UnInitVideoPlayer();

            // Display we have bad content
            Log( EFalse, _L("HandleError: state=BadContent") );
            iCurrentState = EBavpBadContent;
            iBavpView->UpdateView();
            break;

        default:
            // Do nothing
            break;

        }   // end of switch

    Log( EFalse, _L("Out CBavpControllerVideo::HandleError()") );
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::HandleKeysL
// Handles user key presses when we're Activated (aka Top of CCoeControl stack)
// -----------------------------------------------------------------------------
TKeyResponse CBavpControllerVideo::HandleKeysL( const TKeyEvent& aKeyEvent,
                                                TEventCode aType )
    {
    TKeyResponse keyResponse( EKeyWasNotConsumed );

    // If we're in full screen mode, and any key is pressed, toggle to normal
    // screen. Consume all key presses (down, key, up) when in full screen mode.
    if ( IsClipFullScreen() )
        {
        if ( aType == EEventKeyDown )
            {
            ToggleScreen();
            }
        keyResponse = EKeyWasConsumed;
        }
    else
        {
        // Use the default HandleKeysL method
        keyResponse = CBavpController::HandleKeysL( aKeyEvent, aType );
        }

    return keyResponse;
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::HandleCommandL
// Default implementation, override as needed.
// Handles the commands from the Option Menu defined in TBavpCommandIds
// -----------------------------------------------------------------------------
void CBavpControllerVideo::HandleCommandL( TBavpCommandIds aCommand )
    {
    Log( EFalse, _L("CBavpControllerVideo::HandleCommandL(): aCommand="), (TInt)aCommand );

    switch ( aCommand )
        {
        case EBavpCmdPlayFullScreen:
            ToggleScreen();
            break;

        default:
            // Use the default HandleCommandL method
            CBavpController::HandleCommandL( aCommand );
            break;
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::PlayAudioVideoL
// Handles request to open and play an audio or video content
// -----------------------------------------------------------------------------
void CBavpControllerVideo::PlayAudioVideoL()
    {
    Log( EFalse, _L("CBavpControllerVideo::PlayAudioVideoL() ") );

    if ( iClipInfo->iMediaType == ELocalRamFile )
        {
        ParseRamFileL();
        }
#if defined(BRDO_ASX_FILE)
    else if ( iClipInfo->iMediaType == ELocalAsxFile )
        {
        ParseAsxFileL();
        }
#endif // BRDO_ASX_FILE
    // Check for the mediatype and open the file
    InitVideoPlayerL();
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::Stop
// Stops video from playing, if playing.
// -----------------------------------------------------------------------------
void CBavpControllerVideo::Stop()
    {
    Log( EFalse, _L("CBavpControllerVideo::Stop") );

    // If we are fast forwarding or rewind, stop the position updater
    iPositionUpdater->Cancel();

    if ( iVideoPlayer )
        {
        // Stop video and set the state to stop
        iVideoPlayer->Stop();
        iCurrentState = EBavpStopped;

        iClipInfo->iPosition = 0;
        TRAP_IGNORE
            (
            iVideoPlayer->SetPositionL( iClipInfo->iPosition );
            iVideoPlayer->RefreshFrameL();
            );
        }

    // The videoPlayer can play audio-only content, so we need to updateDisplay
    // to show the audio stop animation
    if ( !IsClipVideo() )
        {
        iBavpView->UpdateView();
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::PlayL
// Plays video
//
// NOTE: The MMF throws an error when interrupted by a telephone call, so
// iCurrentState=EBavpBadContent and PlayL called after call is done.
// -----------------------------------------------------------------------------
void CBavpControllerVideo::PlayL()
    {
    Log( EFalse, _L("CBavpControllerVideo::PlayL") );

    if ( !iVideoPlayer || !iPlayerInitialized )
        {
        // 1) Live-stream clip that was stopped will close the iVideoPlayer and
        // need buffering, because Play() will have to be reinitialized with
        // server.
        // 2) The initialization of multiple video instances and a single
        // playback hardware/software (ie Julie H/W) will cause the second plus
        // instances to return an error. HandleError() will display stop animation,
        // and if the user selects PlayL(), and the iVideoPlayer needs to be
        // reinitialized.
        //
        // The buffering animation will be stopped, when MvpuoPrepareComplete()
        // is called.
        iCurrentState = EBavpBuffering;
        iBavpView->UpdateView();
        Log( EFalse, _L("PlayL - Calling InitVideoPlayerL()") );

        // This calls OpenUrlL or OpenFileL, which will call iVideoPlayer->Play
        InitVideoPlayerL();
        return;
        }

    if ( iCurrentState == EBavpPaused ||
         iCurrentState == EBavpFastForwarding ||
         iCurrentState == EBavpRewinding )
        {
        // If we are fast forwarding or rewind, pause
        if ( iPositionUpdater->IsActive() )
            {
            // PauseL stops the PositionUpdater
            PauseL();
            }

        // We are resuming play and the clip is seekable, get the last
        // position and adjust the last position, if needed
        if ( iClipInfo->iPosition >=
             iClipInfo->iDurationInSeconds.Int64() - KBavpOneSecond )
            {
            // Take back one second if too near end of clip
            TTimeIntervalMicroSeconds adjustPosition =
                iClipInfo->iPosition.Int64() - KBavpOneSecond;

            // Negative value is not allowed, use beginning of clip
            if ( adjustPosition.Int64() < 0 )
                {
                adjustPosition = 0;
                }

            // Use the adjusted position
            iClipInfo->iPosition = adjustPosition;
            }
        }
    else
        {
        // Handle all other states, EBavpStopped, EBavpPlayComplete,
        // EBavpRewindComplete, EBavpFastForwardComplete, EBavpBadContent
        iClipInfo->iPosition = 0;
        }

    if ( IsClipOnDemand() && iCurrentState == EBavpStopped )
        {
        // On-demand that was stopped will need buffering,
        // because Play() will have to be reinitialized with server.
        // The buffering animation is stopped, for on-demand when
        // MvloLoadingComplete() is called
        iCurrentState = EBavpBuffering;
        Log( EFalse, _L("PlayL - Set EBavpBuffering") );
        }
    else
        {
        // Set state and update view to stop any animations running
        iCurrentState = EBavpPlaying;
        Log( EFalse, _L("PlayL - Set EBavpPlaying") );
        }

    iBavpView->UpdateView();

    iVideoPlayer->SetPositionL( iClipInfo->iPosition );
    //Fix for "EABU-7ZW9YT" if the pause is initiated for foreground to background and also for plugin invisible and plugin playpause
    if(iCurrentState == EBavpPaused)
        iCurrentState = EBavpPlaying;

    iVideoPlayer->Play();
  // ETwelveOClock: Timer tick is on the second - from Symbian
  if (!iBackLightUpdater->IsActive())
    {
    iBackLightUpdater->Start( ETwelveOClock, this );
    }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::PauseL
// Pauses video while playing. Saves the last position, if seekable content.
// -----------------------------------------------------------------------------
void CBavpControllerVideo::PauseL()
    {
    Log( EFalse, _L("CBavpControllerVideo::PauseL") );

    // If the position updater is running, stop it
    iPositionUpdater->Cancel();

    if ( iVideoPlayer )
        {
    //disable screensaver
    iBackLightUpdater->Cancel();
        if ( IsClipSeekable() )
            {
            // Seekable content, save position for possible resume
            iVideoPlayer->PauseL();
            iClipInfo->iPosition = iVideoPlayer->PositionL();
            iCurrentState = EBavpPaused;
            }
        else
            {
            // Not seekable content, i.e. Live stream
            iVideoPlayer->Stop();
            iClipInfo->iPosition = 0;
            iVideoPlayer->SetPositionL( iClipInfo->iPosition );
            iCurrentState = EBavpStopped;
            }
        }
    // The videoPlayer can play audio-only content, so we need to updateDisplay
    // to show the audio paused animation
    if ( !IsClipVideo() )
        {
        iBavpView->UpdateView();
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::FastForwardL
// Fast forwards the video. This is only called on seekable and local file
// content.
// -----------------------------------------------------------------------------
void CBavpControllerVideo::FastForwardL()
    {
    // Check that the content can be Fast Forwarded, this is used when media key
    // is pressed. We don't allow the user to select FF from Option Menu, if this
    // condition is not already meet.
    if ( !IsClipSeekable() && !IsClipLocalMedia() )
        {
        return;
        }

    // If playing, pause it (PauseL stops the PositionUpdater)
    if ( iCurrentState == EBavpPlaying )
        {
        PauseL();
        }

    iCurrentState = EBavpFastForwarding;
    iPositionUpdater->Start( KInitialDelay, KUpdateFrequency,
                             TCallBack( &DoFastForwardL, this ) );
  if (!iBackLightUpdater->IsActive())
    {
    iBackLightUpdater->Start( ETwelveOClock, this );
    }
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::RewindL
// Rewinds the video. This is only called on seekable and local file
// content.
// -----------------------------------------------------------------------------
void CBavpControllerVideo::RewindL()
    {
    // Check that the content can be Rewound, this is used when media key
    // is pressed. We don't allow the user to select FF from Option Menu, if this
    // condition is not already meet.
    if ( !IsClipSeekable() && !IsClipLocalMedia() )
        {
        return;
        }

    // If playing, pause it (PauseL stops the PositionUpdater)
    if ( iCurrentState == EBavpPlaying )
        {
        PauseL();
        }

    iCurrentState = EBavpRewinding;
    iPositionUpdater->Start( KInitialDelay, KUpdateFrequency,
                             TCallBack( &DoRewindL, this ) );
  if (!iBackLightUpdater->IsActive())
    {
    iBackLightUpdater->Start( ETwelveOClock, this );
    }

    }
// -----------------------------------------------------------------------------
// CBavpControllerVideo::OpenVideoUrlL
// Handles request to open the url link
// -----------------------------------------------------------------------------
void CBavpControllerVideo::OpenVideoUrlL()
    {
    Log( EFalse, _L("CBavpControllerVideo::OpenUrlL() entered") );

    // Note: Play() is called from MvpuoPrepareComplete()
    iVideoPlayer->OpenUrlL( *iClipInfo->iUrl, iAccessPtId,
                            KNullDesC8, KRopControllerUid );
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::OpenVideoFileL
// Open a video file
// -----------------------------------------------------------------------------
void CBavpControllerVideo::OpenVideoFileL()
    {
    Log( EFalse, _L("CBavpControllerVideo::OpenFileL() ") );

    // Note: Play() is called from MvpuoPrepareComplete()
    iVideoPlayer->OpenFileL( *iClipInfo->iFileName );
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::SetPlayerVolume
// -----------------------------------------------------------------------------
void CBavpControllerVideo::SetPlayerVolume( TInt aVolume )
    {
    if ( iVideoPlayer && iPlayerInitialized )
        {
        // The CR volume is 0-10, convert to Video player volume 0-100
        TInt playerVolume = ( aVolume * iPlayerMaxVolume ) / KCRVolumeMax;

        // Set Video player volume
        TRAP_IGNORE( iVideoPlayer->SetVolumeL( playerVolume) );
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::GetPlayerVolume
// -----------------------------------------------------------------------------
TInt CBavpControllerVideo::GetPlayerVolume()
    {
    TInt playerVolume( 0 );
    TInt CRVolume( 0 );

    if ( iVideoPlayer && iPlayerInitialized )
        {
        // Video player returns 0-100
        playerVolume = iVideoPlayer->Volume();

        // Convert to CR volume 0-10
        CRVolume = ( playerVolume * KCRVolumeMax ) / iPlayerMaxVolume;
        }

    return CRVolume;
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::MvpuoOpenComplete
// Notification to the client that the opening of the video clip has completed,
// successfully, or otherwise.
// -----------------------------------------------------------------------------
void CBavpControllerVideo::MvpuoOpenComplete( TInt aError )
    {
    Log( EFalse, _L("MvpuoOpenComplete err="), aError );

    if ( aError != KErrNone )
        {
        // Got an error from the MMF callback
        HandleError( aError );
        }
    else
        {
        // No error opening player, proceed with preparing player
        iVideoPlayer->Prepare();
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::MvpuoPrepareComplete
// Notification to the client that the opening of the video clip has been prepared
// successfully, or otherwise.
// This callback is called in response to a call to CVideoPlayerUtility::Prepare().
// -----------------------------------------------------------------------------
void CBavpControllerVideo::MvpuoPrepareComplete( TInt aError )
    {
    Log( EFalse, _L("MvpuoPrepareComplete err="), aError );

    if ( aError != KErrNone )
        {
        HandleError( aError );
        }
    else
        {
        // Proceed with initialization and playing content

        // Video player initialized and ready for interaction,
        // volume, option menus, etc...
        iPlayerInitialized = ETrue;

        // Cleanup clip info
        iClipInfo->ReInit();

        // Handle all of the leaving methods of preparing the clip and
        // controller. Reading meta data, max volume, ...
        TRAP_IGNORE( PrepareCompleteL() );

        // Full screen display mode, rotate if necessary
        if ( IsClipFullScreen() )
            {
            RotateScreen90();
            }

        // Use the iCurrrentVolume, everytime we "set" the volume in the
        // BavpController we check for Profile setting, so we can use it here
        SetPlayerVolume( iCurrentVolume );

        // Start to play this media object
        iVideoPlayer->Play();

        // Stop the buffering animation now, to avoid screen flicker
        iCurrentState = EBavpPlaying;
        iBavpView->UpdateView();
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::MvpuoPlayComplete
// Notification that video playback has completed. This is not called if playback
// is explicitly stopped by calling Stop.
// -----------------------------------------------------------------------------
void CBavpControllerVideo::MvpuoPlayComplete( TInt aError )
    {
    Log( EFalse, _L("MvpuoPlayComplete err="), aError );

    if ( aError != KErrNone )
        {
        // Got an error from the MMF callback
        HandleError( aError );
        }
    else
        {
        // No error, proceed with playing complete
        iCurrentState = EBavpPlayComplete;
        iClipInfo->iPosition = 0;

        // Set the previous call state to complete, this will ensure we
        // don't try to replay the content if a call comes in
        iPreCallState = EBavpPlayComplete;

        // Revert back to normal screen if it's full screen display
        if ( IsClipFullScreen() )
            {
            TRAP_IGNORE
                (
                RevertToNormalScreenL();
                iBavpView->ControllerStateChangedL();
                );
            }

        iBavpView->UpdateView();
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::MvpuoEvent
// General event notification from controller. These events are specified by the
// supplier of the controller. Take the user initiaiated event, like selecting
// "pause" from the menu item, or another MMF event and convert into a
// "video plugin error", so we can process in HandleError().
// -----------------------------------------------------------------------------
void CBavpControllerVideo::MvpuoEvent( const TMMFEvent& aEvent )
    {
    Log( EFalse, _L("MvpuoEvent aEvent.iErrorCode="), aEvent.iErrorCode );

    if ( aEvent.iEventType == KMMFEventCategoryVideoPlayerGeneralError )
        {
        HandleError( aEvent.iErrorCode );
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::MvloLoadingComplete
// Notification that video clip loading/rebuffering has started
// -----------------------------------------------------------------------------
void CBavpControllerVideo::MvloLoadingStarted()
    {
    Log( EFalse, _L("MvloLoadingStarted") );

    if ( iCurrentState !=  EBavpPlaying )
        {
        // Start the buffering animation
        iCurrentState = EBavpBuffering;
        iBavpView->UpdateView();
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::MvloLoadingComplete
// Notification that video clip loading/rebuffering has completed
// and starting to play
// -----------------------------------------------------------------------------
void CBavpControllerVideo::MvloLoadingComplete()
    {
    Log( EFalse, _L("MvloLoadingComplete") );

  if ( !iBackLightUpdater->IsActive() )
    {
    iBackLightUpdater->Start( ETwelveOClock, this );
    }
    // Stop the buffering animation
    iCurrentState = EBavpPlaying;
    iBavpView->UpdateView();
    }

 // -----------------------------------------------------------------------------
// CBavpControllerVideo::getPositionL
// To get the current position of the clip
// -----------------------------------------------------------------------------

 TTimeIntervalMicroSeconds CBavpControllerVideo::getPositionL()
    {
    if ( iVideoPlayer )
    {
        return iVideoPlayer->PositionL();
    }
    return NULL;
    }

 // -----------------------------------------------------------------------------
// CBavpControllerVideo::setPositionL
// To set the position of the clip
// -----------------------------------------------------------------------------

  void CBavpControllerVideo::setPositionL(TTimeIntervalMicroSeconds val)
    {
        if ( iVideoPlayer )
        {
            if ( !IsClipSeekable() && !IsClipLocalMedia() )
            {
                return;
            }

            TBool didWePause = EFalse;
            // If playing, pause it (PauseL stops the PositionUpdater)
            if ( iCurrentState == EBavpPlaying )
            {
                PauseL();
                didWePause = ETrue;
            }

            if ( (val > iClipInfo->iDurationInSeconds ) || (val < 0) )
            {
                val = 0;
            }

            iVideoPlayer->SetPositionL(val);
            iVideoPlayer->RefreshFrameL();
            iClipInfo->iPosition = val;
            if ( didWePause )
            {
                PlayL();
            }
        }
    }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::Beat
// Call HandleScreenSaver
// -----------------------------------------------------------------------------
//
void CBavpControllerVideo::Beat()
  {
  HandleScreenSaver();
  }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::Synchronize
// Call HandleScreenSaver
// -----------------------------------------------------------------------------
//
void CBavpControllerVideo::Synchronize()
  {
  HandleScreenSaver();
  }

// -----------------------------------------------------------------------------
// CBavpControllerVideo::HandleScreenSaver
// To disable backlight for screensaver if needed
// -----------------------------------------------------------------------------
//
void CBavpControllerVideo::HandleScreenSaver()
{
   Log( EFalse, _L("CBavpControllerVideo::HandleScreenSaver() entered" ));

    //disable screensaver when it's in playing|forwarding|backwarding mode
    if ( NoScreenSaverMode() && iVideoPlayer )
      {
      User::ResetInactivityTime();
      }
}

TBool CBavpControllerVideo::NoScreenSaverMode()
{
  return (iCurrentState == EBavpPlaying ||
        iCurrentState == EBavpFastForwarding ||
        iCurrentState == EBavpRewinding);
}


// -----------------------------------------------------------------------------
// CBavpControllerVideo::SetDefaultAspectRatioL
// Sets default aspect ratio (code is taken from mmplayer)
// -----------------------------------------------------------------------------
/*
 * This function was added for consistency with mmplayer.
 * Not used is 5.0 (S60_50) since we want to preserve correct
 * aspect ratio and this function will stretch the video to
 * the full screen, which due to S60_50 screen size doesn't match
 * to correct ratio.
 * If needed it should be called in  WaitForScreenSwitchL() after 
 * screen mode changed.
 */
void CBavpControllerVideo::SetDefaultAspectRatioL()
{
  TSize frameSize;
  TRect rect = iBavpView->CoeControl().Rect(); 
  
  if ( iVideoPlayer ) {

    iVideoPlayer->VideoFrameSizeL( frameSize );
  }

  if ( frameSize.iWidth > 0 && frameSize.iHeight > 0) {
 
    TMMFScalingType scalingType = EMMFNatural;
 
    TReal32 videoAspectRatio = (TReal)frameSize.iWidth /
                                (TReal)frameSize.iHeight;
    TReal32 dispAspectRatio = (TReal)rect.Width() / (TReal)rect.Height(); 
 
 // Choose the scaling type through the rule:
 // videoAspectRatio - iDisplayAspectRatio > 0.1 ==> zoom
 // videoAspectRatio - iDisplayAspectRatio < 0.1 ==> stretch
 // videoAspectRatio = iDisplayAspectRatio ==> natural
     if ( videoAspectRatio - dispAspectRatio > 0.1 ) {
       scalingType = EMMFZoom;
     }
     else if ( videoAspectRatio != dispAspectRatio ) {
       scalingType = EMMFStretch;
     }

     TMMFMessageDestinationPckg destinationPckg(KUidInterfaceMMFControllerScalingMsg );
      
     TPckgBuf<TInt> scaleTypePckg( scalingType );
     
     iVideoPlayer->CustomCommandSync( destinationPckg,
                                       EMMFSetScalingType,
                                       scaleTypePckg,
                                       KNullDesC8 );
     iVideoPlayer->RefreshFrameL();

   }
}

void CBavpControllerVideo::HandleInComingCallL()

{
	if ( IsClipFullScreen() )
      {
      RevertToNormalScreenL();
      iBavpView->ControllerStateChangedL();
      }	
}

// -----------------------------------------------------------------------------
// CBavpControllerVideo::HandleGesture
// -----------------------------------------------------------------------------
TBool CBavpControllerVideo::HandleGesture(TGestureEvent *gesture)
{
    TBool ret = EFalse;
    TGestureCode gtype =  gesture->Code(EAxisBoth); 
    switch(gtype)
        {
        case EGestureTap:
            {
                if (IsClipFullScreen())
                    {
                    ToggleScreen();
                    ret = ETrue;
                    }
                break;
            }
        }
    return ret;
}
#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF
// -----------------------------------------------------------------------------
// CBavpControllerVideo::UpdateWindowSize
// Refreshing the window co-ordinates.
// -----------------------------------------------------------------------------
/*
 * MMF Client API has updated with new methods to control video display 
 * Windows and video picture positioning to produce a new version of the API, 
 * CVideoPlayerUtility2. The new API is the preferred way to play video on graphics
 * surfaces, and will support new features such as more flexible window positioning.
 * Updatewindow has made in separate method so that in future, timer can be 
 * implemented for redusing the call to update window. 
 */
void CBavpControllerVideo::UpdateWindowSize()
    {
    TRect rect( iBavpView->CoeControl().Rect() );
    TRect wr(iBavpView->WindowRect()); //can have negative coordinates
    //if video scrolled out of viewport
    CEikonEnv* eikon = CEikonEnv::Static();
    RWsSession& ws = eikon->WsSession();
    CWsScreenDevice* screenDevice = eikon->ScreenDevice();
    // Reset clipRect
    TRect clipRect = rect;
    clipRect = GetClipRect( rect );
    if( ( iActiveWindow != &iBavpView->WindowInst() ) && ( iActiveWindow != NULL ) )
        {
        //Remove the active window and add the new window
        iVideoPlayer->RemoveDisplayWindow(iBavpView->WindowInst());
        TRAPD(errAdd, iVideoPlayer->AddDisplayWindowL(ws,*screenDevice,iBavpView->WindowInst(), wr, clipRect));
        TRAP_IGNORE(iVideoPlayer->SetAutoScaleL(iBavpView->WindowInst(), EAutoScaleBestFit));
        if( errAdd == KErrNone )
            iActiveWindow = &iBavpView->WindowInst();
        }
    else
        {
        //window is already active, only needs the window size changed or position change
        TRAPD(err1,iVideoPlayer->SetVideoExtentL(iBavpView->WindowInst(),wr));
        TRAPD(err2,iVideoPlayer->SetWindowClipRectL(iBavpView->WindowInst(),clipRect));
        Log( EFalse, _L("SetVideoExtent err1 = %d"), err1 );
        Log( EFalse, _L("SetVideoExtent err2 = %d"), err2 );
        }
    }
#endif
//  End of File
