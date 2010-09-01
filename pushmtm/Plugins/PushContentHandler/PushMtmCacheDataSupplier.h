/*
* Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
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
*
*/
/*
* ==============================================================================
*  Name        : PushMtmCacheDataSupplier.h
*  Part of     : PushMtm
*  Interface   : 
*  Description : 
*  Version     : 3.1
*/


#ifndef CPUSHMTMCACHEDATASUPPLIER_H
#define CPUSHMTMCACHEDATASUPPLIER_H

//  INCLUDES
#include <http/MHTTPDataSupplier.h>

// CONSTANTS

// MACROS

// DATA TYPES

// FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// CLASS DECLARATION

/**
 *  Data supplier to supply response body if the url is loaded
 *  from cache
 *
 *  @lib 
 *  @since 3.1
 */
class CPushMtmCacheDataSupplier : public CBase, public MHTTPDataSupplier
    {
    public:  // Constructors and destructor
        
        /**
        * Two-phased constructor.
        */
        static CPushMtmCacheDataSupplier* NewL( HBufC8* aBody );
        
        /**
        * Destructor.
        */
        virtual ~CPushMtmCacheDataSupplier();      
     
    public: // from MHTTPDataSupplier
        
        /**
        * From MHTTPDataSupplier. Free the data chunk that was requested.
        * @since 2.0
        */
        virtual void ReleaseData() { delete iBody; iBody = NULL;}
        
        /**
        * From MHTTPDataSupplier. It is not possible to know
        * the data size, so return KErrNotFound
        * @since 2.0
        */
        virtual TInt OverallDataSize () { return KErrNotFound; }
        
        /**
        * From MHTTPDataSupplier. Get the next data part of the response body
        * @since 2.0
        */
        virtual TBool GetNextDataPart( TPtrC8& aDataPart );
        
        /**
        * From MHTTPDataSupplier. Not Supported
        * @since 2.0
        */
        virtual TInt Reset() { return KErrNotFound; }
        
    private:
        
        /**
        * C++ default constructor.
        */
        CPushMtmCacheDataSupplier( HBufC8* aBody );
        
    private:    // Data
        
        //owned by datasupplier and is destroyed 
        //when the releasedata function is called
        HBufC8* iBody;        
    };

#endif      // CPUSHMTMCACHEDATASUPPLIER_H

// End of File
