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
* Description:  Handles the volume, gets the initial volume from central
* 				 repository and monitors changes in volume.
*
*/


#include <centralrepository.h>	
#include <ProfileEngineSDKCRKeys.h>
#include <browseruisdkcrkeys.h>

#include "BavpLogger.h"
#include "BavpVolumeHandler.h"
#include "BavpVolumeObserver.h"


// -----------------------------------------------------------------------------
// CBavpVolumeHandler::NewL
// Two-phased constructor.
// ----------------------------------------------------------------------------
CBavpVolumeHandler* CBavpVolumeHandler::NewL( MBavpVolumeObserver* aObserver )
    {
	Log( EFalse, _L("CBavpVolumeHandler::NewL") );

    CBavpVolumeHandler* self = new (ELeave) CBavpVolumeHandler( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }
	
// ----------------------------------------------------------------------------
// CBavpVolumeHandler::CBavpVolumeHandler
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------------------------
CBavpVolumeHandler::CBavpVolumeHandler( MBavpVolumeObserver* aObserver )
	: iObserver( aObserver )
    {
    }

// -----------------------------------------------------------------------------
// CBavpVolumeHandler::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CBavpVolumeHandler::ConstructL()
    {		
	Log( EFalse, _L("CBavpVolumeHandler::ConstructL") );
	
	iCurrentVolume = KCRVolume5;
	iCurrentProfile = KGeneralProfile;
	
	// Get volume settings from central repository
	iVolumeRepository = CRepository::NewL( KCRUidBrowser );
    iVolumeRepository->Get( KBrowserMediaVolumeControl, iCurrentVolume );

    // If media volume key exists, watch it for changes
    iVolumeCRHandler = CCenRepNotifyHandler::NewL( *this, *iVolumeRepository, 
	    					    			       CCenRepNotifyHandler::EIntKey, 
											      (TUint32)KBrowserMediaVolumeControl);  
    iVolumeCRHandler->StartListeningL();
    iPreviousVolume = iCurrentVolume;

    // Get the current profile
	iProfileRepository = CRepository::NewL( KCRUidProfileEngine );
    iProfileRepository->Get( KProEngActiveProfile, iCurrentProfile );

    // Watch the Profile changes using a CR Notify Handler
    iProfileCRHandler = CCenRepNotifyHandler::NewL( *this, *iProfileRepository,
                                                    CCenRepNotifyHandler::EIntKey,
                                                    (TUint32)KProEngActiveProfile );
    iProfileCRHandler->StartListeningL();

    // Now that we have initial volume and profile, lets use them.
    // Check if we're in Silent profile or Meeting profile.
    if ( iCurrentProfile == KSilentProfile ||
         iCurrentProfile == KMeetingProfile )
        {
        // Mute the volume, we already saved the CR volume
        // as iPreviousVolume
        iCurrentVolume = KCRVolumeMute;
        }

    // Tell observer (BavpController) the initial volume
    if ( iObserver )
        {
        iObserver->UpdateVolume( iCurrentVolume ); 
        }
	}

// ----------------------------------------------------------------------------
// CBavpVolumeHandler::~CBavpVolumeHandler
//  Destructor
// ----------------------------------------------------------------------------
CBavpVolumeHandler::~CBavpVolumeHandler()
    {
	Log( EFalse, _L("CBavpVolumeHandler::~CBavpVolumeHandler") );

    // Clean up the media volume CR and CR Handler
	if ( iVolumeCRHandler )
        {
        iVolumeCRHandler->StopListening();
        delete iVolumeCRHandler;
        }
        
	delete iVolumeRepository;

    // Clean up the Profile CR and CR Handler
    if ( iProfileCRHandler )
        {
        iProfileCRHandler->StopListening();
        delete iProfileCRHandler;
        }
 
    delete iProfileRepository;

    }

// ----------------------------------------------------------------------------
// CBavpVolumeHandler::HandleNotifyInt()
// Handles volume and profile changes in CR
// ----------------------------------------------------------------------------
void CBavpVolumeHandler::HandleNotifyInt( const TUint32 aKeyId, TInt aNewValue )
    {
    if ( aKeyId == KBrowserMediaVolumeControl && aNewValue != iCurrentVolume )
        {
        // Media Volume property updated, and it changed
        if ( iCurrentProfile == KSilentProfile ||
             iCurrentProfile == KMeetingProfile )
            {
            // We got a new volume, and Profile is "quiet", save and use later
            iPreviousVolume = aNewValue;
            }
        else
            {
            // We got a new volume and Profile allows us to use it
            iCurrentVolume = aNewValue;
            }
        }
    else if ( aKeyId == KProEngActiveProfile && aNewValue != iCurrentProfile )
        {
        // Profile property updated, and it changed
        iCurrentProfile = aNewValue;

        // If we got a change in profile, mute or unmute        
        if ( iCurrentProfile == KSilentProfile ||
             iCurrentProfile == KMeetingProfile )
                {
                // Mute the volume
                iPreviousVolume = iCurrentVolume;
                iCurrentVolume = KCRVolumeMute;
                }
            else // Unmute the volume
                {
                iCurrentVolume = iPreviousVolume;
                }
            }

    // Tell observer the new volume
    if ( iObserver )
        {
        iObserver->UpdateVolume( iCurrentVolume ); 
        }
    }

// End of file
