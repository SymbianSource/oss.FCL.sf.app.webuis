/*
* Copyright (c) 2003, 2004 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Declatarion of CWhiteListImporter.
*
*/


#ifndef WHITE_LIST_CONVERTER_H
#define WHITE_LIST_CONVERTER_H

// INCLUDE FILES

#include <e32base.h>
#include <e32std.h>

// FORWARD DECLARATIONS

class CPushInitiatorList;

// CLASS DECLARATION

/**
* Converter utility.
* Import White List data from a comma and semicolon separated descriptor to 
* push initiator list, or export that to a descriptor.
*/
class CWhiteListConverter : public CBase
    {
    public:     // Constructors and destructor

        /**
        * Two-phased constructor. Leaves on failure.
        * @param aPushInitiatorList PushInitiatorList.
        * @return The constructed importer.
        */
        static CWhiteListConverter* NewL( CPushInitiatorList& aPushInitiatorList );

        /**
        * Destructor.
        */
        virtual ~CWhiteListConverter();

    public:     // New methods

        /**
        * Import White List data from a comma and semicolon 
        * separated descriptor.
        * @param aSource Input descriptor.
        * @throw KErrOverflow No more space in input buffer.
        * @throw KErrCorrupt Corrupt input.
        */
        void Buffer2ListL( const TDesC& aSource );

        /**
        * Export White List data to a comma and semicolon separated descriptor.
        * @return Output descriptor. Ownership transferred to the caller.
        * @throw KErrOverflow No more space in input buffer.
        * @throw KErrCorrupt Corrupt input.
        */
        HBufC* List2BufferL();

    private:    // Constructors and destructor

        /**
        * Constructor.
        */
        CWhiteListConverter( CPushInitiatorList& aPushInitiatorList );

        /**
        * Second phase constructor. Leaves on failure.
        */
        void ConstructL();

    private:    // New methods

        /**
        * Get next character and store it in iCurCh.
        */
        inline void GetChar();

        /**
        * Process next line = Record.
        * @return ETrue if more lines to go, EFalse on EOS.
        */
        TBool NextLineL();

        /**
        * Parse next token = Unit. 
        * Next token spans from current character up to (but excluding) 
        * to the next stop character.
        * @param aStopChar Stop character (terminate the token).
        * @return Pointer to token. This may be empty string. Note that the
        * returned pointer is valid until next call (consecutive calls reuse
        * the same buffer).
        */
        TPtrC NextTokenL( TUint aStopChar );

        /**
        * Parse the separated list of White List entries and 
        * add it to PushMtmSettings.
        */
        void AttrsL();

    private:    // data

        CPushInitiatorList& iPushInitiatorList; ///< The initiator list.
        TLex  iSource;  ///< Helper to parse iSource.
        TUint iCurCh;   ///< Current (lookahead) character.
        TText* iBuf;    ///< Token buffer. Owned.
        TText* iNextCh; ///< Next character is stored here. Not owned.
        TText* iMaxCh;  ///< End of buffer (points past the buffer). Not owned.
        HBufC* iConverterBuf; ///< Owned.
    };

#endif // WHITE_LIST_CONVERTER_H
            
// End of File
