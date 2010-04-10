/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Logging support for Bavp
*
*/


#ifndef BAVPLOGGER_H
#define BAVPLOGGER_H


// Logging methods for Bavp
void Log( TBool aWrite, TPtrC aData );
void Log( TBool aWrite, TPtrC aData, TInt aInt );
void Log( TBool aWrite, const TPtrC s, const TPtrC p);
void Log( TBool aWrite, const TPtrC8 s, const TPtrC8 p);

#endif // BAVPLOGGER_H
