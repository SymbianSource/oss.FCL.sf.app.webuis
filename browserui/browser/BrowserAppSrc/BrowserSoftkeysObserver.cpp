/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Handle special load events such as network connection, deal with non-http or non-html requests
*
*/



// INCLUDE FILES

#include "BrowserSoftkeysObserver.h"
#include "BrowserWindow.h"
#include "BrowserWindowManager.h"
#include "BrowserContentView.h"

// ---------------------------------------------------------
// CBrowserSoftkeysObserver::NewL()
// ---------------------------------------------------------
//
CBrowserSoftkeysObserver* CBrowserSoftkeysObserver::NewL(
        CBrowserWindow& aWindow )
    {
    CBrowserSoftkeysObserver* self = new (ELeave)
        CBrowserSoftkeysObserver( aWindow );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();    // self
    return self;
    }

// ---------------------------------------------------------
// CBrowserSoftkeysObserver::~CBrowserSoftkeysObserver()
// ---------------------------------------------------------
//
CBrowserSoftkeysObserver::~CBrowserSoftkeysObserver()
    {
    }

// ---------------------------------------------------------
// CBrowserSoftkeysObserver::UpdateSoftkeyL()
// ---------------------------------------------------------
//
void CBrowserSoftkeysObserver::UpdateSoftkeyL(
        TBrCtlKeySoftkey /* aKeySoftkey */,
        const TDesC& /* aLabel */,
        TUint32 /* aCommandId */,
        TBrCtlSoftkeyChangeReason /* aReason */ )
    {
    // update only the active window's softkeys
    if( iWindow->IsWindowActive() )
        {
        CBrowserContentView* cv = iWindow->WindowMgr().ContentView();
        if( cv )
            {
            cv->UpdateCbaL();
            }
        }
    }

// ---------------------------------------------------------
// CBrowserSoftkeysObserver::CBrowserSoftkeysObserver()
// ---------------------------------------------------------
//
CBrowserSoftkeysObserver::CBrowserSoftkeysObserver(
        CBrowserWindow& aWindow ) :
    iWindow( &aWindow )
    {
    }

// ---------------------------------------------------------
// CBrowserSoftkeysObserver::ConstructL()
// ---------------------------------------------------------
//
void CBrowserSoftkeysObserver::ConstructL()
    {
    }

// End of File

