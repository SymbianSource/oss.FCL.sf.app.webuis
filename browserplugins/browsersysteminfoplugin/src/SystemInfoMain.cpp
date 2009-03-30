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
#include <implementationproxy.h>
#include <CEcomBrowserPluginInterface.h>
#include "SystemInfoPlugin.h"


/***************************************************/
/**********ECOM STYLE SystemInfo PLUGIN ENTRY POINTS******/
/***************************************************/

// -----------------------------------------------------------------------------
// CSystemInfoPluginEcomMain::NewL
// -----------------------------------------------------------------------------
CSystemInfoPluginEcomMain* CSystemInfoPluginEcomMain::NewL( TAny* aInitParam )
    {
    TFuncs* funcs = REINTERPRET_CAST( TFuncs*, aInitParam );
    CSystemInfoPluginEcomMain* self = new (ELeave) CSystemInfoPluginEcomMain( funcs->iNetscapeFuncs );
    CleanupStack::PushL( self );
    self->ConstructL( funcs->iPluginFuncs );
    CleanupStack::Pop( self );

    Dll::SetTls ( (void*) self );

    return self;
    }

// -----------------------------------------------------------------------------
// CSystemInfoPluginEcomMain::ConstructL
// -----------------------------------------------------------------------------
void CSystemInfoPluginEcomMain::ConstructL( NPPluginFuncs* aPluginFuncs )
    {
    InitializeFuncs( aPluginFuncs );
    }

// -----------------------------------------------------------------------------
// CSystemInfoPluginEcomMain::CSystemInfoPluginEcomMain
// -----------------------------------------------------------------------------
CSystemInfoPluginEcomMain::CSystemInfoPluginEcomMain( NPNetscapeFuncs* aNpf )
    : CEcomBrowserPluginInterface(),
      iNpf( aNpf )
    {
    }

// -----------------------------------------------------------------------------
// CSystemInfoPluginEcomMain::~CSystemInfoPluginEcomMain
// -----------------------------------------------------------------------------
CSystemInfoPluginEcomMain::~CSystemInfoPluginEcomMain()
    {
    }

// -----------------------------------------------------------------------------
// KImplementationTable
// -----------------------------------------------------------------------------
const TImplementationProxy KImplementationTable[] =
    {
    {{KFirstSystemInfoImplementationValue}, (TProxyNewLPtr)CSystemInfoPluginEcomMain::NewL}
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
    aPpf->newp          = NewNPP_NewProc(SystemInfoNewp);
    aPpf->destroy       = NewNPP_DestroyProc(SystemInfoDestroy);
    aPpf->setwindow     = 0;
    aPpf->newstream     = 0;
    aPpf->destroystream = 0;
    aPpf->asfile        = 0;
    aPpf->writeready    = 0;
    aPpf->write         = 0;
    aPpf->print         = 0;
    aPpf->event         = 0;
    aPpf->urlnotify     = 0;
    aPpf->javaClass     = 0;
    aPpf->getvalue      = NewNPP_GetValueProc(SystemInfoGetvalue);
    aPpf->setvalue      = 0;

    return NPERR_NO_ERROR;
    }

// -----------------------------------------------------------------------------
// NPP_Shutdown
// -----------------------------------------------------------------------------
EXPORT_C void NPP_Shutdown(void)
    {
    CSystemInfoPluginEcomMain* npm = (CSystemInfoPluginEcomMain*)Dll::Tls();
    delete npm;
    Dll::SetTls( NULL );
    }

//  End of File
