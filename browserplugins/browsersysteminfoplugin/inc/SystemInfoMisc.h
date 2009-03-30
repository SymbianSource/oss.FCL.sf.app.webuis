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


#ifndef SYSTEMINFOMISC_H
#define SYSTEMINFOMISC_H

// INCLUDES
#include <e32base.h>
#include <npscript.h>
#include <HWRMVibra.h>
#include <WidgetRegistryClient.h>

// FORWARD DECLARATIONS
class CHWRMLight;
class CHWRMVibra;
class CSystemInfoBeep;

// CONSTANTS

// State enum and member variable

// CLASS DECLARATIONS
/**
*  CSystemInfoMisc.
*  Interface to various platform services not requiring observers.
*  @lib npSystemInfoPlugin.lib
*  @since S60 v3.2
*/
NONSHARABLE_CLASS(CSystemInfoMisc) : public CBase
    {
public:
    /**
     * Two-phased constructor.
     */
    static CSystemInfoMisc* NewL();

    /**
     * Destructor.
     */
    virtual ~CSystemInfoMisc();

public:
    /**
     * Called to sound a tone.
     * @since 3.2
     * @param void
     * @return charge level
     */
    void Beep( TInt aFrequency, TTimeIntervalMicroSeconds aDuration );


    TInt VibraMinIntensity() const;

    TInt VibraMaxIntensity() const;

    TInt VibraMaxDuration() const;

    TInt VibraSettings() const;  // TBD rename VibraProfileEnabled a "boolean"

    void StartVibra( TInt aDuration, TInt aIntensity );

    void StopVibra();


    TInt LightMinIntensity() const;

    TInt LightMaxIntensity() const;

    TInt LightDefaultIntensity() const;

    TInt LightInfiniteDuration() const;

    TInt LightMaxDuration() const;

    TInt LightDefaultCycleTime() const;

    TInt LightTargetPrimaryDisplayAndKeyboard() const;

    TInt LightTargetSystem() const;

    void LightOn( TInt aLightTarget, TInt aDuration, TInt aIntensity, TInt aFadeIn );

    void LightBlink( TInt aLightTarget, TInt aDuration,
                     TInt aOnDuration, TInt aOffDuration, TInt aIntensity );

    void LightOff( TInt aLightTarget, TInt aDuration, TInt aFadeOut );


    TInt TotalRam();

    TInt FreeRam();


    void Language( TDes& aLangCode );


    void DriveList( TDes8& aDriveList );

    TReal DriveSize( TChar c );

    TReal DriveFree( TChar c );

private:
    /**
     * C++ default constructor.
     */
    CSystemInfoMisc();

    /**
     * By default Symbian 2nd phase constructor is private.
     * @param none
     * @return void
     */
    void ConstructL();

    CHWRMLight* iLightControl;
    CHWRMVibra* iVibraControl;
    CSystemInfoBeep* iBeeper;
    RWidgetRegistryClientSession iWidgetRegistryClient;
    TBool iWidgetRegistryConnected; // connected to Widget Registry server ?
   };

#endif // SYSTEMINFOMISC_H

// End of File
