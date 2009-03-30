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
* Description:  The definition of CPushInitiatorList.
*
*/



// INCLUDE FILES

#include "PushInitiatorList.h"
#include "PushMtmLog.h"
#include "PushMtmUtilPanic.h"
#include "WhiteListImporter.h"
#include "PushMtmPrivateCRKeys.h"
#include <centralrepository.h> 

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CPushInitiatorList::NewL
// ---------------------------------------------------------
//
EXPORT_C CPushInitiatorList* CPushInitiatorList::NewL()
    {
    const TInt KGranularity( 8 );
    return new (ELeave) CPushInitiatorList( KGranularity );
    }

// ---------------------------------------------------------
// CPushInitiatorList::~CPushInitiatorList
// ---------------------------------------------------------
//
EXPORT_C CPushInitiatorList::~CPushInitiatorList()
    {
    ResetAndDestroy();
    }

// ---------------------------------------------------------
// CPushInitiatorList::AddL
// ---------------------------------------------------------
//
EXPORT_C TUint32 CPushInitiatorList::AddL
    ( const CPushInitiator& aPushInitiator )
    {
    CPushInitiator* temp = aPushInitiator.AllocLC();
    TUint32 entryId = AddL( temp );
    CleanupStack::Pop( temp );
    return entryId;
    }

// ---------------------------------------------------------
// CPushInitiatorList::AddL
// ---------------------------------------------------------
//
EXPORT_C TUint32 CPushInitiatorList::AddL( CPushInitiator* aPushInitiator )
    {
    PUSHLOG_ENTERFN("CPushInitiatorList::AddL")

    __ASSERT_DEBUG( aPushInitiator, UtilPanic( EPushMtmUtilPanNull ) );

    TUint32 entryId( ENullInitiatorEntryId );

    if ( !aPushInitiator )
        {
        // Nothing to insert.
        }
    else
        {
        // Get an ID for the new entry before we insert it to the list if it 
        // has not set yet:
        entryId = aPushInitiator->EntryID();
        if ( entryId == ENullInitiatorEntryId )
            {
            entryId = FreeId();
            // Give this new ID to the new entry:
            aPushInitiator->SetEntryID( entryId );
            }
        __ASSERT_DEBUG( entryId != ENullInitiatorEntryId, 
                        UtilPanic( EPushMtmUtilPanNull ) );
        // And append it to the list:
        AppendL( aPushInitiator );
        // Set flag that indicates that the list has changed:
        iPushInitiatorListChanged = ETrue;
        }

    PUSHLOG_WRITE_FORMAT(" entryId <%d>",entryId)
    PUSHLOG_LEAVEFN("CPushInitiatorList::AddL")
    return entryId;
    }

// ---------------------------------------------------------
// CPushInitiatorList::At
// ---------------------------------------------------------
//
EXPORT_C CPushInitiator& CPushInitiatorList::At( TInt aIndex ) const
    {
    return (CPushInitiator&)
            (*CArrayPtrFlat<CPushInitiator>::At( aIndex ));
    }

// ---------------------------------------------------------
// CPushInitiatorList::Delete
// ---------------------------------------------------------
//
EXPORT_C void CPushInitiatorList::Delete( TInt aIndex )
    {
    delete &At( aIndex );
    CArrayPtrFlat<CPushInitiator>::Delete( aIndex );
    // Set indicator flag.
    iPushInitiatorListChanged = ETrue;
    }

// ---------------------------------------------------------
// CPushInitiatorList::Count
// ---------------------------------------------------------
//
EXPORT_C TInt CPushInitiatorList::Count() const
    {
    return CArrayPtrFlat<CPushInitiator>::Count();
    }

// ---------------------------------------------------------
// CPushInitiatorList::FreeId
// ---------------------------------------------------------
//
TUint32 CPushInitiatorList::FreeId() const
    {
    TUint32 largestId( 0 );
    // Find the largest ID:
    const TInt count( Count() );
    for ( TInt i = 0; i < count; ++i )
        {
        if ( largestId < At(i).EntryID() )
            {
            largestId = At(i).EntryID();
            }
        }
    if ( largestId == KMaxTUint32 )
        {
        return 0;
        }

    return ( largestId + 1 );
    }

// ---------------------------------------------------------
// CPushInitiatorList::Changed
// ---------------------------------------------------------
//
TBool CPushInitiatorList::Changed() const
    {
    return iPushInitiatorListChanged;
    }

// ---------------------------------------------------------
// CPushInitiatorList::ResetChanged
// ---------------------------------------------------------
//
void CPushInitiatorList::ResetChanged()
    {
    iPushInitiatorListChanged = EFalse;
    }

// ---------------------------------------------------------
// CPushInitiatorList::ResetAndDestroy
// ---------------------------------------------------------
//
EXPORT_C void CPushInitiatorList::ResetAndDestroy()
    {
    CArrayPtrFlat<CPushInitiator>::ResetAndDestroy();
    // Set indicator flag.
    iPushInitiatorListChanged = ETrue;
    }

// ---------------------------------------------------------
// CPushInitiatorList::Find
// ---------------------------------------------------------
//
EXPORT_C TInt CPushInitiatorList::Find( const CPushInitiator& aPushInitiator, 
                                        TInt& aIndex ) const
    {
    PUSHLOG_ENTERFN("CPushInitiatorList::Find")

    TInt ret( KErrNotFound );

    PUSHLOG_WRITE_FORMAT2(" Searching for: <%S>, <%d>",
        &aPushInitiator.Addr(),aPushInitiator.Type())

    const TInt count( Count() );
    for ( TInt i = 0; i < count; ++i )
        {
        PUSHLOG_WRITE_FORMAT2(" Item: <%S>, <%d>",
                              &At( i ).Addr(),At( i ).Type())

        if ( At( i ) == aPushInitiator )
            {
            // We have found push initiator which is identical with the one 
            // given in the parameter.
            PUSHLOG_WRITE(" WL Found!")
            aIndex = i;
            ret = KErrNone;
            break;
            }
        }

    PUSHLOG_LEAVEFN("CPushInitiatorList::Find")
    return ret;
    }

