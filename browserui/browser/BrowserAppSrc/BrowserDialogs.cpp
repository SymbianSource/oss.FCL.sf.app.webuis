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
*      Helper class that pops up several dialogs.
*      
*
*/


// INCLUDE FILES

// System includes
#include <aknnotewrappers.h>
#include <BrowserNG.rsg>
#include <StringLoader.h>
#include <AknNoteDialog.h>

// User includes
#include "BrowserDialogs.h"

// ================= MEMBER FUNCTIONS =========================================
// ----------------------------------------------------------------------------
// TBrowserDialogs::DialogPromptReqL
// ----------------------------------------------------------------------------
//
TInt TBrowserDialogs::DialogPromptReqL( const TDesC& aMsg, 
                                        TDes* aResp, 
                                        TBool aIsUrlAddress,
                                        TInt aMaxLength
                                        )
	{
	TPtrC msg( aMsg );
	CAknTextQueryDialog* dialog = new( ELeave )CAknTextQueryDialog( *aResp, msg );
	if( aMaxLength )
	    {
		dialog->SetMaxLength(aMaxLength);
	    }
    TInt resource;
    if ( aIsUrlAddress )
        {
        resource = R_WB_SCRIPT_PROMPT_URL_ADDRESS;
        }
    else
        {
        resource = R_WB_SCRIPT_PROMPT_LINES;
        }
    dialog->SetPredictiveTextInputPermitted(ETrue);
	return dialog->ExecuteLD( resource );
	}


// ----------------------------------------------------------------------------
// TBrowserDialogs::DialogQueryReqL
// ----------------------------------------------------------------------------
//
TInt TBrowserDialogs::DialogQueryReqL(  const TDesC& aMsg, 
                                        const TDesC& aYesMessage, 
                                        const TDesC& aNoMessage )
	{
	TPtrC msg(aMsg);
	CAknQueryDialog* dialog = 
            new ( ELeave ) CAknQueryDialog( msg, CAknQueryDialog::ENoTone );
	
    dialog->PrepareLC( R_WB_SCRIPT_QUERY_LINES );

    if( aYesMessage.Length() )
        {
        dialog->ButtonGroupContainer().SetCommandL(EAknSoftkeyYes, aYesMessage );
        }

    if( aNoMessage.Length() )
        {
        dialog->ButtonGroupContainer().SetCommandL(EAknSoftkeyNo, aNoMessage );
        }

    return dialog->RunLD();
	}

// ----------------------------------------------------------------------------
// TBrowserDialogs::ConfirmQueryDialogL
// ----------------------------------------------------------------------------
//
TInt TBrowserDialogs::ConfirmQueryDialogL( const TInt aPromptResourceId, 
                                            TInt aResId, TInt aAnimation )
    {
    HBufC* prompt = StringLoader::LoadLC( aPromptResourceId );
    TInt ret = ConfirmQueryDialogL( *prompt, aResId, aAnimation );
    CleanupStack::PopAndDestroy();  // prompt
    return ret;
    }



// ----------------------------------------------------------------------------
// TBrowserDialogs::ConfirmQueryDialogL
// ----------------------------------------------------------------------------
//
TInt TBrowserDialogs::ConfirmQueryDialogL(  const TDesC& aPrompt, 
                                            TInt aResId, 
                                            TInt aAnimation )
    {
    CAknQueryDialog* dialog = new (ELeave) CAknQueryDialog();

	// TO DO: Store dialog pointer for CBrowserDialogsProvider::CancelAll()

    dialog->PrepareLC( R_BROWSER_CONFIRM_QUERY );
    // passing prompt text in constructor is depracated according to 
    // AknQueryDialog.h
    dialog->SetPromptL( aPrompt );
    if( aResId )
        {
        dialog->ButtonGroupContainer().SetCommandSetL( aResId );
        }

    if( aAnimation )
        {
        CAknQueryControl* control = STATIC_CAST( CAknQueryControl*, dialog->Control(EGeneralQuery) );
        control->SetAnimationL( aAnimation );
        }

    return dialog->RunLD();
    }


// ---------------------------------------------------------
// TBrowserDialogs::ConfirmQueryYesNoL
// ---------------------------------------------------------
//
TBool TBrowserDialogs::ConfirmQueryYesNoL( TDesC& aPrompt )
    {
    return ConfirmQueryDialogL( aPrompt );
    }


