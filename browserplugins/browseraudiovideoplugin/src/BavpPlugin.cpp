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


// INCLUDE FILES
#include <BrowserAudioVideoPlugin.rsg>
#include <StringLoader.h>
#include <data_caging_path_literals.hrh>
#include <bautils.h>

#include "BavpLogger.h"
#include "BavpPlugin.h"
#include "BavpView.h"
#include "BavpControllerAudio.h"
#include "BavpControllerVideo.h"

#include <npscript.h>
using namespace RT_GestureHelper;

// CONSTANTS
_LIT( KBrowserAVPluginRscFileZ, "z:BrowserAudioVideoPlugin.rsc" );
_LIT( KBrowserAVPluginRscFile, "\\resource\\BrowserAudioVideoPlugin.rsc");

// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBavpPlugin::CBavpPlugin
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CBavpPlugin::CBavpPlugin()
    : iError( EFalse ),
    iMimeType( NULL )
    {
    
    }

// -----------------------------------------------------------------------------
// CBavpPlugin::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
void CBavpPlugin::ConstructL()
    {
    TFileName fileName;
    TParse parse;
               
    Dll::FileName (fileName);
    parse.Set(KBrowserAVPluginRscFile, &fileName, NULL);
    fileName = parse.FullName();        
    BaflUtils::NearestLanguageFile(CCoeEnv::Static()->FsSession(), fileName);
    if ( !BaflUtils::FileExists( CCoeEnv::Static()->FsSession(), fileName ) )        
        {
        // Use the Z drive one
        parse.Set( KBrowserAVPluginRscFileZ, &KDC_RESOURCE_FILES_DIR, NULL );
        fileName = parse.FullName(); 
        BaflUtils::NearestLanguageFile( CCoeEnv::Static()->FsSession(),fileName );
        }
    iResourceOffset = CCoeEnv::Static()->AddResourceFileL( fileName );
    iDrmHelper = CDRMHelper::NewL( *CCoeEnv::Static() );             
    }

// -----------------------------------------------------------------------------
// CBavpPlugin::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
CBavpPlugin* CBavpPlugin::NewL()
    {
    Log( EFalse, _L("CBavpPlugin::NewL") );
    
    CBavpPlugin* self = new (ELeave) CBavpPlugin;
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    
    return self;
    }

// -----------------------------------------------------------------------------
// CBavpPlugin::Destructor
// -----------------------------------------------------------------------------
CBavpPlugin::~CBavpPlugin()
    {
    Log( EFalse, _L("CBavpPlugin::~CBavpPlugin") );

    delete iBavpController;

    delete iFileName;

    delete iBavpView;
        
    delete iDrmHelper;
    
    if ( iMimeType )
    {
        delete iMimeType;
    }
    
    
    if ( iResourceOffset )
        {
        CCoeEnv::Static()->DeleteResourceFile( iResourceOffset );
        }
    if ( iNPObject )
        {
        if (iNPObject->plugin == this)
            iNPObject->plugin = 0;
        NPN_ReleaseObject((struct NPObject*)(iNPObject));
        }
    }

// -----------------------------------------------------------------------------
// CBavpPlugin::SetWindowL
// Handles the creation of the window for the video file.
// Called by Browser for window creation.
// aPluginAdapter allows callback to Browser. aRect is window size.
// -----------------------------------------------------------------------------
void CBavpPlugin::SetWindowL( MPluginAdapter *aPluginAdapter,
                              NPWindow* aWindow, NPP aInstance )
    {
    Log( EFalse, _L("CBavpPlugin::SetWindowL") );

    // Set our PluginAdapter
    iPluginAdapter = aPluginAdapter;
    iNPP = aInstance;

    // Check if error occurs last time for setWindowL
    if ( iError )
        {
        // Stop browser calling NotifyL
        iPluginAdapter->SetPluginNotifier( NULL );
        
        // Error detected on OpenAndPlayFile
        return;
        }

    if ( !iBavpView )
        {
        iBavpView = CBavpView::NewL( (CCoeControl*)iPluginAdapter->GetParentControl(), TRect(0, 0, 0, 0), this);

        // Note: The pluginAdapter returns NULL
        iBavpView->SetObserver( aPluginAdapter->GetParentControlObserver() );

        // Register for Browser Notifications
        iPluginAdapter->SetPluginNotifier( (MPluginNotifier *)this );
        
        // Add option menu handler
        iPluginAdapter->SetOptionMenuHandler( (MOptionMenuHandler *)iBavpView );
    
        // Notify Browser that plugin has been constructed
        iPluginAdapter->PluginConstructedL( iBavpView );
        }
    else
        {
        // Only setRect() is enough, since it calls SizeChanged() and
        // inside it refreshes the rect's coordinate
        iBavpView->SetRect( aWindow );
        
        // If we have a Rtsp scheme, we need to play it (aka start the
        // BavpControllerVideo). We only do this once
        if ( iRtspUrl )
            {
            OpenAndPlayUrlL( *iRtspUrl );
            delete iRtspUrl;
            iRtspUrl = NULL;
            }
        }
    }

