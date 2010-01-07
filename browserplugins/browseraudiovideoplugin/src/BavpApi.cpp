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
#include <Uri16.h>
#include <mmf/common/MmfControllerFrameworkBase.h>
#include <stdlib.h>

#include "BavpLogger.h"
#include "BavpPlugin.h"
#include "BavpVolumeHandler.h"

#include <npscript.h>
#include "BavpNPObject.h"
// CONSTANTS
_LIT(KAccessPtId, "accesspointid");
_LIT(KLoop,"loop");
_LIT(KSrc,"src");
_LIT(KData,"data");
_LIT(KRtsp,"rtsp");
_LIT(KInfinite,"infinite");
_LIT(KVolume, "volume");
_LIT(KHigh, "high");
_LIT(KMiddle, "middle");
_LIT(KLow, "low");

const TInt KOneLoopValue = 1;
const TInt KMaxLoopValue = 50; 

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// BavpNewp
// Create a new instance of a plugin. This is non-leaving method.
// Returns: NPError: Error codes recognized by Browser
// -----------------------------------------------------------------------------
NPError BavpNewp( NPMIMEType /* pluginType*/,   // Not used locally
                  NPP aInstance,                // Stores pointer to Bavp 
                  uint16 /* mode */,            // 
                  CDesCArray* argn,             // The number of arguments passed
                  CDesCArray* argv,             // The values of arguments in the array
                  NPSavedData* /*saved*/ )      // 
    {
        
    // Fix for TSW error ID:  SLON-74588K, as ControlPanel is unsupported so for the time being
    // controlPanel plugin window shown as emplty non supported window.

    _LIT(KControls, "controls");     
    _LIT(KControlPanel, "controlpanel");
   
    for ( TInt i=0; i < argn->Count(); i++ )
    {
    if(((*argn)[i].Compare( KControls) == 0)&&((*argv)[i].Compare(KControlPanel ) == 0))
        {
            return NPERR_INVALID_PLUGIN_ERROR;
        }
    }
    // Create BrowserAudioVideoPlugin, call this leaving method to wrap leaving methods
    TRAPD( err, BavpConstructL( aInstance, argn, argv ) );
    if ( err == KErrNoMemory )
        {       
        Log( EFalse, _L("FUNCTION BavpNewp ERROR! Out of Memory Error") );

        return NPERR_OUT_OF_MEMORY_ERROR;
        }
        
    if ( err != KErrNone )
        {
        Log( EFalse, _L("FUNCTION BavpNewp ERROR! Module Load Failure") );
        return NPERR_MODULE_LOAD_FAILED_ERROR;
        }
        
    return NPERR_NO_ERROR;
    }

