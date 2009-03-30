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
#include "GpsPlugin.h"
#include <implementationproxy.h>
#include <CEcomBrowserPluginInterface.h>


/*********************************************************/
/******** ECOM STYLE GPS PLUGIN ENTRY POINTS ********/
/*********************************************************/

// -----------------------------------------------------------------------------
// CGpsPluginEcomMain::NewL
// -----------------------------------------------------------------------------
CGpsPluginEcomMain* CGpsPluginEcomMain::NewL( TAny* aInitParam )
    {
    TFuncs* funcs = REINTERPRET_CAST( TFuncs*, aInitParam );
    CGpsPluginEcomMain* self = new (ELeave) CGpsPluginEcomMain( funcs->iNetscapeFuncs );
    CleanupStack::PushL( self );
    self->ConstructL( funcs->iPluginFuncs );
    CleanupStack::Pop( self );
    Dll::SetTls ( (void*)self );
    return self;
    }

// -----------------------------------------------------------------------------
// CGpsPluginEcomMain::ConstructL
// -----------------------------------------------------------------------------
void CGpsPluginEcomMain::ConstructL( NPPluginFuncs* aPluginFuncs )
    {
    InitializeFuncs( aPluginFuncs );
    }

// -----------------------------------------------------------------------------
// CGpsPluginEcomMain::CGpsPluginEcomMain
// -----------------------------------------------------------------------------
CGpsPluginEcomMain::CGpsPluginEcomMain( NPNetscapeFuncs* aNpf )
    : CEcomBrowserPluginInterface(),
      iNpf( aNpf )
    {
    }

// -----------------------------------------------------------------------------
// CGpsPluginEcomMain::~CGpsPluginEcomMain
// -----------------------------------------------------------------------------
CGpsPluginEcomMain::~CGpsPluginEcomMain()
    {
    }

// -----------------------------------------------------------------------------
// KImplementationTable
// -----------------------------------------------------------------------------
const TImplementationProxy KImplementationTable[] =
    {
    {{KFirstGpsImplementationValue}, (TProxyNewLPtr)CGpsPluginEcomMain::NewL}
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
    aPpf->newp          = NewNPP_NewProc(GpsNewp);
    aPpf->destroy       = NewNPP_DestroyProc(GpsDestroy);
    aPpf->setwindow     = NewNPP_SetWindowProc(GpsSetwindow);
    aPpf->newstream     = 0;
    aPpf->destroystream = 0;
    aPpf->asfile        = 0;
    aPpf->writeready    = 0;
    aPpf->write         = 0;
    aPpf->print         = 0;
    aPpf->event         = 0;
    aPpf->urlnotify     = 0;
    aPpf->javaClass     = 0;
    aPpf->getvalue      = NewNPP_GetValueProc(GpsGetvalue);
    aPpf->setvalue      = 0;

    return NPERR_NO_ERROR;
    }

// -----------------------------------------------------------------------------
// NPP_Shutdown
// -----------------------------------------------------------------------------
EXPORT_C void NPP_Shutdown(void)
    {
    CGpsPluginEcomMain* npm = (CGpsPluginEcomMain*)Dll::Tls();
    delete npm;
    Dll::SetTls( NULL );
    }

//  End of File
