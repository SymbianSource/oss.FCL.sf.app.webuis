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
* Description:  Panic utility
*
*/



// INCLUDE FILES

#include "PushMtmCliSrvPanic.h"

// LOCAL CONSTANTS AND MACROS

_LIT( KCliSrvPanicString, "PushMtmCliSrv" );

// ---------------------------------------------------------
// CliSrvPanic
// ---------------------------------------------------------
//
GLDEF_C void CliSrvPanic( TPushMtmCliSrvPanic aReason )
	{
	User::Panic( KCliSrvPanicString, aReason );
	}
