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
* Description:  Store the meta information and the state of the clip played
*
*/


#ifndef BAVPCLIPINFO_H
#define BAVPCLIPINFO_H

// INCLUDES
#include <e32base.h>

#include "BavpMediaRecognizer.h"

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  CBavpClipInfo 
*  Store the meta information and the state of the clip to be played
*  @lib npBrowserVideoPlugin.lib
*  @since 3.2
*/
NONSHARABLE_CLASS( CBavpClipInfo ) : public CBase
    {
    public:  // Constructors and destructor
        
        /**
        * C++ default constructor.
        */
        CBavpClipInfo();

        /**
        * Destructor.
        */
        ~CBavpClipInfo();

    public:  // New methods
    
         /**
         * ReInit()
         * Reinitialize the data members
         * @param none
         * @returns void
         */
         void ReInit();

    public:    // Data

		// Audio or Video file name
        HBufC* iFileName;

		// Audio or Video url link
        HBufC* iUrl;

		// Media type
		TBavpMediaType iMediaType;

		// Resolution for width
        TInt iResolutionWidth;

		// Resolution for height
        TInt iResolutionHeight;
        
		// Bitrate
		TInt iBitrate;

		// Maximum volume
        TInt iMaxVolume;

        // Duration of the clip 
        TTimeIntervalMicroSeconds iDurationInSeconds;        
        
        // If clip is seekable
		TBool iSeekable;

        // If clip is a live stream
        TBool iLiveStream;

        // Is audio track
		TBool iAudioTrack;
		
		// Is video track
        TBool iVideoTrack;

		// Is display in fullscreen mode
        TBool iInFullScreenMode;
		
        // Current file position
        TTimeIntervalMicroSeconds iPosition;
    };

#endif      // BAVPCLIPINFO_H  
            
// End of File
