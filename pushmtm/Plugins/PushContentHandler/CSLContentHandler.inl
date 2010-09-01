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
* Description:  Inline functions.
*
*/




inline void CSLContentHandler::SetActionFlag( TBool aAction )
	{
	if ( aAction )
        {
		iAttributes |= EAction;
        }
	else
        {
		iAttributes &=~ EAction;
        }
	}

inline TBool CSLContentHandler::ActionFlag() const
	{
	return iAttributes & EAction;
	}

inline void CSLContentHandler::SetHrefFlag( TBool aHref )
	{
	if ( aHref )
        {
		iAttributes |= EHref;
        }
	else
        {
		iAttributes &=~ EHref;
        }
	}

inline TBool CSLContentHandler::HrefFlag() const
	{
	return iAttributes & EHref;
	}


