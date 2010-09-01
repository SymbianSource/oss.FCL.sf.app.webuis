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
*      Implementation of CSessionAndSecurity
*  
*
*/


// INCLUDE FILES

#include "Browser.hrh"
#include <es_sock.h>
#include "SessionAndSecurity.h"
#include <mconnection.h>
#include <BrowserNG.rsg>
#include <AknQueryDialog.h>
#include <AknMessageQueryDialog.h>
#include <StringLoader.h>
#include <FeatMgr.h>

#include "BrowserUtil.h"
#include "ApiProvider.h"
#include <ssl.h> // for TCertInfo
#include <brctlinterface.h>

// CONSTANTS
const   TInt KWmlSessionInfoMaxLength = 1000;
const   TInt KWmlSecurityInfoMaxLength = 1000;
const   TInt KMaxLengthTextDateString = 16;
_LIT( KWmlConnSpeed9600, "9600" );
_LIT( KWmlConnSpeed14400, "14400" );
_LIT( KWmlConnSpeed19200, "19200" );
_LIT( KWmlConnSpeed28800, "28800" );
_LIT( KWmlConnSpeed38400, "38400" );
_LIT( KWmlConnSpeed43200, "43200" );
_LIT( KWmlNewLine,  "\n" );
_LIT( KWmlSeparatorComma, "," );

// ================= MEMBER FUNCTIONS =======================

// ----------------------------------------------------
// CSessionAndSecurity::CSessionAndSecurity
// ----------------------------------------------------
//
CSessionAndSecurity::CSessionAndSecurity(MApiProvider& aApiProvider)
: iApiProvider(aApiProvider)
    {
    }

//-----------------------------------------------------------------------------
// CSessionAndSecurity::NewL
//-----------------------------------------------------------------------------
//
CSessionAndSecurity* CSessionAndSecurity::NewL(MApiProvider& aApiProvider)
    {
    CSessionAndSecurity* self = CSessionAndSecurity::NewLC(aApiProvider);
    CleanupStack::Pop();
    return self;
    }

//-----------------------------------------------------------------------------
// CSessionAndSecurity::NewLC
//-----------------------------------------------------------------------------
//
CSessionAndSecurity* CSessionAndSecurity::NewLC(MApiProvider& aApiProvider)
    {
    CSessionAndSecurity* self = new(ELeave) CSessionAndSecurity(aApiProvider);
    CleanupStack::PushL(self);
    self->ConstructL();
    return self;
    }

//-----------------------------------------------------------------------------
// CSessionAndSecurity::~CSessionAndSecurity
//-----------------------------------------------------------------------------
//
CSessionAndSecurity::~CSessionAndSecurity()
    {
    }

// ----------------------------------------------------
// CSessionAndSecurity::ConstructL
// ----------------------------------------------------
//
void CSessionAndSecurity::ConstructL()
    {
    }

// ----------------------------------------------------
// CSessionAndSecurity::ShowPageInfoL
// ----------------------------------------------------
//
void CSessionAndSecurity::ShowPageInfoL()
    {
    HBufC* messagebody = NULL;
    HBufC* header = StringLoader::LoadLC( R_BROWSER_QUERY_CURRENT_PAGE );
    HBufC* urltxt = StringLoader::LoadLC( R_BROWSER_QUERY_CURRENT_PAGE_URL );
    HBufC* currentUrl = ApiProvider().BrCtlInterface().
        PageInfoLC( TBrCtlDefs::EPageInfoUrl );
    if( currentUrl == NULL )
        {
        CleanupStack::PopAndDestroy( currentUrl );
        currentUrl = KNullDesC().AllocLC();
        }
    TInt length = currentUrl->Length();
    messagebody = HBufC::NewLC( length + urltxt->Length()+2 + KWmlSecurityInfoMaxLength);
    messagebody->Des().Append( *urltxt );
    messagebody->Des().Append( KWmlNewLine() );
    messagebody->Des().Append( *currentUrl );
    
    
    // append security info
    const TCertInfo* certInfo = ApiProvider().BrCtlInterface().CertInfo();
    HBufC* message = HBufC::NewLC( KWmlSecurityInfoMaxLength ); 
    HttpSecurityInfoL( certInfo, *message);
    messagebody->Des().Append( KWmlNewLine() );
    messagebody->Des().Append( KWmlNewLine() );
    messagebody->Des().Append( *message );


    // output page and security info to dialog
    CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL( *messagebody );
    dlg->PrepareLC( R_BROWSER_PAGE_INFO );
    CAknPopupHeadingPane* hPane = dlg->QueryHeading();
    if ( hPane )
        {
        hPane->SetTextL( *header );
        }
    dlg->RunLD();
    CleanupStack::PopAndDestroy( 5 );   // header, urltxt, currentUrl, messagebody, message
    }

