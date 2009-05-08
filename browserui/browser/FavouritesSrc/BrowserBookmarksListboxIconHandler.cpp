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
*      Implementation of TBrowserBookmarksListboxIconHandler.
*      
*
*/


// INCLUDE FILES

#include <ApListItem.h>
#include <ApListItemList.h>
#include <ApEngineConsts.h>
#include <FavouritesItem.h>
#include <FavouritesWapAp.h>
#include <BrowserNG.rsg>
#include "BrowserBookmarksListboxIconHandler.h"
#include "BrowserFavouritesListbox.h"
#include "BrowserFavouritesIconIndexArray.h"
#include "BrowserFavouritesView.h"
#include <AknsUtils.h>
#include <avkon.mbg>
#include <gulicon.h>
#include <AknIconArray.h>
#include <browser.mbg>

#include "CommonConstants.h"
#include <data_caging_path_literals.hrh> 

// LOCAL TYPES

/**
* Icon indexes. Numeric values MUST match the resource definition order.
*/
enum
    {
    EIconIndexListboxMark = 0,      ///< Listbox mark icon index.
    EIconIndexFolder = 1,           ///< Folder icon index.
    EIconIndexHomepage = 2,         ///< Homepage icon index.
    EIconIndexLastVisitedPage = 3,  ///< Last Visited icon index.
    EIconIndexBookmark = 4,         ///< Bookmark icon index.
    EIconIndexCsdBearer = 5,        ///< CSD bearer icon index.
    EIconIndexHscsdBearer = 6,      ///< HSCSD bearer icon index.
    EIconIndexGprsBearer = 7,       ///< GPRS bearer icon index.
	EIconIndexSeamlessLinksFolder = 8, ///< Seamless links icon index
	EIconIndexAdaptiveFolder = 9,      ///< Adaptive Bookmarks domain folder icon index
	EIconIndexAdaptiveBookmark = 10,      ///< Adaptive Bookmarks bookmark icon index
	EIconIndexDefaultBookmark = 11,      ///< Adaptive Bookmarks bookmark icon index
	EIconIndexRSSFolder = 12,	     ///< RSS Folder  icon index
	EIconIndexService = 13             ///< service icon
    };

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// TBrowserBookmarksListboxIconHandler::ItemIconIndex
// ---------------------------------------------------------
//
TInt TBrowserBookmarksListboxIconHandler::ItemIconIndex
( const CFavouritesItem& aItem ,CBrowserFavouritesListbox* aListbox ) const
    {
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
    //service icon
    if( aItem.ContextId() == KFavouritesServiceContextId  )
        {
        return EIconIndexService;
        }
  
   	//seamless links folders have a different icon
   	if( CBrowserFavouritesView::IsSeamlessFolder( aItem.ContextId() ) )
   		{
   		return EIconIndexSeamlessLinksFolder;
   		}
   		
    // recent url folder
    if ( aItem.Uid() == KFavouritesAdaptiveItemsFolderUid )
        {
        return EIconIndexAdaptiveFolder;
        }
   		   	
    /// RSS folder handling
    
    if (aItem.Uid()==KFavouritesRSSItemUid)
        {
         return EIconIndexRSSFolder;   
        }

    /// The order is important, because Adaptive bookmarks folder accidentally has a context id
    /// and acts like a seamless folder
    if( aListbox->GetDefaultData().iPreferedId == aItem.Uid() && aListbox->GetDefaultData().iInSeamlessFolder)
		{
		return EIconIndexDefaultBookmark;
		}

	return aItem.IsItem() ? EIconIndexBookmark : EIconIndexFolder;
    }

// ---------------------------------------------------------
// TBrowserBookmarksListboxIconHandler::BearerIconIndex
// ---------------------------------------------------------
//
TInt TBrowserBookmarksListboxIconHandler::BearerIconIndex
        (
        const TFavouritesWapAp aAccessPoint,
        const CApListItemList& aAccessPoints
        ) const
    {
    if ( !aAccessPoint.IsNull() && !aAccessPoint.IsDefault() )
        {
        // Set an invalid bearer type value.
        TApBearerType bearer = EApBearerTypeAll;
        // Get access point with this uid.
        TUint32 apUid = aAccessPoint.ApId();
        TInt i;
        TInt count = aAccessPoints.Count();
        for ( i = 0; i < count; i++ )
            {
            if ( aAccessPoints.At( i )->Uid() == apUid )
                {
                // Got the AP, set bearer type to its bearer.
                bearer = aAccessPoints.At( i )->BearerType();
                break;
                }
            }
        // If the bearer type is set, get icon index for it.
        switch( bearer )
            {
            case EApBearerTypeCSD:
                {
                return EIconIndexCsdBearer;
                }

            case EApBearerTypeGPRS:
                {
                return EIconIndexGprsBearer;
                }

            case EApBearerTypeHSCSD:
                {
                return EIconIndexHscsdBearer;
                }

			default:
                {
				break;
                }
            }
        }
    // Access Point is "Null", "Default", or just not found.
    return KBrowserFavouritesNoBearerIcon;
    }

