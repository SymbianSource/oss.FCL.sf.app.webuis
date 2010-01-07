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
* Description:  Uses the interface MPluginAdapter to notify browser of construction 
				 and destruction of the plug-in, implements interface MPluginOptionHandler
				 to add menus and draws animated images while video player loading the data
*
*/


// INCLUDE FILES
#include <AknUtils.h>
#include <aknclearer.h>
#include <AknBitmapAnimation.h>
#include <data_caging_path_literals.hrh>
#include <barsread.h>                       // For TResourceReader
#include <StringLoader.h>
#include <pluginadapterinterface.h>
#include <BrowserAudioVideoPlugin.rsg>      // For text

#include <brctldefs.h>

#include "BavpLogger.h"
#include "BavpView.h"
#include "BavpController.h"
#include "BavpPlugin.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBavpView::CBavpView
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CBavpView::CBavpView()
    {
    }

// -----------------------------------------------------------------------------
// CBavpView::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
// 
void CBavpView::ConstructL( const CCoeControl* aParent, 
                            const TRect& aRect, CBavpPlugin* aBavPlugin )
    {
    Log( EFalse, _L("CBavpView::ConstructL") );
    iBavPlugin = aBavPlugin;
    
	CreateWindowL(aParent);
	ActivateL(); // Draws icon
	iRect = aRect;
	CCoeControl::SetRect( aRect ); // Calls SizeChanged

    User::LeaveIfError( iWsSession.Connect() );
    iWindowId = iCoeEnv->RootWin().Identifier();

	// Video buffering animation, used to show that content is buffering
	iVideoBuffAnimation = CAknBitmapAnimation::NewL();
    iVideoBuffAnimation->SetContainerWindowL( *this );
    TResourceReader rrBuffering;
    iCoeEnv->CreateResourceReaderLC( rrBuffering, R_ANIMATION_BUFFERING );
    iVideoBuffAnimation->ConstructFromResourceL( rrBuffering );
    CleanupStack::PopAndDestroy();  // rrBuffering
	iIsVideoBuffAnimRunning = EFalse;

	// Bad content animation, used to show that content is not available
	iBadContentAnimation = CAknBitmapAnimation::NewL();
    iBadContentAnimation->SetContainerWindowL( *this );
    TResourceReader rrNoContentAnim;
    iCoeEnv->CreateResourceReaderLC( rrNoContentAnim, R_NO_CONTENT_ANIMATION );
    iBadContentAnimation->ConstructFromResourceL( rrNoContentAnim );
    CleanupStack::PopAndDestroy();  // rrNoContentAnim
	iIsBadContentAnimRunning = EFalse;

	// Video stopped animation, used to show that video content is stopped
	iVideoStoppedAnimation = CAknBitmapAnimation::NewL();
    iVideoStoppedAnimation->SetContainerWindowL( *this );
    TResourceReader rrVideoStoppedAnim;
    iCoeEnv->CreateResourceReaderLC( rrVideoStoppedAnim, R_VIDEO_STOPPED_ANIMATION );
    iVideoStoppedAnimation->ConstructFromResourceL( rrVideoStoppedAnim );
    CleanupStack::PopAndDestroy();  // rrVideoStoppedAnim
	iIsVideoStoppedAnimRunning = EFalse;

	// Video paused animation, used to show that video content is paused
	iVideoPausedAnimation = CAknBitmapAnimation::NewL();
    iVideoPausedAnimation->SetContainerWindowL( *this );
    TResourceReader rrVideoPausedAnim;
    iCoeEnv->CreateResourceReaderLC( rrVideoPausedAnim, R_VIDEO_PAUSED_ANIMATION );
    iVideoPausedAnimation->ConstructFromResourceL( rrVideoPausedAnim );
    CleanupStack::PopAndDestroy();  // rrVideoPausedAnim
	iIsVideoPausedAnimRunning = EFalse;

	// Audio playing animation, used when no video is present, only audio
	iAudioPlayAnimation = CAknBitmapAnimation::NewL();
    iAudioPlayAnimation->SetContainerWindowL( *this );
    TResourceReader rrAudioPlayAnim;
    iCoeEnv->CreateResourceReaderLC( rrAudioPlayAnim, R_AUDIO_PLAY_ANIMATION );
    iAudioPlayAnimation->ConstructFromResourceL( rrAudioPlayAnim );
    CleanupStack::PopAndDestroy();  // rrAudioPlayAnim
	iIsAudioPlayAnimRunning = EFalse;
	
	// Audio stopped animation, used to show that audio content is stopped
	iAudioStoppedAnimation = CAknBitmapAnimation::NewL();
    iAudioStoppedAnimation->SetContainerWindowL( *this );
    TResourceReader rrAudioStoppedAnim;
    iCoeEnv->CreateResourceReaderLC( rrAudioStoppedAnim, R_AUDIO_STOPPED_ANIMATION );
    iAudioStoppedAnimation->ConstructFromResourceL( rrAudioStoppedAnim );
    CleanupStack::PopAndDestroy();  // rrAudioStoppedAnim
	iIsAudioStoppedAnimRunning = EFalse;

	// Audio paused animation, used to show that audio content is paused
	iAudioPausedAnimation = CAknBitmapAnimation::NewL();
    iAudioPausedAnimation->SetContainerWindowL( *this );
    TResourceReader rrAudioPausedAnim;
    iCoeEnv->CreateResourceReaderLC( rrAudioPausedAnim, R_AUDIO_PAUSED_ANIMATION );
    iAudioPausedAnimation->ConstructFromResourceL( rrAudioPausedAnim );
    CleanupStack::PopAndDestroy();  // rrAudioPausedAnim
	iIsAudioPausedAnimRunning = EFalse;

    Log( EFalse, _L("CBavpView::ConstructL - Start Buffering animation") );
    RunAnimation(iVideoBuffAnimation, iIsVideoBuffAnimRunning);
	}

