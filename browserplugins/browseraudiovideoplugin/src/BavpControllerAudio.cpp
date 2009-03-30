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
#include <AudioPreference.h>        // For priority/pref values
#include <mmf/common/mmferrors.h>

#include "BavpLogger.h"
#include "BavpControllerAudio.h"
#include "BavpView.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBavpControllerAudio::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
CBavpControllerAudio* CBavpControllerAudio::NewL( MBavpView* aView,
                                                  TUint aAccessPtId,
                                                  TBavpMediaType aMediaType,
                                                  const TDesC& aFileName )
    {
    CBavpControllerAudio* self = new( ELeave ) CBavpControllerAudio( aView, aAccessPtId );

    Log( EFalse, _L("CBavpControllerAudio::NewL this="), (TInt)self );

    CleanupStack::PushL( self );
    self->ConstructL( aMediaType, aFileName );
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::~CBavpControllerAudio()
// Destructor
// -----------------------------------------------------------------------------
CBavpControllerAudio::~CBavpControllerAudio()
    {
    Log( EFalse, _L("CBavpControllerAudio::~CBavpControllerAudio") );

    // If currently playing, call Stop before you close and delete
    if ( iCurrentState == EBavpPlaying && iAudioPlayer != NULL)
        {
        iAudioPlayer->Stop();
        }
    if ( iAudioPlayer )
        {
        iAudioPlayer->Close();
        delete iAudioPlayer;
        }

    iFs.Close();
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::CBavpControllerAudio
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CBavpControllerAudio::CBavpControllerAudio( MBavpView* aView,
                                            TUint aAccessPtId )
    : CBavpController( aView, aAccessPtId )
    {
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CBavpControllerAudio::ConstructL( TBavpMediaType aMediaType,
                                       const TDesC& aFileName )
    {
    // Construct the BavpController base class
    BaseConstructL( aMediaType, aFileName );

    // Create Audio Player Client:
    iAudioPlayer = CMdaAudioPlayerUtility::NewL( *this, KAudioPriorityRealOnePlayer,
                        TMdaPriorityPreference( KAudioPrefRealOneLocalPlayback ) );
    
    // Connect to file server:
    User::LeaveIfError( iFs.Connect() );
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::OpenAudioUrlL
// Open the audio url link
// -----------------------------------------------------------------------------
void CBavpControllerAudio::OpenAudioUrlL()
    {
    Log( EFalse, _L("CBavpControllerAudio::OpenUrlL() ") );

    // Note: Play() is called from MapcInitComplete()
    iAudioPlayer->OpenUrlL( *iClipInfo->iUrl, iAccessPtId, KNullDesC8 );
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::OpenAudioFileL
// Open the audio file
// -----------------------------------------------------------------------------
void CBavpControllerAudio::OpenAudioFileL()
    {
    Log( EFalse, _L("CBavpControllerAudio::OpenFileL() ") );

    // Note: Play() is called from MapcInitComplete()
    iAudioPlayer->OpenFileL( *iClipInfo->iFileName );
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::PlayAudioVideoL
// Handles request to open and play an audio or video file
// TDesC& aFilename: Name of audio file, or video file, or URL link
// -----------------------------------------------------------------------------
void CBavpControllerAudio::PlayAudioVideoL()
    {
    Log( EFalse, _L("CBavpControllerAudio::OpenAndPlayL() ") );

    // If currently playing, call Stop before you attempt to open and play
    if ( iCurrentState == EBavpPlaying )
        {
        Stop();
        }

    OpenAudioFileL();
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::Stop
// Stops audio from playing, if playing.
// -----------------------------------------------------------------------------
void CBavpControllerAudio::Stop()
    {
    Log( EFalse, _L("CBavpControllerAudio::Stop") );

    iAudioPlayer->Stop();
    iClipInfo->iPosition = 0;
    iAudioPlayer->SetPosition( iClipInfo->iPosition );
    iCurrentState = EBavpStopped;

    iBavpView->UpdateView();
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::Play
// Non leaving method to play audio
// -----------------------------------------------------------------------------
void CBavpControllerAudio::Play()
    {
    // To keep LeaveScan from complaining everwhere, we use this non leaving
    // method
    TRAP_IGNORE( PlayL() );
    }
    
// -----------------------------------------------------------------------------
// CBavpControllerAudio::PlayL
// Plays audio
// -----------------------------------------------------------------------------
void CBavpControllerAudio::PlayL()
    {
    Log( EFalse, _L("CBavpControllerAudio::PlayL") );
    
    SetPlayerVolume( iCurrentVolume );

    // Handle multiple media object case
    if ( iBavpControllerArray.Count() > 1 && IsAnotherControllerPlaying() )
        {
        if ( iLastCommand == EBavpCmdPlay )
            {
            // Another controller is playing, and the user wants to play
            // this media object. Pause (or stop) all of the others and
            // play this one.
            PauseOtherControllersPlaying();
            }
        }

    iAudioPlayer->SetPosition( iClipInfo->iPosition );
    iAudioPlayer->Play();
    iCurrentState = EBavpPlaying;

    iBavpView->UpdateView();
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::Pause
// Non leaving method to pause playing audio
// -----------------------------------------------------------------------------
void CBavpControllerAudio::Pause()
    {
    // To keep LeaveScan from complaining everwhere, we use this non leaving
    // method
    TRAP_IGNORE( PauseL() );
    }
    
// -----------------------------------------------------------------------------
// CBavpControllerAudio::PauseL
// Pauses video while playing, if playing.
// -----------------------------------------------------------------------------
void CBavpControllerAudio::PauseL()
    {
    Log( EFalse, _L("CBavpControllerAudio::PauseL") );

    if ( IsClipSeekable() && 
        (iCurrentState == EBavpPlaying || iCurrentState == EBavpPaused))
        {
        // Save clip position, in case we want to resume playing
        iAudioPlayer->GetPosition(iClipInfo->iPosition);
        iAudioPlayer->Pause();
        iCurrentState = EBavpPaused;
        }
    else
        {
        // Not seekable content, i.e. Live stream
        iAudioPlayer->Stop();
        iClipInfo->iPosition = 0;
        iCurrentState = EBavpStopped;
        }

    iBavpView->UpdateView();
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::FastForwardL
// Fast forwards the audio
// -----------------------------------------------------------------------------
void CBavpControllerAudio::FastForwardL()
    {
    }
    
// -----------------------------------------------------------------------------
// CBavpControllerAudio::RewindL
// Rewinds the audio
// -----------------------------------------------------------------------------
void CBavpControllerAudio::RewindL()
    {
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::SetPlayerVolume
// Calculate the player volume from the CR volume (aka BavpController
// volume value) and sets it
// -----------------------------------------------------------------------------
void CBavpControllerAudio::SetPlayerVolume( TInt aVolume )
    {
    TInt playerVolume( 0 );
    if ( iPlayerInitialized )
        {
        TInt maxVolume = iAudioPlayer->MaxVolume();
        if (aVolume < 0)
          {
          playerVolume = 0;
          }
        else if (aVolume > maxVolume)
          {
          playerVolume = maxVolume;
          }
        else 
          {
          // The CR volume is 0-10, convert to Audio player volume 0-100
          playerVolume = ( aVolume * iPlayerMaxVolume ) / KCRVolumeMax;     
          }
    
        // Set Video player volume
        iAudioPlayer->SetVolume( playerVolume );
      }
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::GetPlayerVolume
// Calculate the player volume value and sets the playback volume for the audio 
// track of the video clip.
// -----------------------------------------------------------------------------
TInt CBavpControllerAudio::GetPlayerVolume()
    {
    TInt playerVolume( 0 );
    TInt CRVolume( 0 );
    
    if ( iPlayerInitialized )
        {
        // Audio player returns 0-100 
        iAudioPlayer->GetVolume( playerVolume );

        // Convert to CR volume 0-10
        CRVolume = ( playerVolume * KCRVolumeMax ) / iPlayerMaxVolume;
        }

    return CRVolume;
    }

// --------------------------------------------------------------------------
// CBavpControllerAudio::HandleError
// Handle error codes. We can receive errors from the MMF Video Player
// and its MMF callbacks (mvpuoPlayComplete, etc).
// The MMF errors are for handling content issues, such as decoding content
// and network access.
// NOTES:
// MMF errors start at -12000, see /epoc32/include/mmf/common/MMFErrors.h
// MMF also returns -1 (KErrNotFound) and few other system-wide errors
// -----------------------------------------------------------------------------
void CBavpControllerAudio::HandleError( TInt aError )
    {
    Log( EFalse, _L("In CBavpControllerAudioo::HandleError(): err="), aError );
    
    switch ( aError )
        {
        case KErrMMVideoDevice:
        case KErrMMAudioDevice:
            // We get this when the controllers can only support
            // playing one media at a time.
            HandleMultiInstanceError();
            break;

        default:
            iCurrentState = EBavpBadContent;
            iBavpView->UpdateView();
            break;
        }
    }
    
// -----------------------------------------------------------------------------
// CBavpControllerAudio::MapcInitComplete
// Required due to MMdaAudioPlayerCallback derivation.
// Called by client to indicate initialization has completed and device is ready
// to play.
// -----------------------------------------------------------------------------
void CBavpControllerAudio::MapcInitComplete( TInt aError,
                                const TTimeIntervalMicroSeconds& /*aDuration */ )
    {
    Log( EFalse, _L("MapcInitComplete aError="), aError );

    // Check for error
    if ( aError != KErrNone )
        {
        HandleError( aError );
        return;
        }

    // Audio player is initialized, so we can interact with it, volume,
    iPlayerInitialized = ETrue;
    
    iClipInfo->iDurationInSeconds = iAudioPlayer->Duration();
    iClipInfo->iPosition = 0;
    iAudioPlayer->SetPosition( iClipInfo->iPosition );
    
    // Get the max player volume, so we can scale volume
    iClipInfo->iMaxVolume = iAudioPlayer->MaxVolume();
    iPlayerMaxVolume = iClipInfo->iMaxVolume;

    // Handle multiple media object case
    if ( iBavpControllerArray.Count() > 1 && IsAnotherControllerPlaying() )
        {
        if ( iLastCommand == EBavpCmdUnknown )
            {
            // Another controller is playing, and this controller was
            // initializing. Because we can only play one media object at a
            // time, stop this media and let the first one
            // initialized continue to play.
            Stop();
            }
        else if ( iCurrentState == EBavpPlaying && iLastCommand == EBavpCmdPlay )
            {
            // Another controller is playing, and the user wants to play
            // another media object. Pause (or stop) all of the others and
            // play this one.
            PauseOtherControllersPlaying();
            Play();
            }
        }
    // Handle the single media object case
    else
        {
        Play();        
        }

    // Stop the buffering animation and play the audio only animation
    iBavpView->UpdateView();
    }

// -----------------------------------------------------------------------------
// CBavpControllerAudio::MapcPlayComplete
// Required due to MMdaAudioPlayerCallback derivation.
// Called by client to indicate audio file has completed or has stopped playing.
// -----------------------------------------------------------------------------
void CBavpControllerAudio::MapcPlayComplete( TInt aError )
    {
    Log( EFalse, _L("MapcPlayComplete aError="), aError );
    
    if ( iInfiniteLoopFlag )
        {
        // If infinite flag set: play audio again and again...
        iClipInfo->iPosition = 0;
        Play();
        }
    else if ( ( iLoopCount - 1 ) > 0 )
        {
        // If loop count: play, decrement loop count, play again...
        iClipInfo->iPosition = 0;
        Play();
        iLoopCount-- ;
        }
    else
        {
        // We're done looping, or just done playing the audio
        iCurrentState = EBavpPlayComplete;
        iClipInfo->iPosition = 0;
        iLoopCount = iInitLoopCount;

        iBavpView->UpdateView();
        }
    }
    
// -----------------------------------------------------------------------------
// CBavpControllerAudio::getPositionL
// To get the current position of the clip
// -----------------------------------------------------------------------------
    
 TTimeIntervalMicroSeconds CBavpControllerAudio::getPositionL() 
    {
        if ( iAudioPlayer )
        {
            TTimeIntervalMicroSeconds pos;
            iAudioPlayer->GetPosition( pos );
            return pos;
        }
        return NULL;
    }
    
 // -----------------------------------------------------------------------------
// CBavpControllerAudio::setPositionL
// To set the position of the clip
// -----------------------------------------------------------------------------
    
  void CBavpControllerAudio::setPositionL( TTimeIntervalMicroSeconds val )    
    {
        if ( iAudioPlayer )
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
                
            iAudioPlayer->SetPosition(val);
            iClipInfo->iPosition = val;
            if ( didWePause )
            {
                PlayL();
            }
        }
    }


//  End of File
