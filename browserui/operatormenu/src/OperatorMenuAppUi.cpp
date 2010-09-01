/*
* Copyright (c) 2003 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  COperatorMenuAppUi implementation
*
*/


// INCLUDE FILES
#include "OperatorMenuAppUi.h"

#include <featmgr.h>
#include <bldvariant.hrh>
#include <FeatMgr.h>

#include <centralrepository.h>
#include <menu2internalCRkeys.h>
// CONSTANTS
_LIT(KUrlLaunchParameter, "4 ");
_LIT(KLongZeroLaunchParameter, "5");
const TInt KUrlLaunchParamLength = 2;

const TInt KCenRepBufferSize = 255;
const TText KStrComma = ',';
#define KOperatorMenuUID 0x10008D5E

// ================= LOCAL FUNCTIONS ========================

// ----------------------------------------------------------------------------
// OperatorMenuUIDText
//
// @param aOperatorMenuUid buffer to set OperatorMenu UID as text
// ----------------------------------------------------------------------------
//
void OperatorMenuUIDText( TDes& aOperatorMenuUid )
    {
    // TUid method name return UID in format [UID], but we need only UID part.
    TUidName uidName( TUid::Uid( KOperatorMenuUID ).Name() );
    aOperatorMenuUid.Copy( uidName.Mid( 1, KMaxUidName - 2 ) );
    aOperatorMenuUid.UpperCase();
    }

// ----------------------------------------------------------------------------
// SetOperatorMenuHiddenInAppShellL
//
// @param aHidden Whether OperatorMenu should be hidden
// ----------------------------------------------------------------------------
//
void SetOperatorMenuHiddenInAppShellL( TBool aHidden )
    {
    CRepository* appShellRepository = CRepository::NewL( KCRUidMenu );
    CleanupStack::PushL( appShellRepository );

    // Read current value of the app shell hidden apps key
    TBuf<KCenRepBufferSize> keyContent;
    TInt err = appShellRepository->Get( KMenuHideApplication, keyContent );
    if ( err != KErrNone )
        {
        User::Leave( err );
        }

    TBool updated( EFalse );

    TBuf<8> OperatorMenuUid;
    OperatorMenuUIDText( OperatorMenuUid );
    
    // Search for PoC uid in the content (ignore case)
    TInt offset = keyContent.FindC( OperatorMenuUid );
    if ( offset == KErrNotFound && aHidden )
        {
        // PoC is not in the list but should be
        //
        __ASSERT_ALWAYS( keyContent.Length() + 9 < KCenRepBufferSize,
                         User::Leave( KErrOverflow ) );
        if ( keyContent.Length() > 0 )
            {
            keyContent.Append( KStrComma );
            }
        keyContent.Append( OperatorMenuUid );
        updated = ETrue;
        }
    else if ( offset != KErrNotFound && !aHidden )
        {
        // PoC is in the list but shouldn't be
        //
        if ( offset == 0 )
            {
            // Delete PoC uid and following comma (if any)
            keyContent.Delete( 0, Min( 9, keyContent.Length() ) );
            updated = ETrue;
            }
        else if ( keyContent[offset - 1] == ',' )
            {
            // Delete PoC uid and preceding comma
            keyContent.Delete( offset - 1, 9 );
            updated = ETrue;
            }
        else
            {
            User::Leave( KErrCorrupt );
            }
        }

    if ( updated )
        {
        err = appShellRepository->Set( KMenuHideApplication, keyContent );
        if ( err != KErrNone )
            {
            User::Leave( err );
            }
        }

    CleanupStack::PopAndDestroy( appShellRepository );
    }

// ----------------------------------------------------------
// COperatorDelayedStarter::COperatorDelayedStarter(COperatorMenuAppUi* aOperatorMenuAppUi)
// ----------------------------------------------------------
//
COperatorDelayedStarter::COperatorDelayedStarter(COperatorMenuAppUi* aOperatorMenuAppUi)
:   CActive( EPriorityLow )
	{
	CActiveScheduler::Add( this );
	iOperatorMenuAppUi = aOperatorMenuAppUi;
	}

