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
*/

#include "BrowserContentViewContainer.h"
#include "BrowserAppUi.h"
#include "BrowserAppViewBase.h"
#include "BrowserGotoPane.h"
#include "CommonConstants.h"
#include "BrowserContentView.h"
#include "Display.h"
#include "Preferences.h"
#include "BrowserUtil.h"

#include "BrowserPreferences.h"
#include "BrowserWindow.h"
#include "BrowserWindowManager.h"
#include <FeatMgr.h>

#include <eikrted.h>
#include <eikbctrl.h>
#include <Eikmenub.h>

#include <aknutils.h>

#include "BrowserUIVariant.hrh"

#ifdef __SERIES60_HELP
// Context-Sensitve Help File
#include "BrowserApplication.h"
#include <csxhelp/ope.hlp.hrh>
#include <csxhelp/browser.hlp.hrh>
#endif // __SERIES60_HELP

#include "eikon.hrh"


//Constant
// The interval zoom tooltip is visible, it will disappear after KZoomLevelShowTime seconds
const TInt KZoomLevelShowTime( 2 * 1000 ); // 2 seconds

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::NewL()
// ---------------------------------------------------------------------------
//
CBrowserContentViewContainer*
CBrowserContentViewContainer::NewL(CBrowserContentView* aView,
                                   MApiProvider& aApiProvider )
    {
    CBrowserContentViewContainer* container =
        new (ELeave) CBrowserContentViewContainer( aView, aApiProvider );
    CleanupStack::PushL( container );
    container->ConstructL();
    CleanupStack::Pop(); // container

    return container;
    }

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::CBrowserContentViewContainer()
// ---------------------------------------------------------------------------
//
CBrowserContentViewContainer::
CBrowserContentViewContainer(CBrowserContentView* aView,
                             MApiProvider& aApiProvider ) :
    iView( aView ),
    iApiProvider( aApiProvider )
    {
    }

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::~CBrowserContentViewContainer()
// ---------------------------------------------------------------------------
//
CBrowserContentViewContainer::~CBrowserContentViewContainer()
    {
    delete iGotoPane;
    delete iFindKeywordPane;
    iView = NULL; // Not owned

    iShortCutFuncMap.Close();
    }

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::ConstructL()
// ---------------------------------------------------------------------------
//
void CBrowserContentViewContainer::ConstructL()
    {
    CreateWindowL();
    SetAllowStrayPointers();
    SetMopParent( iView );

    iGotoPane = CBrowserGotoPane::NewL( this,
                                        EMbmAvkonQgn_indi_find_goto,
                                        EMbmAvkonQgn_indi_find_goto_mask,
                                        GOTOPANE_POPUPLIST_ENABLE,
                                        iView );

    // Create the find pane with magnifier glass icon, and
    // without adaptive popup list...
    iFindKeywordPane = CBrowserGotoPane::NewL( this,
                                               EMbmAvkonQgn_indi_find_glass,
                                               EMbmAvkonQgn_indi_find_glass_mask,
                                               GOTOPANE_POPUPLIST_DISABLE,
                                               iView,
                                               ETrue );
    }

