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
* Description:  Header file for CBrowserPopupEngine
*
*
*/


#ifndef __BROWSERPOPUPENGINE_H
#define __BROWSERPOPUPENGINE_H

// INCLUDE FILES 
#include <f32file.h>
#include <e32base.h>
#include <badesca.h>

// CLASS DECLARATION

/**
*  CBrowserPopupEngine.
* 
*  @lib Browser.app
*  @since Series 60 3.0 
*/
class CBrowserPopupEngine : public CBase
{
	public:// Constructors and destructor
        /**
        * Default constructor.
        */
    	CBrowserPopupEngine();
        /**
        * Destructor.
        */
    	~CBrowserPopupEngine();
        static CBrowserPopupEngine* NewLC();
        static CBrowserPopupEngine* NewL();	
    public:// New functions
        /**
        * Adds url to white list
        * @param aUrl url to ad to white list
        */
    	void AddUrlToWhiteListL(const TDesC& aUrl);
        /**
        * Checks whether the url is on the white list
        * @param aUrl url to check
        * @return ETrue if the url is on white list
        */
    	TBool IsUrlOnWhiteListL(const TDesC& aUrl);

        void RemoveUrlFromWhiteListL(const TDesC& aUrl);
   	
	private:
    	void ConstructL();
        /**
        * Checks whether the url is in white cache
        * @param aUrl url to check
        * @return ETrue if the url is in white cache
        */
        TBool IsUrlInWhiteCacheL(const TDesC& aUrl);
        /**
        * Checks whether the url is in the white file
        * @param aUrl url to check
        * @return ETrue if the url is in white file
        */
        TBool IsUrlInWhiteFileL(const TDesC& aUrl);
        /**
        * Checks whether the url is in file
        * @param aUrl url to check
        * @return ETrue if the url is in file
        */
        TBool IsUrlInFileL(const RFile* aFile, const TDesC& aUrl);
        /**
        * Checks whether the url is in the cache
        * @param aUrl url to check
        * @return ETrue if the url is in cache
        */
        TBool IsUrlInCacheL( const CDesCArrayFlat* aCacheArray, const TDesC& aUrl, const TInt aLoadedurlno );
        /**
        * Loads urls to cache
        * @param aFile file where from the urls will be loaded
        * @param aCacheArray where to load the urls
        * @param aLoadedUrlNo number of loaded urls
        */
        void LoadUrlsToCacheL(const RFile* aFile, CDesCArrayFlat* aCacheArray, TInt* aLoadedUrlNo);
        /**
        * Adds url to cache
        * @param aCacheArray array where to add the url
        * @param aUrl url to add to cache
        */
        void AddUrlToCacheL( CDesCArrayFlat* aCacheArray, const TDesC& aUrl );
        /**
        * Converts TDesc16 to TPtrC8
        * @param aString string to convert
        * @return a TPtrC8
        */
        TPtrC8 TDesC16ToTPtrC8(const TDesC16 &aString);
        /**
        * Converts TDesc8 to TPtrC16
        * @param aString string to convert
        * @return a TPtrC16
        */
        TPtrC16 TDesC8ToTPtrC16(const TDesC8 &aString);
        /**
        * Converts TDesc16 to HBufC8
        * @param aString string to convert
        * @return a pointer to HBufC8
        */       
        HBufC8 *TDesC16ToHBufC8LC(const TDesC16 &string);        
        /**
        * Converts TDesc8 to HBufC16
        * @param aString string to convert
        * @return a pointer to HBufC16
        */       
        HBufC16 *TDesC8ToHBufC16LC(const TDesC8 &aString);
        /**
        * Opens the database
        * @param aFile pointer to the file handle
        * @param aIsBlack whether the databas is black 
        * @return 
        */
        void OpenDatabaseL(RFile* aFile);
        /**
        * Handles order changes in the database
        * @param aFile database file
        * @param aUrl the url which cause the hange
        * @param aExists whether the url exists in the database allready
        * @param aIsBlack whether the database is black
        * @param aToAdd whether to add the url to the database
        * @return 
        */
        void HandleUrlOrderChangeL(RFile* aFile, const TDesC& aUrl, const TBool aExists,const TBool aToAdd );
        /**
        * Removes Url from the cache
        * @param aCacheArray 
        * @param aUrl
        * @param aCacheNo
        * @return 
        */
        void RemoveUrlFromCacheL( CDesCArrayFlat* aCacheArray, const TDesC& aUrl, TInt &aCacheNo );
        /**
        * Checks whether the database is valid
        * @param aFile pointer to the file which will be checked
        * @return ETrue if the file is OK
        */
        TBool CheckDbValidity(RFile* aFile);
        //member variables
    	RFs iFs;
    	RFile iWhiteListFile;//stores the white urls
        CDesCArrayFlat* iCachedWhiteUrls;//owned
        TInt iWhiteCacheNo;//number of white urls loaded to cache
};



#endif //__BROWSERPOPUPENGINE_H