// ----------------------------------------------------
// CSessionAndSecurity::ShowSessionInfoL
// ----------------------------------------------------
//
void CSessionAndSecurity::ShowSessionInfoL()
    {
    HBufC* header = StringLoader::LoadLC( R_WML_SESSION_VIEW_TITLE );
    HBufC* message = HBufC::NewLC( KWmlSessionInfoMaxLength );

    HBufC* value = StringLoader::LoadLC( R_WML_SESSION_VIEW_AP );
    message->Des().Append( *value );
    message->Des().Append( TChar( '\n' ) );
    CleanupStack::PopAndDestroy();  // value

    value = ApiProvider().Connection().CurrentApNameLC();
    message->Des().Append( value->Des() );
    message->Des().Append( TChar( '\n' ) ); 
    CleanupStack::PopAndDestroy();  // value

    //
    // Bearer type
    //
    value = StringLoader::LoadLC( R_WML_SESSION_VIEW_BT );
    message->Des().Append( *value );
    message->Des().Append( TChar( '\n' ) );
    CleanupStack::PopAndDestroy();  // value

    TApBearerType bt = ApiProvider().Connection().CurrentBearerTypeL();
    switch ( bt )
    {
    case EApBearerTypeCSD:
        {
        value = StringLoader::LoadLC( R_WML_BEARER_DATA_CALL );
        break;
        }
    case EApBearerTypeGPRS:
        {
        value = StringLoader::LoadLC( R_WML_BEARER_PACKET_DATA );
        break;
        }
    case EApBearerTypeHSCSD:
        {
        value = StringLoader::LoadLC( R_WML_BEARER_HSCSD );
        break;
        }
    default:
        value = HBufC::NewLC( 1 );
        break;
    }

    message->Des().Append( value->Des() );
    message->Des().Append( TChar( '\n' ) ); 
    CleanupStack::PopAndDestroy();  // value

    //
    // Max conn. speed
    //
    value = StringLoader::LoadLC( R_WML_SESSION_VIEW_CS );
    message->Des().Append( *value );
    message->Des().Append( TChar( '\n' ) );
    CleanupStack::PopAndDestroy();  // value
    TApCallSpeed speed = ApiProvider().Connection().CurrentConnectionSpeed();
    value = HBufC::NewLC( sizeof( KWmlConnSpeed43200 ) );
    switch ( speed )
    {
	case KSpeed9600:
        {
        *value = KWmlConnSpeed9600();
        break;
        }
    case KSpeed14400:
        {
        *value = KWmlConnSpeed14400();
        break;
        }
    case KSpeed19200:
        {
        *value = KWmlConnSpeed19200();
        break;
        }
    case KSpeed28800:
        {
        *value = KWmlConnSpeed28800();
        break;
        }
    case KSpeed38400:
        {
        *value = KWmlConnSpeed38400();
        break;
        }
    case KSpeed43200:
        {
        *value = KWmlConnSpeed43200();
        break;
        }
	case KSpeedAutobaud:	// default value
    default:
        {
        CleanupStack::PopAndDestroy();  // value
		value = StringLoader::LoadLC( R_WML_CONN_SPEED_AD );
        break;
        }
    }
    
    message->Des().Append( value->Des() );
    message->Des().Append( TChar( '\n' ) ); 
    CleanupStack::PopAndDestroy();  // value

    CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL( *message );
    dlg->PrepareLC( R_BROWSER_SESSION_INFO );
    CAknPopupHeadingPane* hPane = dlg->QueryHeading();
    if ( hPane )
        {
        hPane->SetTextL( *header );
        }
    dlg->RunLD();
    CleanupStack::PopAndDestroy( 2 );   // header, message
    }

