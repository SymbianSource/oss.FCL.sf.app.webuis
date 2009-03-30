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
*      Implementation of CWmlBrowserFavouritesListboxState.
*      
*
*/


// INCLUDE FILES

#include "BrowserFavouritesListboxState.h"
#include <FavouritesLimits.h>

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserFavouritesListboxState::CBrowserFavouritesListboxState
// ---------------------------------------------------------
//
CBrowserFavouritesListboxState::CBrowserFavouritesListboxState()
    {
    iHighlightUid = KFavouritesNullUid;
    iTopItemUid = KFavouritesNullUid;
    }

// ---------------------------------------------------------
// CBrowserFavouritesListboxState::~CBrowserFavouritesListboxState
// ---------------------------------------------------------
//
CBrowserFavouritesListboxState::~CBrowserFavouritesListboxState()
    {
    delete iMarkUids;
    }
// End of File