// ---------------------------------------------------------
// CPushInitiatorList::operator==
// ---------------------------------------------------------
//
EXPORT_C TBool CPushInitiatorList::operator==
                                   ( const CPushInitiatorList& aList ) const
    {
    TBool equal( EFalse );
    const TInt count = Count();

    if ( count == aList.Count() )
        {
        TBool inequalFound( EFalse );
        for ( TInt i = 0; i < count; ++i )
            {
            if ( At(i) != aList.At(i) )
                {
                inequalFound = ETrue;
                break;
                }
            }
        equal = !inequalFound;
        }

    return equal;
    }

// ---------------------------------------------------------
// CPushInitiatorList::operator!=
// ---------------------------------------------------------
//
EXPORT_C TBool CPushInitiatorList::operator!=
                                   ( const CPushInitiatorList& aList ) const
    {
    return !( *this == aList );
    }

// ---------------------------------------------------------
// CPushInitiatorList::ExternalizeL
// ---------------------------------------------------------
//
void CPushInitiatorList::ExternalizeL( CRepository& aRepository ) const
    {
    PUSHLOG_ENTERFN("CPushInitiatorList::ExternalizeL");
    
    HBufC* wlBuf = ParseAndProcessList2BufL();
    CleanupStack::PushL( wlBuf );
    User::LeaveIfError( aRepository.Set( KPushMtmWhiteList, *wlBuf ) );
    CleanupStack::PopAndDestroy( wlBuf ); // wlBuf
    
    PUSHLOG_LEAVEFN("CPushInitiatorList::ExternalizeL");
    }

// ---------------------------------------------------------
// CPushInitiatorList::InternalizeL
// ---------------------------------------------------------
//
void CPushInitiatorList::InternalizeL( CRepository& aRepository )
    {
    PUSHLOG_ENTERFN("CPushInitiatorList::InternalizeL");
    
    HBufC* wlBuf = HBufC::NewLC( NCentralRepositoryConstants::KMaxUnicodeStringLength );
    TPtr nonConstWLBuf = wlBuf->Des();
    User::LeaveIfError( aRepository.Get( KPushMtmWhiteList, nonConstWLBuf ) );
    ParseAndProcessBuf2ListL( *wlBuf );
    CleanupStack::PopAndDestroy( wlBuf ); // wlBuf
    
    PUSHLOG_LEAVEFN("CPushInitiatorList::InternalizeL");
    }

// ---------------------------------------------------------
// CPushInitiatorList::CPushInitiatorList
// ---------------------------------------------------------
//
CPushInitiatorList::CPushInitiatorList( const TInt aGranularity )
:   CArrayPtrFlat<CPushInitiator>( aGranularity ), 
    iPushInitiatorListChanged( EFalse )
    {
    }

// ---------------------------------------------------------
// CPushInitiatorList::ParseAndProcessBuf2ListL
// ---------------------------------------------------------
//
void CPushInitiatorList::ParseAndProcessBuf2ListL
                         ( const TDesC& aStreamedBuf )
    {
    PUSHLOG_ENTERFN("CPushInitiatorList::ParseAndProcessBuf2ListL");
    PUSHLOG_WRITE_FORMAT(" param: <%S>",&aStreamedBuf)
    
    CWhiteListConverter* converter = CWhiteListConverter::NewL( *this );
    CleanupStack::PushL( converter );

    TRAPD( err, converter->Buffer2ListL( aStreamedBuf ) );
    PUSHLOG_WRITE_FORMAT(" <%d>",err);
    if ( err == KErrOverflow || err == KErrCorrupt )
        {
        // The input is corrupted. Leave the White List as is and 
        // do not forward these LEAVE values, because it would 
        // terminate the RFS of Push.
        }
    else
        {
        // Other error. Must be forwarded.
        User::LeaveIfError( err );
        }

    CleanupStack::PopAndDestroy( converter ); // converter

    PUSHLOG_LEAVEFN("CPushInitiatorList::ParseAndProcessBuf2ListL");
    }

// ---------------------------------------------------------
// CPushInitiatorList::ParseAndProcessList2BufL
// ---------------------------------------------------------
//
HBufC* CPushInitiatorList::ParseAndProcessList2BufL() const
    {
    PUSHLOG_ENTERFN("CPushInitiatorList::ParseAndProcessList2BufL");

    CPushInitiatorList& nonConstPiList = (CPushInitiatorList&)*this;
    CWhiteListConverter* converter = CWhiteListConverter::NewL( nonConstPiList );
    CleanupStack::PushL( converter );

    HBufC* retBuf(0);
    TRAPD( err, retBuf = converter->List2BufferL() );
    PUSHLOG_WRITE_FORMAT(" <%d>",err);
    User::LeaveIfError( err );

    CleanupStack::PopAndDestroy( converter ); // converter

    PUSHLOG_WRITE_FORMAT(" ret: <%S>",retBuf)
    PUSHLOG_LEAVEFN("CPushInitiatorList::ParseAndProcessList2BufL");
    return retBuf;
    }

//  End of File.
