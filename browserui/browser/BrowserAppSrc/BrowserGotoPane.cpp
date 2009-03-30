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
*
*
*
*/


// INCLUDE FILES
#include <eikmenub.h>
#include <eikdef.h>
#include <akninfrm.h>
#include <eikon.rsg>
#include <eikenv.h>
#include <txtglobl.h>
#include <gulfont.h>
#include <txtfrmat.h>
#include <eikgted.h>
#include <aknconsts.h>
#include <akneditstateindicator.h>
#include <AknsListBoxBackgroundControlContext.h>
#include <BrowserNG.rsg>
#include <Featmgr.h>
#include <fepbase.h>
#include <aknutils.h>

#include <centralrepository.h> 
#include <AknFepInternalCRKeys.h>
#include <PtiDefs.h>

#include "CommonConstants.h"
#include "BrowserGotoPane.h"
#include "BrowserAppUi.h"
#include "BrowserUtil.h"
#include "browser.hrh"
#include "FavouritesLimits.h"
#include "BrowserAdaptiveListPopup.h"
#include "BrowserContentView.h"

#include "eikon.hrh"


// ---------------------------------------------------------------------------
// CBrowserGotoPane::NewL
// ---------------------------------------------------------------------------
//
CBrowserGotoPane* CBrowserGotoPane::NewL(
        const CCoeControl* aParent, TInt aIconBitmapId, TInt aIconMaskId,
        TBool aPopupListStatus, CBrowserContentView* aContentView,
        TBool aFindKeywordMode )
    {
    CBrowserGotoPane* gotoPane = new(ELeave)
        CBrowserGotoPane( aContentView, aFindKeywordMode );

    CleanupStack::PushL( gotoPane );
    gotoPane->ConstructL( aParent, aIconBitmapId, aIconMaskId, aPopupListStatus );
    gotoPane->MakeVisibleL( EFalse );
    gotoPane->SetFocus( EFalse );
    CleanupStack::Pop();    // gotoPane

    return gotoPane;
    }

// ---------------------------------------------------------------------------
// CBrowserGotoPane::MakeVisibleL
// ---------------------------------------------------------------------------
//
void CBrowserGotoPane::MakeVisibleL( TBool aVisible )
    {
    if ( aVisible )
    {
        HandleFindSizeChanged();
        if ( AUTOCOMP ) //ask the feature manager
        {
        //2.1 does not put http:// there
            SetTextL( KNullDesC );
        }
        else
        {
        //2.0 does put the http:// there
            SetTextL( KHttpString );
        }
    }
    if (AknLayoutUtils::PenEnabled())
        {
        SetPointerCapture(aVisible);
        if (aVisible)
            {
            ActivateVKB();
            }
        }

    CCoeControl::MakeVisible( aVisible );

    iGotoKeyHandled->Reset();
    iGotoKeyHandled->EnableL( aVisible );
    }

// ---------------------------------------------------------------------------
// CBrowserGotoPane::MakeVisible
// ---------------------------------------------------------------------------
//
void CBrowserGotoPane::MakeVisible( TBool aVisible )
    {
    TRAP_IGNORE( MakeVisibleL( aVisible ) );
    }

// ---------------------------------------------------------------------------
// CBrowserGotoPane::~CBrowserGotoPane
// ---------------------------------------------------------------------------
//
CBrowserGotoPane::~CBrowserGotoPane()
    {
    if (iAvkonAppUi!=NULL)
        {
        iAvkonAppUi->RemoveFromStack( iGotoKeyHandled );
        }
    delete iGotoKeyHandled;
    delete iEditor;
    delete iInputFrame;
    delete iPrevKeyword;
    delete iBAdaptiveListPopup;
    iContentView = NULL;
    CCoeEnv::Static()->RemoveFepObserver(static_cast<MCoeFepObserver &>(*this));
    }

// ---------------------------------------------------------------------------
// CBrowserGotoPane::CBrowserGotoPane
// ---------------------------------------------------------------------------
//
CBrowserGotoPane::CBrowserGotoPane( CBrowserContentView* aContentView, TBool aFindKeywordMode )
:   iContentView( aContentView),
    iFindKeywordMode( aFindKeywordMode ),
    iHandleFEPFind( ETrue )
    ,iBAdaptiveListPopup( NULL )
    {
    }

