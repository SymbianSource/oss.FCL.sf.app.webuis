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
* Description:  Encapsulates a edit feed dialog.
*
*/


#include <AknPopupFieldText.h>
#include <aknslider.h>
#include <avkon.hrh>
#include <avkon.rsg>
#include <eikedwin.h>
#include <eikmenup.h>
#include <charconv.h>
#include <aknviewappui.h>
#include <StringLoader.h>

#ifdef __SERIES60_HELP
// Context-Sensitve Help File
#include <hlplch.h>
#include <csxhelp/browser.hlp.hrh>
#include "BrowserApplication.h"
#endif // __SERIES60_HELP

#include "BrowserAppUi.h"
#include "BrowserAppViewBase.h"
#include "Browser.hrh"
#include <BrowserNG.rsg>
#include <AknRadioButtonSettingPage.h>

#include "FeedsEditFeedDialog.h"

//Constants
const TInt KAutoUpdatingOff = 0;
const TInt KFifteen = 15;
const TInt KOneHour = 60;
const TInt KFourHour = 240;
const TInt KDay = 1440;
const TInt KWeek = 10080;

// -----------------------------------------------------------------------------
// CFeedsEditFeedDialog::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeedsEditFeedDialog* CFeedsEditFeedDialog::NewL(MFeedsEditFeedDialogObserver& aObserver,
        CAknViewAppUi* aAppUi,
        const TDesC& aName, const TDesC& aUrl, TInt aFreq) 
    {
    CFeedsEditFeedDialog* self = new (ELeave) CFeedsEditFeedDialog(aObserver, aAppUi, aName, aUrl, aFreq);   
    CleanupStack::PushL(self);
    self->ConstructL();
    self->iSelectedDlgLine = 0;
    CleanupStack::Pop(self);

    return self;
    }


// -----------------------------------------------------------------------------
// CFeedsEditFeedDialog::NewL
//
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeedsEditFeedDialog* CFeedsEditFeedDialog::NewL(MFeedsEditFeedDialogObserver& aObserver, CAknViewAppUi* aAppUi)
    {
    CFeedsEditFeedDialog* self = new (ELeave) CFeedsEditFeedDialog(aObserver, aAppUi, KNullDesC, KNullDesC,0);   
    
    CleanupStack::PushL(self);
    self->iIsNewFeed = ETrue;
    self->ConstructL();
    CleanupStack::Pop(self);

    return self;
    }


// -----------------------------------------------------------------------------
// CFeedsEditFeedDialog::CFeedsEditFeedDialog
//
// C++ default constructor.
// -----------------------------------------------------------------------------
//
CFeedsEditFeedDialog::CFeedsEditFeedDialog (MFeedsEditFeedDialogObserver& aObserver,
        CAknViewAppUi* aAppUi,
        const TDesC& aName, const TDesC& aUrl, TInt aFreq)
    :iObserver(aObserver), iAppUi(aAppUi), iName(aName), iUrl(aUrl),iFreq(aFreq), iActionCancelled(EFalse)
    {
    iPreviousFreq = iFreq;
    }



// ----------------------------------------------------
// CFeedsEditFeedDialog::OfferKeyEventL
// Keyevent process.
// ----------------------------------------------------
//
TKeyResponse CFeedsEditFeedDialog::OfferKeyEventL(const TKeyEvent& aKeyEvent,
                                      TEventCode aType)
    {
    if ((aKeyEvent.iScanCode == EStdKeyDevice3) || (aKeyEvent.iScanCode == EStdKeyEnter))
        {
        if ((aType == EEventKey) && (iSelectedDlgLine == EFeedsEditAutomaticUpdatingId))
            {         
            HandleFreqCmdL();
            return EKeyWasConsumed;
            }
        }
    return CAknForm::OfferKeyEventL(aKeyEvent, aType);
    }    

