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
*      Implementation of TWmlBrowserFavouritesFolderListboxIconHandler.
*      
*
*/


// INCLUDE FILES
#include <BrowserNG.rsg>
#include <FavouritesItem.h>
#include "BrowserFavouritesFolderListboxIconHandler.h"
#include "BrowserFavouritesIconIndexArray.h"
#include "BrowserFavouritesView.h"
#include <AknsUtils.h>
#include <avkon.mbg>
#include <gulicon.h>
#include <AknIconArray.h>
#include <browser.mbg>

#include "CommonConstants.h"
#include <data_caging_path_literals.hrh>

// CONSTANTS

// MACROS

// LOCAL TYPES

/**
* Icon indexes. Numeric values MUST match the resource definition order.
*/
enum
    {
    EIconIndexListboxMark = 0,			///< Listbox mark icon index.
    EIconIndexFolder = 1,				///< Folder icon index.
    EIconIndexRootFolder = 2,			///< Root folder icon index.
	EIconIndexHomepage = 3,				///< Homepage icon index.
    EIconIndexLastVisitedPage = 4,		///< Last Visited icon index.
    EIconIndexBookmark = 5,				///< Bookmark icon index.
    EIconIndexCsdBearer = 6,			///< CSD bearer icon index.
    EIconIndexHscsdBearer = 7,			///< HSCSD bearer icon index.
    EIconIndexGprsBearer = 8,			///< GPRS bearer icon index.
	EIconIndexSeamlessLinksFolder = 9	///< Seamless links icon index
    };

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// TBrowserFavouritesFolderListboxIconHandler::IconArrayResourceId
// ---------------------------------------------------------
//
TInt TBrowserFavouritesFolderListboxIconHandler::IconArrayResourceId() const
    {
    return R_BROWSER_FAVOURITES_FOLDER_LISTBOX_ICONS;
    }

//-----------------------------------------------------------------------------
// MBrowserFavouritesListboxIconHandler::IconIndexes
//-----------------------------------------------------------------------------
//
TBrowserFavouritesIconIndexes
    TBrowserFavouritesFolderListboxIconHandler::IconIndexes( 
                                    const CFavouritesItem& aItem ) const
    {
    TBrowserFavouritesIconIndexes indexes;
     indexes.iItemIcon = ItemIconIndex( aItem );
     indexes.iBearerIcon = KBrowserFavouritesNoBearerIcon; //Bearer icons removed
     return indexes;
    }

// ---------------------------------------------------------
// TBrowserFavouritesFolderListboxIconHandler::ItemIconIndex
// ---------------------------------------------------------
//
TInt TBrowserFavouritesFolderListboxIconHandler::ItemIconIndex
( const CFavouritesItem& aItem,CBrowserFavouritesListbox* /*aListbox*/  ) const
    {
    return ItemIconIndex(aItem);
    }

// ---------------------------------------------------------
// TBrowserFavouritesFolderListboxIconHandler::ItemIconIndex
// ---------------------------------------------------------
//
TInt TBrowserFavouritesFolderListboxIconHandler::ItemIconIndex
( const CFavouritesItem& aItem ) const
    {
	if ( aItem.Uid() == KFavouritesRootUid )
		{
        return EIconIndexRootFolder;
		}
	if ( aItem.Uid() == KFavouritesHomepageUid )
        {
        return EIconIndexHomepage;
        }
    if ( aItem.Uid() == KFavouritesStartPageUid )
        {
        return EIconIndexHomepage;
        }
    if ( aItem.Uid() == KFavouritesLastVisitedUid )
        {
        return EIconIndexLastVisitedPage;
        }

   	//seamless links folders have a different icon
   	if( CBrowserFavouritesView::IsSeamlessFolder( aItem.ContextId() ) )
   		{
   		return EIconIndexSeamlessLinksFolder;
   		}
	
	return aItem.IsItem() ? EIconIndexBookmark : EIconIndexFolder;        
    }


