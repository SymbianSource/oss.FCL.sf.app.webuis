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
*      Implementation of CBrowserBookmarkEditDialog.
*      
*
*/


// INCLUDE FILES

#include <BrowserNG.rsg>
#include <eikmenup.h>
#include <eikseced.h>
#include <eikenv.h>
#include <avkon.hrh>
#include <AknQueryDialog.h>
#include <eikcapc.h>
#include <AknPopupField.h>
#include <AknQueryValueText.h>
#include <AknKeys.h>
#include <FavouritesItem.h>
#include <aknnavi.h>
#include <aknnavide.h>

#include <aknconsts.h>
#include <akneditstateindicator.h>
#include <AknsListBoxBackgroundControlContext.h>
#include <Uri16.h> // TUriParser

#include "BrowserBookmarksEditFormApModel.h"
#include "BrowserBookmarksEditDialog.h"
#include "BrowserFavouritesModel.h"
#include "BrowserDialogs.h"
#include "BrowserAppUi.h"
#include "BrowserAppViewBase.h"
#include "commonconstants.h"
#include "Browser.hrh"
#include "BrowserUtil.h"

#ifdef __SERIES60_HELP
// Context-Sensitve Help File
#include "BrowserApplication.h"
#include <csxhelp/browser.hlp.hrh>
#endif // __SERIES60_HELP

