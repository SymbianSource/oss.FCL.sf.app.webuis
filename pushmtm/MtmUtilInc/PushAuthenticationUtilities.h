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
* Description:  Authentication utilities.
*
*/



#ifndef PUSHAUTHENTICATIONUTILITIES_H
#define PUSHAUTHENTICATIONUTILITIES_H

// INCLUDE FILES

#include <e32base.h>

// FORWARD DECLARATIONS

class CPushMtmSettings;
class CPushMessage;
class CPushMsgEntryBase;

// CLASS DECLARATION

/**
* Utility class.
*/
class TPushAuthenticationUtil
    {
    public: // New functions

		/**
		* Authenticate the push message.
		* @param aMtmSettings The push settings.
		* @param aPushMsg The push message.
		* @return ETrue if authenticated. EFalse otherwise.
		*/
		IMPORT_C static TBool AuthenticateMsgL
								( 
								const CPushMtmSettings& aMtmSettings, 
							    const CPushMessage& aPushMsg 
								);

		/**
		* Authenticate the push message.
		* @param aMtmSettings The push settings.
		* @param aPushMsg The push message.
		* @return ETrue if authenticated. EFalse otherwise.
		*/
		IMPORT_C static TBool AuthenticateMsgL
								( 
								const CPushMtmSettings& aMtmSettings, 
								const CPushMsgEntryBase& aPushMsg 
								);

    };

#endif // PUSHAUTHENTICATIONUTILITIES_H

// End of file.
