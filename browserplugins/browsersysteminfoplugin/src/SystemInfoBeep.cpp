/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
* Description: 
*
*/


//  INCLUDES
#include "SystemInfoBeep.h"

// -----------------------------------------------------------------------------
// CSystemInfoBeep::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
CSystemInfoBeep* CSystemInfoBeep::NewL()
    {
    CSystemInfoBeep* self = new (ELeave) CSystemInfoBeep();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
    };

// -----------------------------------------------------------------------------
// CSystemInfoBeep::CSystemInfoBeep
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CSystemInfoBeep::CSystemInfoBeep()
    : iState( EBeepIdle )
    {
    }

// -----------------------------------------------------------------------------
// CSystemInfoBeep::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CSystemInfoBeep::ConstructL()
    {
    iToneUtil = CMdaAudioToneUtility::NewL( *this );
    }

// -----------------------------------------------------------------------------
// CSystemInfoBeep::~CSystemInfoBeep
// Destructor
// -----------------------------------------------------------------------------
CSystemInfoBeep::~CSystemInfoBeep()
    {
    delete iToneUtil;
    }

// -----------------------------------------------------------------------------
// CSystemInfoBeep::Play
// -----------------------------------------------------------------------------
void CSystemInfoBeep::Play( TInt aFrequency, TTimeIntervalMicroSeconds aDuration )
    {
    if ( iState == EBeepPreparing )
        {
        iToneUtil->CancelPrepare();
        }
    else if ( iState == EBeepPlaying )
        {
        iToneUtil->CancelPlay();
        }
    iState = EBeepPreparing;
    iToneUtil->PrepareToPlayTone( aFrequency, aDuration );
    }

// -----------------------------------------------------------------------------
// CSystemInfoBeep::MatoPrepareComplete
// -----------------------------------------------------------------------------
void CSystemInfoBeep::MatoPrepareComplete( TInt aError )
    {
    if ( aError == KErrNone )
        {
        iState = EBeepPlaying;
        iToneUtil->Play();
        }
    else
        {
        iState = EBeepIdle;
        }
    }

// -----------------------------------------------------------------------------
// CSystemInfoBeep::MatoPlayComplete
// -----------------------------------------------------------------------------
void CSystemInfoBeep::MatoPlayComplete( TInt /*aError*/ )
    {
    iState = EBeepIdle;
    }
