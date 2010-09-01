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


// INCLUDE FILES
#include "SystemInfoMisc.h"
#include <HWRMLight.h>
#include <HWRMVibra.h>
#include <hal.h>
#include <coemain.h>
#include "SystemInfoBeep.h"

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CSystemInfoMisc::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
CSystemInfoMisc* CSystemInfoMisc::NewL()
    {
    CSystemInfoMisc* self = new (ELeave) CSystemInfoMisc();
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }


// -----------------------------------------------------------------------------
// CSystemInfoMisc::CSystemInfoMisc
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CSystemInfoMisc::CSystemInfoMisc()
    {
    }

// -----------------------------------------------------------------------------
// CSystemInfoMisc::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CSystemInfoMisc::ConstructL()
    {
    iLightControl = CHWRMLight::NewL();
    iVibraControl = CHWRMVibra::NewL();
    iBeeper = CSystemInfoBeep::NewL();
    // Create the widget registryclient session
    User::LeaveIfError( iWidgetRegistryClient.Connect() );
    iWidgetRegistryConnected = ETrue;
    }

// -----------------------------------------------------------------------------
// CSystemInfoMisc::~CSystemInfoMisc
// Destructor
// -----------------------------------------------------------------------------
CSystemInfoMisc::~CSystemInfoMisc()
    {
    delete iLightControl;
    delete iVibraControl;
    delete iBeeper;
    if ( iWidgetRegistryConnected )
        {
        iWidgetRegistryClient.Disconnect();
        }
    }

// -----------------------------------------------------------------------------
// CSystemInfoMisc::Beep
// -----------------------------------------------------------------------------
void CSystemInfoMisc::Beep(TInt aFrequency, TTimeIntervalMicroSeconds aDuration )
    {
    iBeeper->Play( aFrequency, aDuration );
    }

TInt CSystemInfoMisc::LightMinIntensity() const
    {
    return KHWRMLightMinIntensity;
    }

TInt CSystemInfoMisc::LightMaxIntensity() const
    {
    return KHWRMLightMaxIntensity;
    }
TInt CSystemInfoMisc::LightDefaultIntensity() const
    {
    return KHWRMDefaultIntensity;
    }
TInt CSystemInfoMisc::LightInfiniteDuration() const
    {
    return KHWRMInfiniteDuration;
    }
TInt CSystemInfoMisc::LightMaxDuration() const
    {
    return KHWRMLightMaxDuration;
    }
TInt CSystemInfoMisc::LightDefaultCycleTime() const
    {
    return KHWRMDefaultCycleTime;
    }
TInt CSystemInfoMisc::LightTargetPrimaryDisplayAndKeyboard() const
    {
    return CHWRMLight::EPrimaryDisplayAndKeyboard;
    }
TInt CSystemInfoMisc::LightTargetSystem() const
    {
    return CHWRMLight::ESystemTarget;
    }

void CSystemInfoMisc::LightOn( TInt aLightTarget, TInt aDuration, TInt aIntensity, TInt aFadeIn )
    {
    TRAP_IGNORE( iLightControl->LightOnL(aLightTarget, aDuration, aIntensity, aFadeIn) )
    }

void CSystemInfoMisc::LightBlink( TInt aLightTarget, TInt aDuration,
                                  TInt aOnDuration, TInt aOffDuration, TInt aIntensity )
    {
    TRAP_IGNORE( iLightControl->LightBlinkL(aLightTarget, aDuration,
                                            aOnDuration, aOffDuration, aIntensity) )
    }

void CSystemInfoMisc::LightOff( TInt aLightTarget, TInt aDuration, TInt aFadeOut )
    {
    TRAP_IGNORE( iLightControl->LightOffL(aLightTarget, aDuration, aFadeOut) )
    }


TInt CSystemInfoMisc::VibraMinIntensity() const
    {
    return KHWRMVibraMinIntensity;
    }

TInt CSystemInfoMisc::VibraMaxIntensity() const
    {
    return KHWRMVibraMaxIntensity;
    }

TInt CSystemInfoMisc::VibraMaxDuration() const
    {
    return KHWRMVibraMaxDuration;
    }

TInt CSystemInfoMisc::VibraSettings() const
    {
    TInt s = iVibraControl->VibraSettings();
    // explicitly map enum values in case Vibra API changes.
    switch ( s )
        {
    case CHWRMVibra::EVibraModeUnknown:
        s  = 0;
        break;
    case CHWRMVibra::EVibraModeON:
        s = 1;
        break;
    case CHWRMVibra::EVibraModeOFF:
        s = 2;
        break;
        }
    return s;
    }

void CSystemInfoMisc::StartVibra( TInt aDuration, TInt aIntensity )
    {
    TRAP_IGNORE( iVibraControl->StartVibraL( aDuration, aIntensity ) )
    }

void CSystemInfoMisc::StopVibra()
    {
    TRAP_IGNORE( iVibraControl->StopVibraL( ) )
    }


TInt CSystemInfoMisc::TotalRam()
    {
    TInt m = 0;
    (void) HAL::Get( HALData::EMemoryRAM, m );
    return m;
    }

TInt CSystemInfoMisc::FreeRam()
    {
    TInt m = 0;
    (void) HAL::Get( HALData::EMemoryRAMFree, m );
    return m;
    }

void CSystemInfoMisc::Language( TDes& aLangCode )
    {
    if ( iWidgetRegistryConnected )
        {
        iWidgetRegistryClient.GetLprojName( aLangCode );
        }
    else
        {
        aLangCode.Copy(_L(""));
        }
    }

// -----------------------------------------------------------------------------
// CSystemInfoPlugin::DriveList
//
// -----------------------------------------------------------------------------
void CSystemInfoMisc::DriveList( TDes8& aDriveList )
    {
    RFs& fs = CCoeEnv::Static()->FsSession();
    TDriveList driveList;
    TInt err = fs.DriveList( driveList );
    TInt effectiveDriveCount = 0;
    aDriveList.SetLength( 0 );

    for ( TInt i = 0; i < KMaxDrives && err == KErrNone; i++ )
        {
        if ( driveList[i] && i != EDriveD && i != EDriveZ )
            {
            TChar c;
            if ( fs.DriveToChar( i, c ) == KErrNone )
                {
                if ( effectiveDriveCount )
                    {
                    aDriveList.Append( ' ' );
                    }
                aDriveList.Append( (TUint)c );
                effectiveDriveCount++;
                }
            }
        }
    }

TReal CSystemInfoMisc::DriveSize( TChar c )
    {
    RFs& fs = CCoeEnv::Static()->FsSession();
    TInt drive;
    TVolumeInfo v;
    if ( fs.CharToDrive( c, drive ) == KErrNone )
        {
        if ( drive != EDriveD && drive != EDriveZ )
            {
            if ( !fs.Volume( v, drive ) )
              {
              TInt64 si = v.iSize;
              TReal sr = si;
              return sr;
              }
            }
        }
    return 0;
    }

TReal CSystemInfoMisc::DriveFree( TChar c )
    {
    RFs& fs = CCoeEnv::Static()->FsSession();
    TInt drive;
    TVolumeInfo v;
    if ( fs.CharToDrive( c, drive ) == KErrNone )
        {
        if ( drive != EDriveD && drive != EDriveZ )
            {
            if ( !fs.Volume( v, drive ) )
              {
              TInt64 fi = v.iFree;
              TReal fr = fi;
              return fr;
              }
            }
        }
    return 0;
    }


//  End of File
