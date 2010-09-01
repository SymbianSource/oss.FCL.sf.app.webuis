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
* Description:  Declaration of class TPushMtmProgress.
*
*/



//**********************************
// TPushMtmProgress
//**********************************

inline TPushMtmProgress::TPushMtmProgress()
    {
    Reset();
    }

inline void TPushMtmProgress::Reset()
    {
    iTotalValue = 0;
    iCurrentValue = 0;
    iErrorCode1 = KErrNone;
    iErrorCode2 = KErrNone;
    iErrorCode3 = KErrNone;
    };

//  End of File
