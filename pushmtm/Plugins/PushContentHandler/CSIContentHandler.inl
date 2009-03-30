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



inline void CSIContentHandler::SetActionFlag(TBool aAction)      
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

inline TBool CSIContentHandler::ActionFlag() const
	{
	return iAttributes & EAction;
	}

inline void CSIContentHandler::SetCreatedFlag( TBool aCreated )
	{
	if ( aCreated )
        {
		iAttributes |= ECreated;
        }
	else
        {
		iAttributes &=~ ECreated;
        }
	}

inline TBool CSIContentHandler::CreatedFlag() const
	{
	return iAttributes & ECreated;
	}

inline void CSIContentHandler::SetHrefFlag( TBool aHref )
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

inline TBool CSIContentHandler::HrefFlag() const
	{
	return iAttributes & EHref;
	}

inline void CSIContentHandler::SetExpiresFlag( TBool aExpires )
	{
	if ( aExpires )
        {
		iAttributes |= EExpires;
        }
	else
        {
		iAttributes &=~ EExpires;
        }
	}

inline TBool CSIContentHandler::ExpiresFlag() const
	{
	return iAttributes & EExpires;
	}

inline void CSIContentHandler::SetSiIdFlag( TBool aSiId )
	{
	if ( aSiId )
        {
		iAttributes |= ESiId;
        }
	else
        {
		iAttributes &=~ ESiId;
        }
	}

inline TBool CSIContentHandler::SiIdFlag() const
	{
	return iAttributes & ESiId;
	}

inline void CSIContentHandler::SetDataFlag( TBool aData )
	{
	if ( aData )
        {
		iAttributes |= EData;
        }
	else
        {
		iAttributes &=~ EData;
        }
	}

inline TBool CSIContentHandler::DataFlag() const
	{
	return iAttributes & EData;
	}