// -----------------------------------------------------------------------------
// BavpConstructL
// This is the leaving method to create the plugin.  We have leaving setters
// that need to be wrapped.
// Returns: void
// -----------------------------------------------------------------------------
void BavpConstructL( NPP aInstance,     // Stores pointer to Bavp 
                     CDesCArray* argn,  // The number of arguments passed
                     CDesCArray* argv ) // The values of arguments in the array
    {
    Log( EFalse, _L("Calling FUNCTION BavpConstructL") );
    
    CBavpPlugin *pluginInstance( NULL );

    // Create BrowserVideoPlugin        
    pluginInstance = CBavpPlugin::NewL();   

    // Bavp functions can now be called
    aInstance->pdata = pluginInstance;
    
    // Retrieve the various attributes from the HTTP file and the
    // object/embed/bgsound/soundstart tags on that page
    //
    // Retrieve the access point attribute
    TInt accessPtIndex;
    TInt accessPtId( KUseDefaultIap );
    if  ( !argn->Find( KAccessPtId, accessPtIndex, ECmpFolded ) )
        {
        // If the string is found get the access pt id and convert to TInt
        TPtrC16 accessPtValue( (*argv)[accessPtIndex] );
        TLex lex( accessPtValue );
        TInt intVal( 0 );
        if ( lex.Val( intVal ) == KErrNone)
          {
          accessPtId = intVal;
          }
        }
    pluginInstance->SetAccessPtId( accessPtId );
    
    // Retrieve the "loop" attribute value, default value is "1",
    // with infiniteFlag off. This handles the loop attribute missing
    // and loop="0" cases.
    TInt loopIndex( 0 );
    TPtrC16 loopValue;
    TBool infiniteFlag( EFalse );   // Always false, use loop="50" instead "infinite" 
    TInt loop( KOneLoopValue );     // Our default loop value is "1"
    
    TInt positionLoop = argn->Find( KLoop, loopIndex, ECmpFolded );
    if ( positionLoop == 0 )
        {
        // Found a loop attribute value
        TPtrC16 pointerLoopCount( (*argv)[loopIndex] );
        loopValue.Set( pointerLoopCount );

        // Is the loop value equal to string "infinite"
        if  ( loopValue.Compare( KInfinite ) == 0 )
            {
            // We never play true infinite, we loop max 50 times
            loop = KMaxLoopValue;
            }
        else
            {
            // Ensure that only a valid loop value is used (1 to 50)
            HBufC8* convertPointerLoopCount = HBufC8::New( pointerLoopCount.Length() );
            TPtr8 convertToByteLoop = convertPointerLoopCount->Des();
            convertToByteLoop.Copy( pointerLoopCount );
                
            loop = atoi( (char*)(convertToByteLoop.Ptr()) );
            if ( loop < KOneLoopValue )
                {
                loop = KOneLoopValue;
                }
            else if ( loop > KMaxLoopValue )
                {
                loop = KMaxLoopValue;
                }
            }
        }

    // Retrieve the "volume" attribute value, default is max, so that if the
    // "volume" attribute isn't present, the user preferences will not get
    // overwritten.  There is a check in controller->SetVolumeByAttribute that
    // checks current and attribute volume and choses lower volume.
    TInt positionVolumeValue( 0 ); 
    TInt CRVolume( KCRVolumeMax );
    TPtrC16 volumeValue;
    TInt positionVol = argn->Find( KVolume, positionVolumeValue, ECmpFolded );
    if ( positionVol == 0 )
    {
        TPtrC16 pointerVolumeValue( (*argv)[positionVolumeValue] );
        volumeValue.Set( pointerVolumeValue );
        if (  !(volumeValue.Compare( KHigh )) )
            {
            CRVolume = KCRVolume10;
            }
        else if ( !(volumeValue.Compare( KMiddle )) )
            {
            CRVolume = KCRVolume5;
            }
        else if ( !(volumeValue.Compare( KLow )) )
            {
            CRVolume = KCRVolume2;
            }
        else
            {
            // Convert volume numeric string (i.e. volume="6") to CR volume
            HBufC8* convertPointerVolumeValue = HBufC8::New( pointerVolumeValue.Length() );
            TPtr8 convertToByteVolume = convertPointerVolumeValue->Des();
            convertToByteVolume.Append( pointerVolumeValue );
            CRVolume = atoi( (char*)(convertToByteVolume.Ptr()) );
            if ( CRVolume < KCRVolumeMute )
                {
                CRVolume = KCRVolumeMute;
                }
            else if ( CRVolume > KCRVolumeMax )
                {
                CRVolume = KCRVolumeMax;
                }
            }    
    }
    
    // Set the Volume and Loop for plugin
    pluginInstance->SetVolumeAndLoop( CRVolume, loop, infiniteFlag );

    // Retrieve the "src" or "data" attribute value, if it is rtsp scheme lets
    // save it, so we can start a controller, since rtsp content is not passed
    // by WebKit.  It is our (Bavp) responsibilty to load rtsp content.
    TInt srcIndex( 0 ); 
    TInt ret = argn->Find( KSrc, srcIndex, ECmpFolded );
    if  ( ret != 0 )
        {
        // If the src attribute NOT found, try to find data attribute
        ret = argn->Find( KData, srcIndex, ECmpFolded );
        }

    if ( ret == 0 )
        {
        // We found either data or src attribute
        TPtrC16 srcValue( (*argv)[ srcIndex ] );
        TUriParser uriParser;
        uriParser.Parse( srcValue );
        if ( !uriParser.Extract( EUriScheme ).Compare( KRtsp ) )
            {
            // We found a RTSP scheme. Lets save it up, so when the BavpView
            // is create we can start the BavpController and load rtsp. This
            // is because rtsp will not be started by a call to BavpAsFile().
            // NOTE: Files (http) are downloaded first and then BavpAsFile()
            // is called, which starts the BavpController.
            pluginInstance->SetRtspUriL( uriParser.UriDes() );
            
            // We may have to move this to a different place if eg: the url of
            // the object is changed later (say from javascript)
            CBavpPluginEcomMain* lBavpPluginEcomMain = (CBavpPluginEcomMain*) Dll::Tls();
            NPNetscapeFuncs* funcPtr = lBavpPluginEcomMain->Funcs();
            if ( funcPtr && funcPtr->getvalue ) 
               {
               funcPtr->getvalue( aInstance, NPNNetworkAccess, (void *)&accessPtId);
               pluginInstance->SetAccessPtId( accessPtId );
               }
            
            }
        }
    }

