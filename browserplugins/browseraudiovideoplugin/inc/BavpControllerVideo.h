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
* Description:  Controller class for handling browser requests to play video.
*
*/


#ifndef BAVPCONTROLLERVIDEO_H
#define BAVPCONTROLLERVIDEO_H

// INCLUDES
#include <browser_platform_variant.hrh>

#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF
#include <VideoPlayer2.h>
#else 
#include <VideoPlayer.h>
#endif
#include "BavpController.h"

// FORWARD DECLARATIONS
class MBavpView;
class CBavpViewFullScreen;


// CLASS DECLARATIONS
/**
*  CBavpController.
*  Controller class for handling browser requests to play video.
*  @lib npBrowserVideoPlugin.lib
*  @since 3.2
*/
class CBavpControllerVideo : public CBavpController,
                             public MVideoPlayerUtilityObserver,
                             public MVideoLoadingObserver,
                             public MBeating
    {
    public:    // Constructors and destructor

        /**
         * Two-phased constructor.
         */
        static CBavpControllerVideo* NewL( MBavpView* aView, 
                                           TUint aAccessPtId,
                                           TBavpMediaType aMediaType,
                                           const TDesC& aFileName );

        /**
         * Destructor.
         */
        ~CBavpControllerVideo();

    private:    // New methods

        /**
        * C++ default constructor.
        */
        CBavpControllerVideo( MBavpView* aView, TUint aAccessPtId );

        /**
        * By default Symbian 2nd phase constructor is private.
        * @param none
        * @return void
        */
        void ConstructL( TBavpMediaType aMediaType, const TDesC& aFileName );

        /**
        * Open a video URL link
        * @since 3.2
        * @param  none
        * @return void
        */
        void OpenVideoUrlL();

        /**
        * Opens the video file to be played.
        * @param none
        * @return void
        */
        void OpenVideoFileL();

        /**
        * Handle the leaving methods needed by MvpuoPrepareComplete
        * @param none
        * @return void
        */
        void PrepareCompleteL();
        
        /**
        * Non leaving version of initialize the video player plugin
        * @param none
        * @return void
        */
        void InitVideoPlayer();

        /**
        * Initialize the video player plugin
        * @param none
        * @return void
        */
        void InitVideoPlayerL();

        /**
        * Closes the player and delete the player instance
        * @param none
        * @return void
        */
        void UnInitVideoPlayer();

        /**
        * Rotate the screen if its height is bigger than width
        * @param none
        * @return void
        */
        void RotateScreen90();

        /**
        * Switch from normal screen to full screen display
        * @param none
        * @return void
        */
        void RevertToFullScreenL();

        /**
        * Switch from full screen to normal screen display
        * @param none
        * @return void
        */
        void RevertToNormalScreenL(); 

        /**
        * Switch between normal screen display and full screen display
        * @since 3.2
        * @param none
        * @return void
        */
        void ToggleScreen();

        /**
        * Refresh the coordinates of the rectangle to be drawn
        * @since 3.2
        * @param none
        * @return void
        */
        void RefreshRectCoordinatesL();

        /**
        * Switches between normal screen display and full screen display
        * @since 3.2
        * @param TAny* aThisData: the controller itself
        * @return TInt error code
        */
        static TInt WaitForScreenSwitch( TAny* aThisData );

        /**
        * wrapper function for WaitForScreenSwitch, that leaves
        * Switches between normal screen display and full screen display
        * @since 3.2
        * @param TAny* aThisData: the controller itself
        * @return TInt error code
        */
        static void WaitForScreenSwitchL( TAny* aThisData );

        /**
        * Fastforward the video file currently being played or paused
        * @since S60 v3.2
        * @param TAny* aThisData: the controller itself
        * @return TInt error code
        */
        static TInt DoFastForwardL( TAny* aThisData );

        /**
        * Rewind the video file currently being played or paused
        * @since S60 v3.2
        * @param TAny* aThisData: the controller itself
        * @return TInt error code
        */
        static TInt DoRewindL( TAny* aThisData );

    public:   // Methods overridden from base class CBavpController

        /**
        * Handles the error 
        * @param TInt aError: error to be handled
        * @return void
        */
        void HandleError( TInt aError );

        /**
        * Handles the keys offered by the CCoeControl
        * @since 3.2
        * @param TInt aKeyEvent: Which key pressed
        * @param TInt aEventCode: Key down, key up...
        * @return TKeyResponse: Was the key consumed
        */
        TKeyResponse HandleKeysL( const TKeyEvent& aKeyEvent, TEventCode aType );

        /**
        * Handles commands from Option Menu, defined in resource files
        * @since 3.2
        * @param TBavpCommandIds aCommand:  a Command Id
        * @return void
        */
        void HandleCommandL( TBavpCommandIds aCommand );
        
        TBool HandleGesture(RT_GestureHelper::TGestureEvent *gesture); 

    public: // From MBavpController

        /**
        * Handles request to play the audio or video content
        * @since 3.2
        * @param none
        * @return void
        */
        void PlayAudioVideoL();

        /**
        * Handles request to stop playing the content
        * @param none
        * @return void
        */
        void Stop();

        /**
        * Handles request to play content 
        * @param none
        * @return void
        */
        void PlayL();

        /**
        * Handles request to pause playing content
        * @param none
        * @return void
        */
        void PauseL();

        /**
        * Handles request to fast forward content
        * @param none
        * @return void
        */
        void FastForwardL();

        /**
        * Handles request to rewind content
        * @param none
        * @return void
        */
        void RewindL();

        /**
        * Set the volume in the player, we send volume value 0-10, this method
        * will scale before calling player's SetVolume()
        * @param aVolume the new volume: 0(mute) - 10(max)
        * @return void
        */
        void SetPlayerVolume( TInt aVolume );
        
        /**
        * Get the current volume of the player
        * This needs to be scaled to 0-10 (CR value) before returning
        * @param none
        * @return player volume scaled: 0(mute) - 10(max) 
        */
        TInt GetPlayerVolume();

        /**
        * To get the current position of the clip
        * @since 3.2
        * @param none
        * @return TTimeIntervalMicroSeconds
        */
        TTimeIntervalMicroSeconds getPositionL();

        /**
        * To set the position of the clip
        * @since 3.2
        * @param TTimeIntervalMicroSeconds
        * @return void
        */
        void setPositionL(TTimeIntervalMicroSeconds);
        
        /**
        * Handles the incoming call
        * @param none
        * @return void
        */
        void HandleInComingCallL();


    public: // Methods from MVideoPlayerUtilityObserver
    
        /**
        * Notification to the client that the opening of the video clip has completed, successfully, or otherwise.
        * with the system clock
        * @since 3.2
        * @param TInt aError: The status of the video sample after opening
        * @return void
        */
        void MvpuoOpenComplete( TInt aError );

        /**
        * Notification to the client that the opening of the video clip has been prepared 
        * successfully, or otherwise. This callback is called in response to a call to CVideoPlayerUtility::Prepare().
        * @since 3.2
        * @param TInt aError: The status of the video sample after opening
        * @return void
        */
        void MvpuoPrepareComplete( TInt aError );

        /**
        * Notification that the frame requested by a call to GetFrameL is ready
        * @since 3.2
        * @param TInt aError: The status of the video sample after opening
        * @return void
        */
        void MvpuoFrameReady( CFbsBitmap& /*aFrame*/, TInt /*aError*/ ) {};

        /**
        * Notification that video playback has completed. This is not called if playback is 
        * explicitly stopped by calling Stop.
        * @since 3.2
        * @param TInt aError: The status of the video sample after opening
        * @return void
        */
        void MvpuoPlayComplete( TInt aError );
        
        /**
        * General event notification from controller. These events are specified by the 
        * supplier of the controller
        * @since 3.2
        * @param TMMFEvent &aEvent:  multimedia framework event
        * @return void
        */
        void MvpuoEvent( const TMMFEvent &aEvent );

        /**
        * Notification that video clip loading/rebuffering has completed
        * @since 3.2
        * @param TMMFEvent &aEvent:  multimedia framework event
        * @return void
        */
        void MvloLoadingStarted();

        /**
        * Notification that video clip loading/rebuffering has started
        * @since 3.2
        * @param TMMFEvent &aEvent:  multimedia framework event
        * @return void
        */
        void MvloLoadingComplete();
          // MBeating
  public:
    /**
    * Handles a regular heartbeat timer event
    * @since S60 v3.2
    * @param none
    * @return void
    */
    void Beat();

    /**
    * Handles a heartbeat timer event where the timer completes out of synchronisation 
    * with the system clock
    * @since S60 v3.2
    * @param none
    * @return void
    */
    void Synchronize();

  private:
    /**
        * Handles the heart beat to retrieve the current playback position
    * @param none
    * @return void
        */
    void HandleScreenSaver();

    TBool NoScreenSaverMode();
    void SetDefaultAspectRatioL();
#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF
    /**
    * Handles a window rect update timer event where the timer completes the KWindowChangeUpdateTimeout 
    * with the system clock
    * @since S60 v5.2
    * @param none
    * @return void
    */
    void UpdateWindowSize();
#endif
    private:      // Data
    
        // Used to call MMF for video functionality
#ifdef BRDO_VIDEOPLAYER2_ENABLED_FF
    /** 
     * MMF Client API has updated with new methods to control video display
     *windows and video picture positioning to produce a new version of the API, 
     *CVideoPlayerUtility2. The new API is the preferred way to play video on graphics 
     *surfaces, and will support new features such as more flexible window positioning.  
     */
	CVideoPlayerUtility2*   iVideoPlayer;
#else   
	CVideoPlayerUtility*    iVideoPlayer;
#endif
    // Active object for display update
    CIdle*                  iDisplayUpdater;
    
    // The full screen view
    CBavpViewFullScreen*    iBavpViewFullScreen;
    //heart beat progress
    CHeartbeat*             iBackLightUpdater;
    TRect                   iNormalScreenRect;
    RWindow*                iActiveWindow; 
    };

#endif      // CBAVPCONTROLLERVIDEO_H

// End of File