// ----------------------------------------------------------
// COperatorDelayedStarter::~COperatorDelayedStarter()
// ----------------------------------------------------------
//
COperatorDelayedStarter::~COperatorDelayedStarter()
	{
	Cancel();
	Deque();
	}

// ----------------------------------------------------------
// COperatorDelayedStarter::Start()
// ----------------------------------------------------------
//
void COperatorDelayedStarter::Start()
	{
	SetActive();
	TRequestStatus* reqStat= &iStatus;
	User::RequestComplete( reqStat, KErrNone );
	}

// ----------------------------------------------------------
// COperatorDelayedStarter::RunL()
// ----------------------------------------------------------
//
void COperatorDelayedStarter::RunL()
	{
	iOperatorMenuAppUi->LaunchBrowserL();
	}

// ----------------------------------------------------------
// COperatorDelayedStarter::
// ----------------------------------------------------------
//
void COperatorDelayedStarter::DoCancel()
	{
	}

// ================= MEMBER FUNCTIONS =======================
//

// ----------------------------------------------------------
// COperatorMenuAppUi::COperatorMenuAppUi()
// ----------------------------------------------------------
//
COperatorMenuAppUi::COperatorMenuAppUi()
	{	
	}

// ----------------------------------------------------------
// COperatorMenuAppUi::ConstructL()
// ----------------------------------------------------------
//
void COperatorMenuAppUi::ConstructL()
    {
    BaseConstructL(/* ENoScreenFurniture |*/ ENonStandardResourceFile /*| ENoAppResourceFile | EAknEnableSkin*/ );
    
	// Connect to HttpCacheManager central repository
    iRepository = CRepository::NewL( KCRUidCacheManager );
	// Connect to OperatorMenu central repository
    iRepositoryOp = CRepository::NewL( KCrUidOperatorMenu );

	//Launch the browser with the Operator specific Url
	//or with default AP's homepage if Operator domain url not available
	//LaunchBrowserL();
	iLaunched = EFalse;
    
    TBool OperatorMenuSupport = FeatureManager::FeatureSupported( KFeatureIdOperatorMenu );
    if ( OperatorMenuSupport )
        {
        iDelayedStarter = new (ELeave) COperatorDelayedStarter( this );
	    iDelayedStarter->Start();
        }
    else
        {
        SetOperatorMenuHiddenInAppShellL( ETrue );
		Exit();
		}
	}
// ----------------------------------------------------
// COperatorMenuAppUi::~COperatorMenuAppUi()
// ----------------------------------------------------
//
COperatorMenuAppUi::~COperatorMenuAppUi()
    {
    delete iRepository;
    delete iRepositoryOp;

	if( iOverriddenSettings )
		{
		delete iOverriddenSettings;
		iOverriddenSettings = NULL;
		}
	if( iDelayedStarter )
		{
		delete iDelayedStarter;
		iDelayedStarter = NULL;
		}
   }


// ----------------------------------------------------
// COperatorMenuAppUi::HandleKeyEventL(
// ----------------------------------------------------
//
TKeyResponse COperatorMenuAppUi::HandleKeyEventL( 
    const TKeyEvent& /*aKeyEvent*/,TEventCode /*aType*/ )
    {
    return EKeyWasNotConsumed;
    }

// ----------------------------------------------------
// COperatorMenuAppUi::HandleCommandL(TInt aCommand)
// ----------------------------------------------------
//
void COperatorMenuAppUi::HandleCommandL( TInt aCommand )
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
// COperatorMenuAppUi::HandleContentL
// Handles the content coming from the embedded browser.
// Returns EFalse: content will be passed on to framework
// ----------------------------------------------------
//    
TBool COperatorMenuAppUi::HandleContentL( const TDesC& /*aFileName*/, const CAiwGenericParamList& /*aParamList*/, TBool& aContinue )
	{
	aContinue = EFalse;
	return EFalse;
	}                                      

// ----------------------------------------------------
// COperatorMenuAppUi::DownloadedContentHandlerReserved1
// ( Reserved for future use )
// ----------------------------------------------------
//
TAny* COperatorMenuAppUi::DownloadedContentHandlerReserved1( TAny* /*aAnyParam*/ )
	{
	// Empty implementation.		
	return NULL;
	}

