/*
* Copyright (c) 2002-2007 Nokia Corporation and/or its subsidiary(-ies).
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
*      Inline methods for CWmlBrowserBookmarksContainer
*      
*
*/


#ifndef BROWSER_BOOKMARKS_CONTAINER_INL
#define BROWSER_BOOKMARKS_CONTAINER_INL

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserBookmarksContainer::GotoPane
// ---------------------------------------------------------
//
CBrowserBookmarksGotoPane* CBrowserBookmarksContainer::GotoPane() const
    {
    return iGotoPane;
    }


// ---------------------------------------------------------
// CBrowserBookmarksContainer::GotoPaneActive
// ---------------------------------------------------------
//
TBool CBrowserBookmarksContainer::GotoPaneActive() const
    {
    return iGotoPaneActive;
    }


#endif

// End of File
