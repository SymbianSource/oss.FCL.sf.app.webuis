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
* Description:  Implementation of CWhiteListConverter.
*
*/


// INCLUDE FILES

#include "WhiteListImporter.h"
#include "PushMtmSettings.h"
#include "PushInitiatorList.h"
#include "PushInitiator.h"
#include "PushMtmLog.h"
#include <e32std.h>

// CONSTANTS

/// Max length allowed.
LOCAL_C const TInt KMaxLength = 2048;

/*
* Special characters.
*/
/// ',' character.
LOCAL_C const TUint KComma = ',';
/// ';' character.
LOCAL_C const TUint KSemicolon = ';';
/// Escape '\' character.
LOCAL_C const TUint KEscape = '\\';

/*
* Characters considered as EOS.
*/
/// '\r' character.
LOCAL_C const TUint KCr = '\r';
/// '\n' character.
LOCAL_C const TUint KLf = '\n';

/*
* Separator characters.
*/
/// End-Of-String (0) character. It is a separator.
LOCAL_C const TUint KEos = 0;
/// Record separator (30) character.
LOCAL_C const TUint KRecordSeparator = 30;
/// Unit separator (31) character.
LOCAL_C const TUint KUnitSeparator = 31;

/*
* Keywords.
*/
/// "Ipv4" keyword.
_LIT( KIpv4, "Ipv4" );
/// "Ipv6" keyword.
_LIT( KIpv6, "Ipv6" );
/// "E164" keyword.
_LIT( KE164, "E164" );
/// "Alpha" keyword.
_LIT( KAlpha, "Alpha" );


// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CWhiteListConverter::NewL
// ---------------------------------------------------------
//
CWhiteListConverter* CWhiteListConverter::NewL( CPushInitiatorList& aPushInitiatorList )
    {
    CWhiteListConverter* converter = 
        new (ELeave) CWhiteListConverter( aPushInitiatorList );
    CleanupStack::PushL( converter );
    converter->ConstructL();
    CleanupStack::Pop( converter );    // converter
    return converter;
    }

// ---------------------------------------------------------
// CWhiteListConverter::~CWhiteListConverter
// ---------------------------------------------------------
//
CWhiteListConverter::~CWhiteListConverter()
    {
    delete iConverterBuf;
    delete iBuf;
    iNextCh = NULL; // Only used.
    iMaxCh = NULL; // Only used.
    }

// ---------------------------------------------------------
// CWhiteListConverter::Buffer2ListL
// ---------------------------------------------------------
//
void CWhiteListConverter::Buffer2ListL( const TDesC& aSource )
    {
    PUSHLOG_ENTERFN("CWhiteListConverter::Buffer2ListL");

    iSource.Assign( aSource );
    iCurCh = KRecordSeparator; // Cannot be 0, because of GetChar()
    GetChar();
    while( NextLineL() )
        {
        };

    PUSHLOG_LEAVEFN("CWhiteListConverter::Buffer2ListL");
    }

// ---------------------------------------------------------
// CWhiteListConverter::List2BufferL
// ---------------------------------------------------------
//
HBufC* CWhiteListConverter::List2BufferL()
    {
    PUSHLOG_ENTERFN("CWhiteListConverter::List2BufferL");

    HBufC* newConverterBuf = KNullDesC().AllocL(); // initial zero length buffer
    delete iConverterBuf;
    iConverterBuf = newConverterBuf;
    TPtr writableConvBuf = iConverterBuf->Des();
    writableConvBuf.SetLength(0);
    TInt itemsToExport = iPushInitiatorList.Count();
    // temp buffers
    TBuf<16> type;
    TBuf<16> entryId;

    for ( TInt i=0; i < itemsToExport; ++i )
        {
        CPushInitiator& curr = iPushInitiatorList.At(i);
        switch ( curr.Type() )
            {
            case CPushInitiator::ETypeIpv4: type=KIpv4; break;
            case CPushInitiator::ETypeIpv6: type=KIpv6; break;
            case CPushInitiator::ETypeE164: type=KE164; break;
            case CPushInitiator::ETypeAlpha: type=KAlpha; break;
            default: type=KE164; break;
            }
        entryId.Format( _L("%d"), curr.EntryID() );
        
        // check available space
        // 3 is required for 2 commas and 1 semicolon.
        TInt newItemLength = curr.Addr().Length()+type.Length()+entryId.Length()+3;
        if ( writableConvBuf.MaxLength() < writableConvBuf.Length() + newItemLength )
            {
            // reallocate a greater buffer
            TInt currentMaxLength = writableConvBuf.MaxLength();
            iConverterBuf = iConverterBuf->ReAllocL( currentMaxLength + newItemLength );
            writableConvBuf.Set( iConverterBuf->Des() ); // re-initialize
            }
            
        writableConvBuf.Append( curr.Addr() );
        writableConvBuf.Append( TChar(KComma) );
        writableConvBuf.Append( type );
        writableConvBuf.Append( TChar(KComma) );
        writableConvBuf.Append( entryId );
        if ( i+1 < itemsToExport )
            {
            // there are still items to add
            writableConvBuf.Append( TChar(KSemicolon) );
            }
        }

    HBufC* ret = iConverterBuf; // ownersip is transferred to the caller.
    iConverterBuf = 0;

    PUSHLOG_LEAVEFN("CWhiteListConverter::List2BufferL");
    return ret;
    }

// ---------------------------------------------------------
// CWhiteListConverter::CWhiteListConverter
// ---------------------------------------------------------
//
CWhiteListConverter::CWhiteListConverter( CPushInitiatorList& aPushInitiatorList ) 
:   iPushInitiatorList( aPushInitiatorList )
    {
    }

