/*
* Copyright (c) 2002 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Application class declaration.
*
*/



// INCLUDE FILES

#include "PushViewerApp.h"
#include "PushViewerDoc.h"
#include "PushViewerDef.h"
#include "PushViewerAppServer.h"
#include <eikstart.h>


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushViewerApplication::AppDllUid
// ---------------------------------------------------------
//
TUid CPushViewerApplication::AppDllUid() const
    {
    return KUidPushViewerApp;
    }

// ---------------------------------------------------------
// CPushViewerApplication::CreateDocumentL
// ---------------------------------------------------------
//
CApaDocument* CPushViewerApplication::CreateDocumentL()
    {
    return CPushViewerDocument::NewL( *this );
    }

// ---------------------------------------------------------
// CPushViewerApplication::NewAppServerL
// ---------------------------------------------------------
//
void CPushViewerApplication::NewAppServerL
    ( CApaAppServer*& aAppServer )
    {
    aAppServer = new (ELeave) CPushViewerAppServer;
    }

// ================= OTHER EXPORTED FUNCTIONS ==============

LOCAL_C CApaApplication* NewApplication()
    {
    return new CPushViewerApplication;
    }

GLDEF_C TInt E32Main()
    {
    return EikStart::RunApplication( NewApplication );
    }

