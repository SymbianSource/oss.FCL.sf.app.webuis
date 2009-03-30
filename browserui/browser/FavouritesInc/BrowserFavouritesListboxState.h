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
*      Declaration of class CWmlBrowserFavouritesListboxState.
*      
*
*/


#ifndef BROWSER_FAVOURITES_LISTBOX_STATE_H
#define BROWSER_FAVOURITES_LISTBOX_STATE_H

// INCLUDE FILES

#include <e32base.h>

// CLASS DECLARATION

/**
* Listbox state. This contains highlighted item Uid, marked item Uids
* and top item Uid. When swithching views or there is a database update,
* the listbox state is saved, and after update an attempt is made to
* restore the listbox in a way so the display is same (or at least similar to)
* as it was before the change.
* Data to this class is fed by CBrowserFavouritesListbox.
* All attributes are public.
*/
class CBrowserFavouritesListboxState: public CBase
	{	
	friend class CBrowserFavouritesListbox;
    friend class CBrowserFavouritesView;
    public:     // construct / destruct

        /**
        * Constructor.
        */
        CBrowserFavouritesListboxState();

        /**
        * Destructor.

        */
        ~CBrowserFavouritesListboxState();

    protected:     // data

        TInt iHighlightUid;         ///< Uid of highlighted item.
        CArrayFix<TInt>* iMarkUids; ///< Uids of marked items.
        TInt iTopItemUid;           ///< Uid of top item.
    };

#endif

// End of file