// ---------------------------------------------------------
// CFeedsEditFeedDialog::HandlePointerEventL
// ---------------------------------------------------------
//
void CFeedsEditFeedDialog::HandlePointerEventL(const TPointerEvent& aPointerEvent) 
    {
    if ((iSelectedDlgLine == EFeedsEditAutomaticUpdatingId) && (aPointerEvent.iType == TPointerEvent::EButton1Up))
        HandleFreqCmdL();
    else 
        CAknForm::HandlePointerEventL(aPointerEvent);
    }

// ---------------------------------------------------------
// CFeedsEditFeedDialog::OkToExitL
// ---------------------------------------------------------
//

TBool CFeedsEditFeedDialog::OkToExitL( TInt aButtonId  )
    {
    if (aButtonId == EAknSoftkeyOptions)
        {
        DisplayMenuL();
        return EFalse;
        }
    else
        if ((aButtonId == EAknSoftkeyChange) && (iSelectedDlgLine == EFeedsEditAutomaticUpdatingId))
        {
        HandleFreqCmdL();
        return EFalse;
        }
    else
        {
        return CAknForm::OkToExitL(aButtonId);    	
        }    
    }

// ---------------------------------------------------------
// CFeedsEditFeedDialog::HandleFreqCmdL
// Handles the auto update freuency modification commands
// ---------------------------------------------------------
//
void CFeedsEditFeedDialog::HandleFreqCmdL()
    {
    TBuf<32> title(_L("Auto update"));
    TInt currentlySelected = KAutoUpdatingOff;

    // options array
    CDesCArrayFlat* values = new( ELeave )CDesCArrayFlat(1);

    CleanupStack::PushL( values );
    HBufC* text = iCoeEnv->AllocReadResourceLC(R_FEEDS_AUTOUPDATING_FEEDS_OFF);
    values->AppendL(text->Des());
    CleanupStack::Pop();

    text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_15MIN);
    values->AppendL(text->Des());
    CleanupStack::Pop();

    text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_HOURLY);
    values->AppendL(text->Des());
    CleanupStack::Pop();

    text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_4HOURS);
    values->AppendL(text->Des());
    CleanupStack::Pop();

    text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_DAILY);
    values->AppendL(text->Des());
    CleanupStack::Pop();

    text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_WEEKLY);
    values->AppendL(text->Des());
    CleanupStack::Pop();

    switch(iFreq)
        {
        case KAutoUpdatingOff:
            currentlySelected = EFeedsEditAutomaticUpdatingOff;
            break;
        case KFifteen:
            currentlySelected = EFeedsEditAutomaticUpdating15min;
            break;
        case KOneHour:
            currentlySelected = EFeedsEditAutomaticUpdatingHourly;
            break;
        case KFourHour:
            currentlySelected = EFeedsEditAutomaticUpdating4hours;
            break;
        case KDay:
            currentlySelected = EFeedsEditAutomaticUpdatingDaily;
            break;
        case KWeek:
            currentlySelected = EFeedsEditAutomaticUpdatingWeekly;
            break;
        }

    currentlySelected = ShowRadioButtonSettingPageL(title,values,currentlySelected);

    if(!iActionCancelled)
        {
        switch(currentlySelected)
            {
            case EFeedsEditAutomaticUpdatingOff:
                iFreq = KAutoUpdatingOff;
                break;
            case EFeedsEditAutomaticUpdating15min:
                iFreq = KFifteen;
                break;
            case EFeedsEditAutomaticUpdatingHourly:
                iFreq = KOneHour;
                break;
            case EFeedsEditAutomaticUpdating4hours:
                iFreq = KFourHour;
                break;
            case EFeedsEditAutomaticUpdatingDaily:
                iFreq = KDay;
                break;
            case EFeedsEditAutomaticUpdatingWeekly:
                iFreq = KWeek;
                break;
            }
        CAknPopupField *freqControl =
        static_cast< CAknPopupField* >( Control( EFeedsEditAutomaticUpdatingId ) );	
        switch(iFreq)
            {
            case KAutoUpdatingOff:
                text = iCoeEnv->AllocReadResourceLC(R_FEEDS_AUTOUPDATING_FEEDS_OFF);
                break;
            case KFifteen:
                text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_15MIN);
                break;
            case KOneHour:
                text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_HOURLY);
                break;
            case KFourHour:
                text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_4HOURS);
                break;
            case KDay:
                text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_DAILY);
                break;
            case KWeek:
                text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_WEEKLY);
                break;
            }
        freqControl->SetEmptyTextL(text->Des());
        CleanupStack::Pop();
        SetChangesPending(ETrue);
        }
    CleanupStack::PopAndDestroy(values);
    }

