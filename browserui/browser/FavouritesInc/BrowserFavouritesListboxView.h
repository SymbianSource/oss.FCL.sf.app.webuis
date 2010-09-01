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
*      Declaration of class CWmlBrowserFavouritesListboxView.
*      
*
*/


#ifndef BROWSER_FAVOURITES_LISTBOX_VIEW_H
#define BROWSER_FAVOURITES_LISTBOX_VIEW_H

// INCLUDE FILES
#include <aknlists.h>

// FORWARD DECLARATION

class MBrowserFavouritesListboxCursorObserver;

// CLASS DECLARATION

/**
* Listbox view, which notifies its observer about cursor movements.
*/
class CBrowserFavouritesListboxView: public CAknColumnListBoxView
	{
    public:

        /**
        * Constructor.
        * @param aListbox Listbox which has this view.
        */
		CBrowserFavouritesListboxView( CEikListBox& aListbox );

	public:     // new methods.

        /**
        * Set cursor observer. Can be set to NULL.
        * @param aCursorObserver This observer (if any) will be
        * notified about cursor changes.
        */
		void SetCursorObserver
            ( MBrowserFavouritesListboxCursorObserver* aCursorObserver );

        /**
        * Move the cursor. Execute base class's method and notify observer.
        * @param aCursorMovement Movement occurred in the listbox.
        * @param aSelectionMode Selection mode.
        */
        void MoveCursorL( CListBoxView::TCursorMovement aCursorMovement,
            TSelectionMode aSelectionMode );

        /**
        * Select / highlight items witout moving the cursor.
        * @param aSelectionMode Selection mode.
        */
        void UpdateSelectionL( TSelectionMode aSelectionMode );

	private:	// data

        /// The listbox.
        CEikListBox* iListbox;
        /// The cursor movement observer. Can be NULL.
        MBrowserFavouritesListboxCursorObserver* iCursorObserver;
	};

#endif

// End of file