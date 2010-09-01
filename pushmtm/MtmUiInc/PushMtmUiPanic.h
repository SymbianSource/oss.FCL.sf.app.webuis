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



#ifndef PUSHMTMUIPANIC_H
#define PUSHMTMUIPANIC_H

// INCLUDE FILES

#include <e32std.h>

enum TPushMtmUiPanic
	{
    EPushMtmUiPanCommandNotSupported = 1,  ///< Not supported command.
    EPushMtmUiPanBadType,  ///< Unsupported message type for an operation.
    EPushMtmUiPanFoldersNotSupported,    ///< MTM loaded with bad message entry.
    EPushMtmUiPanEmptySelection, ///< Entry selection is empty.
    EPushMtmUiPanAlreadyExistingTitle,
    EPushMtmUiPanMissingTitle,
    EPushMtmUiPanAlreadyExistingModel,
    EPushMtmUiPanMissingModel,
    EPushMtmUiPanNotInitialized,
    EPushMtmUiPanAlreadyInitialized,
    EPushMtmUiPanMissingListBox,
    EPushMtmUiPanBadEvent,
    EPushMtmUiPanBadMtm,  ///< Bad MTM type.
    EPushMtmUiPanBadBioType,  ///< Bad BIO type.
    EPushMtmUiPanCodeReached,  ///< The code line shouldn't be reached.
    EPushMtmUiPanTooManyServiceEntries
	};

GLREF_C void UiPanic( TPushMtmUiPanic aReason );

#endif // PUSHMTMUIPANIC_H