// ----------------------------------------------------
// COperatorMenuAppUi::LaunchBrowserL()
// ----------------------------------------------------
//
void COperatorMenuAppUi::LaunchBrowserL() 
    {
	TBool LaunchWithUrl( EFalse );
    TInt value;

	if( !iLaunched )
	    {
	    iLaunched = ETrue;
	    
	    //If centralrepository of HttpCahemanager is available...
        if ( iRepository )
	        {
    		// Read the URL from CacheManager central repository
	        if ( KErrNone == iRepository->Get( KOperatorDomainUrl, iUrl ) )
				{
				LaunchWithUrl = ETrue;
				}
	        }

	    //If centralrepository of OperatorMenu is available...
        if ( iRepositoryOp )
	        {
            delete iOverriddenSettings;
            iOverriddenSettings = NULL;

            iOverriddenSettings = new (ELeave) TBrowserOverriddenSettings;

            //Read the values to override
            if ( KErrNone == iRepositoryOp->Get( KOpMenuVerticalLayoutEnabled, value ) )
                {
                iOverriddenSettings->SetBrowserSetting( EBrowserOverSettingsSmallScreen, (TUint) value);
                }
            if ( KErrNone == iRepositoryOp->Get( KOpMenuAutoLoadImages, value ) )
                {
                iOverriddenSettings->SetBrowserSetting( EBrowserOverSettingsAutoLoadImages, (TUint) value);
                }
            if ( KErrNone == iRepositoryOp->Get( KOpMenuFontSize, value ) )
                {
                iOverriddenSettings->SetBrowserSetting( EBrowserOverSettingsFontSize, (TUint) value);
                }
            if ( KErrNone == iRepositoryOp->Get( KOpMenuFullScreen, value ) )
                {
                iOverriddenSettings->SetBrowserSetting( EBrowserOverSettingsFullScreen, (TUint) value);
                }
            if ( KErrNone == iRepositoryOp->Get( KOpMenuDefAp, value ) )
	            {
                iOverriddenSettings->SetBrowserSetting( EBrowserOverSettingsCustomAp, (TUint) value);
	            }
            }
        }

	// Launch browser according to the given launch type
	if( LaunchWithUrl && iUrl.Length() )
		{
		LaunchBrowserEmbeddedWithOperatorUrlL();
		} 
	else
		{
		LaunchBrowserEmbeddedWithDefaultAPHomepageL();
		}
    Exit();
    }

// ----------------------------------------------------
// COperatorMenuAppUi::LaunchBrowserEmbeddedWithUrlL
// ----------------------------------------------------
//
void COperatorMenuAppUi::LaunchBrowserEmbeddedWithOperatorUrlL()
	{
	//Pass the operator specific url to browser
	//(read from cachemanager ini file in LaunchBrowserL)
    HBufC* url = NULL;
	url = HBufC::NewLC( iUrl.Length() + KUrlLaunchParamLength );
	url->Des().Append( KUrlLaunchParameter );
	url->Des().Append( iUrl );
	
	CBrowserLauncher* launcher = CBrowserLauncher::NewLC();

    launcher->LaunchBrowserSyncEmbeddedL( url->Des(), this, iOverriddenSettings );

	CleanupStack::PopAndDestroy(); //Launcher
	CleanupStack::PopAndDestroy();  // url HBufC
	}

// ----------------------------------------------------
// COperatorMenuAppUi::NotifyExit
// ----------------------------------------------------
//
void COperatorMenuAppUi::NotifyExit(TExitMode /*aMode*/)
	{
	}

void COperatorMenuAppUi::LaunchBrowserEmbeddedWithDefaultAPHomepageL()
	{
	//Start the browser with parameter "5" to launch to default AP homepage
	CBrowserLauncher* launcher = CBrowserLauncher::NewLC();

    launcher->LaunchBrowserSyncEmbeddedL( KLongZeroLaunchParameter, this, iOverriddenSettings );

	CleanupStack::PopAndDestroy();
	}

// ----------------------------------------------------
// COperatorMenuAppUi::HandleStatusPaneSizeChange
// ----------------------------------------------------
//
void COperatorMenuAppUi::HandleStatusPaneSizeChange()
	{
	}
// End of File  

