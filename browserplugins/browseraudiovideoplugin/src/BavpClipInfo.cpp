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


// INCLUDE FILES
#include "BavpClipInfo.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBavpClipInfo::CBavpClipInfo
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
 CBavpClipInfo::CBavpClipInfo()
    : iMediaType( EUnidentified ),
      iSeekable( ETrue ),
      iLiveStream( EFalse ),
      iVideoTrack( EFalse ),
      iInFullScreenMode( EFalse )
    {
    }

// -----------------------------------------------------------------------------
// CBavpClipInfo::~CBavpClipInfo   
// Destructor
// -----------------------------------------------------------------------------
CBavpClipInfo::~CBavpClipInfo()
    {
    // iFileName is created by controller, but we have ownership
	delete iFileName;
	
    // iUrl is created by controller, but we have ownership
	delete iUrl;
    }

// -----------------------------------------------------------------------------
// CBavpClipInfo::Reinit   
// Reinitialize all the values
// -----------------------------------------------------------------------------
void CBavpClipInfo::ReInit()
	{
    iSeekable = ETrue;
    iLiveStream = EFalse;
    iResolutionHeight = 0;
    iResolutionWidth = 0;
	iBitrate = 0;
	iMaxVolume = 0;
	iDurationInSeconds = 0;        
	iAudioTrack = EFalse;
	iVideoTrack = EFalse;
	iPosition = 0;
	// Don't modify iMediaType, iFilename, and iUrl
	}

//  End of File  
