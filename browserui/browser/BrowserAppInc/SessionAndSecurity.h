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
* Description:  Header file for CSessionAndSecurity
*
*
*/


#ifndef SESSION_SECURITY_H
#define SESSION_SECURITY_H

// INCLUDE FILES
#include <bldvariant.hrh>
#include <e32base.h>




// FORWARD DECLARATIONS

class MApiProvider;
class TCertInfo;
class TDNInfo;

// CLASS DECLARATION

/**
* CSessionAndSecurity
* @lib Browser.app
* @since Series60 1.2
*/
class CSessionAndSecurity : public CBase
	{
	public:		// Constructors and destructor					
		/**
        * Destructor.
        */
		~CSessionAndSecurity();

        /**
        * Two-phased constructor. Leaves on failure.
        * @return The constructed CSessionAndSecurity object.
        */		
		static CSessionAndSecurity* NewL(MApiProvider& aApiProvider);
		static CSessionAndSecurity* NewLC(MApiProvider& aApiProvider);

	public:		// New functions    
        /**
        * Shows session info dialog
        */
        void ShowSessionInfoL();

        /**
        * Shows security info dialog / certificate details
        */
        void ShowSecurityInfoL();

		/**
        * Shows page info dialog 
        */
        void ShowPageInfoL();

    private:
        
        /**
        * Converts TDesC8 buffer to HBufC16 
        * @since Series60 2.0
        * @param aSource
        * @param aDestination ownership passed to caller
        */                
        void ConvertTDesC8ToHBufC16LC( TDesC8& aSource, HBufC16*& aDestination );

        /**
        * Adds two distinguished names to security info 
        * (names are: CN, O, OU, L, C)
        * @since Series60 2.0
        * @param aDNInfo
        * @param aBuf
        */                
        void FillDNinfoL ( TDNInfo aDNInfo,  HBufC* aBuf );
        
        /**
        * Creates security info message for WPS connection 
        * @since Series60 2.0
        * @param aCertInfo
        * @param aMessage
        */        
        // void WspSecurityInfoL( TCertInfo* aCertInfo, HBufC& aMessage );
   
        /**
        * Creates security info message for HTTP connection
        * @since Series60 2.0
        * @param aCertInfo
        * @param aMessage
        */ 
		// TO DO: changed to fit in with BaP
        void HttpSecurityInfoL( const TCertInfo* aCertInfo, HBufC& aMessage );

		inline MApiProvider& ApiProvider() { return iApiProvider; }

    private:
        /**
        * Default constructor.
        */
        CSessionAndSecurity(MApiProvider& aApiProvider);

        /**
        * 2nd phase constructor.
        */
        void ConstructL();

	private:	// Data
        MApiProvider& iApiProvider;

	};


#endif

// End of File
