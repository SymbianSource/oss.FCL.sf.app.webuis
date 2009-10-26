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
#include <u32std.h>
#include <eikapp.h>
#include <eikstart.h>
#include "MemoryManager.h"
#include <OOMMonitorSession.h>

const int KFreeMemoryTarget = 6000000;

LOCAL_C CApaApplication* NewApplication( )
    {
    return new CBrowserApplication;
    }

EXPORT_C TInt UserHeap::SetupThreadHeap(TBool, SStdEpocThreadCreateInfo& aInfo)
	{
    TInt r = KErrNone;
    if (!aInfo.iAllocator && aInfo.iHeapInitialSize>0)
		{
		// new heap required
		RHeap* pH = NULL;
		r = CreateThreadHeap(aInfo, pH);
		if (r == KErrNone)
			{       // should happen for main thread, otherwise panic      
					RAllocator* oldAllocator = MemoryManager::SwitchToFastAllocator();
			}
		}
	else if (aInfo.iAllocator)
		{
		// sharing a heap
		RAllocator* pA = aInfo.iAllocator;
		pA->Open();
		User::SwitchAllocator(pA);
		}

    return r;
	}

GLDEF_C TInt E32Main()
    {
    ROomMonitorSession oomMs;
    TInt r = oomMs.Connect();
    if (r == KErrNone)
        {
        r = oomMs.RequestFreeMemory(KFreeMemoryTarget);
        oomMs.Close();
        }
    
	return EikStart::RunApplication(NewApplication);
    }

//  End of File  