// -----------------------------------------------------------------------------
// CBrowserContentViewContainer::HandlePointerEventL
// Handles pointer events
// -----------------------------------------------------------------------------
void CBrowserContentViewContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
     switch ( aPointerEvent.iType )
        {
        case TPointerEvent::EButton1Down:
            {
            // Do nothing
            break;
            }
        case TPointerEvent::EDrag:
            {
            // Do nothing
            break;
            }
        case TPointerEvent::EButton1Up:
            {
            // Close the zoom silder when the user selects anywhere in the
            // BrowserContainerView
            if ( iView->ZoomSliderVisible() )
                {
                iView->MakeZoomSliderVisibleL( EFalse );
                }
            break;
            }
        default:
            {
            break;
            }
        }   // end of switch

    // Must pass the pointer event to the CoeControl
    CCoeControl::HandlePointerEventL( aPointerEvent );
    }

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::OfferKeyEventL()
// ---------------------------------------------------------------------------
//
TKeyResponse CBrowserContentViewContainer::OfferKeyEventL(const TKeyEvent& aKeyEvent,TEventCode aType)
    {
    CBrowserAppUi* ui = CBrowserAppUi::Static();
    TKeyResponse result( EKeyWasNotConsumed );

    if (ui->OfferApplicationSpecificKeyEventL(aKeyEvent, aType) == EKeyWasConsumed)
        {
        return EKeyWasConsumed;
        }

    // For Short Cut
    if (!iShortCutFuncsReady)
    {
        CreateShortCutFuncsHashTable();
        iShortCutFuncsReady = ETrue;
    }

    // Turn off some controls, if we receive events
    if ( aType == EEventKey )
        {
        // Turn off the keymap if on
        if ( iView->KeymapIsUp() )
            {
            iView->HandleCommandL( EWmlCmdHideShortcutKeymap );
            }

        // Turn off the zoom slider if on
        if ( iView->ZoomSliderVisible() )
            {
            iView->MakeZoomSliderVisibleL( EFalse );
            }
        }

    // If goto pane is visible, offer key events to it
    if ( iGotoPane->IsVisible() )
        {
        return iGotoPane->OfferKeyEventL( aKeyEvent, aType );
        }

    TKeyEvent keyEvent( aKeyEvent );

    // Don't allow virtual keyboard backspace key event to close the window
    if ( !AknLayoutUtils::PenEnabled() && aType == EEventKey
        && keyEvent.iCode == EKeyBackspace )
        {
        if ( iApiProvider.Preferences().UiLocalFeatureSupported(
                                                    KBrowserMultipleWindows ) )
            {
            TInt winCount = iApiProvider.WindowMgr().WindowCount();
            if ( ( winCount > 1 ) && ( !iView->IsMiniatureViewUp() ) )
                {
                ui->CloseWindowL();
                result = EKeyWasConsumed;
                }
            }
        }

    // Handle zooming events
    // 1. RemConInterface will translate dedicated HW key press to OfferKeyEventL().
    // See eikon.hrh for mapping.
    // 2. When the browser is in Zoom Mode (slider or tooltip displayed), the
    // navi-keys will zoom.
    if ( aType == EEventKey )
        {
        // Handle dedicated HW key zoom-in: if not in zoom mode, HW zoom key takes
        // zooming immediately without going to zoom mode
        // if it's already in zoom mode, then HW zoom key acts the same as up
        // and down navigation key mode
        // Zoom mode can display tooltip (current) or slider (future)
        if ( iView->isZoomMode() )
            {
            }
        else if (!iView->IsHistoryViewUp() && !iView->IsMiniatureViewUp())
            {
            // Not in zoom mode, or history view, or page overview, and HW zoom key
            // pressed -  zoom immediately without entering zoom mode.
            if ( keyEvent.iCode == EKeyZoomIn )
                {
                iApiProvider.BrCtlInterface().HandleCommandL(
                        (TInt)TBrCtlDefs::ECommandZoomIn +
                        (TInt)TBrCtlDefs::ECommandIdBase );
                iView->ZoomImagesInL(KZoomLevelShowTime);
                result = EKeyWasConsumed;
                }
            // Handle dedicated HW key zoom-out
            else if ( keyEvent.iCode == EKeyZoomOut )
                {
                iApiProvider.BrCtlInterface().HandleCommandL(
                        (TInt)TBrCtlDefs::ECommandZoomOut +
                        (TInt)TBrCtlDefs::ECommandIdBase );
                iView->ZoomImagesOutL(KZoomLevelShowTime);
                result = EKeyWasConsumed;
                }
            }

        }   // End of handling key events for zooming

    // convert Enter key to KeyOk, this is only for emulator
    if ( aType == EEventKey &&  keyEvent.iCode == EKeyEnter )
        {
        keyEvent.iCode = EKeyOK;
        }

    // Web Engine opens the highlighted link into a new window. (long press)
    // Currently the option is disabled and the link is opened in the same window
    // The commented part below enables the "Open link in new window" option on long press.
    // UI change request AHUN-6U3NT4, S60 bug AHUN-6UYT6N

    if ( aType == EEventKey && keyEvent.iCode == EKeyOK )
        {
        if ( keyEvent.iRepeats && iSelectionKeyPressed )
            {
            TBrCtlDefs::TBrCtlElementType elementtype =
                iApiProvider.BrCtlInterface().FocusedElementType();
            TInt command( KErrNotFound );
            switch( elementtype )
                {
                case TBrCtlDefs::EElementAnchor:
                    {
    //                command = TBrCtlDefs::ECommandOpenNewWindow;
                      command = TBrCtlDefs::ECommandOpen;
                    break;
                    }
                default: break;
                }
    //        if ( command == TBrCtlDefs::ECommandOpenNewWindow &&
    //          iApiProvider.Preferences().UiLocalFeatureSupported( KBrowserMultipleWindows ) &&
    //          !iApiProvider.Preferences().UiLocalFeatureSupported( KBrowserMinimalMultipleWindows ))
              if ( command == TBrCtlDefs::ECommandOpen)
                {
                iSelectionKeyPressed = EFalse;
                keyEvent.iCode = EKeyNull;
                result = EKeyWasConsumed;
                iApiProvider.BrCtlInterface().HandleCommandL(
                    command + TBrCtlDefs::ECommandIdBase );
                }
            }
        }

    // stop the event handling when find item was actived
    if ( (keyEvent.iScanCode == EStdKeyDevice3 || keyEvent.iScanCode == EStdKeyXXX)
        && aType == EEventKeyDown && !iView->FindItemIsInProgress() )
        {
        iSelectionKeyPressed = ETrue;
        keyEvent.iCode = EKeyNull;
        }

    // Ignore key event in zoom mode
    if ( result == EKeyWasNotConsumed && !iView->isZoomMode())
        {
        TRAP_IGNORE(result = iApiProvider.BrCtlInterface().OfferKeyEventL(keyEvent, aType));
        }

    /**
    * When checking long or short key presses, iScanCode must be used, because
    * when EEventKeyUp/EEventKeyUp are handled, the iCode is not available.
    * iIsKeyLongPressed must be checked in short key presses because, when the
    * button is released short key code will be called again. And that time it
    * must be ignored.
    */

    /**
    // This key is inactive, only for testing short / long key presses.
    if ( keyEvent.iScanCode == 48 && keyEvent.iRepeats && !iIsKeyLongPressed)
        {
        iEikonEnv->InfoMsg(_L("0 Long"));
        iIsKeyLongPressed = ETrue;
        keyEvent.iCode = EKeyNull;
        result = EKeyWasConsumed;
        }
    if ( aType == EEventKeyUp && keyEvent.iScanCode == 48 && !keyEvent.iRepeats)
        {
        if ( !iIsKeyLongPressed )
            {
            iEikonEnv->InfoMsg(_L("0 Short"));
            keyEvent.iCode = EKeyNull;
            result = EKeyWasConsumed;
            }
        iIsKeyLongPressed = EFalse;
        }
    **/

    /*
    * Not processing short keys, if the History view, or the
    * Miniature View ( old name: Thumbnail View ) is up,
    * except short key: '8'.
    */
    if (!iApiProvider.Preferences().AccessKeys())
    {
        // Key short press, activate function based on keyEvent.iCode
        if ( aType == EEventKey && result == EKeyWasNotConsumed )
            {
            if (!AknLayoutUtils::PenEnabled())
                {
                if (iView->isZoomMode())
                    {
                    result = EKeyWasConsumed;
                    }
                else
                    {
                    if (iApiProvider.Preferences().ShortcutKeysForQwerty())
                        {
                        result = InvokeFunction ( keyEvent.iCode );
                        }
                    else
                        {
                        if ( !iView->IsMiniatureViewUp() && !iView->IsHistoryViewUp() )
                            {
                            TInt function = -1;
                            switch(keyEvent.iCode)
                                {
                                case '0':
                                    {
                                    function = ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey0Cmd()][KShortcutsCommandDecodeMatrixCmdVal];
                                    break;
                                    }
                                case '1':
                                    {
                                    function = ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey1Cmd()][KShortcutsCommandDecodeMatrixCmdVal];
                                    break;
                                    }
                                case '2':
                                    {
                                    function = ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey2Cmd()][KShortcutsCommandDecodeMatrixCmdVal];
                                    break;
                                    }
                                case '3':
                                    {
                                    function = ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey3Cmd()][KShortcutsCommandDecodeMatrixCmdVal];
                                    break;
                                    }
                                case '4':
                                    {
                                    function = ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey4Cmd()][KShortcutsCommandDecodeMatrixCmdVal];
                                    break;
                                    }
                                case '5':
                                    {
                                    function = ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey5Cmd()][KShortcutsCommandDecodeMatrixCmdVal];
                                    break;
                                    }
                                case '6':
                                    {
                                    function = ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey6Cmd()][KShortcutsCommandDecodeMatrixCmdVal];
                                    break;
                                    }
                                case '7':
                                    {
                                    function = ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey7Cmd()][KShortcutsCommandDecodeMatrixCmdVal];
                                    break;
                                    }
                                case '8':
                                    {
                                    function = ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey8Cmd()][KShortcutsCommandDecodeMatrixCmdVal];
                                    break;
                                    }
                                case '9':
                                    {
                                    function = ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKey9Cmd()][KShortcutsCommandDecodeMatrixCmdVal];
                                    break;
                                    }
                                case '*':
                                    {
                                    function = ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKeyStarCmd()][KShortcutsCommandDecodeMatrixCmdVal];
                                    break;
                                    }
                                case '#':
                                    {
                                    function = ShortcutsCommandDecodeMatrix[iApiProvider.Preferences().ShortcutKeyHashCmd()][KShortcutsCommandDecodeMatrixCmdVal];
                                    break;
                                    }
                                default:
                                    {
                                    break;
                                    }
                                }

                            if (function != -1)
                                {
                                iView->HandleCommandL(function);
                                result = EKeyWasConsumed;
                                }
                            }
                        } //endof handle shortcut keys

                    if (result == EKeyWasConsumed)
                        {
                            keyEvent.iCode = EKeyNull;
                        }
                    }
                }
            }

            if ( iView->IsHistoryViewUp() && result == EKeyWasConsumed )
                {
                if (    ( aType == EEventKey || aType == EEventKeyUp )
                     && (    keyEvent.iCode == EKeyLeftUpArrow       // Northwest
                          || keyEvent.iCode == EStdKeyDevice10       //   : Extra KeyEvent supports diagonal event simulator wedge
                          || keyEvent.iCode == EKeyLeftArrow         // West
                          || keyEvent.iCode == EKeyLeftDownArrow     // Southwest
                          || keyEvent.iCode == EStdKeyDevice13       //   : Extra KeyEvent supports diagonal event simulator wedge

                          || keyEvent.iCode == EKeyRightUpArrow      // Northeast
                          || keyEvent.iCode == EStdKeyDevice11       //   : Extra KeyEvent supports diagonal event simulator wedge
                          || keyEvent.iCode == EKeyRightArrow        // East
                          || keyEvent.iCode == EKeyRightDownArrow    // Southeast
                          || keyEvent.iCode == EStdKeyDevice12 ) )   //   : Extra KeyEvent supports diagonal event simulator wedge
                    {
                    iView->UpdateTitleL( iApiProvider );
                    }
                }

            // If in zoom mode, then show the zoom tooltip
            if (iView->isZoomMode() && result == EKeyWasConsumed )
                {
                iView->SetZoomLevelTitleTextL(R_BROWSER_ZOOM_LEVEL);
                }

        }   // if (!iApiProvider.Preferences().AccessKeys())

    if ( result == EKeyWasNotConsumed &&
         (keyEvent.iScanCode == EStdKeyDevice3 || keyEvent.iScanCode == EStdKeyXXX) &&
         aType == EEventKeyUp  &&
         iSelectionKeyPressed )
        {
        // Selection key was released and it is not a long press!
        iSelectionKeyPressed = EFalse;
        aType = EEventKey;
        keyEvent.iCode = EKeyOK;

        // first offer event to browser control, e.g. link activation
        // result = iApiProvider.BrCtlInterface().OfferKeyEventL(keyEvent, aType);

        iView->ResetPreviousViewFlag();
        }
    return result;
    }

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::FocusChanged()
// ---------------------------------------------------------------------------
//
void CBrowserContentViewContainer::FocusChanged(TDrawNow aDrawNow)
    {
    // do not proceed further if a browser app exit is in progress
    if ( iApiProvider.ExitInProgress() )
        {
        return;
        }

    if ( iApiProvider.StartedUp() )
        iApiProvider.BrCtlInterface().SetFocus(IsFocused());
    if ( iGotoPane->IsVisible() )
        {
        TRAP_IGNORE(iGotoPane->EnableKeyEventHandlerL( IsFocused() ));
        }

    CCoeControl::FocusChanged( aDrawNow );
    }

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::SizeChanged()
// ---------------------------------------------------------------------------
//
void CBrowserContentViewContainer::SizeChanged()
    {
    // BrCtl could be uninitialized
    if ( iApiProvider.StartedUp() )
        {
        if(iApiProvider.LastActiveViewId() != KUidBrowserFeedsFeedViewId)
            {
            const TRect& oldRect = iApiProvider.BrCtlInterface().Rect();
            iApiProvider.BrCtlInterface().SetRect( TRect( oldRect.iTl, Size() ) );
            }
        }

    if ( iFindKeywordPane->IsFocused() )
        {
        iFindKeywordPane->HandleFindSizeChanged();
        }
    else
        {
        iGotoPane->HandleFindSizeChanged();
        }

    }

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::CountComponentControls()
// ---------------------------------------------------------------------------
//
TInt CBrowserContentViewContainer::CountComponentControls() const
    {
    TInt ctrls = 0;
    ctrls++;        // iGotoPane
    if ( iApiProvider.StartedUp() )
        {
        ctrls++;
        }
    ctrls++;        // iFindKeywordPane
    return ctrls;
    }

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::ComponentControl()
// ---------------------------------------------------------------------------
//
CCoeControl* CBrowserContentViewContainer::ComponentControl( TInt aIndex ) const
    {
    CCoeControl *ctrl = NULL;
    switch( aIndex )
        {
        case 0:
            {
            ctrl = iGotoPane;
            break;
            }
        case 1:
            {
            if ( iApiProvider.StartedUp() )
                ctrl = &(iApiProvider.BrCtlInterface());
            break;
            }
        case 2:
            {
            ctrl = iFindKeywordPane;
            break;
            }

        default:
            break;
        }
    return ctrl;
    }

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::ShutDownGotoURLEditorL()
// ---------------------------------------------------------------------------
//
void CBrowserContentViewContainer::ShutDownGotoURLEditorL()
    {
    iGotoPane->MakeVisible( EFalse );
    iGotoPane->SetFocus( EFalse );
    SetFocus( ETrue );
    }


