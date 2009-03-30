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
*     Class the represents a physical proxy (in context of WAP Provisioning).
*
*/


#ifndef WHITELISTADAPTERITEM_H
#define WHITELISTADAPTERITEM_H

// INCLUDE FILES

	// User includes
#include "PushInitiator.h"
#include "WhiteListAdapterDef.h"
	// System includes
#include <e32std.h>

// CLASS DECLARATION

/**
* This class encapsulates a proxy item found in a WAP Provisioning document.
* It is designed for describing a particular White List address.
*/
class TWhiteListAdapterItem
	{
	public :	// construction, destruction
		/**
		* Default constructor.
		*/
		TWhiteListAdapterItem();

	public :	// getter/setter methods
		/**
		* Use this method to get the address.
		* @return The stored proxy address.
		*/
		const TDesC& Address() const;

		/**
		* Use this method to get the address type.
		* @return The type of the stored proxy address.
		*/
		CPushInitiator::TAddrType AddressType() const;

        /**
        * Tells the ID of the item.
        * @return Item ID.
        */
        TUint32 Id() const;

        /**
        * Tells the ID of the item in a buffer.
        * @return Item ID in a buffer.
        */
        const TDesC8& IdBuf() const;

        /**
        * Tells the push support.
        * @return Push support.
        */
        TPushSupport PushSupport() const;

		/**
		* Sets the address.
		* @param aAddress The proxy address to be set.
		*/
		void SetAddress( const TDesC& aAddress );

		/**
		* Sets the address type.
		* @param aAddressType The proxy address type to be set.
		*/
		void SetAddressType( const TDesC& aAddressType );

		/**
		* Sets the ID of the item.
		* @param aID The ID.
		*/
        void SetId( TUint32 aId );

        /**
		* Sets the physical proxy ID.
		* @param aProxyId The physical proxy identifier to be set.
		*/
		void SetProxyId( const TDesC& aProxyId );

		/**
		* Sets whether or not the proxy address is push enabled.
		* @param aPushEnabled If the value is 1, then push is enabled,
		* otherwise (i.e. when the value is 0) the push is disabled.
		*/
		void SetPushSupport( TPushSupport aPushSupport );

	public :	// other public methods

        /**
		* Tells whether the freshly initialized adapter item contains
		* information about a valid proxy address. The proxy address is
		* invalid if either it is not type of the correct address type
		* or the proxy server is not push enabled.
		* @return ETrue if the proxy address is valid, EFalse otherwise.
		*/
		TBool Valid() const;

	private :	// data members

        TPtrC iAddress;				///< Physical proxy gateway address

		CPushInitiator::TAddrType iAddressType;

        /// ID of the item after it has been added to MTM Settings.
        TUint32 iId;
        /// ID of the item packaged into a buffer pointer.
        TPckgC<TUint32> iIdBufPtr;

		TPtrC iProxyId;				///< Proxy identifier (or name)

		TPushSupport iPushSupport;
	};

#endif	// WHITELISTADAPTERITEM_H

// End of file