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


#ifndef GPSPLUGIN_H
#define GPSPLUGIN_H

//  INCLUDES
#include <cecombrowserplugininterface.h>
#include <npscript.h>
#include <pluginadapterinterface.h> // For base class MPluginNotifier

// FORWARD DECLARATIONS
class CGpsObserver;

// CLASS DECLARATION

/******************************************
This class is specific to ECom Style Plugin.  This is used for passing
plugin specific initialization information to and from browser.
*******************************************/
IMPORT_C NPError InitializeFuncs( NPPluginFuncs* aPpf );

// CONSTANTS
const TInt KFirstGpsImplementationValue = 0x10282870;

/**
*  CGpsPluginEcomMain
*  Used for passing plugin specific initialization information to and from browser.
*  @lib npGpsPlugin.lib
*  @since S60 v3.2
*/
NONSHARABLE_CLASS(CGpsPluginEcomMain) : public CEcomBrowserPluginInterface
    {
 public: // Functions from base classes

    /**
     * Two-phased constructor.
     */
    static CGpsPluginEcomMain* NewL( TAny* aInitParam );

    /**
     * Destructor
     */
    virtual ~CGpsPluginEcomMain();

    /**
     * Passed from the browser to the plugin and delcares what
     * functions the browser makes available
     */
    NPNetscapeFuncs* Funcs() const { return iNpf; }

 private:

    // Default constructor
    CGpsPluginEcomMain( NPNetscapeFuncs* aNpf );

    // By default Symbian 2nd phase constructor is private.
    void ConstructL( NPPluginFuncs* aPluginFuncs );

    // Netscape Function Pointer
    NPNetscapeFuncs* iNpf;
    };

/**
*  CGpsPlugin class
*  Interfaces to Browser, used to pass information between browser and plugin
*  @lib npGpsPlugin.lib
*  @since S60 v3.2
*/
NONSHARABLE_CLASS(CGpsPlugin) : public CBase
    {
 public:
    static CGpsPlugin* NewL();

    virtual ~CGpsPlugin();

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
    bool InvokeCallback( TDesC8& name, const NPVariant *args, uint32_t argCount );

    /**
     * SetWindowL
     * @since 2.8
     * @param MPluginAdapter *pluginAdapter: Pointer to Browser adapter class
     * @param const TRect& aRect: rectangle representing current UI
     * @return void
     */
    void SetWindowL( MPluginAdapter *pluginAdapter, const TRect& aRect );

 private:
    CGpsPlugin();
    void ConstructL();

 private:
    NPP iInstanceHandle;
    NPIdentifier* iGpsPluginIdentifiers; // not owned

    // Interface to Browser
    MPluginAdapter* iPluginAdapter; // not owned

    // gpsplugin feature class instance
    CGpsObserver* iGpsObserver; // owned
    };

/**
********************************************************************************
    Plugin API methods. Source located in CGpsApi.cpp
*********************************************************************************
*/

/**
 * Create a plugin.
 */

NPError GpsNewp( NPMIMEType pluginType, NPP instance, uint16 mode,
                      CDesCArray* argn, CDesCArray* argv, NPSavedData* saved );

/**
 * Wrapper the errors
 */
void GpsConstructL( NPP aInstance, CDesCArray* argn, CDesCArray* argv );

/**
 * Destroy a plugin.
 */
NPError GpsDestroy( NPP instance, NPSavedData** save );

/**
 * Generic hook to retrieve values/attributes from the plugin.
 */
NPError GpsGetvalue( NPP instance, NPPVariable variable, void *ret_value );

/**
  * This is the parent window of a plugin.
  */
NPError GpsSetwindow(NPP instance, NPWindow* window);

/**
********************************************************************************
    Gps Plugin API methods. Source located in CGpsApi.cpp
*********************************************************************************
*/
typedef struct
    {
    NPObject object;
    CGpsPlugin *plugin;
    } GpsPluginObject;

/**
 * GpsPluginAllocate
 */
NPObject* GpsPluginAllocate( NPP /*npp*/, NPClass* /*aClass*/ );

/**
 * GpsPluginDeallocate
 */
void GpsPluginDeallocate( GpsPluginObject* obj );

/**
 * GpsPluginInvalidate
 */
void GpsPluginInvalidate( NPObject* /*obj*/ );

/**
 * GpsPluginHasMethod
 */
bool GpsPluginHasMethod( GpsPluginObject* obj,
                         NPIdentifier name );

/**
 * GpsPluginInvoke
 */
bool GpsPluginInvoke( GpsPluginObject* obj,
                      NPIdentifier name,
                      NPVariant* args,
                      uint32_t argCount,
                      NPVariant* result );

/**
 * GpsPluginHasProperty
 */
bool GpsPluginHasProperty( GpsPluginObject* obj,
                           NPIdentifier name );

/**
 * GpsPluginGetProperty
 */
bool GpsPluginGetProperty( GpsPluginObject* obj,
                           NPIdentifier name,
                           NPVariant* variant );

/**
 * GpsPluginSetProperty
 */
bool GpsPluginSetProperty( GpsPluginObject* obj,
                           NPIdentifier name,
                           NPVariant* variant );

/**
 * _GpsPluginClass
 */
static NPClass _GpsPluginClass =
    {
    0, // structVersion
    (NPAllocateFunctionPtr) GpsPluginAllocate,
    (NPDeallocateFunctionPtr) GpsPluginDeallocate,
    (NPInvalidateFunctionPtr) GpsPluginInvalidate,
    (NPHasMethodFunctionPtr) GpsPluginHasMethod,
    (NPInvokeFunctionPtr) GpsPluginInvoke,
    (NPInvokeDefaultFunctionPtr) 0,
    (NPHasPropertyFunctionPtr) GpsPluginHasProperty,
    (NPGetPropertyFunctionPtr) GpsPluginGetProperty,
    (NPSetPropertyFunctionPtr) GpsPluginSetProperty,
    (NPRemovePropertyFunctionPtr) 0,
    (NPEnumerationFunctionPtr) 0
    };

/**
 * static instance of the function pointer table
 */
static NPClass* GpsPluginClass = &_GpsPluginClass;

#endif      // GPSPLUGIN_H

// End of File