// -----------------------------------------------------------------------------
// CBavpView::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
CBavpView* CBavpView::NewL( const CCoeControl* aParent, 
                            const TRect& aRect, CBavpPlugin* aBavPlugin )
    {
    Log( EFalse, _L("CBavpView::NewL") );

    CBavpView* self = new( ELeave ) CBavpView;
    CleanupStack::PushL( self );
    self->ConstructL( aParent, aRect, aBavPlugin );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CBavpView::Destructor
// -----------------------------------------------------------------------------
CBavpView::~CBavpView()
    {
	Log( EFalse, _L("CBavpView::~CBavpView") );

	// Check if the keyhandling is on the stack, if yes, remove it
	if ( iIsOnStack )
		{
		// Check if the animation is active 
		// if it is not start animation
		iEikonEnv->EikAppUi()->RemoveFromStack( this );
		}
	
	// Cancel video buffering animation, if it's still running
	if ( iIsVideoBuffAnimRunning )
		{
		iVideoBuffAnimation->CancelAnimation();
		}
	delete iVideoBuffAnimation;

    // Cancel video stopped animation, if it's still running
    if ( iIsVideoStoppedAnimRunning )
        {
        iVideoStoppedAnimation->CancelAnimation();
        }
    delete iVideoStoppedAnimation;

    // Cancel video paused animation, if it's still running
    if ( iIsVideoPausedAnimRunning )
        {
        iVideoPausedAnimation->CancelAnimation();
        }
    delete iVideoPausedAnimation;

    // Cancel audio only animation, if it's still running
	if ( iIsAudioPlayAnimRunning )
		{
		iAudioPlayAnimation->CancelAnimation();
		}
	delete iAudioPlayAnimation;

    // Cancel audio stopped animation, if it's still running
    if ( iIsAudioStoppedAnimRunning )
        {
        iAudioStoppedAnimation->CancelAnimation();
        }
    delete iAudioStoppedAnimation;

    // Cancel audio paused animation, if it's still running
    if ( iIsAudioPausedAnimRunning )
        {
        iAudioPausedAnimation->CancelAnimation();
        }
    delete iAudioPausedAnimation;

    // Cancel bad content animation, if it's still running
    if ( iIsBadContentAnimRunning )
        {
        iBadContentAnimation->CancelAnimation();
        }
    delete iBadContentAnimation;

    iWsSession.Close();
    }

// -----------------------------------------------------------------------------
// CBavpView::SizeChanged
// Responds to size changes to sets the size and position of the contents of 
// this control
// -----------------------------------------------------------------------------
void CBavpView::SizeChanged()
    {
	Log( EFalse, _L("CBavpView::SizeChanged") );
	
	if ( iBavpController )
		{
		iRect = Rect();
		iBavpController->RefreshRectCoordinatesL();
		}
    }

// -----------------------------------------------------------------------------
// CBavpView::AddPluginOptionMenuItemsL
// Add the Option Menu items to the menuPane, these will be handled by plugin.
// -----------------------------------------------------------------------------
void CBavpView::AddPluginOptionMenuItemsL( CEikMenuPane& aMenuPane,
                                           TInt aCommandBase,
                                           TInt /*aAfter*/ )
    {
	Log( EFalse, _L("CBavpView::AddPluginOptionMenuItemsL") );

	iCommandBase = aCommandBase;
	TInt index( 0 );

	// The menus are only added, if the controller exists and we're in
	// normal screen and not bad content
	if ( !iBavpController || iBavpController->IsClipFullScreen() ||
	     iBavpController->State() == EBavpBadContent )
		{
		return;
	    }

    // Display the "Mute" menu item, unless we have HW volume keys, and...
	if ( !HAS_HW_VOLUME_KEYS  &&
		 !iBavpController->IsVolumeMuted() &&
		 !iBavpController->IsClipRewinding() &&
		 !iBavpController->IsClipFastForwarding() )
		{
		InsertOneMenuItemL( aMenuPane, EBavpCmdMuteVolume,
		                    R_TEXT_VIDEO_PLUGIN_VOLUME_MUTE, index);
		index++;
		}

	// Check CurrentState to determine the menu items to add
	switch ( iBavpController->State() )
		{
		case EBavpStopped:
		case EBavpRewindComplete:
		case EBavpPlayComplete:
			{
			// The content is at the beginning, user can play
			InsertOneMenuItemL( aMenuPane, EBavpCmdPlay,
			                    R_TEXT_VIDEO_PLUGIN_PLAY, index );
			index ++;
			break;
			}
		case EBavpPlaying:
			{
			// Check if it can be paused
			if ( iBavpController->IsClipSeekable() )
    			{
				InsertOneMenuItemL( aMenuPane, EBavpCmdPause,
				                    R_TEXT_VIDEO_PLUGIN_PAUSE, index);
				index++;
    			}
	
			// If playing, user can also stop
			InsertOneMenuItemL( aMenuPane, EBavpCmdStop,
			                    R_TEXT_VIDEO_PLUGIN_STOP, index );
			index++;
			// Add FF and Rewind, only if content seekable, local media, video
			if ( iBavpController->IsClipSeekable() &&
			     iBavpController->IsClipLocalMedia() &&
			     iBavpController->IsClipVideo() )
				{
				InsertOneMenuItemL( aMenuPane, EBavpCmdFastForward,
				                    R_TEXT_VIDEO_PLUGIN_FAST_FORWARD, index ); 
				index++;
				InsertOneMenuItemL( aMenuPane, EBavpCmdRewind,
				                    R_TEXT_VIDEO_PLUGIN_REWIND, index );
			    index++;
				}
			// Add full screen, only if video content
			if ( iBavpController->IsClipVideo() )
    			{
				InsertOneMenuItemL( aMenuPane, EBavpCmdPlayFullScreen,
				                    R_TEXT_VIDEO_PLUGIN_FULL_SCREEN, index );
				index++;
    			}
			break;
			}
		case EBavpPaused:
			{
			// If paused, user can continue (play) or stop
			InsertOneMenuItemL( aMenuPane, EBavpCmdPlay,
			                    R_TEXT_VIDEO_PLUGIN_PLAY, index );
			index++;
			
			InsertOneMenuItemL( aMenuPane, EBavpCmdStop,
			                    R_TEXT_VIDEO_PLUGIN_STOP, index );
			index++;
			// Add FF and Rewind, only if content seekable, local media, video
			if ( iBavpController->IsClipSeekable() &&
			     iBavpController->IsClipLocalMedia() &&
			     iBavpController->IsClipVideo() )
				{
				InsertOneMenuItemL( aMenuPane, EBavpCmdFastForward,
				                    R_TEXT_VIDEO_PLUGIN_FAST_FORWARD, index ); 
				index++;
				InsertOneMenuItemL( aMenuPane, EBavpCmdRewind,
				                    R_TEXT_VIDEO_PLUGIN_REWIND, index );
			    index++;
				}
			break;
			}
		case EBavpFastForwarding:
		case EBavpRewinding:
			{
			// User can play, while FF or Rewinding
			InsertOneMenuItemL( aMenuPane, EBavpCmdPlay,
			                    R_TEXT_VIDEO_PLUGIN_PLAY, index );
			index++;
			// Add pause, we should only be fast-forwarding or rewinding
			// seekable content, no need to check
			InsertOneMenuItemL( aMenuPane, EBavpCmdPause,
			                    R_TEXT_VIDEO_PLUGIN_PAUSE, index);
			index++;
			break;
			}
		case EBavpFastForwardComplete:
            {
			// Content is at the end, user can replay
			InsertOneMenuItemL( aMenuPane, EBavpCmdPlay,
			                    R_TEXT_VIDEO_PLUGIN_PLAY, index );
			index++;
			// Add rewind, we should only be fast-forwarding seekable content,
			// no need to check
			InsertOneMenuItemL( aMenuPane, EBavpCmdRewind,
			                    R_TEXT_VIDEO_PLUGIN_REWIND, index );
		    index++;
			break;
            }
		default:
			{
			break;
			}
		} // End of switch
	
    // Display "Media Volume" menu item, unless we have HW volume keys and...
	if ( !HAS_HW_VOLUME_KEYS &&  
		 !iBavpController->IsClipRewinding() &&
		 !iBavpController->IsClipFastForwarding() )
		{
		// User can adjust volume
		InsertOneMenuItemL( aMenuPane, EBavpCmdChangeVolume,
		                    R_TEXT_VIDEO_PLUGIN_MEDIA_VOLUME, index );
		}
    }

// -----------------------------------------------------------------------------
// CBavpView::HandlePluginCommandL
// Handle the user command from the option menu to the plugin.
// -----------------------------------------------------------------------------
void CBavpView::HandlePluginCommandL( TInt aCommand )
	{
	iBavpController->HandleCommandL( (TBavpCommandIds)(aCommand - iCommandBase) );
	}

// -----------------------------------------------------------------------------
// CBavpView::OfferKeyEventL
// Control framework calls this function to handle the key event
// -----------------------------------------------------------------------------
TKeyResponse CBavpView::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                        TEventCode aType )
    {
	Log( EFalse, _L("CBavpView::OfferKeyEventL") );

    TKeyResponse keyResponse( EKeyWasNotConsumed );
    
	//Check if the controller is already initialized
    if (iBavpController)
        {
        // Let the Audio or Video controller handle the key presses
        keyResponse = iBavpController->HandleKeysL( aKeyEvent, aType );    	
        }
	
	return keyResponse;
    }

