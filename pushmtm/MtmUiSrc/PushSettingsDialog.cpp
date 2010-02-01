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
* Description:  Member definitions of CPushSettingsDialog.
*
*/



//  INCLUDES
#include "browser_platform_variant.hrh"
#include "PushSettingsDialog.h"
#include "PushMtmUiDef.h"
#include "PushMtmUiPanic.h"
#include <PushMtmUi.rsg>
#include "PushMtmUi.hrh"
#include "PushMtmDef.hrh"
#include <akntitle.h>
#include <akncontext.h>
#include <AknQueryDialog.h>
#include <barsread.h>
#include <AknRadioButtonSettingPage.h>
#include <bldvariant.hrh>
#ifdef __SERIES60_HELP
// Context-Sensitve Help File
#include <hlplch.h>
#include <csxhelp/wpush.hlp.hrh>
#endif // __SERIES60_HELP
#include <msvapi.h>
#include <data_caging_path_literals.hrh>
#include <f32file.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushSettingsDialog::CPushSettingsDialog
// ---------------------------------------------------------
//
CPushSettingsDialog::CPushSettingsDialog( CMsvSession& aMsvSession ) 
:   CAknDialog(), 
    iMsvSession( aMsvSession ), 
    iResourceLoader( *iCoeEnv ), 
    iNewTitleTextUsed( EFalse ), 
    iRestoreSP( ETrue )
    {
    }

// ---------------------------------------------------------
// CPushSettingsDialog::ExecuteLD
// ---------------------------------------------------------
//
void CPushSettingsDialog::ExecuteLD()
    {
    CleanupStack::PushL( this );
    ConstructL();
    CAknDialog::ConstructL( R_PUSHSD_MENU );
    CleanupStack::Pop( this );
    CAknDialog::ExecuteLD( R_PUSHSD_DIALOG );
    }

// ---------------------------------------------------------
// CPushSettingsDialog::ConstructL
// ---------------------------------------------------------
//
void CPushSettingsDialog::ConstructL()
    {
    // Add resource file.
    TParse* fileParser = new (ELeave) TParse;
    // Unnecessary to call CleanupStack::PushL( fileParser );
    fileParser->Set( KPushMtmUiResourceFileAndDrive, &KDC_MTM_RESOURCE_DIR, NULL ); 
    iResourceFile = fileParser->FullName();
    delete fileParser;
    fileParser = NULL;
    iResourceLoader.OpenL( iResourceFile );

    // Construct model.
    ConstructModelL();

    // Now model is ready for view.
    const TInt KGranularity( 8 );
    iSettingType = new (ELeave) CArrayFixFlat<TSettingType>( KGranularity );

    // Set up Title Pane and Context Pane.
    SetUpStatusPaneL();
    }

// ---------------------------------------------------------
// CPushSettingsDialog::~CPushSettingsDialog
// ---------------------------------------------------------
//
CPushSettingsDialog::~CPushSettingsDialog()
    {
    if ( iRestoreSP )
        {
        TRAP_IGNORE( RestoreStatusPaneL() );
        }
    iResourceLoader.Close();
    delete iPreviousTitleText;
    delete iModel;
    delete iSettingType;
    iSettingListBox = NULL;
    iSettingListBoxItemTextArray = NULL;
    }

