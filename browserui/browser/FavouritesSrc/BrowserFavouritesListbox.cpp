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
*      Implementation of CBrowserFavouritesListbox.
*
*
*/


// INCLUDE FILES

#include <aknkeys.h>
#include <EIKCLBD.H>
#include <gulicon.h>
#include <AknIconArray.h>
#include <FavouritesDb.h>
#include <BrowserNG.rsg>
#include <ApListItemList.h>
#include <FavouritesItem.h>
#include <FavouritesItemList.h>
#include <AknsConstants.h>

#include "BrowserFavouritesListbox.h"
#include "BrowserFavouritesListboxState.h"
#include "BrowserFavouritesListboxModel.h"
#include "BrowserFavouritesListboxView.h"
#include "BrowserFavouritesModel.h"
#include "CommsModel.h"
#include "BrowserFavouritesListboxIconHandler.h"
#include "BrowserFavouritesIconIndexArray.h"
#include "BrowserUtil.h"
#include "BrowserFavouritesListboxCursorObserver.h"
#include "BrowserFavouritesListboxState.h"
#include "BrowserFavouritesListboxItemDrawer.h"
#include "ApiProvider.h"
#include "BrowserAppUi.h"

#include "BrowserFaviconHandler.h"
#include <AknLayout2ScalableDef.h>
#include <aknlayoutscalable_avkon.cdl.h>

#include "eikon.hrh"


// CONSTANTS
/// Granularity of the icon index array.
LOCAL_C const TInt KGranularity = 4;


// ================= LOCAL FUNCTIONS =======================

