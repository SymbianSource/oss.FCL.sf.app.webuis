/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Encapsulates a edit folder dialog.
*
*/


#include <AknPopupFieldText.h>
#include <avkon.hrh>
#include <avkon.rsg>
#include <eikedwin.h>
#include <eikmenup.h>
#include <charconv.h>
#include <aknviewappui.h>

#ifdef __SERIES60_HELP
#include <hlplch.h>
#endif

#include "BrowserAppUi.h"
#include "BrowserAppViewBase.h"
#include "FeedsEditFolderDialog.h"

#include "Browser.hrh"
#include <BrowserNG.rsg>

// -----------------------------------------------------------------------------
// CFeedsEditFolderDialog::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeedsEditFolderDialog* CFeedsEditFolderDialog::NewL(MFeedsEditFolderDialogObserver& aObserver,
        CAknViewAppUi* aAppUi,
        const TDesC& aName)
    {
    CFeedsEditFolderDialog* self = new (ELeave) CFeedsEditFolderDialog(aObserver, aAppUi, aName);   
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);

    return self;
    }


// -----------------------------------------------------------------------------
// CFeedsEditFolderDialog::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeedsEditFolderDialog* CFeedsEditFolderDialog::NewL(MFeedsEditFolderDialogObserver& aObserver,
                                           CAknViewAppUi* aAppUi)
    {
    CFeedsEditFolderDialog* self = new (ELeave) CFeedsEditFolderDialog(aObserver, aAppUi, KNullDesC);   
    
    CleanupStack::PushL(self);
    self->iIsNewFolder = ETrue;
    self->ConstructL();
    CleanupStack::Pop(self);

    return self;
    }


// -----------------------------------------------------------------------------
// CFeedsEditFolderDialog::CFeedsEditFolderDialog
//
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CFeedsEditFolderDialog::CFeedsEditFolderDialog(MFeedsEditFolderDialogObserver& aObserver,
        CAknViewAppUi* aAppUi,
        const TDesC& aName):
    iObserver(aObserver), iAppUi(aAppUi), iName(aName)
    {
    }

    
// -----------------------------------------------------------------------------
// CFeedsEditFolderDialog::~CFeedsEditFolderDialog
//
// Deconstructor.
// -----------------------------------------------------------------------------
//
CFeedsEditFolderDialog::~CFeedsEditFolderDialog()
    {
    }


// -----------------------------------------------------------------------------
// CFeedsEditFolderDialog::PreLayoutDynInitL
//
// Called to setup the options menu with the dialog is active.
// -----------------------------------------------------------------------------
//
void CFeedsEditFolderDialog::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
    {
    CAknForm::DynInitMenuPaneL(aResourceId, aMenuPane);
    
    if (aResourceId == R_AVKON_FORM_MENUPANE)
        {
        aMenuPane->SetItemDimmed(EAknFormCmdLabel, ETrue);
        aMenuPane->SetItemDimmed(EAknFormCmdAdd, ETrue);
        aMenuPane->SetItemDimmed(EAknFormCmdDelete, ETrue);

        aMenuPane->AddMenuItemsL(R_FEEDS_HELP_EXIT_FEED_MENU);
        }
    }


// -----------------------------------------------------------------------------
// CFeedsEditFolderDialog::SaveFormDataL
//
// Called by the framework whenver the 'Save' menu item is selected.
// -----------------------------------------------------------------------------
//
TBool CFeedsEditFolderDialog::SaveFormDataL()
    {
    CEikEdwin*  textEditor;
    HBufC*      name = NULL;
    TBool       valid = EFalse;

    // Extract the new name.
    textEditor = static_cast<CEikEdwin*>(ControlOrNull(EFeedsEditDialogNameId));
    name = textEditor->GetTextInHBufL();
    CleanupStack::PushL(name);

    // Notify the observer.
    valid = iObserver.IsFolderNameValidL(name, !iIsNewFolder);

    // If invalid force the user to reentry it.
    if (!valid)
        {
        // Focus and select the name field.
        textEditor = static_cast<CEikEdwin*>(ControlOrNull(EFeedsEditDialogNameId));

        textEditor->SetFocus(ETrue);
        textEditor->SelectAllL();

        iExitDialog = EFalse;
        }

    // Otherwise pass the new values to the observer.
    else
        {
        if (iIsNewFolder)
            {
            iObserver.NewFolderL(*name);
            }
        else
            {
            iObserver.UpdateFolderL(*name);
            }

        iExitDialog = ETrue;
        }

    // Clean up.
    CleanupStack::PopAndDestroy(name);  

    return valid;
    }


// -----------------------------------------------------------------------------
// CFeedsEditFolderDialog::PreLayoutDynInitL
//
// Called by the framework before the form is initialised.
// -----------------------------------------------------------------------------
//
void CFeedsEditFolderDialog::PreLayoutDynInitL()
    {
    CEikEdwin*  textEditor = NULL;

    CAknForm::PreLayoutDynInitL();

    // Get the name field
    textEditor = static_cast<CEikEdwin*>(ControlOrNull(EFeedsEditDialogNameId));

    // If the name was set then set it.
    if (iName.Length() > 0)
        {
        textEditor->SetTextL(&iName);
        }

    // Otherwise use the default name.
    else
        {
    	HBufC*  text = NULL;
	
        // Load the label
        text = CCoeEnv::Static()->AllocReadResourceAsDes16L(R_FEEDS_NEW_FOLDER_ITEM);
        CleanupStack::PushL(text);

        textEditor->SetTextL(text);
  		SetFormFlag(EUnsavedEdit, ETrue);

        CleanupStack::PopAndDestroy(text);
        }
    }


// -----------------------------------------------------------------------------
// CFeedsEditFolderDialog::ProcessCommandL
//
// Called by the framework to process the options menu.
// -----------------------------------------------------------------------------
//
void CFeedsEditFolderDialog::ProcessCommandL(TInt aCommandId)
    {
    switch (aCommandId)
        {
        case EAknCmdExit:
            {
            // Custom added commands. Forward to view.
            // Note that we respond to EAknCmdExit, not CEikCmdExit; it's
            // because we don't get it through the AppUi framework (instead,
            // we forward it there now).
            CBrowserAppUi::Static()->ActiveView()->ProcessCommandL( aCommandId ); 
            break;
            }
                    
        case EEikCmdExit:
            break;

#ifdef __SERIES60_HELP
        case EAknCmdHelp:
            HlpLauncher::LaunchHelpApplicationL(iEikonEnv->WsSession(), 
                    iAppUi->AppHelpContextL());

            break;
#endif //__SERIES60_HELP 

        default:
            // Standard form commands.
            CAknForm::ProcessCommandL(aCommandId);

            if (iExitDialog)
                {
                TryExitL(EAknSoftkeyBack);
                }
            break;
        }
    }