// -----------------------------------------------------------------------------
// CBavpView::SetRect
// Sets the control's extent, specifying a TRect
// -----------------------------------------------------------------------------
void CBavpView::SetRect( NPWindow* aWindow )
    {
	Log( EFalse, _L("CBavpView::SetRect") );
    iWindowRect = TRect(TPoint(aWindow->x, aWindow->y), TSize(aWindow->width, aWindow->height));
    iClipRect = TRect(aWindow->clipRect.left, aWindow->clipRect.top, aWindow->clipRect.right, aWindow->clipRect.bottom);
    TRect r(iClipRect);
    r.Intersection(iWindowRect);
    TBool flag = iRect != r;
    iRect = r;
	CCoeControl::SetRect( iRect );

	// Set the rect only when the animation is playing
	if ( iIsVideoBuffAnimRunning && iVideoBuffAnimation && 
		 flag)
    	{
    	iVideoBuffAnimation->CancelAnimation();
    	TRect animRect( iWindowRect);
    	iVideoBuffAnimation->SetRect( animRect );
    	TRAP_IGNORE( iVideoBuffAnimation->StartAnimationL() );
    	}
	else if ( iIsAudioPlayAnimRunning && iAudioPlayAnimation &&
    	      flag)
	    {
		iAudioPlayAnimation->CancelAnimation();
    	TRect animRect( iWindowRect);
		iAudioPlayAnimation->SetRect( animRect );
		TRAP_IGNORE( iAudioPlayAnimation->StartAnimationL() );
        }
	else if ( iIsAudioStoppedAnimRunning && iAudioStoppedAnimation &&
    	      flag)
	    {
		iAudioStoppedAnimation->CancelAnimation();
    	TRect animRect( iWindowRect);
		iAudioStoppedAnimation->SetRect( animRect );
		TRAP_IGNORE( iAudioStoppedAnimation->StartAnimationL() );
        }
	else if ( iIsAudioPausedAnimRunning && iAudioPausedAnimation &&
    	      flag)
	    {
		iAudioPausedAnimation->CancelAnimation();
    	TRect animRect( iWindowRect);
		iAudioPausedAnimation->SetRect( animRect );
		TRAP_IGNORE( iAudioPausedAnimation->StartAnimationL() );
        }
	else if ( iIsVideoStoppedAnimRunning && iVideoStoppedAnimation &&
    	      flag)
	    {
		iVideoStoppedAnimation->CancelAnimation();
    	TRect animRect( iWindowRect);
		iVideoStoppedAnimation->SetRect( animRect );
		TRAP_IGNORE( iVideoStoppedAnimation->StartAnimationL() );
        }
	else if ( iIsVideoPausedAnimRunning && iVideoPausedAnimation &&
    	      flag)
	    {
		iVideoPausedAnimation->CancelAnimation();
    	TRect animRect( iWindowRect);
		iVideoPausedAnimation->SetRect( animRect );
		TRAP_IGNORE( iVideoPausedAnimation->StartAnimationL() );
        }
	else if ( iIsBadContentAnimRunning && iBadContentAnimation &&
    	      flag)
	    {
		iBadContentAnimation->CancelAnimation();
    	TRect animRect( iWindowRect);
		iBadContentAnimation->SetRect( animRect );
		TRAP_IGNORE( iBadContentAnimation->StartAnimationL() );
        }
    }