// ---------------------------------------------------------------------------
// CBrowserGotoPane::ConstructL
// ---------------------------------------------------------------------------
//
void CBrowserGotoPane::ConstructL ( const CCoeControl* aParent, TInt aIconBitmapId, TInt aIconMaskId,
            TBool
            aPopupListStatus
            )
    {
    CreateWindowL();

    // remove 'const' modifier and set parent control
    SetMopParent( CONST_CAST( CCoeControl*, aParent ) );

    iEditor = new (ELeave) CEikGlobalTextEditor;

    iInputFrame = CAknInputFrame::NewL(
        iEditor,
        EFalse,
        KAvkonBitmapFile,
        aIconBitmapId,
        aIconMaskId,
        CAknInputFrame::EPopupLayout );

    iInputFrame->SetContainerWindowL( *this );

    AknEditUtils::ConstructEditingL (   iEditor,
                                        KFavouritesMaxUrlGotoPaneDefine,
                                        1,
                                        EAknEditorCharactersLowerCase,
                                        EAknEditorAlignRight,
                                        EFalse,
                                        ETrue,
                                        EFalse );

    iEditor->SetContainerWindowL( *this );
    iEditor->SetObserver( this );
    iEditor->SetBorder( TGulBorder::ENone );
    iEditor->SetAknEditorCase( EAknEditorLowerCase );
    iEditor->SetAknEditorInputMode( EAknEditorTextInputMode );

    if (AVKONAPAC)
        {
        // Disallow chinese input.
        iEditor->SetAknEditorAllowedInputModes( EAknEditorTextInputMode |
                                            EAknEditorNumericInputMode );
        }
    TInt editorFlags =  ((iFindKeywordMode) ? EAknEditorFlagDefault : EAknEditorFlagLatinInputModesOnly) |EAknEditorFlagUseSCTNumericCharmap;
#ifdef RD_INTELLIGENT_TEXT_INPUT
    TInt physicalKeyboards = 0; 
    CRepository* aknFepRepository = CRepository::NewL( KCRUidAknFep );
	User::LeaveIfNull( aknFepRepository );

    aknFepRepository->Get( KAknFepPhysicalKeyboards,  physicalKeyboards );
    delete aknFepRepository; 
        
	if ( physicalKeyboards &&   EPtiKeyboardQwerty3x11 ) {
        editorFlags = (editorFlags | EAknEditorFlagNoT9);
	}
#endif 
    iEditor->SetAknEditorFlags( editorFlags ); 

    iEditor->SetAknEditorPermittedCaseModes (   EAknEditorUpperCase |
                                                EAknEditorLowerCase );

    iGotoKeyHandled = new (ELeave) CBrowserKeyEventHandled( *this );
    iAvkonAppUi->AddToStackL( iGotoKeyHandled,
                                ECoeStackPriorityFep + 1,
                                ECoeStackFlagRefusesFocus );
    iEditor->SetEdwinObserver( iGotoKeyHandled );
    iEditor->SetSkinBackgroundControlContextL(NULL);

    //adaptive popuplist
    if ( aPopupListStatus && AUTOCOMP )
        {
        iBAdaptiveListPopup =
            new (ELeave) CBrowserAdaptiveListPopup( iEditor, this, EGotoPane );
        iBAdaptiveListPopup->ConstructL();
        iEditor->SetObserver( iBAdaptiveListPopup );
        if (iContentView)
            {
            iBAdaptiveListPopup->SetUrlSuffixList(iContentView->ApiProvider().Preferences().URLSuffixList());
            iBAdaptiveListPopup->SetMaxRecentUrls(iContentView->ApiProvider().Preferences().MaxRecentUrls());
            }
        }

    CCoeEnv::Static()->AddFepObserverL(static_cast<MCoeFepObserver &>(*this));
    iPrevKeyword = KNullDesC().AllocL();
    }

// ---------------------------------------------------------------------------
// CBrowserGotoPane::SetOrdinalPosition
// ---------------------------------------------------------------------------
//
void CBrowserGotoPane::SetOrdinalPosition( TInt aPos )
    {
    Window().SetOrdinalPosition( aPos );
    if ( iBAdaptiveListPopup )
        {
        iBAdaptiveListPopup->SetOrdinalPosition( aPos );
        }
    }