// -----------------------------------------------------------------------------
// CBavpPlugin::OpenAndPlayFileL
// Called by Browser when ready for file to be played
// -----------------------------------------------------------------------------
void CBavpPlugin::OpenAndPlayFileL( const TDesC& aFilename, const HBufC* originalFileName )
    {
    Log( EFalse, _L("CBavpPlugin::OpenAndPlayFileL"), (TInt)this );

    if ( !iBavpView )
        {
        // The view is not set yet.
        // Re-invoke the OpenAndPlayFile after the window has been initialized
        delete iFileName;
        iFileName = NULL;
        iFileName = aFilename.AllocL();
        }
    else
        {
        if ( iBavpController )
            {
            // Will stop current audio or video player
            delete iBavpController;
            iBavpController = NULL;
            
            // Set to null, otherwise, plugin will crash when it switches file
            iBavpView->SetBavpController( NULL );
            }

        // Determine if file is audio or video, so we can start up an audio
        // or video controller
        CBavpMediaRecognizer* mediaRecognizer = CBavpMediaRecognizer::NewL();
        CleanupStack::PushL( mediaRecognizer );
        TBavpMediaType mediaType = mediaRecognizer->IdentifyMediaTypeL( aFilename );
        CleanupStack::PopAndDestroy();  // mediaRecognizer
        
        // Do we have audio content, let the audio controller handle it
        if ( mediaType == ELocalAudioFile || mediaType == ELocalAudioPlaylist )
            {
            iBavpController = CBavpControllerAudio::NewL( iBavpView,
                                                          iAccessPtId,
                                                          mediaType,
                                                          aFilename );
            }
        else
            {
            // We got other content, let the video controller handle it.
            iBavpController = CBavpControllerVideo::NewL( iBavpView,
                                                          iAccessPtId,
                                                          mediaType,
                                                          aFilename );
            }

        // Set the volume and loop from the attribute values
        iBavpController->SetVolumeFromAttribute( iVolume );
        iBavpController->SetLoopFromAttribute( iLoop, iInfiniteFlag );
        
        // Set the original file name & mime type
        iBavpController->SetOriginalFileName(originalFileName);
        iBavpController->SetMimeType(iMimeType);
        
        iBavpView->SetBavpController( iBavpController );

        TRAPD( err, iBavpController->PlayAudioVideoL() );
        if ( err )
            {
            Log( EFalse, _L("CBavpPlugin::OpenAndPlayFileL ERROR from PlayAudioVideoL") );
            iBavpController->iCurrentState = EBavpBadContent;
            iBavpView->UpdateView();
            User::Leave( err );
            }
        }
    }