//---------------------------------------------------------------------------
// CBrowserContentViewContainer::ShutDownFindKeywordEditorL()
// ---------------------------------------------------------------------------
//
void CBrowserContentViewContainer::ShutDownFindKeywordEditorL()
    {
    iFindKeywordPane->MakeVisible( EFalse );
    iFindKeywordPane->SetFocus( EFalse );
    SetFocus( ETrue );
    }


//---------------------------------------------------------------------------
// CBrowserContentViewContainer::HandleResourceChange()
// ---------------------------------------------------------------------------
//
void CBrowserContentViewContainer::HandleResourceChange(TInt aType)
    {
    CCoeControl::HandleResourceChange( aType );
    if ( aType == KEikDynamicLayoutVariantSwitch)
        {
        if (!iView->FindItemIsInProgress())
            {
            iView->UpdateFullScreenL();
            iApiProvider.Display().RestoreTitleL();
            }

        SetRect(iView->ClientRect());
        DrawDeferred();

        // For Touch only-If find pane open, force toolbar to show since avkon disables
        // touch toolbar whenever a dialog is open and a rotate event is handled.
        // ** This workaround was provided by Avkon toolbar group
        if ( AknLayoutUtils::PenEnabled() &&
             iFindKeywordPane->IsVisible() &&
             iView->Toolbar() )
            {
            iView->Toolbar()->DrawableWindow()->SetOrdinalPosition( 0 );
            }
        }
    }


