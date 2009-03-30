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
* Description:  Miscellaneous definitions for the Push MTM modules.
*
*/



#ifndef PUSHMTMDEF_H
#define PUSHMTMDEF_H


struct TMessageInfoParam         ///< Parameter for the Message info UI function
    {
    TBool iDontShowNotification; ///< Set to ETrue not to show info note.
    };

/// TMessageInfoParam packaged into a package buffer.
typedef TPckgBuf<TMessageInfoParam> TMessageInfoParamBuf;

/// TMessageInfoParam packaged into a pointer.
typedef TPckgC<TMessageInfoParam> TMessageInfoParamPckg;


#endif // PUSHMTMDEF_H
