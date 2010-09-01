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
* Description:  Interface to Browser for handling embedded system info requests.
*
*/


// INCLUDE FILES
#include <e32std.h>
#include <npupp.h>
#include <string.h>
#include <utf.h>
#include <MProfileEngine.h>
#include <MProfile.h>
#include "SystemInfoPlugin.h"
#include "NetworkObserver.h"
#include "PowerObserver.h"
#include "SystemInfoMisc.h"

// CONSTANTS

enum TIdentifiersNames
    {
    EIdChargeLevel,
    EIdOnChargeLevel,
    EIdChargerConnected,
    EIdOnChargerConnected,
    EIdSignalBars,
    EIdNetworkName,
    EIdNetworkRegistrationStatus,
    EIdOnNetworkRegistrationStatus,
    EIdLightMinIntensity,
    EIdLightMaxIntensity,
    EIdLightDefaultIntensity,
    EIdLightInfiniteDuration,
    EIdLightMaxDuration,
    EIdLightDefaultCycleTime,
    EIdLightTargetPrimaryDisplayAndKeyboard,
    EIdLightTargetSystem,
    EIdLightOn,
    EIdLightBlink,
    EIdLightOff,
    EIdVibraMinIntensity,
    EIdVibraMaxIntensity,
    EIdVibraMaxDuration,
    EIdVibraSettings,
    EIdStartVibra,
    EIdStopVibra,
    EIdBeep,
    EIdTotalRam,
    EIdFreeRam,
    EIdDriveList,
    EIdDriveSize,
    EIdDriveFree,
    EIdLanguage,
    EIdLast
    };

const NPUTF8 *systemInfoPluginIdentifierNames[EIdLast] =
    {
    "chargelevel",
    "onchargelevel",
    "chargerconnected",
    "onchargerconnected",
    "signalbars",
    "networkname",
    "networkregistrationstatus",
    "onnetworkregistrationstatus",
    "lightminintensity",
    "lightmaxintensity",
    "lightdefaultintensity",
    "lightinfiniteduration",
    "lightmaxduration",
    "lightdefaultcycletime",
    "lighttargetprimarydisplayandkeyboard",
    "lighttargetsystem",
    "lighton",
    "lightblink",
    "lightoff",
    "vibraminintensity",
    "vibramaxintensity",
    "vibramaxduration",
    "vibrasettings",
    "startvibra",
    "stopvibra",
    "beep",
    "totalram",
    "freeram",
    "drivelist",
    "drivesize",
    "drivefree",
    "language"
    };

const TBool IsMethod[EIdLast] =
    {
    EFalse, // "chargelevel"
    EFalse, // "onchargelevel"
    EFalse, // "chargerconnected"
    EFalse, // "onchargerconnected"
    EFalse, // "signalbars"
    EFalse, // "networkname"
    EFalse, // "networkregistrationstatus"
    EFalse, // "onnetworkregistrationstatus"
    EFalse, // "lightminintensity"
    EFalse, // "lightmaxintensity"
    EFalse, // "lightdefaultintensity"
    EFalse, // "lightinfiniteduration"
    EFalse, // "lightmaxduration"
    EFalse, // "lightdefaultcycletime"
    EFalse, // "lighttargetprimarydisplayandkeyboard"
    EFalse, // "lighttargetsystem"
    ETrue, // "lighton"
    ETrue, // "lightblink"
    ETrue, // "lightoff"
    EFalse, // "vibraminintensity"
    EFalse, // "vibramaxintensity"
    EFalse, // "vibramaxduration"
    EFalse, // "vibrasettings"
    ETrue, // "startvibra"
    ETrue, // "stopvibra"
    ETrue, // "beep"
    EFalse, // "totalram"
    EFalse, // "freeram"
    EFalse, // "drivelist"
    ETrue, // "drivesize"
    ETrue, // "drivefree"
    EFalse  // "language"
    };


