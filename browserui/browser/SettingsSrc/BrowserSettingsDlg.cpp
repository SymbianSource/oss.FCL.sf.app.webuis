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
*      Dialog for setting the gateway. It's used temporarily.
*     
*
*/


// INCLUDE FILES
// System includes
#include <avkon.hrh>
#include <eikdef.h>
#include <eikenv.h>
#include <eikedwin.h>
#include <BrowserNG.rsg>

// User includes
#include "BrowserSettingsDlg.h"
#include "Browser.hrh"
#include "BrowserUtil.h"

// ================= MEMBER FUNCTIONS =========================================

// ---------------------------------------------------------
// CBrowserSettingsDlg::ConstructL()
// ---------------------------------------------------------
//
void CBrowserSettingsDlg::ConstructL()
	{
	CAknDialog::ConstructL( R_BROWSER_SETTINGS_MENUBAR );
	}

// ---------------------------------------------------------
// CBrowserSettingsDlg::NewL()
// ---------------------------------------------------------
//
CBrowserSettingsDlg* CBrowserSettingsDlg::NewL()
	{
	CBrowserSettingsDlg* self = new (ELeave) CBrowserSettingsDlg;
	CleanupStack::PushL( self );
	self->ConstructL();
	CleanupStack::Pop();
	return self;
	}

// ---------------------------------------------------------
// CBrowserSettingsDlg::OkToExitL
// Good to know : EAknSoftkeyCancel is never called, because
// EEikDialogFlagNotifyEsc flag is not set in the resource.
// ---------------------------------------------------------
//
TBool CBrowserSettingsDlg::OkToExitL( TInt aButtonId )
	{
	TBool result( EFalse );

	if ( aButtonId == EAknSoftkeyOk )
		{
		TBuf<256> buf;

		STATIC_CAST( CEikEdwin*, Control(
					EWmlControlSettingsEditGateway ) )->GetText( buf );
		buf.ZeroTerminate();

        result = ETrue;
		}
	return result;
	}

//  End of File
