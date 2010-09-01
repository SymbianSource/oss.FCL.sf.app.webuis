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
*      Inline methods for CBrowserPreferences.
*
*
*/


#ifndef BROWSER_WINDOWMANAGER_INL
#define BROWSER_WINDOWMANAGER_INL

// -----------------------------------------------------------------------------
// CBrowserWindowManager::ApiProvider()
// -----------------------------------------------------------------------------
//
MApiProvider* CBrowserWindowManager::ApiProvider() const
    {
    return iApiProvider;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::ContentView()
// -----------------------------------------------------------------------------
//
CBrowserContentView* CBrowserWindowManager::ContentView() const
    {
    return iContentView;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::SetUserExit()
// -----------------------------------------------------------------------------
//
void CBrowserWindowManager::SetUserExit( TBool aUserExit )
    {
    iUserExit = aUserExit;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::MaxWindowCount()
// -----------------------------------------------------------------------------
//
TInt CBrowserWindowManager::MaxWindowCount() const
    {
    return iMaxWindowCount;
    }

// -----------------------------------------------------------------------------
// CBrowserWindowManager::IsContentExist()
// -----------------------------------------------------------------------------
//
// is there any content in the window?
TBool CBrowserWindowManager::IsContentExist() const
    {
    return iIsContentExist;
    }
        
// -----------------------------------------------------------------------------
// CBrowserWindowManager::SetContentExist()
// -----------------------------------------------------------------------------
//
// content was shown in the window
void CBrowserWindowManager::SetContentExist( TBool aValue )
    {
    iIsContentExist = aValue;
    }
        
#endif  // BROWSER_WINDOWMANAGER_INL

// End of File
