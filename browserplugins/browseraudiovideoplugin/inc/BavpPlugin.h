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
* Description:  Interface to Browser for handling embedded video requests.
*
*/


#ifndef BAVPPLUGIN_H
#define BAVPPLUGIN_H

//  INCLUDES
#include <e32std.h>
#include <npupp.h>
#include <PluginAdapterInterface.h> // For base class MPluginNotifier
#include <aknnotewrappers.h>  // For displaying note
#include <drmhelper.h>
#include <DRMCommon.h>
#include <CEcomBrowserPluginInterface.h>
#include "BavpNPObject.h"
#include <rt_gesturehelper.h>

// FORWARD DECLARATIONS
class CBavpView;
class CBavpController;

// CLASS DECLARATION

/******************************************
This class is specific to ECom Style Plugin.
This is used for passing plugin specific initialization information to and from
browser.
*******************************************/
IMPORT_C NPError InitializeFuncs( NPPluginFuncs* aPpf );

// CONSTANTS
const TInt KFirstBavpImplementationValue = 0x10282811;
const TInt KSecondBavpImplementationValue = 0x10282812;
const TInt KThirdBavpImplementationValue = 0x10282813;
const TInt KFourthBavpImplementationValue = 0x10282814;

/**
*  CBavpPluginEcomMain
*  Used for passing plugin specific initialization information to and from browser.
*  @lib npBrowserVideoPlugin.lib
*  @since 3.2
*/
class CBavpPluginEcomMain : public CEcomBrowserPluginInterface
    {
    public: // Methods from base classes
        
        /**
        * Two-phased constructor.
        */
        static CBavpPluginEcomMain* NewL( TAny* aInitParam );
        
        /**
        * Destructor
        */
        ~CBavpPluginEcomMain();
        
        /**
        * Passed from the browser to the plugin and delcares what functions the browser 
        * makes available
        */
        NPNetscapeFuncs* Funcs() { return iNpf; }

    private:

        // Default constructor
        CBavpPluginEcomMain( NPNetscapeFuncs* aNpf );

        // By default Symbian 2nd phase constructor is private.
        void ConstructL( NPPluginFuncs* aPluginFuncs );

        // Netscape Function Pointer
        NPNetscapeFuncs* iNpf;
    };