/**
* Append elements from aSource to the end of aTarget.
* @param aTarget Array which receives copied elements from aSource.
* @param aSource Elements from this will be appended to aTarget.
*/
LOCAL_D void AppendArrayL
( CArrayFix<TInt>& aTarget, const CArrayFix<TInt>& aSource )
    {
    TInt i;
    TInt count = aSource.Count();
    for ( i = 0; i < count; i++ )
        {
        aTarget.AppendL( aSource.At( i ) );
        }
    }

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserFavouritesListbox::NewL
// ---------------------------------------------------------
//
CBrowserFavouritesListbox* CBrowserFavouritesListbox::NewL
        (
        MApiProvider& aApiProvider,
        const CCoeControl* aParent,
        const MBrowserFavouritesListboxIconHandler& aIconHandler
        )
    {
    CBrowserFavouritesListbox* listbox =
        new (ELeave) CBrowserFavouritesListbox( aApiProvider );
    CleanupStack::PushL( listbox );
    listbox->ConstructL( aParent, aIconHandler );
    CleanupStack::Pop();    // listbox
    return listbox;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::~CBrowserFavouritesListbox
// ---------------------------------------------------------
//
CBrowserFavouritesListbox::~CBrowserFavouritesListbox()
    {
    delete iFaviconHandler;
    delete iIconIndexes;
    delete iItems;
    delete iNewState;
    
    if ( iFontItalic )
        {
        CWsScreenDevice& screenDev = *( CEikonEnv::Static()->ScreenDevice() );
        screenDev.ReleaseFont( iFontItalic );
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::SetListboxCursorObserver
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::SetListboxCursorObserver
(MBrowserFavouritesListboxCursorObserver* aObserver)
    {
    iCursorObserver = aObserver;
    if ( View() )
        {
        STATIC_CAST( CBrowserFavouritesListboxView*, View() )->
            SetCursorObserver( aObserver );
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::SetDataL
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::SetDataL
( CFavouritesItemList* aItems, /*MCommsModel& aCommsModel,*/ TBool aKeepState )
    {
    __ASSERT_DEBUG( aItems, Util::Panic( Util::EUnExpected ) );

    // Ownership of aItems is taken, and will become a member (but not yet
    // that) - push until then.
    CleanupStack::PushL( aItems );

    // Get all favicons asynchronously by iteration on icon array
    iFaviconHandler->StartGetFaviconsL( aItems );

    // Get icon indexes into new list. Replace the existing
    // data only if successfully gotten. This ensures that they don't go out
    // of sync (there cannot be different number of items in the two list).
    CBrowserFavouritesIconIndexArray* newIconIndexes =
        GetIconIndexesLC( /*aCommsModel,*/ *aItems );

    if ( aKeepState )
        {
        // Calculate new state now, while we still have old items.
        CalcNewStateL( *aItems );
        }

    // Got new data. Empty existing data, but keep the empty lists.
    ClearSelection();
    iItems->ResetAndDestroy();
    iIconIndexes->Reset();
    SetTopItemIndex(0);
    UpdateFilterL();
    HandleItemRemovalL();
    // Replace data with new.
    delete iIconIndexes;                            // Not NULL-ed, because...
    delete iItems;                                  // Not NULL-ed, because...
    iItems = aItems;                                // ... this cannot leave
    iIconIndexes = newIconIndexes;                  // ... this cannot leave

    // Let the model know the change.
    TheModel()->SetData( *iItems, *iIconIndexes );  // ... this cannot leave

    CleanupStack::Pop( 2 ); // newIconIndexes, aItems: now members.
    HandleItemAdditionL();
    UpdateFilterL();
    if ( aKeepState )
        {
        // Now set the new state into listbox, then discard it.
        __ASSERT_DEBUG( iNewState, Util::Panic( Util::EUnExpected ) );
        SetStateL( *iNewState );
        delete iNewState;
        iNewState = NULL;
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::DrawFavicons
// ---------------------------------------------------------
//
TInt CBrowserFavouritesListbox::DrawFavicons()
    {
    // We have some favicons to draw
    TInt err = KErrNone;

    //Update the icon indices
    TRAP(   err,
            CBrowserFavouritesIconIndexArray* newIconIndexes =
                GetIconIndexesLC( /*aCommsModel,*/ *iItems );
            iIconIndexes->Reset();
            delete iIconIndexes;
            iIconIndexes = newIconIndexes;
            CleanupStack::Pop( newIconIndexes );
        );

    // Don't draw the favicons if there is a leave
    if ( !err )
        {
        // Let the model know the change and update the lisbox.
        TheModel()->SetData( *iItems, *iIconIndexes );
        DrawNow();
        }

    return err;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::SelectionStateL
// ---------------------------------------------------------
//
TBrowserFavouritesSelectionState
CBrowserFavouritesListbox::SelectionStateL() const
    {
    TBrowserFavouritesSelectionState state;

    // Check marking first.
    const CArrayFix<TInt>* selection = MarkedRealIndexesLC();
    TInt i;
    TInt count = selection->Count();
    for ( i = 0; i < count; i++ )
        {
        state.iMarkFlags |=
            SelectionFlags( *(iItems->At( selection->At( i ) )) );
        if ( iItems->At( selection->At( i ) )->IsItem() )
            {
            state.iMarkedItemCount++;
            }
        else
            {
            state.iMarkedFolderCount++;
            }
        }

    if ( count == iItems->Count() )
        {
        state.iMarkFlags |= TBrowserFavouritesSelectionState::EAll;
        }

    TInt visibleCount;
    CAknListBoxFilterItems* filter = TheModel()->Filter();
    visibleCount = filter ? filter->FilteredNumberOfItems() : iItems->Count();
    for ( i = 0; i < visibleCount; i++ )
        {
        if ( iItems->At( RealIndex( i ) )->IsItem() )
            {
            state.iVisibleItemCount++;
            }
        else
            {
            state.iVisibleFolderCount++;
            }
        }

    TKeyArrayFix key( 0, ECmpTInt32 );
    TInt dummy;
    if ( !selection->Find( CurrentItemRealIndex(), key, dummy ) )
        {
        state.iMarkFlags |=
            TBrowserFavouritesSelectionState::ECurrent;
        }

    CleanupStack::PopAndDestroy();  // selection

    // Then comes the current.
    const CFavouritesItem* current = CurrentItem();
    if ( current )
        {
        state.iCurrentFlags |= SelectionFlags( *current );
        }

    return state;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::CurrentItem
// ---------------------------------------------------------
//
const CFavouritesItem* CBrowserFavouritesListbox::CurrentItem() const
    {
    TInt index = CurrentItemRealIndex();
    if ( index >= 0 )
        {
        return iItems->At( index );
        }
    return NULL;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::MarkedItemL
// ---------------------------------------------------------
//
const CFavouritesItem* CBrowserFavouritesListbox::MarkedItemL() const
    {
    CFavouritesItem* item = NULL;
    CArrayFix<TInt>* marked = MarkedRealIndexesLC();
    if ( marked->Count() == 1 )
        {
        item = iItems->At( marked->At( 0 ) );
        }
    CleanupStack::PopAndDestroy();
    return item;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::HighlightUidNow
// ---------------------------------------------------------
//
TBool CBrowserFavouritesListbox::HighlightUidNow( TInt aUid )
    {
    TInt index = 0;
    if (aUid != 0)
        {
        index = UidToViewIndex( aUid );
        }

    if ( index >= 0 )
        {
        SetCurrentItemIndexAndDraw( index );
        return ETrue;
        }
    return EFalse;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::HandleMarkableListCommandL
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::HandleMarkableListCommandL( TInt aCommand )
    {
    if ( ListboxFlags() & EAknListBoxMarkableList )
        {
        TInt index;
        switch ( aCommand )
            {
            case EWmlCmdMark:
                {
                index = CurrentItemIndex();
                if ( index >= 0 )
                    {
                    View()->SelectItemL( index );
                    }
                break;
                }

            case EWmlCmdUnmark:
                {
                index = CurrentItemIndex();
                if ( index >= 0 )
                    {
                    View()->DeselectItem( index );
                    }
                break;
                }

            case EWmlCmdMarkAll:
                {
                ClearSelection();
                TInt count = Model()->NumberOfItems();
                if ( count )
                    {
                    TInt i = 0;
                    CArrayFixFlat<TInt>* selection =
                        new(ELeave) CArrayFixFlat<TInt>( 1 );
                    CleanupStack::PushL( selection );
                    selection->SetReserveL( count );
                    for ( i = 0; i < count; i++ )
                        {
                        selection->AppendL( i );
                        }
                    SetSelectionIndexesL( selection );
                    CleanupStack::PopAndDestroy();  // selection
                    }
                break;
                }

            case EWmlCmdUnmarkAll:
                {
                ClearSelection();
                break;
                }

            default:
                {
                Util::Panic( Util::EUnExpected );
                }
            }
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::MarkedUidsLC
// ---------------------------------------------------------
//
CArrayFix<TInt>* CBrowserFavouritesListbox::MarkedUidsLC() const
    {
    CArrayFix<TInt>* array = MarkedRealIndexesLC();
    RealIndexesToUids( *array );
    return array;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::SelectedUidsLC
// ---------------------------------------------------------
//
CArrayFix<TInt>* CBrowserFavouritesListbox::SelectedUidsLC() const
    {
    CArrayFix<TInt>* array = SelectedRealIndexesLC();
    RealIndexesToUids( *array );
    return array;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::SelectedItemsLC
// ---------------------------------------------------------
//
CArrayPtr<const CFavouritesItem>*
CBrowserFavouritesListbox::SelectedItemsLC(TBool aIncludeSpecialItems) const
    {
    CArrayPtrFlat<const CFavouritesItem>* itemPtrs =
        new (ELeave) CArrayPtrFlat<const CFavouritesItem>( KGranularity );
    CleanupStack::PushL( itemPtrs );
    CArrayFix<TInt>* indexes = SelectedRealIndexesLC();
    TInt i;
    TInt count = indexes->Count();
    for ( i = 0; i < count; i++ )
        {
        CFavouritesItem* item = iItems->At( indexes->At( i ) );

        if ( !aIncludeSpecialItems )
            {
            // Skip special items:
            // folders, homepage, last visited & start page URLs.
            // Need a workaround for the start page since CFavouritesItem
            // does not provide IsStartPage() or something similar.
            if ( (item->Uid() == KFavouritesHomepageUid ) ||
                (item->Uid() == KFavouritesLastVisitedUid ) ||
                item->IsFolder() ||
                item->Uid() == KFavouritesStartPageUid )
                    continue;
            }
        itemPtrs->AppendL( item );
        }
    CleanupStack::PopAndDestroy();  // indexes
    return itemPtrs;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::ItemByUid
// ---------------------------------------------------------
//
const CFavouritesItem* CBrowserFavouritesListbox::ItemByUid
( TInt aUid ) const
    {
    return iItems->ItemByUid( aUid );
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::UnfilteredNumberOfItems
// ---------------------------------------------------------
//
TInt CBrowserFavouritesListbox::UnfilteredNumberOfItems()
    {
    return iItems->Count();
    }


// ---------------------------------------------------------
// CBrowserFavouritesListbox::FilteredNumberOfItems
// ---------------------------------------------------------
//
TInt CBrowserFavouritesListbox::FilteredNumberOfItems()
    {

    CAknListBoxFilterItems* filter = TheModel()->Filter();
    return filter->FilteredNumberOfItems();
    }


// ---------------------------------------------------------
// CBrowserFavouritesListbox::UpdateFilterL
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::UpdateFilterL()
    {
    CAknListBoxFilterItems* filter = TheModel()->Filter();
    if ( filter )
        {
        filter->HandleItemArrayChangeL();
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::AnyFoldersL
// ---------------------------------------------------------
//
TBool CBrowserFavouritesListbox::AnyFoldersL()
    {
    TInt i;
    TInt count = iItems->Count();
    for ( i = 0; i < count; i++ )
        {
        if ( iItems->At( i )->IsFolder() )
            {
            if (iItems->At( i )->Uid()!=KFavouritesAdaptiveItemsFolderUid) return ETrue; //exclude Adaptive Bookmarks Folder
            }
        }
    return EFalse;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::OfferKeyEventL
// ---------------------------------------------------------
//
TKeyResponse CBrowserFavouritesListbox::OfferKeyEventL
( const TKeyEvent& aKeyEvent, TEventCode aType )
    {
    TKeyResponse result = EKeyWasNotConsumed;

    switch ( aKeyEvent.iCode )
        {
        case EKeyLeftUpArrow:     // Northwest
        case EStdKeyDevice10:     //   : Extra KeyEvent supports diagonal event simulator wedge
        case EKeyLeftArrow:       // West
        case EKeyLeftDownArrow:   // Southwest
        case EStdKeyDevice13:     //   : Extra KeyEvent supports diagonal event simulator wedge

        case EKeyRightUpArrow:    // Northeast
        case EStdKeyDevice11:     //   : Extra KeyEvent supports diagonal event simulator wedge
        case EKeyRightArrow:      // East
        case EKeyRightDownArrow:  // Southeast
        case EStdKeyDevice12:     //   : Extra KeyEvent supports diagonal event simulator wedge
            {
            // This listbox does not consume left/right or diagonal keypresses.
            // (The base class always does, even if it doesn't use them )
            break;
            }

        default:
            {
            result = CAknSingleGraphicStyleListBox::OfferKeyEventL
                ( aKeyEvent, aType );
            // Call UpdateCBA to update MSK in case of mark/unmark
            CBrowserAppUi::Static()->UpdateCbaL();
            }

        }

    return result;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::FocusChanged
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::FocusChanged( TDrawNow aDrawNow )
    {
    // Do nothing until the listbox is fully constructed
    // The dialogpage sets the focus before calling ConstructL
    if ( iView )
        {
        CAknSingleGraphicStyleListBox::FocusChanged( aDrawNow );
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::CreateModelL
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::CreateModelL()
    {
    iModel = CBrowserFavouritesListboxModel::NewL( *iItems, *iIconIndexes );
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::MakeViewClassInstanceL
// ---------------------------------------------------------
//
CListBoxView* CBrowserFavouritesListbox::MakeViewClassInstanceL()
    {
    return new (ELeave) CBrowserFavouritesListboxView( *this );
    }


// ---------------------------------------------------------
// CBrowserFavouritesListbox::CreateItemDrawerL
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::CreateItemDrawerL()
    {
    CAknSingleGraphicStyleListBox::CreateItemDrawerL();
    }


// ---------------------------------------------------------
// CBrowserFavouritesListbox::CBrowserFavouritesListbox
// ---------------------------------------------------------
//
CBrowserFavouritesListbox::CBrowserFavouritesListbox
                            ( MApiProvider& aApiProvider ) :
    iApiProvider( aApiProvider )
    {
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::ConstructL
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::ConstructL
        (
        const CCoeControl* aParent,
        const MBrowserFavouritesListboxIconHandler& aIconHandler
        )
    {
    iSkinUpdated = EFalse;
    iIconHandler = &aIconHandler;
    iItems = new (ELeave) CFavouritesItemList();
    iIconIndexes =
        new (ELeave) CBrowserFavouritesIconIndexArray( KGranularity );
    CAknSingleGraphicStyleListBox::ConstructL( aParent, ListboxFlags() );
    // MUST call this after the listbox has finished; listbox construction
    // is buggy and overwrites the model's itemarray ptr.
    TheModel()->SetItemArray();
    CreateScrollBarFrameL( ETrue );
    ScrollBarFrame()->SetScrollBarVisibilityL
        ( CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );

    // Load icons
    ItemDrawer()->ColumnData()->SetIconArray
        ( iIconHandler->CreateIconArrayL() );

    // Store the size of an icon
    TAknWindowComponentLayout layout =
                AknLayoutScalable_Avkon::list_single_graphic_pane_g1(0);

    TAknLayoutRect naviPaneGraphicsLayoutRect;
    TRect listBoxGraphicRect;
    TRect rect = iAvkonAppUi->ClientRect();


    naviPaneGraphicsLayoutRect.LayoutRect( rect, layout);
    listBoxGraphicRect = naviPaneGraphicsLayoutRect.Rect();

    TSize listBoxGraphicSize( listBoxGraphicRect.iBr.iX - listBoxGraphicRect.iTl.iX,
                                listBoxGraphicRect.iBr.iY - listBoxGraphicRect.iTl.iY );
    // Favicon handler
    iFaviconHandler = CBrowserFaviconHandler::NewL(
                                iApiProvider,
                                ItemDrawer()->ColumnData()->IconArray(),
                                *this,
                                listBoxGraphicSize );

    ItemDrawer()->ColumnData()->EnableMarqueeL( ETrue );


    // Setup italicized font for use later    
    const CFont* logicalFont = AknLayoutUtils::FontFromId(EAknLogicalFontPrimaryFont);
    
    // Note: This font is owned by the application's system font array (where it 
    // is likely already to have been created) and does not need to be
    // released. It can just go out of scope.// Extract font information
    TFontSpec fontSpec = logicalFont->FontSpecInTwips();// Desired height, weight, and posture already set in locals
    fontSpec.iFontStyle.SetPosture( EPostureItalic );// Obtain new font
    CWsScreenDevice& screenDev = *( CEikonEnv::Static()->ScreenDevice() );
    screenDev.GetNearestFontInTwips( ( CFont*& )iFontItalic, fontSpec );
 
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::ListboxFlags
// ---------------------------------------------------------
//
TInt CBrowserFavouritesListbox::ListboxFlags()
    {
    return AknLayoutUtils::PenEnabled() ?  
         EAknListBoxStylusMarkableList :  EAknListBoxMarkableList;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::GetIconIndexesLC
// ---------------------------------------------------------
//
CBrowserFavouritesIconIndexArray*
CBrowserFavouritesListbox::GetIconIndexesLC
( /*MCommsModel& aCommsModel, */CFavouritesItemList& aItems )
    {
    // Create new empty list.
    CBrowserFavouritesIconIndexArray* iconIndexes =
        new (ELeave) CBrowserFavouritesIconIndexArray( KGranularity );
    CleanupStack::PushL( iconIndexes );
    // Get list of access points. Not copy, owned by the AP model.
   // const CApListItemList* apList = aCommsModel.AccessPoints();
    // Fill the list.
    TInt i = 0;
    TInt count = aItems.Count();
    for ( i = 0; i < count; i++ )
        {
        iconIndexes->AppendL
            ( iIconHandler->IconIndexes( *(aItems.At( i ))/*, apList*/,this ) );
        }
    return iconIndexes;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::CalcNewStateL
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::CalcNewStateL
( CFavouritesItemList& aNewItems )
    {
    delete iNewState;
    iNewState = NULL;
    iNewState = new( ELeave ) CBrowserFavouritesListboxState;

    // 1. Check if we have any new item. If yes, highlight goes to first
    // new one.
    TInt i;
    TInt count = aNewItems.Count();
    for ( i = 0; i < count; i++ )
        {
        if ( iItems->UidToIndex( aNewItems.IndexToUid( i ) ) ==
                                                            KErrNotFound )
            {
            // New item, not found among the old ones.
            // Set highlight to that, and also becomes top item.
            iNewState->iHighlightUid = iNewState->iTopItemUid =
                aNewItems.IndexToUid( i );
            break;
            }
        }

    // 2. If there is no new item, get the new highlight. That is the old
    // one, if still exists, or the next remaining one after the
    // deleted old one(s).
    TInt uid;
    if ( iNewState->iHighlightUid == KFavouritesNullUid )
        {
        i = CurrentItemRealIndex();
        if ( i >= 0 )
            {
            // Have current item.
            // Now go down the old list, beginning from highlighted one;
            // find first item that still exists.
            count = iItems->Count();
            for ( /*current highlight index: i*/; i < count; i++ )
                {
                uid = iItems->At( i )->Uid();
                if ( aNewItems.UidToIndex( uid ) != KErrNotFound )
                    {
                    iNewState->iHighlightUid = uid;
                    break;
                    }
                }
            // Here we should have higlight uid, unless the deleted element(s)
            // were last. In that case, set last item highlighted (if there is
            // anything to highlight).
            if ( iNewState->iHighlightUid == KFavouritesNullUid &&
                                                            aNewItems.Count() )
                {
                iNewState->iHighlightUid = aNewItems.IndexToUid
                    ( aNewItems.Count() - 1 );
                }
            }
        }

    // 3. Get mark uids. Whether these uids exist in the new list or not, is
    // not checked here; setting marks to the already changed listbox is
    // fool-proof.
    iNewState->iMarkUids = MarkedUidsLC();
    CleanupStack::Pop();    // Uid list; ownership is now in the state.
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::SetStateL
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::SetStateL
( const CBrowserFavouritesListboxState& aState )
    {
    TInt topIndex;

    // Set marks.
    if ( aState.iMarkUids->Count() )
        {
        CArrayFix<TInt>* marks = new (ELeave) CArrayFixFlat<TInt>( 4 );
        CleanupStack::PushL( marks );
        marks->AppendL
            ( aState.iMarkUids->Back( 0 ), aState.iMarkUids->Count() );
        UidsToViewIndexes( *marks );
        SetSelectionIndexesL( marks );
        CleanupStack::PopAndDestroy();  // marks
        }

    // Set top item index.
    topIndex = UidToViewIndex ( aState.iTopItemUid );
    if ( topIndex >= 0 )
        {
        // Always try to fill the listbox (if there were empty lines below,
        // "scroll down", instead of strictly restoring the top item index).
        TInt topIndexToSeeLastItem = Model()->NumberOfItems() -
            View()->NumberOfItemsThatFitInRect( View()->ViewRect() );
        if ( topIndexToSeeLastItem < 0 )
            {
            topIndexToSeeLastItem = 0;
            }
        topIndex = Min( topIndex, topIndexToSeeLastItem );
        SetTopItemIndex( topIndex );
        }

    // Set higlight.
    TInt curIndex = UidToViewIndex ( aState.iHighlightUid );
    if ( curIndex < 0 )
        {
        // If cannot restore highlight, set it to top item.
        curIndex = topIndex;
        }
    if ( curIndex >= 0 )
        {
        SetCurrentItemIndex( curIndex );
        }

    DrawNow();

    if ( iCursorObserver )
        {
        // Cursor now points to some different item. Let the observer know it.
        iCursorObserver->HandleCursorChangedL( this );
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::UidToViewIndex
// ---------------------------------------------------------
//
TInt CBrowserFavouritesListbox::UidToViewIndex( TInt aUid ) const
    {
    TInt viewIndex = KErrNotFound;
    TInt realIndex = iItems->UidToIndex( aUid );
    if ( realIndex >= 0 )
        {
        CAknListBoxFilterItems* filter = TheModel()->Filter();
        if ( filter )
            {
            // Have filter, do the conversion.
            viewIndex = filter->VisibleItemIndex( realIndex );
            }
        else
            {
            // No filter, no conversion necessary.
            viewIndex = realIndex;
            }
        }
    return viewIndex;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::UidsToViewIndexes
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::UidsToViewIndexes
( CArrayFix<TInt>& aList ) const
    {
    TInt i;
    TInt j;
    TInt index;
    TInt count = aList.Count();
    for ( i = 0, j = 0; i < count; i++ )
        {
        index = UidToViewIndex( aList.At ( i ) );
        if ( index != KErrNotFound )
            {
            aList.At( j++ ) = index;
            }
        }
    if ( j < count )
        {
        // Not all Uids were converted. Delete the trailing rubbish.
        aList.Delete( j, count - j );
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::RealIndexesToUids
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::RealIndexesToUids
( CArrayFix<TInt>& aList ) const
    {
    TInt i;
    TInt uid;
    TInt count = aList.Count();
    for ( i = 0; i < count; i++ )
        {
        uid = iItems->IndexToUid( aList.At( i ) );
        __ASSERT_DEBUG( uid != KFavouritesNullUid, \
            Util::Panic( Util::EFavouritesBadBookmarkUid ) );
        aList.At( i ) = uid;
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::MarkedRealIndexesLC
// ---------------------------------------------------------
//
CArrayFix<TInt>* CBrowserFavouritesListbox::MarkedRealIndexesLC() const
    {
    CListBoxView::CSelectionIndexArray* array =
        new (ELeave) CArrayFixFlat<TInt>( KGranularity );
    CleanupStack::PushL( array );
    if ( iItems->Count() )
        {
        const CArrayFix<TInt>* marked;
        CAknListBoxFilterItems* filter = TheModel()->Filter();
        if ( filter )
            {
            // Filter knows all.
            filter->UpdateSelectionIndexesL();
            marked = filter->SelectionIndexes();
#if 1 /* TODO remove when AVKON bug fixed*/
            // Overcoming AVKON bug. If filter criteria is empty (i.e. the
            // user is not typing in the filter), selection indexes between
            // the filter and the listbox are not synchronized! Unfortunately,
            // this strange back-way is the only means by we can discover if
            // we have criteria or not; CAknListBoxFilterItems has the
            // criteria as private; and the CAknSearchField, which holds the
            // filter (and has the string) is not accessible here in the
            // listbox (it's in the container).
            if ( filter->FilteredNumberOfItems() ==
                filter->NonFilteredNumberOfItems() )
                {
                marked = View()->SelectionIndexes();
                }
#endif
            }
        else
            {
            // No filter.
            marked = View()->SelectionIndexes();
            }
        AppendArrayL( /*aTarget=*/*array, /*aSource=*/*marked );
        }
    return array;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::SelectedRealIndexesLC
// ---------------------------------------------------------
//
CArrayFix<TInt>* CBrowserFavouritesListbox::SelectedRealIndexesLC() const
    {
    CListBoxView::CSelectionIndexArray* selection =
        MarkedRealIndexesLC();
    if ( selection->Count() == 0 )
        {
        // No marks; use the highlighted one.
        selection->AppendL( CurrentItemRealIndex() );
        }
    return selection;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::CurrentItemRealIndex
// ---------------------------------------------------------
//
TInt CBrowserFavouritesListbox::CurrentItemRealIndex() const
    {
    return RealIndex( CurrentItemIndex() /*that's view index*/ );
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::RealIndex
// ---------------------------------------------------------
//
TInt CBrowserFavouritesListbox::RealIndex( TInt aViewIndex ) const
    {
    TInt index = KErrNotFound;
    if ( aViewIndex >= 0 )
        {
        CAknListBoxFilterItems* filter = TheModel()->Filter();
        if ( filter )
            {
            // If we have the index and the filter also, do the conversion.
            index = filter->FilteredItemIndex( aViewIndex );
            }
        else
            {
            index = aViewIndex;
            }
        }
    return index;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::RealIndexes
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::RealIndexes( CArrayFix<TInt>& aList ) const
    {
    TInt i;
    TInt j;
    TInt index;
    TInt count = aList.Count();
    for ( i = 0, j = 0; i < count; i++ )
        {
        index = RealIndex( aList.At ( i ) );
        if ( index != KErrNotFound )
            {
            aList.At( j++ ) = index;
            }
        }
    if ( j < count )
        {
        // Not all indexes were converted. Delete the trailing rubbish.
        aList.Delete( j, count - j );
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::SelectionFlags
// ---------------------------------------------------------
//
TInt CBrowserFavouritesListbox::SelectionFlags
( const CFavouritesItem& aItem ) const
    {
    TInt flags = 0;
    if ( aItem.IsItem() )
        {
        switch ( aItem.Uid() )
            {
            case KFavouritesHomepageUid:
                {
                flags |=
                    TBrowserFavouritesSelectionState::EAny |
                    TBrowserFavouritesSelectionState::EHomepage |
                    TBrowserFavouritesSelectionState::EItem;
                break;
                }

            case KFavouritesStartPageUid:
                {
                flags |=
                    TBrowserFavouritesSelectionState::EAny |
                    TBrowserFavouritesSelectionState::EStartPage |
                    TBrowserFavouritesSelectionState::EItem;
                break;
                }

            case KFavouritesLastVisitedUid:
                {
                flags |=
                    TBrowserFavouritesSelectionState::EAny |
                    TBrowserFavouritesSelectionState::ELastVisited |
                    TBrowserFavouritesSelectionState::EItem;
                break;
                }

            default:
                {
                flags |=
                    TBrowserFavouritesSelectionState::EAny |
                    TBrowserFavouritesSelectionState::EPlainItem |
                    TBrowserFavouritesSelectionState::EItem;
                break;
                }
            }
        }
    else
        {
        flags |=
            TBrowserFavouritesSelectionState::EAny |
            TBrowserFavouritesSelectionState::EFolder;
        }
    return flags;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::TheModel
// ---------------------------------------------------------
//
CBrowserFavouritesListboxModel*
CBrowserFavouritesListbox::TheModel() const
    {
    return REINTERPRET_CAST( CBrowserFavouritesListboxModel*, Model() );
    }


// ---------------------------------------------------------
// CBrowserFavouritesListbox::HandleResourceChange
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::HandleResourceChange( TInt aType )
    {
    CAknSingleGraphicStyleListBox::HandleResourceChange( aType );
    if ( aType == KAknsMessageSkinChange )
        {
        CArrayPtr<CGulIcon>* array =
        ItemDrawer()->ColumnData()->IconArray();

        array->ResetAndDestroy();
        delete array;

        CAknIconArray* iconArray = NULL;
        TRAPD( err,
                iconArray = iIconHandler->CreateIconArrayL()
                )

        if ( !err )
            {
            ItemDrawer()->ColumnData()->SetIconArray( iconArray );
            iFaviconHandler->UpdateIconArray(ItemDrawer()->ColumnData()->IconArray());
            SetSkinUpdated(ETrue);
            }
        }
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::SetSkinUpdated
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::SetSkinUpdated(TBool aSkinUpdated)
    {
    iSkinUpdated = aSkinUpdated;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListbox::ItalicizeRowItemL
// ---------------------------------------------------------
//
void CBrowserFavouritesListbox::ItalicizeRowItemL(TInt aRowIndex)
    {
    if (ItemDrawer() && ItemDrawer()->ColumnData() && iFontItalic)
        {
        ItemDrawer()->ColumnData()->SetColumnFontForRowL( aRowIndex, 1, iFontItalic );
        }
    }

//  End of File
