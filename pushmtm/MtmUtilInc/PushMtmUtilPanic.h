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



#ifndef PUSHMTMUTILPANIC_H
#define PUSHMTMUTILPANIC_H

// INCLUDE FILES

#include <e32std.h>

enum TPushMtmUtilPanic
	{
    EPushMtmUtilPanNull = 1, 
    EPushMtmUtilPanNotNull, 
    EPushMtmUtilPanNotSupported,
    EPushMtmUtilPanBadMtm,
    EPushMtmUtilPanBadBioType
	};

GLREF_C void UtilPanic( TPushMtmUtilPanic aReason );

#endif // PUSHMTMUTILPANIC_H
