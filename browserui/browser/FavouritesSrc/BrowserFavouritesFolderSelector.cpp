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
*      Implementation of CBrowserFavouritesFolderSelector.
*
*
*/


// INCLUDE FILES

#include <AknLists.h>
#include <AknPopup.h>
#include <gulicon.h>
#include <AknIconArray.h>
#include <FavouritesDb.h>
#include <FavouritesItem.h>
#include <FavouritesItemList.h>
#include <BrowserNG.rsg>
#include <AknsConstants.h>
#include "BrowserFavouritesFolderSelector.h"
#include "BrowserFavouritesModel.h"
#include "BrowserFavouritesIconIndexArray.h"
#include "BrowserUtil.h"
#include "ApiProvider.h"
#include "CommsModel.h"
#include "CommonConstants.h"
#include "BrowserFavouritesView.h"

// CONSTANTS

/**
* Granularity of the listbox item array.
*/
LOCAL_C const TInt KGranularity = 4;

/**
* Buffer size for formatting listbox text. Maximum item name
* plus listbox internals (tabulators for icons etc.) must fit into it.
* The format is "<icon index max 3 char>\t<name>",
* so the added length is 4.
*/
LOCAL_C const TInt KMaxListboxLineLen = KFavouritesMaxName + 4;

/**
* Format for formatting a listbox line.
*/
_LIT( KListboxLineFormat, "%d\t%S" );

// ---------------------------------------------------------
// CBrowserFavouritesFolderSelectorListbox::CBrowserFavouritesFolderSelectorListbox
// ---------------------------------------------------------
//
CBrowserFavouritesFolderSelectorListbox::CBrowserFavouritesFolderSelectorListbox( const TBrowserFavouritesFolderListboxIconHandler* aIconHandler ) :
 CAknSingleGraphicPopupMenuStyleListBox()
    {
    iIconHandler = aIconHandler;
    }

// ---------------------------------------------------------
// CBrowserFavouritesFolderSelectorListbox::HandleResourceChange
// ---------------------------------------------------------
//
void CBrowserFavouritesFolderSelectorListbox::HandleResourceChange( TInt aType )
{
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
            }
        delete iconArray;
        }
	return;
}

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserFavouritesFolderSelector::NewL
// ---------------------------------------------------------
//
CBrowserFavouritesFolderSelector* CBrowserFavouritesFolderSelector::NewL
        (
        CBrowserFavouritesModel& aModel,
        MApiProvider& aApiProvider,
        TInt aExcludeFolder
        )
    {
    CBrowserFavouritesFolderSelector* selector =
        new (ELeave) CBrowserFavouritesFolderSelector
        ( aModel, aApiProvider, aExcludeFolder );
    CleanupStack::PushL( selector );
    selector->ConstructL();
    CleanupStack::Pop();    // selector
    return selector;
    }

// ---------------------------------------------------------
// CBrowserFavouritesFolderSelector::CBrowserFavouritesFolderSelector
// ---------------------------------------------------------
//
CBrowserFavouritesFolderSelector::CBrowserFavouritesFolderSelector
        (
        CBrowserFavouritesModel& aModel,
        MApiProvider& aApiProvider,
        TInt aExcludeFolder
        )
: iModel( &aModel ),
  iApiProvider( &aApiProvider ),
  iExcludeFolder( aExcludeFolder )
    {
    }

// ---------------------------------------------------------
// CBrowserFavouritesFolderSelector::~CBrowserFavouritesFolderSelector
// ---------------------------------------------------------
//
CBrowserFavouritesFolderSelector::~CBrowserFavouritesFolderSelector()
    {
    delete iListboxItemArray;
    delete iListbox;
    delete iIconIndexes;
    delete iItems;
    }

// ---------------------------------------------------------
// CBrowserFavouritesFolderSelector::ConstructL
// ---------------------------------------------------------
//
void CBrowserFavouritesFolderSelector::ConstructL()
    {
    iListbox = new (ELeave) CBrowserFavouritesFolderSelectorListbox( &iIconHandler );
	iPopupList = CAknPopupList::NewL
        ( iListbox, R_BROWSER_FAVOURITES_CBA_MOVETOFOLDER_MOVECANCEL_MOVE );
    HBufC* title = CCoeEnv::Static()->AllocReadResourceLC
        ( R_BROWSER_FAVOURITES_TEXT_MOVE_TO_PRMPT );
    iPopupList->SetTitleL( *title );
    CleanupStack::PopAndDestroy();  // title
    iListbox->ConstructL
        ( iPopupList, EAknListBoxSelectionList | EAknListBoxLoopScrolling );
    iListboxItemArray = new (ELeave) CDesCArrayFlat( KGranularity );
    CTextListBoxModel* listboxModel = iListbox->Model();
    listboxModel->SetItemTextArray( iListboxItemArray );
    listboxModel->SetOwnershipType( ELbmDoesNotOwnItemArray );
    iListbox->CreateScrollBarFrameL( ETrue );
    iListbox->ScrollBarFrame()->SetScrollBarVisibilityL
        ( CEikScrollBarFrame::EOff, CEikScrollBarFrame::EAuto );
    iListbox->ItemDrawer()->ColumnData()->SetIconArray
        ( iIconHandler.CreateIconArrayL() );
	iListbox->ItemDrawer()->ColumnData()->EnableMarqueeL( ETrue );
    }

// ---------------------------------------------------------
// CBrowserFavouritesFolderSelector::ExecuteLD
// ---------------------------------------------------------
//
TInt CBrowserFavouritesFolderSelector::ExecuteLD()
    {
    TInt uid = KFavouritesNullUid;
    GetDataL();
    FillListboxL();
    if ( iPopupList->ExecuteLD() )
        {
        uid = iItems->IndexToUid( iListbox->View()->CurrentItemIndex() );
        }
    iPopupList = NULL;  // ExecuteLD has already deleted it.
    delete this;
    return uid;
    }