// ============================ MEMBER FUNCTIONS ===============================

// ----------------------------------------------------------------------------
// CSystemInfoPlugin::CSystemInfoPlugin
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------------------------
CSystemInfoPlugin::CSystemInfoPlugin()
    {
    }

// ----------------------------------------------------------------------------
// CSystemInfoPlugin::ConstructL
// Symbian 2nd phase constructor can leave.
// ----------------------------------------------------------------------------
void CSystemInfoPlugin::ConstructL()
    {
    iSystemInfoPluginIdentifiers = new (ELeave) NPIdentifier[EIdLast];
    NPN_GetStringIdentifiers( systemInfoPluginIdentifierNames, EIdLast,
                              iSystemInfoPluginIdentifiers );
    iUid = RProcess().SecureId();

    // need profile to determine silent mode for beep control
    iProfileEngine = CreateProfileEngineL();

    // sysinfo feature class instances
    iPowerObserver = CPowerObserver::NewL( this );
    iNetworkObserver = CNetworkObserver::NewL( this );
    iSystemInfoMisc = CSystemInfoMisc::NewL();
    }

// ----------------------------------------------------------------------------
// CSystemInfoPlugin::NewL
// Two-phased constructor.
// ----------------------------------------------------------------------------
CSystemInfoPlugin* CSystemInfoPlugin::NewL()

    {
    CSystemInfoPlugin* self = new (ELeave) CSystemInfoPlugin;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CSystemInfoPlugin::Destructor
// ----------------------------------------------------------------------------
CSystemInfoPlugin::~CSystemInfoPlugin()
    {
    delete [] iSystemInfoPluginIdentifiers;

    if (iProfileEngine != NULL)
        {
        iProfileEngine->Release();
        }
    // sysinfo feature class instances
    delete iPowerObserver;
    delete iNetworkObserver;
    delete iSystemInfoMisc;
    }

// ----------------------------------------------------------------------------
// CSystemInfoPlugin::Deallocate
//
// ----------------------------------------------------------------------------
void CSystemInfoPlugin::Deallocate()
    {
    }

// ----------------------------------------------------------------------------
// CSystemInfoPlugin::InvokeCallback
//
// ----------------------------------------------------------------------------
bool CSystemInfoPlugin::InvokeCallback( TDesC8& name, const NPVariant *args, uint32_t argCount )
    {
    CSystemInfoPluginEcomMain* lSystemInfoPPluginEcomMain
        = (CSystemInfoPluginEcomMain*) Dll::Tls();
    NPNetscapeFuncs* lNetscapeFuncs = lSystemInfoPPluginEcomMain->Funcs();
    if ( lNetscapeFuncs && iInstanceHandle )
        {
        void* value = 0;
        NPError npErr = lNetscapeFuncs->getvalue( iInstanceHandle, NPNVWindowNPObject,
                                                 (void*)&value );
        if ( npErr == NPERR_NO_ERROR )
            {
            NPVariant resultVariant;
            VOID_TO_NPVARIANT( resultVariant );
            NPIdentifier ident = NPN_GetStringIdentifier( (const NPUTF8 *)(name.Ptr()) );
            return NPN_Invoke( iInstanceHandle, (NPObject*)value, ident,
                               args, argCount, &resultVariant );
            }
        }
    return false;
    }

// ----------------------------------------------------------------------------
// CSystemInfoPlugin::HasMethod
//
// ----------------------------------------------------------------------------
bool CSystemInfoPlugin::HasMethod( NPIdentifier name )
    {
    for (TInt i= 0; i < EIdLast; i++)
        {
        if ( name == iSystemInfoPluginIdentifiers[i] )
            {
            return IsMethod[i];
            }
        }
    return false;
    }

// ----------------------------------------------------------------------------
// CSystemInfoPlugin::VariantToInt
//
// ----------------------------------------------------------------------------
void CSystemInfoPlugin::VariantToInt( NPVariant& v, TInt& i )
    {
    if ( NPVARIANT_IS_INT32( v ) )
        {
        i = NPVARIANT_TO_INT32( v );
        }
    else if ( NPVARIANT_IS_DOUBLE( v ) )
        {
        double d = NPVARIANT_TO_DOUBLE( v );
        i = d;
        }
    // leave unchanged if some other type tag on variant
    }

// ----------------------------------------------------------------------------
// CSystemInfoPlugin::Invoke
//
// ----------------------------------------------------------------------------
bool CSystemInfoPlugin::Invoke( NPIdentifier name,
                                NPVariant* args,
                                uint32_t argCount,
                                NPVariant *result )
    {
    bool ret = false;
    VOID_TO_NPVARIANT( *result );

    if ( name == iSystemInfoPluginIdentifiers[EIdBeep] )
        {
        // don't beep if profile is silent
        MProfile* profile = NULL;
        TRAPD( error,
               profile = iProfileEngine->ActiveProfileL() );
        if ( KErrNone != error )
            {
            return true;
            }
        if ( EFalse == profile->IsSilent() )
            {
            TInt f = 440; // default 440 hertz
            TInt d = 500; // default 0.5 seconds (in milliseconds)
            if ( argCount > 0 ) VariantToInt( args[0], f );
            if ( argCount > 1 ) VariantToInt( args[1], d );
            // convert duration from milliseconds to microseconds, the
            // underlying functionality doesn't really handle microseconds
            // even though the interface specs it as microseconds
            d = d * 1000;
            // Note that d = 0 seems to mean "forever". Also, there
            // are max and min frequencies but they are not defined.
            // Below min there is silence, above max the equation
            // seems to be f = min( max_freq, f)
            iSystemInfoMisc->Beep( f, d );
            }
        profile->Release();
        ret = true;
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdLightOn] )
        {
        TInt l = iSystemInfoMisc->LightTargetSystem();
        TInt d = iSystemInfoMisc->LightInfiniteDuration();
        TInt i = iSystemInfoMisc->LightDefaultIntensity();
        TInt f = 1;
        if ( argCount > 0 ) VariantToInt( args[0], l );
        if ( argCount > 1 ) VariantToInt( args[1], d );
        if ( argCount > 2 ) VariantToInt( args[2], i );
        if ( argCount > 3 ) VariantToInt( args[3], f );
        iSystemInfoMisc->LightOn( l, d, i, f );
        ret = true;
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdLightBlink] )
        {
        TInt l = iSystemInfoMisc->LightTargetSystem();
        TInt d = iSystemInfoMisc->LightInfiniteDuration();
        TInt don = iSystemInfoMisc->LightDefaultCycleTime();
        TInt doff = iSystemInfoMisc->LightDefaultCycleTime();
        TInt i = iSystemInfoMisc->LightDefaultIntensity();
        if ( argCount > 0 ) VariantToInt( args[0], l );
        if ( argCount > 1 ) VariantToInt( args[1], d );
        if ( argCount > 2 ) VariantToInt( args[2], don );
        if ( argCount > 3 ) VariantToInt( args[3], doff );
        if ( argCount > 4 ) VariantToInt( args[4], i );
        iSystemInfoMisc->LightBlink( l, d, don, doff, i );
        ret = true;
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdLightOff] )
        {
        TInt l = iSystemInfoMisc->LightTargetSystem();
        TInt d = iSystemInfoMisc->LightInfiniteDuration();
        TInt f = 1;
        if ( argCount > 0 ) VariantToInt( args[0], l );
        if ( argCount > 1 ) VariantToInt( args[1], d );
        if ( argCount > 2 ) VariantToInt( args[2], f );
        iSystemInfoMisc->LightOff( l, d, f );
        ret = true;
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdStartVibra] )
        {
        TInt d = iSystemInfoMisc->VibraMaxDuration();
        TInt i = iSystemInfoMisc->VibraMaxIntensity();
        if ( argCount > 0 ) VariantToInt( args[0], d );
        if ( argCount > 1 ) VariantToInt( args[1], i );
        iSystemInfoMisc->StartVibra( d, i );
        ret = true;
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdStopVibra] )
        {
        iSystemInfoMisc->StopVibra();
        ret = true;
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdDriveSize] )
        {
        TUint c = 0;
        if ( argCount > 0 && NPVARIANT_IS_STRING( args[0] ) )
            {
            NPString nps = NPVARIANT_TO_STRING( args[0] );
            if ( nps.UTF8Characters ) c = nps.UTF8Characters[0];
            }
        if ( c > 0 )
            {
            TChar drive( c );
            TReal size = iSystemInfoMisc->DriveSize( drive );
            DOUBLE_TO_NPVARIANT( size, *result );
            ret = true;
            }
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdDriveFree] )
        {
        TUint c = 0;
        if ( argCount > 0 && NPVARIANT_IS_STRING( args[0] ) )
            {
            NPString nps = NPVARIANT_TO_STRING( args[0] );
            if ( nps.UTF8Characters ) c = nps.UTF8Characters[0];
            }
        if ( c > 0 )
            {
            TChar drive( c );
            TReal free = iSystemInfoMisc->DriveFree( drive );
            DOUBLE_TO_NPVARIANT( free, *result );
            ret = true;
            }
        }

    return ret;
    }

