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



#ifndef BROWSER_COMMAND_LINE_PARSER_H
#define BROWSER_COMMAND_LINE_PARSER_H

// INCLUDES
#include <E32Base.h>

// CLASS DECLARATION

/**
*  CBrowserCommandLineParser8 class
*  Purpose: parsing command lines could be used in view activation.
*  @lib Browser.app
*  @since Series 60 1.2
*/
class CBrowserCommandLineParser8 : public CBase
	{
	public:     //Constructors and destructor
        
        /**
        * Two-phased constructor.
        * @param aParamString String of parameters
        */
		static CBrowserCommandLineParser8* NewL( const TDesC8& aParamString );
		
        /**
        * Two-phased constructor. Cleanup stack used.
        * @param aParamString String of parameters
        */
        static CBrowserCommandLineParser8* NewLC( const TDesC8& aParamString );
		
        /**
        * C++ Default constructor.
        */
		CBrowserCommandLineParser8();
        
        /**
        * Destructor.
        */
		~CBrowserCommandLineParser8();
		
    public:     // New functions
        
        /**
        * Count the parameters.
        * @since Series 60 1.2
        * @return Count of parameters
        */
		TUint Count() const;
		
        /**
        * Set parameters to member variable.
        * @since Series 60 1.2
        * @param aParamString String of parameters
        */
		void SetL( const TDesC8&  aParamString );
		
        /**
        * Get pointer to a certain parameter in parameter array.
        * @since Series 60 1.2
        * @param aIndex Index from where to get the parameter
        */
		TPtrC8 Param( TUint aIndex );
		
        /**
        * Check whether the parameter is integer or not.
        * @since Series 60 1.2
        * @param aIndex Index from where to get the parameter
        * @return ETrue if the parameter is integer
        */
		TBool IntegerParam(  TUint aIndex );
		
        /**
        * Extract a signed integer from parameter array.
        * @since Series 60 1.2
        * @param aIndex Index from where the parameter is extracted
        * @return Parameter as signed integer
        */
		TInt ParamToInteger( TUint aIndex );

	 
	private:
        
        /**
        * Symbian 2nd phase constructor.
        */
		void ConstructL( const TDesC8& aParamString );

        /**
        * Fill array with parameters. 
        * Strips any white spaces.
        * @since Series 60 1.2
        */
		void FillUpParamsL();
    
    private:    // Data

		HBufC8* iParamString ; //copy created in ConstructL

		CArrayFixFlat<TPtrC8>* iParams; //array of parameters
	};

#endif // BROWSER_COMMAND_LINE_PARSER_H// End of File