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


#ifndef SYSTEMINFOPLUGIN_H
#define SYSTEMINFOPLUGIN_H

//  INCLUDES
#include <CEcomBrowserPluginInterface.h>
#include <npscript.h>


// FORWARD DECLARATIONS
class CPowerObserver;
class CNetworkObserver;
class CSystemInfoMisc;
class MProfileEngine;

// CLASS DECLARATION

/******************************************
This class is specific to ECom Style Plugin.  This is used for passing
plugin specific initialization information to and from browser.
*******************************************/
IMPORT_C NPError InitializeFuncs( NPPluginFuncs* aPpf );

// CONSTANTS
const TInt KFirstSystemInfoImplementationValue = 0x10282854;

/**
*  CSystemInfoPluginEcomMain
*  Used for passing plugin specific initialization information to and from browser.
*  @lib npSystemInfoPlugin.lib
*  @since S60 v3.2
*/
NONSHARABLE_CLASS(CSystemInfoPluginEcomMain) : public CEcomBrowserPluginInterface
    {
    public: // Functions from base classes

        /**
        * Two-phased constructor.
        */
        static CSystemInfoPluginEcomMain* NewL( TAny* aInitParam );

        /**
        * Destructor
        */
        virtual ~CSystemInfoPluginEcomMain();

        /**
        * Passed from the browser to the plugin and delcares what
        * functions the browser makes available
        */
        NPNetscapeFuncs* Funcs() const { return iNpf; }

    private:

        // Default constructor
        CSystemInfoPluginEcomMain( NPNetscapeFuncs* aNpf );

        // By default Symbian 2nd phase constructor is private.
        void ConstructL( NPPluginFuncs* aPluginFuncs );

        // Netscape Function Pointer
        NPNetscapeFuncs* iNpf;
    };

/**
*  CSystemInfoPlugin class
*  Interfaces to Browser, used to pass information between browser and plugin
*  @lib npSystemInfoPlugin.lib
*  @since S60 v3.2
*/
NONSHARABLE_CLASS(CSystemInfoPlugin) : public CBase
    {
 public:
    static CSystemInfoPlugin* NewL();

    virtual ~CSystemInfoPlugin();

 public:
    void Deallocate ();
    bool HasMethod( NPIdentifier name );
    bool Invoke( NPIdentifier name,
                 NPVariant *args,
                 uint32_t argCount,
                 NPVariant *result );
    bool HasProperty( NPIdentifier name );
    bool GetProperty( NPIdentifier name, NPVariant *variant );
    bool SetPropertyL( NPIdentifier name, NPVariant *variant );

 public:
    void SetInstance( NPP instance ) { iInstanceHandle = instance; }
    bool InvokeCallback ( TDesC8& name, const NPVariant *args, uint32_t argCount );

 private:
    CSystemInfoPlugin();
    void ConstructL();

    void VariantToInt( NPVariant& v, TInt& i );
    void DesToNpvariant(TPtr8& string, NPVariant*& variant);

 private:
    NPP iInstanceHandle;
    NPIdentifier* iSystemInfoPluginIdentifiers;
    TUid iUid;
    MProfileEngine* iProfileEngine;

    // sysinfo feature class instances
    CPowerObserver* iPowerObserver;  // owned
    CNetworkObserver* iNetworkObserver;  // owned
    CSystemInfoMisc* iSystemInfoMisc;  // owned
    };

/**
********************************************************************************
    Plugin API methods. Source located in CSystemInfoApi.cpp
*********************************************************************************
*/

/**
 * Create a plugin.
 */

NPError SystemInfoNewp( NPMIMEType pluginType, NPP instance, uint16 mode,
                        CDesCArray* argn, CDesCArray* argv, NPSavedData* saved );

/**
 * Wrapper the errors
 */
void SystemInfoConstructL( NPP aInstance, CDesCArray* argn, CDesCArray* argv );

/**
 * Destroy a plugin.
 */
NPError SystemInfoDestroy( NPP instance, NPSavedData** save );

/**
 * Generic hook to retrieve values/attributes from the plugin.
 */
NPError SystemInfoGetvalue( NPP instance, NPPVariable variable, void *ret_value );


/**
********************************************************************************
    System Info Plugin API methods. Source located in CSystemInfoApi.cpp
*********************************************************************************
*/
typedef struct
    {
    NPObject object;
    CSystemInfoPlugin *plugin;
    } SystemInfoPluginObject;

/**
 * systemInfoPluginAllocate
 */
NPObject* systemInfoPluginAllocate( NPP /*npp*/, NPClass* /*aClass*/ );

/**
 * systemInfoPluginDeallocate
 */
void systemInfoPluginDeallocate( SystemInfoPluginObject* obj );

/**
 * systemInfoPluginInvalidate
 */
void systemInfoPluginInvalidate( NPObject* /*obj*/ );

/**
 * systemInfoPluginHasMethod
 */
bool systemInfoPluginHasMethod( SystemInfoPluginObject* obj,
                                NPIdentifier name );

/**
 * systemInfoPluginInvokeFunction
 */
bool systemInfoPluginInvokeFunction( SystemInfoPluginObject* obj,
                                     NPIdentifier name,
                                     NPVariant* args,
                                     uint32_t argCount,
                                     NPVariant* result );

/**
 * systemInfoPluginHasProperty
 */
bool systemInfoPluginHasProperty( SystemInfoPluginObject* obj,
                                  NPIdentifier name );

/**
 * systemInfoPluginGetProperty
 */
bool systemInfoPluginGetProperty ( SystemInfoPluginObject* obj,
                                   NPIdentifier name,
                                   NPVariant* variant);

/**
 * systemInfoPluginSetProperty
 */
bool systemInfoPluginSetProperty( SystemInfoPluginObject* obj,
                                  NPIdentifier name,
                                  NPVariant* variant);

/**
 * _systemInfoPluginClass
 */
static NPClass _systemInfoPluginClass =
    {
    0, // structVersion
    (NPAllocateFunctionPtr) systemInfoPluginAllocate,
    (NPDeallocateFunctionPtr) systemInfoPluginDeallocate,
    (NPInvalidateFunctionPtr) systemInfoPluginInvalidate,
    (NPHasMethodFunctionPtr) systemInfoPluginHasMethod,
    (NPInvokeFunctionPtr) systemInfoPluginInvokeFunction,
    (NPInvokeDefaultFunctionPtr) 0,
    (NPHasPropertyFunctionPtr) systemInfoPluginHasProperty,
    (NPGetPropertyFunctionPtr) systemInfoPluginGetProperty,
    (NPSetPropertyFunctionPtr) systemInfoPluginSetProperty,
    (NPRemovePropertyFunctionPtr) 0,
    (NPEnumerationFunctionPtr) 0
    };

/**
 * static instance of the function pointer table
 */
static NPClass* systemInfoPluginClass = &_systemInfoPluginClass;

#endif      // SYSTEMINFOPLUGIN_H

// End of File
