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
* Description:  Base Controller class for handling browser requests to play
*                audio or video.
*
*/


#ifndef BAVPCONTROLLER_H
#define BAVPCONTROLLER_H

// INCLUDES
#include <e32base.h>
#include <coecntrl.h>
#include <e32property.h>                // RProperty class
#include "FeatMgr.h"

#include "BrowserAudioVideoPlugin.hrh"
#include "BavpVolumeObserver.h"         // Volume Observer class
#include "BavpVolumeHandler.h"
#include "BavpHwKeyEvents.h"            // Hardware Key Events
#include "BavpClipInfo.h"               // Clip info
#include <rt_gesturehelper.h>

// FORWARD DECLARATIONS
class RFs;
class CBavpPlugin;
class MBavpView;

// CONSTANTS
// Does this build include Side Volume Keys support
#define HAS_HW_VOLUME_KEYS ( FeatureManager::FeatureSupported( KFeatureIdSideVolumeKeys ) )

// Used to walk the content's meta data
_LIT(KBavpTrue, "1");
_LIT(KBavpFalse, "0");
_LIT(KBavpLiveStream, "LiveStream");
_LIT(KBavpSeekable, "Seekable");

// ROP controller UID, ?? Helix UID ??
const TUid KRopControllerUid =
    {
    0x101F8514
    };

// The Audio or Video player state
enum TAudioVideoState
    {
    EBavpNone,                  // Player is idle or not created yet
    EBavpBuffering,             // Loading the content
    EBavpPlaying,               // Playing the content
    EBavpPlayComplete,          // Done playing the content (end)
    EBavpStopped,               // User stopped the content
    EBavpPaused,                // User paused the content
    EBavpBadContent,            // Bad content or network error encountered
    EBavpFastForwarding,        // Fast Forwarding the content
    EBavpFastForwardComplete,   // Fast Forward reached end of content
    EBavpRewinding,             // Rewinding the content
    EBavpRewindComplete         // Rewind reached beginning of content
    };

// Use these values for fast-forwarding and rewinding
const TInt KSkipToPosition = 2*1000*1000;   // 2 seconds
const TInt KUpdateFrequency = 400*1000;     // 400 milliseconds
const TInt KInitialDelay = 600*1000;        // 600 milliseconds

// CLASS DECLARATIONS

