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
*      Implementation of CBrowserFavouritesListboxModel.
*      
*
*/


// INCLUDE FILES

#include <eiklbx.h>
#include <calslbs.h>
#include <FavouritesItem.h>
#include <FavouritesItemList.h>
#include "BrowserFavouritesListboxModel.h"
#include "BrowserFavouritesIconIndexArray.h"
#include "BrowserUtil.h"

// ================= LOCAL FUNCTIONS =======================

/**
* Copy aSOurce to aTarget, replacing TAB characters with spaces.
* @param aSource Source descriptor.
* @param aTarget Target descriptor.
*/
LOCAL_C void RemoveTabs( const TDesC& aSource, TDes& aTarget )
    {
    TChar ch;
    TInt i;
    TInt length = aSource.Length();
    // Initialize target to empty string.
    aTarget.SetLength( 0 );
    // Copy source to target, replace whitespace with space.
    for ( i = 0; i < length; i++ )
        {
        ch = aSource[i];
        if ( ch == '\t' )
            {
            ch = ' ';
            }
        aTarget.Append( ch );
        }
    }

// ================= MEMBER FUNCTIONS =======================
// ====== TBrowserFavouritesListboxFilterTextArray =======

// ---------------------------------------------------------
// TBrowserFavouritesListboxItemTextArray::MdcaCount
// ---------------------------------------------------------
//
TInt TBrowserFavouritesListboxItemTextArray::MdcaCount() const
    {
    return iItems->Count();
    }

// ---------------------------------------------------------
// TBrowserFavouritesListboxItemTextArray::MdcaPoint
// ---------------------------------------------------------
//
TPtrC TBrowserFavouritesListboxItemTextArray::MdcaPoint
( TInt aIndex ) const
    {
    // Sorry, need to cast away the const-ness from the buffer.
    // It was made definitely for formatting! Odd that this method
    // is const.
    CFavouritesItem* item = iItems->At( aIndex );
    TBrowserFavouritesIconIndexes iconIndexes = iIconIndexes->At( aIndex );

    TBuf<KFavouritesMaxName> name;

	RemoveTabs( item->Name(), name );

    if ( iconIndexes.iBearerIcon == KBrowserFavouritesNoBearerIcon )
        {
        _LIT( KFormatNoBearer, "%d\t%S\t\t" );
        MUTABLE_CAST( TBuf<KMaxFavouritesItemListboxName>&, iBuf 
            ). //lint !e665 expression macro param ok
            Format
                (
                KFormatNoBearer,
                iconIndexes.iItemIcon,
                &name
                );
        }
    else
        {
        _LIT( KFormatWithBearer, "%d\t%S\t%d\t" );
        MUTABLE_CAST( TBuf<KMaxFavouritesItemListboxName>&, iBuf 
            ). //lint !e665 expression macro param ok
            Format
                (
                KFormatWithBearer,
                iconIndexes.iItemIcon,
                &name,
                iconIndexes.iBearerIcon
                );
        }
    return iBuf;
    }

// ================= MEMBER FUNCTIONS =======================
// ====== TBrowserFavouritesListboxFilterTextArray =======

// ---------------------------------------------------------
// TBrowserFavouritesListboxFilterTextArray::MdcaCount
// ---------------------------------------------------------
//
TInt TBrowserFavouritesListboxFilterTextArray::MdcaCount() const
    {
    return iItems->Count();
    }

// ---------------------------------------------------------
// TBrowserFavouritesListboxFilterTextArray::MdcaPoint
// ---------------------------------------------------------
//
TPtrC TBrowserFavouritesListboxFilterTextArray::MdcaPoint
( TInt aIndex ) const
    {
    return iItems->At( aIndex )->Name();
    }

// ================= MEMBER FUNCTIONS =======================
// =========== CBrowserFavouritesListboxModel ============

// ---------------------------------------------------------
// CBrowserFavouritesListboxModel::NewL
// ---------------------------------------------------------
//
CBrowserFavouritesListboxModel* CBrowserFavouritesListboxModel::NewL
        (
        const CFavouritesItemList& aItems,
        const CBrowserFavouritesIconIndexArray& aIconIndexes
        )
    {
    CBrowserFavouritesListboxModel* model =
        new (ELeave) CBrowserFavouritesListboxModel( aItems, aIconIndexes );
    CleanupStack::PushL( model );
    model->ConstructL();
    CleanupStack::Pop();    // model
    return model;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListboxModel::SetItemArray
// ---------------------------------------------------------
//
void CBrowserFavouritesListboxModel::SetItemArray()
    {
    // Need to set them after the listbox has been constructed; otherwise
    // listbox construction will reset the item text array to NULL.
    SetItemTextArray( &iItemTexts );
    SetOwnershipType( ELbmDoesNotOwnItemArray ); 
    }

// ---------------------------------------------------------
// CBrowserFavouritesListboxModel::~CBrowserFavouritesListboxModel
// ---------------------------------------------------------
//
CBrowserFavouritesListboxModel::~CBrowserFavouritesListboxModel()
    {
    }

// ---------------------------------------------------------
// CBrowserFavouritesListboxModel::SetData
// ---------------------------------------------------------
//
void CBrowserFavouritesListboxModel::SetData
        (
        const CFavouritesItemList& aItems,
        const CBrowserFavouritesIconIndexArray& aIconIndexes
        )
    {
    iItems = &aItems;
    iIconIndexes = &aIconIndexes;
    __ASSERT_DEBUG( iItems->Count() == iIconIndexes->Count(), \
        Util::Panic( Util::EFavouritesBadIconIndexArray ) );
    // Propagate data to formatting classes.
    iItemTexts.iItems = iItems;
    iItemTexts.iIconIndexes = iIconIndexes;
    iFilterTexts.iItems = iItems;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListboxModel::MatchableTextArray
// ---------------------------------------------------------
//
const MDesCArray* CBrowserFavouritesListboxModel::MatchableTextArray() const
    {
    return &iFilterTexts;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListboxModel::CBrowserFavouritesListboxModel
// ---------------------------------------------------------
//
CBrowserFavouritesListboxModel::CBrowserFavouritesListboxModel
        (
        const CFavouritesItemList& aItems,
        const CBrowserFavouritesIconIndexArray& aIconIndexes
        )
: CAknFilteredTextListBoxModel(), iItems( &aItems ),
  iIconIndexes( &aIconIndexes )
    {
    __ASSERT_DEBUG( iItems->Count() == iIconIndexes->Count(), \
        Util::Panic( Util::EFavouritesBadIconIndexArray ) );
    }

// ---------------------------------------------------------
// CBrowserFavouritesListboxModel::ConstructL
// ---------------------------------------------------------
//
void CBrowserFavouritesListboxModel::ConstructL()
    {
    // Propagate data to formatting classes.
    iItemTexts.iItems = iItems;
    iItemTexts.iIconIndexes = iIconIndexes;
    iFilterTexts.iItems = iItems;
    // Base class method already called by listbox. Nasty buggy mess.
    }

// End of File