// -----------------------------------------------------------------------------
// BavpDestroy(NPP aInstance, NPSavedData**)
// Called by Browser to destroy the plugin
// Returns: NPError: Error Code
// -----------------------------------------------------------------------------
NPError BavpDestroy( NPP    aInstance,          // Link to Browser
                     NPSavedData** /*save*/ )   // Not used locally
    {
    Log( EFalse, _L("Calling FUNCTION BavpDestroy") );

    CBavpPlugin *pluginInstance = STATIC_CAST( CBavpPlugin*, aInstance->pdata );
    delete pluginInstance;

    return NPERR_NO_ERROR;
    }

// -----------------------------------------------------------------------------
// BavpSetwindow
// This is the parent window of plugin
// Returns: NPError: Error Code
// -----------------------------------------------------------------------------
NPError BavpSetwindow( NPP aInstance,       // Link to Browser
                       NPWindow* aWindow )  // Browser's window passed to Bavp
    {
    Log( EFalse, _L("Calling FUNCTION BavpSetwindow") );

    CBavpPlugin *pluginInstance = STATIC_CAST( CBavpPlugin*, aInstance->pdata );
    MPluginAdapter *pluginAdapter = STATIC_CAST( MPluginAdapter*, aWindow->window);
    
    TRAPD( err, pluginInstance->SetWindowL(pluginAdapter, aWindow, aInstance) );
    if ( err == KErrNoMemory )
        {
        Log( EFalse, _L("FUNCTION BavpSetWindow ERROR! Out of Memory Error") );

        // Set the error to plugin if err occurs, it's for the next create plugin check
        pluginInstance->iError = ETrue;
        return NPERR_OUT_OF_MEMORY_ERROR;
        }
        
    if ( err != KErrNone )
        {
        Log( EFalse, _L("FUNCTION BavpSetWindow ERROR: %d"), err );

        // Set the error to plugin if err occurs, it's for the next create plugin check
        pluginInstance->iError = ETrue;
        return NPERR_GENERIC_ERROR;
        }
        
    return NPERR_NO_ERROR;
    }

// -----------------------------------------------------------------------------
// BavpNewstream
// Indicates start of downloading of an video file from the internet
// Returns: NPError: Error Code
// -----------------------------------------------------------------------------
NPError BavpNewstream( NPP instance,         
                       NPMIMEType type,      
                       NPStream* /*stream*/,    // 
                       NPBool /*seekable*/,     // 
                       uint16* stype )          // Will set to NP_ASFILEONLY to notify Browser how to send data 
    {
    Log( EFalse, _L("Calling FUNCTION BavpNewstream") );

    // Bavp needs to set stype to alert Browser to send data via Bavp_asfile
    *stype = NP_ASFILEONLY; 
    
    CBavpPlugin *pluginInstance = STATIC_CAST( CBavpPlugin*, instance->pdata );
    
    pluginInstance->SetMimeType( type );
    
    return NPERR_NO_ERROR;
    }

// -----------------------------------------------------------------------------
// BavpDestroystream    NOT USED
// Indicates completion of downloading an video file from the internet
// Returns: NPError: Error Code
// -----------------------------------------------------------------------------
NPError BavpDestroystream( NPP  /*instance*/,       // Not used locally
                           NPStream* /*stream*/,    //
                           NPReason /*aReason*/ )   //
    {
    Log( EFalse, _L("Calling FUNCTION BavpDestroyStream") );

    return NPERR_NO_ERROR; 
    }

