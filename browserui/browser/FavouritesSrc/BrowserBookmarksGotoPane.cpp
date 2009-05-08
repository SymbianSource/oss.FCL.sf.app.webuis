/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
*      Implementation of CBrowserBookmarksGotoPane.
*
*
*/


// INCLUDE FILES

#include <akninfrm.h>
#include <eikedwin.h>
#include <FavouritesLimits.h>

#include <aknconsts.h>
#include <akneditstateindicator.h>
#include <AknsListBoxBackgroundControlContext.h>
#include <AknsFrameBackgroundControlContext.h>

#include <BrowserNG.rsg>

#include <AknsUtils.h>
#include <AppApacLayout.cdl.h>
#include <AppLayout.cdl.h>
#include <AknLayout.cdl.h>

#include <centralrepository.h>
#include <AknFepInternalCRKeys.h>
#include <PtiDefs.h>

#include "BrowserBookmarksGotoPane.h"
#include "BrowserUtil.h"
#include "commonconstants.h"
#include "BrowserAdaptiveListPopup.h"
#include "WmlBrowserBuild.h" //feature manager
#include "browser.hrh"
#include "BrowserAppUi.h"
#include "BrowserFavouritesView.h"
#include "Preferences.h"
#include <Featmgr.h>

#include <eikpriv.rsg>
#include <BARSREAD.H>

#include <AknLayout2ScalableDef.h>
#include <aknlayoutfont.h>
#include <aknlayoutscalable_avkon.cdl.h>

#include <StringLoader.h>
#include <skinlayout.cdl.h>
using namespace SkinLayout;

// CONSTANTS
const TInt EUseSkinContext = 0x01;
const TInt EParentAbsolute = 0x02;
const TInt KRightSpace = 10; //space in pixels left at the end of text editor.
// CONSTANTS

