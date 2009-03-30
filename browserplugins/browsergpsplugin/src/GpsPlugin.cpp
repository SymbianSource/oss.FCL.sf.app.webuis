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
* Description:  Interface to Browser for handling embedded Gps requests.
*
*/


// INCLUDE FILES
#include <e32std.h>
#include <npupp.h>
#include <E32HAL.H>
#include "GpsPlugin.h"
#include "GpsObserver.h"
#include <CSatelliteInfoUI.h>


// CONSTANTS

enum TIdentifiersNames {
    EIdLatitude,
    EIdLongitude,
    EIdAlt,
    EIdHorizontalAccuracy,
    EIdVerticalAccuracy,
    EIdOnGps,
    EIdSatelliteStatus,
    EIdTickCount,
    EIdLast
    };

const NPUTF8 *GpsPluginIdentifierNames[EIdLast]
= { "latitude",              // 0
    "longitude",             // 1
    "altitude",              // 2
    "horizontalaccuracy",    // 3
    "verticalaccuracy",      // 4
    "ongps",                 // 5
    "showsatellitestatus",   // 6
    "tickcount",             // 7
  };

const TBool IsMethod[] = {
    EFalse, // latitude,
    EFalse, // longitude,
    EFalse, // altitude
    EFalse, // horizontalaccuracy
    EFalse, // verticalaccuracy
    EFalse, // ongps
    ETrue,  // showSatelliteStatus
    EFalse, // tickcount
    };


// ============================ MEMBER FUNCTIONS ===============================

// ----------------------------------------------------------------------------
// CGpsPlugin::CGpsPlugin
// C++ default constructor can NOT contain any code, that
// might leave.
// ----------------------------------------------------------------------------
CGpsPlugin::CGpsPlugin()
    {
    }