// ----------------------------------------------------------------------------
// CSessionAndSecurity::HttpSecurityInfoL
// ----------------------------------------------------------------------------
//
void CSessionAndSecurity::HttpSecurityInfoL( const TCertInfo* aCertInfo,
															HBufC& aMessage )
    {
    HBufC* value;
    HBufC* prompt; 
		
    if ( !aCertInfo )
        {
        // Connection: Unsecure
        value = StringLoader::LoadLC( R_WML_SECURITY_CONN_UNSECURE );
        prompt = StringLoader::LoadLC( R_WML_SECURITY_CONN_TYPE, *value );
        aMessage.Des().Append( *prompt );
        aMessage.Des().Append( TChar( '\n' ) );
		aMessage.Des().Append( TChar( '\n' ) );
        CleanupStack::PopAndDestroy(2);   // value, prompt
        }        
    else
        {
        // Connection: Secure
        value = StringLoader::LoadLC( R_WML_SECURITY_CONN_SECURE );
        prompt = StringLoader::LoadLC( R_WML_SECURITY_CONN_TYPE, *value );
        aMessage.Des().Append( *prompt );
        aMessage.Des().Append( TChar( '\n' ) );   
		aMessage.Des().Append( TChar( '\n' ) );
        CleanupStack::PopAndDestroy(2);   // value, prompt

        HBufC* buf2 = HBufC::NewMaxLC(256);
        
		// Server authentication
		value = StringLoader::LoadLC( R_WML_SECURITY_SERVER_AUT, KNullDesC ); 
		aMessage.Des().Append( *value );
		aMessage.Des().Append( TChar( '\n' ) );
		CleanupStack::PopAndDestroy(); //value
		
        // Subject
        FillDNinfoL( aCertInfo->iSubjectDNInfo, buf2 );
        value = StringLoader::LoadLC( R_WML_SECURITY_CERT_OWNER, buf2->Des() );
        aMessage.Des().Append( *value );
        aMessage.Des().Append( TChar( '\n' ) );
		aMessage.Des().Append( TChar( '\n' ) );
        CleanupStack::PopAndDestroy(2); //value, buf2

        //Issuer
        buf2 = HBufC::NewMaxLC(256);
        FillDNinfoL( aCertInfo->iIssuerDNInfo, buf2 );
        value = StringLoader::LoadLC( R_WML_SECURITY_INFO_ISSUER, buf2->Des() );
        aMessage.Des().Append( *value );
        aMessage.Des().Append( TChar( '\n' ) );
		aMessage.Des().Append( TChar( '\n' ) );
        CleanupStack::PopAndDestroy(2); //value, buf2

        // Valid from
        TBuf< KMaxLengthTextDateString > validFromString;
        HBufC* dateFormatString = StringLoader::LoadLC( R_QTN_DATE_USUAL_WITH_ZERO );
        aCertInfo->iStartValDate.FormatL( validFromString, *dateFormatString );
        CleanupStack::PopAndDestroy(); //dateFormatString
        value = StringLoader::LoadLC( R_WML_SECURITY_VALID_FROM, validFromString );
        aMessage.Des().Append( *value );
        aMessage.Des().Append( TChar( '\n' ) );
		aMessage.Des().Append( TChar( '\n' ) );
        CleanupStack::PopAndDestroy(); //value

        // Valid to
        dateFormatString = StringLoader::LoadLC( R_QTN_DATE_USUAL_WITH_ZERO );
        aCertInfo->iEndValDate.FormatL( validFromString, *dateFormatString );
        CleanupStack::PopAndDestroy(); //dateFormatString
        value = StringLoader::LoadLC( R_WML_SECURITY_VALID_TO, validFromString );
        aMessage.Des().Append( *value );
        aMessage.Des().Append( TChar( '\n' ) );
		aMessage.Des().Append( TChar( '\n' ) );
        CleanupStack::PopAndDestroy(); //value

        // hex formatting
        TBuf<2> buf;
        _LIT( KHexFormat, "%02X");

        // Serial number
        HBufC* buffer = HBufC::NewLC( aCertInfo->iSerialNo.Length()*2 );
        for (TInt i = 0; i < aCertInfo->iSerialNo.Length(); i++)
            {
            buf.Format( KHexFormat, aCertInfo->iSerialNo[i] );
            buffer->Des().Append( buf );
            }
        value = StringLoader::LoadLC( R_WML_SECURITY_SERIALNO, *buffer );
        aMessage.Des().Append( *value );
        aMessage.Des().Append( TChar( '\n' ) );
		aMessage.Des().Append( TChar( '\n' ) );
        CleanupStack::PopAndDestroy( 2 ); // value, buffer

        // Fingerprint
			{
			TInt emptyChars = aCertInfo->iFingerprint.Length()/2;
			emptyChars++;
			HBufC* buffer = HBufC::NewLC( aCertInfo->iFingerprint.Length()*2 + emptyChars );
			TInt ind = 1;
			for (TInt i = 0; i < aCertInfo->iFingerprint.Length(); i++)
				{					
				buf.Format( KHexFormat, aCertInfo->iFingerprint[i] );
				buffer->Des().Append( buf );
				if ( ind == 2 )
					{
					buffer->Des().Append( TChar( ' ' ) );
					ind = 0;
					}
				ind++;
				}
			value = StringLoader::LoadLC( R_WML_SECURITY_FINGERPRINT, *buffer );
			aMessage.Des().Append( *value );
			CleanupStack::PopAndDestroy( 2 ); // value, buffer
			}
		}

    }

