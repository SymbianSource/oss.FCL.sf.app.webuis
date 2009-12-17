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
#include <Browser_Platform_Variant.hrh>
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
#include <browseruisdkcrkeys.h>

#include <centralrepository.h>
#include <AknLayout2ScalableDef.h>
#include <AknFepInternalCRKeys.h>
#include <aknlayoutfont.h>
#include <PtiDefs.h>
#include <aknlayoutscalable_avkon.cdl.h>

#include "CommonConstants.h"
#include "BrowserGotoPane.h"
#include "BrowserAppUi.h"
#include "BrowserUtil.h"
#include "browser.hrh"
#include "favouriteslimits.h"
#include "BrowserAdaptiveListPopup.h"
#include "BrowserContentView.h"

#include "eikon.hrh"

#include <StringLoader.h>


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
        if ( iSearchEditor )
            {
            ConstructSearchPaneL();
            }
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
    else
        {
        // Clear the search text if goto is cancelled ( hidden ).
        if ( iSearchEditor )
            {
            SetSearchTextL( KNullDesC );
            }
        }

    if (AknLayoutUtils::PenEnabled() &&  ( !iSearchEditor )  )
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


    if ( iSearchEditor )
        {
        SetTextModeItalicL();
        }
    // To avoid Flickring Effect when SearchInputFrame is
    // re-created with new search provider icon.
    DrawNow( );

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
    delete iSearchEditor;
    delete iSearchInputFrame;

    delete iGotoKeyHandled;
    delete iEditor;
    delete iInputFrame;
    delete iPrevKeyword;
    delete iBAdaptiveListPopup;
    delete iDefaultSearchText;
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
    iHandleFEPFind( ETrue ),
    iBAdaptiveListPopup( NULL ),
    iGotoPaneActive( ETrue ),
    iSearchPaneActive( EFalse),
    iSearchIconId( 0 )
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

    TBool searchFeature = iContentView->ApiProvider().Preferences().SearchFeature();
    if (  searchFeature && !iFindKeywordMode )
        {
        iDefaultSearchText = StringLoader::LoadL( R_IS_WEB_SEARCH );
        ConstructSearchPaneL();
        }

      //adaptive popuplist
    if ( aPopupListStatus && AUTOCOMP )
        {
        iBAdaptiveListPopup =
            new (ELeave) CBrowserAdaptiveListPopup( iEditor, this, EGotoPane, searchFeature );
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
        if ( iSearchEditor && !iFindKeywordMode)
            {
            TRect clientRect = CBrowserAppUi::Static()->ClientRect();
            TAknWindowLineLayout findWindow = AknLayout::popup_find_window();

            TRect findWindowRect = AknLayoutUtils::RectFromCoords( clientRect,findWindow.il, findWindow.it,
                    findWindow.ir, findWindow.ib, findWindow.iW, findWindow.iH);

            // Now Increase the height of rect to make room for two editors (Goto + search)//
            findWindowRect.iTl.iY -= ( findWindow.iH  );
            SetRect( findWindowRect );
            }
        else
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
    TKeyResponse resp = EKeyWasNotConsumed;

    if (    AknLayoutUtils::PenEnabled()
         && PopupList()
         && PopupList()->DrawableWindow()->OrdinalPosition() > 0 )
        {
        PopupList()->SetOrdinalPosition(0);
        }

    if ( aType == EEventKey && aKeyEvent.iCode == EKeyEscape )  // Escape key event handling
        {
        CCoeEnv::Static()->AppUi()->RemoveFromStack( this );
        if ( PopupList() )
            {
            PopupList()->SetDirectoryModeL( ETrue );
            PopupList()->HidePopupL();
            }
        MakeVisible( EFalse );                                  // Hide Goto pane
        SetFocus( EFalse );
        resp = EKeyWasConsumed;                                 // should set the focus of container to ETrue...how?
        }

    if ( iFindKeywordMode )
        {
        if ( iContentView->MenuBar()->MenuPane()->IsVisible() )
            {
            return iContentView->MenuBar()->MenuPane()->OfferKeyEventL(  aKeyEvent, aType );
            }
        else
            {
            if ( aType == EEventKey )                           // Handle up and down arow keys to search for
                {                                               //   next and previous keywords.

                if (    aKeyEvent.iCode == EKeyLeftUpArrow      // Northwest
                     || aKeyEvent.iCode == EStdKeyDevice10      //   : Extra KeyEvent supports diagonal event simulator wedge
                     || aKeyEvent.iCode == EKeyUpArrow          // North
                     || aKeyEvent.iCode == EKeyRightUpArrow     // Northeast
                     || aKeyEvent.iCode == EStdKeyDevice11 )    //   : Extra KeyEvent supports diagonal event simulator wedge
                    {                                           // Any of those? If so, then...
                    iContentView->FindKeywordL( NULL, EFalse ); // Find previous keyword
                    iHandleFEPFind = EFalse;                    //   :
                    return EKeyWasConsumed;                     // And that consumes the key
                    }

                if (    aKeyEvent.iCode == EKeyLeftDownArrow    // Southwest
                     || aKeyEvent.iCode == EStdKeyDevice13      //   : Extra KeyEvent supports diagonal event simulator wedge
                     || aKeyEvent.iCode == EKeyDownArrow        // South
                     || aKeyEvent.iCode == EKeyRightDownArrow   // Southeast
                     || aKeyEvent.iCode == EStdKeyDevice12 )    //   : Extra KeyEvent supports diagonal event simulator wedge
                    {                                           // Any of those? If so, then...
                    iContentView->FindKeywordL( NULL, ETrue );  // Find next keyword
                    iHandleFEPFind = EFalse;                    //   :
                    return EKeyWasConsumed;                     // And that consumes the key
                    }

                if ( aKeyEvent.iCode == EKeyDevice3 )           // Select key?
                    {                                           // If so, then...
                    return EKeyWasConsumed;                     // Ignore select key
                    }

                }
            return iEditor->OfferKeyEventL( aKeyEvent, aType );  // Otherwise, just pass the key on to the editor
            }
        }

    else                                                        // *NOT* iFindKeywordMode
        {

        if (    aKeyEvent.iCode == EKeyRightUpArrow             // Northeast
             || aKeyEvent.iCode == EStdKeyDevice11              //   : Extra KeyEvent supports diagonal event simulator wedge
             || aKeyEvent.iCode == EKeyRightDownArrow           // Southeast
             || aKeyEvent.iCode == EStdKeyDevice12              //   : Extra KeyEvent supports diagonal event simulator wedge
             || aKeyEvent.iCode == EKeyLeftDownArrow            // Southwest
             || aKeyEvent.iCode == EStdKeyDevice13              //   : Extra KeyEvent supports diagonal event simulator wedge
             || aKeyEvent.iCode == EKeyLeftUpArrow              // Northwest
             || aKeyEvent.iCode == EStdKeyDevice10 )            //   : Extra KeyEvent supports diagonal event simulator wedge
            {                                                   // Any of those? If so, then...
            return EKeyWasConsumed;                             // Ignore diagonal navigation events here
            }

        // For touch UI, handle enter key from VKB as a "GOTO"
        if (AknLayoutUtils::PenEnabled() && aKeyEvent.iCode == EKeyEnter)
            {
            CBrowserAppUi::Static()->ActiveView()->HandleCommandL(EWmlCmdGotoPaneGoTo);
            return EKeyWasConsumed;
            }
        else if ( ((aKeyEvent.iCode == EKeyOK ) || (aKeyEvent.iCode == EKeyEnter))
             && iGPObserver 
             && IsFocused() )
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

        if ( iSearchEditor && iSearchPaneActive )
            {
            resp = iSearchEditor->OfferKeyEventL( aKeyEvent, aType );
            }
        else
            {
            resp = iEditor->OfferKeyEventL( aKeyEvent, aType );
            }

        if ((iBAdaptiveListPopup ) && (resp != EKeyWasConsumed))
          {
          resp = iBAdaptiveListPopup->OfferKeyEventL( aKeyEvent, aType );
          }


        if ( iSearchEditor )
            {
            if (    ( iGotoPaneActive || iSearchPaneActive)
                 && (resp != EKeyWasConsumed) )
                {

                if (  aKeyEvent.iCode == EKeyUpArrow )
                    {
                    resp = EKeyWasConsumed;
                    if ( iSearchPaneActive )
                        {
                        SetGotoPaneActiveL();

#if defined(BRDO_SEARCH_INTEGRATION_FF)
                        iEditor->RemoveFlagFromUserFlags( CEikEdwin::EAvkonDisableVKB );
#endif

                        }
                    }
                if ( aKeyEvent.iCode == EKeyDownArrow )
                    {
                    resp = EKeyWasConsumed;
                    if ( iGotoPaneActive )
                        {
                        SetSearchPaneActiveL();

#if defined(BRDO_SEARCH_INTEGRATION_FF)
                        iSearchEditor->RemoveFlagFromUserFlags( CEikEdwin::EAvkonDisableVKB );
#endif

                        }
                    }
                }
            }

        }

    return resp;

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

        // If search feature exists, check and route to appropriate editor //
        if ( iSearchEditor  )
            {
            if ( iSearchInputFrame->Rect().Contains(aPointerEvent.iPosition))
                {
                if ( iSearchPaneActive )
                    {
                    iSearchEditor->HandlePointerEventL(aPointerEvent);
                    }
                else
                    {
                    SetSearchPaneActiveL();
                    }
#if defined(BRDO_SEARCH_INTEGRATION_FF)
                iSearchEditor->RemoveFlagFromUserFlags( CEikEdwin::EAvkonDisableVKB);
#endif
                }
            else
                {
                if ( iGotoPaneActive )
                    {
                    iEditor->HandlePointerEventL(aPointerEvent);
                    }
                else
                    {
                    SetGotoPaneActiveL();
                    }
#if defined(BRDO_SEARCH_INTEGRATION_FF)
                iEditor->RemoveFlagFromUserFlags( CEikEdwin::EAvkonDisableVKB);
#endif
                }
            }
        else
            {
            iEditor->HandlePointerEventL(aPointerEvent);
            }
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
    if ( iSearchEditor && !iFindKeywordMode )
        {
        return 4; // iEditor, input frame, SearchEditor and Searchinput frame.
        }
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
        case 2:
            {
            return iSearchInputFrame;
            }
        case 3:
            {
            return iSearchEditor;
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
    if (iSearchEditor && !iFindKeywordMode )
        {
        // We need height and width of FindWindow
        TAknWindowLineLayout findWindow = AknLayout::popup_find_window();
        TRect findWindowRect = AknLayoutUtils::RectFromCoords( Rect(), findWindow.il,
                findWindow.it, findWindow.ir, findWindow.ib, findWindow.iW, findWindow.iH);

        TSize gotoSize( findWindowRect.Size() );
        TRect gotoRect( TPoint( 0,0 ), gotoSize );
        iInputFrame->SetRect( gotoRect );

        // Now set SearchPane right below GoTo pane //
        TRect searchRect( TPoint( 0, gotoRect.iBr.iY ), gotoSize );
        iSearchInputFrame->SetRect( searchRect );
        }
    else
        {
        iInputFrame->SetRect( Rect() );
        }
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
    if ( iGotoPaneActive )
        {
        iEditor->SetFocus( IsFocused() );
        }
    else if ( iSearchEditor && !iFindKeywordMode && iSearchPaneActive )
        {
        iSearchEditor->SetFocus( IsFocused() );
        }
    }



// ----------------------------------------------------------------------------
// CBrowserGotoPane::TextLength
// ----------------------------------------------------------------------------
//
TInt CBrowserGotoPane::SearchTextLength() const
    {
    TInt len = 0;
    if ( iSearchEditor )
        {
        len = iSearchEditor->TextLength();
        }
    return len;
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

    if ( TextLength()
            || SearchTextLength()
            || iFindKeywordMode)
        {
        if ( iSearchEditor && iSearchPaneActive )
            {
            retVal = HBufC::NewL( SearchTextLength() + 1 );
            }
        else
            {
            retVal = HBufC::NewL( TextLength() + 1 );
            }
        TPtr ptr = retVal->Des();
        if ( iSearchEditor && iSearchPaneActive )
            {
            iSearchEditor->GetText( ptr );
            }
        else
            {
            iEditor->GetText( ptr );
            }
        ptr.ZeroTerminate();
        if ( !iFindKeywordMode && !iSearchPaneActive )
            {
            Util::EncodeSpaces(retVal);
            }
        }

    return retVal;
    }


// ----------------------------------------------------------------------------
// CBrowserGotoPane::SetSearchTextL
// ----------------------------------------------------------------------------
//
void CBrowserGotoPane::SetSearchTextL( const TDesC& aTxt )
    {
    // for search pane, no default text
    if ( aTxt == KHttpString || aTxt == KWWWString)
        {
        iSearchEditor->SetTextL( &KNullDesC );
        }
    else
        {
        iSearchEditor->SetTextL( &aTxt );
        }
    TInt curPos = SearchTextLength();
    // Cursor to end, no selection.
    iSearchEditor->SetSelectionL( curPos, curPos );
    iSearchEditor->DrawNow();
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
    if ( iSearchEditor && iSearchPaneActive )
        {
        iSearchEditor->SelectAllL();
        }
    else
        {
        iEditor->SelectAllL();
        }
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
    if ( ( iEditor && iEditor->TextView() )
            || ( iSearchEditor && iSearchEditor->TextView()) )
        {
        // make sure observer is set
        iCoeEnv->SyncNotifyFocusObserversOfChangeInFocus();

           // simulate pointer event to force VKB

           // first get point at cursor location
        TInt pos = 0;
        CTextView* textView = NULL;
        if ( iSearchEditor && iSearchPaneActive )
            {
            pos = iSearchEditor->CursorPos();
            textView = iSearchEditor->TextView();
            }
        else
            {
            pos = iEditor->CursorPos();
            textView = iEditor->TextView();
            }

        TPoint curPos;
        textView->DocPosToXyPosL(pos, curPos);

        TPointerEvent pe;
        pe.iPosition = curPos;

        pe.iType = TPointerEvent::EButton1Down;

        TInt err(KErrNone);
        if ( iSearchEditor && iSearchPaneActive )
            {
            TRAP(err, iSearchEditor->HandlePointerEventL(pe));
            }
        else
            {
            TRAP(err, iEditor->HandlePointerEventL(pe));
            }

        if (err != KErrNone)
            {
            return;
            }

        pe.iType = TPointerEvent::EButton1Up;

        // VKB will only activate is nothing selected
        if ( iSearchEditor && iSearchPaneActive )
            {
            iSearchEditor->SetSelectionL(pos,pos);
            TRAP_IGNORE(iSearchEditor->HandlePointerEventL(pe));
            }
        else
            {
            iEditor->SetSelectionL(pos,pos);
            TRAP_IGNORE(iEditor->HandlePointerEventL(pe));
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::ConstructSearchPaneL
// ----------------------------------------------------------------------------
void CBrowserGotoPane::ConstructSearchPaneL()
    {

    TFileName iconFile;
    TInt iconId = iContentView->ApiProvider().Preferences().GetIntValue( KBrowserSearchIconId );
    iContentView->ApiProvider().Preferences().GetStringValueL( KBrowserSearchIconPath, KMaxFileName, iconFile );

    // If Icon File Path Changed or Icon Id Changed, Refresh the Icon for Search Pane.
    // Comparing Icon File path as well, because it may be possible that two different
    // Icon files have same icon id.
    if ( iconId != iSearchIconId
            || iSearchIconFilePath.Compare( iconFile ) != 0 )
        {

        TInt iconMaskId = iContentView->ApiProvider().Preferences().GetIntValue( KBrowserSearchIconMaskId );
        // Save IconId
        iSearchIconId = iconId;
        // Save Icon File
        iSearchIconFilePath = iconFile;

        // No Icon file or IconId or IconMaskId set , then it means no search provider is still
        // selected and set by search application, in that case we use the default icon for Search.
        if ( ! iconFile.Length()
                || iconId == -1
                || iconMaskId == -1 )
            {
            iconId = EMbmAvkonQgn_indi_find_glass;
            iconMaskId = EMbmAvkonQgn_indi_find_glass_mask;
            iconFile = KAvkonBitmapFile;
            }


        if ( iSearchEditor )
           {
           delete iSearchEditor;
           iSearchEditor = NULL;
           }

        if ( iSearchInputFrame )
           {
           delete iSearchInputFrame;
           iSearchInputFrame = NULL;
           }


        // iSearchEditor != NULL, implies presence of Search Feature, which can be
        // used to validate search feature exsistence, avoiding unecessary feature
        // check calls and need of separate variable.
        iSearchEditor = new (ELeave) CEikGlobalTextEditor;
        iSearchInputFrame = CAknInputFrame::NewL(
                iSearchEditor,
                EFalse,
                iconFile,
                iconId,
                iconMaskId,
                CAknInputFrame::EPopupLayout );

        iSearchInputFrame->SetContainerWindowL( *this);
        AknEditUtils::ConstructEditingL (   iSearchEditor,
                                              KFavouritesMaxUrlGotoPaneDefine,
                                              1,
                                              EAknEditorCharactersLowerCase,
                                              EAknEditorAlignRight,
                                              EFalse,
                                              ETrue,
                                              EFalse );

        iSearchEditor->SetContainerWindowL( *this );
        iSearchEditor->SetObserver( this );
        iSearchEditor->SetBorder( TGulBorder::ENone );
        iSearchEditor->SetAknEditorCase( EAknEditorLowerCase );
        iSearchEditor->SetAknEditorInputMode( EAknEditorTextInputMode );

        if (AVKONAPAC)
            {
            // Disallow chinese input.
            iSearchEditor->SetAknEditorAllowedInputModes( EAknEditorTextInputMode |
                    EAknEditorNumericInputMode );
            }

        //Search should use EAknEditorFlagDefault as search allows all types of input
        iSearchEditor->SetAknEditorFlags
            ( EAknEditorFlagDefault | EAknEditorFlagUseSCTNumericCharmap );

        iSearchEditor->SetAknEditorPermittedCaseModes (   EAknEditorUpperCase |
            EAknEditorLowerCase );

        iSearchEditor->SetEdwinObserver( iGotoKeyHandled );
        iSearchEditor->SetSkinBackgroundControlContextL(NULL);
        iSearchEditor->MakeVisible( ETrue );
        iSearchInputFrame->MakeVisible( ETrue );
        // Set the default text if not active//
        if ( ! iSearchPaneActive )
            {
            SetSearchTextL( *iDefaultSearchText );
            }
        iSearchInputFrame->ActivateL();
        }

    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::SetGotoPaneActive
// ----------------------------------------------------------------------------
void CBrowserGotoPane::SetGotoPaneActiveL()
    {

    iGotoPaneActive = ETrue;
    iSearchPaneActive = EFalse;

    if ( iSearchEditor )
        {
#if defined(BRDO_SEARCH_INTEGRATION_FF)
        iEditor->AddFlagToUserFlags( CEikEdwin::EAvkonDisableVKB );
#endif
        // if searchpane is empty add default text
        if ( !SearchTextLength() )
            {
            SetSearchTextL( *iDefaultSearchText );
            }

        // if gotopane is empty add default text
        if ( !TextLength() )
            {
            SetTextL( KWWWString );
            }

        iSearchEditor->SetFocus( EFalse);
        }

    iEditor->SetFocus( ETrue );
    CBrowserAppUi::Static()->UpdateCbaL();
    SetTextModeItalicL();
    iEditor->SetCursorPosL(iEditor->TextLength(), EFalse);
    DrawDeferred();
    }

// ----------------------------------------------------------------------------
// CBrowserGotoPane::SetSearchPaneActive
// ----------------------------------------------------------------------------
void CBrowserGotoPane::SetSearchPaneActiveL()
    {

    if ( iSearchEditor )
        {
#if defined(BRDO_SEARCH_INTEGRATION_FF)
        iSearchEditor->AddFlagToUserFlags( CEikEdwin::EAvkonDisableVKB );
#endif
        // if gotopane is empty add default text
        if ( !TextLength() )
            {
            SetTextL( KWWWString );
            }

        // if searchpane has default text remove it
        HBufC* text = iSearchEditor->GetTextInHBufL();
        if ( text )
            {
            CleanupStack::PushL( text );
            if ( !text->Compare( iDefaultSearchText->Des() ) )
                {
                SetSearchTextL( KNullDesC );
                }

            CleanupStack::PopAndDestroy( text );
            }

        iSearchPaneActive = ETrue;
        iGotoPaneActive = EFalse;
        iEditor->SetFocus( EFalse );
        if ( iBAdaptiveListPopup )
            iBAdaptiveListPopup->HidePopupL();
        iSearchEditor->SetFocus( ETrue );
        iSearchEditor->SetCursorPosL(iSearchEditor->TextLength(), EFalse);
        CBrowserAppUi::Static()->UpdateCbaL();

        SetTextModeItalicL();
        DrawDeferred();
        }
    }


// ----------------------------------------------------------------------------
// CBrowserBookmarksGotoPane::SetTextModeItalic
// ----------------------------------------------------------------------------
void CBrowserGotoPane::SetTextModeItalicL()
    {

    // Editor Control is laid in a scalable way, so we need to get the correct font
    // specification for setting CharFormatLayer, We could have used GetNearestFontInTwips,
    // as done above in SetTextL() but it does not provide correct fonts for editor.
    // We do not need to set the FontPosture back to EPostureUpright ( Normal ), as it
    // is automatically handled by AknLayoutUtils::LayoutEdwinScalable called by
    // iInputFrame->SetRect(), which overwrites all the properties for Editor.
    if ( iSearchEditor )
        {
        TAknTextComponentLayout   editorLayout;
        TBool apac( AknLayoutUtils::Variant() == EApacVariant && ( CAknInputFrame::EShowIndicators ) );
        editorLayout    = AknLayoutScalable_Avkon::input_popup_find_pane_t1( apac ? 2 : 0 );
        TAknTextLineLayout lineLayout = editorLayout.LayoutLine();
        TInt fontid =  lineLayout.FontId();
        const CAknLayoutFont *font = AknLayoutUtils::LayoutFontFromId( fontid  );

        TCharFormat charFormat;
        TCharFormatMask charFormatMask;
        charFormat.iFontSpec = font->FontSpecInTwips();
        charFormat.iFontSpec.iFontStyle.SetPosture( EPostureItalic );
        charFormatMask.SetAttrib(EAttFontTypeface);
        charFormatMask.SetAttrib(EAttFontHeight);
        charFormatMask.SetAttrib(EAttFontStrokeWeight);
        charFormatMask.SetAttrib(EAttFontPosture);

        // Owner ship of charFormatLayer is taken by Editor
        CCharFormatLayer* charFormatLayerItalics = NULL;
        CCharFormatLayer* charFormatLayerUpright = NULL;
        charFormatLayerItalics = CCharFormatLayer::NewL(charFormat,charFormatMask);
        charFormat.iFontSpec.iFontStyle.SetPosture( EPostureUpright );
        charFormatLayerUpright = CCharFormatLayer::NewL(charFormat,charFormatMask);
        if ( iSearchPaneActive   )
            {
            iSearchEditor->SetCharFormatLayer(charFormatLayerUpright);
            iEditor->SetCharFormatLayer(charFormatLayerItalics);
            }
        else
            {
            iSearchEditor->SetCharFormatLayer(charFormatLayerItalics);
            iEditor->SetCharFormatLayer(charFormatLayerUpright);
            }
        }
   }


//  END OF FILE
