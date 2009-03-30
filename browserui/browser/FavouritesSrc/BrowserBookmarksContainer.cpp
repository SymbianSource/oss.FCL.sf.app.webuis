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
#include <BrowserNG.rsg>
#include <FavouritesItem.h>
#include <FeatMgr.h>

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
    	
    	// redraw
    	SizeChanged();
    	
    	iGotoPane->BeginEditingL();
    	
       	DrawDeferred();      	
    	}
    }


// ---------------------------------------------------------
// CBrowserBookmarksContainer::SetGotoInactiveL();
// ---------------------------------------------------------
//

void CBrowserBookmarksContainer::SetGotoInactiveL()
	{
	if(iGotoPaneActive)
		{
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
	
// ---------------------------------------------------------
// CBrowserBookmarksContainer::CountComponentControls
// ---------------------------------------------------------
//
TInt CBrowserBookmarksContainer::CountComponentControls() const
    {
    return (Listbox() ? 1 : 0) + (iGotoPane ? 1 : 0);
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
    if( iGotoPaneActive )
    	{
        	// Enable / disable line in Goto Pane (hide if listbox is empty).
        	iGotoPane->SetLineState( unfilteredNumberOfItems > 0 );
        	
        	// In Goto Mode, the listbox is laid out leaving space for Goto Pane.
        	AknLayoutUtils::LayoutControl( Listbox(), rect, AknLayout::list_gen_pane( 1 ));
        	
        	// Lay out Goto Pane as if it was the old Find Pane.
        	AknLayoutUtils::LayoutControl( iGotoPane, rect, AknLayout::find_pane() );
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
		// If the Goto Pane exists and we're not fetching, then decide
		// if we should pass keystroke to it
	if(iGotoPane)
		{
		// If the key so far hadn't been consumed or if the pane is already active,
		// pass the keystroke on
		if( (result == EKeyWasNotConsumed) || (iGotoPaneActive) )
			{			
        	// Goto pane has highest priority; if it's active, arrow keys go there.
        	// If fetching is in progress, no key events are offered, to prevent it
        	// from getting the focus and popping up a new CBA.
        	
        	// Enter Key  now handled through HandleCommand in BrowserBookmarksView like MSK
			// This change affects the enter key on the QWERTY keyboard when we run emulator
        	if(EStdKeyEnter == aKeyEvent.iScanCode && EEventKeyUp == aType && AknLayoutUtils::MSKEnabled() && iGotoPaneActive )
				{
				CEikButtonGroupContainer* myCba = CEikButtonGroupContainer::Current();
				if(myCba != NULL)
					{
					TInt cmdId = myCba->ButtonGroup()->CommandId(CEikButtonGroupContainer::EMiddleSoftkeyPosition);
					View().HandleCommandL(cmdId);
					result = EKeyWasConsumed;
					}
				}
			else
				{
				result = iGotoPane->OfferKeyEventL( keyEvent, aType );	
				}
        
			// if key is consumed, goto pane was not active, make it active now.
        	if(result == EKeyWasConsumed &&
        		!iGotoPaneActive)
        		{
        		SetGotoActiveL();	
        		}
        	}
		}
	// For handling Enter key in emulator / Keyboard ( Enter key should behave similar to MSK )
	if(EStdKeyEnter == keyEvent.iScanCode && EEventKey == aType && AknLayoutUtils::MSKEnabled() && result == EKeyWasNotConsumed )
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
        result = CBrowserFavouritesContainer::OfferKeyEventL
            ( keyEvent, aType );
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
: iGotoPaneActive( EFalse )
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
    iGotoPane = CBrowserBookmarksGotoPane::NewL( *this, &aView );
    iGotoPane->SetFocus( EFalse );
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
        if (iGotoPaneActive && iGotoPane)
			{		
    		iGotoPane->HandlePointerEventL(aPointerEvent);
			}
        else
        	{
    		// normally, pass all pointer events down to the listbox
    		Listbox()->HandlePointerEventL(aPointerEvent);        		
        	}
        }
    }

// End of File
