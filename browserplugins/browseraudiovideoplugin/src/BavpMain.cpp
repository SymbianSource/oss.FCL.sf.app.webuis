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
* Description:  Interface to Browser
*
*/



// INCLUDE FILES
#include <e32std.h>     // For _LIT
#include <npapi.h>      // For NPError
#include <npupp.h>      // For NPNetscapeFuncs
#include <implementationproxy.h>
#include <CEcomBrowserPluginInterface.h>

#include "BavpLogger.h"
#include "BavpPlugin.h" // For bap_xx methods

/***************************************************/
/**********ECOM STYLE BAVP PLUGIN ENTRY POINTS******/
/***************************************************/

// -----------------------------------------------------------------------------
// CBavpPluginEcomMain::NewL
// -----------------------------------------------------------------------------
CBavpPluginEcomMain* CBavpPluginEcomMain::NewL( TAny* aInitParam )
    {
    Log( EFalse, _L("Calling CBavpPluginEcomMain::NewL ") );

    TFuncs* funcs = REINTERPRET_CAST( TFuncs*, aInitParam );
    CBavpPluginEcomMain* self = new (ELeave) CBavpPluginEcomMain( funcs->iNetscapeFuncs );
    CleanupStack::PushL( self );
    self->ConstructL( funcs->iPluginFuncs );
    CleanupStack::Pop( self );

    Dll::SetTls ( (void*) self );

    return self;
    }

// -----------------------------------------------------------------------------
// CBavpPluginEcomMain::ConstructL
// -----------------------------------------------------------------------------
void CBavpPluginEcomMain::ConstructL( NPPluginFuncs* aPluginFuncs )
    {
    Log( EFalse, _L("Calling CBavpPluginEcomMain::ConstructL ") );
    InitializeFuncs( aPluginFuncs );
    }

// -----------------------------------------------------------------------------
// CBavpPluginEcomMain::CBavpPluginEcomMain
// -----------------------------------------------------------------------------
CBavpPluginEcomMain::CBavpPluginEcomMain( NPNetscapeFuncs* aNpf ) 
    : CEcomBrowserPluginInterface(),
      iNpf( aNpf )
    {
    }

// -----------------------------------------------------------------------------
// CBavpPluginEcomMain::~CBavpPluginEcomMain
// -----------------------------------------------------------------------------
CBavpPluginEcomMain::~CBavpPluginEcomMain()
    {
    }

// -----------------------------------------------------------------------------
// KImplementationTable
// -----------------------------------------------------------------------------
const TImplementationProxy KImplementationTable[] =
    {
    {{KFirstBavpImplementationValue}, (TProxyNewLPtr)CBavpPluginEcomMain::NewL},
    {{KSecondBavpImplementationValue}, (TProxyNewLPtr)CBavpPluginEcomMain::NewL},
    {{KThirdBavpImplementationValue}, (TProxyNewLPtr)CBavpPluginEcomMain::NewL},
    {{KFourthBavpImplementationValue}, (TProxyNewLPtr)CBavpPluginEcomMain::NewL}
    };

// -----------------------------------------------------------------------------
// ImplementationGroupProxy
// Returns the filters implemented in this DLL
// Returns: The filters implemented in this DLL
// -----------------------------------------------------------------------------
EXPORT_C const TImplementationProxy* ImplementationGroupProxy(TInt& aTableCount)
    {
    aTableCount = sizeof(KImplementationTable) / sizeof(TImplementationProxy);
    return KImplementationTable;
    }

// -----------------------------------------------------------------------------
// InitializeFuncs
// -----------------------------------------------------------------------------
EXPORT_C NPError InitializeFuncs( NPPluginFuncs* aPpf )
    {
    aPpf->size          = sizeof(NPPluginFuncs);
    aPpf->version       = 1;
    aPpf->newp          = NewNPP_NewProc(BavpNewp);
    aPpf->destroy       = NewNPP_DestroyProc(BavpDestroy);
    aPpf->setwindow     = NewNPP_SetWindowProc(BavpSetwindow);
    aPpf->newstream     = NewNPP_NewStreamProc(BavpNewstream);
    aPpf->destroystream = NewNPP_DestroyStreamProc(BavpDestroystream);
    aPpf->asfile        = NewNPP_StreamAsFileProc(BavpAsfile);
    aPpf->writeready    = NewNPP_WriteReadyProc(BavpWriteready);
    aPpf->write         = NewNPP_WriteProc(BavpWrite);
    aPpf->print         = NewNPP_PrintProc(BavpPrint);
    aPpf->event         = NewNPP_HandleEventProc(BavpEvent);
    aPpf->urlnotify     = NewNPP_URLNotifyProc(BavpUrlnotify);
    aPpf->javaClass     = NULL;
    aPpf->getvalue      = NewNPP_GetValueProc(BavpGetvalue);
    aPpf->setvalue      = NewNPP_SetValueProc(BavpSetvalue);

    return NPERR_NO_ERROR;
    }

// -----------------------------------------------------------------------------
// NPP_Shutdown
// -----------------------------------------------------------------------------
EXPORT_C void NPP_Shutdown(void)
    {
    CBavpPluginEcomMain* npm = (CBavpPluginEcomMain*)Dll::Tls();
    delete npm;
    Dll::SetTls( NULL );
    }

//  End of File