//---------------------------------------------------------------------------
// CBrowserContentViewContainer::CreateShortCutFuncsHashTable()
// ---------------------------------------------------------------------------
//
void CBrowserContentViewContainer::CreateShortCutFuncsHashTable()
    {
    InsertFuncToHashTable(iApiProvider.Preferences().ShortCutFuncHomePg(), EWmlSettingsShortCutsActivateHomepage);
    InsertFuncToHashTable(iApiProvider.Preferences().ShortCutFuncBkMark(), EWmlSettingsShortCutsActivateBkmkview);
    InsertFuncToHashTable(iApiProvider.Preferences().ShortCutFuncFindKeyWord(), EWmlSettingsShortCutsShowFindKeyword);
    InsertFuncToHashTable(iApiProvider.Preferences().ShortCutFuncPrePage(), EWmlSettingsShortCutsGotoPreviousPage);
    InsertFuncToHashTable(iApiProvider.Preferences().ShortCutFuncSwitchWin(), EWmlSettingsShortCutsShowSwitchWindowList);
    InsertFuncToHashTable(iApiProvider.Preferences().ShortCutFuncMiniImage(), EWmlSettingsShortCutsShowMiniature);
    InsertFuncToHashTable(iApiProvider.Preferences().ShortCutFuncFullScreen(), EWmlSettingsShortCutsShowFullScreen);
    InsertFuncToHashTable(iApiProvider.Preferences().ShortCutFuncGoAddr(), EWmlSettingsShortCutsGotoPane);
    InsertFuncToHashTable(iApiProvider.Preferences().ShortCutFuncZoomIn(), EWmlSettingsShortCutsZoomIn);
    InsertFuncToHashTable(iApiProvider.Preferences().ShortCutFuncZoomOut(), EWmlSettingsShortCutsZoomOut);
    // InsertFuncToHashTable(iApiProvider.Preferences().ShortCutFuncZoomMode(), EWmlSettingsShortCutsZoomMode);
    }

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::InsertFuncToHashTable()
// ---------------------------------------------------------------------------
//
void CBrowserContentViewContainer::InsertFuncToHashTable(HBufC* aKeyStr, TInt aFunc)
    {

    __ASSERT_DEBUG( (aKeyStr != NULL), Util::Panic( Util::EUninitializedData ));

        if (aKeyStr->Length() == 0)
        {
            return;
        }

        TBuf<4> key1Str; //  Ex.: 0075
        TBuf<4> key2Str; //  Ex.: 0032

        // Get the position of the separator ',' character from '0075, 0032'.
        TInt pos = aKeyStr->LocateF( ',' );
        if ( ( pos != KErrNotFound ) )
        {    //we have two string
            // Extract the first unicode string
            key1Str.Copy( aKeyStr->Des().Left( pos ) );
            TUint key1 = MyAtoi(key1Str);

            // Extract the second unicode string; second "-1" below is for remove space
            key2Str.Copy( aKeyStr->Des().Right( aKeyStr->Des().Length() - pos - 1 -1) );
            TUint key2 = MyAtoi(key2Str);

            // insert key and function
            iShortCutFuncMap.Insert(key1, aFunc);
            iShortCutFuncMap.Insert(key2, aFunc);
        }
        else
        {    //we only have one string
            // Extract the first unicode string
            key1Str.Copy( aKeyStr->Des() );
            TUint key1 = MyAtoi(key1Str);
            // insert key and function
            iShortCutFuncMap.Insert(key1, aFunc);
        }
    }

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::MyAtoi()
// ---------------------------------------------------------------------------
//
TUint CBrowserContentViewContainer::MyAtoi(TPtrC aData)
    {
        TLex lex(aData);
        TUint dataValue = 0;
        lex.Val(dataValue, EHex);
        return dataValue;
    }

