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
* Description:  This header file contains the declaration of CPushInitiatorList.
*
*/



#ifndef PUSHINITIATORLIST_H
#define PUSHINITIATORLIST_H

// INCLUDE FILES

#include "PushInitiator.h"
#include <e32base.h>

// FORWARD DECLARATIONS

class CRepository;

// CLASS DECLARATION

/**
* This class is used to store push initiator addresses.
*/
class CPushInitiatorList : protected CArrayPtrFlat<CPushInitiator>
    {
    public: // Constructors and destructor

        /**
        * Two-phased constructor.
        * @return New CPushInitiatorList object.
        */
        IMPORT_C static CPushInitiatorList* NewL();

        /**
        * Destructor.
        */
        IMPORT_C ~CPushInitiatorList();

    public: // New functions

        /**
        * Append the initiator to the list. An Initiator is defined by an 
        * address and an address type together. 
        * The address with the given type is added to the list even if an 
        * initiator with the same address & type already exists in it.
        * @param aPushInitiator The push initiator.
        * @return A unique ID of the Initiator in the list (greater than 0).
        */
        IMPORT_C TUint32 AddL( const CPushInitiator& aPushInitiator );

        /**
        * Append the initiator to the list. An Initiator is defined by an 
        * address and an address type together. 
        * The address with the given type is added to the list even if an 
        * initiator with the same address & type already exists in it.
        * @param aPushInitiator The push initiator. Ownership is taken!
        * @return A unique ID of the Initiator in the list (greater than 0).
        */
        IMPORT_C TUint32 AddL( CPushInitiator* aPushInitiator );

        /**
        * Give a reference to the indicated initiator.
        * @param aIndex Index of the initiator starting from zero.
        * @return Reference to the initiator.
        */
        IMPORT_C CPushInitiator& At( TInt aIndex ) const;

        /**
        * Delete the initiator at index.
        * @param aIndex Index of the initiator starting from zero.
        * @return None.
        */
        IMPORT_C void Delete( TInt aIndex );

        /**
        * Number of initiators in the list.
        * @return Item count.
        */
        IMPORT_C TInt Count() const;

        /**
        * Get a free ID that is not used by the list members yet. 
        * The smallest ID got is 1.
        * See CPushInitiator::EntryID()!
        * @return The largest ID used plus 1 is returned.
        */
        TUint32 FreeId() const;

        /**
        * Return the "changed" flag.
        * @return See above.
        */
        TBool Changed() const;

        /**
        * Reset the "changed" flag.
        * @return None.
        */
        void ResetChanged();

        /**
        * Delete the items in the list.
        * @return None.
        */
        IMPORT_C void ResetAndDestroy();

        /**
        * Find the first occurrence of the given initiator in the list. 
        * This method uses only the address & type properties of the 
        * Initiator - it does not use the ID property!
        * @param aPushInitiator The push initiator to be found.
        * @param aIndex If found, the index of it starting from zero.
        * @return KErrNone if found, KErrNotFound otherwise.
        */
        IMPORT_C TInt Find( const CPushInitiator& aPushInitiator, 
                            TInt& aIndex ) const;

        IMPORT_C TBool operator==( const CPushInitiatorList& aList ) const;
        IMPORT_C TBool operator!=( const CPushInitiatorList& aList ) const;

        /**
        * Externalize into the Repository.
        * @param aRepository CenRep
        * @return none
        */
        void ExternalizeL( CRepository& aRepository ) const;

        /**
        * Internalize from the Repository.
        * @param aRepository CenRep
        * @return none
        */
        void InternalizeL( CRepository& aRepository );

        void ParseAndProcessBuf2ListL( const TDesC& aStreamedBuf );
                         
        HBufC* ParseAndProcessList2BufL() const;
                         
    private: // Constructors and operators

        CPushInitiatorList( const TInt aGranularity );

    private: // Data

        /// The following members indicate if a value has been changed.
        TBool iPushInitiatorListChanged;
    };

#endif // PUSHINITIATORLIST_H

// End of File
