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
* Description:  This header file contains the declaration of TPushInitiator.
*
*/



#ifndef PUSHINITIATOR_H
#define PUSHINITIATOR_H

// INCLUDE FILES

#include <e32base.h>
#include <e32def.h>

// CONSTANTS

const TUint32 ENullInitiatorEntryId = 0;

// CLASS DECLARATION

/**
* This class represents a push initiator's address.
*/
class CPushInitiator : public CBase
    {
    public: // Data type

        enum TAddrType      ///< Address type
            {
            ETypeIpv4,      ///< IPv4
            ETypeIpv6,      ///< IPv6
            ETypeE164,      ///< MSISDN
            ETypeAlpha,     ///< Alphanumeric
            ETypeAny        ///< Any
            };

    public: // Constructors

        IMPORT_C CPushInitiator();
        IMPORT_C ~CPushInitiator();

    public: // New functions

        IMPORT_C void SetAddressL( const TDesC& aAddr, 
                                   TAddrType aType );

        inline const TDesC& Addr() const;
        inline TAddrType Type() const;

        inline void SetEntryID( TUint32 aEntryID );
        inline TUint32 EntryID() const;

        inline CPushInitiator* AllocL() const;
        inline CPushInitiator* AllocLC() const;

        /**
        * Compares two initiators for equality. It compares the address and 
        * type. Note that ETypeAny matches any!
        * @param aPushInitiator The other push initiator.
        * @return ETrue if they are equal.
        */
        inline TBool operator==( const CPushInitiator& aOther ) const;
        inline TBool operator!=( const CPushInitiator& aOther ) const;

    private: // Data members

        HBufC* iAddress; ///< Address string. Owned.
        TAddrType iType; ///< Address type.
        TUint32 iEntryID; ///< Unique ID of the entry.
    };

#include "PushInitiator.inl"

#endif // PUSHINITIATOR_H

// End of File