// ---------------------------------------------------------
// CFeedsEditFeedDialog::PostLayoutDynInitL
// ---------------------------------------------------------
//

void CFeedsEditFeedDialog::PostLayoutDynInitL()
    {
    //Call the Base class LineChangedL() before the form is actually drawn
    // Call the LineChangedL to avoid the line clicking error for the first line
    LineChangedL(EFeedsEditDialogUrlId);
    // Call the base class PostLayoutDynInitL()
    CAknForm::PostLayoutDynInitL();
    }

// ---------------------------------------------------------
// CFeedsEditFeedDialog::LineChangedL
// Takes any action required when the current line is changed
// to aControlId.
// ---------------------------------------------------------
//
void CFeedsEditFeedDialog::LineChangedL( TInt aControlId )
    {	
    iSelectedDlgLine = aControlId;		
    //get the reference to the buttomgroup container
    CEikButtonGroupContainer& cba = ButtonGroupContainer();
    //add the 'Change' command
    //cba.SetCommandSetL(R_EDIT_FEEDS_CBA_OPTIONS_AUTO_UPDATE_CONTEXT_MENU);

    //now we check where is the control, as 'Change' command is not to be 
    //shown for the first two form lines
    if((aControlId == EFeedsEditDialogUrlId) ||
                    (aControlId == EFeedsEditDialogNameId))
        {
        // make the 'Change' button invisible
        cba.MakeCommandVisible(EAknSoftkeyChange, EFalse);
        }
    else if(aControlId == EFeedsEditAutomaticUpdatingId) 
        {
        // make the 'Change' Button visible
        cba.MakeCommandVisible(EAknSoftkeyChange, ETrue);
        }
    else
        {
        //blank implementation
        }
        // draw the CBA	
    cba.DrawNow();	
    }


// -----------------------------------------------------------------------------
// CFeedsEditFeedDialog::~CFeedsEditFeedDialog
//
// Deconstructor.
// -----------------------------------------------------------------------------
//
CFeedsEditFeedDialog::~CFeedsEditFeedDialog()
    {
    }