//---------------------------------------------------------------------------
// CBrowserContentViewContainer::InvokeFunction()
// ---------------------------------------------------------------------------
//
TKeyResponse CBrowserContentViewContainer::InvokeFunction(TUint aCode)
    {
    TInt function = -1;
    // if error, no function defined for this shortcut key
    TRAPD( err, function = iShortCutFuncMap.FindL(aCode) );
    if ( err != KErrNone )
        {
            return EKeyWasNotConsumed;
        }

    TBool embedded = iApiProvider.IsEmbeddedModeOn();

    if ( !iView->IsMiniatureViewUp() && !iView->IsHistoryViewUp() )
    {
        CBrowserAppUi* ui = CBrowserAppUi::Static();

        switch(function)
            {
            case EWmlSettingsShortCutsActivateHomepage:
                {
                TBool homepage = iApiProvider.Preferences().UiLocalFeatureSupported( KBrowserUiHomePageSetting );
                // home page
                if ( homepage )
                    {
                    //
                    TWmlSettingsHomePage pgtype = iApiProvider.Preferences().HomePageType();

                    if ( !(embedded || pgtype == EWmlSettingsHomePageBookmarks) )
                        {
                        ui->LaunchHomePageL();
                        }
                    else
                        {
                        if ( pgtype == EWmlSettingsHomePageBookmarks )
                            {
                            iApiProvider.SetViewToBeActivatedIfNeededL( KUidBrowserBookmarksViewId );
                            }
                        }
                    }
                return EKeyWasConsumed;
                }
            case EWmlSettingsShortCutsActivateBkmkview:
                {
                if ( !embedded )
                {
                    iApiProvider.SetViewToBeActivatedIfNeededL( KUidBrowserBookmarksViewId );
                }
                return EKeyWasConsumed;
                }
            case EWmlSettingsShortCutsGotoPane:
                {
                iView->LaunchGotoAddressEditorL();
                iView->MenuBar()->MenuPane()->MakeVisible(EFalse);
                return EKeyWasConsumed;
                }
            case EWmlSettingsShortCutsShowFindKeyword:
                {
                if (!iView->ApiProvider().WindowMgr().CurrentWindow()->HasWMLContent(ETrue /* is current page wml?*/))
                    {
                    iView->LaunchFindKeywordEditorL();
                    iView->MenuBar()->MenuPane()->MakeVisible(EFalse);
                    }
                return EKeyWasConsumed;
                }
            case EWmlSettingsShortCutsShowSwitchWindowList:
                {
                if (!iView->ApiProvider().WindowMgr().CurrentWindow()->HasWMLContent(EFalse /*is any page wml?*/))
                    {
                    if (iApiProvider.Preferences().UiLocalFeatureSupported( KBrowserMultipleWindows ) &&
                        !iApiProvider.Preferences().UiLocalFeatureSupported( KBrowserMinimalMultipleWindows ) &&
                        iApiProvider.WindowMgr().WindowCount() > 1)
                        {
                        iApiProvider.SetViewToBeActivatedIfNeededL( KUidBrowserWindowSelectionViewId );
                        }
                    else
                        {
                        if (iApiProvider.WindowMgr().WindowCount() > 1)
                            {
                            ui->SwitchWindowL();
                            }
                        }
                    }
                return EKeyWasConsumed;
                }

            case EWmlSettingsShortCutsZoomOut:
                {
                iView->ZoomImagesOutL();
                return EKeyWasConsumed;
                }

            case EWmlSettingsShortCutsZoomIn:
                {
                iView->ZoomImagesInL();
                return EKeyWasConsumed;
                }

            case EWmlSettingsShortCutsGotoPreviousPage:
                {
                if ( iApiProvider.BrCtlInterface().NavigationAvailable( TBrCtlDefs::ENavigationBack ) )
                    {
                        iApiProvider.LoadObserver().DoStartLoad( CBrowserLoadObserver::ELoadUrlTypeOther );
                        iApiProvider.BrCtlInterface().HandleCommandL( (TInt)TBrCtlDefs::ECommandOneStepBack +
                                                            (TInt)TBrCtlDefs::ECommandIdBase );
                    }
                return EKeyWasConsumed;
                }

            case EWmlSettingsShortCutsShowFullScreen:
                {
                iView->HandleCommandL(EWmlCmdEnterFullScreenBrowsing);
                return EKeyWasConsumed;
                }
          }
    } // END if ( !iView->IsMiniatureViewUp() && !iView->IsHistoryViewUp() && !iView->isZoomMode())


    if ( function == EWmlSettingsShortCutsShowMiniature && !iView->IsHistoryViewUp() )
        {   // Process short key '8' only if History view is not shown.
            if (!iView->ApiProvider().WindowMgr().CurrentWindow()->HasWMLContent(ETrue /* is current page wml?*/))
                {
                iApiProvider.BrCtlInterface().HandleCommandL(
                        (TInt)TBrCtlDefs::ECommandShowThumbnailView + (TInt)TBrCtlDefs::ECommandIdBase );
                }
            return EKeyWasConsumed;
        }

    return EKeyWasNotConsumed;
   }



#ifdef __SERIES60_HELP
// ---------------------------------------------------------
// CBrowserContentViewContainer::GetHelpContext()
// ---------------------------------------------------------
//
void CBrowserContentViewContainer::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    const TUid KUidOperatorMenuApp = { 0x10008D5E };
    aContext.iMajor = KUidBrowserApplication;
    if ( iApiProvider.IsEmbeddedModeOn() )
        {
        if ( CBrowserAppUi::Static()->IsEmbeddedInOperatorMenu() )
            {
            aContext.iMajor = KUidOperatorMenuApp;
            aContext.iContext = KOPERATOR_HLP_OPTIONS_LIST;
            }
        else
            {
            aContext.iContext = KOSS_HLP_PAGEVIEW_EMBED;
            }

        }
    else
        {
        aContext.iContext = KOSS_HLP_PAGEVIEW;
        }
    }

#endif // __SERIES60_HELP

// End of File
