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
* Description: 
*      Implementation of CBrowserFaviconHandler
*
*
*/


// INCLUDES
#include "BrowserFaviconHandler.h"
#include "ApiProvider.h"
#include "BrowserUtil.h"
#include <e32base.h>
#include <brctldefs.h>
#include <fbs.h>
#include <MdaImageConverter.h>
#include <favouritesitemlist.h>
#include <favouritesitem.h>

// number of favicons to get before redrawing
const TInt KNumFaviconsToRedraw = 3	;

// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::CBrowserFaviconHandler()
// ----------------------------------------------------------------------------
//
CBrowserFaviconHandler::CBrowserFaviconHandler( 
										MApiProvider& aApiProvider,
										MBrowserFaviconObserver& aObserver,
										TSize aFaviconSize ) :
	CActive( CActive::EPriorityIdle ),
	iApiProvider( aApiProvider ),
	iObserver( aObserver ),
	iFaviconSize( aFaviconSize )
	{
	CActiveScheduler::Add( this );	
	}


// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::~CBrowserFaviconHandler()
// ----------------------------------------------------------------------------
//
CBrowserFaviconHandler::~CBrowserFaviconHandler()
	{
	Cancel();
	if(iBmpScaler!=NULL)
		{
			iBmpScaler->Cancel();
		}
	delete iBmpScaler;
	delete iFaviconArrayIndices;
	delete iFavicon;
	}


// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::NewL()
// ----------------------------------------------------------------------------
//	
CBrowserFaviconHandler* CBrowserFaviconHandler::NewL( 
										MApiProvider& aApiProvider,
										CArrayPtr<CGulIcon>* aIconArray,
										MBrowserFaviconObserver& aObserver,
										TSize aFaviconSize )
	{
	CBrowserFaviconHandler* self = 
					CBrowserFaviconHandler::NewLC( 	aApiProvider, 
													aIconArray, 
													aObserver,
													aFaviconSize );
    
    CleanupStack::Pop();
    return self;	
	}
	

// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::NewLC()
// ----------------------------------------------------------------------------
//	
CBrowserFaviconHandler* CBrowserFaviconHandler::NewLC(
										MApiProvider& aApiProvider,
										CArrayPtr<CGulIcon>* aIconArray,
										MBrowserFaviconObserver& aObserver,
										TSize aFaviconSize )
	{
	CBrowserFaviconHandler* self = 
					new ( ELeave ) CBrowserFaviconHandler( 	aApiProvider,
															aObserver,
															aFaviconSize );
	CleanupStack::PushL( self );
    self->ConstructL( aIconArray );
    return self;	
	}
	

// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::ConstructL()
// ----------------------------------------------------------------------------
//		
void CBrowserFaviconHandler::ConstructL( CArrayPtr<CGulIcon>* aIconArray )
	{
	iIconArray = aIconArray; // not owned
	iFaviconArrayIndices = new ( ELeave ) CArrayFixFlat<TFaviconIndex>( 4 );
		
	// Construct Favicon Engine
	iBmpScaler = CBrowserFaviconScaler::NewL(*this);
	}

// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::StartGetFaviconsL()
// ----------------------------------------------------------------------------
//		
void CBrowserFaviconHandler::StartGetFaviconsL( CFavouritesItemList* aFavItems )
	{
	Cancel();
	iBmpScaler->Cancel();
		
	if ( aFavItems->Count() )
		{
		// New request for favicons: 
		iFavItems = aFavItems;
		
	    // Initialisation.
		iFavItemsCurrentIndex = 0;      // will be incremented before favicon getting
        iFaviconsFound = 0;             // keeps track of favicons found
		iFaviconArrayIndices->Reset();  // refresh the array mapping uid's to favicons
		
		// Initiate the getting of favicons	
		GetFaviconL();
		}
	}
	
// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::GetFaviconL()
// ----------------------------------------------------------------------------
//		
void CBrowserFaviconHandler::GetFaviconL()
	{	
	// The whole list has already been checked - we're finished
	if ( iFavItemsCurrentIndex >= iFavItems->Count() )
		{
		iObserver.DrawFavicons();
		}
	else
	    {
	    // Step through the item array until we find a favicon, or reach the end
		do
			{
			iWasLastItemFavicon = EFalse;
			
			CFavouritesItem& item = *iFavItems->At( iFavItemsCurrentIndex );
			HBufC* url = HBufC::NewLC( item.Url().Length() );
			url->Des().Copy( item.Url() );
			
			if ( iFavicon )
				{
				// Make sure ongoing scaling is cancelled (if any)
				Cancel();
				iBmpScaler->Cancel();
				
				delete iFavicon;
				iFavicon = NULL;
				}
						
			// Get Favicon from Engine
			if ( item.Url().Length() )
				{
				iFavicon = iApiProvider.BrCtlInterface().GetBitmapData( 
										*url, TBrCtlDefs::EBitmapFavicon );
				}
            CleanupStack::PopAndDestroy();//url
			// Asynchronously scales the favicon and stores it in an array
			if ( iFavicon )
				{
				iWasLastItemFavicon = ETrue;	
				RescaleFaviconL();
				iFaviconsFound++;
				__ASSERT_DEBUG( !( iFavItemsCurrentIndex > iFavItems->Count() ),
								Util::Panic( Util::EOutOfRange ) );
				break;
				}
			} while ( ++iFavItemsCurrentIndex < iFavItems->Count() );
		}
		
		// If we exited the 'do' and the last item wasn't a favicon
		// it means we're finished
		if ( !iWasLastItemFavicon &&  ( iFaviconsFound %  KNumFaviconsToRedraw ) )
			{
			iObserver.DrawFavicons();
			}
	}
	
// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::RescaleFaviconL()
// ----------------------------------------------------------------------------
//
void CBrowserFaviconHandler::RescaleFaviconL()
	{
	iHasMask = (iFavicon->Mask() != NULL);
	if (iHasMask)
		{
		iBmpScaler->StartScalingL( *iFavicon->Mask(), iFaviconSize, ETrue );
		}
	else
		{
		iBmpScaler->StartScalingL( *iFavicon->Bitmap(), iFaviconSize, EFalse );
		}
	//  Complete two asynchronous requests.
	iStatus = KRequestPending;
	SetActive();

	}
	
// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::AppendToIconArray()
// ----------------------------------------------------------------------------
//		
TInt CBrowserFaviconHandler::AppendToIconArray()
	{
	TInt err( KErrNone );
	TInt arrayIndex(-1);
	
	// Add the favicon to the icon array
	TRAP( err, iIconArray->AppendL( iFavicon ); );

	// Favicon was successfully added to the array
	if ( !err )
		{		
		arrayIndex = iIconArray->Count() - 1;
		
		// Keep track of the index of the favicon in the icon array
		if ( arrayIndex > -1 )
			{
			TFaviconIndex faviconIndex;			
			faviconIndex.iFavouritesItemIndex = iFavItems->At( iFavItemsCurrentIndex )->Uid();
			faviconIndex.iIndex = arrayIndex;		
			TRAP( err, iFaviconArrayIndices->AppendL( faviconIndex ); )
			
			// If there was an error remove the favicon from the icon array
			if ( err )
				{
				iIconArray->Delete( iIconArray->Count() - 1 );				
				}
			else
				{
				iFavicon = NULL; // favicon ownership passed
				iFavItemsCurrentIndex++; // we can go to the next item now
				}
			}
		}
		
	// Ownership of the favicon was not passed, so we need to delete it
	if ( err )
		{
		iFaviconsFound--; // one less favicon in the array
		}	
		
	return err;
	}
	
	
// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::GetFaviconArrayIndex()
// ----------------------------------------------------------------------------
//
TInt CBrowserFaviconHandler::GetFaviconArrayIndex( const CFavouritesItem& aItem )
	{
	TInt arrayIndex( KErrNotFound );
	
	// Find the index of the favicon in the icon array
	for ( TInt i=0; i<iFaviconArrayIndices->Count(); i++ )
		{
		// if uid is 0 then don't associate with favicon.  for adaptive bookmarks
		if ( aItem.Uid() && aItem.Uid() == iFaviconArrayIndices->At(i).iFavouritesItemIndex )
			{
			arrayIndex = iFaviconArrayIndices->At(i).iIndex;
			break;
			}			
		}	
	return arrayIndex;
	}

// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::ScalingCompletedL()
// ----------------------------------------------------------------------------
//		
void CBrowserFaviconHandler::ScalingCompletedL(CFbsBitmap* aResult, const TRect& /*aTargetRect*/)
	{
	// Check that favicon hasn't already disappeared (i.e. if race cond in which scaler returns 
	// after you've already left and deleted the icon handle concerned
	if(NULL == iFavicon)
		{
		return;
		}
	
	
	// Add the favicon to the icon array (which also contains other icons)
	if (iHasMask)
		{
		iFavicon->SetMask(aResult);
		iHasMask = EFalse;
		iBmpScaler->StartScalingL( *iFavicon->Bitmap(), iFaviconSize, EFalse );
		}
	else
		{
		iFavicon->SetBitmap(aResult);
		iFavicon->SetBitmapsOwnedExternally(EFalse);
		TInt err = AppendToIconArray();
		
		// Complete the AO
		TRequestStatus* status = &iStatus;
		User::RequestComplete( status, err );	
		}
	}
// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::DoCancel()
// ----------------------------------------------------------------------------
//	
void CBrowserFaviconHandler::DoCancel()
	{
	TRequestStatus* s = &iStatus;
	User::RequestComplete( s, KErrNone );
	}
	
		
// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::RunL()
// ----------------------------------------------------------------------------
//		
void CBrowserFaviconHandler::RunL()
	{		
	TBool drawFavicons( EFalse );
		
	// Redraw the favicons at a predefined frequency
	if ( iFaviconsFound % KNumFaviconsToRedraw == 0 )
		{
		drawFavicons = ETrue;
		}
	else
		{
		// We're not getting any more favicons: draw any undrawn ones
		if ( iStatus != KErrNone )
			{
			// do a final draw
			drawFavicons = ETrue;
			}
		}
	
	if ( drawFavicons && iFaviconsFound )
		{
		iObserver.DrawFavicons();
		}
	
	// Continue getting favicons if no errors
	if ( iStatus == KErrNone )
		{
		GetFaviconL();
		}	
	}

// ----------------------------------------------------------------------------
// CBrowserFaviconHandler::UpdateIconArray()
// ----------------------------------------------------------------------------
//	
void CBrowserFaviconHandler::UpdateIconArray(CArrayPtr<CGulIcon>* aIconArray)
    {
   	iIconArray = aIconArray; 
    }
 
// End of File
