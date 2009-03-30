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
* Description: 
*     Browser App Entry point
*     
*
*/


#include "BrowserApplication.h"
#include <e32std.h>
#include <eikapp.h>
#include <eikstart.h>
#include "MemoryManager.h"
#include <OOMMonitorSession.h>

const int KFreeMemoryTarget = 6000000;

LOCAL_C CApaApplication* NewApplication( )
    {
    return new CBrowserApplication;
    }

GLDEF_C TInt E32Main()
    {
    ROomMonitorSession oomMs;
    TInt result = oomMs.Connect();
    if ( result == KErrNone )
        {
        result = oomMs.RequestFreeMemory( KFreeMemoryTarget );
        oomMs.Close();
        if ( result == KErrNone )
            {
            RAllocator* oldAllocator = MemoryManager::SwitchToFastAllocator();
            result = EikStart::RunApplication( NewApplication );
            MemoryManager::CloseFastAllocator(oldAllocator);
            }
        }
    return result;
    }

//  End of File  
