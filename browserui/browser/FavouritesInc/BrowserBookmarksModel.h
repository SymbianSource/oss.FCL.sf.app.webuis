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
*      Declaration of class CBrowserBookmarksModel.
*      
*
*/


#ifndef BROWSER_BOOKMARKS_MODEL_H
#define BROWSER_BOOKMARKS_MODEL_H

// INCLUDE FILES

#include "BrowserFavouritesModel.h"

// CLASS DECLARATION
class MApiProvider;

/**
* Model for the Bookmarks Views of the WML Browser.
*/
class CBrowserBookmarksModel : public CBrowserFavouritesModel
	{
	public:
	    /**
        * Constructor.
        * @param aApiProvider The API provider
        */
		CBrowserBookmarksModel( MApiProvider& aApiProvider );
		// construction

        /**
        * Two-phased constructor. Leaves on failure.
        * @return The constructed model.
        */
        static CBrowserBookmarksModel* NewL( MApiProvider& aApiProvider );

    public:     // from CBrowserFavouritesModel

        /**
        * Get resource id of some text for an item.
        * @param aItem The item to get resource id for.
        * @param aType Type of resource text.
        * @return Resource id of the text.
        */
        TInt StringResourceId
            (
            const CFavouritesItem& aItem,
            CBrowserFavouritesModel::TTextResourceType aType
            ) const;

        MApiProvider& ApiProvider();



    protected:  // from CBrowserFavouritesModel

		CFavouritesItemList* GetFoldersSortedLC();
		
    private:
	    const TDesC& GetDBName();
	    
    private: //data members
        MApiProvider& iApiProvider; // API provider
    };

#endif

// End of file