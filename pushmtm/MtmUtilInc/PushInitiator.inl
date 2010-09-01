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
* Description:  Inline functions of CPushInitiator.
*
*/



// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushInitiator::Addr
// ---------------------------------------------------------
//
inline const TDesC& CPushInitiator::Addr() const
    {
    return *iAddress;
    }

// ---------------------------------------------------------
// CPushInitiator::Type
// ---------------------------------------------------------
//
inline CPushInitiator::TAddrType CPushInitiator::Type() const
    {
    return iType;
    }

// ---------------------------------------------------------
// CPushInitiator::SetEntryID
// ---------------------------------------------------------
//
inline void CPushInitiator::SetEntryID( TUint32 aEntryID )
    {
    iEntryID = aEntryID;
    }

// ---------------------------------------------------------
// CPushInitiator::EntryID
// ---------------------------------------------------------
//
inline TUint32 CPushInitiator::EntryID() const
    {
    return iEntryID;
    }

// ---------------------------------------------------------
// CPushInitiator::AllocL
// ---------------------------------------------------------
//
inline CPushInitiator* CPushInitiator::AllocL() const
    {
    CPushInitiator* temp = AllocLC();
    CleanupStack::Pop( temp );
    return temp;
    }

// ---------------------------------------------------------
// CPushInitiator::AllocLC
// ---------------------------------------------------------
//
inline CPushInitiator* CPushInitiator::AllocLC() const
    {
    CPushInitiator* temp = new (ELeave) CPushInitiator;
    CleanupStack::PushL( temp );
    temp->SetAddressL( Addr(), Type() );
    return temp;
    }

// ---------------------------------------------------------
// CPushInitiator::operator==
// ---------------------------------------------------------
//
inline TBool CPushInitiator::
    operator==( const CPushInitiator& aOther ) const
    {
    return (Addr().Compare(aOther.Addr())==KErrNone) && 
        ( (Type()==aOther.Type()) || 
            Type()==ETypeAny || 
            aOther.Type()==ETypeAny );
    }

// ---------------------------------------------------------
// CPushInitiator::operator!=
// ---------------------------------------------------------
//
inline TBool CPushInitiator::
    operator!=( const CPushInitiator& aOther ) const
    {
    return (*this==aOther) == EFalse;
    }

//  End of File.
