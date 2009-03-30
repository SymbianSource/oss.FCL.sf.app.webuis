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



#ifndef PUSHMTMCLISRVPANIC_H
#define PUSHMTMCLISRVPANIC_H

// INCLUDE FILES

#include <e32std.h>

enum TPushMtmCliSrvPanic
	{
    EPushMtmCliSrvPanCommandNotSupported = 1,  ///< Not supported command.
    EPushMtmCliSrvPanBadType,  ///< Unsupported message type for an operation.
    EPushMtmCliSrvPanFoldersNotSupported,    ///< MTM loaded with bad message entry.
    EPushMtmCliSrvPanEmptySelection, ///< Entry selection is empty.
    EPushMtmCliSrvPanNotInitUrl,
    EPushMtmCliSrvPanNoContext,
    EPushMtmCliSrvPanAlreadyInitialized
	};

GLREF_C void CliSrvPanic( TPushMtmCliSrvPanic aReason );

#endif // PUSHMTMCLISRVPANIC_H