// ----------------------------------------------------------------------------
// CSystemInfoPlugin::HasProperty
//
// ----------------------------------------------------------------------------
bool CSystemInfoPlugin::HasProperty (NPIdentifier name)
    {
    for ( TInt i= 0; i < EIdLast; i++ )
        {
        if ( name == iSystemInfoPluginIdentifiers[i] )
            {
            return !IsMethod[i];
            }
        }
    return false;
    }


// ----------------------------------------------------------------------------
// CSystemInfoPlugin::GetProperty
//
// ----------------------------------------------------------------------------
bool CSystemInfoPlugin::GetProperty( NPIdentifier name, NPVariant *variant )
    {
    // default variant value maps to javascript undefined
    VOID_TO_NPVARIANT( *variant );

    if ( name == iSystemInfoPluginIdentifiers[EIdChargeLevel] )
        {
        INT32_TO_NPVARIANT( iPowerObserver->ChargeLevel(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdChargerConnected] )
        {
        INT32_TO_NPVARIANT( iPowerObserver->ChargerConnected(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdSignalBars] )
        {
        INT32_TO_NPVARIANT( iNetworkObserver->SignalBars(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdNetworkRegistrationStatus] )
        {
        INT32_TO_NPVARIANT( iNetworkObserver->RegistrationStatus(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdNetworkName] )
        {
        const TDesC& inBuf = iNetworkObserver->NetworkName();
        HBufC8* outBuf = HBufC8::New( inBuf.Length() + 1 );
        if ( outBuf )
            {
            TPtr8 ptr( outBuf->Des() );
            TInt ret = CnvUtfConverter::ConvertFromUnicodeToUtf8( ptr, inBuf );
            DesToNpvariant(ptr, variant);
            delete outBuf;
            }
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdLightMinIntensity] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->LightMinIntensity(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdLightMaxIntensity] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->LightMaxIntensity(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdLightDefaultIntensity] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->LightDefaultIntensity(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdLightInfiniteDuration] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->LightInfiniteDuration(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdLightMaxDuration] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->LightMaxDuration(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdLightDefaultCycleTime] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->LightDefaultCycleTime(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdLightTargetPrimaryDisplayAndKeyboard] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->LightTargetPrimaryDisplayAndKeyboard(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdLightTargetSystem] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->LightTargetSystem(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdVibraMinIntensity] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->VibraMinIntensity(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdVibraMaxIntensity] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->VibraMaxIntensity(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdVibraMaxDuration] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->VibraMaxDuration(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdVibraSettings] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->VibraSettings(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdTotalRam] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->TotalRam(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdFreeRam] )
        {
        INT32_TO_NPVARIANT( iSystemInfoMisc->FreeRam(), *variant );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdDriveList] )
        {
        HBufC8* drives = HBufC8::New( KMaxDrives*2 );
        if ( drives )
            {
            TPtr8 ptr( drives->Des() );
            iSystemInfoMisc->DriveList( ptr );
            DesToNpvariant(ptr, variant);
            delete drives;
            }
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdLanguage] )
        {
        TBuf<64> langCode;
        iSystemInfoMisc->Language( langCode );
        HBufC8* outBuf = HBufC8::New( langCode.Length() + 1 );
        if ( outBuf )
            {
            TPtr8 ptr( outBuf->Des() );
            ptr.SetLength( 0 );
            // Note: the copy below goes from 16-bit to 8-bit by
            // dropping the high 8-bits.  This will work since all
            // codes are made from 7-bit ASCII characters see
            // http://www.rfc-editor.org/rfc/rfc4646.txt section 7
            // "Character Set Considerations".  Also see
            // http://www.w3.org/International/questions/qa-lang-2or3
            ptr.Copy( langCode );
            ptr.ZeroTerminate();
            DesToNpvariant( ptr, variant );
            delete outBuf;
            }
        }
    else
        {
        return EFalse;
        }
    return ETrue;
    }

// ----------------------------------------------------------------------------
// CSystemInfoPlugin::SetProperty
// ----------------------------------------------------------------------------
bool CSystemInfoPlugin::SetPropertyL( NPIdentifier name, NPVariant *variant )
    {
    // all writeable properties are names of callback functions
    HBufC8* pFunctionText = NULL;

    // anything but a string will result in nulling out the callback
    if ( NPVARIANT_IS_STRING( *variant ) )
        {
        // canonicalize function name string: only fname (no args) and zero terminate
        NPString str = NPVARIANT_TO_STRING( *variant );

        if ( (const TUint8 *)str.UTF8Characters )
            {
            // allocate a copy
            TInt length = str.UTF8Length;
            pFunctionText = HBufC8::NewL( length+1 );
            TPtr8 tptr = pFunctionText->Des();
            tptr.Copy( (const TUint8 *)str.UTF8Characters, length );

            //Remove any arguments passed in the with the function name
            TInt pos = pFunctionText->Locate(TChar('('));
            if ( pos != KErrNotFound )
                {
                length = pos;
                }
            tptr.SetLength(length);
            tptr.ZeroTerminate();
            }
        }

    // NOTE: pFunctionText can be NULL

    if ( name == iSystemInfoPluginIdentifiers[EIdOnChargeLevel] )
        {
        iPowerObserver->RequestNotificationL( CPowerObserver::EIdBatteryInfo,
                                              pFunctionText );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdOnChargerConnected] )
        {
        iPowerObserver->RequestNotificationL( CPowerObserver::EIdIndicator,
                                              pFunctionText );
        }
    else if ( name == iSystemInfoPluginIdentifiers[EIdOnNetworkRegistrationStatus] )
        {
        iNetworkObserver->RequestNotificationL( CNetworkObserver::EIdRegistrationStatus,
                                                pFunctionText );
        }
    else
        {
        delete pFunctionText;
        return EFalse;
        }
    return ETrue;
    }

void CSystemInfoPlugin::DesToNpvariant(TPtr8& string, NPVariant*& variant)
{
    char* newString = NULL;
    if (string.Length()) {
        newString = new char[string.Length()];
    }
    if (newString) {
        Mem::Copy(newString, string.Ptr(), string.Length());
        STRINGN_TO_NPVARIANT(newString, string.Length(), *variant);
    }
}



//  End of File