// ----------------------------------------------------
// CSessionAndSecurity::ConvertTDesC8ToHBufC16LC
// ----------------------------------------------------
//
void CSessionAndSecurity::ConvertTDesC8ToHBufC16LC( TDesC8& aSource, 
                                                   HBufC16*& aDestination)
    {
    const TInt length = aSource.Length(); // length of name 
    TPtrC8 ptr ( aSource.Ptr(), aSource.Length() ); 
    aDestination = HBufC::NewLC( length );
    aDestination->Des().Copy( ptr );
    }

// ----------------------------------------------------
// CSessionAndSecurity::FillDNinfoL
// ----------------------------------------------------
//
void CSessionAndSecurity::FillDNinfoL( TDNInfo aDNInfo, HBufC* aBuf )
    {
    HBufC16* buf2 = NULL;
    TInt num = 0;
    if ( aDNInfo.iCommonName.Length() )
        {
        aBuf->Des().Copy( aDNInfo.iCommonName );
        num++;
        }
    if ( aDNInfo.iOrganization.Length() )
        {
        if ( !num )
            {
            aBuf->Des().Copy( aDNInfo.iOrganization );
            }
        else
            {
            aBuf->Des().Append( KWmlSeparatorComma() );
            ConvertTDesC8ToHBufC16LC( aDNInfo.iOrganization, buf2 );
            aBuf->Des().Append( buf2->Ptr(),aDNInfo.iOrganization.Length()  );
            CleanupStack::PopAndDestroy(); //buf2
            }
        num++;
        }
    if ( aDNInfo.iOrganizationUnit.Length() && num < 2 )
        {
        if ( !num )
            {
            aBuf->Des().Copy( aDNInfo.iOrganizationUnit );
            }
        else
            {
            aBuf->Des().Append( KWmlSeparatorComma() );
            ConvertTDesC8ToHBufC16LC( aDNInfo.iOrganizationUnit, buf2 );
            aBuf->Des().Append( buf2->Ptr(),aDNInfo.iOrganizationUnit.Length() );
            CleanupStack::PopAndDestroy(); //buf2
            }
        num++;
        }
    if ( aDNInfo.iLocality.Length() && num < 2 )
        {
        if ( !num )
            {
            aBuf->Des().Copy( aDNInfo.iLocality );
            }
        else
            {
            aBuf->Des().Append( KWmlSeparatorComma() );
            ConvertTDesC8ToHBufC16LC( aDNInfo.iLocality, buf2 );
            aBuf->Des().Append( buf2->Ptr(),aDNInfo.iLocality.Length() );
            CleanupStack::PopAndDestroy(); //buf2
            }
        num++;
        }
    if ( aDNInfo.iCountry.Length() && num < 2 )
        {
        if ( !num )
            {
            aBuf->Des().Copy( aDNInfo.iCountry );
            }
        else
            {
            aBuf->Des().Append( KWmlSeparatorComma() );
            ConvertTDesC8ToHBufC16LC( aDNInfo.iCountry, buf2 );
            aBuf->Des().Append( buf2->Ptr(),aDNInfo.iCountry.Length() );
            CleanupStack::PopAndDestroy(); //buf2
            }
        }   
    }

// ----------------------------------------------------
// CSessionAndSecurity::ShowSecurityInfoL
// ----------------------------------------------------
//
void CSessionAndSecurity::ShowSecurityInfoL()
    {
	const TCertInfo* certInfo = ApiProvider().BrCtlInterface().CertInfo();

    HBufC* header;
    HBufC* message = HBufC::NewLC( KWmlSecurityInfoMaxLength ); 

    header = StringLoader::LoadLC( R_WML_SECURITY_VIEW_TITLE );    
    HttpSecurityInfoL( certInfo, *message);

    CAknMessageQueryDialog* dlg = CAknMessageQueryDialog::NewL( *message );

    dlg->PrepareLC( R_BROWSER_SECURITY_INFO );
    CAknPopupHeadingPane* hPane = dlg->QueryHeading();
    if ( hPane )
        {
        hPane->SetTextL( *header );
        }

    dlg->RunLD();
    CleanupStack::PopAndDestroy( 2 ); // header, message,
    }

// End of File
