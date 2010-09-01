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
* Description:  AO that completes on a change in the volume property.
*
*/


#ifndef BAVPVOLUMEHANDLER_H
#define BAVPVOLUMEHANDLER_H

#include <e32std.h>
#include <CenRepNotifyHandler.h>

// CONSTANTS
const TInt KGeneralProfile = 0;
const TInt KSilentProfile = 1;
const TInt KMeetingProfile = 2;

// The Central Depository volume values:
// 0(mute), 2, 5, 8, 10(maximum)
// Used to populate the initial volume and the volume dialog 
const TInt KCRVolume0 = 0;
const TInt KCRVolume2 = 2;
const TInt KCRVolume5 = 5;
const TInt KCRVolume8 = 8;
const TInt KCRVolume10 = 10;
const TInt KCRVolumeMute = KCRVolume0;
const TInt KCRVolumeMax = KCRVolume10;

// FORWARD DECLARATIONS
class MBavpVolumeObserver;
class CCenRepNotifyHandler;
class CRepository;

/**
*  CBavpVolumeHandler class for soft volume handling
*  @lib npBrowserVideoPlugin.lib
*  @since 3.2
*/
class CBavpVolumeHandler : public CBase,
						   public MCenRepNotifyHandlerCallback
	{
        public:
            // Constructors and destructor

        /**
        * Two-phased constructor.
        */
    	static CBavpVolumeHandler* NewL( MBavpVolumeObserver* aObserver );
    	
    	/**
        * Destructor.
        */
        ~CBavpVolumeHandler();
        
        /**
        * function impl for MCenRepNotifyHandlerCallback interface 
        * @since 3.1
        * @param TInt aVolume - volume value to be set to
        * @return void
        */
    	void HandleNotifyInt( const TUint32 aKeyId, TInt aNewValue );
        
    private:
        /**
        * C++ default constructor.
        */
    	CBavpVolumeHandler( MBavpVolumeObserver* aObserver );
    	    
    	/**
        * By default Symbian 2nd phase constructor is private.
        */
    	void ConstructL();
    	
    private:
        
        MBavpVolumeObserver* iObserver;

		// Central repository for Volume settings 
    	CRepository* iVolumeRepository;
  	
		// Handler to notify volume changes in central repository
    	CCenRepNotifyHandler* iVolumeCRHandler;

    	// Current volume settings
    	TInt iCurrentVolume;
    	
    	// Previous volume settings, save for mute/unmute
    	TInt iPreviousVolume;

		// Central repository for Profiles 
		CRepository*  iProfileRepository;

		// Handler to notify Profile changes in central repository
		CCenRepNotifyHandler* iProfileCRHandler;

    	// Current profile settings
    	TInt iCurrentProfile;
	};

#endif //BAVPVOLUMEHANDLER_H
