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
*      Declaration of class CWmlBrowserBookmarksEditFormApModel.
*      
*
*/


#ifndef BROWSER_BOOKMARKS_EDIT_FORM_AP_MODEL_H
#define BROWSER_BOOKMARKS_EDIT_FORM_AP_MODEL_H

// INCLUDE FILES

#include <e32base.h>
#include <bamdesca.h>
#include <FavouritesWapAp.h>

// FORWARD DECLARATION

class CAknQueryValueText;
class CAknQueryValueTextArray;
class MCommsModel;
class CApListItemList;

// CLASS DECLARATION

/**
* Model for the WAP AP pop-up field in the Edit Bookmark form. It owns a list
* of Access Points, gotten from the AP Model; and also holds a descriptor
* array containing names of access points. The descriptor array always contains
* string "Default" as first, unless there are no AP-s at all. In that case,
* the array is empty.
*/
class CBrowserBookmarksEditFormApModel : public CBase
	{
	public:     // construction

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aCOmmsModel Comms model to use for accessing AP-s. Not owned.
        * @return The constructed model.
        */
        static CBrowserBookmarksEditFormApModel* NewL
            ( MCommsModel& aCommsModel );

        /**
        * Destructor.
        */
        virtual ~CBrowserBookmarksEditFormApModel();

    public:     // new methods

        /**
        * Get the query value.
        * @return The query value.
        */
        inline CAknQueryValueText* QueryValue();

        /**
        * Get and store Access Point data from the AP Model.
        */
		void ReadApDataL();

        /**
        * Set the current WAP Access point.
        * @param aAp Access Point to set as current.
        */
        void SetCurrentWapApL( const TFavouritesWapAp& aAp );

        /**
        * Get the current WAP Access point.
        * @return The current WAP Access point.
        */
        TFavouritesWapAp CurrentWapAp() const;

        /**
        * Count WAP Access points.
        * @return Number of WAP Access points.
        */
        TInt ApCount();


    protected:  // construction

        /**
        * Constructor.
        * @param aCommsModel Comms model to use for accessing AP-s.
        * Not owned.
        */
        CBrowserBookmarksEditFormApModel( MCommsModel& aCommsModel );

        /**
        * Second phase constructor. Leaves on failure.
        * @return The constructed model.
        */
		void ConstructL();

    private:    // data

        CDesCArrayFlat* iDesCArrayText;     ///< AP names as strings.
        CAknQueryValueTextArray* iTextArray;///< Text array for the value.
        CAknQueryValueText* iTextValue;     ///< Value for pop-up query.
        MCommsModel* iCommsModel;           ///< AP data source. Not owned.
        CApListItemList* iApList;           ///< List of access points. Owned.
    };

#include "BrowserBookmarksEditFormApModel.inl"

#endif

// End of file