// -----------------------------------------------------------------------------
// CBavpView::IsTopWindow
// Return ETrue if the CCoeControl is the top window instance
// -----------------------------------------------------------------------------
TBool CBavpView::IsTopWindowL()
    {
	Log( EFalse, _L("CBavpView::IsTopWindowL") );

    TBool ret( EFalse );
	TInt numWg( iWsSession.NumWindowGroups() );
    CArrayFixFlat<TInt>* wgList = new (ELeave) CArrayFixFlat<TInt>( numWg );
    
    // Check if our window is front or not
    if ( iWsSession.WindowGroupList( 0, wgList ) == KErrNone )
        {
        ret = ( iWindowId == wgList->At(0) );
        }
    else
        {
        ret = EFalse;
        }

    delete wgList;
    return ret;
    }

// -----------------------------------------------------------------------------
// CBavpView::UpdateView
// Check animation running status, display the animated image if the video
// player is not playing
// -----------------------------------------------------------------------------
void CBavpView::UpdateView()
    {
	Log( EFalse, _L("CBavpView::UpdateView") );

  	// Handle the bad content case, ie MMF reports an error that needs to
   	// be displayed to the user. (loss of network or bad content)
	if ( iBavpController->State() == EBavpBadContent && !iIsBadContentAnimRunning )
	    {
    	Log( EFalse, _L("CBavpView::UpdateView - Bad Content") );		
		RunAnimation( iBadContentAnimation, iIsBadContentAnimRunning );
	    }
    // Handle the beffering content state
	else if ( iBavpController->State() == EBavpBuffering && !iIsVideoBuffAnimRunning )
		{
    	Log( EFalse, _L("CBavpView::UpdateView - Buffering media") );
        RunAnimation( iVideoBuffAnimation, iIsVideoBuffAnimRunning );
		}
    // Handle the video animations states
    else if ( iBavpController->IsClipVideo() )
        {
       	Log( EFalse, _L("CBavpView::UpdateView - We got Video") );
    	if ( iBavpController->State() == EBavpPlaying )
    		{
        	Log( EFalse, _L("CBavpView::UpdateView - Video Playing, stop animation") );
        	TBool flag( EFalse );
            RunAnimation( NULL, flag );
    		}
    	else if ( iBavpController->State() == EBavpStopped && !iIsVideoStoppedAnimRunning )
    		{
        	Log( EFalse, _L("CBavpView::UpdateView - Video Stopped") );
            RunAnimation( iVideoStoppedAnimation, iIsVideoStoppedAnimRunning );
    		}
    	else if ( iBavpController->State() == EBavpPaused && !iIsVideoPausedAnimRunning )
    		{
        	Log( EFalse, _L("CBavpView::UpdateView - Video Paused") );
            RunAnimation( iVideoPausedAnimation, iIsVideoPausedAnimRunning );
    		}
   		}
    // Handle the audio animations states
    else 
        {
       	Log( EFalse, _L("CBavpView::UpdateView - We got Audio") );
    	if ( iBavpController->State() == EBavpPlaying && !iIsAudioPlayAnimRunning )
    		{
        	Log( EFalse, _L("CBavpView::UpdateView - Audio Playing") );
            RunAnimation( iAudioPlayAnimation, iIsAudioPlayAnimRunning );
    		}
    	else if ( iBavpController->State() == EBavpStopped ||
    	          iBavpController->State() == EBavpPlayComplete &&
    	          !iIsAudioStoppedAnimRunning )
    	    {
        	Log( EFalse, _L("CBavpView::UpdateView - Audio Stopped") );
            RunAnimation( iAudioStoppedAnimation, iIsAudioStoppedAnimRunning );
    	    }
    	else if ( iBavpController->State() == EBavpPaused && !iIsAudioPausedAnimRunning )
    		{
        	Log( EFalse, _L("CBavpView::UpdateView - Audio Paused") );
            RunAnimation( iAudioPausedAnimation, iIsAudioPausedAnimRunning );
    		}
        }
    }