// ----------------------------------------------------------------------------
// CGpsPlugin::NewL
// Two-phased constructor.
// ----------------------------------------------------------------------------
CGpsPlugin* CGpsPlugin::NewL()
    {
    CGpsPlugin* self = new (ELeave) CGpsPlugin;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// ----------------------------------------------------------------------------
// CGpsPlugin::ConstructL
// Symbian 2nd phase constructor can leave.
// ----------------------------------------------------------------------------
void CGpsPlugin::ConstructL()
    {
    iGpsPluginIdentifiers = new (ELeave) NPIdentifier[EIdLast];
    NPN_GetStringIdentifiers( GpsPluginIdentifierNames, EIdLast,
                              iGpsPluginIdentifiers );

    iGpsObserver = CGpsObserver::NewL( this );
    }

// ----------------------------------------------------------------------------
// CGpsPlugin::Destructor
// ----------------------------------------------------------------------------
CGpsPlugin::~CGpsPlugin()
    {
    delete [] iGpsPluginIdentifiers;
    delete iGpsObserver;
    }

// ----------------------------------------------------------------------------
// CGpsPlugin::Deallocate
// ----------------------------------------------------------------------------
void CGpsPlugin::Deallocate ()
    {
    }

// ----------------------------------------------------------------------------
// CGpsPlugin::InvokeCallback
// ----------------------------------------------------------------------------
bool CGpsPlugin::InvokeCallback( TDesC8& name,
                                 const NPVariant *args, uint32_t argCount )
    {
    CGpsPluginEcomMain* lGpsPPluginEcomMain = (CGpsPluginEcomMain*) Dll::Tls();
    NPNetscapeFuncs* lNetscapeFuncs = lGpsPPluginEcomMain->Funcs();
    if (lNetscapeFuncs && iInstanceHandle)
        {
        void* value = 0;
        NPError npErr
          = lNetscapeFuncs->getvalue( iInstanceHandle, NPNVWindowNPObject,
                                      (void*)&value );
        if (npErr == NPERR_NO_ERROR)
            {
            NPVariant resultVariant;
            VOID_TO_NPVARIANT( resultVariant );
            NPIdentifier ident
              = NPN_GetStringIdentifier( (const NPUTF8 *)(name.Ptr()) );
            return NPN_Invoke( iInstanceHandle, (NPObject*)value,
                               ident, args, argCount, &resultVariant );
            }
        }
    return false;
    }

// ----------------------------------------------------------------------------
// CWidgetManagerPlugin::HasMethod
// ----------------------------------------------------------------------------

bool CGpsPlugin::HasMethod( NPIdentifier name )
    {
    for ( TInt i= 0; i < EIdLast; i++ )
        {
        if ( name == iGpsPluginIdentifiers[i] )
            {
            return IsMethod[i];
            }
        }
    return false;
    }

// ----------------------------------------------------------------------------
// CGpsPlugin::HasProperty
// ----------------------------------------------------------------------------
bool CGpsPlugin::HasProperty( NPIdentifier name )
    {
    for ( TInt i= 0; i < EIdLast; i++ )
        {
        if ( name == iGpsPluginIdentifiers[i] )
            {
            return !IsMethod[i];
            }
        }
    return false;
    }

// ----------------------------------------------------------------------------
// CGpsPlugin::GetProperty
// ----------------------------------------------------------------------------
bool CGpsPlugin::GetProperty( NPIdentifier name, NPVariant *variant )
    {
    // default variant value maps to javascript undefined
    VOID_TO_NPVARIANT( *variant );
    if ( name == iGpsPluginIdentifiers[EIdLatitude] )
        {
        DOUBLE_TO_NPVARIANT( iGpsObserver->Latitude(), *variant );
        }
    else if ( name == iGpsPluginIdentifiers[EIdLongitude] )
        {
        DOUBLE_TO_NPVARIANT( iGpsObserver->Longitude(), *variant );
        }
    else if ( name == iGpsPluginIdentifiers[EIdAlt] )
        {
        DOUBLE_TO_NPVARIANT( iGpsObserver->Altitude(), *variant );
        }
    else if ( name == iGpsPluginIdentifiers[EIdHorizontalAccuracy] )
        {
        DOUBLE_TO_NPVARIANT( iGpsObserver->HorizontalAccuracy(), *variant );
        }
    else if ( name == iGpsPluginIdentifiers[EIdVerticalAccuracy] )
        {
        DOUBLE_TO_NPVARIANT( iGpsObserver->VerticalAccuracy(), *variant );
        }
    else if ( name == iGpsPluginIdentifiers[EIdTickCount] )
        {
        INT32_TO_NPVARIANT( iGpsObserver->TickCount(), *variant );
        }
    else
        {
        return  EFalse;
        }
    return ETrue;
    }

// ----------------------------------------------------------------------------
// CGpsPlugin::Invoke
// ----------------------------------------------------------------------------
bool CGpsPlugin::Invoke( NPIdentifier name,
                         NPVariant* /*args*/,
                         uint32_t /*argCount*/,
                         NPVariant *result )
    {
    VOID_TO_NPVARIANT( *result );
    if ( name == iGpsPluginIdentifiers[EIdSatelliteStatus] )
        {
        TRAP_IGNORE(
            CSatelliteInfoUI* satelliteUI = CSatelliteInfoUI::NewL();
            (void) satelliteUI->ExecuteLD( _L("BrowserGpsPlugin") ) );
        }
    return true;
    }

// ----------------------------------------------------------------------------
// CGpsPlugin::SetPropertyL
// ----------------------------------------------------------------------------
bool CGpsPlugin::SetPropertyL( NPIdentifier name, NPVariant *variant )
    {
    if ( name == iGpsPluginIdentifiers[EIdOnGps] )
        {
        NPString str = NPVARIANT_TO_STRING( *variant );
        iGpsObserver->RequestNotificationL(str);
        }
    else
        {
        return EFalse;
        }
    return ETrue;
    }

// ----------------------------------------------------------------------------
// CBAPPlugin::SetWindowL
// Called by Browser for window creation.
// (other items were commented in a header).
// ----------------------------------------------------------------------------
void CGpsPlugin::SetWindowL(
    MPluginAdapter* /*aPluginAdapter*/,  // Allows callback to Browser
    const TRect& /*aRect*/)              // Window information
    {
    }
//  End of File