// ---------------------------------------------------------
// TBrowserFavouritesFolderListboxIconHandler::LoadIconsL
// ---------------------------------------------------------
//
CAknIconArray* TBrowserFavouritesFolderListboxIconHandler::LoadIconsL() const
{
    CAknIconArray* icons = 
    new(ELeave) CAknIconArray( 20 );
    CleanupStack::PushL(icons); // PUSH Icon
	CGulIcon* newIcon;
	CFbsBitmap *newIconBmp;
	CFbsBitmap *newIconMaskBmp;

    TBuf<KMaxFileName> iconFileAvkon= KAvkonMbmPath();
    TParse* fp = new(ELeave) TParse(); 
    CleanupStack::PushL(fp);
    TInt err = fp->Set(KBrowserDirAndFile, &KDC_APP_BITMAP_DIR, NULL); 
    if (err != KErrNone)
        {
        User::Leave(err);
        }
    TBuf<KMaxFileName> iconFileBrowser= fp->FullName();
    CleanupStack::PopAndDestroy(fp);
	
	MAknsSkinInstance* skinInstance = AknsUtils::SkinInstance();

	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnIndiMarkedAdd,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileAvkon,
                                           EMbmAvkonQgn_indi_marked_add,
										   EMbmAvkonQgn_indi_marked_add_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2); // newIconBmp, newIconMaskBmp (see CreateIconLC)
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmAvkonQgn_prop_folder_small; maskId = EMbmAvkonQgn_prop_folder_small_mask; },
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropFolderSmall,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileAvkon,
                                           EMbmAvkonQgn_prop_folder_small,
										   EMbmAvkonQgn_prop_folder_small_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2); // newIconBmp, newIconMaskBmp (see CreateIconLC)
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmAvkonQgn_indi_level_back; maskId = EMbmAvkonQgn_indi_level_back_mask; }
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnIndiLevelBack,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileAvkon,
                                           EMbmAvkonQgn_indi_level_back,
										   EMbmAvkonQgn_indi_level_back_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2); // newIconBmp, newIconMaskBmp (see CreateIconLC)
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
    //"z:\\system\\apps\\browser\\Browser.mbm";
	//EMbmBrowserQgn_prop_wml_home; maskId = EMbmBrowserQgn_prop_wml_home_mask; },
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropWmlHome,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_prop_wml_home,
										   EMbmBrowserQgn_prop_wml_home_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2); // newIconBmp, newIconMaskBmp (see CreateIconLC)
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_prop_wml_bm_last; maskId = EMbmBrowserQgn_prop_wml_bm_last_mask; },
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropWmlBmLast,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_prop_wml_bm_last,
										   EMbmBrowserQgn_prop_wml_bm_last_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2); // newIconBmp, newIconMaskBmp (see CreateIconLC)
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_prop_wml_bm; maskId = EMbmBrowserQgn_prop_wml_bm_mask; },
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropWmlBm,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_prop_wml_bm,
										   EMbmBrowserQgn_prop_wml_bm_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2); // newIconBmp, newIconMaskBmp (see CreateIconLC)
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_indi_wml_csd_add; maskId = EMbmBrowserQgn_indi_wml_csd_add_mask; },
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnIndiWmlCsdAdd,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_indi_wml_csd_add,
										   EMbmBrowserQgn_indi_wml_csd_add_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2); // newIconBmp, newIconMaskBmp (see CreateIconLC)
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_indi_wml_hscsd_add; maskId = EMbmBrowserQgn_indi_wml_hscsd_add_mask; },
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnIndiWmlHscsdAdd,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_indi_wml_hscsd_add,
										   EMbmBrowserQgn_indi_wml_hscsd_add_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2); // newIconBmp, newIconMaskBmp (see CreateIconLC)
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_indi_wml_gprs_add; maskId = EMbmBrowserQgn_indi_wml_gprs_add_mask; },
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnIndiWmlGprsAdd,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_indi_wml_gprs_add,
										   EMbmBrowserQgn_indi_wml_gprs_add_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2); // newIconBmp, newIconMaskBmp (see CreateIconLC)
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_prop_wml_folder_link_seamless; maskId = EMbmBrowserQgn_prop_wml_folder_link_seamless_mask; }
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropWmlFolderLinkSeamless,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_prop_wml_folder_link_seamless,
										   EMbmBrowserQgn_prop_wml_folder_link_seamless_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2); // newIconBmp, newIconMaskBmp (see CreateIconLC)
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);

    CleanupStack::Pop();    // POP Icon
	return icons;
}

// End of File