_LIT(KAddressText,"http://www.");

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::NewL
// ---------------------------------------------------------
//
CBrowserBookmarksGotoPane* CBrowserBookmarksGotoPane::NewL
        (
        const CCoeControl& aParent,
        CBrowserFavouritesView* aContentView,
        const TDesC& aBitmapfile,
        TInt aIconBitmapId,
        TInt aIconMaskId,
        TBool aPopupListStatus,
        TBool aSearchPaneMode
        )
    {
    CBrowserBookmarksGotoPane* gotoPane =
        new(ELeave) CBrowserBookmarksGotoPane(aContentView, aSearchPaneMode);
    CleanupStack::PushL( gotoPane );
    gotoPane->ConstructL( aParent, aBitmapfile, aIconBitmapId, aIconMaskId, aPopupListStatus );
    CleanupStack::Pop();    // gotoPane
    return gotoPane;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::~CBrowserBookmarksGotoPane
// ---------------------------------------------------------
//
CBrowserBookmarksGotoPane::~CBrowserBookmarksGotoPane()
    {
    delete iEditor;
    delete iInputFrame;
    delete iSkinContext;
    delete iInputContext;
    delete iBAdaptiveListPopup;
    delete iDefaultSearchText;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::BeginEditingL()
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::BeginEditingL()
    {
    if ( !iFrozen )
        {
        // If its a GoTo Pane Mode.
        if( !iSearchPaneMode )
            {
            if( iView->ApiProvider().Preferences().SearchFeature() )
                {
                // If there is no text, then we need to place back
                // the default text, else preserve the previously
                // typed text.
                HBufC* text = iEditor->GetTextInHBufL();
                if( !text )
                    {
                    SetTextL( KWWWString );
                    }
                else
                    {
                    delete text;
                    }
                }
            else
                SetTextL( KWWWString );
            }
        else
            {
            if( iView->ApiProvider().Preferences().SearchFeature()  )
                {
                //Clear searchpane on focus if default text is there
                HBufC* text = iEditor->GetTextInHBufL();
                if ( text )
                    {
                    CleanupStack::PushL( text );
                    if ( !text->Compare( iDefaultSearchText->Des() ) )
                        {
                        iEditor->SetTextL( &KNullDesC );
                        }
                    CleanupStack::PopAndDestroy( text );
                    }
                }
            }

        MakeVisible( ETrue );
        SetEditingL( ETrue );
        iEditor->SetCursorPosL( iEditor->TextLength(), EFalse );
        if (AknLayoutUtils::PenEnabled()
                &&  ( !iView->ApiProvider().Preferences().SearchFeature())  )
            {
            ActivateVkbL();
            }
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::CancelEditingL()
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::CancelEditingL()
    {
    if ( !iFrozen )
        {
        // Clear selection when focus moved, only
        // affects when both the editors are visible.
        // no need for condition.
        TInt pos = iEditor->CursorPos();
        iEditor->SetSelectionL(pos,pos);
        SetEditingL( EFalse );

        // Cancel Adaptive List popup if its active
        if ( NULL != iBAdaptiveListPopup)
            {
            iBAdaptiveListPopup->SetDirectoryModeL( ETrue );
            iBAdaptiveListPopup->HidePopupL();
            }
        }

    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::SetTextL
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::SetTextL
( const TDesC& aText, TBool aClipToFit /*=ETrue*/, TBool aCancelEditing )
    {

    CFbsFont * font = NULL;

    if ( !iFrozen )
        {
        if ( aClipToFit && aText.Length() )
            {
            // Allocate 3 chars more (make sure "..." fits).
            HBufC* buf = HBufC::NewLC( aText.Length() + 3 );
            *buf = aText;
            TPtr ptr( buf->Des() );
            CTextView* textView = iEditor->TextView();
            if ( textView )
                {
                //The Textview does not use the iConEnv->NormalFont() for drawing so we can not use it for measurement.
                //Textview uses the default char format font for drawing. See bug: EHCN-6U59SN
                TBuf<KMaxTypefaceNameLength> typeface;
                CEikonEnv::Static()->ReadResource(typeface,R_EIK_DEFAULT_CHAR_FORMAT_TYPEFACE);
                TResourceReader reader;
                CEikonEnv::Static()->CreateResourceReaderLC(reader,R_EIK_DEFAULT_CHAR_FORMAT_HEIGHT);
                TInt height =  reader.ReadInt16();
                CleanupStack::PopAndDestroy(); // reader

                TFontSpec fontSpec(typeface, height);
                CEikonEnv::Static()->ScreenDevice()->GetNearestFontInTwips((CFont*&)font, fontSpec);

                AknTextUtils::ClipToFit
                    (
                    ptr,
                    *font,
                    textView->ViewRect().Width() - KRightSpace
                    );
                }
            iEditor->SetTextL( &ptr );
            CleanupStack::PopAndDestroy();  // buf
            }
        else
            {
            if( iSearchPaneMode )
                {
                iEditor->SetTextL( &aText );
                }
            else
                {
                if ( aText.Length() )
                    {
                    iEditor->SetTextL( &aText );
                    }
                else
                    {
                    TBufC<16> buf(KAddressText);
                    HBufC* tmpBuf = NULL;
                    tmpBuf = buf.AllocLC();
                    iEditor->SetTextL( tmpBuf );
                    CleanupStack::PopAndDestroy(); // tmpBuf
                    }
                }
            }

        iEditor->SetCursorPosL( iEditor->TextLength(), EFalse );
        if ( !iEverFocused )
            {
            iEditor->SetFocus( ETrue );
            }

        if ( !iEverFocused )
            {
            iEditor->SetFocus( IsFocused() );
            iEverFocused = ETrue;
            }
            SetEditingL( !aCancelEditing );
            DrawDeferred();
        }

      // release font if it was used
      if (font != NULL)
          {
          CEikonEnv::Static()->ScreenDevice()->ReleaseFont(font);
          }

    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::GetTextL
// ---------------------------------------------------------
//
HBufC* CBrowserBookmarksGotoPane::GetTextL()
    {
    HBufC* text = HBufC::NewL( iEditor->TextLength() + 1 );
    TPtr ptr = text->Des();
    iEditor->GetText( ptr );
    ptr.ZeroTerminate();
    if( !iSearchPaneMode )
        {
        Util::EncodeSpaces(text);
        }

    return text;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::SetObserver
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::SetGPObserver
( MBookmarksGotoPaneObserver* aObserver )
    {
    /*__ASSERT_DEBUG( !iGPObserver, \
        Util::Panic( Util::EGotoPaneObserverAlreadySet ) );*/
    iGPObserver = aObserver;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::SetLineState
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::SetLineState( TBool aLineVisible )
    {
    iInputFrame->SetLineState( aLineVisible );
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::OfferKeyEventL
// ---------------------------------------------------------
//
// Note:
//
//   Below, there are several keys in the ISO-Latin-1 range that we
//   exclude from use in a URL:
//
//     - EStdKeyDevice10 0xC9  E-accent acute         Northwest
//     - EStdKeyDevice11 0xCA  E-accent circumflex    Northeast
//     - EStdKeyDevice12 0xCB  E-umlaut               Southeast
//     - EStdKeyDevice13 0xCC  I-accent grave         Southwest
//
//   While these ought to be vaguely acceptable in certain parts of a URL,
//   they are also used by some navigation drivers as our "diagonal event"
//   codes so turn up as the user navigates the cursor. Because of this,
//   today, we prevent them from being inserted as characters. There's
//   a second set of code ranges (in the 0x8XXX range) that's also defined
//   for these events; if the driver writers ever manage to get all of
//   the navigation drivers using this "out-of-band" range, we can
//   eliminate filtering the ISO-Latin-1 range of characters that we
//   do here.
//
//

TKeyResponse CBrowserBookmarksGotoPane::OfferKeyEventL
( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    TKeyResponse response = EKeyWasNotConsumed;
    TChar iInputChar = TChar( aKeyEvent.iScanCode );


    if ( iBAdaptiveListPopup )
        {
        response = iBAdaptiveListPopup->OfferKeyEventL( aKeyEvent, aType );
        }

    if ( iEditing && response != EKeyWasConsumed)
        {

        switch ( aKeyEvent.iCode )              // Already editing; most keys simply forwarded to the editor.
            {

            case EKeyOK:
                {
                response = EKeyWasConsumed;     // MSK is now handled through HandleCommand in BrowserBookmarksView
                break;
                }

            case EKeyRightUpArrow:              // Northeast
            case EStdKeyDevice11:               //   : Extra KeyEvent supports diagonal event simulator wedge
            case EKeyRightDownArrow:            // Southeast
            case EStdKeyDevice12:               //   : Extra KeyEvent supports diagonal event simulator wedge
            case EKeyLeftDownArrow:             // Southwest
            case EStdKeyDevice13:               //   : Extra KeyEvent supports diagonal event simulator wedge
            case EKeyLeftUpArrow:               // Northwest
            case EStdKeyDevice10:               //   : Extra KeyEvent supports diagonal event simulator wedge
                {                               // Here, "eat" all of the diagonals so they have no effect...
                response = EKeyWasConsumed;
                break;
                }

            case EKeyEnter:                     // Handle EnterKey as "GOTO" for Touch, VKB's enter key
                if ( AknLayoutUtils::PenEnabled() )
                        {
                        CBrowserAppUi::Static()->ActiveView()->HandleCommandL(EWmlCmdGotoPaneGoTo);
                        // In case of search feature, we need to pass EKeyWasConsumed for EKeyEnter
                        if ( iView->ApiProvider().Preferences().SearchFeature() )
                            {
                            response = EKeyWasConsumed;
                            }
                        break;
                        }                       // Else *FALL THROUGH* to default in case Pen is not enabled.
            default:
                {
                response = iEditor->OfferKeyEventL(aKeyEvent, aType );
                break;
                }
            }

         // In the absence of search feature
         // Force key event to be consumed if you're editing
         // regardless of what's happened to it.
         //
         // This forces user to press cancel to continue other tasks
         if ( !iView->ApiProvider().Preferences().SearchFeature() )
             {
             response = EKeyWasConsumed;
             }
        }
    else if ( response != EKeyWasConsumed )     // Key consumed yet? If not, process onwards...
        {

        if ( (aKeyEvent.iScanCode == EStdKeyHash)  ||
                ( aKeyEvent.iModifiers &
                ( EModifierShift | EModifierLeftShift | EModifierRightShift |
                EModifierCtrl | EModifierLeftCtrl | EModifierRightCtrl ) ) )
            {

            // Hash key press will be used for Mark/UnMark functionality
            // Let Platform Listbox handle this.
            response = EKeyWasNotConsumed;
            }

        //
        // Not currently editing the GoTo pane.
        // Catch alpha and numeric to pop up the GoTo pane
        //
        // Only popup the GoTo Pane with a KeyDown event.
        // Otherwise, if browser is pushed to the backround (but still active)
        // in favor of the App Shell, A long "0" press will reactivate the browser and
        // leave a KeyUp to (wrongly) pop the pane up
        //
        // Note that we exclude Unicode 0xC4 and 0xC5 which
        // are defined as EStdkeyYes and EStdkeyNo. These
        // map to the Send and End keys respectively. Normally,
        // 0xC4 and 0xC5 can be considered alpha digits, but we
        // exclude them so that they will not pop the GoTo pane
        //
        // We do the same thing for all of the diagonal motion events
        //   so they have no effect either.
        //
        // Otherwise, key event is generated again (to channel through FEP)
        //
        else if (    !iFrozen
                  && aType == EEventKeyDown
                  && iInputChar.IsAlphaDigit()
                  && iInputChar != EStdKeyYes
                  && iInputChar != EStdKeyNo
                  && iInputChar != EKeyRightUpArrow     // Northeast
                  && iInputChar != EStdKeyDevice11      //   : Extra KeyEvent supports diagonal event simulator wedge
                  && iInputChar != EKeyRightDownArrow   // Southeast
                  && iInputChar != EStdKeyDevice12      //   : Extra KeyEvent supports diagonal event simulator wedge
                  && iInputChar != EKeyLeftDownArrow    // Southwest
                  && iInputChar != EStdKeyDevice13      //   : Extra KeyEvent supports diagonal event simulator wedge
                  && iInputChar != EKeyLeftUpArrow      // Northwest
                  && iInputChar != EStdKeyDevice10 )    //   : Extra KeyEvent supports diagonal event simulator wedge
            {                                           // Most other alphanumeric keys activate the Goto Pane...
            response = EKeyWasConsumed;                 // We'll consume the key event so it ends here
            BeginEditingL();
            iEditor->SetCursorPosL( iEditor->TextLength(), EFalse );
            iCoeEnv->SyncNotifyFocusObserversOfChangeInFocus();
            iCoeEnv->SimulateKeyEventL( aKeyEvent, aType );
            }
        else
            {                                           // Other keys do not activate the Goto Pane.
            response = EKeyWasNotConsumed;              // Let someone else take the key event
            }
        }
    return response;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::CountComponentControls
// ---------------------------------------------------------
//
TInt CBrowserBookmarksGotoPane::CountComponentControls() const
    {
    return 2;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::ComponentControl
// ---------------------------------------------------------
//
CCoeControl* CBrowserBookmarksGotoPane::ComponentControl
( TInt aIndex ) const
    {
    switch ( aIndex )
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
            {
            return NULL;
            }
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::FocusChanged
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::FocusChanged( TDrawNow aDrawNow )
    {
    iEditor->SetFocus( IsFocused(), aDrawNow );
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::CBrowserBookmarksGotoPane
// ---------------------------------------------------------
//
CBrowserBookmarksGotoPane::CBrowserBookmarksGotoPane(CBrowserFavouritesView* aView, TBool aSearchPaneMode)
:   iView( aView ),
    iEditing( EFalse ),
    iFrozen( EFalse ),
    iBAdaptiveListPopup( NULL ),
    iSearchPaneMode( aSearchPaneMode )
    {
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::ConstructL
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::ConstructL ( const CCoeControl& aParent, const TDesC& aBitmapfile,
        TInt aIconBitmapId, TInt aIconMaskId, TBool aPopupListStatus )

    {
    SetContainerWindowL( aParent ); // This is now non-window owning control
    SetMopParent(const_cast<CCoeControl*>(&aParent));
    iEditor = new (ELeave) CEikEdwin;
    iInputFrame = CAknInputFrame::NewL( iEditor, EFalse, aBitmapfile,
                                    aIconBitmapId, aIconMaskId, 0 );
    iInputFrame->SetContainerWindowL( *this );
    iEditor->SetContainerWindowL( *this );
    AknEditUtils::ConstructEditingL(iEditor, KFavouritesMaxUrlGotoPaneDefine, 1, EAknEditorCharactersLowerCase, EAknEditorAlignRight, EFalse, ETrue, EFalse);
    iEditor->AddFlagToUserFlags( CEikEdwin::EAlwaysShowSelection );
    iEditor->SetBorder( TGulBorder::ENone );
    iEditor->SetAknEditorCase( EAknEditorLowerCase );
    iEditor->SetAknEditorInputMode( EAknEditorTextInputMode );
    iEditor->SetSkinBackgroundControlContextL(NULL);
    if (AVKONAPAC)
        {
        // Disallow chinese input.
        iEditor->SetAknEditorAllowedInputModes( EAknEditorTextInputMode | EAknEditorNumericInputMode );
        }

    // In Search Mode we allow all types of inputs
    TInt editorFlags( (iSearchPaneMode ? EAknEditorFlagDefault : EAknEditorFlagLatinInputModesOnly) | EAknEditorFlagUseSCTNumericCharmap );
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


    iEditor->SetAknEditorPermittedCaseModes( EAknEditorUpperCase | EAknEditorLowerCase );

    iEditor->CreateTextViewL();
    SetupSkinContextL();
    iEditing = EFalse;
    TBool searchFeature = iView->ApiProvider().Preferences().SearchFeature();
    if ( searchFeature )
        {
        iDefaultSearchText = StringLoader::LoadL( R_IS_WEB_SEARCH );
        }

    //adaptive popuplist
    iBAdaptiveListPopup = NULL;
    if( aPopupListStatus && AUTOCOMP ) //ask the feature manager
        {
        iBAdaptiveListPopup= new (ELeave) CBrowserAdaptiveListPopup(
            iEditor, this, EBookmarksGotoPane, searchFeature);
        iBAdaptiveListPopup->ConstructL();
        iEditor->SetObserver( iBAdaptiveListPopup );
        if (iView)
            {
            iBAdaptiveListPopup->SetUrlSuffixList(iView->ApiProvider().Preferences().URLSuffixList());
            iBAdaptiveListPopup->SetMaxRecentUrls(iView->ApiProvider().Preferences().MaxRecentUrls());
            }
        }

    ActivateL();
    MakeVisible( EFalse );
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::SizeChanged
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::SizeChanged()
    {
    TAknLayoutRect layoutRectTemp;
    TAknWindowLineLayout windowLayoutTmp, windowLayoutTmp2;

    windowLayoutTmp = AknLayout::Find_pane_elements_Line_4();
    windowLayoutTmp2 = windowLayoutTmp;
    layoutRectTemp.LayoutRect( Rect(), windowLayoutTmp2 );
    iEditor->SetRect( layoutRectTemp.Rect() );
    iInputFrame->SetRect( Rect() );
    TRAP_IGNORE(SetupSkinContextL());
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::SetEditingL
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::SetEditingL( TBool aEditing )
    {
    if ( aEditing != iEditing )
        {
        iEditing = aEditing;
        SetFocus( iEditing );
        if ( iGPObserver )
            {
            iGPObserver->HandleBookmarksGotoPaneEventL
                (
                this,
                MBookmarksGotoPaneObserver::EEventEditingModeChanged
                );
            }
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::PopupList
// ---------------------------------------------------------
//
CBrowserAdaptiveListPopup* CBrowserBookmarksGotoPane::PopupList()
    {
    return iBAdaptiveListPopup;
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::HandleResourceChange
// ---------------------------------------------------------
//
void CBrowserBookmarksGotoPane::HandleResourceChange( TInt aType )
    {
    CCoeControl::HandleResourceChange( aType );

    if ( iBAdaptiveListPopup )
        {
        iBAdaptiveListPopup->HandleResourceChange( aType );
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksGotoPane::MopSupplyObject
// ---------------------------------------------------------
//
TTypeUid::Ptr CBrowserBookmarksGotoPane::MopSupplyObject( TTypeUid aId )
    {
    return MAknsControlContext::SupplyMopObject( aId, iSkinContext );
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksGotoPane::Editor
// ----------------------------------------------------------------------------
CEikEdwin* CBrowserBookmarksGotoPane::Editor() const
    {
    return iEditor;
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksGotoPane::HandlePointerEventL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksGotoPane::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
    if (AknLayoutUtils::PenEnabled())
        {
        if(iEditor && Rect().Contains(aPointerEvent.iPosition))
            {
            iEditor->HandlePointerEventL(aPointerEvent);
            if ( !iEditing )
                {
                SetEditingL( ETrue );
                iCoeEnv->SyncNotifyFocusObserversOfChangeInFocus();
                CBrowserAppUi::Static()->UpdateCbaL();
                }
            }
        else
            {
            // pointer outside of control
            CBrowserAppUi::Static()->ActiveView()->HandleCommandL(EWmlCmdGotoPaneCancel);
            }
        }
    }
// ----------------------------------------------------------------------------
// CBrowserBookmarksGotoPane::SetupSkinContextL
// ----------------------------------------------------------------------------
void CBrowserBookmarksGotoPane::SetupSkinContextL()
    {
    TAknsItemID tileIID = KAknsIIDQsnBgColumnA;

    TAknWindowLineLayout tile =
        Column_background_and_list_slice_skin_placing_Line_2();

    TAknLayoutRect mainPane;
    mainPane.LayoutRect(
        iAvkonAppUi->ApplicationRect(),
        AKN_LAYOUT_WINDOW_main_pane( iAvkonAppUi->ApplicationRect(),
                                     0, 0, 1) );
    TAknLayoutRect listRect;
    listRect.LayoutRect( mainPane.Rect(), AknLayout::list_gen_pane(0) );

    TAknLayoutRect tileRect;
    tileRect.LayoutRect(mainPane.Rect(), tile);

    if ( !iSkinContext )
        {
        iSkinContext = CAknsListBoxBackgroundControlContext::NewL(
            KAknsIIDQsnBgAreaMainListGene,
            listRect.Rect(),
            EUseSkinContext & EParentAbsolute,
            tileIID,
            tileRect.Rect() );
        }
    else
        {
        iSkinContext->SetRect( listRect.Rect() );
        }


    if ( !iInputContext )
        {
        iInputContext = CAknsFrameBackgroundControlContext::NewL(
            KAknsIIDQsnFrInput, TRect(0,0,0,0),TRect(0,0,0,0),EFalse );
        }

    iInputFrame->SetInputContext(iInputContext);
    iEditor->SetSkinBackgroundControlContextL(iInputContext);

    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksGotoPane::ActivateVkbL
// ----------------------------------------------------------------------------
void CBrowserBookmarksGotoPane::ActivateVkbL()
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
        iEditor->HandlePointerEventL(pe);

        // VKB will only activate if nothing selected
        iEditor->SetSelectionL(pos, pos);

        pe.iType = TPointerEvent::EButton1Up;
        iEditor->HandlePointerEventL(pe);
        iEditor->SetCursorPosL( iEditor->TextLength(), EFalse );

        }
    }


// ----------------------------------------------------------------------------
// CBrowserBookmarksGotoPane::SetTextModeItalic
// ----------------------------------------------------------------------------
void CBrowserBookmarksGotoPane::SetTextModeItalicL( )
    {

    // Editor Control is laid in a scalable way, so we need to get the correct font
    // specification for setting CharFormatLayer, We could have used GetNearestFontInTwips,
    // as done above in SetTextL() but it does not provide correct fonts for editor.
    // We do not need to set the FontPosture back to EPostureUpright ( Normal ), as it
    // is automatically handled by AknLayoutUtils::LayoutEdwinScalable called by
    // iInputFrame->SetRect(), which overwrites all the properties for Editor.

    TAknTextComponentLayout   editorLayout;
    TBool apac( AknLayoutUtils::Variant() == EApacVariant && ( CAknInputFrame::EShowIndicators ) );
    editorLayout = AknLayoutScalable_Avkon::input_find_pane_t1( apac ? 1: 0 );
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
    CCharFormatLayer* charFormatLayer = NULL;
    charFormatLayer = CCharFormatLayer::NewL(charFormat,charFormatMask);
    iEditor->SetCharFormatLayer(charFormatLayer);
   }

// ----------------------------------------------------------------------------
// CBrowserBookmarksGotoPane::SetVKBFlag
// ----------------------------------------------------------------------------
void CBrowserBookmarksGotoPane::SetVKBFlag( TBool aVKBFlag )
    {
    if( aVKBFlag )
        {
        iEditor->RemoveFlagFromUserFlags( CEikEdwin::EAvkonDisableVKB );
        }
    else
        {
        iEditor->AddFlagToUserFlags( CEikEdwin::EAvkonDisableVKB );
        }
    }
// End of File
