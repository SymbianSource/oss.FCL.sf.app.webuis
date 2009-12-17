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
*      Implementation of CBrowserFavouritesContainer.
*
*
*/


// INCLUDE FILES
#include <eikimage.h>
#include <AknAppUi.h>
#include <AknNavi.h>
#include <AknNaviDe.h>
#include <AknTabGrp.h>
#include <AknNaviLabel.h>
#include <AknTitle.h>
#include <aknsfld.h>
#include <calslbs.h>
#include <barsread.h>	// for TResourceReader
#include <EikSpane.h>
#include <Avkon.hrh>
#include <BrowserNG.rsg>
#include <favouritesdb.h>

#include <aknconsts.h>
#include <akneditstateindicator.h>
#include <AknsListBoxBackgroundControlContext.h>
#include <aknlayout.cdl.h>

#include "Display.h"
#include "Preferences.h"
#include "BrowserAppUi.h"
#include "BrowserUIVariant.hrh"
#include "CommonConstants.h"
#include "BrowserFavouritesView.h"
#include "BrowserFavouritesContainer.h"
#include "BrowserFavouritesListbox.h"
#include "BrowserFavouritesListboxModel.h"
#include "BrowserFavouritesListboxIconHandler.h"
#include "logger.h" 

// CONSTANTS
LOCAL_D const TInt KTabId = 88888;
LOCAL_D const TInt KMaxNaviText = 25;   // format is "<int>/<int>".
_LIT( KFormat, "%d/%d" );

// ============================= LOCAL FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::~CBrowserFavouritesContainer
// -----------------------------------------------------------------------------
//
CBrowserFavouritesContainer::~CBrowserFavouritesContainer()
	{
    delete iListbox;
    delete iIconHandler;
	delete iSkinContext;
	delete iNaviPaneTabsFolder;
	}


// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::ShowRootNaviPane
// -----------------------------------------------------------------------------
//
void CBrowserFavouritesContainer::ShowRootNaviPane()
    {
    // If text is showing, destroy now - this pops off teh Navi Pane.
	delete iNaviPaneTabsFolder;
    iNaviPaneTabsFolder = NULL;
    }

// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::ShowFolderNaviPaneL
// -----------------------------------------------------------------------------
//
void CBrowserFavouritesContainer::ShowFolderNaviPaneL
( TInt aFolderIndex, TInt aFolderCount )
    {
	CAknNavigationControlContainer* naviPane =
        iView->ApiProvider().Display().NaviPaneL();
    if ( !iNaviPaneTabsFolder )
        {
        // Folder's tab group not yet created. Make it now.
        iNaviPaneTabsFolder = naviPane->CreateTabGroupL();
        iNaviPaneTabsFolder->SetNaviDecoratorObserver(this);
        }

    TBuf<KMaxNaviText> buf;
    // Format Navi Pane text "1/4" style.
    buf.Format( KFormat, aFolderIndex + 1, aFolderCount );
    AknTextUtils::LanguageSpecificNumberConversion( buf );

    CEikImage* folderIcon = new (ELeave) CEikImage;
    CleanupStack::PushL( folderIcon );
    TResourceReader rr;
    iCoeEnv->CreateResourceReaderLC( rr, R_BROWSER_ICON_PROP_FOLDER );
    folderIcon->ConstructFromResourceL( rr );
    folderIcon->SetPictureOwnedExternally( EFalse );    // has ownership now
    CleanupStack::PopAndDestroy(); // rr

    CAknTabGroup* tabGroup = 
        STATIC_CAST( CAknTabGroup*, iNaviPaneTabsFolder->DecoratedControl() );
    if ( tabGroup->TabCount() )
        {
        // Already created. Change the contents.
        tabGroup->ReplaceTabL
            (
            KTabId,
            buf,
            folderIcon->Bitmap(),                       // ownership passed
            folderIcon->Mask()                          // ownership passed
            );
        }
    else
        {
        // Created just now. Add the contents.
        tabGroup->AddTabL
            (
            KTabId,
            buf,
            folderIcon->Bitmap(),                       // ownership passed
            folderIcon->Mask()                          // ownership passed
            );
        }
    folderIcon->SetPictureOwnedExternally( ETrue );     // ownership passed
    CleanupStack::PopAndDestroy();  // folderIcon

    tabGroup->SetTabFixedWidthL( EAknTabWidthWithOneTab );
    tabGroup->SetActiveTabById( KTabId );

	iNaviPaneTabsFolder->SetScrollButtonDimmed
		( CAknNavigationDecorator::ELeftButton, aFolderIndex == 0 );
	iNaviPaneTabsFolder->SetScrollButtonDimmed
		( 
		CAknNavigationDecorator::ERightButton,
		aFolderIndex == aFolderCount - 1
		);

	// If not yet pushed, this will do the push; if already there, this brings
    // it to top and draws.
    naviPane->PushL( *iNaviPaneTabsFolder );
    }

// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::ComponentControl
// -----------------------------------------------------------------------------
//
CCoeControl* CBrowserFavouritesContainer::ComponentControl
( TInt aIndex ) const
	{
    switch (aIndex)
        {
        case 0:
            return iListbox;

        default:
            return 0;
        }
	}

// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::CountComponentControls
// -----------------------------------------------------------------------------
//
TInt CBrowserFavouritesContainer::CountComponentControls() const
	{
    return (iListbox ? 1 : 0) ;
	}

// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::SizeChanged
// -----------------------------------------------------------------------------
//
void CBrowserFavouritesContainer::SizeChanged()
	{
    // Listbox is empty; listbox takes the whole area.
    iListbox->SetRect( Rect() );
	}

// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::OfferKeyEventL
// -----------------------------------------------------------------------------
//
TKeyResponse CBrowserFavouritesContainer::OfferKeyEventL
( const TKeyEvent& aKeyEvent, TEventCode aType )
	{
	CBrowserAppUi* ui = CBrowserAppUi::Static();
    TKeyResponse result = EKeyWasNotConsumed;
    
	if (ui->OfferApplicationSpecificKeyEventL(aKeyEvent, aType) == EKeyWasConsumed)
		{
		return EKeyWasConsumed;
		}


    if ( result == EKeyWasNotConsumed )
        {
        // Otherwise, give the view a chance to handle arrow presses
        // (moving between subfolders).
        result = iView->OfferKeyEventL( aKeyEvent, aType );
        }

    if ( result == EKeyWasNotConsumed && iListbox )
        {
        // Otherwise, let the listbox fiddle with it. This will NOT consume
        // arrow presses (base class overridden).
        result = iListbox->OfferKeyEventL( aKeyEvent, aType );
        
        if( (aKeyEvent.iCode == EKeyDownArrow) || (aKeyEvent.iCode == EKeyUpArrow) )  
                {  
                BROWSER_LOG( ( _L("Need to update tool bar buttons") ) );  
                iView->UpdateToolbarButtonsState();  
                }  

        }

    return result;
	}

// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::HandleCursorChangedL
// -----------------------------------------------------------------------------
//
void CBrowserFavouritesContainer::HandleCursorChangedL
( CEikListBox* /*aListBox*/ )
    {
    }

// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::HandleNaviDecoratorEventL
//
// Called when user touches the left or right arrow in navipane
// -----------------------------------------------------------------------------
//
void CBrowserFavouritesContainer::HandleNaviDecoratorEventL( TInt aEventID )
    {
    // used to swap the arrow key functionality for Arabic, etc...
    TBool forward(!(AknLayoutUtils::PenEnabled() && AknLayoutUtils::LayoutMirrored()));
    
    switch (aEventID)
        {
        case MAknNaviDecoratorObserver::EAknNaviDecoratorEventRightTabArrow:
            iView->ShowNextFolerL(forward);
            break;
        case MAknNaviDecoratorObserver::EAknNaviDecoratorEventLeftTabArrow:
            iView->ShowNextFolerL(!forward);
            break;
        default:
            break;
        }
    }


// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::ConstructComponentControlsL
// -----------------------------------------------------------------------------
//
void CBrowserFavouritesContainer::ConstructComponentControlsL(
        const TRect& /*aRect*/,
        CBrowserFavouritesView& aView )
	{
    // Construct listbox.
    iIconHandler = CreateListboxIconHandlerL();
    iListbox = CBrowserFavouritesListbox::NewL( iView->ApiProvider(), this, *iIconHandler );
    iListbox->SetListBoxObserver( &aView );
    iListbox->SetListboxCursorObserver( this );
    HBufC* buf = iCoeEnv->AllocReadResourceLC( ListboxEmptyTextResourceId() );
    iListbox->View()->SetListEmptyTextL( *buf );
    CleanupStack::PopAndDestroy();  // buf

	}

// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::ConstructL
// -----------------------------------------------------------------------------
//
void CBrowserFavouritesContainer::ConstructL
( const TRect& aRect, CBrowserFavouritesView& aView )
	{
    iView = &aView;

	CAknNavigationControlContainer* naviPane =
        iView->ApiProvider().Display().NaviPaneL();

    CreateWindowL();
    SetMopParent( iView );

    ConstructComponentControlsL( aRect, *iView );

    TAknsItemID tileIID = KAknsIIDSkinBmpListPaneNarrowA;
    TAknLayoutRect listGenPane;
    listGenPane.LayoutRect(iAvkonAppUi->ClientRect() , 
        AknLayout::list_gen_pane( 0 ));

    TAknLayoutRect column;
    column.LayoutRect(iAvkonAppUi->ClientRect() , 
        AknLayout::A_column());

    iSkinContext = CAknsListBoxBackgroundControlContext::NewL(
        KAknsIIDSkinBmpMainPaneUsual, 
        listGenPane.Rect(), //TODOVRa: Should come from LAF!
        EFalse, tileIID,
        column.Rect() ); //TODOVRa: Should come from LAF!
    SetRect( aRect );
    ActivateL();
    }


// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::MopSupplyObject
// -----------------------------------------------------------------------------
//
TTypeUid::Ptr CBrowserFavouritesContainer::MopSupplyObject( TTypeUid aId )
    {
    if( aId.iUid == MAknsControlContext::ETypeId  )
        {
        return MAknsControlContext::SupplyMopObject( aId, iSkinContext );
        }

	return SupplyMopObject( aId, (MAknEditingStateIndicator*)NULL );
    }

// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::HandleResourceChange
// -----------------------------------------------------------------------------
//
void CBrowserFavouritesContainer::HandleResourceChange( TInt aType )
    {
    // goes through all the subcomponents
    CCoeControl::HandleResourceChange( aType );
    
    if (aType == KEikDynamicLayoutVariantSwitch)
        {
        SetRect(iView->ClientRect());
        DrawDeferred();
        }
    }

// -----------------------------------------------------------------------------
// CBrowserFavouritesContainer::FocusChanged
// -----------------------------------------------------------------------------
//
void CBrowserFavouritesContainer::FocusChanged( TDrawNow aDrawNow )
    {
	if ( iListbox && iListbox->IsVisible() )
        {
        Listbox()->SetFocus( IsFocused(), aDrawNow );
        }
    }

// End of File