// ---------------------------------------------------------
// CWhiteListConverter::ConstructL
// ---------------------------------------------------------
//
void CWhiteListConverter::ConstructL()
    {
    iBuf = new (ELeave) TText[KMaxLength];
    iMaxCh = iBuf + KMaxLength;
    }

// ---------------------------------------------------------
// CWhiteListConverter::GetChar
// ---------------------------------------------------------
//
inline void CWhiteListConverter::GetChar()
    {
    // Do not continue if we already reached EOS:
    if ( iCurCh == KEos )
        {
        return;
        }

    iCurCh = iSource.Get();

    if ( iCurCh == KCr || iCurCh == KLf )
        {
        // They are considered as EOS.
        iCurCh = KEos;
        }
    else if ( iCurCh == KEscape )
        {
        // Drop the Escape character and get the next as is.
        iCurCh = iSource.Get();
        }
    else if ( iCurCh == KComma )
        {
        // It is a Unit Separator.
        iCurCh = KUnitSeparator;
        }
    else if ( iCurCh == KSemicolon )
        {
        // It is a Record Separator.
        iCurCh = KRecordSeparator;
        }
    }

// ---------------------------------------------------------
// CWhiteListConverter::NextLineL
// ---------------------------------------------------------
//
TBool CWhiteListConverter::NextLineL()
    {
    switch( iCurCh )
        {
        case KEos:
            // EOS
            return EFalse;

        default:
            // Parse White List attributes and process them.
            AttrsL();
            break;
        }

    return ETrue;
    }

// ---------------------------------------------------------
// CWhiteListConverter::NextTokenL
// ---------------------------------------------------------
//
TPtrC CWhiteListConverter::NextTokenL( TUint aStopChar )
    {
    iNextCh = iBuf; // Start storing token at start of buffer.
    while ( iNextCh < iMaxCh )
        {
        if ( iCurCh == aStopChar || iCurCh == KEos )
            {
            // Stop character found - return what we have stored so far. 
            // This may be an empty string as well.
            return TPtrC( iBuf, iNextCh - iBuf );
            }
        else
            {
            *iNextCh = STATIC_CAST( TText16, iCurCh );
            iNextCh++;
            GetChar();
            }
        }
    // No more space in buffer to store token.
    User::Leave( KErrOverflow );
    /*NOTREACHED*/
    return TPtrC();
    }

// ---------------------------------------------------------
// CWhiteListConverter::AttrsL
// ---------------------------------------------------------
//
void CWhiteListConverter::AttrsL()
    {
    CPushInitiator* newInitiator = new (ELeave) CPushInitiator;
    CleanupStack::PushL( newInitiator );

    TPtrC token;

    // Address.
    token.Set( NextTokenL( KUnitSeparator ) );
    HBufC* address = token.AllocLC(); // Copy the token to a separate area.
    GetChar();

    // Address type and/or entry ID.
    token.Set( NextTokenL( KRecordSeparator ) );
    HBufC* addressTypeBuf = 0;
    HBufC* entryIdBuf = 0;
    // check if it contains an entry ID separated with a comma (KUnitSeparator)
    TInt offset = token.Locate( TChar(KUnitSeparator) );
    if ( offset == KErrNotFound )
        {
        // the whole token is address type
        addressTypeBuf = token.AllocLC(); // Copy the token to a separate area.
        entryIdBuf = 0;
        }
    else
        {
        addressTypeBuf = token.Mid(0,offset).AllocLC();
        TInt tokenLength = token.Length();
        entryIdBuf = token.Mid((offset+1),tokenLength-(offset+1)).AllocLC();
        }
    
    // Convert the address type string to enum.
    // addressTypeBuf must not be NULL.
    CPushInitiator::TAddrType addrType = CPushInitiator::ETypeE164;
    if      ( !(*addressTypeBuf).Compare( KIpv4 ) )
        {
        addrType = CPushInitiator::ETypeIpv4;
        }
    else if ( !(*addressTypeBuf).Compare( KIpv6 ) )
        {
        addrType = CPushInitiator::ETypeIpv6;
        }
    else if ( !(*addressTypeBuf).Compare( KE164 ) )
        {
        addrType = CPushInitiator::ETypeE164;
        }
    else if ( !(*addressTypeBuf).Compare( KAlpha ) )
        {
        addrType = CPushInitiator::ETypeAlpha;
        }
    else
        {
        User::Leave( KErrCorrupt );
        }
    PUSHLOG_WRITE_FORMAT2("WL Importer: <%S> <%S>",&(address->Des()),
                                                   &(addressTypeBuf->Des()));
        
    TUint32 entryId(0);
    if ( entryIdBuf )
        {
        PUSHLOG_WRITE_FORMAT("WL Importer entryIdBuf: <%S>",&(entryIdBuf->Des()));
        TLex lex( *entryIdBuf );
        lex.Val( entryId, EDecimal );
        CleanupStack::PopAndDestroy( entryIdBuf ); // entryIdBuf
        }
    
    GetChar();

    // No more attribute to parse.

    // Add the new initiator data to the Push settings.
    newInitiator->SetAddressL( *address, addrType );
    newInitiator->SetEntryID( entryId );
    CleanupStack::PopAndDestroy( 2, address ); // addressTypeBuf, address

    iPushInitiatorList.AddL( newInitiator );
    CleanupStack::Pop( newInitiator ); // newInitiator
    }