// -----------------------------------------------------------------------------
// CBavpView::FocusChanged
// This method is called once CCoeControl::SetFocus is called, could be when
// BavpPlugin::NotifyL causes SetFocus to be called
// -----------------------------------------------------------------------------
void CBavpView::FocusChanged( TDrawNow /*aDrawNow*/ )
    {
    iBavpController->BavpFocusChanged( IsFocused() );
    }

// -----------------------------------------------------------------------------
// CBavpView::Draw
// This method is called by CCoeControl::Draw
// -----------------------------------------------------------------------------
void CBavpView::Draw( const TRect& aRect ) const
    {
    
    if (iBavpController && !iBavpController->IsClipFullScreen()) {
    if ( Window().DisplayMode() == EColor16MA )
        {
        Log( EFalse, _L("BavpView::Draw() - DisplayMode=EColor16MA") );
        CWindowGc& gc = SystemGc();
        gc.SetDrawMode( CGraphicsContext::EDrawModeWriteAlpha );
#ifdef _DEBUG
        gc.SetBrushColor(TRgb::Color16MA(0x0000ff00));
#else        
        gc.SetBrushColor( TRgb::Color16MA(0) );
#endif        
        gc.Clear( aRect );
        }
    
        if (iBavpController->IsClipLocalMedia() && iBavpController->IsClipSeekable()) {
        if (iBavpController->State() == EBavpPlayComplete) {
          TRAP_IGNORE(iBavpController->setPositionL(iBavpController->Duration()));
        }
        else if (iBavpController->State() == EBavpPaused) {
          TRAP_IGNORE(iBavpController->setPositionL(iBavpController->getPositionL()));
        }
    }
        }        
        
    }