// -----------------------------------------------------------------------------
// BavpAsfile
// Called by Browser when data is copied to a file.
// Returns: void
// -----------------------------------------------------------------------------
void BavpAsfile( NPP aInstance,             // Link to Browser
                 NPStream* stream,      
                 const TDesC& aFileName )   // file name and path
    {
    Log( EFalse, _L("Calling FUNCTION BavpAsfile") );
    Log( EFalse, _L("Filename:") );
    Log( EFalse, aFileName );

    CBavpPlugin *pluginInstance = STATIC_CAST( CBavpPlugin*, aInstance->pdata );
    
    // Get the error, and set this error to iError in plugin
    TRAPD( err, pluginInstance->OpenAndPlayFileL( aFileName, stream->url ) );
    if ( err != KErrNone )
        {
        Log( EFalse, _L("FUNCTION BavpAsfile ERROR!") );

        // If error, set for plugin, it's for the next create plugin check
        pluginInstance->iError = ETrue;
        }
    }

// -----------------------------------------------------------------------------
// BavpWriteready       NOT SUPPORTED
// -----------------------------------------------------------------------------
int32 BavpWriteready( NPP /*instance*/,     
                      NPStream* /*stream*/ )    
    {
    // Not implemented
    return NPERR_NO_ERROR;
    }

// -----------------------------------------------------------------------------
// BavpWrite     NOT SUPPORTED
// -----------------------------------------------------------------------------
int32 BavpWrite( NPP /*instance*/,  
                 NPStream* /*stream*/,   
                 int32 /*offset*/,      
                 int32 /*len*/,     
                 void* /*buffer*/ )
        {
        // Not implemented
        return NPERR_NO_ERROR; 
        }

// -----------------------------------------------------------------------------
// BavpPrint        NOT SUPPORTED
// -----------------------------------------------------------------------------
void BavpPrint( NPP /*instance*/,           
                NPPrint* /*platformPrint*/ )    
    {
    // Not implemented
    }

// -----------------------------------------------------------------------------
// BavpEvent        NOT SUPPORTED
// -----------------------------------------------------------------------------
int16 BavpEvent( NPP instance,
                 void* event )      
    {
    TInt ret = EFalse;
    if (instance)
        {
        CBavpPlugin *plugin = static_cast<CBavpPlugin*>(instance->pdata);
        NPEvent *ev = static_cast<NPEvent*>(event);
    
        if (plugin)
            {
            if (ev->event == ENppEventPointer)
                {
                NPEventPointer *evp =static_cast<NPEventPointer*> (ev->param);
                ret = plugin->HandleGesture(evp->reserved);
                }
            }
        }
    return ret;
    }

// -----------------------------------------------------------------------------
// BavpUrlnotify        NOT SUPPORTED
// -----------------------------------------------------------------------------
void BavpUrlnotify( NPP /*instance*/,       
                    const TDesC8& /*url*/,  
                    NPReason /*reason*/,    
                    void* /*notifyData*/ )
    {
    // Not implemented
    }

// -----------------------------------------------------------------------------
// BavpSetvalue     NOT SUPPORTED
// -----------------------------------------------------------------------------
NPError BavpSetvalue( NPP /*instance*/,     
                      NPNVariable /*variable*/,
                      void* /*ret_value*/ )
    {
    // Not implemented
    return NPERR_NO_ERROR;
    }

// -----------------------------------------------------------------------------
// BavpGetvalue     NOT SUPPORTED
// -----------------------------------------------------------------------------
NPError BavpGetvalue( NPP aInstance,        
                      NPPVariable aVariable,  
                      void* ret_value ) 
    {
    NPError err( NPERR_NO_ERROR );

        switch( aVariable )
        {
        case NPPVpluginInteractiveBool:
            // Interactive=0, Not Interactive=1
             *(TBool *)ret_value = EFalse;
             break;
        case NPPVpluginScriptableNPObject:
            {        
            // return instance of scripting object to the browser
            BavpNPObject *pluginNPObject = BavpNPObject_new( aInstance );        
        
            CBavpPlugin *bavpplugin = (CBavpPlugin*)aInstance->pdata;
            bavpplugin->SetNPObject(pluginNPObject);
            
            pluginNPObject->plugin = bavpplugin; 
            pluginNPObject->nppInstance = aInstance;
            void** ret = (void**)ret_value;
            *ret = (void*)pluginNPObject;        
            }
            break;
        default:
            break;
        }   //end switch

    return err;
    }

//  End of File  