// ---------------------------------------------------------
// CBrowserFavouritesFolderSelector::GetDataL
// ---------------------------------------------------------
//
void CBrowserFavouritesFolderSelector::GetDataL()
    {
    // Getting data. Make sure that the item and icon index data does not
    // go out of sync - only set them to members when both is gotten.
    CFavouritesItemList* items = GetItemsLC();
    CBrowserFavouritesIconIndexArray* iconIndexes =
        GetIconIndexesLC( *items );
    // Replace data with new.
    delete iIconIndexes;                            // Not NULL-ed, because...
    delete iItems;                                  // Not NULL-ed, because...
    iItems = items;                                 // ... this cannot leave
    iIconIndexes = iconIndexes;                     // ... this cannot leave
    CleanupStack::Pop( 2 ); // iconIndexes, items: now members.
    }

// ---------------------------------------------------------
// CBrowserFavouritesFolderSelector::FillListboxL
// ---------------------------------------------------------
//
void CBrowserFavouritesFolderSelector::FillListboxL()
    {
    TBuf<KMaxListboxLineLen> buf;
    TBuf<KMaxListboxLineLen> name;
    TInt i;
    TInt resId = 0;

    iListboxItemArray->Reset();

    for ( i = 0; i < iItems->Count(); i++ )
        {
            if(!iItems->At(i)->IsHidden())
            {
        
                resId = CBrowserFavouritesView::GetSeamlessFolderResourceID( 
                            iItems->At(i)->ContextId() );

               if(resId) // a seamless link folder. Get the localised name.
                    {
                    HBufC* seamlessLinkName = CCoeEnv::Static()->AllocReadResourceLC( resId );
                    name = seamlessLinkName->Des();
                    CleanupStack::PopAndDestroy(); // seamlessLinkName
                    }
                else // not a seamless link folder.
                    {
                    name = iItems->At( i )->Name();
                    // Replace TAB characters with spaces.
                    for ( TInt i = 0; i < name.Length(); i++ )
                        {
                        if ( name[i] == '\t' )
                            {
                            name[i] = ' ';
                            }
                        }
                    }

                buf.Format( KListboxLineFormat, iIconIndexes->At( i ).iItemIcon, &name );
                iListboxItemArray->AppendL( buf );
                }
            else
                {
                iItems->Delete(i);
                i--;
                }
            }
    }

// ---------------------------------------------------------
// CBrowserFavouritesFolderSelector::GetItemsLC
// ---------------------------------------------------------
//
CFavouritesItemList* CBrowserFavouritesFolderSelector::GetItemsLC()
    {
    CFavouritesItemList* items = new (ELeave) CFavouritesItemList();
    CleanupStack::PushL( items );

    if ( iModel->BeginL( /*aWrite=*/EFalse, /*aDbErrorNote=*/ETrue )
                                                                == KErrNone )
        {
        // Get all folders.
        iModel->Database().GetAll
            ( *items, KFavouritesNullUid, CFavouritesItem::EFolder );
        iModel->CommitL();

        // Remove iExcludeFolder.
        TInt index = items->UidToIndex( iExcludeFolder );
        if ( index >= 0 )
            {
            items->Delete( index );
            }

        // Set custom localized name for root.
        index = items->UidToIndex( KFavouritesRootUid );
        if ( index >= 0 )
            {
            HBufC* rootName = CCoeEnv::Static()->AllocReadResourceLC
                ( R_BROWSER_FAVOURITES_TEXT_FLDR_ROOT_LEVEL );
            items->At( index )->SetNameL( *rootName );
            CleanupStack::PopAndDestroy();  // rootName
            }

        CArrayFixFlat<TInt>* orderArray = new (ELeave) CArrayFixFlat<TInt>( KGranularityHigh );
        CleanupStack::PushL( orderArray );
        CBrowserBookmarksOrder* currentOrder = CBrowserBookmarksOrder::NewLC();;
        if ( iModel->Database().GetData( KFavouritesRootUid ,*currentOrder ) == KErrNone)
            {
            if ( currentOrder->GetBookMarksOrder().Count() > 0 )
                {
                orderArray->AppendL( &( currentOrder->GetBookMarksOrder()[0] ), currentOrder->GetBookMarksOrder().Count());
                orderArray->InsertL( 0 , KFavouritesRootUid );
                iModel->ManualSortL( KFavouritesRootUid, currentOrder, orderArray, items );
                }
            }
        else
            {
            iModel->SortL( *items );
            }
        CleanupStack::Pop(2);// orderArray, currentOrder
        }

    return items;
    }

// ---------------------------------------------------------
// CBrowserFavouritesFolderSelector::GetIconIndexesLC
// ---------------------------------------------------------
//
CBrowserFavouritesIconIndexArray*
CBrowserFavouritesFolderSelector::GetIconIndexesLC
( CFavouritesItemList& aItems )
    {
    // Create new empty list.
    CBrowserFavouritesIconIndexArray* iconIndexes =
        new (ELeave) CBrowserFavouritesIconIndexArray( KGranularity );
    CleanupStack::PushL( iconIndexes );
    // Get list of access points. Not copy, owned by the AP model.
   // const CApListItemList* apList = iApiProvider->CommsModel().AccessPoints();
    // Fill the list.
    TInt i = 0;
    TInt count = aItems.Count();
    for ( i = 0; i < count; i++ )
        {
        iconIndexes->AppendL
            ( iIconHandler.IconIndexes(  *(aItems.At( i ) ) ) );
        }
    return iconIndexes;
    }

// End of File
