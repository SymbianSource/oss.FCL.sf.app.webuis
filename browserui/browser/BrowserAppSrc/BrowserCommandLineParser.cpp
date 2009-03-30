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
*      parsing command lines could be used in view activation
*      
*
*/


// INCLUDE FILES
#include "BrowserCommandLineParser.h"
#include "Logger.h"

LOCAL_D const TInt KParamsArrayGranularity = 8;

// ========================= MEMBER FUNCTIONS ================================

// ---------------------------------------------------------------------------
// CBrowserCommandLineParser8::CBrowserCommandLineParser8()
// ---------------------------------------------------------------------------
CBrowserCommandLineParser8::CBrowserCommandLineParser8()
	{
	}

// ---------------------------------------------------------------------------
// CBrowserCommandLineParser8::~CBrowserCommandLineParser8
// ---------------------------------------------------------------------------
CBrowserCommandLineParser8::~CBrowserCommandLineParser8()
    {
    LOG_ENTERFN("CBrowserCommandLineParser8::~CBrowserCommandLineParser8");
    delete ( iParamString );
    delete ( iParams );
    }

// ---------------------------------------------------------------------------
// CBrowserCommandLineParser8::NewL
// ---------------------------------------------------------------------------
CBrowserCommandLineParser8* CBrowserCommandLineParser8::NewL( 
                                                const TDesC8& aParamString )
    {
    CBrowserCommandLineParser8* self = 
                            CBrowserCommandLineParser8::NewLC( aParamString );
    CleanupStack::Pop();
    return self;
    }

// ---------------------------------------------------------------------------
// CBrowserCommandLineParser8::NewLC
// ---------------------------------------------------------------------------
CBrowserCommandLineParser8* CBrowserCommandLineParser8::NewLC( 
                                                const TDesC8& aParamString )
    {
    LOG_ENTERFN("CBrowserCommandLineParser8::NewLC");
    CBrowserCommandLineParser8* self = 
                                new ( ELeave ) CBrowserCommandLineParser8 ( );
    CleanupStack::PushL( self );
    self->ConstructL( aParamString );
    return self;
    }

// ---------------------------------------------------------------------------
// CBrowserCommandLineParser8::ConstructL
// ---------------------------------------------------------------------------
void CBrowserCommandLineParser8::ConstructL( const TDesC8& aParamString )
    {
    LOG_ENTERFN("CBrowserCommandLineParser8::ConstructL");
    TInt length = aParamString.Length();
    iParamString = HBufC8::NewL( length );//the maxlength of buf = the length of the param
    *iParamString = aParamString;
    iParams = new( ELeave ) CArrayFixFlat<TPtrC8>( KParamsArrayGranularity );
    FillUpParamsL();
    }

// ---------------------------------------------------------------------------
// CBrowserCommandLineParser8::Count
// ---------------------------------------------------------------------------
TUint CBrowserCommandLineParser8::Count() const
    {
    LOG_ENTERFN("CBrowserCommandLineParser8::Count");
    return iParams->Count();
    }

// ---------------------------------------------------------------------------
// CBrowserCommandLineParser8::SetL
// ---------------------------------------------------------------------------
void CBrowserCommandLineParser8::SetL( const TDesC8& aParamString )
    {
    LOG_ENTERFN("CBrowserCommandLineParser8::SetL");
    iParamString->ReAllocL( aParamString.Length() );
    *iParamString = aParamString;
    iParams->Reset();
    FillUpParamsL();
    }

// ---------------------------------------------------------------------------
// CBrowserCommandLineParser8::Param
// ---------------------------------------------------------------------------
TPtrC8 CBrowserCommandLineParser8::Param( TUint aIndex )
    {
    LOG_ENTERFN("CBrowserCommandLineParser8::Param");
    TPtrC8 retVal;
    ( aIndex < Count( ) )? ( retVal.Set( iParams->At( aIndex ) ) ) : 
    		( retVal.Set( KNullDesC8 ) );
    return retVal;
    }

// ---------------------------------------------------------------------------
// CBrowserCommandLineParser8::IntegerParam
// ---------------------------------------------------------------------------
TBool CBrowserCommandLineParser8::IntegerParam( TUint aIndex )
    {
    LOG_ENTERFN("CBrowserCommandLineParser8::IntegerParam");
    if ( aIndex >= Count () )
        {
        return EFalse;
        }
    TLex8 lex( iParams->At ( aIndex ) );
    TInt a;
    if ( lex.Val( a ) == KErrNone )
        {
        return ETrue;
        }
    else
        {
        return EFalse;
        }
    }

// ---------------------------------------------------------------------------
// CBrowserCommandLineParser8::ParamToInteger
// ---------------------------------------------------------------------------
TInt CBrowserCommandLineParser8::ParamToInteger( TUint aIndex )
    {
    LOG_ENTERFN("CBrowserCommandLineParser8::ParamToInteger");
    TInt retInt = KMaxTInt;
    if ( aIndex < Count () )
        {
        TLex8 lex ( iParams -> At( aIndex ));
        lex.Val( retInt );
        }
    return retInt;	
    }

// ---------------------------------------------------------------------------
// CBrowserCommandLineParser8::FillUpParamsL
// ---------------------------------------------------------------------------
void CBrowserCommandLineParser8::FillUpParamsL()
    {
    LOG_ENTERFN("CBrowserCommandLineParser8::FillUpParamsL");
    TLex8 lex( *iParamString );
    while ( !lex.Eos() )
        {
        TPtrC8 token = lex.NextToken();
        iParams->AppendL( token );
        }
    }

//End of File