// ---------------------------------------------------------------------------
// CBrowserGotoPane::HandleFindSizeChanged
// ---------------------------------------------------------------------------
//
void CBrowserGotoPane::HandleFindSizeChanged()
    {

    if ( !AknLayoutUtils::PenEnabled() )
        {
        TRect parentrect = iAvkonAppUi->ApplicationRect();

        TAknLayoutRect lrect;
        lrect.LayoutRect( parentrect,
                        AknLayout::main_pane( CBrowserAppUi::Static()->ApplicationRect(), 0, 1, 1 )
                        );
        AknLayoutUtils::LayoutControl ( this,
                                        lrect.Rect(),
                                        AknLayout::popup_find_window() );
        }
    else
        {
        // The ClientRect() will be the application rectangle minus any
        // toolbars/menu bars etc.
        AknLayoutUtils::LayoutControl ( this,
                                        CBrowserAppUi::Static()->ClientRect(),
                                        AknLayout::popup_find_window() );
        }

    }

// ---------------------------------------------------------------------------
// CBrowserGotoPane::EnableKeyEventHandler
// ---------------------------------------------------------------------------
//
void CBrowserGotoPane::EnableKeyEventHandlerL( TBool aEnable )
    {
    iGotoKeyHandled->EnableL( aEnable );
    }

