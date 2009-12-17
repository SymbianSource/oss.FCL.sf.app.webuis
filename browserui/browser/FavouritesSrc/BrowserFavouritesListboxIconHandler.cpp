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
*      Implementation of MWmlBrowserFavouritesListboxIconHandler.
*
*
*/


// INCLUDE FILES
#include <AknIconArray.h>
#include <favouritesitem.h>
#include "BrowserFavouritesListboxIconHandler.h"
#include "BrowserFavouritesIconIndexArray.h"
#include "BrowserFavouritesListbox.h"
#include "BrowserFaviconHandler.h"

// CONSTANTS

//================== MEMBER FUNCTIONS =========================================

//-----------------------------------------------------------------------------
// MBrowserFavouritesListboxIconHandler::CreateIconArrayL
//-----------------------------------------------------------------------------
//
CAknIconArray* MBrowserFavouritesListboxIconHandler::CreateIconArrayL() const
    {
	return LoadIconsL( );
    }

//-----------------------------------------------------------------------------
// MBrowserFavouritesListboxIconHandler::IconIndexes
//-----------------------------------------------------------------------------
//
TBrowserFavouritesIconIndexes
    MBrowserFavouritesListboxIconHandler::IconIndexes( 
                                    const CFavouritesItem& aItem,
                                    CBrowserFavouritesListbox* aListbox ) const
    {
    TBrowserFavouritesIconIndexes indexes;

    // Do we have a Favicon?
    TInt faviconIndex;
    faviconIndex = aListbox->FaviconHandler()->GetFaviconArrayIndex( aItem );

    if ( faviconIndex != KErrNotFound && aListbox->GetDefaultData().iPreferedId != aItem.Uid() )
	    {
		indexes.iItemIcon = faviconIndex;    	
	    }
    else
	    {
	    indexes.iItemIcon = ItemIconIndex( aItem,aListbox );	
	    }

    indexes.iBearerIcon = KBrowserFavouritesNoBearerIcon; //Bearer icons removed
	return indexes;
    }

//-----------------------------------------------------------------------------
// MBrowserFavouritesListboxIconHandler::BearerIconIndex
//-----------------------------------------------------------------------------
//
TInt MBrowserFavouritesListboxIconHandler::BearerIconIndex(
                                        const TFavouritesWapAp /*aAccessPoint*/,
                                        const CApListItemList& /*aAccessPoints*/
                                        ) const
    {
    return KBrowserFavouritesNoBearerIcon;
    }

// End of File