// ----------------------------------------------------------------------------
// TBrowserDialogs::ConfirmQueryYesNoL
// ----------------------------------------------------------------------------
//
TBool TBrowserDialogs::ConfirmQueryYesNoL( const TInt aPromptResourceId )
    {
    HBufC* prompt = StringLoader::LoadLC( aPromptResourceId );
    TBool ret = ConfirmQueryYesNoL( *prompt );
	CleanupStack::PopAndDestroy();  // prompt
    return ret;
    }


// ----------------------------------------------------------------------------
// TBrowserDialogs::ConfirmQueryYesNoL
// ----------------------------------------------------------------------------
//
TBool TBrowserDialogs::ConfirmQueryYesNoL
						( const TInt aPromptResourceId, const TDesC& aString )
    {
    HBufC* prompt = StringLoader::LoadLC( aPromptResourceId, aString );
    TBool ret = ConfirmQueryYesNoL( *prompt );
    CleanupStack::PopAndDestroy();  // prompt
    return ret;
    }


// ----------------------------------------------------------------------------
// TBrowserDialogs::ConfirmQueryYesNoL
// ----------------------------------------------------------------------------
//
TBool TBrowserDialogs::ConfirmQueryYesNoL
							( const TInt aPromptResourceId, const TInt aNum )
    {
    HBufC* prompt = StringLoader::LoadLC( aPromptResourceId, aNum );
    TBool ret = ConfirmQueryYesNoL( *prompt );
	CleanupStack::PopAndDestroy();  // prompt
    return ret;
    }


// ----------------------------------------------------------------------------
// TBrowserDialogs::InfoNoteL
// ----------------------------------------------------------------------------
//
void TBrowserDialogs::InfoNoteL( TInt aDialogResourceId, 
													const TDesC& aPrompt )
    {
    CAknNoteDialog* dialog = new (ELeave) CAknNoteDialog
								( REINTERPRET_CAST( CEikDialog**, &dialog ) );
    dialog->PrepareLC( aDialogResourceId ) ;
    dialog->SetCurrentLabelL( EGeneralNote, aPrompt );
    dialog->RunDlgLD( CAknNoteDialog::EShortTimeout , CAknNoteDialog::ENoTone );
    }


// ----------------------------------------------------------------------------
// TBrowserDialogs::InfoNoteL
// ----------------------------------------------------------------------------
//
void TBrowserDialogs::InfoNoteL
( TInt aDialogResourceId, const TInt aPromptResourceId )
    {
    HBufC* prompt = StringLoader::LoadLC( aPromptResourceId );
    InfoNoteL( aDialogResourceId, *prompt );
    CleanupStack::PopAndDestroy();  // prompt
    }


// ----------------------------------------------------------------------------
// TBrowserDialogs::InfoNoteL
// ----------------------------------------------------------------------------
//
void TBrowserDialogs::InfoNoteL ( TInt aDialogResourceId, 
                                    const TInt aPromptResourceId, 
                                    const TDesC& aString )
    {
    HBufC* prompt = StringLoader::LoadLC( aPromptResourceId, aString );
    InfoNoteL( aDialogResourceId, *prompt );
    CleanupStack::PopAndDestroy();  // prompt
    }


// ----------------------------------------------------------------------------
// TBrowserDialogs::InfoNoteL
// ----------------------------------------------------------------------------
//
void TBrowserDialogs::InfoNoteL
( TInt aDialogResourceId, const TInt aPromptResourceId, const TInt aNum )
    {
    HBufC* prompt = StringLoader::LoadLC( aPromptResourceId, aNum );
    InfoNoteL( aDialogResourceId, *prompt );
    CleanupStack::PopAndDestroy();  // prompt
    }


// ----------------------------------------------------------------------------
	// TBrowserDialogs::ErrorNoteL
// ----------------------------------------------------------------------------
//
void TBrowserDialogs::ErrorNoteL( const TInt aPromptResourceId )
    {
    HBufC* prompt = StringLoader::LoadLC( aPromptResourceId );
    ErrorNoteL( *prompt );
    CleanupStack::PopAndDestroy();  // prompt
    }


// ----------------------------------------------------------------------------
// TBrowserDialogs::ErrorNoteL
// ----------------------------------------------------------------------------
//
void TBrowserDialogs::ErrorNoteL( const TDesC& aPrompt )
    {
    CAknErrorNote* note = new ( ELeave ) CAknErrorNote();
    note->ExecuteLD( aPrompt );
    }

//  End of File