/**
*  CBavpPlugin class 
*  Interfaces to Browser, used to pass information between browser and plugin
*  @lib npBrowserVideoPlugin.lib
*  @since 3.2
*/
class CBavpPlugin : public CBase,
                    public MPluginNotifier
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        */
        static CBavpPlugin* NewL();

        /**
        * Destructor.
        */
        virtual ~CBavpPlugin();

    public: // New methods

       /**
        * SetWindowL
        * @since 3.2
        * @param MPluginAdapter *pluginAdapter: Pointer to Browser adapter class
        * @param const TRect& aRect: rectangle representing current UI
        * @return void
        */
        void SetWindowL( MPluginAdapter *pluginAdapter, NPWindow* aWindow, NPP aInstance );

        /**
        * OpenAndPlayFileL
        * @since 3.2
        * @param const TDesC& aFileName: Name and path of file to be played
        * @param const HBufC* originalFileName: name of the file to be played. added for scripting.
        * @return void
        */
        void OpenAndPlayFileL( const TDesC& aFileName, const HBufC* originalFileName );

        /**
        * OpenAndPlayUrlL
        * @since 3.2
        * @param const TDesC& aUrl: Name and path of uri to be played
        * @return void
        */
        void OpenAndPlayUrlL( const TDesC& aUrl );
        
        /**
        * Controller
        * @since 5.0
        * @param void
        * @return controller
        */
        CBavpController* Controller() { return iBavpController; }
        
        /**
        * SetNPObject
        * iNPObject is a scriptable object
        * @since 5.0
        * @param BavpNPObject *aObj
        * @return void
        */ 
        void SetNPObject( BavpNPObject* aObj ) { iNPObject = aObj; }
        
        /**
        * NPObject
        * return the scriptable object
        * @since 5.0
        * @param void
        * @return BavpNPObject*
        */
        BavpNPObject* NPObject() const { return iNPObject; }


    public: // New methods

        /**
        * SetAccessPtId
        * Set access pt id
        * @since 3.2
        * @param TUint aAccessPtId, sets the access pt id
        * @return void
        */
        void SetAccessPtId( TUint aAccessPtId );

        /**
        * SetVolumeAndLoop
        * Set volume, loop, and infinite values
        * @since 3.2
        * @param TInt aVolume, the volume level from web page 
        * @param TInt aLoop, the loop count from web page 
        * @param TInt aInfiniteFlag, not used 
        * @return void
        */
        void SetVolumeAndLoop( TInt aVolume, TInt aLoop, TInt aInfiniteFlag );

        /**
        * SetRtspUri
        * Set Rtsp uri
        * @since 3.2
        * @param TInt aVolume, the volume level from web page 
        * @param TInt aLoop, the loop count from web page 
        * @param TInt aInfiniteFlag, not used 
        * @return void
        */
        void SetRtspUriL( const TDesC& aRtspUri );

    public: // Methods from base classes

        /**
        * From MPluginNotifier
        * @since 3.2
        * @param TNotificationType aCallType: Represents a type of notification from Browser
        * @param void* aParam: Contains value corresponding to notification
        * @return TInt: Error Code
        */
        TInt NotifyL( TNotificationType aCallType, void* aParam );

        /**
        * Set the mime type
        * The mimetype is returned from headers
        * @param NPMIMEType type
        * @return void
        */
        void SetMimeType(NPMIMEType type);
        
        NPP  getNPP() {return iNPP;}
        
        NPNetscapeFuncs* getNPNFuncs();
        
        TBool HandleGesture(void* aEvent);
    private:

        /**
        * C++ default constructor.
        */
        CBavpPlugin();

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();
       
    public: // Data

        // Initialize as false, then set it whenever error occurs
        TBool iError; 

    private:    // Data

        // Interface to Browser
        MPluginAdapter* iPluginAdapter;

        // Audio or Video controller
        CBavpController* iBavpController;

        // CCoeControl interface
        CBavpView* iBavpView;
        
        BavpNPObject* iNPObject;

        // Resource offset
        TInt iResourceOffset;

        // Src file scheme, which is changed by script in run time
        HBufC* iFileName;
        
        // Src RTSP scheme uri, which is used to start BavpController 
        HBufC* iRtspUrl;

        // Access point id  
        TUint iAccessPtId;

        // Value of the Volume embedded in the HTML file
        TInt  iVolume;

        // Value of the Loop embedded in the HTML file
        TInt  iLoop;

        // Flag indication if the loop is set to infinite
        TBool iInfiniteFlag;

        // DRM Helper instance to address the error case from the plugin level
        CDRMHelper* iDrmHelper;
        
        // Mime type. Added for scripting functionality
        HBufC8* iMimeType;
        NPP iNPP;

    };

/**
********************************************************************************
    Plugin API methods. Source located in CBavpApi.cpp
*********************************************************************************
*/

/**
 * Create a plugin.
 */

NPError BavpNewp( NPMIMEType pluginType, NPP instance, uint16 mode,
                  CDesCArray* argn, CDesCArray* argv, NPSavedData* saved );

/**
 * Wrapper the errors
 */
void BavpConstructL( NPP aInstance, CDesCArray* argn, CDesCArray* argv );

/**
 * Destroy a plugin.
 */
NPError BavpDestroy( NPP instance, NPSavedData** save );

/**
 * This is the parent window of a plugin.
 */
NPError BavpSetwindow( NPP instance, NPWindow* window );

/**
 * A new data stream has been created for sending data to the plugin.
 */
NPError BavpNewstream( NPP instance, NPMIMEType type, NPStream* stream,
                       NPBool seekable, uint16* stype );

/**
 * A data stream has been terminated.
 */
NPError BavpDestroystream( NPP instance, NPStream* stream, NPReason reason );

/**
 * A data stream has been fully saved to a file.
 */
void BavpAsfile( NPP instance, NPStream* stream, const TDesC& fname );

/**
 * Return the maximum number of bytes this plugin can accept from the stream.
 */
int32 BavpWriteready( NPP instance, NPStream* stream );

/**
 * Receive more data - return number of bytes processed.
 */
int32 BavpWrite( NPP instance, NPStream* stream,
                 int32 offset, int32 len, void* buffer );

/**
 * This is the URL being sensed.
 */
void BavpUrlnotify( NPP instance, const TDesC8& url, NPReason reason,
                    void* notifyData );

/**
 * Generic hook to retrieve values/attributes from the plugin.
 */
NPError BavpGetvalue( NPP instance, NPPVariable variable, void *ret_value );

/**
 * Generic hook to set values/attributes within the plugin.
 */
NPError BavpSetvalue( NPP instance, NPNVariable variable, void *ret_value );

/**
 * Print
 */
void BavpPrint( NPP instance, NPPrint* platformPrint );

/**
 * Event
 */
int16 BavpEvent( NPP instance, void* event );

#endif      // BAVPPLUGIN_H

// End of File
