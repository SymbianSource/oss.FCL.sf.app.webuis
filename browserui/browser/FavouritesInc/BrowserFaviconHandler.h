/*
* Copyright (c) 2005 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  Header file for CBrowserFaviconHandler
*
*
*/


#ifndef __BROWSERFAVICONHANDLER_H
#define __BROWSERFAVICONHANDLER_H

// INCLUDE FILES 
#include <e32base.h>
#include <gulicon.h>
#include "BrowserFaviconScaler.h"

// CLASS DECLARATION
class MApiProvider;
class CFavouritesItemList;
class CFavouritesItem;

class MBrowserFaviconObserver
	{
	public:
		virtual TInt DrawFavicons() = 0;
	};
	
/**
*  CBrowserFaviconHandler.
* 
*  @lib Browser.app
*  @since Series 60 3.1 
*/
class CBrowserFaviconHandler :	public  CActive,
								public MBrowserFaviconScalerCallback
	{
	public: // Constructors
        
        /**
        * @param aApiProvider access to AppUi functions
        * @param aIconArray an array of icons
        * @param aObserver observer for drawing favicons
        * @param aFaviconSize size in pixels of the favicons
        */
        static CBrowserFaviconHandler* NewLC( 
        								MApiProvider& aApiProvider, 
        								CArrayPtr<CGulIcon>* aIconArray,
        								MBrowserFaviconObserver& aObserver,
        								TSize aFaviconSize );
        
        /**
        * @param aApiProvider access to AppUi functions
        * @param aIconArray an array of icons
        * @param aObserver observer for drawing favicons
        * @param aFaviconSize size in pixels of the favicons
        */								
        static CBrowserFaviconHandler* NewL( 
        								MApiProvider& aApiProvider,
        								CArrayPtr<CGulIcon>* aIconArray,
        								MBrowserFaviconObserver& aObserver,
        								TSize aFaviconSize  );
        
        /**
        * Destructor.
        */
    	~CBrowserFaviconHandler();
        								
    protected: // Constructor and destructor
    
    	/**
        * @param aApiProvider access to AppUi functions
        * @param aObserver observer for drawing favicons
        * @param aFaviconSize size in pixels of the favicons
        */
    	CBrowserFaviconHandler( MApiProvider& aApiProvider, 
    							MBrowserFaviconObserver& aObserver,
        						TSize aFaviconSize  );

    public: // from MBrowserFaviconScalerCallback
    	void ScalingCompletedL(CFbsBitmap* aResult, const TRect& aTargetRect);
    	
    public: // from CActive
        
        /**
        * Cancel any outstanding requests
        */
        void DoCancel();

        /**
        * Handles object`s request completion event.
        */
        void RunL();
        
    public: // New functions
        
        
        /**
        * Initiates decoding of favicons for the favourites list in browserengine.
        * This will request icon database thread to start decoding requested icons in background.
        * @param aFavItems a list of favourites items
        */
        void RequestFavicons( CFavouritesItemList* aFavItems );
        
        /**
        * This is async fetch operation from engine. One icon at a time is fetched and asynchronously
        * scaled. E.g. if there are 5 icons to be fetched, then this will cause the BitmapScaler to be
        * Active for 5 times.
        * overall fetching is Async 
        * @param aFavItems a list of favourites items
        */
        void StartGetFaviconsL( CFavouritesItemList* aFavItems );
        
        /**
        * Appends the icon to the icon array
        * @param aItem the favourites item which is being checked for favicons
        * @return Index of favicon array in iIconArray
        */
        TInt GetFaviconArrayIndex( const CFavouritesItem& aItem );
        
        /**
        * Updates the handle to icon array
        * @param aIconArray an array of icons
        */

        void UpdateIconArray(CArrayPtr<CGulIcon>* aIconArray);
        
    protected: // New functions    
        
        /**
        * Gets the favicon for a given url from the Favicon Engine
        * @param aUrl a url to check if there is an associated favicon
        */
        void GetFaviconL();
        
        /**
        * Rescales a favicon to fit into the list box graphic pane
        * and stores the result in iIconArray
        */
		void RescaleFaviconL();		
			        	    
        /**
        * Appends the icon to the icon array
        * @return Leave code
        */
        TInt AppendToIconArray();
        
        
	private: // New functions
    	
    	void ConstructL( CArrayPtr<CGulIcon>* aIconArray );        
                
        // Structures
		struct TFaviconIndex
			{
			TInt  iFavouritesItemIndex;
			TInt  iIndex;
			};
			
		// Members
	    MApiProvider& 					iApiProvider;			// NOT owned
	    MBrowserFaviconObserver& 		iObserver;				// NOT owned
	     
        CArrayPtr<CGulIcon>* 			iIconArray;				// NOT owned
    	CArrayFixFlat<TFaviconIndex>*	iFaviconArrayIndices; 	// owned
    	
    	CFavouritesItemList* 			iFavItems;				// NOT owned
    	TSize							iFaviconSize;			
    	
    	CGulIcon*						iFavicon;				// NOT owned
    	HBufC*							iUrl;					// owned
    	
    	// Counters
    	TInt 							iFavItemsCurrentIndex;  // position in the favourites item list
    	TInt							iFaviconsFound;         // counter for number of favicon which have been found
    	
    	TBool                           iWasLastItemFavicon;
    	CBrowserFaviconScaler* 			iBmpScaler;				// owned
    	TBool 							iHasMask; 	
	};



#endif // __BROWSERFAVICONHANDLER_H