// -----------------------------------------------------------------------------
// CFeedsEditFeedDialog::PreLayoutDynInitL
//
// Called to setup the options menu with the dialog is active.
// -----------------------------------------------------------------------------
//
void CFeedsEditFeedDialog::DynInitMenuPaneL(TInt aResourceId, CEikMenuPane* aMenuPane)
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
// CFeedsEditFeedDialog::SaveFormDataL
//
// Called by the framework whenver the 'Save' menu item is selected.
// -----------------------------------------------------------------------------
//
TBool CFeedsEditFeedDialog::SaveFormDataL()
    {
    CEikEdwin*  textEditorName;
    CEikEdwin*  textEditorUrl;

    HBufC*      name = NULL;
    HBufC*      url = NULL;  
    TBool       valid = EFalse;

    // Extract the new name.
    textEditorName = static_cast<CEikEdwin*>(ControlOrNull(EFeedsEditDialogNameId));
    name = textEditorName->GetTextInHBufL();
    CleanupStack::PushL(name);

    // Extract the url name.
    textEditorUrl = static_cast<CEikEdwin*>(ControlOrNull(EFeedsEditDialogUrlId));
    url = textEditorUrl->GetTextInHBufL();
    CleanupStack::PushL(url);

    // Notify the observer.
    valid = iObserver.IsFeedNameValidL(name, !iIsNewFeed);

    // If invalid force the user to reentry it.
    if (!valid)
        {
        // Change Focus and select the name field.
        TryChangeFocusToL( EFeedsEditDialogNameId );
        textEditorName->SelectAllL();

        iExitDialog = EFalse;
        }

    // Otherwise, check the url too.
    else
        {
        valid = iObserver.IsFeedUrlValidL(url);

        // If invalid force the user to reentry it.
        if (!valid)
            {
	        // Set focus to url field 
            TryChangeFocusToL( EFeedsEditDialogUrlId );
            // Position focus to end of char, but not highlight the whole url field
            textEditorUrl->SetSelectionL( textEditorUrl->TextLength(), textEditorUrl->TextLength() );
            textEditorUrl->DrawDeferred();
			iExitDialog = EFalse;
			}

        // Otherwise pass the new values to the observer.
        else
            {
            if (iIsNewFeed)
                {
                iObserver.NewFeedL(*name, *url, iFreq);
                }
            else
                {
                // If the setting is changed from Off to other values, a confirmation
                // query with text: 'Enabling automatic updating may increase your monthly
                // phone bill' is shown.
                if ((iPreviousFreq == EFeedsEditAutomaticUpdatingOff) && 
                     (iFreq != EFeedsEditAutomaticUpdatingOff) && (iPreviousFreq != iFreq))
                    {
                    HBufC* note = StringLoader::LoadLC( R_FEEDS_DATAQUERY_AUTOUPDATEWARN );
                    if(iPreviousFreq == 0)
                        {
                      //  ShowInfoDialogwithOkSoftKeyL( note->Des() );    
                        ShowInfoDialogwithOkSoftKeyL( _L("Enabling automatic updating may increase your monthly phone bill") );    
                        }
                    CleanupStack::PopAndDestroy(); // note
                    iPreviousFreq = iFreq;
                    }
                iObserver.UpdateFeedL(*name, *url, iFreq);
                }

            iExitDialog = ETrue;
            }
        }

    // Clean up.
    CleanupStack::PopAndDestroy(url);   
    CleanupStack::PopAndDestroy(name);  

    return valid;
    }


// -----------------------------------------------------------------------------
// CFeedsEditFeedDialog::PreLayoutDynInitL
//
// Called by the framework before the form is initialised.
// -----------------------------------------------------------------------------
//
void CFeedsEditFeedDialog::PreLayoutDynInitL()
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
        text = CCoeEnv::Static()->AllocReadResourceAsDes16L(R_FEEDS_NEW_FEED_ITEM);
        CleanupStack::PushL(text);

        textEditor->SetTextL(text);
  		SetFormFlag(EUnsavedEdit, ETrue);

        CleanupStack::PopAndDestroy(text);
        }

    // Get the url field
    textEditor = static_cast<CEikEdwin*>(ControlOrNull(EFeedsEditDialogUrlId));

    textEditor->SetAknEditorAllowedInputModes( EAknEditorTextInputMode | EAknEditorNumericInputMode );

	textEditor->SetAknEditorFlags
        (
        EAknEditorFlagLatinInputModesOnly |
        EAknEditorFlagUseSCTNumericCharmap
        );
    textEditor->SetAknEditorPermittedCaseModes( EAknEditorUpperCase | EAknEditorLowerCase );

    // If the url was set then set it.
    if (iUrl.Length() > 0)
        {
        textEditor->SetTextL(&iUrl);
        }

    // Otherwise use the url.
    else
        {
    	HBufC*  text = NULL;
	
        // Load the label
        text = CCoeEnv::Static()->AllocReadResourceAsDes16L(R_FEEDS_NEW_FEED_URL_ITEM);
        CleanupStack::PushL(text);

        textEditor->SetTextL(text);
  		SetFormFlag(EUnsavedEdit, ETrue);

        CleanupStack::PopAndDestroy(text);
        }
    CAknPopupField *freqControl =
    static_cast< CAknPopupField* >( Control( EFeedsEditAutomaticUpdatingId ) );
    HBufC* text = NULL;
    switch(iFreq)
        {
        case KAutoUpdatingOff:
            text = iCoeEnv->AllocReadResourceLC(R_FEEDS_AUTOUPDATING_FEEDS_OFF);
            break;
        case KFifteen:
            text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_15MIN);
            break;
        case KOneHour:
            text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_HOURLY);
            break;
        case KFourHour:
            text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_4HOURS);
            break;
        case KDay:
            text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_DAILY);
            break;
        case KWeek:
            text = iCoeEnv->AllocReadResourceLC(R_FEEDS_UPDATING_INTERVAL_WEEKLY);
            break;
        }
    freqControl->SetEmptyTextL(text->Des());
    CleanupStack::Pop();
    }

