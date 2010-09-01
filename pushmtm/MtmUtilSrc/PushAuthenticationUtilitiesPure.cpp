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
* Description:  Authentication utilities not using White List.
*
*/



// INCLUDE FILES

#include "PushAuthenticationUtilities.h"
#include "PushMtmUtilPanic.h"
#include "PushMtmLog.h"

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// TPushAuthenticationUtil::AuthenticateMsgL
// ---------------------------------------------------------
//
EXPORT_C TBool TPushAuthenticationUtil::AuthenticateMsgL
										( 
										const CPushMtmSettings& /*aMtmSettings*/, 
										const CPushMessage& /*aPushMsg*/ 
										)
    {
    PUSHLOG_ENTERFN("TPushAuthenticationUtil::AuthenticateMsgL")
    PUSHLOG_WRITE(" Nothing done.")
    PUSHLOG_LEAVEFN("TPushAuthenticationUtil::AuthenticateMsgL")
    return ETrue;
    }

// ---------------------------------------------------------
// TPushAuthenticationUtil::AuthenticateMsgL
// ---------------------------------------------------------
//
EXPORT_C TBool TPushAuthenticationUtil::AuthenticateMsgL
										( 
										const CPushMtmSettings& /*aMtmSettings*/, 
										const CPushMsgEntryBase& /*aPushMsg*/ 
										)
    {
    PUSHLOG_ENTERFN("TPushAuthenticationUtil::AuthenticateMsgL 2")
    PUSHLOG_WRITE(" Nothing done.")
    PUSHLOG_LEAVEFN("TPushAuthenticationUtil::AuthenticateMsgL 2")
    return ETrue;
    }

// End of file.
