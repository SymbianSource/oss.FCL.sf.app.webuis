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



#ifndef PUSHVIEWERPANIC_H
#define PUSHVIEWERPANIC_H

// INCLUDE FILES

#include <e32std.h>

enum TPushViewerPanic
	{
    EPushViewerPanCommandNotSupported = 1,  ///< Not supported command.
    EPushViewerPanBadType = 2,  ///< Unsupported message type for an operation.
    EPushViewerPanFoldersNotSupported = 3,    ///< MTM loaded with bad message entry.
    EPushViewerPanEmptySelection = 4,  ///< Entry selection is empty.
    EPushViewerPanBadItemType = 5,
    EPushViewerPanNotInitialized = 6,
    EPushViewerPanListBoxTooLarge = 7,
    EPushViewerPanNull = 8,
    EPushViewerPanIllegalArgument = 9,
    EPushViewerPanBadPushType = 10,
    EPushViewerPanBadMtmType = 11,
    EPushViewerPanNotEmptyRichText = 12,
    EPushViewerPanBadParaCount = 13,
    EPushViewerPanSlEmptyHref = 14
	};

GLREF_C void ViewerPanic( TPushViewerPanic aReason );

#endif // PUSHVIEWERPANIC_H