// -----------------------------------------------------------------------------
// CFeedsEditFeedDialog::ProcessCommandL
//
// Called by the framework to process the options menu.
// -----------------------------------------------------------------------------
//
void CFeedsEditFeedDialog::ProcessCommandL(TInt aCommandId)
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
        case EAknFepCmdPredHelp:
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


// -----------------------------------------------------------------------------
// CFeedsEditFeedDialog::GetHelpContext
//
// Get help context for the control.
// -----------------------------------------------------------------------------
//
#ifdef __SERIES60_HELP
void CFeedsEditFeedDialog::GetHelpContext(TCoeHelpContext& aContext) const
    {
    // This must be the Browser's uid becasue the help texts are under Browser topics.
    aContext.iMajor = KUidBrowserApplication;
    
    if (iIsNewFeed)
        {        
        aContext.iContext = KOSS_HLP_RSS_ADD;
        }
    else
        {        
        aContext.iContext = KOSS_HLP_RSS_EDIT;
        }    
    }
#endif // __SERIES60_HELP

// -----------------------------------------------------------------------------
// CFeedsEditFeedDialog::ShowRadioButtonSettingPageL
// -----------------------------------------------------------------------------
//
TInt CFeedsEditFeedDialog::ShowRadioButtonSettingPageL(
                                                    TDesC& aTitle,
                                                    CDesCArrayFlat* aValues,
                                                    TInt aCurrentItem )
    {

    // index must be turned upside down, because options list is upside down
    //TInt newItem = aCurrentItem = aValues->Count() - 1 - aCurrentItem;
    TInt newItem = aCurrentItem;

    // We have everything to create dialog
    CAknRadioButtonSettingPage* dlg = new ( ELeave )CAknRadioButtonSettingPage(
        R_RADIO_BUTTON_SETTING_PAGE, newItem, aValues );

    CleanupStack::PushL( dlg );
    dlg->SetSettingTextL( aTitle );
    CleanupStack::Pop(); // dlg
    iActionCancelled = EFalse;
    if ( !dlg->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ) )
        {
        // Changes confirmed
        newItem = aCurrentItem;
        iActionCancelled = ETrue;
        }
    //iActionCancelled = EFalse;

    // index must be re-turned upside down, because options list is upside down
    return newItem;
    }

// ---------------------------------------------------------
// CFeedsEditFeedDialog::ShowInfoDialogwithOkSoftKeyL
// ---------------------------------------------------------
//
void CFeedsEditFeedDialog::ShowInfoDialogwithOkSoftKeyL( const TDesC& aNoteText )
    {
    CAknNoteDialog* dlg = new (ELeave) CAknNoteDialog(CAknNoteDialog::EConfirmationTone,CAknNoteDialog::ENoTimeout);
    dlg->SetTextL( aNoteText );
    dlg->PrepareLC( R_FEEDS_EDIT_AUTOUPDATE_INFORMATION_NOTE_DIALOG );
    dlg->RunLD();
    }
