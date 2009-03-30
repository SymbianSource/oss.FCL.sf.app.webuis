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
*      Declaration of class MWmlBrowserFavouritesListboxCursorObserver.
*      
*
*/


#ifndef BROWSER_FAVOURITES_LISTBOX_CURSOR_OBSERVER_H
#define BROWSER_FAVOURITES_LISTBOX_CURSOR_OBSERVER_H

// FORWARD DECLARATION

class CEikListBox;

// CLASS DECLARATION

/**
* Observer class for CBrowserFavouritesListbox. Classes implementing
* this protocol will be notified about cursor changes.
*/
class MBrowserFavouritesListboxCursorObserver
	{
	public:     // new methods

        /**
        * Handle cursor change.
        * @param aListBox Listbox in which the cursor has changed.
        */
        virtual void HandleCursorChangedL( CEikListBox* aListBox  ) = 0;
	};

#endif

// End of file