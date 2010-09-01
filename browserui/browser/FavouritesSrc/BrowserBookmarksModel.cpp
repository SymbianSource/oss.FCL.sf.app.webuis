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
*      Implementation of CWmlBrowserBookmarksModel.
*      
*
*/


// INCLUDE FILES

#include "BrowserBookmarksModel.h"
#include "BrowserUtil.h"
#include <BrowserNG.rsg>
#include <favouritesitem.h>
#include <favouritesitemlist.h>
#include <StringLoader.h>
#include <FeatMgr.h>
#include "ApiProvider.h"
#include "Preferences.h"
#include "CommonConstants.h"


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserBookmarksModel::CBrowserBookmarksModel
// ---------------------------------------------------------
//
CBrowserBookmarksModel::CBrowserBookmarksModel( MApiProvider& aApiProvider )
	: CBrowserFavouritesModel( aApiProvider ), iApiProvider( aApiProvider )
	{
	}
	
// ---------------------------------------------------------
// CBrowserBookmarksModel::ApiProvider
// ---------------------------------------------------------
//
MApiProvider& CBrowserBookmarksModel::ApiProvider()
    {
    return iApiProvider;
    }

// ---------------------------------------------------------
// CBrowserBookmarksModel::NewL
// ---------------------------------------------------------
//
CBrowserBookmarksModel* CBrowserBookmarksModel::NewL( MApiProvider& aApiProvider )
	{
	CBrowserBookmarksModel* model =
        new (ELeave) CBrowserBookmarksModel( aApiProvider );
	CleanupStack::PushL( model );
	model->ConstructL();
	CleanupStack::Pop();
	return model;
	}

// ---------------------------------------------------------
// CBrowserBookmarksModel::StringResourceId
// ---------------------------------------------------------
//
TInt CBrowserBookmarksModel::StringResourceId
        (
        const CFavouritesItem& aItem,
        CBrowserFavouritesModel::TTextResourceType aType
        ) const
    {
    switch ( aType )
        {
        case CBrowserFavouritesModel::EDefaultName:
            {
            if ( aItem.IsFolder() )
                {
                return R_BROWSER_FAVOURITES_TEXT_FLDR_NAME_DEFAULT;
                }
            else
                {
                return R_BROWSER_BOOKMARKS_TEXT_DEFAULT_BM_TITLE;
                }
            }

        case CBrowserFavouritesModel::ERenamePrompt:
            {
            if ( aItem.IsFolder() )
                {
                return R_BROWSER_FAVOURITES_TEXT_FLDR_ITEM_NAME_PRMPT;
                }
            else
                {
                return R_BROWSER_FAVOURITES_TEXT_PRMPT_EDIT_NAME;
                }
            }

        case CBrowserFavouritesModel::ENewPrompt:
            {
            if ( aItem.IsFolder() )
                {
                return R_BROWSER_FAVOURITES_TEXT_FLDR_NAME_PRMPT;
                }
            else
                {
                return R_BROWSER_FAVOURITES_TEXT_PRMPT_EDIT_NAME;
                }
            }

        case CBrowserFavouritesModel::ESaved:
            {
            return R_BROWSER_BOOKMARKS_TEXT_BM_SAVED;
            }

        case CBrowserFavouritesModel::ENotSaved:
            {
            return R_BROWSER_BOOKMARKS_TEXT_BM_NOT_SAVED;
            }

        default:
            {
            break;
            }
        }

    Util::Panic( Util::EUnExpected );
    /*NOTREACHED*/
    return 0;
    }

// ---------------------------------------------------------
// CBrowserBookmarksModel::GetFoldersSortedLC
// ---------------------------------------------------------
//
CFavouritesItemList* CBrowserBookmarksModel::GetFoldersSortedLC()
    {
    CFavouritesItemList* folders = new (ELeave) CFavouritesItemList();
    CleanupStack::PushL( folders );

    if ((ADAPTIVEBOOKMARKS)&&(ApiProvider().Preferences().AdaptiveBookmarks()==EWmlSettingsAdaptiveBookmarksOn ))
        {
        CFavouritesItem* adaptiveItemsFolder = Database().CreateAdaptiveItemsFolderL();
        CleanupStack::PushL(adaptiveItemsFolder);
        HBufC* folderName=StringLoader::LoadLC(R_BROWSER_ADAPTIVE_BOOKMARKS_FOLDER);
        adaptiveItemsFolder->SetNameL(*folderName);
        CleanupStack::PopAndDestroy();//folderName
        folders->AppendL( adaptiveItemsFolder );
        CleanupStack::Pop();    // adaptiveItemsFolder: owner is the list now.
        }

    // Get all folders.
    Database().GetAll( *folders, KFavouritesRootUid, CFavouritesItem::EFolder );
    // Sort folders,
    CBrowserBookmarksOrder* currentOrder = CBrowserBookmarksOrder::NewLC();
    CArrayFixFlat<TInt>* orderArray = new (ELeave) CArrayFixFlat<TInt>(KGranularityHigh);
    CleanupStack::PushL(orderArray);
    if( Database().GetData( KFavouritesRootUid ,*currentOrder ) == KErrNone )
        {
        if( currentOrder->GetBookMarksOrder().Count() > 0 )
            {
            orderArray->AppendL( &( currentOrder->GetBookMarksOrder()[0] ), currentOrder->GetBookMarksOrder().Count());
            ManualSortL(KFavouritesRootUid, currentOrder, orderArray, folders);
            }
        }
    CleanupStack::PopAndDestroy(2); //currentOrder, orderArray

    return folders;
    }

// ---------------------------------------------------------
// CBrowserBookmarksModel::GetDBName
// ---------------------------------------------------------
//
const TDesC& CBrowserBookmarksModel::GetDBName()
    {
    return KBrowserBookmarks; 
    }

// End of File
