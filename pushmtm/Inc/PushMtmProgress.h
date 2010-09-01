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



#ifndef PUSHMTMPROGRESS_H
#define PUSHMTMPROGRESS_H

// INCLUDE FILES

#include <e32std.h>

// CLASS DECLARATION

/**
* This class provides progress information for the client side Push MTM 
* operations. This class is packaged into an 8 bit descriptor.
* This information packaged is returned by CMsvOperation::ProgressL(), for 
* example.
*/
class TPushMtmProgress 
    {
    public: // Constructors

        inline TPushMtmProgress();

    public: // New functions

        inline void Reset();

    public: // Data

        TInt iTotalValue;   ///< Final progress value.
        TInt iCurrentValue; ///< Current progress value.
        TInt iErrorCode1;   ///< Error code.
        TInt iErrorCode2;   ///< Error code.
        TInt iErrorCode3;   ///< Error code.
    };


/// TPushMtmProgress packaged into a package buffer.
typedef TPckgBuf<TPushMtmProgress> TPushMtmProgressBuf;

/// TPushMtmProgress packaged into a pointer.
typedef TPckgC<TPushMtmProgress> TPushMtmProgressPckg;


#include "PushMtmProgress.inl"

#endif // PUSHMTMPROGRESS_H

//  End of File
