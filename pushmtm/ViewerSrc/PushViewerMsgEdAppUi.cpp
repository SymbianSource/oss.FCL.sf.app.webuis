/*
* Copyright (c) 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Application Ui member definitions.
*
*/



// INCLUDE FILES

#include "PushViewerMsgEdAppUi.h"
#include "PushViewerDoc.h"
#include "PushViewerDef.h"
#include "PushViewerPanic.h"
#include "PushMtmUtil.h"
#include "PushViewer.hrh"
#include "PushMtmLog.h"
#include <CSIPushMsgEntry.h>
#include <CSLPushMsgEntry.h>
#include <PushViewer.rsg>
#include <aknmessagequerydialog.h>
#include <AknQueryDialog.h>
#include <MsgEditorView.h>
#include <MsgBodyControl.h>
#include <txtrich.h>
#include <txtfrmat.h>
#include <txtfmlyr.h>
#include <gdi.h>
#include <bldvariant.hrh>
#include <eikrted.h>
#include <barsread.h>
#include <AknNoteWrappers.h>
#ifdef __SERIES60_HELP
// Context-Sensitve Help File
#include <hlplch.h>
#include <csxhelp/wpush.hlp.hrh>
#endif // __SERIES60_HELP

#include "eikon.hrh"


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::CPushViewerMsgEdAppUi
// ---------------------------------------------------------
//
CPushViewerMsgEdAppUi::CPushViewerMsgEdAppUi()
    {
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::~CPushViewerMsgEdAppUi
// ---------------------------------------------------------
//
CPushViewerMsgEdAppUi::~CPushViewerMsgEdAppUi()
    {
    PUSHLOG_ENTERFN("CPushViewerMsgEdAppUi::~CPushViewerMsgEdAppUi")
    delete iView;
    iView = NULL;
    delete iContext;
    iContext = NULL;
    PUSHLOG_LEAVEFN("CPushViewerMsgEdAppUi::~CPushViewerMsgEdAppUi")
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::ConstructL
// ---------------------------------------------------------
//
void CPushViewerMsgEdAppUi::ConstructL()
    {
    PUSHLOG_ENTERFN("CPushViewerMsgEdAppUi::ConstructL")

    CMsgEditorAppUi::ConstructL();

    if ( !iEikonEnv->StartedAsServerApp( ) )
        {
        // If the app was not started as server app,
        // we can call PrepareLaunchL
        Document()->PrepareToLaunchL( this );
        }

    PUSHLOG_LEAVEFN("CPushViewerMsgEdAppUi::ConstructL")
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::ConstructBodyTextL
// ---------------------------------------------------------
//
TInt CPushViewerMsgEdAppUi::ConstructBodyTextL
    ( CMsgBodyControl& aBodyControl1, CMsgBodyControl& aBodyControl2 ) const
    {
    PUSHLOG_ENTERFN("CPushViewerMsgEdAppUi::ConstructBodyTextL");

    HBufC* staticText = iEikonEnv->AllocReadResourceLC
                        ( R_PUSHVIEWER_MAINPANE_TEXT );

    TPtrC renderedTextPtr( *staticText ); // Static text by default.
    TPtrC renderedHrefPtr( KNullDesC );

    if ( Model().PushMsgType() == KUidWapPushMsgSI.iUid )
        {
        CSIPushMsgEntry* si = STATIC_CAST( CSIPushMsgEntry*, &Model() );

        const TPtrC text = si->Text();
        if ( text.Length() )
            {
            renderedTextPtr.Set( text );
            }
        else
            {
            renderedTextPtr.Set( *staticText );
            }

        const TPtrC url = si->Url();
        if ( url.Length() )
            {
            renderedHrefPtr.Set( url );
            }
        }
    else if ( Model().PushMsgType() == KUidWapPushMsgSL.iUid )
        {
        CSLPushMsgEntry* sl = STATIC_CAST( CSLPushMsgEntry*, &Model() );

        const TPtrC url = sl->Url();
        if ( url.Length() )
            {
            renderedHrefPtr.Set( url );
            }
        }
    else
        {
        User::Leave( KErrNotSupported );
        }

    // The message goes to the first body control by default, but
    // if it does not fit into one screen, then first comes the href.

    aBodyControl1.InsertTextL( renderedTextPtr );

    CMsgBodyControl* controlContainingTheUrl = NULL;
    TInt indexOfHrefControl = KErrNotFound;

    // Add the href
    if ( renderedHrefPtr.Length() == 0 )
        {
        // No href.
        controlContainingTheUrl = NULL;
        iAvkonAppUi->Cba()->SetCommandSetL( R_PUSHVIEWER_SOFTKEYS );
        iAvkonAppUi->Cba()->DrawNow();
        }
    else
        {
        aBodyControl2.InsertTextL( renderedHrefPtr );
        controlContainingTheUrl = &aBodyControl2;
        indexOfHrefControl = 1; // Zero based.
        }

    aBodyControl1.SetPlainTextMode( ETrue );
    aBodyControl2.SetPlainTextMode( ETrue );

    // Switch on URL highlighting. We use FindItem for it -
    // otherwise Find Item has no other role!
    if ( controlContainingTheUrl != NULL )
        {
        if ( controlContainingTheUrl->ItemFinder() )
            {
            controlContainingTheUrl->
                ItemFinder()->SetFindModeL( CItemFinder::EUrlAddress );
            controlContainingTheUrl->
                SetupAutomaticFindAfterFocusChangeL( ETrue );
            }
        controlContainingTheUrl->Editor().SetAlignment( EAknEditorAlignCenter );
        }

    CleanupStack::PopAndDestroy( staticText ); // staticText

    PUSHLOG_LEAVEFN("CPushViewerMsgEdAppUi::ConstructBodyTextL");
    return indexOfHrefControl;
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::ConstructViewL
// ---------------------------------------------------------
//
void CPushViewerMsgEdAppUi::ConstructViewL()
    {
    CMsgEditorView* newView = CMsgEditorView::NewL
                              ( *this, CMsgEditorView::EMsgReadOnly );
    delete iView;
    iView = newView;

    // Let two body controls: one for the message text and one for the Href

    // There is already one by default - get a pointer to it.
    CMsgBodyControl* bodyControl1 = REINTERPRET_CAST( CMsgBodyControl*,
                                    iView->ControlById( EMsgComponentIdBody ) );

    // Create the second:
    CMsgBodyControl* bodyControl2 = CMsgBodyControl::NewL( iView );
    CleanupStack::PushL( bodyControl2 );
    iView->AddControlL( bodyControl2,
                        /*aControlId*/EMyMsgComponentIdBody,
                        /*aIndex*/1,
                        /*aFormComponent*/EMsgBody );
    CleanupStack::Pop( bodyControl2 ); // bodyControl2

    TInt indexOfHrefControl = ConstructBodyTextL( *bodyControl1, *bodyControl2 );

    // Construct the view.
    TInt controlIdForFocus = indexOfHrefControl==1?EMyMsgComponentIdBody:EMsgComponentIdBody;
    iView->ExecuteL( ClientRect(), controlIdForFocus );
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::HandleKeyEventL
// ---------------------------------------------------------
//
TKeyResponse CPushViewerMsgEdAppUi::HandleKeyEventL
                                    ( const TKeyEvent& aKeyEvent,
                                      TEventCode aType )
    {
    PUSHLOG_WRITE_FORMAT
        ("CPushViewerMsgEdAppUi::HandleKeyEventL <%d>",aKeyEvent.iCode)
    // Check for iView and iContext objects are created before Handle
    if ( (NULL == iView) || (NULL == iContext) )
        {
        return (EKeyWasConsumed);
        }

    if ( aType == EEventKey )
        {
        switch ( aKeyEvent.iCode )
            {

            case EKeyDevice3:
                {
                // Selection key pressed. Behave as "Load service" if there is
                // service to download.
                const TMsvEntry& context = Model().Entry();
                if ( CPushMtmUtil::Attrs( context ) & EPushMtmAttrHasHref )
                    {
                    HandleCommandL( EPushViewerCmdLoadService );
                    }
                break;
                }

            case EKeyLeftUpArrow:         // Northwest
            case EStdKeyDevice10:         //   : Extra KeyEvent supports diagonal event simulator wedge
            case EKeyLeftArrow:           // West
            case EKeyLeftDownArrow:       // Southwest
            case EStdKeyDevice13:         //   : Extra KeyEvent supports diagonal event simulator wedge
                {
                HandleCommandL( EPushViewerCmdPreviousMessage );
                break;
                }

            case EKeyRightUpArrow:        // Northeast
            case EStdKeyDevice11:         //   : Extra KeyEvent supports diagonal event simulator wedge
            case EKeyRightArrow:          // East
            case EKeyRightDownArrow:      // Southeast
            case EStdKeyDevice12:         //   : Extra KeyEvent supports diagonal event simulator wedge
                {
                HandleCommandL( EPushViewerCmdNextMessage );
                break;
                }

            case EKeyBackspace :
                {
                CAknQueryDialog* dlg = CAknQueryDialog::NewL();
                if ( dlg->ExecuteLD
                    ( R_PUSHVIEWER_ENTRY_DELETE_CONFIRM ) == EAknSoftkeyYes )
                    {
                    DeleteAndExitL();
                    }
                break;
                }

            default:
                {
                iView->OfferKeyEventL( aKeyEvent, aType );
                break;
                }

            }
        }

    return EKeyWasConsumed;
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::DynInitMenuPaneL
// ---------------------------------------------------------
//
void CPushViewerMsgEdAppUi::DynInitMenuPaneL( TInt aMenuId,
                                         CEikMenuPane* aMenuPane )
    {
    if ( aMenuId == R_PUSHVIEWER_OPTIONS_MENU_PANE )
        {
        // Remove EPushViewerCmdLoadService if the current message
        // does not contain URL.
        CPushMsgEntryBase& modelBase = Model();
        if ( Model().PushMsgType() == KUidWapPushMsgSI.iUid )
            {
            CSIPushMsgEntry& si = (CSIPushMsgEntry&)modelBase;
            if ( !si.Url().Length() )
                {
                aMenuPane->DeleteMenuItem( EPushViewerCmdLoadService );
                Cba()->SetCommandSetL( R_PUSHVIEWER_SOFTKEYS );
                }
            }
        else if ( Model().PushMsgType() == KUidWapPushMsgSL.iUid )
            {
            CSLPushMsgEntry& sl = (CSLPushMsgEntry&)modelBase;
            __ASSERT_DEBUG( sl.Url().Length() != 0,
                            ViewerPanic( EPushViewerPanSlEmptyHref ) );
            if ( !sl.Url().Length() )
                {
                aMenuPane->DeleteMenuItem( EPushViewerCmdLoadService );
                Cba()->SetCommandSetL( R_PUSHVIEWER_SOFTKEYS );
                }
            }
        }
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::Model
// ---------------------------------------------------------
//
CPushMsgEntryBase& CPushViewerMsgEdAppUi::Model() const
    {
    PUSHLOG_WRITE_FORMAT("CPushViewerMsgEdAppUi::Model <%x>",iContext)
    return *iContext;
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::PushDoc
// ---------------------------------------------------------
//
CPushViewerDocument& CPushViewerMsgEdAppUi::PushDoc() const
    {
    PUSHLOG_ENTERFN("CPushViewerMsgEdAppUi::PushDoc")
    return REINTERPRET_CAST( CPushViewerDocument&, *Document() );
    PUSHLOG_LEAVEFN("CPushViewerMsgEdAppUi::PushDoc")
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::UpdateNaviPaneL
// ---------------------------------------------------------
//
void CPushViewerMsgEdAppUi::UpdateNaviPaneL()
    {
    PUSHLOG_ENTERFN("CPushViewerMsgEdAppUi::UpdateNaviPaneL")

    CEikImage* image = new (ELeave) CEikImage;
    CleanupStack::PushL( image );

    // Read image from resource.
    //
    TResourceReader reader;
    iCoeEnv->CreateResourceReaderLC( reader, R_PUSHVIEWER_NAVI_IMAGE );
    image->ConstructFromResourceL( reader );
    CleanupStack::PopAndDestroy(); // reader

    CMsgEditorAppUi::UpdateNaviPaneL( image->Bitmap(), image->Mask() );
    image->SetPictureOwnedExternally( ETrue );

    CleanupStack::PopAndDestroy(); // image

    PUSHLOG_LEAVEFN("CPushViewerMsgEdAppUi::UpdateNaviPaneL")
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::IsExpired
// ---------------------------------------------------------
//
TBool CPushViewerMsgEdAppUi::IsExpired()
    {
    TBool ret( EFalse );

    if ( Document()->Entry().iBioType == KUidWapPushMsgSI.iUid )
        {
        CSIPushMsgEntry* si = STATIC_CAST( CSIPushMsgEntry*, iContext );
        // Check expiration if expiration time was set.
        if ( si->Expires() != Time::NullTTime() )
            {
            TTime today;
            today.UniversalTime();
            if ( si->Expires() < today )
                {
                ret = ETrue;
                }
            }
        }

    return ret;
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::DoMsgSaveExitL
// ---------------------------------------------------------
//
void CPushViewerMsgEdAppUi::DoMsgSaveExitL()
    {
    PUSHLOG_ENTERFN("CPushViewerMsgEdAppUi::DoMsgSaveExitL");

    Exit( EAknSoftkeyBack );

    PUSHLOG_LEAVEFN("CPushViewerMsgEdAppUi::DoMsgSaveExitL")
    };

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::NotifyAndExitL
// ---------------------------------------------------------
//
void CPushViewerMsgEdAppUi::NotifyAndExitL( TInt aResId )
    {
    PUSHLOG_ENTERFN("CPushViewerMsgEdAppUi::NotifyAndExitL");

    HBufC* noteText = iCoeEnv->AllocReadResourceLC( aResId );
    CAknInformationNote* note = new (ELeave) CAknInformationNote( ETrue );
    note->ExecuteLD( *noteText );
    CleanupStack::PopAndDestroy( noteText ); // noteText

    // Close the application.
    if ( !IsAppShutterRunning() )
        {
        PUSHLOG_WRITE(" RunAppShutter")
        RunAppShutter();
        }

    PUSHLOG_LEAVEFN("CPushViewerMsgEdAppUi::NotifyAndExitL");
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::LaunchViewL
// ---------------------------------------------------------
//
void CPushViewerMsgEdAppUi::LaunchViewL()
    {
    __ASSERT_ALWAYS( Document()->Entry().iMtm == KUidMtmWapPush,
                     ViewerPanic( EPushViewerPanBadMtmType ) );

    // Construct model.
    CPushMsgEntryBase* newContext = NULL;
    const TInt32 bioType( Document()->Entry().iBioType );

    if ( bioType == KUidWapPushMsgSI.iUid )
        {
        newContext = CSIPushMsgEntry::NewL();
        }
    else if ( bioType == KUidWapPushMsgSL.iUid )
        {
        newContext = CSLPushMsgEntry::NewL();
        }
    else if ( bioType == KUidWapPushMsgMultiPart.iUid )
        {
#ifdef __TEST_MULTIPART_SUPP
        newContext = CMultiPartPushMsgEntry::NewL();
#else // __TEST_MULTIPART_SUPP
        User::Leave( KErrNotSupported );
#endif // __TEST_MULTIPART_SUPP
        }
    else
        {
        __ASSERT_DEBUG( EFalse, ViewerPanic( EPushViewerPanBadPushType ) );
        User::Leave( KErrNotSupported );
        }

    CleanupStack::PushL( newContext );
    newContext->RetrieveL( Document()->Session(), Document()->Entry().Id() );
    delete iContext;
    iContext = NULL;
    iContext = newContext;
    CleanupStack::Pop( newContext ); // newContext

    // Construct view.
    UpdateNaviPaneL();
    ConstructViewL();
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::HandleEntryChangeL
// ---------------------------------------------------------
//
void CPushViewerMsgEdAppUi::HandleEntryChangeL()
    {
    PUSHLOG_ENTERFN("CPushViewerMsgEdAppUi::HandleEntryChangeL");


    // We have to act only in case of such changes where the content
    // of the message changes, not only the unread/read flag.
    // The content handlers and the Push subsystem behave so that
    // the content is changed only when the entry becomes 'unread',
    // except one case where the CH changes the content, but the
    // message becomes 'read' (SL-execute-high). In this case
    // a flag indicates that the content was changed.

    // Get an up-to-date entry and check the necessary flags:
    TMsvEntry tEntry;
    TMsvId service;
    TMsvId entryId = Document()->Entry().Id();
    CMsvSession& msvSession = Document()->Session();
    User::LeaveIfError( msvSession.GetEntry( entryId, service, tEntry ) );
    TBool isChangeToUnread = tEntry.Unread();
    TBool contentChangedFlagSet =
          CPushMtmUtil::Attrs( tEntry ) & EPushMtmReadButContentChanged;

    if ( !isChangeToUnread && !contentChangedFlagSet )
        {
        // Nothing to do. Somebody just set it as read.
        PUSHLOG_WRITE(" Content not changed");
        }
    else
        {
        // Show note about the message changes & close the application.
        NotifyAndExitL( R_PUSHVIEWER_INFO_REPLACED );
        }

    PUSHLOG_LEAVEFN("CPushViewerMsgEdAppUi::HandleEntryChangeL")
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::HandleEntryDeletedL
// ---------------------------------------------------------
//
void CPushViewerMsgEdAppUi::HandleEntryDeletedL()
    {
    PUSHLOG_ENTERFN("CPushViewerMsgEdAppUi::HandleEntryDeletedL");
    // Check for iView and iContext objects are created before Handle
    if ( (NULL == iView) || (NULL == iContext))
        {
        return;
        }
    // Show note about the message deletion & close the application.
    NotifyAndExitL( R_PUSHVIEWER_INFO_DELETED );

    PUSHLOG_LEAVEFN("CPushViewerMsgEdAppUi::HandleEntryDeletedL")
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::GetHelpContextForControl
// ---------------------------------------------------------
//
void CPushViewerMsgEdAppUi::GetHelpContextForControl
                           ( TCoeHelpContext& /*aContext*/ ) const
    {
    }

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::HandleCommandL
// See DynInitMenuPaneL() to see which commands are valid
// on a given type of item.
// ---------------------------------------------------------
//
void CPushViewerMsgEdAppUi::HandleCommandL( TInt aCommand )
    {
    PUSHLOG_WRITE_FORMAT("CPushViewerMsgEdAppUi::HandleCommandL <%d>",aCommand)
    // Check for iView and iContext objects are created before Handle
    if ( (NULL == iView) || (NULL == iContext))
        {
        return;
        }
    switch ( aCommand )
        {
        case EPushViewerCmdLoadService:
            {
            if ( IsExpired() )
                {
                // The entry is expired.
                HBufC* value =
                    iCoeEnv->AllocReadResourceLC( R_PUSHVIEWER_EXPIRED_NOTE );
                CAknInformationNote* note =
                    new (ELeave) CAknInformationNote( ETrue );
                note->ExecuteLD( *value );
                CleanupStack::PopAndDestroy( value ); // value

                // Delete the expired message and exit.
                DeleteAndExitL();
                }
            else
                {
                PushDoc().IssueLoadServiceL();
                }
            break;
            }

        case EPushViewerCmdDelete:
            {
            CAknQueryDialog* dlg = CAknQueryDialog::NewL();
            if ( dlg->ExecuteLD
                ( R_PUSHVIEWER_ENTRY_DELETE_CONFIRM ) == EAknSoftkeyYes )
                {
                DeleteAndExitL();
                }
            break;
            }

        case EPushViewerCmdMessageInfo:
            {
            PushDoc().IssueMessageInfoL();
            break;
            }

        case EPushViewerCmdPreviousMessage:
            {
            if ( IsNextMessageAvailableL( EFalse ) )
                {
                NextMessageL( EFalse );
                }
            break;
            }

        case EPushViewerCmdNextMessage:
            {
            if ( IsNextMessageAvailableL( ETrue ) )
                {
                NextMessageL( ETrue );
                }
            break;
            }

#ifdef __SERIES60_HELP

        case EPushViewerCmdHelp:
            {
            HlpLauncher::LaunchHelpApplicationL
                ( iEikonEnv->WsSession(), AppHelpContextL() );
            break;
            }

#endif //__SERIES60_HELP

        case EPushViewerCmdBack:
        case EPushViewerCmdExit:
            {
            DoMsgSaveExitL();
            break;
            }

        default:
            {
            break;
            }
        }
    }

#ifdef __SERIES60_HELP

// ---------------------------------------------------------
// CPushViewerMsgEdAppUi::HelpContextL
// ---------------------------------------------------------
//
CArrayFix<TCoeHelpContext>* CPushViewerMsgEdAppUi::HelpContextL() const
    {
    CArrayFix<TCoeHelpContext>* contexts = new (ELeave) CArrayFixFlat<TCoeHelpContext>( 1 );
    CleanupStack::PushL( contexts );
    TCoeHelpContext help( TUid::Uid( EUidPushViewerApp ), KWPUSH_HLP_VIEWER );
    contexts->AppendL( help );
    CleanupStack::Pop(); // contexts
    return contexts;
    }

#endif // __SERIES60_HELP

// End of file.