// ---------------------------------------------------------
// TBrowserBookmarksListboxIconHandler::LoadIconsL
// ---------------------------------------------------------
//
CAknIconArray* TBrowserBookmarksListboxIconHandler::LoadIconsL() const
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
	//EMbmAvkonQgn_indi_marked_add
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnIndiMarkedAdd,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileAvkon,
                                           EMbmAvkonQgn_indi_marked_add,
										   EMbmAvkonQgn_indi_marked_add_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
    //EMbmAvkonQgn_prop_folder_small
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropFolderSmall,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileAvkon,
                                           EMbmAvkonQgn_prop_folder_small,
										   EMbmAvkonQgn_prop_folder_small_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//"z:\\system\\apps\\browser\\Browser.mbm";
	//EMbmBrowserQgn_prop_wml_home
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropWmlHome,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_prop_wml_home,
										   EMbmBrowserQgn_prop_wml_home_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_prop_wml_bm_last
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropWmlBmLast,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_prop_wml_bm_last,
										   EMbmBrowserQgn_prop_wml_bm_last_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_prop_wml_bm
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropWmlBm,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_prop_wml_bm,
										   EMbmBrowserQgn_prop_wml_bm_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_indi_wml_csd_add
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnIndiWmlCsdAdd,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_indi_wml_csd_add,
										   EMbmBrowserQgn_indi_wml_csd_add_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_indi_wml_hscsd_add
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnIndiWmlHscsdAdd,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_indi_wml_hscsd_add,
										   EMbmBrowserQgn_indi_wml_hscsd_add_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_indi_wml_gprs_add
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnIndiWmlGprsAdd,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_indi_wml_gprs_add,
										   EMbmBrowserQgn_indi_wml_gprs_add_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_prop_wml_folder_link_seamless
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropWmlFolderLinkSeamless,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_prop_wml_folder_link_seamless,
										   EMbmBrowserQgn_prop_wml_folder_link_seamless_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);
	//EMbmBrowserQgn_prop_wml_folder_adap
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropWmlFolderAdap,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_prop_wml_folder_adap,
										   EMbmBrowserQgn_prop_wml_folder_adap_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);

	//EMbmBrowserQgn_prop_wml_bm_adap
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropWmlBmAdap,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_prop_wml_bm_adap,
										   EMbmBrowserQgn_prop_wml_bm_adap_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2);
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);

	//EMbmBrowserQgn_prop_psln_active

	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropPslnActive,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_prop_psln_active,
										   EMbmBrowserQgn_prop_psln_active_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2); // newIconBmp, newIconMaskBmp (see CreateIconLC)
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);

	//EMbmBrowserQgn_prop_folder_rss
	
	AknsUtils::CreateIconLC( skinInstance,
                                           KAknsIIDQgnPropFolderRss,
                                           newIconBmp,
										   newIconMaskBmp,
                                           iconFileBrowser,
                                           EMbmBrowserQgn_prop_folder_rss,
										   EMbmBrowserQgn_prop_folder_rss_mask);    
	newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp);
    CleanupStack::Pop(2); // newIconBmp, newIconMaskBmp (see CreateIconLC)
	CleanupStack::PushL(newIcon);
	icons->AppendL( newIcon );
	CleanupStack::Pop(newIcon);

    //Browser service

    AknsUtils::CreateIconLC( skinInstance,
                                       KAknsIIDQgnPropBrowserInfoSmall,
                                       newIconBmp,
                                       newIconMaskBmp,
                                       iconFileBrowser,
                                       EMbmBrowserQgn_prop_browser_info_small,
                                       EMbmBrowserQgn_prop_browser_info_small_mask );
    newIcon = CGulIcon::NewL( newIconBmp, newIconMaskBmp );
    CleanupStack::Pop(2);
    CleanupStack::PushL(newIcon);
    icons->AppendL( newIcon );
    CleanupStack::Pop(newIcon);
    
    CleanupStack::Pop();    // POP Icon
	return icons;
}

// End of File
