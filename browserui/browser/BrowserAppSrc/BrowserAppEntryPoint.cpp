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
#include <MemoryManager.h>
#include <avkon.rsg>
#include <BrowserNG.rsg>
#include <bautils.h>
#include <f32file.h>
#include <data_caging_path_literals.hrh>
#include <hal.h>
#include <barsc.h>

_LIT(KBrowserResourceFile, "BrowserNG.rsc");  


const int KFreeMemoryTarget = 6000000;


LOCAL_C CApaApplication* NewApplication( )
    {
    return new CBrowserApplication; 
    }

// -----------------------------------------------------------------------------
// SetupThreadHeap - Called for heap creation of thread in this process.
// This approach used to keep correct heap for pointers held in static data objects
// when they are destructed after E32Main() by OS.
// -----------------------------------------------------------------------------
EXPORT_C TInt UserHeap::SetupThreadHeap(TBool aSubThread, SStdEpocThreadCreateInfo& aInfo)
    {
    TInt r = KErrNone;
    if (!aInfo.iAllocator && aInfo.iHeapInitialSize>0)
        {
        // new heap required
        RHeap* pH = NULL;
        r = CreateThreadHeap(aInfo, pH);
        if (r == KErrNone && !aSubThread)
            {
            // main thread - new allocator created and set as default heap      
            MemoryManager::CreateFastAllocator();
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


// ---------------------------------------------------------------------------
// GetUiUtilitiesResourceFilenameL
// ---------------------------------------------------------------------------
//
TInt GetCorrectResourceFilenameL(TFileName& aResourceFileName)
    {

    TParse parse;
    parse.Set(KBrowserResourceFile, &KDC_APP_RESOURCE_DIR, NULL);
    aResourceFileName.Copy(parse.FullName());
    
    RFs fs;
    User::LeaveIfError(fs.Connect());
    CleanupClosePushL( fs );
    
    // Resource File Name on C:
    aResourceFileName.Insert( 0, TDriveUnit( EDriveC).Name() );
    // try to locate the localised resource
    BaflUtils::NearestLanguageFile( fs, aResourceFileName ); 
    
    // if the localised resource is found, the file name is changed to the localised name (z:\resources\webkit.r001)
    if( !BaflUtils::FileExists( fs, aResourceFileName ) )        {
    	
        aResourceFileName.Replace( 0, KMaxDriveName, TDriveUnit( EDriveZ ).Name() );
        // not found on c drive, try z try to locate the localised resource again
        BaflUtils::NearestLanguageFile( fs, aResourceFileName ); 
        	
        // if file was not found this time, there is no localised  resource with the name
        if (!BaflUtils::FileExists( fs, aResourceFileName ) )             {
            User::Leave(KErrNotFound);
            }
        }
    CleanupStack::PopAndDestroy( &fs );
    return KErrNone;
    }

void ShowOOMDialogWithNotifierL()
    {
    
    TFileName resourceFileName;
    User::LeaveIfError(GetCorrectResourceFilenameL(resourceFileName));

    RFs fsSession;
    User::LeaveIfError(fsSession.Connect());
    CleanupClosePushL(fsSession);

    RResourceFile resFile;
    resFile.OpenL(fsSession,resourceFileName );
    resFile.ConfirmSignatureL(0);

    HBufC8* dataOkButtonBuffer = resFile.AllocReadLC(R_QTN_OK_BUTTON);
    HBufC8* dataOOMMessageBuffer = resFile.AllocReadLC(R_QTN_BROWSER_DIALOG_OOM);
    
    TPtrC16 OkButtonBuffer( ( TUint16* ) dataOkButtonBuffer->Ptr(),dataOkButtonBuffer->Length() / 2 );
    TPtrC16 OOMMessageBuffer( ( TUint16* ) dataOOMMessageBuffer->Ptr(),dataOOMMessageBuffer->Length() / 2 );

    _LIT(KTxtEmptyLine,"   ");
    _LIT(KTxtEmptyButton,"");
    
    RNotifier      notifier;
    TInt err = notifier.Connect();
    if(err == KErrNone)   {
            TInt           whichbutton(-1);
            TRequestStatus stat;
            notifier.Notify(OOMMessageBuffer,KTxtEmptyLine,OkButtonBuffer,KTxtEmptyButton,whichbutton,stat);
            User::WaitForRequest(stat);
        }    
    
    notifier.Close();
    
    //this will clear OkButtonBuffer and OOMMessageBuffer 
    CleanupStack::PopAndDestroy(2); 
    resFile.Close();
    CleanupStack::PopAndDestroy(&fsSession);
    
    }

// -----------------------------------------------------------------------------
// E32Main
// -----------------------------------------------------------------------------
GLDEF_C TInt E32Main()
    { 
    //Initilize memory manager...
    MemoryManager::InitFastAllocator();   
	
    TInt freeRAM = 0;
    TInt r = HAL::Get(HAL::EMemoryRAMFree, freeRAM);
        
    if ((r != KErrNone)  || freeRAM < KFreeMemoryTarget)  
    	{
    		// create clean-up stack as none is created
            CTrapCleanup* cleanup=CTrapCleanup::New(); 
			
            // Show OOM message through RNotifier ...
             TRAP_IGNORE(ShowOOMDialogWithNotifierL()); 
            
			// destroy clean-up stack
            delete cleanup; 
            return KErrNoMemory;
        }
    
     // Run application event loop
	 return EikStart::RunApplication(NewApplication);
    }

//  End of File  
