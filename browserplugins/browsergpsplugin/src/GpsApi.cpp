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
* Description:  API functions that were loaded into TLS for the Browser to use.
*
*/


// INCLUDE FILES
#include <e32std.h>
#include <npupp.h>
#include <npscript.h>
#include "GpsPlugin.h"

// CONSTANTS

// ============================= LOCAL FUNCTIONS ===============================

// ----------------------------------------------------------------------------
// GpsNewp
// Create a new instance of a plugin. This is non-leaving method.
// Returns: NPError: Error codes recognized by Browser
// ----------------------------------------------------------------------------
NPError GpsNewp( NPMIMEType /*pluginType*/,  // Not used locally
                 NPP aInstance,              // Stores pointer to Gps
                 uint16 /*mode*/,            //
                 CDesCArray* argn,           // The number of arguments passed
                 CDesCArray* argv,           // The values of arguments in the array
                 NPSavedData* /*saved*/ )    //
    {
    // Create GpsPlugin, call this leaving method to wrap leaving methods
    TRAPD( err, GpsConstructL( aInstance, argn, argv ) );
    if ( err == KErrNoMemory )
        {
        return NPERR_OUT_OF_MEMORY_ERROR;
        }

    if ( err != KErrNone )
        {
        return NPERR_MODULE_LOAD_FAILED_ERROR;
        }

    return NPERR_NO_ERROR;
    }

// ----------------------------------------------------------------------------
// GpsConstructL
// This is the leaving method to create the plugin.  We have leaving setters
// that need to be wrapped.
// Returns: void
// ----------------------------------------------------------------------------
void GpsConstructL( NPP aInstance,             // Stores pointer to Gps
                    CDesCArray* /*argn*/,      // The number of arguments passed
                    CDesCArray* /*argv*/ )     // The values of arguments in the array
    {
    aInstance->pdata = CGpsPlugin::NewL();
    }

// ----------------------------------------------------------------------------
// GpsDestroy(NPP aInstance, NPSavedData**)
// Called by Browser to destroy the plugin
// Returns: NPError: Error Code
// ----------------------------------------------------------------------------
NPError GpsDestroy( NPP aInstance,             // Link to Browser
                    NPSavedData** /*save*/ )   // Not used locally
    {
    CGpsPlugin *pluginInstance = STATIC_CAST( CGpsPlugin*, aInstance->pdata );
    delete pluginInstance;
    return NPERR_NO_ERROR;
    }


// ----------------------------------------------------------------------------
// GpsGetvalue
// ----------------------------------------------------------------------------
//
NPError GpsGetvalue( NPP instance,
                     NPPVariable variable,
                     void* ret_value)
    {
    if (variable==NPPVpluginScriptableNPObject)
        {
        CGpsPlugin *siplugin = (CGpsPlugin*)instance->pdata;
        GpsPluginObject *pluginObject
            = (GpsPluginObject *)NPN_CreateObject (instance, GpsPluginClass);
        pluginObject->plugin = siplugin;
        siplugin->SetInstance(instance);
        void** ret = (void**)ret_value;
        *ret = (void*)pluginObject;
        }
    return NPERR_NO_ERROR;
    }


// ============================= LOCAL FUNCTIONS ===============================

// ----------------------------------------------------------------------------
// GpsPluginAllocate
// ----------------------------------------------------------------------------
//
NPObject* GpsPluginAllocate( NPP /*npp*/, NPClass* /*aClass*/ )
    {
    GpsPluginObject* newInstance
        = (GpsPluginObject*)User::Alloc( sizeof(GpsPluginObject) );
    return (NPObject*)newInstance;
    }

// ----------------------------------------------------------------------------
// GpsPluginDeallocate
// ----------------------------------------------------------------------------
//
void GpsPluginDeallocate( GpsPluginObject* obj )
    {
    obj->plugin->Deallocate();
    User::Free( (void*)obj );
    }

// ----------------------------------------------------------------------------
// GpsPluginInvalidate
// ----------------------------------------------------------------------------
//
void GpsPluginInvalidate( NPObject* /*obj*/ )
    {
    }

// ----------------------------------------------------------------------------
// GpsPluginHasMethod
// ----------------------------------------------------------------------------
//
bool GpsPluginHasMethod( GpsPluginObject* obj,
                         NPIdentifier name )
    {
    return obj->plugin->HasMethod( name );
    }

//-----------------------------------------------------------------------------
// GpsPluginInvoke
//-----------------------------------------------------------------------------
//
bool GpsPluginInvoke( GpsPluginObject* obj,
                      NPIdentifier name,
                      NPVariant* args,
                      uint32_t argCount,
                      NPVariant *result )
    {
    return obj->plugin->Invoke( name, args, argCount, result );
    }

// ----------------------------------------------------------------------------
// GpsPluginHasProperty
// ----------------------------------------------------------------------------
//
bool GpsPluginHasProperty( GpsPluginObject* obj,
                           NPIdentifier name )
    {
    return obj->plugin->HasProperty( name );
    }

// ----------------------------------------------------------------------------
// GpsPluginGetProperty
// ----------------------------------------------------------------------------
//
bool GpsPluginGetProperty( GpsPluginObject* obj,
                           NPIdentifier name,
                           NPVariant *variant )
    {
    return obj->plugin->GetProperty( name, variant );
    }

// ----------------------------------------------------------------------------
// GpsPluginSetProperty
// ----------------------------------------------------------------------------
//
bool GpsPluginSetProperty( GpsPluginObject* obj,
                           NPIdentifier name,
                           NPVariant *variant )
    {
    TBool r = EFalse;
    TRAPD( error, r = obj->plugin->SetPropertyL( name, variant ) );
    return (r || error)? true : false;
    }

// ----------------------------------------------------------------------------
// BapSetwindow
// This is the parent window of plugin
// Returns: NPError: Error Code
// ----------------------------------------------------------------------------
//
NPError GpsSetwindow(
    NPP /*aInstance*/,      // Link to Browser
    NPWindow* /*aWindow*/ )  // Browser's window passed to BAP
    {
    return NPERR_NO_ERROR;
    }

//  End of File
