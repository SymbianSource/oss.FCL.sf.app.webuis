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
*      Declaration of class CWmlBrowserFavouritesModelObserver.
*      
*
*/


#ifndef BROWSER_FAVOURITES_MODEL_OBSERVER_H
#define BROWSER_FAVOURITES_MODEL_OBSERVER_H

// CLASS DECLARATION

/**
* Mixin protocol, observing model changes of a CBrowserFavouritesModel.
*/
class MBrowserFavouritesModelObserver
    {

    public:     // new methods

        /**
        * Derived classes should implement this method, and act accordingly.
        */
        virtual void HandleFavouritesModelChangeL() = 0;

    };

#endif

// End of file