/**
*  CBavpController.
*  Controller class for handling browser requests to play audio or video.
*  @lib npBrowserAudioVideoPlugin.lib
*  @since 3.2
*/
class CBavpController : public CActive,
                        public MBavpVolumeObserver,
                        public MBavpHwKeyEventsObserver,
                        public MCenRepNotifyHandlerCallback
    {
    public:    // Constructors and destructor

        /**
        * C++ default constructor.
        */
        CBavpController( MBavpView* aView, TUint aAccessPtId );

        /**
        * Destructor.
        */
        virtual ~CBavpController();

    public: // New pure virtual methods

        /**
        * Handles request to play the audio or video content
        * @since 3.2
        * @param none
        * @return void
        */
        virtual void PlayAudioVideoL() = 0;

        /**
        * Handles request to stop playing the content
        * @param none
        * @return void
        */
        virtual void Stop() = 0;

        /**
        * Handles request to play content
        * @param none
        * @return void
        */
        virtual void PlayL() = 0;

        /**
        * Handles request to pause playing content
        * @param none
        * @return void
        */
        virtual void PauseL() = 0;

        /**
        * Handles request to fast forward content
        * @param none
        * @return void
        */
        virtual void FastForwardL() = 0;

        /**
        * Handles request to rewind content
        * @param none
        * @return void
        */
        virtual void RewindL() = 0;

        /**
        * Set the volume in the player, we send volume value 0-10, this method
        * will scale before calling Player::SetVolume()
        * @param aVolume the new volume: 0(mute) - 10(max)
        * @return void
        */
        virtual void SetPlayerVolume( TInt aVolume ) = 0;

        /**
        * Get the current volume of the player
        * This needs to be scaled to 0-10 (CR value) before returning
        * @param none
        * @return player volume scaled: 0(mute) - 10(max)
        */
        virtual TInt GetPlayerVolume() = 0;

        /**
        * To get the current position of the clip
        * @since 3.2
        * @param none
        * @return TTimeIntervalMicroSeconds
        */
        virtual TTimeIntervalMicroSeconds getPositionL() = 0;

        /**
        * To set the position of the clip
        * @since 3.2
        * @param TTimeIntervalMicroSeconds
        * @return void
        */
        virtual void setPositionL(TTimeIntervalMicroSeconds) = 0;
        
        /**
        * Handles the incoming call
        * @param none
        * @return void
        */
        virtual void HandleInComingCallL() = 0;

    public: // New virtual methods

        /**
        * Handles Notifications from Browser
        * @since 3.2
        * @param TBool: Tell bavp plugin if Browser is in or out of focus
        * @return void
        */
        virtual void HandleBrowserNotification( TBool aBrowserFocus );

        /**
        * Handles the error
        * @param TInt aError: error to be handled
        * @return void
        */
        virtual void HandleError( TInt aError );

        /**
        * Handles the keys offered by the CCoeControl
        * @since 3.2
        * @param TInt aKeyEvent: Which key pressed
        * @param TInt aEventCode: Key down, key up...
        * @return TKeyResponse: Was the key consumed
        */
        virtual TKeyResponse HandleKeysL( const TKeyEvent& aKeyEvent,
                                          TEventCode aType );

        /**
        * Handles commands from Option Menu, defined in resource files
        * @since 3.2
        * @param TBavpCommandIds aCommand:  a Command Id
        * @return void
        */
        virtual void HandleCommandL( TBavpCommandIds aCommand );

        /**
        * Called by CBavpView to CBavpPlugin whenever Bavp focus changes
        * @since 3.2
        * @param aFocus True if plugin has focus
        * @return none
        */
        virtual void BavpFocusChanged( TBool aFocus );
        
        virtual TBool HandleGesture(RT_GestureHelper::TGestureEvent *gesture); 

    public: // New methods

        /**
        * Sets volume using the value from <object> attribute
        * @since 3.2
        * @param aVolume
        * @return void
        */
        void SetVolumeFromAttribute( TInt aVolume );

        /**
        * Sets loop using the value from <object> attribute
        * @since 3.2
        * @param aLoopValue: The number of times to play file
        * @param aInfiniteFlag: Set to ETrue if play infinitely, we use a loop
        *        countValue = 50 for infinite and always set this to EFalse
        * @return void
        */
        void SetLoopFromAttribute( TInt aLoopValue, TBool aInfiniteFlag );

        /**
        * Refresh the coordinates of the rectangle to be drawn
        * @since 3.2
        * @param none
        * @return void
        */
        virtual void RefreshRectCoordinatesL();

        /**
        * Sets original file name (used for scripting functionality)
        * @since 3.2
        * @param originalFilename
        * @return void
        */
        void SetOriginalFileName( const TDesC* originalFileName );


        /**
        * Sets the mime type(used for scripting functionality)
        * @since 3.2
        * @param mime type
        * @return void
        */
        void SetMimeType( const TDesC8* mimetype );

    public: // New inline methods

        /**
        * Bavp has focus
        * @since 3.2
        * @param void
        * @return : TBool 0 or 1 indicating the focus
        */
        inline TBool BavpHasFocus()
            {
            return iBavpHasFocus;
            }

        /**
        * Returns the video player plugin current state
        * @since 3.2
        * @param none
        * @return TVideoState
        */
        inline TAudioVideoState State()
            {
            return iCurrentState;
            }

        /**
        * returns ETrue is the volume is muted
        * @param none
        * @return TBool
        */
        inline TBool IsVolumeMuted()
            {
            return ( iCurrentVolume == KCRVolumeMute );
            }

        /**
        * Returns ETrue is this is a video clip
        * @since 3.2
        * @param none
        * @return TBool
        */
        inline TBool IsClipVideo()
            {
            return iClipInfo->iVideoTrack;
            }

        /**
        * Returns ETrue is the clip is seekable.
        * @param none
        * @return TBool
        */
        inline TBool IsClipSeekable()
           {
           return iClipInfo->iSeekable;
           }

        /**
        * returns ETrue is the content is local media file.
        * Local files can be FF and Rewind
        * @param none
        * @return TBool
        */
        inline TBool IsClipLocalMedia()
            {
            return ( iClipInfo->iMediaType == ELocalVideoFile ||
                     iClipInfo->iMediaType == ELocalAudioFile );
            }

        /**
        * Returns TRUE if the display is on full screen, otherwise FALSE
        * @since 3.2
        * @param none
        * @return TBool
        */
        inline TBool IsClipFullScreen()
            {
            return iClipInfo->iInFullScreenMode;
            }

        /**
        * Returns TRUE if the content is being fast-forwarded,
        * otherwise FALSE
        * @since 3.2
        * @param none
        * @return TBool
        */
        inline TBool IsClipFastForwarding()
            {
            return ( iCurrentState == EBavpFastForwarding );
            }

        /**
        * Returns TRUE if the content is being rewound,
        * otherwise FALSE
        * @since 3.2
        * @param none
        * @return TBool
        */
        inline TBool IsClipRewinding()
            {
            return ( iCurrentState == EBavpRewinding );
            }

        /**
        * Returns ETrue is the clip is onDemand stream.
        * @param none
        * @return TBool
        */
        inline TBool IsClipOnDemand()
            {
            return ( iClipInfo->iMediaType != ELocalVideoFile && iClipInfo->iSeekable );
            }

        /**
        * Returns ETrue is the clip is live stream.
        * @param none
        * @return TBool
        */
        inline TBool IsClipLive()
            {
            return ( iClipInfo->iLiveStream );
            }


        /** Returns the original filename of the clip
        * @param none
        * @return HBufC*
        */
        inline const HBufC& ClipName()
            {
            return *iOriginalFileName;
            }

        /** Returns the duration of the clip
        * @param none
        * @return TTimeIntervalMicroSeconds
        */
        inline TTimeIntervalMicroSeconds Duration()
            {
            return ( iClipInfo->iDurationInSeconds );
            }

        /** Returns the mime type of the clip
        * @param none
        * @return HBufC*
        */
        inline const HBufC8& MimeType()
            {
            return *iMimeType;
            }



    protected:  // New methods

        /**
        * By default Symbian 2nd phase constructor is private.
        * @param none
        * @return void
        */
        void BaseConstructL( TBavpMediaType aMediaType,
                             const TDesC& aFileName );

        /**
        * Parse the ram file
        * @param none
        * @return void
        */
        void ParseRamFileL();
#if defined(BRDO_ASX_FF) 
    /**
	* Parse the asx file
	* @param none
	* @return void
	*/
        void ParseAsxFileL();
#endif //BRDO_ASX_FF
        /**
        * Set the rectangle to prepare the display
        * @param TRect
        * @return TRect aRect
        */
        TRect GetClipRect( TRect aRect );

        /**
        * Create a query dialog for the volume setting list
        * @param none
        * @return void
        */
        void CreateVolumeListDlgL();

        /**
        * Check if an incoming call comes
        * @param none
        * @return TBool
        */
        TBool IsVideoOrAudioCall();

        /**
        * Check if other audio or video controllers are playing
        * @param none
        * @return TBool
        */
        TBool IsAnotherControllerPlaying();

        /**
        * Pause or Stop the other audio or video controllers from playing
        * @param none
        * @return void
        */
        void PauseOtherControllersPlaying();

        /**
        * Handle the error from MMF for multiple media object case
        * @param aError error code from MMF
        * @return void
        */
        void HandleMultiInstanceError();

    protected:  // Methods from CActive

        /**
        * Implements cancellation of an outstanding request.
        * @param none
        * @return void
        */
        void DoCancel() { /* Not implemented */ }

        /**
        * Handles an active object’s request completion event.
        * @param none
        * @return void
        */
        void RunL();

    public: // Methods from MBavpVolumeObserver

        /**
        * Updates the volume
        * From MBavpVolumeObserver
        * @since 3.2
        * @param TInt aVolume: volume value
        * @return void
        */
        void UpdateVolume( TInt aVolume );

    private:       // From MHwKeyEventsObserver

        /**
        * Called when volume level should be changed.
        * Set volume control visible into navi pane.
        * @param aVolumeChange: +1 change volume up
        *                       -1 change volume down
        */
        void BavpHwKeyVolumeChange( TInt aVolumeChange );

        /**
        * Called when a media hardware key (play, pause...) was presed.
        * @since 3.2
        * @param aCommand: Command ID representing Play, Pause, ...
        * @return void
        */
        // void BavpHwKeyCommand( TBavpCommandIds aCommand );

    public:         // Data, public

        // Is Bavp Plugin in focus
        TBool iBavpHasFocus;

        // Audio Video plugin current state
        TAudioVideoState iCurrentState;

        // ClipInfo contains our metadata from the MMF/Helix and content file
        CBavpClipInfo* iClipInfo;

        // Array of pointers of CBavpController (this) instances
        static RPointerArray<CBavpController> iBavpControllerArray;

    protected:      // Data

        // The player is initialized and ready to be used
        TBool iPlayerInitialized;

        // Bavp plugin previous state - used when handling interrupted
        // media, because of a phone call
        TAudioVideoState iPreCallState;

        // Last command
        TBavpCommandIds iLastCommand;

        // The number of times the audio or video file will play (loop)
        TInt iLoopCount;

        // Hold the initial value of loop
        TInt iInitLoopCount;

        // The infinite loop flag, we currently set to EFalse and use loop=50
        // if a web site requests infinite looping
        TBool iInfiniteLoopFlag;

        // Access pt id
        TUint iAccessPtId;

        // Bavp View
        MBavpView* iBavpView;

        // Events from Hardware Keys
        CBavpHwKeyEvents* iHwKeyEvents;

        // System state
        RProperty iIncomingCalls;

        // Volume hander
        CBavpVolumeHandler* iBavpVolumeHandler;

        // Current volume level
        TInt iCurrentVolume;

        // Audio or Video player max volume
        TInt iPlayerMaxVolume;

        // Timer to jump to a new position, used for Fast Forward, Rewind
        CPeriodic* iPositionUpdater;

        // original file name. added for scripting functionality
        HBufC* iOriginalFileName;

        // mime type. added for scripting functionality
        HBufC8* iMimeType;

        //Normal display mode screen rect
        TRect iNormalScreenRect;
    };

#endif      // CBAVPCONTROLLER_H

// End of File
