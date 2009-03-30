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


#ifndef SYSTEMINFOBEEP_H
#define SYSTEMINFOBEEP_H

//  INCLUDES
#include <MdaAudioTonePlayer.h>

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
*  CSystemInfoBeep.
*  Interface to information about the radio network connection.
*  @lib npSystemInfoPlugin.lib
*  @since S60 v3.2
*/
NONSHARABLE_CLASS(CSystemInfoBeep) : public CBase, public MMdaAudioToneObserver
    {
public: // Constructors and destructor

    /**
     * Two-phased constructor.
     */
    static CSystemInfoBeep* NewL();

    /**
     * Destructor.
     */
    virtual ~CSystemInfoBeep();

public:
    /**
     * Play a pure tone.
     * @since 3.2
     * @param frequency of tone in Hertz
     * @param duration of tone in microseconds
     * @return void
     */
    void Play( TInt aFrequency, TTimeIntervalMicroSeconds iDuration);

private:
    /**
     * C++ default constructor.
     */
    CSystemInfoBeep();

    /**
     * By default Symbian 2nd phase constructor is private.
     * @param none
     * @return void
     */
    void ConstructL();

public:
    // from MMdaAudioToneObserver
    void MatoPrepareComplete(TInt aError);
    void MatoPlayComplete(TInt aError);

private:
    typedef enum TBeepState
        {
        EBeepIdle,
        EBeepPreparing,
        EBeepPlaying
        };

private:

    CMdaAudioToneUtility* iToneUtil; // owned
    TBeepState iState;
    };

#endif // SYSTEMINFOBEEP_H

// End of File
