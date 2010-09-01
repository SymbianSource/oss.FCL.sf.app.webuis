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
* Description:  CVideoServicesAppUi implementation
*
*/


// INCLUDE FILES

#include "VideoServicesAppUi.h"
#include "VideoServicesContainer.h"

// CONSTANTS
_LIT(KUrlLaunchParameter, "4 ");
const TInt KUrlLaunchParamLength = 2;


// ================= MEMBER FUNCTIONS =======================
//

// ----------------------------------------------------------
// CVideoServicesAppUi::CVideoServicesAppUi()
// ----------------------------------------------------------
//
CVideoServicesAppUi::CVideoServicesAppUi()
	{	
	}

// ----------------------------------------------------------
// CVideoServicesAppUi::ConstructL()
// ----------------------------------------------------------
//
void CVideoServicesAppUi::ConstructL()
    {
    BaseConstructL( ENonStandardResourceFile | EAknEnableSkin /*| ENoScreenFurniture | ENoAppResourceFile*/ );
    iAppContainer = CVideoServicesContainer::NewL( ClientRect() );
    AddToStackL( iAppContainer );
    
	// Connect to VideoServices central repository, leave if we don't have
	// a VideoServices CR present
    iVideoServicesRepository = CRepository::NewL( KCrUidVideoServices );

  	LaunchBrowserL();
    }

// ----------------------------------------------------
// CVideoServicesAppUi::~CVideoServicesAppUi()
// ----------------------------------------------------
//
CVideoServicesAppUi::~CVideoServicesAppUi()
    {
    if ( iAppContainer )
        {
        RemoveFromStack( iAppContainer );
        delete iAppContainer;
		iAppContainer = NULL;
        }

    delete iVideoServicesRepository;
   }

// ----------------------------------------------------
// CVideoServicesAppUi::HandleKeyEventL(
// ----------------------------------------------------
//
TKeyResponse CVideoServicesAppUi::HandleKeyEventL( 
    const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/ )
    {
    return EKeyWasNotConsumed;
    }

// ----------------------------------------------------
// CVideoServicesAppUi::HandleCommandL(TInt aCommand)
// ----------------------------------------------------
//
void CVideoServicesAppUi::HandleCommandL( TInt aCommand )
    {
    switch ( aCommand )
        {
        case EAknSoftkeyBack:
		case EAknSoftkeyExit:
        case EEikCmdExit:
		case EAknCmdExit:
            {
            Exit();
            break;
            }
        default:
            break;      
        }
    }

// ----------------------------------------------------
// CVideoServicesAppUi::HandleContentL
// Handles the content coming from the embedded browser.
// Returns EFalse: content will be passed on to framework
// ----------------------------------------------------
//    
    TBool CVideoServicesAppUi::HandleContentL( const TDesC& /*aFileName*/, const CAiwGenericParamList& /*aParamList*/, TBool& aContinue )
    	{
    	aContinue = EFalse;
		return EFalse;
    	}                                      

// ----------------------------------------------------
// CVideoServicesAppUi::DownloadedContentHandlerReserved1
// ( Reserved for future use )
// ----------------------------------------------------
//
	TAny* CVideoServicesAppUi::DownloadedContentHandlerReserved1( TAny* /*aAnyParam*/ )
		{
		// Empty implementation.		
		return NULL;
		}

// ----------------------------------------------------
// CVideoServicesAppUi::LaunchBrowserL()
// ----------------------------------------------------
//
void CVideoServicesAppUi::LaunchBrowserL() 
    {
	// Read the video URL from Video Services central repository
    if ( KErrNone == iVideoServicesRepository->Get( KVideoServicesUrl, iUrl ) )
        {
	    if ( iUrl.Length() )
    		{
    		LaunchBrowserEmbeddedWithVideoUrlL();
    		} 
        }
    Exit();
    }

// ----------------------------------------------------
// CVideoServicesAppUi::LaunchBrowserEmbeddedWithVideoUrlL
// ----------------------------------------------------
//
void CVideoServicesAppUi::LaunchBrowserEmbeddedWithVideoUrlL()
{
	// Pass the specified video url to browser (read from Central Repository)
    HBufC* url = NULL;
	url = HBufC::NewLC( iUrl.Length() + KUrlLaunchParamLength );
	url->Des().Append( KUrlLaunchParameter );
	url->Des().Append( iUrl );
	
	CBrowserLauncher* launcher = CBrowserLauncher::NewLC();
    launcher->LaunchBrowserSyncEmbeddedL( url->Des(), this );

	CleanupStack::PopAndDestroy();  // launcher
	CleanupStack::PopAndDestroy();  // url
}

// ----------------------------------------------------
// CVideoServicesAppUi::NotifyExit
// ----------------------------------------------------
//
void CVideoServicesAppUi::NotifyExit(TExitMode /*aMode*/)
{
}

// ----------------------------------------------------
// CVideoServicesAppUi::HandleStatusPaneSizeChange
// ----------------------------------------------------
//
void CVideoServicesAppUi::HandleStatusPaneSizeChange()
{
   iAppContainer->SetRect( ClientRect() );
} 

// End of File