// ---------------------------------------------------------------------------
// CBrowserGotoPane::HandleControlEventL
// ---------------------------------------------------------------------------
//
void CBrowserGotoPane::HandleControlEventL ( CCoeControl* /*aControl*/,
                                                        TCoeEvent aEventType )
    {
    if ( iFindKeywordMode )
        {
        if ( iEditor->AknEditorCurrentInputMode() == EAknEditorCalculatorNumberModeKeymap
            // When the user copies and paste's content to the find dialog, the editor
            // is in Text Input mode, The engine should still get the find keyword request
            || iEditor->AknEditorCurrentInputMode() == EAknEditorTextInputMode
            )
            {
            if ( aEventType == MCoeControlObserver::EEventStateChanged )
                {
                HBufC* newText = GetTextL();
                if (!newText)
                    {
                    newText = KNullDesC().AllocL();
                    }

                if ( iPrevKeyword->Compare( *newText ) )
                    {
                    delete iPrevKeyword;
                    iPrevKeyword = NULL;
                    iPrevKeyword = newText;
                    // Find the typed keyword.
                    iContentView->FindKeywordL( GetTextL() );
                    iHandleFEPFind = EFalse;
                    }
                else
                    {
                    delete newText;
                    }
                }
            }
        }

    switch (aEventType)
        {
        case EEventRequestFocus:
            {
            FocusChanged( EDrawNow );
            break;
            }

        default:
            break;
        }
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::OfferKeyEventL
// ----------------------------------------------------------------------------
//
TKeyResponse CBrowserGotoPane::OfferKeyEventL
( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    TKeyResponse resp;

    if ( AknLayoutUtils::PenEnabled() &&
         PopupList() &&
         PopupList()->DrawableWindow()->OrdinalPosition() > 0)
        {
        PopupList()->SetOrdinalPosition(0);
        }

    // Escape key event handling
    if ( aType == EEventKey && aKeyEvent.iCode == EKeyEscape )
        {
        CCoeEnv::Static()->AppUi()->RemoveFromStack( this );
        if ( PopupList() )
            {
            PopupList()->SetDirectoryModeL( ETrue );
            PopupList()->HidePopupL();
            }

        // Hide Goto pane
        MakeVisible( EFalse );
        SetFocus( EFalse );
        // should set the focus of container to ETrue...how?
        resp = EKeyWasConsumed;
        }

    if ( iFindKeywordMode )
        {
        if ( iContentView->MenuBar()->MenuPane()->IsVisible() )
            {
            return iContentView->MenuBar()->MenuPane()->OfferKeyEventL(  aKeyEvent, aType );
            }
        else
            {
            // Handle up and down arow keys to search for
            // next and previous keywords.
            if ( aType == EEventKey )
                {
                if (    aKeyEvent.iCode == EKeyLeftUpArrow      // Northwest
                     || aKeyEvent.iCode == EStdKeyDevice10      //   : Extra KeyEvent supports diagonal event simulator wedge
                     || aKeyEvent.iCode == EKeyUpArrow          // North
                     || aKeyEvent.iCode == EKeyRightUpArrow     // Northeast
                     || aKeyEvent.iCode == EStdKeyDevice11 )    //   : Extra KeyEvent supports diagonal event simulator wedge
                    {
                    // Find previous keyword
                    iContentView->FindKeywordL( NULL, EFalse );
                    iHandleFEPFind = EFalse;
                    return EKeyWasConsumed;
                    }
                if (    aKeyEvent.iCode == EKeyLeftDownArrow    // Southwest
                     || aKeyEvent.iCode == EStdKeyDevice13      //   : Extra KeyEvent supports diagonal event simulator wedge
                     || aKeyEvent.iCode == EKeyDownArrow        // South
                     || aKeyEvent.iCode == EKeyRightDownArrow   // Southeast
                     || aKeyEvent.iCode == EStdKeyDevice12 )    //   : Extra KeyEvent supports diagonal event simulator wedge
                    {
                    // Find next keyword
                    iContentView->FindKeywordL( NULL, ETrue );
                    iHandleFEPFind = EFalse;
                    return EKeyWasConsumed;
                    }
                if ( aKeyEvent.iCode == EKeyDevice3 )
                    {
                    // ignore select key
                    return EKeyWasConsumed;
                    }
                }
            return iEditor->OfferKeyEventL( aKeyEvent, aType );
            }
        }
    else
        {
        if ( (aKeyEvent.iCode == EKeyOK ) && iGPObserver && IsFocused() )
            {
            if ( CBrowserAppUi::Static()->ContentView()->MenuBar()->MenuPane()->IsVisible() )
                {
                return CBrowserAppUi::Static()->ContentView()->MenuBar()->MenuPane()->OfferKeyEventL( aKeyEvent, aType );
                }
            else
                {
                // If there is an observer and we have the focus, enter key is
                // consumed and observer is notified.
                iGPObserver->HandleGotoPaneEventL
                                ( this, MGotoPaneObserver::EEventEnterKeyPressed );
                return EKeyWasConsumed;
                }
            }
        // For touch UI, handle enter key from VKB as a "GOTO"
        else if (AknLayoutUtils::PenEnabled() && aKeyEvent.iCode == EKeyEnter)
            {
            CBrowserAppUi::Static()->ActiveView()->HandleCommandL(EWmlCmdGotoPaneGoTo);
            return EKeyWasConsumed;
            }

        resp = iEditor->OfferKeyEventL( aKeyEvent, aType );

        if ((iBAdaptiveListPopup ) && (resp != EKeyWasConsumed))
            {
            resp = iBAdaptiveListPopup->OfferKeyEventL( aKeyEvent, aType );
            }
        return resp;
        }
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::HandlePointerEventL
// ----------------------------------------------------------------------------
//
void CBrowserGotoPane::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
    if ( !AknLayoutUtils::PenEnabled() )
        {
        return;
        }

    // ponter is in goto pane
    if (Rect().Contains(aPointerEvent.iPosition))
        {
        iGotoKeyHandled->EnableL(ETrue);
        iEditor->HandlePointerEventL(aPointerEvent);
        iGotoKeyHandled->SetFirstKeyEvent(EFalse);
        }
    else
        {
        // pointer outside of control
        CBrowserAppUi::Static()->ActiveView()->HandleCommandL(EWmlCmdGotoPaneCancel);
        }
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::CountComponentControls
// ----------------------------------------------------------------------------
//
TInt CBrowserGotoPane::CountComponentControls() const
    {
    return 2;   // iEditor and input frame
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::ComponentControl
// ----------------------------------------------------------------------------
//
CCoeControl* CBrowserGotoPane::ComponentControl ( TInt aIndex ) const
    {
    switch(aIndex)
        {
        case 0:
            {
            return iInputFrame;
            }
        case 1:
            {
            return iEditor;
            }
        default:
            return NULL;
        }
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::SizeChanged
// ----------------------------------------------------------------------------
//
void CBrowserGotoPane::SizeChanged()
    {
    iInputFrame->SetRect( Rect() );
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::FocusChanged
// ----------------------------------------------------------------------------
//
void CBrowserGotoPane::FocusChanged( TDrawNow aDrawNow )
    {
    // this is a workaround
    TRAP_IGNORE( iGotoKeyHandled->EnableL( IsFocused() ) );
    CCoeControl::FocusChanged( aDrawNow );
    iEditor->SetFocus( IsFocused() );
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::TextLength
// ----------------------------------------------------------------------------
//
TInt CBrowserGotoPane::TextLength() const
    {
    return iEditor->TextLength();
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::GetText
// ----------------------------------------------------------------------------
//
HBufC* CBrowserGotoPane::GetTextL() const
    {
    HBufC *retVal = NULL;

    if ( TextLength() || iFindKeywordMode)
        {
        retVal = HBufC::NewL( TextLength() + 1 );
        TPtr ptr = retVal->Des();
        iEditor->GetText( ptr );
        ptr.ZeroTerminate();
        if (!iFindKeywordMode)
           Util::EncodeSpaces(retVal);

        }

    return retVal;
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::SetTextL
// ----------------------------------------------------------------------------
//
void CBrowserGotoPane::SetTextL( const TDesC& aTxt )
    {
    iEditor->SetTextL( &aTxt );
    TInt curPos = TextLength();
    // Cursor to end, no selection.
    iEditor->SetSelectionL( curPos, curPos );
    iEditor->DrawNow();
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::SelectAllL
// ----------------------------------------------------------------------------
//
void CBrowserGotoPane::SelectAllL()
    {
    iEditor->SelectAllL();
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::SetInfoTextL
// ----------------------------------------------------------------------------
//
void CBrowserGotoPane::SetInfoTextL( const TDesC& aText )
    {
    iInputFrame->SetInfoTextL( aText );
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::ClipboardL
// ----------------------------------------------------------------------------
//
void CBrowserGotoPane::ClipboardL( CEikEdwin::TClipboardFunc aClipboardFunc )
    {
    iEditor->ClipboardL( aClipboardFunc );
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::Editor
// ----------------------------------------------------------------------------
CEikEdwin* CBrowserGotoPane::Editor() const
    {
    return iEditor;
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::SetGPObserver
// ----------------------------------------------------------------------------
//
void CBrowserGotoPane::SetGPObserver( MGotoPaneObserver* aObserver )
    {
    __ASSERT_DEBUG( !iGPObserver, \
        Util::Panic( Util::EGotoPaneObserverAlreadySet ) );
    iGPObserver = aObserver;
    }

// ----------------------------------------------------------------------------
// CBrowserKeyEventHandled::CBrowserKeyEventHandled
// ----------------------------------------------------------------------------
//
CBrowserKeyEventHandled::CBrowserKeyEventHandled(
                                    CBrowserGotoPane& aGotoPane ) :
    iEnabled( EFalse )
    ,iFirstKeyEvent( ETrue )
    ,iLeftRightEvent ( EFalse )
    ,iGotoPane( aGotoPane )
    {
    }


// ----------------------------------------------------------------------------
// CBrowserKeyEventHandled::OfferKeyEventL
// ----------------------------------------------------------------------------
//
TKeyResponse
    CBrowserKeyEventHandled::OfferKeyEventL( const TKeyEvent& aKeyEvent,
                                                TEventCode aType)
    {
    TKeyResponse response( EKeyWasNotConsumed );
    iLeftRightEvent = EFalse;

    if ( iEnabled && aType == EEventKey )
        {
        switch( aKeyEvent.iCode )
            {
            case    EKeyUpArrow:
            case    EKeyDownArrow:
                {
                iFirstKeyEvent = EFalse;
                break;
                }

            case    EKeyLeftArrow:
            case    EKeyRightArrow:
                {
                iFirstKeyEvent = EFalse;
                iLeftRightEvent = ETrue;

                // Below added to compensate for a weird key event situation:
                //   For some reason the CBrowserGotoPane::OfferKeyEventL function
                //   is not being called for an EEventKey when you press the left/right
                //   arrows in the menupane produced to paste previously used special
                //   chars (this is brought up by pressing the pen button on the phone).
                //   This only happens when the cursor is in the first or last position in
                //   the goto pane with text in it.
                //   Therefore, in this particular scenario, we need to process the EEventKey
                //   here since we will not be able to in CBrowserGotoPane::OfferKeyEventL
                if (CBrowserAppUi::Static()->ContentView()->MenuBar()->MenuPane()->IsFocused())
                    {
                    response = CBrowserAppUi::Static()->ContentView()->MenuBar()->MenuPane()->OfferKeyEventL( aKeyEvent, aType );
                    }
                break;
                }

            case    EKeyBackspace:
            case    EKeyDelete:
                {
                if ( iFirstKeyEvent )
                    {
                    iFirstKeyEvent = EFalse;
                    iCoeEnv->SimulateKeyEventL( aKeyEvent, aType );
                    response = EKeyWasConsumed;
                    }
                break;
                }

            default:
                {
                if ( iFirstKeyEvent )
                    {
                    if ( TChar( aKeyEvent.iCode ).IsPrint() )
                        {
                        iFirstKeyEvent = EFalse;
                        response = EKeyWasConsumed;
                        iCoeEnv->SimulateKeyEventL( aKeyEvent, aType );
                        }
                    }
                break;
                }
            }
//        CBrowserAppUi::Static()->UpdateCbaL();
        }
    return response;
    }

// ----------------------------------------------------------------------------
// CBrowserKeyEventHandled::HandleEdwinEventL
// ----------------------------------------------------------------------------
//
void CBrowserKeyEventHandled::HandleEdwinEventL( CEikEdwin* /*aEdwin*/,
                                                    TEdwinEvent aEventType )
    {
    // if text changed in the editor we don't have to handle clear key anymore
    if ( aEventType == EEventTextUpdate )
        {
        iFirstKeyEvent = EFalse;
        }
    }

// ----------------------------------------------------------------------------
// CBrowserKeyEventHandled::EnableL
// ----------------------------------------------------------------------------
//
void CBrowserKeyEventHandled::EnableL( TBool aEnable )
    {
    iEnabled = aEnable;
    }

// ----------------------------------------------------------------------------
// CBrowserKeyEventHandled::Reset
// ----------------------------------------------------------------------------
//
void CBrowserKeyEventHandled::Reset()
    {
    iFirstKeyEvent = ETrue;
    }

// ---------------------------------------------------------------------------
// CBrowserGotoPane::PopupList
// ---------------------------------------------------------------------------
//
CBrowserAdaptiveListPopup* CBrowserGotoPane::PopupList()
    {
    return iBAdaptiveListPopup;
    }
// ---------------------------------------------------------------------------
// CBrowserGotoPane::HandleResourceChange
// ---------------------------------------------------------------------------
//
void CBrowserGotoPane::HandleResourceChange(
                TInt
                aType
                )
    {
    CCoeControl::HandleResourceChange( aType );

    if ( iBAdaptiveListPopup )
        {
        iBAdaptiveListPopup->HandleResourceChange( aType );
        }
    }

// ---------------------------------------------------------------------------
// CBrowserGotoPane::HandleStartOfTransactionL()
// ---------------------------------------------------------------------------
//
void CBrowserGotoPane::HandleStartOfTransactionL()
    {
    }

// ---------------------------------------------------------------------------
// CBrowserGotoPane::HandleCompletionOfTransactionL()
// ---------------------------------------------------------------------------
//
void CBrowserGotoPane::HandleCompletionOfTransactionL()
    {
    if ( iContentView && iFindKeywordMode && this->IsVisible() &&
        !iContentView->MenuBar()->MenuPane()->IsVisible() )
        {
        if ( GetTextL() )
            {
            // Find the typed keyword.
            if ( iHandleFEPFind && iPrevKeyword->Compare(GetTextL()->Des()) )
                {
                iContentView->FindKeywordL( GetTextL() );
                }
            }
        }
    iHandleFEPFind = ETrue;
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::ActivateVKB
// ----------------------------------------------------------------------------
void CBrowserGotoPane::ActivateVKB()
    {
    if (iEditor && iEditor->TextView())
        {
        // make sure observer is set
        iCoeEnv->SyncNotifyFocusObserversOfChangeInFocus();

        // simulate pointer event to force VKB

        // first get point at cursor location
        TInt pos = iEditor->CursorPos();
        CTextView* textView = iEditor->TextView();
        TPoint curPos;
        textView->DocPosToXyPosL(pos, curPos);

        TPointerEvent pe;
        pe.iPosition = curPos;

        pe.iType = TPointerEvent::EButton1Down;
        TInt err(KErrNone);
        TRAP(err, iEditor->HandlePointerEventL(pe));
        if (err != KErrNone)
            {
            return;
            }

        // VKB will only activate is nothing selected
        iEditor->SetSelectionL(pos,pos);

        pe.iType = TPointerEvent::EButton1Up;
        if (KErrNone == err)
           {
           TRAP_IGNORE(iEditor->HandlePointerEventL(pe));
           }
        }
    }

//  END OF FILE
