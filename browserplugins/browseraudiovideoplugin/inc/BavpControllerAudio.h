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
* Description:  Controller class for handling browser requests to play audio.
*
*/


#ifndef BAVPCONTROLLERAUDIO_H
#define BAVPCONTROLLERAUDIO_H

// INCLUDES
#include <MdaAudioSamplePlayer.h>   // for MMdaAudioPlayerCallback
#include "BavpController.h"

// FORWARD DECLARATIONS

// CONSTANTS

// CLASS DECLARATIONS
/**
*  CBavpControllerAudio.
*  Controller class for handling browser requests to play audio.
*  @lib npBrowserAudioVideoPlugin.lib
*  @since 3.2
*/
class CBavpControllerAudio : public CBavpController,
                             public MMdaAudioPlayerCallback 
    {
    public:    // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBavpControllerAudio* NewL( MBavpView* aView, 
                                           TUint aAccessPtId,
                                           TBavpMediaType aMediaType,
                                           const TDesC& aFileName );

        /**
        * Destructor.
        */
        ~CBavpControllerAudio();

    public: // New methods

        /*
        * Non Leaving method to handle request to play content.
        * @param none
        * @return void
        */
        void Play(); 

        /**
        * Non Leaving method to handle request to pause playing content
        * @param none
        * @return void
        */
        void Pause(); 

    private:    // New methods

        /**
        * C++ default constructor.
        */
        CBavpControllerAudio( MBavpView* aView, TUint aAccessPtId );

        /**
        * By default Symbian 2nd phase constructor is private.
        * @param none
        * @return void
        */
        void ConstructL( TBavpMediaType aMediaType, const TDesC& aFileName );

        /**
        * Open an URL link
        * @since 3.2
        * @param  none
        * @return void
        */
        void OpenAudioUrlL();

        /**
        * Opens the file to be played.
        * @param none
        * @return void
        */
        void OpenAudioFileL();

    public: // Methods from MBavpController

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
        void HandleInComingCallL() { }//not implemented


    public:   // Methods overridden from base class CBavpController

        /**
        * Handles the error 
        * @param TInt aError: error to be handled
        * @return void
        */
        void HandleError( TInt aError );

    public: // Methods from MMdaAudioPlayerCallback

        /**
        * From MMdaAudioPlayerCallback.
        * @since 3.2
        * @param TInt aError Error code returned from call to Init
        * @param const TTimeIntervalMicroSeconds& aDuration, duration time
        * @return void
        */
        void MapcInitComplete( TInt aError,
                               const TTimeIntervalMicroSeconds& aDuration );

        /**
        * From MMdaAudioPlayerCallback.
        * @since 3.2
        * @param TInt aError: Error code returned from call to Play
        * @return void
        */
        void MapcPlayComplete( TInt aError );

    private:      // Data

        // Used to call MMF for audio functionality
        CMdaAudioPlayerUtility* iAudioPlayer;
        
        // File Server
        RFs iFs;
    };

#endif      // CBAVPCONTROLLERAUDIO_H

// End of File
