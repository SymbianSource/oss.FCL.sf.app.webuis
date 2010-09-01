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
*     Panic handling for white list management.
*
*/


#ifndef WHITELISTADAPTERPANIC_H
#define WHITELISTADAPTERPANIC_H

// INCLUDE FILES
	// System includes
#include <e32def.h>

// DATA TYPES

enum TWhiteListAdapterPanics
	{
	ENoItemsToBeDisplayed,	///< A UI control tries to display items
	EArrayIndexOutOfBounds,	///< Self-explanatory
	ENullProxy				///< Physicial proxy cannot be found
	};

// FUNCTION PROTOTYPES

/**
* Panic handling in White List Adapter module. It's role is centralizing
* panics.
* @param aPanic Panic code, defined in THermesProtocolPanic.
*/
void Panic( TInt aPanic );


#endif	// WHITELISTADAPTERPANIC_H

// End of file