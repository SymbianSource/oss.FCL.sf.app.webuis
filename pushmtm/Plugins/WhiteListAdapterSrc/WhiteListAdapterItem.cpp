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
* Description: 
*     Handles white list settings in provisioning.
*
*/


// INCLUDE FILES
	// User includes
#include "WhiteListAdapterItem.h"

// CONSTANTS

_LIT( KIPv4AddressType, "IPV4" );
_LIT( KIPv6AddressType, "IPV6" );
_LIT( KE164AddressType, "E164" );
_LIT( KAlphanumericAddressType, "ALPHA" );

// ================= MEMBER FUNCTIONS ======================

// ---------------------------------------------------------
// TWhiteListAdapterItem::TWhiteListAdapterItem
// ---------------------------------------------------------
//
TWhiteListAdapterItem::TWhiteListAdapterItem() 
:   iAddress( KNullDesC ), 
    iAddressType( CPushInitiator::ETypeE164 ), 
    iId( 0 ), 
    iIdBufPtr( iId ), 
    iProxyId( KNullDesC ), 
    iPushSupport( EPushDontCare )
	{
	iAddressType = CPushInitiator::ETypeIpv4;
	iPushSupport = EPushDontCare;
	}

// ---------------------------------------------------------
// TWhiteListAdapterItem::Address
// ---------------------------------------------------------
//
const TDesC& TWhiteListAdapterItem::Address() const
	{
	return iAddress;
	}

// ---------------------------------------------------------
// TWhiteListAdapterItem::AddressType
// ---------------------------------------------------------
//
CPushInitiator::TAddrType TWhiteListAdapterItem::AddressType() const
	{
	return iAddressType;
	}

// ---------------------------------------------------------
// TWhiteListAdapterItem::Id
// ---------------------------------------------------------
//
TUint32 TWhiteListAdapterItem::Id() const
    {
    return iId;
    }

// ---------------------------------------------------------
// TWhiteListAdapterItem::IdBuf
// ---------------------------------------------------------
//
const TDesC8& TWhiteListAdapterItem::IdBuf() const
    {
    return iIdBufPtr;
    }

// ---------------------------------------------------------
// TWhiteListAdapterItem::PushSupport
// ---------------------------------------------------------
//
TPushSupport TWhiteListAdapterItem::PushSupport() const
    {
    return iPushSupport;
    }

// ---------------------------------------------------------
// TWhiteListAdapterItem::SetAddress
// ---------------------------------------------------------
//
void TWhiteListAdapterItem::SetAddress( const TDesC& aAddress )
	{
	iAddress.Set( aAddress );
	}

// ---------------------------------------------------------
// TWhiteListAdapterItem::SetAddressType
// ---------------------------------------------------------
//
void TWhiteListAdapterItem::SetAddressType( const TDesC& aAddressType )
	{
	if ( !aAddressType.Compare( KIPv4AddressType ) )
		{
		iAddressType = CPushInitiator::ETypeIpv4;
		}
	else if ( !aAddressType.Compare( KIPv6AddressType ) )
		{
		iAddressType = CPushInitiator::ETypeIpv6;
		}
	else if ( !aAddressType.Compare( KE164AddressType ) )
		{
		iAddressType = CPushInitiator::ETypeE164;
		}
	else if ( !aAddressType.Compare( KAlphanumericAddressType ) )
		{
		iAddressType = CPushInitiator::ETypeAlpha;
		}
	}

// ---------------------------------------------------------
// TWhiteListAdapterItem::SetId
// ---------------------------------------------------------
//
void TWhiteListAdapterItem::SetId( TUint32 aId )
	{
	iId = aId;
	}

// ---------------------------------------------------------
// TWhiteListAdapterItem::SetProxyId
// ---------------------------------------------------------
//
void TWhiteListAdapterItem::SetProxyId( const TDesC& aProxyId )
	{
	iProxyId.Set( aProxyId );
	}

// ---------------------------------------------------------
// TWhiteListAdapterItem::SetPushSupport
// ---------------------------------------------------------
//
void TWhiteListAdapterItem::SetPushSupport( TPushSupport aPushSupport )
	{
	iPushSupport = aPushSupport;
	}

// ---------------------------------------------------------
// TWhiteListAdapterItem::Valid
// ---------------------------------------------------------
//
TBool TWhiteListAdapterItem::Valid() const
	{
	TBool valid = EFalse;

	// TODO : checking IP address/phone number validity (MsgBioUtils?)

	if ( ( iPushSupport == EPushEnabled || iPushSupport == EPushDontCare ) &&
		( iAddressType == CPushInitiator::ETypeE164 ) &&
		( 0 < iAddress.Length() ) )
		{
		valid = ETrue;
		}

	return valid;
	}