// CONSTANTS

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::NewL
// ---------------------------------------------------------
//
CBrowserBookmarkEditDialog* CBrowserBookmarkEditDialog::NewL
        (
        CFavouritesItem& aItem,
        CBrowserFavouritesModel& aModel,
		MApiProvider& aApiProvider,
        //MCommsModel& aCommsModel,
        TInt aUid /* = KFavouritesNullUid */
        )
    {
    CBrowserBookmarkEditDialog* form =
        new (ELeave) CBrowserBookmarkEditDialog(	
												aItem, 
												aModel, 
												aApiProvider, 
												aUid 
												);
    CleanupStack::PushL( form );
    form->ConstructL( /* aApiProvider.CommsModel() */ );
    CleanupStack::Pop();    // form
    return form;
    }

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::~CBrowserBookmarkEditDialog
// ---------------------------------------------------------
//
CBrowserBookmarkEditDialog::~CBrowserBookmarkEditDialog()
    {
    delete iFormApModel;
    delete iNaviDecorator;
    }

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::ProcessCommandL
// ---------------------------------------------------------
//
void CBrowserBookmarkEditDialog::ProcessCommandL( TInt aCommandId )
    {
    switch ( aCommandId )
        {
        case EAknCmdExit:
            {
            // Custom added commands. Forward to view.
            // Note that we respond to EAknCmdExit, not CEikCmdExit; it's
            // because we don't get it through the AppUi framework (instead,
            // we forward it there now).
            CBrowserAppUi::Static()->HandleCommandL(EWmlCmdUserExit);
            break;
            }

#ifdef __SERIES60_HELP

        case EAknCmdHelp:
            {
            CBrowserAppUi::Static()->HandleCommandL(EAknCmdHelp);
	        break;
            }

#endif //__SERIES60_HELP

        default:
            {
            // Standard form commands.
            CAknForm::ProcessCommandL( aCommandId );
            if ( iExitDialog )
                {
                TryExitL( EAknSoftkeyBack );
                }
            break;
            }
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::CBrowserBookmarkEditDialog
// ---------------------------------------------------------
//
CBrowserBookmarkEditDialog::CBrowserBookmarkEditDialog
        (
        CFavouritesItem& aItem,
        CBrowserFavouritesModel& aModel,
        MApiProvider& aApiProvider,
		TInt aUid
        )
		: iItem( &aItem ),
		  iModel( &aModel ),
		  iUid( aUid ),		  
		  iExitDialog( EFalse ),
		  iExitNoteResourceId( 0 ),  // By default do not show any note.
		  iApiProvider( aApiProvider )
    {
    }

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::ConstructL
// ---------------------------------------------------------
//
void CBrowserBookmarkEditDialog::ConstructL( /*MCommsModel& aCommsModel*/ )
    {
    CAknForm::ConstructL();
    iFormApModel = CBrowserBookmarksEditFormApModel::NewL( ApiProvider().CommsModel()/*aCommsModel*/ );
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarkEditDialog::OkToExitL
// ----------------------------------------------------------------------------
//
TBool CBrowserBookmarkEditDialog::OkToExitL( TInt aButtonId )
    {
    TBool ret = CAknForm::OkToExitL( aButtonId );
    if ( ret && iExitNoteResourceId )
        {
        // Exiting; show appropriate note if applicable.
		TBrowserDialogs::InfoNoteL(
									R_BROWSER_INFO_NOTE, iExitNoteResourceId );
        }
    //pop empty label from navipane
    if ( ret )
        iNaviPane->Pop();
    return ret;
    }

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::SaveFormDataL
// ---------------------------------------------------------
//
TBool CBrowserBookmarkEditDialog::SaveFormDataL()
    {
    // Of KWmlFavouritesMaxUrlDefine, KWmlFavouritesMaxUsernameDefine, KWmlFavouritesMaxPasswordDefine
    // and KWmlFavouritesMaxBookmarkNameDefine the biggest is KWmlFavouritesMaxUrlDefine.
    TBuf<KFavouritesMaxUrlDefine> buf;

    // Get title.
    GetEdwinText( buf, EWmlControlEditBmEditTitle );
    iItem->SetNameL( buf.Left(KFavouritesMaxName) );

    // Get address.
    GetEdwinText( buf, EWmlControlEditBmEditAddress );
    HBufC* text = HBufC::NewL(KFavouritesMaxUrlDefine);

    // Do not check return value from Parse(). If the user wants to create a 
    // badly formed url then let them do so.
    TUriParser url;
    url.Parse( buf ); 

    // Add a default scheme if it is not already present.
    if( !url.IsPresent( EUriScheme ) )
        { 
        _LIT( KBookmarkDefaultScheme, "http://" );
    
        text->Des().Copy( KBookmarkDefaultScheme ); 
        }
    // Error prone code. If user enters the whole KFavouritesMaxUrlDefine(1024) amount of
    // characters with out the http:// infront, well have a USER:23 here.
    // Hack aplied for now and included in FIX: EKDN-7P64KZ
    // Needs a proper error raised and a fix.
    //
    // Check if long enough and append, if not we dont add http://
    // and alert user in SaveItemL() with popup that there must be an url
    // NOTICE: THIS NEEDS A PROPER FIX - THIS IS A TEMP WORKAROUND
    if( ( buf.Length() + text->Length() ) <= KFavouritesMaxUrlDefine )
        {
        text->Des().Append( buf );
        }
    else
        {
        text->Des().Copy( buf );
        }
        
    
    Util::EncodeSpaces(text);
    CleanupStack::PushL(text);// push after encode because the pointer may change
    iItem->SetUrlL( text->Des() );
    CleanupStack::PopAndDestroy();// text

    // Get username.
    GetEdwinText( buf, EWmlControlEditBmEditUsername );
    iItem->SetUserNameL( buf );

    // Get password.
    STATIC_CAST( CEikSecretEditor*, Control( EWmlControlEditBmEditPassword ) )
        ->GetText( buf );
    iItem->SetPasswordL( buf );

    // Get WAP Access point.
    TFavouritesWapAp ap = iFormApModel->CurrentWapAp();
    if ( !ap.IsNull() )
        {
        // We set it back only if not NULL.
        // AP is NULL if invalid (bookmark was edited, but the ap was not and
        // it is left "Invalid". In that case we leave ap as-is.
        iItem->SetWapAp( ap );
        }

    return SaveItemL();
    }

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::DynInitMenuPaneL
// ---------------------------------------------------------
//
void CBrowserBookmarkEditDialog::DynInitMenuPaneL
( TInt aResourceId, CEikMenuPane* aMenuPane )
    {
    CAknForm::DynInitMenuPaneL( aResourceId, aMenuPane );
    if ( aResourceId == R_AVKON_FORM_MENUPANE )
        {
        // Disallow user addition / deletion / labeling.
        aMenuPane->SetItemDimmed( EAknFormCmdAdd, ETrue );
        aMenuPane->SetItemDimmed( EAknFormCmdDelete, ETrue );
        aMenuPane->SetItemDimmed( EAknFormCmdLabel, ETrue );
        // Add "Exit".
        aMenuPane->AddMenuItemsL
            ( R_BROWSER_BOOKMARKS_MENU_PANE_EDIT_BOOKMARK );
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::PreLayoutDynInitL
// ---------------------------------------------------------
//
void CBrowserBookmarkEditDialog::PreLayoutDynInitL()
    {
    LoadFormDataL();
    CAknForm::PreLayoutDynInitL();

    CEikStatusPane *sp = ((CAknAppUi*)iEikonEnv->EikAppUi())->StatusPane();
    // Fetch pointer to the default navi pane control
    iNaviPane = 
		(CAknNavigationControlContainer*)sp->ControlL( 
		TUid::Uid( EEikStatusPaneUidNavi ) );
    iNaviDecorator = iNaviPane->CreateNavigationLabelL( _L("") );

    // push empty label, to avoid tab group visibility in BookmarkEditor. 
    iNaviPane->PushL( *iNaviDecorator );    
    }

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::LoadFormDataL
// ---------------------------------------------------------
//
void CBrowserBookmarkEditDialog::LoadFormDataL()
    {

    // Set title.
    const TPtrC tptrcName = iItem->Name();
    STATIC_CAST( CEikEdwin*, Control( EWmlControlEditBmEditTitle ) )->
        SetTextL( &(tptrcName) );

    // Set address.
    const TPtrC tptrcUrl = iItem->Url();
    STATIC_CAST( CEikEdwin*, Control( EWmlControlEditBmEditAddress ) )->
        SetTextL( &(tptrcUrl) );

    // Set username.
    const TPtrC tptrcUserName = iItem->UserName();
    STATIC_CAST( CEikEdwin*, Control( EWmlControlEditBmEditUsername ) )->
        SetTextL( &(tptrcUserName) );

    // Set password.
    STATIC_CAST( CEikSecretEditor*,Control( EWmlControlEditBmEditPassword ) )->
        SetText( iItem->Password() );

    // Set WAP Access Point popup field's query value.
    CAknPopupField* wapApSelector = STATIC_CAST
        ( CAknPopupField*, Control( EWmlControlEditBmWapApSelector ) );
    wapApSelector->SetQueryValueL( iFormApModel->QueryValue() );
    // Set current WAP Access Point.
    iFormApModel->SetCurrentWapApL( iItem->WapAp() );
    }

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::SaveItemL
// ---------------------------------------------------------
//
TBool CBrowserBookmarkEditDialog::SaveItemL()
    {
    TInt res;
    CBrowserFavouritesModel::TRenameMode renameMode =
        CBrowserFavouritesModel::EDontRename;

    iExitNoteResourceId = 0;    // By default, do not show any note.

    if ( !iItem->Name().Length() )
        {
        // No title for this item. Make a default name and ensure auto renaming
        // to non-conflicting name (without user interaction).
        iModel->SetNameToDefaultL( *iItem );
        renameMode = CBrowserFavouritesModel::EAutoRename;
        }

    if ( iItem->Url().Length() && iItem->Url() != KHttpString && IsValidUrlHost( iItem->Url() ) )
        {
        if ( iUid == KFavouritesNullUid )
            {
            // This is a new item. Add it.
            res = iModel->AddL( *iItem, /*aDbErrorNote=*/ETrue, renameMode );
            }
        else
            {
            // Editing an existing item. Update now.
            res = iModel->UpdateL
                ( *iItem, iUid, /*aDbErrorNote=*/ETrue, renameMode );
            }
        switch ( res )
            {
            case KErrNone:
                {
                iExitDialog = ETrue;
                break;
                }
            case KErrAlreadyExists:
                {
                // Name conflicts. Let the user know.
				TBrowserDialogs::InfoNoteL
                    (
                    R_BROWSER_INFO_NOTE,
                    R_BROWSER_BOOKMARKS_NAME_ALREADY_USED
                    );
                // Set focus to name field and keep editing.
                TryChangeFocusToL( EWmlControlEditBmEditTitle );
                // Select all, cursor is at end.
                CEikEdwin* nameEditor = STATIC_CAST
                    ( CEikEdwin*, Control( EWmlControlEditBmEditTitle ) );
                nameEditor->SetSelectionL( nameEditor->TextLength(), 0 );
                nameEditor->DrawDeferred();
                iExitDialog = EFalse;
                break;
                }
            default:
                {
                iExitDialog = EFalse;
                break;
                }
            }
        }
    else
        {
        // No Address. Cannot save this bookmark.
        res = TBrowserDialogs::ConfirmQueryYesNoL(
									R_BROWSER_BOOKMARKS_TEXT_QUERY_ADDRESS );
        if ( !res /*res == EAknSoftkeyNo*/ )
            {
            // User decided not to save the bookmark. Exit now.
            iExitNoteResourceId = iModel->StringResourceId
                ( *iItem, CBrowserFavouritesModel::ENotSaved );
            iExitDialog = ETrue;
            }
        else
            {
            // Set focus to address field and keep editing.
            TryChangeFocusToL( EWmlControlEditBmEditAddress );
            CEikEdwin* urlEditor = STATIC_CAST
                ( CEikEdwin*, Control( EWmlControlEditBmEditAddress ) );
            // Select none, cursor is at end.
            urlEditor->SetSelectionL
                ( urlEditor->TextLength(), urlEditor->TextLength() );
            urlEditor->DrawDeferred();
            iExitDialog = EFalse;
            }
        }
    return iExitDialog;
    }

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::IsValidUrlHost
// ---------------------------------------------------------
//
TBool CBrowserBookmarkEditDialog::IsValidUrlHost( const TDesC& aUrl )
    {
    TBool isValid = EFalse;
    _LIT(KHttpMark, "://");
    // Get the position of end of protocol definition
    TInt position = aUrl.Find( KHttpMark );
    if ( position != KErrNotFound  )
        {
        TPtrC16 url = aUrl.Right( aUrl.Length() - position - KHttpMark().Length() );
        // Locate the next slash which is the end of host part of url
        TInt slashLocation = url.Locate('/');
        // We check the total length of host part of url.
        // If over 255 we return EFalse
        if ( slashLocation == KErrNotFound )
            {
            if ( ( url.Length() + position + KHttpMark().Length() ) <= 255 )
                {
                isValid = ETrue;
                }
            }
        else
            {
            if ( ( position + slashLocation + KHttpMark().Length() ) <= 255 )
                {
                isValid = ETrue;
                }
            }
        }
    return isValid;
    }

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::GetEdwinText
// ---------------------------------------------------------
//
void CBrowserBookmarkEditDialog::GetEdwinText
( TDes& aText, TInt aControlId )
    {
    STATIC_CAST( CEikEdwin*, Line( aControlId )->iControl )->GetText( aText );
    }
    
    
// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::HandleResourceChange
// ---------------------------------------------------------
//
void CBrowserBookmarkEditDialog::HandleResourceChange( TInt aType )
    {
    TRAP_IGNORE( HandleResourceChangeL( aType ) );
    }

// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::HandleResourceChangeL
// ---------------------------------------------------------
//
void CBrowserBookmarkEditDialog::HandleResourceChangeL( TInt aType )
    {
    CAknForm::HandleResourceChange(aType);
    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        TRect  rect;
        if (AknLayoutUtils::LayoutMetricsRect(AknLayoutUtils::EMainPane, rect))
            {
            SetRect(rect);
            }
        }
    }    

#ifdef __SERIES60_HELP
// ---------------------------------------------------------
// CBrowserBookmarkEditDialog::GetHelpContext()
// ---------------------------------------------------------
//
void CBrowserBookmarkEditDialog::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidBrowserApplication;
    if ( iUid == KFavouritesNullUid )
        {
        // Add bookmark help
		aContext.iContext = KOSS_HLP_BM_ADDBM;
        }
    else
        {
        // Edit bookmark help
	    aContext.iContext = KOSS_HLP_BM_EDIT;
		}
    }
#endif // __SERIES60_HELP

// End of File
