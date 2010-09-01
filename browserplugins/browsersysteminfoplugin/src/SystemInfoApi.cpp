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
* Description:  API functions that were loaded into TLS for the Browser to use.
*
*/


// INCLUDE FILES
#include <e32std.h>
#include <npapi.h>
#include <npscript.h>
#include "SystemInfoPlugin.h"

// CONSTANTS

// ============================= LOCAL FUNCTIONS ===============================

// ----------------------------------------------------------------------------
// SystemInfoNewp
// Create a new instance of a plugin. This is non-leaving method.
// Returns: NPError: Error codes recognized by Browser
// ----------------------------------------------------------------------------
NPError SystemInfoNewp( NPMIMEType /*pluginType*/,  // Not used locally
                  NPP aInstance,            // Stores pointer to SystemInfo
                  uint16 /*mode*/,          //
                  CDesCArray* argn,         // The number of arguments passed
                  CDesCArray* argv,         // The values of arguments in the array
                  NPSavedData* /*saved*/ )  //
    {
    // Create SystemInfoPlugin, call this leaving method to wrap leaving methods
    TRAPD( err, SystemInfoConstructL( aInstance, argn, argv ) );
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
// SystemInfoConstructL
// This is the leaving method to create the plugin.  We have leaving setters
// that need to be wrapped.
// Returns: void
// ----------------------------------------------------------------------------
void SystemInfoConstructL( NPP aInstance,               // Stores pointer to SystemInfo
                     CDesCArray* /*argn*/,      // The number of arguments passed
                     CDesCArray* /*argv*/ )     // The values of arguments in the array
    {
    aInstance->pdata = CSystemInfoPlugin::NewL();
    }

// ----------------------------------------------------------------------------
// SystemInfoDestroy(NPP aInstance, NPSavedData**)
// Called by Browser to destroy the plugin
// Returns: NPError: Error Code
// ----------------------------------------------------------------------------
NPError SystemInfoDestroy( NPP aInstance,               // Link to Browser
                     NPSavedData** /*save*/ )   // Not used locally
    {
    CSystemInfoPlugin *pluginInstance = STATIC_CAST( CSystemInfoPlugin*, aInstance->pdata );
    delete pluginInstance;

    return NPERR_NO_ERROR;
    }


// ----------------------------------------------------------------------------
// SystemInfoGetvalue
// ----------------------------------------------------------------------------
//
NPError SystemInfoGetvalue(
    NPP instance,
    NPPVariable variable,
    void* ret_value)
    {
    if (variable==NPPVpluginScriptableNPObject)
        {
        CSystemInfoPlugin *siplugin = (CSystemInfoPlugin*)instance->pdata;
        SystemInfoPluginObject *pluginObject = (SystemInfoPluginObject *)NPN_CreateObject (instance, systemInfoPluginClass);
        pluginObject->plugin = siplugin;
        siplugin->SetInstance(instance);
        void** ret = (void**)ret_value;
        *ret = (void*)pluginObject;
        }
    return NPERR_NO_ERROR;
    }


// ============================ LOCAL FUNCTIONS ===============================

// ----------------------------------------------------------------------------
// systemInfoPluginAllocate
// ----------------------------------------------------------------------------
//
NPObject* systemInfoPluginAllocate( NPP /*npp*/, NPClass* /*aClass*/ )
    {
    SystemInfoPluginObject *newInstance =
        (SystemInfoPluginObject*)User::Alloc( sizeof(SystemInfoPluginObject) );
    return (NPObject*)newInstance;
    }

// ----------------------------------------------------------------------------
// systemInfoPluginDeallocate
// ----------------------------------------------------------------------------
//
void systemInfoPluginDeallocate( SystemInfoPluginObject* obj)
    {
    obj->plugin->Deallocate();
    User::Free( (void*)obj );
    }

// ----------------------------------------------------------------------------
// systemInfoPluginInvalidate
// ----------------------------------------------------------------------------
//
void systemInfoPluginInvalidate( NPObject* /*obj*/ )
    {
    }

// ----------------------------------------------------------------------------
// systemInfoPluginHasMethod
// ----------------------------------------------------------------------------
//
bool systemInfoPluginHasMethod( SystemInfoPluginObject* obj,
                                NPIdentifier name )
    {
    return obj->plugin->HasMethod( name );
    }

// ----------------------------------------------------------------------------
// systemInfoPluginInvokeFunction
// ----------------------------------------------------------------------------
//
bool systemInfoPluginInvokeFunction( SystemInfoPluginObject* obj,
                                     NPIdentifier name,
                                     NPVariant* args,
                                     uint32_t argCount,
                                     NPVariant* result)
    {
    return obj->plugin->Invoke( name, args, argCount, result );
    }

// ----------------------------------------------------------------------------
// systemInfoPluginHasProperty
// ----------------------------------------------------------------------------
//
bool systemInfoPluginHasProperty( SystemInfoPluginObject* obj,
                                  NPIdentifier name )
    {
    return obj->plugin->HasProperty( name );
    }

// ----------------------------------------------------------------------------
// systemInfoPluginGetProperty
// ----------------------------------------------------------------------------
//
bool systemInfoPluginGetProperty( SystemInfoPluginObject* obj,
                                  NPIdentifier name,
                                  NPVariant *variant )
    {
    return obj->plugin->GetProperty( name, variant );
    }

// ----------------------------------------------------------------------------
// systemInfoPluginSetProperty
// ----------------------------------------------------------------------------
//
bool systemInfoPluginSetProperty( SystemInfoPluginObject* obj,
                                  NPIdentifier name,
                                  NPVariant *variant )
    {
    TBool r = EFalse;
    TRAPD( error, r = obj->plugin->SetPropertyL( name, variant ) );
    return (error || !r)? false : true;
    }

//  End of File
