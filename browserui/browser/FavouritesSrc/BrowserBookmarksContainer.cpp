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
*       Implementation of CBrowserBookmarksContainer.
*
*
*/


// INCLUDE FILES

#include <AknViewAppUi.h>
#include <aknsfld.h>
#include <avkon.mbg>
#include <BrowserNG.rsg>
#include <FavouritesItem.h>
#include <FeatMgr.h>
#include "BrowserBookmarksView.h"

#include "CommonConstants.h"
#include "BrowserFavouritesView.h"
#include "BrowserBookmarksContainer.h"
#include "BrowserBookmarksGotoPane.h"
#include "BrowserFavouritesListbox.h"
#include "BrowserUtil.h"
#include "BrowserAppUi.h"
#include "BrowserBookmarksListboxIconHandler.h"
#ifdef __SERIES60_HELP
// Context-Sensitve Help File
#include "BrowserApplication.h"
#include <csxhelp/browser.hlp.hrh>
#endif // __SERIES60_HELP
#include "Logger.h"
#include "e32event.h"
#include "BrowserUiSDKCRKeys.h"
#include <StringLoader.h>

// CONSTANTS

/// Tab index for Bookmarks View.
LOCAL_D const TInt KBookmarkTabIndex = 0;

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserBookmarksContainer::NewL
// ---------------------------------------------------------
//
CBrowserBookmarksContainer* CBrowserBookmarksContainer::NewL(
        const TRect& aRect,
        CBrowserFavouritesView& aView)
    {
    CBrowserBookmarksContainer* container =
        new (ELeave) CBrowserBookmarksContainer;
    CleanupStack::PushL( container );
    container->ConstructL( aRect, aView );
    CleanupStack::Pop();    // container
    return container;
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::~CBrowserBookmarksContainer
// ---------------------------------------------------------
//
CBrowserBookmarksContainer::~CBrowserBookmarksContainer()
    {
    delete iGotoPane;
    delete iSearchPane;
    delete iDefaultSearchText;
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::SetGotoActiveL
// ---------------------------------------------------------
//
void CBrowserBookmarksContainer::SetGotoActiveL()
    {
    if( !iGotoPaneActive)
        {
        iGotoPaneActive = ETrue;

        Listbox()->UpdateFilterL();
        if( iSearchPane )
            {
            iGotoPane->SetVKBFlag( EFalse );
            // Check for Search Configuration Changes //
            if( View().ApiProvider().Preferences().GetIntValue( KBrowserSearchIconId )
                    != iSearchIconId )
                {
                ConstructSearchPaneL();
                }

            iSearchPane->SetFocus( EFalse );
            // only one editor can be active at a time //
            iSearchPaneActive = EFalse;
            // we still need to display search, when showing for first time //
            iSearchPane->MakeVisible(ETrue);
            iSearchPane->CancelEditingL();
            }

        // redraw
        SizeChanged();

        // It is important to set the Text and Italic property after SizeChanged, because
        // iInputFrame->SetRect() event on GoTo/Search Pane calls the scalable version
        // ( AknLayoutUtils::LayoutEdwinScalable ) which overwrites all the properties for
        // Char and Para Formats.
        if( iSearchPane )
            {
            iSearchPane->SetTextModeItalicL();
            HBufC* text = iSearchPane->GetTextL();
            CleanupStack::PushL( text );
            if ( !text->Length() )
                {
                iSearchPane->SetTextL( *iDefaultSearchText );
                }
            CleanupStack::PopAndDestroy( text );
            }

        iGotoPane->BeginEditingL();
        DrawDeferred();
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::SetSearchActiveL
// ---------------------------------------------------------
//
void CBrowserBookmarksContainer::SetSearchActiveL()
    {
    if( ! iSearchPaneActive )
        {
        // Check for Search Configuration Changes //
        if( View().ApiProvider().Preferences().GetIntValue( KBrowserSearchIconId )
                != iSearchIconId )
            {
            ConstructSearchPaneL();
            }

        iSearchPane->SetVKBFlag( EFalse );
        iSearchPane->SetFocus( ETrue );
        iSearchPaneActive = ETrue;

        Listbox()->UpdateFilterL();
        // only one editor can be active at a time
        iGotoPaneActive = EFalse;
        iGotoPane->MakeVisible(ETrue);
        iGotoPane->SetFocus( EFalse );
        iGotoPane->CancelEditingL();

        // redraw
        SizeChanged();

        // Not Focused, so set the current Text Mode to italics.
        iGotoPane->SetTextModeItalicL();
        // if gotopane is empty add default text
        HBufC* text = iGotoPane->GetTextL();
        CleanupStack::PushL( text );
        if ( !text->Length() )
            {
            iGotoPane->SetTextL( KWWWString );
            }
        CleanupStack::PopAndDestroy( text );

        iSearchPane->BeginEditingL();
        DrawDeferred();
        }

    }


// ---------------------------------------------------------
// CBrowserBookmarksContainer::SetGotoInactiveL();
// ---------------------------------------------------------
//

void CBrowserBookmarksContainer::SetGotoInactiveL()
    {
    if( iGotoPaneActive || iSearchPaneActive )
        {

          if( iSearchPane )
            {
            /* In Search Feature we have to move between the editors preserving the
             * text added, clear the text when cancel is pressed.
             */
            iGotoPane->SetTextL( KNullDesC , ETrue );
            SetSearchInactiveL();
            }
        // Deactivate GoTo Pane
        iGotoPaneActive = EFalse;

        // Deactivate GoTo Pane
        iGotoPane->MakeVisible( EFalse );
        iGotoPane->CancelEditingL();
        iGotoPane->SetFocus( EFalse );

        // redraw
        Listbox()->UpdateFilterL();
        SizeChanged();
        DrawDeferred();
        }
    }

void CBrowserBookmarksContainer::SetSearchInactiveL()
    {
    iSearchPaneActive = EFalse;
    iSearchPane->SetTextL( KNullDesC , ETrue);
    iSearchPane->MakeVisible( EFalse );
    iSearchPane->CancelEditingL();
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::CountComponentControls
// ---------------------------------------------------------
//
TInt CBrowserBookmarksContainer::CountComponentControls() const
    {
    return (Listbox() ? 1 : 0) + (iGotoPane ? 1 : 0) + (iSearchPane? 1 : 0) ;
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::ComponentControl
// ---------------------------------------------------------
//
CCoeControl* CBrowserBookmarksContainer::ComponentControl
( TInt aIndex ) const
    {
    switch (aIndex)
        {
        case 0:
            {
            return Listbox();
            }

        case 1:
            {
            return iGotoPane;
            }
        case 2:
            {
            return iSearchPane;
            }
        default:
            {
            return NULL;
            }
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::SizeChanged
// ---------------------------------------------------------
//
void CBrowserBookmarksContainer::SizeChanged()
    {
    TRect rect = Rect();
    TInt unfilteredNumberOfItems = Listbox()->UnfilteredNumberOfItems();

    // if goto pane is active
    if( iGotoPaneActive || iSearchPaneActive )
        {
        if( iSearchPane )
            {
            TAknWindowLineLayout findPane = AknLayout::find_pane();

            // Enable / disable line in Goto Pane (hide if listbox is empty).
            iGotoPane->SetLineState( unfilteredNumberOfItems > 0 );

            TAknWindowLineLayout listLayout = AknLayout::list_gen_pane( 1 );

            TRect listBoxRect = AknLayoutUtils::RectFromCoords( rect,listLayout.il,
                    listLayout.it, listLayout.ir, listLayout.ib, listLayout.iW, listLayout.iH);

            /* Now we need to make room for both Goto and Search Pane.
             * No layout specification was suitable for this case, so
             * substracting the height of pane.
             */
            listBoxRect.iBr.iY -= findPane.iH;

            // In Goto Mode, the listbox is laid out leaving space for Goto Pane.
            Listbox()->SetRect( listBoxRect );


            // Now Lay out Search Pane as if it was the old Find Pane.
            AknLayoutUtils::LayoutControl( iSearchPane, rect, findPane );

            TRect gotoRect = AknLayoutUtils::RectFromCoords( rect,findPane.il, findPane.it,
                    findPane.ir, findPane.ib, findPane.iW, findPane.iH);

            // Calculate the new position for GoToPane.
            gotoRect.iTl.iY -= ( findPane.iH );
            gotoRect.iBr.iY -= ( findPane.iH );
            iGotoPane->SetRect( gotoRect );

            }
        else
            {
            // Enable / disable line in Goto Pane (hide if listbox is empty).
            iGotoPane->SetLineState( unfilteredNumberOfItems > 0 );

            // In Goto Mode, the listbox is laid out leaving space for Goto Pane.
            AknLayoutUtils::LayoutControl( Listbox(), rect, AknLayout::list_gen_pane( 1 ));

            // Lay out Goto Pane as if it was the old Find Pane.
            AknLayoutUtils::LayoutControl( iGotoPane, rect, AknLayout::find_pane() );
            }
        }
    else
        {
            // Fall back upon default behavior in base-class, which lays out the bookmarks list only
            CBrowserFavouritesContainer::SizeChanged();
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::OfferKeyEventL
// ---------------------------------------------------------
//
TKeyResponse CBrowserBookmarksContainer::OfferKeyEventL
( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
/*
LOG_ENTERFN( "BookmarksContainer::OfferKeyEventL" );
BROWSER_LOG( ( _L("aType: %d, aKeyEvent.iCode: %d, iScanCode: %d, iRepeats: %d"),
    aType, aKeyEvent.iCode, aKeyEvent.iScanCode, aKeyEvent.iRepeats ) );
*/
    TKeyResponse result = EKeyWasNotConsumed;
    TKeyEvent keyEvent( aKeyEvent );

    // Selection Key handling
    if( keyEvent.iScanCode == EStdKeyDevice3 )
        {
        // it is possible to activate BookmarksView from ContentView
        // via a long press of Selection key, so here we must capture
        // the KeyDown. Long press processing will be done only if key
        // was pressed originally in this view.
        if( aType == EEventKeyDown )
            {
            iSelectionKeyDownPressed = ETrue;
            result = EKeyWasConsumed;
            }
        else if( aType == EEventKeyUp )
            {
            if( !iSelectionKeyLongPress && iSelectionKeyDownPressed )
                {
                // short press of Selection key, pass it to GotoPane
                keyEvent.iCode = EKeyOK;
                result = EKeyWasNotConsumed;
                }
            else
                {
                // long press of Selection key was already processed
                result = EKeyWasConsumed;
                }
            iSelectionKeyDownPressed = EFalse;
            iSelectionKeyLongPress = EFalse;
            }
        else if( aType == EEventKey )
            {
            if( keyEvent.iRepeats && iSelectionKeyDownPressed )
                {
                const CFavouritesItem* item = Listbox()->CurrentItem();
                if ( item )
                    {
                    // The option for the user to download the page in new window is disabled
                    CBrowserAppUi::Static()->FetchBookmarkL( *item );
                    }
                iSelectionKeyLongPress = ETrue;
                iSelectionKeyDownPressed = EFalse;
                }

                if ( (aKeyEvent.iScanCode == EStdKeyHash)  ||
                        ( aKeyEvent.iModifiers &
                        ( EModifierShift | EModifierLeftShift | EModifierRightShift |
                        EModifierCtrl | EModifierLeftCtrl | EModifierRightCtrl ) ) )
                    {

                    // Hash key press will be used for Mark/UnMark functionality
                    // Let Platform Listbox handle this.
                    result = EKeyWasNotConsumed;
                    }
                else
                    {
                    result = EKeyWasConsumed;
                    }
            }
        }

    // If the Goto Pane exists and we're not fetching, decide
    // if we should pass key event to the goto pane
    if ( iGotoPane && !CBrowserAppUi::Static()->Fetching() )
        {
        // If the key hadn't been consumed, so far, determine if the keystroke
        // should be handled by goto pane
        if ( ( result == EKeyWasNotConsumed ) || iSearchPaneActive )
            {
            // Goto pane has highest priority; if it's active, arrow keys go there.
            // If fetching is in progress, no key events are offered, to prevent it
            // from getting the focus and popping up a new CBA.

            // Enter Key now handled through HandleCommand in BrowserBookmarksView like MSK
            // This change affects the enter key on the QWERTY keyboard when we run emulator
            if ( EStdKeyEnter == aKeyEvent.iScanCode && EEventKey == aType &&
                 AknLayoutUtils::MSKEnabled() &&
                 ( iGotoPaneActive || iSearchPaneActive ) )
                {
                // Handle MSK press
                CEikButtonGroupContainer* myCba = CEikButtonGroupContainer::Current();
                if ( myCba != NULL )
                    {
                    TInt cmdId = myCba->ButtonGroup()->CommandId(CEikButtonGroupContainer::EMiddleSoftkeyPosition);
                    View().HandleCommandL(cmdId);
                    result = EKeyWasConsumed;
                    }
                }
            else
                {
                if ( iSearchPane )
                    {
			        // Process separately for Search Feature
			        if ( iGotoPaneActive )
			            {
                        result = iGotoPane->OfferKeyEventL( keyEvent, aType );
			            }
                    else
                        {
                        result = iSearchPane->OfferKeyEventL( keyEvent, aType );
                        }
                    }
                else
                    {
                    if ( iGotoPaneActive )
                        {
                        // Once activated let goto pane handle all the keys
                        result = iGotoPane->OfferKeyEventL( keyEvent, aType );
                        }
                    else // if ( !iGotoPaneActive )
                        {
                        // Only a valid digit or character should popup the goto pane.
                        // We will ignore h/w key events, i.e. camera cover, camera
                        // shutter, zoom, etc... iCode is only valid for EEventKey,
                        // not EEventKeyDown, so we have to use iScanCode. because
                        // the goto pane decides to popup on EEventKeyDown. Use
                        // upper case of iScanCode since there is no overlap of
                        // h/w scan codes and uppercase letters.
                        TChar inputCharCode( aKeyEvent.iScanCode );
                        TBool isDigit = inputCharCode.IsDigit();
                        TUint upperCase = inputCharCode.GetUpperCase();
                        if ( isDigit ||
                             (( upperCase >= 0x41 /*A*/ ) && ( upperCase <= 0x5A /*Z*/ )) )
                            {
                            // Valid digit or character was entered, let goto pane
                            // determine if it handles the keys from here on.
                            result = iGotoPane->OfferKeyEventL( aKeyEvent, aType );
                            if ( result == EKeyWasConsumed )
                                {
                                // goto pane is not active, make it active now
                                SetGotoActiveL();
                                }
                            }
                        }
                    }
                }
            }

            if ( iSearchPane )
                {
                if ( ( iGotoPaneActive || iSearchPaneActive) &&
                       result == EKeyWasNotConsumed )
                    {

                    if (  aKeyEvent.iCode == EKeyUpArrow )
                        {
                        result = EKeyWasConsumed;
                        if ( iSearchPaneActive )
                            {
                            SetGotoActiveL();
                            iGotoPane->SetVKBFlag( ETrue );
                            }
                        }

                    if ( aKeyEvent.iCode == EKeyDownArrow )
                        {
                        result = EKeyWasConsumed;
                        if ( iGotoPaneActive )
                            {
                            SetSearchActiveL();
                            iSearchPane->SetVKBFlag( ETrue );
                            }
                        }
                    }
                }
            }

    // For handling Enter key in emulator / Keyboard ( Enter key should behave similar to MSK )
    if ( EStdKeyEnter == keyEvent.iScanCode && EEventKey == aType &&
         AknLayoutUtils::MSKEnabled() && result == EKeyWasNotConsumed )
        {
        CEikButtonGroupContainer* myCba = CEikButtonGroupContainer::Current();
        if(myCba != NULL)
            {
            TInt cmdId = myCba->ButtonGroup()->CommandId(CEikButtonGroupContainer::EMiddleSoftkeyPosition);
            if(EAknSoftkeyContextOptions  == cmdId)
                {
                View().MenuBar()->TryDisplayContextMenuBarL();
                result = EKeyWasConsumed;
                }
            else if(Listbox()->Model()->ItemTextArray()->MdcaCount() == 0)
                {
                View().HandleCommandL(cmdId);
                result = EKeyWasConsumed;
                }
            }
        }

    if ( result == EKeyWasNotConsumed )
        {
        // Otherwise, base class handles Find pane, arrows between folders and
        // the listbox.
        result = CBrowserFavouritesContainer::OfferKeyEventL( keyEvent, aType );
        }

    return result;
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::HandleCursorChangedL
// ---------------------------------------------------------
//
void CBrowserBookmarksContainer::HandleCursorChangedL
( CEikListBox*
#ifdef _DEBUG
    aListBox  // used only for debugging purposes
#endif
 )
    {
#ifdef _DEBUG
    __ASSERT_DEBUG( aListBox == Listbox(),
        Util::Panic( Util::EFavouritesBadListbox ) );
#endif

    if ( iGotoPane->IsVisible() )
        {
        // Set Goto Pane text (URL or default http text) - this will cancel
        // Goto Pane editing.
        TPtrC url( KWWWString );
        TBrowserFavouritesSelectionState selection =
            Listbox()->SelectionStateL();
        if ( !selection.AnyMarked() && selection.CurrentIsItem() )
            {
            // Exactly one item is selected.
            const CFavouritesItem* item = Listbox()->CurrentItem();
            if ( item ) // Sanity check.
                {
                // If one item is highlighted, set to current URL.
                url.Set( Util::StripUrl( item->Url() ) );
                }
            }
        iGotoPane->SetTextL( url, ETrue );
        iGotoPane->SetFocus ( EFalse );
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::IsEditing
// ---------------------------------------------------------
//
TBool CBrowserBookmarksContainer::IsEditing()
    {
    TBool editing = EFalse;
    editing = iGotoPane->IsEditing();
    return editing;
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::IsEditingSearch
// ---------------------------------------------------------
//
TBool CBrowserBookmarksContainer::IsEditingSearch()
    {
    TBool editing = EFalse;
    if( iSearchPane )
        {
        editing = iSearchPane->IsEditing();
        }
    return editing;
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::TabIndex
// ---------------------------------------------------------
//
TInt CBrowserBookmarksContainer::TabIndex()
    {
    return KBookmarkTabIndex;
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::ListboxEmptyTextResourceId
// ---------------------------------------------------------
//
TInt CBrowserBookmarksContainer::ListboxEmptyTextResourceId()
    {
    return R_BROWSER_BOOKMARKS_TEXT_NO_BOOKMARKS;
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::TitleResourceId
// ---------------------------------------------------------
//
TInt CBrowserBookmarksContainer::TitleResourceId()
    {
    return R_BROWSER_OPTION_BOOKMARKS;
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::CBrowserBookmarksContainer
// ---------------------------------------------------------
//
CBrowserBookmarksContainer::CBrowserBookmarksContainer()
    : iGotoPaneActive( EFalse ),
    iSearchPaneActive(EFalse)
    {
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::ConstructComponentControlsL
// ---------------------------------------------------------
//
void CBrowserBookmarksContainer::ConstructComponentControlsL(
        const TRect& aRect,
        CBrowserFavouritesView& aView )
    {
    CBrowserFavouritesContainer::ConstructComponentControlsL
        ( aRect, aView );


    // Construct Goto Pane.

    //pass view to bookmarks goto pane
    iGotoPane = CBrowserBookmarksGotoPane::NewL( *this, &aView, KAvkonBitmapFile,
            EMbmAvkonQgn_indi_find_goto,
            EMbmAvkonQgn_indi_find_goto_mask);
    iGotoPane->SetFocus( EFalse );

    if( View().ApiProvider().Preferences().SearchFeature() )
        {
        ConstructSearchPaneL();
        iSearchPane->SetFocus( EFalse );
        iDefaultSearchText = StringLoader::LoadL( R_IS_WEB_SEARCH );
        }
    }

// ---------------------------------------------------------
// CBrowserBookmarksContainer::CreateListboxIconHandlerL
// ---------------------------------------------------------
//
MBrowserFavouritesListboxIconHandler*
CBrowserBookmarksContainer::CreateListboxIconHandlerL()
    {
    return new (ELeave) TBrowserBookmarksListboxIconHandler;
    }

#ifdef __SERIES60_HELP
// ---------------------------------------------------------
// CBrowserBookmarksContainer::GetHelpContext()
// ---------------------------------------------------------
//
void CBrowserBookmarksContainer::GetHelpContext( TCoeHelpContext& aContext ) const
    {
    aContext.iMajor = KUidBrowserApplication;
    aContext.iContext = KOSS_HLP_BM_MAIN;
    }
#endif // __SERIES60_HELP

// ---------------------------------------------------------
// CBrowserBookmarksContainer::FocusChanged()
// ---------------------------------------------------------
//
void CBrowserBookmarksContainer::FocusChanged( TDrawNow aDrawNow )
    {
    if ( IsEditing() )
        {
        iGotoPane->SetFocus( IsFocused(), aDrawNow );
        }
    else if( IsEditingSearch() )
        {
        iSearchPane->SetFocus( IsFocused(), aDrawNow );
        }
    else if ( Listbox() && Listbox()->IsVisible() )
        {
        Listbox()->SetFocus( IsFocused(), aDrawNow );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksContainer::HandlePointerEventL
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksContainer::HandlePointerEventL(const TPointerEvent& aPointerEvent)
    {
    if (AknLayoutUtils::PenEnabled())
        {
        // if goto is active, if a pointer event falls within its rect,
        // pass all pointer events to it (such as, to bring up the editor's letter-entry)
        //
        // otherwise, if it falls within the listbox's rect
        //

        // Now we have two different panes so we have to check explicitly before passing controls
        // to any panes.
        if( iGotoPane && ( iGotoPaneActive || iSearchPaneActive) )
            {
            if( iSearchPane )//implies presence of search feature.
                {
                 if( iGotoPane->Rect().Contains( aPointerEvent.iPosition) )
                     {
                     if( iGotoPaneActive )
                         {
                         iGotoPane->HandlePointerEventL(aPointerEvent);
                         }
                     else
                         {
                         SetGotoActiveL();
                         }
                     iGotoPane->SetVKBFlag( ETrue );
                     }
                 else if( iSearchPane->Rect().Contains( aPointerEvent.iPosition) )
                     {
                     if( iSearchPaneActive )
                         {
                         iSearchPane->HandlePointerEventL(aPointerEvent);
                         }
                     else
                         {
                         SetSearchActiveL();
                         }
                     iSearchPane->SetVKBFlag( ETrue );
                     }
                 else
                     {
                     CBrowserAppUi::Static()->ActiveView()->HandleCommandL(EWmlCmdGotoPaneCancel);
                     SetGotoInactiveL();
                     }
                }
             else
                 {
                 iGotoPane->HandlePointerEventL(aPointerEvent);
                 }
            }
        else
            {
            // normally, pass all pointer events down to the listbox
            Listbox()->HandlePointerEventL(aPointerEvent);
            }
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksContainer::ConstructSearchPaneL
// ----------------------------------------------------------------------------
//

void CBrowserBookmarksContainer::ConstructSearchPaneL()
    {
    TFileName iconFile;
    MPreferences& preferences = View().ApiProvider().Preferences();

    TInt iconId = preferences.GetIntValue( KBrowserSearchIconId );
    // Get the Search Icon File Path.
    preferences.GetStringValueL( KBrowserSearchIconPath, KMaxFileName, iconFile );

    // If Icon File Path Changed or Icon Id Changed, Refresh the Icon for Search Pane.
    // Comparing Icon File path as well, because it may be possible that two different
    // Icon files have same icon id.
    if( iconId != iSearchIconId
            || iSearchIconFilePath.Compare( iconFile ) != 0 )
        {

        TInt iconMaskId = preferences.GetIntValue( KBrowserSearchIconMaskId );

        // Save IconId
        iSearchIconId = iconId;
        // Save Icon File
        iSearchIconFilePath = iconFile;


        // No Icon file or IconId or IconMaskId set , then it means no search provider is still
        // selected and set by search application, in that case we use the default icon for Search.
        if( ! iconFile.Length()
                || iconId == -1
                || iconMaskId == -1 )
            {
            iconId = EMbmAvkonQgn_indi_find_glass;
            iconMaskId = EMbmAvkonQgn_indi_find_glass_mask;
            iconFile = KAvkonBitmapFile;
            }

        if( iSearchPane )
            {
            delete iSearchPane;
            iSearchPane = NULL;
            }


        // iSearchPane != NULL, implies presence of search feature, hence it can be
        // used to validate search feature avoiding repetative calls to utils and
        // avoiding need of separate variable.
        iSearchPane = CBrowserBookmarksGotoPane::NewL( *this,
                            &(View()),
                            iconFile,
                            iconId,
                            iconMaskId,
                            GOTOPANE_POPUPLIST_DISABLE,
                            ETrue
                            );

        CBrowserBookmarksView* bookmarkView = REINTERPRET_CAST( CBrowserBookmarksView* ,  &( View() ));
        // Set Pane observer
        iSearchPane->SetGPObserver( bookmarkView );
        }
    }

// ----------------------------------------------------------------------------
// CBrowserBookmarksContainer::HandleResourceChange
// ----------------------------------------------------------------------------
//
void CBrowserBookmarksContainer::HandleResourceChange(TInt aType)
    {
    CBrowserFavouritesContainer::HandleResourceChange( aType);
    if( aType == KEikDynamicLayoutVariantSwitch)
        {
        // If search feature exsist
        if( iSearchPane  )
            {
            if( iSearchPaneActive )
                {
                iGotoPane->SetTextModeItalicL( );
                }
            else
                {
                iSearchPane->SetTextModeItalicL( );
                }
            }
        }
    }
// End of File