// -----------------------------------------------------------------------------
// CBavpPlugin::OpenAndPlayUrlL
// Called by Browser when ready for url to be played
// -----------------------------------------------------------------------------
void CBavpPlugin::OpenAndPlayUrlL( const TDesC& aUrl )
    {
    Log( EFalse, _L("CBavpPlugin::OpenAndPlayUrlL"), (TInt)this );

    if ( iBavpController )
        {
        // Will stop current audio or video player
        delete iBavpController;
        iBavpController = NULL;
        
        // Set to null, otherwise, plugin will crash when it switches file
        iBavpView->SetBavpController( NULL );
        }

    iBavpController = CBavpControllerVideo::NewL( iBavpView,
                                                  iAccessPtId,
                                                  EUrl,
                                                  aUrl );

    // iRtspUrl is used as a flag in SetWindow, which is re-entrant, so delete
    // once we got a controller for it
    delete iRtspUrl;
    iRtspUrl = NULL;

    // Set the volume and loop from the attribute values
    iBavpController->SetVolumeFromAttribute( iVolume );
    iBavpController->SetLoopFromAttribute( iLoop, iInfiniteFlag );
    
    iBavpView->SetBavpController( iBavpController );

    iBavpController->PlayAudioVideoL();
    }

// -----------------------------------------------------------------------------
// CBavpPlugin::SetAccessPtId
// Sets access pt id to be used
// -----------------------------------------------------------------------------
void CBavpPlugin::SetAccessPtId( TUint aAccessPtId )
    {
    iAccessPtId = aAccessPtId;
    }

// -----------------------------------------------------------------------------
// CBavpPlugin::SetVolumeAndLoop
// Called to set the volume and loop value that we get from the webpage source
// to be able to use it later 
// -----------------------------------------------------------------------------
void CBavpPlugin::SetVolumeAndLoop( TInt aVolume, TInt aLoop, TInt aInfiniteFlag )
    {
    // Save volume attribute value, pass to controller after it initializes
    iVolume = aVolume;

    // Save loop attribute values, pass to controller after it initializes
    iLoop = aLoop;
    iInfiniteFlag = aInfiniteFlag;
    }

// -----------------------------------------------------------------------------
// CBavpPlugin::SetVolumeAndLoop
// Called to set the volume and loop value that we get from the webpage source
// to be able to use it later 
// -----------------------------------------------------------------------------
void CBavpPlugin::SetRtspUriL( const TDesC& aRtspUri )
    {
    iRtspUrl = aRtspUri.AllocL();
    }

// -----------------------------------------------------------------------------
// CBavpPlugin::NotifyL
// MPluginNotifier callback, Browser calls this to inform Plugin of some event.
// The biggest use is when the Browser goes in-focus or out-focus. Out-foucus
// can happen when other component takes top of CCoeControl stack, such as the 
// Option Menu, another application, Screen saver, ... 
// -----------------------------------------------------------------------------
TInt CBavpPlugin::NotifyL( TNotificationType aCallType, void* aParam )
    {
    switch ( aCallType )
        {
        case EApplicationFocusChanged:
            // The browser has gone in-focus (foreground), or out-focus
            // (background), i.e. the browser is not on top of CCoeControl stack,
            // so inform this plugin if it is in-focus or out-focus
            if ( iBavpController )
                {
                // If Browser out-focus, we send this plugin aParam=EFalse.
                // If Browser in-focus, we send this plugin aParam=ETrue, if
                // the plugin is (was) in-focus or activated.
                iBavpController->HandleBrowserNotification( TBool(aParam) );
                }
            break;

        default:
            // Not implemented
            break;

        };  // end of switch
        
    return NPERR_NO_ERROR;
    }
    
// -----------------------------------------------------------------------------
// CBavpPlugin::SetMimeType
// Sets the mime name. Used for scripting functionality
// -----------------------------------------------------------------------------
void CBavpPlugin::SetMimeType( NPMIMEType type )
    {
    iMimeType = type.Alloc();
    }
    
// -----------------------------------------------------------------------------
// CBavpPlugin::HandleGesture
// -----------------------------------------------------------------------------
TBool CBavpPlugin::HandleGesture(void* aEvent)
    {
    TBool ret = EFalse;
    TGestureEvent *gesture = static_cast<TGestureEvent*>(aEvent);
    ret = iBavpController->HandleGesture(gesture);
    
    return ret;
    }

// -----------------------------------------------------------------------------
// CBavpPlugin::getNPNFuncs
// -----------------------------------------------------------------------------
NPNetscapeFuncs* CBavpPlugin::getNPNFuncs()
    {
    CBavpPluginEcomMain* npm = (CBavpPluginEcomMain*)Dll::Tls();
    return npm ? npm->Funcs() : NULL;
    }


//  End of File