// -----------------------------------------------------------------------------
// CBavpView::ControllerStateChangedL
// Check animation running status and the display screen mode
// -----------------------------------------------------------------------------
void CBavpView::ControllerStateChangedL()
    {
	Log( EFalse, _L("CBavpView::ControllerStateChangedL") );
	
	// If not in full screen mode and if it is on the stack, then remove
	// from the stack
	if ( !iBavpController->IsClipFullScreen() && iIsOnStack )
		{
		iIsOnStack = EFalse;
		iEikonEnv->EikAppUi()->RemoveFromStack( this );
		}
	else if ( iBavpController->IsClipFullScreen() && !iIsOnStack )
		{
		// If full screen mode and not on stack, add to stack
		iIsOnStack = ETrue;
		iEikonEnv->EikAppUi()->AddToStackL( this );
		}
    }

// -----------------------------------------------------------------------------
// CBavpView::InsertOneMenuItemL
// Insert one menu item
// -----------------------------------------------------------------------------
void CBavpView::InsertOneMenuItemL( CEikMenuPane& aMenuPane, TInt aCommand,
                                    TInt aResourceId, TInt index )
    {
	Log( EFalse, _L("CBavpView::InsertOneMenuItemL") );

    CEikMenuPaneItem::SData item;
    HBufC* buf = StringLoader::LoadLC( aResourceId );
    item.iText.Copy( *buf );
    CleanupStack::PopAndDestroy();  // buf
    buf = NULL;

    item.iCommandId = aCommand + iCommandBase;
    item.iFlags = 0;
    item.iCascadeId = 0;
    aMenuPane.InsertMenuItemL( item, index );
    }