// ---------------------------------------------------------
// CPushSettingsDialog::SetUpStatusPaneL
// ---------------------------------------------------------
//
void CPushSettingsDialog::SetUpStatusPaneL()
    {
    if ( !iNewTitleTextUsed )
        {
        __ASSERT_DEBUG( !iPreviousTitleText, 
            UiPanic( EPushMtmUiPanAlreadyExistingTitle ) );
        // Set up Title Pane.
        CEikStatusPane* statusPane = iEikonEnv->AppUiFactory()->StatusPane();
        CAknTitlePane* titlePane = 
            STATIC_CAST( CAknTitlePane*, 
            statusPane->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
        // Save content.
        iPreviousTitleText = titlePane->Text()->AllocL();
        // Set new content.
        TResourceReader reader;
        iCoeEnv->CreateResourceReaderLC( reader, R_PUSHSD_TITLE_PANE );
        titlePane->SetFromResourceL( reader );
        CleanupStack::PopAndDestroy(); // reader
        iNewTitleTextUsed = ETrue;
        }
    }

// ---------------------------------------------------------
// CPushSettingsDialog::RestoreStatusPaneL
// ---------------------------------------------------------
//
void CPushSettingsDialog::RestoreStatusPaneL()
    {
    if ( iNewTitleTextUsed )
        {
        __ASSERT_DEBUG( iPreviousTitleText, 
            UiPanic( EPushMtmUiPanMissingTitle ) );
        // Restore Title Pane.
        CEikStatusPane* statusPane = iEikonEnv->AppUiFactory()->StatusPane();
        CAknTitlePane* titlePane = 
            STATIC_CAST( CAknTitlePane*, 
            statusPane->ControlL( TUid::Uid( EEikStatusPaneUidTitle ) ) );
        titlePane->SetTextL( *iPreviousTitleText );
        delete iPreviousTitleText;
        iPreviousTitleText = NULL;
        iNewTitleTextUsed = EFalse;
        }
    }

// ---------------------------------------------------------
// CPushSettingsDialog::ConstructModelL
// ---------------------------------------------------------
//
void CPushSettingsDialog::ConstructModelL()
    {
    __ASSERT_DEBUG( !iModel, UiPanic( EPushMtmUiPanAlreadyExistingModel ) );

    // This will be the model.
    iModel = CPushMtmSettings::NewL();
    }

// ---------------------------------------------------------
// CPushSettingsDialog::UpdateSettingListBoxModelL
// ---------------------------------------------------------
//
void CPushSettingsDialog::UpdateSettingListBoxModelL()
    {
    __ASSERT_DEBUG( iModel && 
                    iSettingListBox && 
                    iSettingListBoxItemTextArray && 
                    iSettingType, UiPanic( EPushMtmUiPanNotInitialized ) );

    // Reset the list box' item text array and the setting type array.
    iSettingListBoxItemTextArray->Reset();
    iSettingType->Reset();

    // Service reception.
    ConstructAndAppendItemTextL
        ( EServiceReception, R_PUSHSD_RECEPT, 
        iModel->ServiceReception() ? R_PUSHSD_RECEP_ON : R_PUSHSD_RECEP_OFF );

#ifdef __SERIES60_PUSH_SL

    // Service loading (SL specific).
    ConstructAndAppendItemTextL
        ( EServiceLoadingType, R_PUSHSD_LOADING, 
        ( iModel->ServiceLoadingType() == CPushMtmSettings::EAutomatic ) ? 
        R_PUSHSD_LOAD_AUTOM : R_PUSHSD_LOAD_MAN );

#endif //__SERIES60_PUSH_SL
    }

// ---------------------------------------------------------
// CPushSettingsDialog::UpdateSettingListBoxViewL
// ---------------------------------------------------------
//
void CPushSettingsDialog::UpdateSettingListBoxViewL() const
    {
    __ASSERT_DEBUG( iSettingListBox, UiPanic( EPushMtmUiPanMissingListBox ) );
    //iSettingListBox->DrawItem( iSettingListBox->CurrentItemIndex() );
    TInt currentItemIndex( iSettingListBox->CurrentItemIndex() );
    iSettingListBox->DrawNow();
    iSettingListBox->SetCurrentItemIndex( currentItemIndex );
    }

// ---------------------------------------------------------
// CPushSettingsDialog::ApplyModelChangesL
// ---------------------------------------------------------
//
void CPushSettingsDialog::ApplyModelChangesL()
    {
    UpdateSettingListBoxModelL();
    UpdateSettingListBoxViewL();
    }

// ---------------------------------------------------------
// CPushSettingsDialog::ShowRadioButtonSettingPageL
// ---------------------------------------------------------
//
void CPushSettingsDialog::ShowRadioButtonSettingPageL
    ( const TSettingType aSettingType )
	{
    __ASSERT_DEBUG( iModel, UiPanic( EPushMtmUiPanMissingModel ) );

    TInt index;
    HBufC* settingTitle = ConstructSettingTitleL( aSettingType );
    CleanupStack::PushL( settingTitle );
    CDesCArrayFlat* itemArray = 
        ConstructSettingValueArrayL( aSettingType, index );
    CleanupStack::PushL( itemArray );
    CAknRadioButtonSettingPage* settingPage = 
        new (ELeave) CAknRadioButtonSettingPage
            ( R_PUSHSD_SP_RADIOBUTTON, index, itemArray );
    CleanupStack::PushL( settingPage );
    settingPage->SetSettingTextL( *settingTitle );

    const TInt previousIndex = index;
    CleanupStack::Pop(); // settingPage
    if ( settingPage->ExecuteLD( CAknSettingPage::EUpdateWhenChanged ) )
        {
        if ( previousIndex != index )
            {
            // Set the changes in the model.
            UpdateSettingL( aSettingType, index );
            }
        }

    CleanupStack::PopAndDestroy( 2 ); // itemArray, settingTitle
	}

// ---------------------------------------------------------
// CPushSettingsDialog::ShowServiceLoadingTypeConfirmationL
// ---------------------------------------------------------
//
TBool CPushSettingsDialog::ShowServiceLoadingTypeConfirmationL() const
    {
    CAknQueryDialog* dlg = CAknQueryDialog::NewL();
    return dlg->ExecuteLD( R_PUSHSD_AUTOLOAD_SETT_CONFIRM ) == EAknSoftkeyYes;
    }

// ---------------------------------------------------------
// CPushSettingsDialog::ChangeCurrentSettingWithoutSettingPageL
// ---------------------------------------------------------
//
void CPushSettingsDialog::ChangeCurrentSettingWithoutSettingPageL()
    {
    __ASSERT_DEBUG( iModel && iSettingListBox, 
        UiPanic( EPushMtmUiPanNotInitialized ) );

    switch ( iSettingType->At( iSettingListBox->CurrentItemIndex() ) )
        {
        case EServiceReception:
            {
            iModel->SetServiceReception( !iModel->ServiceReception() );
            break;
            }

#ifdef __SERIES60_PUSH_SL

        case EServiceLoadingType:
            {
            if ( iModel->ServiceLoadingType() == CPushMtmSettings::EManual )
                {
                // Display a confirmation dialog first.
                if ( ShowServiceLoadingTypeConfirmationL() )
                    {
                    iModel->SetServiceLoadingType
                        ( CPushMtmSettings::EAutomatic );
                    }
                }
            else
                {
                // Do not show confirmation dialog.
                iModel->SetServiceLoadingType( CPushMtmSettings::EManual );
                }
            break;
            }

#endif //__SERIES60_PUSH_SL

        default:
            {
            __ASSERT_DEBUG( EFalse, 
                UiPanic( EPushMtmUiPanCommandNotSupported ) );
            break;
            }
        }

    // Apply changes.
    iModel->SaveL();
    ApplyModelChangesL();
    }

// ---------------------------------------------------------
// CPushSettingsDialog::ChangeCurrentSettingL
// ---------------------------------------------------------
//
void CPushSettingsDialog::ChangeCurrentSettingL()
    {
    __ASSERT_DEBUG( iModel && iSettingListBox, 
        UiPanic( EPushMtmUiPanNotInitialized ) );
    ShowRadioButtonSettingPageL
        ( iSettingType->At( iSettingListBox->CurrentItemIndex() ) );
    // Apply changes.
    iModel->SaveL();
    ApplyModelChangesL();
    }

// ---------------------------------------------------------
// CPushSettingsDialog::ConstructAndAppendItemTextL
// ---------------------------------------------------------
//
void CPushSettingsDialog::ConstructAndAppendItemTextL
( const TSettingType aSettingType, 
  const TInt aTitleResId, const TInt aValueResId, 
  const TDesC& aTitleText, const TDesC& aValueText )
    {
    __ASSERT_DEBUG( iSettingListBoxItemTextArray && iSettingType, 
        UiPanic( EPushMtmUiPanNotInitialized ) );

    // Define separator text.
    _LIT( KHeadOfItemText, " \t" );
    // Define separator text.
    _LIT( KSeparatorOfItemText, "\t\t" );

    HBufC* titleText = ( aTitleResId != KErrNotFound ) ? 
        iEikonEnv->AllocReadResourceLC( aTitleResId ) : aTitleText.AllocLC();
    HBufC* valueText = ( aValueResId != KErrNotFound ) ? 
        iEikonEnv->AllocReadResourceLC( aValueResId ) : aValueText.AllocLC();

    // Define a heap descriptor to hold all the item text.
    HBufC* itemText = HBufC::NewMaxLC( 
        KHeadOfItemText().Length() + titleText->Length() + 
        KSeparatorOfItemText().Length() + valueText->Length() );
    TPtr itemTextPtr = itemText->Des();
    itemTextPtr =  KHeadOfItemText;
    itemTextPtr += *titleText;
    itemTextPtr += KSeparatorOfItemText;
    itemTextPtr += *valueText;
    iSettingListBoxItemTextArray->AppendL( itemTextPtr );
    iSettingType->AppendL( aSettingType );

    CleanupStack::PopAndDestroy( 3 ); // itemText, valueText, titleText
    }

// ---------------------------------------------------------
// CPushSettingsDialog::ConstructSettingTitleL
// ---------------------------------------------------------
//
HBufC* CPushSettingsDialog::ConstructSettingTitleL
( const TSettingType aSettingType ) const
    {
    const TInt titleResId = 
        aSettingType == EServiceReception           ? R_PUSHSD_RECEPT : 
#ifdef __SERIES60_PUSH_SL
        aSettingType == EServiceLoadingType         ? R_PUSHSD_LOADING : 
#endif //__SERIES60_PUSH_SL
        KErrNotFound;

    __ASSERT_DEBUG( titleResId != KErrNotFound, 
        UiPanic( EPushMtmUiPanNotInitialized ) );

    return iEikonEnv->AllocReadResourceL( titleResId );
    }

// ---------------------------------------------------------
// CPushSettingsDialog::ConstructSettingValueArray
// ---------------------------------------------------------
//
void CPushSettingsDialog::ConstructSettingValueArray
    ( 
        const TSettingType aSettingType, 
        TInt& aVal1ResId, 
        TInt& aVal2ResId 
    ) const
    {
    aVal1ResId = 
        aSettingType == EServiceReception           ? R_PUSHSD_RECEP_ON : 
#ifdef __SERIES60_PUSH_SL
        aSettingType == EServiceLoadingType         ? R_PUSHSD_LOAD_AUTOM : 
#endif //__SERIES60_PUSH_SL
        KErrNotFound;

    aVal2ResId = 
        aSettingType == EServiceReception           ? R_PUSHSD_RECEP_OFF : 
#ifdef __SERIES60_PUSH_SL
        aSettingType == EServiceLoadingType         ? R_PUSHSD_LOAD_MAN : 
#endif //__SERIES60_PUSH_SL
        KErrNotFound;

    __ASSERT_DEBUG( aVal1ResId != KErrNotFound && 
                    aVal2ResId != KErrNotFound, 
        UiPanic( EPushMtmUiPanNotInitialized ) );
    }

// ---------------------------------------------------------
// CPushSettingsDialog::CurrentlySelectedSettingValueResId
// ---------------------------------------------------------
//
TInt CPushSettingsDialog::CurrentlySelectedSettingValueResId
( const TSettingType aSettingType, TInt& aIndex ) const
    {
    TInt val1ResId( KErrNotFound );
    TInt val2ResId( KErrNotFound );
    ConstructSettingValueArray( aSettingType, val1ResId, val2ResId );

    const TBool firstSelected = 
        aSettingType == EServiceReception ? iModel->ServiceReception() : 
#ifdef __SERIES60_PUSH_SL
        aSettingType == EServiceLoadingType ? iModel->ServiceLoadingType() == 
                                              CPushMtmSettings::EAutomatic : 
#endif //__SERIES60_PUSH_SL
        EFalse;

    aIndex = firstSelected ? 0 : 1;
    return firstSelected ? val1ResId : val2ResId;
    }

// ---------------------------------------------------------
// CPushSettingsDialog::ConstructSettingValueArrayL
// ---------------------------------------------------------
//
CDesCArrayFlat* CPushSettingsDialog::ConstructSettingValueArrayL
( const TSettingType aSettingType, TInt& aIndex ) const
    {
    CDesCArrayFlat* valueArray = new (ELeave) CDesCArrayFlat( 2 );
    CleanupStack::PushL( valueArray );

    TInt val1ResId( KErrNotFound );
    TInt val2ResId( KErrNotFound );
    ConstructSettingValueArray( aSettingType, val1ResId, val2ResId );
    HBufC* value1Text = iEikonEnv->AllocReadResourceLC( val1ResId );
    HBufC* value2Text = iEikonEnv->AllocReadResourceLC( val2ResId );
    valueArray->AppendL( *value1Text );
    valueArray->AppendL( *value2Text );
    CleanupStack::PopAndDestroy( 2 ); // value2Text, value1Text

    // Returned val. is ignored.
    CurrentlySelectedSettingValueResId( aSettingType, aIndex );

    CleanupStack::Pop(); // valueArray
    return valueArray;
    }

// ---------------------------------------------------------
// CPushSettingsDialog::UpdateSettingL
// ---------------------------------------------------------
//
void CPushSettingsDialog::UpdateSettingL
( const TSettingType aSettingType, const TInt aIndex ) const
    {
    switch ( aSettingType )
        {
        case EServiceReception:
            {
            iModel->SetServiceReception( aIndex == 0 ? ETrue : EFalse );
            break;
            }

#ifdef __SERIES60_PUSH_SL

        case EServiceLoadingType:
            {
            if ( aIndex == 0 )
            // iModel->ServiceLoadingType() == CPushMtmSettings::EManual
                {
                // Display a confirmation dialog first.
                if ( ShowServiceLoadingTypeConfirmationL() )
                    {
                    iModel->SetServiceLoadingType
                        ( CPushMtmSettings::EAutomatic );
                    }
                }
            else
                {
                // Do not show confirmation dialog.
                iModel->SetServiceLoadingType( CPushMtmSettings::EManual );
                }
            break;
            }

#endif //__SERIES60_PUSH_SL

        default:
            {
            __ASSERT_DEBUG( EFalse, 
                UiPanic( EPushMtmUiPanCommandNotSupported ) );
            break;
            }
        }
    }

// ---------------------------------------------------------
// CPushSettingsDialog::OfferKeyEventL
// ---------------------------------------------------------
//
TKeyResponse CPushSettingsDialog::OfferKeyEventL
( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    if ( aType == EEventKey && aKeyEvent.iCode == EKeyEscape )
		{
        // Shutting down. Do not restore SP.
		iRestoreSP = EFalse;
		}
    return CAknDialog::OfferKeyEventL( aKeyEvent, aType );
    }

// ---------------------------------------------------------
// CPushSettingsDialog::DynInitMenuPaneL
// ---------------------------------------------------------
//
void CPushSettingsDialog::DynInitMenuPaneL
    ( TInt /*aResourceId*/, CEikMenuPane* /*aMenuPane*/ )
    {
    }

// ---------------------------------------------------------
// CPushSettingsDialog::PreLayoutDynInitL
// ---------------------------------------------------------
//
void CPushSettingsDialog::PreLayoutDynInitL()
    {
    __ASSERT_DEBUG( !iSettingListBox && 
                    !iSettingListBoxItemTextArray, 
        UiPanic( EPushMtmUiPanAlreadyInitialized ) );

    iSettingListBox = STATIC_CAST( CAknSettingStyleListBox*, 
        Control( EPushSettingsDialogCntrlListBox ) );
	iSettingListBoxItemTextArray = STATIC_CAST( CDesCArrayFlat*, 
        iSettingListBox->Model()->ItemTextArray() );
    iSettingListBox->SetListBoxObserver( this );
    TRect rect = Rect();
    iSettingListBox->SetRect( rect );
    // Set up scroll bar.
	iSettingListBox->CreateScrollBarFrameL( ETrue );
	iSettingListBox->ScrollBarFrame()->SetScrollBarVisibilityL
        ( CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );

    // Update setting list box model.
    UpdateSettingListBoxModelL();
    }

// ---------------------------------------------------------
// CPushSettingsDialog::ProcessCommandL
// ---------------------------------------------------------
//
void CPushSettingsDialog::ProcessCommandL( TInt aCommand )
    {
    HideMenu();

    switch ( aCommand )
        {
	    case EPushSettingsDialogCmdChangeWithoutSettingPage:
            {
            ChangeCurrentSettingWithoutSettingPageL();
		    break;
            }

        case EPushSettingsDialogCmdChange:
            {
            ChangeCurrentSettingL();
		    break;
            }

#ifdef __SERIES60_HELP

        case EPushSettingsDialogCmdHelp:
            {
            HlpLauncher::LaunchHelpApplicationL
                ( iEikonEnv->WsSession(), 
                iEikonEnv->EikAppUi()->AppHelpContextL() );
		    break;
            }

#endif //__SERIES60_HELP

        case EPushSettingsDialogCmdBack:
        case EPushSettingsDialogCmdExit:
            {
            TryExitL( EPushSettingsDialogCmdAknExit );
            break;
            }

        case EPushSettingsDialogCmdAknExit:
            {
            STATIC_CAST( MEikCommandObserver*, iEikonEnv->EikAppUi() )->
                ProcessCommandL( EPushSettingsDialogCmdAknExit );
            break;
            }

        default:
            {
            break;
            }
        }
    }

// ---------------------------------------------------------
// CPushSettingsDialog::OkToExitL
// ---------------------------------------------------------
//
TBool CPushSettingsDialog::OkToExitL( TInt aButtonId )
    {
    TBool ret( EFalse );

    switch ( aButtonId )
        {
        case EPushSettingsDialogCmdOptions:
            {
            if ( !MenuShowing() )
                {
                DisplayMenuL();
                }
            break;
            }
        case EPushSettingsDialogCmdChangeWithoutSettingPage:
            {
            ChangeCurrentSettingWithoutSettingPageL();
            break;
            }
        case EPushSettingsDialogCmdBack:
        case EPushSettingsDialogCmdExit:
        case EPushSettingsDialogCmdAknExit:
            {
            RestoreStatusPaneL();
            ret = ETrue;
            break;
            }
        default:
            {
            break;
            }
        }

    return ret;
    }

// ---------------------------------------------------------
// CPushSettingsDialog::HandleListBoxEventL
// ---------------------------------------------------------
//
void CPushSettingsDialog::HandleListBoxEventL
( CEikListBox* /*aListBox*/, TListBoxEvent aEventType )
	{
	// Generate change command only if double tapped/center key is pressed. 
	//Removed single tap check from here
	if ( aEventType == EEventEnterKeyPressed || 
         aEventType == EEventItemDoubleClicked 
#ifdef BRDO_SINGLE_CLICK_ENABLED_FF         
         || aEventType == EEventItemSingleClicked 
#endif         
         )
        {
        ProcessCommandL( EPushSettingsDialogCmdChangeWithoutSettingPage );
        }
	}

#ifdef __SERIES60_HELP
// ---------------------------------------------------------
// CPushSettingsDialog::GetHelpContext
// ---------------------------------------------------------
//
void CPushSettingsDialog::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = TUid::Uid( EUidPushViewerApp );
	aContext.iContext = KWPUSH_HLP_SETTINGS;
    }
#endif //__SERIES60_HELP

// End of file.
