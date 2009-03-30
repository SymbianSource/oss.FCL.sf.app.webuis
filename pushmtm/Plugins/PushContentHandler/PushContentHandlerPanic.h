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



#ifndef PUSHCONTENTHANDLERPANIC_H
#define PUSHCONTENTHANDLERPANIC_H

// INCLUDE FILES

#include <e32std.h>

enum TPushContentHandlerPanic
	{
    EPushContHandPanCommandNotSupported = 1, ///< Not supported command.
    EPushContHandPanBadType, ///< Unsupported message type for an operation.
    EPushContHandPanFoldersNotSupported, ///< MTM loaded with bad message entry.
    EPushContHandPanEmptySelection, ///< Entry selection is empty.
    EPushContHandPanNotInitUrl,
    EPushContHandPanNoContext,
    EPushContHandPanAlreadyInitialized,
    EPushContHandPanBadActionValue,
    EPushContHandPanBadState, 
    EPushContHandPanNoFetchOp,
    EPushContHandPanBadTransaction,
    EPushContHandPanNoSiIdAttr,
    EPushContHandPanNoCreatedAttr,
    EPushContHandPanNoActionAttr,
    EPushContHandPanTooManySi,
    EPushContHandPanTooManySl,
    EPushContHandPanMsgNull,
    EPushContHandPanBadCreatedType,
    EPushContHandPanExpiresNotOpaque,
    EPushContHandPanHrefNotString,
    EPushContHandPanClassTokenReached,
    EPushContHandPanUnexpHrefAttrVal,
    EPushContHandPanUnexpSiToken,
    EPushContHandPanUnexpSlToken,
    EPushContHandPanUnspecSiAction,
    EPushContHandPanUnspecSlAction,
    EPushContHandPanUnexpectedTimeType,
    EPushContHandPanBadExpiresType,
    EPushContHandPanBadTTimeLength,
    EPushContHandPanNoSiIdOrHrefAttr,
    EPushContHandPanUnspecSiHref,
    EPushContHandPanUnspecSlHref,
    EPushContHandPanSlMsgIdSet,
    EPushContHandPanSlMsgIdNotSet,
    EPushContHandPanFetchAlreadyInit,
    EPushContHandPanAutBadState,
    EPushContHandPanSiResNotOnStack,
    EPushContHandPanNullCharSetId,
    EPushContHandPanNullUcs2Buf
	};

GLREF_C void ContHandPanic( TPushContentHandlerPanic aReason );

#endif // PUSHCONTENTHANDLERPANIC_H