// -----------------------------------------------------------------------------
// CBavpView::RunAnimation
// Run the animation specified, and stop all the other animations
// If the animation and run flag are NULL, we stop all animations
// Check if animation is already running, if so don't stop, this causes flicker
// -----------------------------------------------------------------------------
void CBavpView::RunAnimation( CAknBitmapAnimation* aAnimation,
                              TBool &aAnimRunning )
    {
	Log( EFalse, _L("CBavpView::RunAnimation") );
	// Display the animation specified, after cancelling any
	// other animations first.
	//
    // Stop the video buffering animation
    if ( iIsVideoBuffAnimRunning && iVideoBuffAnimation != aAnimation )
		{
    	Log( EFalse, _L("CBavpView::RunAnimation - cancel VideoBuffAnimation") );
		iVideoBuffAnimation->CancelAnimation();
		iIsVideoBuffAnimRunning = EFalse;
		}
	else if ( iIsVideoStoppedAnimRunning && iVideoStoppedAnimation != aAnimation  )
	    {
		// Stop the video stopped animation
    	Log( EFalse, _L("CBavpView::RunAnimation - cancel VideoStoppedAnimation") );
		iVideoStoppedAnimation->CancelAnimation();
		iIsVideoStoppedAnimRunning = EFalse;
	    }
	else if ( iIsVideoPausedAnimRunning && iVideoPausedAnimation != aAnimation  )
	    {
		// Stop the video paused animation
    	Log( EFalse, _L("CBavpView::RunAnimation - cancel VideoPausedAnimation") );
		iVideoPausedAnimation->CancelAnimation();
		iIsVideoPausedAnimRunning = EFalse;
	    }
	else if ( iIsAudioPlayAnimRunning && iAudioPlayAnimation != aAnimation  )
		{
    	// Stop the audio playing animation
    	Log( EFalse, _L("CBavpView::RunAnimation - cancel AudioPlayAnimation") );
		iAudioPlayAnimation->CancelAnimation();
		iIsAudioPlayAnimRunning = EFalse;
		}
	else if ( iIsAudioStoppedAnimRunning && iAudioStoppedAnimation != aAnimation  )
	    {
		// Stop the audio stopped animation
    	Log( EFalse, _L("CBavpView::RunAnimation - cancel AudioStoppedAnimation") );
		iAudioStoppedAnimation->CancelAnimation();
		iIsAudioStoppedAnimRunning = EFalse;
	    }
	else if ( iIsAudioPausedAnimRunning && iAudioPausedAnimation != aAnimation  )
	    {
		// Stop the audio paused animation
    	Log( EFalse, _L("CBavpView::RunAnimation - cancel AudioPausedAnimation") );
		iAudioPausedAnimation->CancelAnimation();
		iIsAudioPausedAnimRunning = EFalse;
	    }
	else if ( iIsBadContentAnimRunning && iBadContentAnimation != aAnimation  )
	    {
		// Stop the bad content animation
    	Log( EFalse, _L("CBavpView::RunAnimation - cancel BadContentAnimation") );
		iBadContentAnimation->CancelAnimation();
		iIsBadContentAnimRunning = EFalse;
	    }
	    
	// Now, start the animation specified, if it isn't already running.
	// If the animation and run flag are NULL, we don't start any animations.
	if ( aAnimation && !aAnimRunning )
		{
    	Log( EFalse, _L("CBavpView::RunAnimation - start new animation") );
	    if ( aAnimation->Rect().Size() != iRect.Size() )
			{
        	TRect animRect( iWindowRect);
			aAnimation->SetRect( animRect );
			}
		TRAP_IGNORE
		(
		aAnimation->StartAnimationL();
		aAnimRunning = ETrue;
        );
		}

    }


void CBavpView::HandlePointerEventL(const TPointerEvent &aPointerEvent)
    {
    /*
     * Plugin is sending all pointer events to the browser.
     * Browser will process them to gesture and return back using
     * event() function. (see PluginWin::HandleGesture(), BavpEvent() and
     * CBavpPlugin::HandleGesture().
     * Browser expects event position to be in absolute co-ordinates,
     * so we convert position of the pointer event here.
     */
    TPoint point(aPointerEvent.iPosition  + PositionRelativeToScreen());
    TPointerEvent tmpEvent(aPointerEvent);
    tmpEvent.iPosition = point;
    
    NPNetscapeFuncs* funcs = iBavPlugin->getNPNFuncs();
    
    
    if(funcs && funcs->setvalue)
        {
        (funcs->setvalue)(iBavPlugin->getNPP(), 
                            (NPPVariable) NPPVPluginPointerEvent,
                            (void*) &(tmpEvent));
        }
    }

//  End of File
