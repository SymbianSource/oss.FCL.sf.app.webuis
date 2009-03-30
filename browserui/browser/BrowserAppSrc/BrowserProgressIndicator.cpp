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
*
*
*/


// INCLUDE FILES

#include <AknAppUi.h>
#include <e32math.h>
#include <BrowserNG.rsg>
#include <FeatMgr.h>
#include <stringloader.h>

// USER INCLUDES

#include "Display.h"
#include "ApiProvider.h"
#include "CommonConstants.h"  // for View Ids
#include "BrowserProgressIndicator.h"
#include "Logger.h"
#include "AknUtils.h"

// CONSTANTS

const TInt KGranularity = 4;
const TUint KkBLimit = 999;
const TUint KOnekB = 1024;
const TUint KOneMB = 1048580;
const TInt KMaxPercentage = 100;
const TInt KMBLimit = 10;
const TInt KMaxMBLimit = 100;
_LIT(text,"(%S) %S ");
_LIT(twoDecimals,"%2.2lf");
_LIT(oneDecimal, "%2.1lf");
_LIT(noDecimals, "%d");

// ================= MEMBER FUNCTIONS =======================

// ---------------------------------------------------------
// CBrowserProgressIndicator::NewL
// ---------------------------------------------------------
//
CBrowserProgressIndicator* CBrowserProgressIndicator::NewL(
        MApiProvider& aApiProvider )
	{
	CBrowserProgressIndicator* self = new (ELeave)
	    CBrowserProgressIndicator( aApiProvider );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();
    return self;
	}

// ---------------------------------------------------------
// CBrowserProgressIndicator::ConstructL
// ---------------------------------------------------------
//
void CBrowserProgressIndicator::ConstructL()
    {
    iProgressData = new(ELeave) CArrayFixFlat<TProgressData>( KGranularity );
    iInitialDownloadIndicatorState = EFalse;

    HBufC* myKb = StringLoader::LoadL( R_WML_UNIT_KB );
    iMyKb = myKb->Alloc();
    delete myKb;

    HBufC* myMb = StringLoader::LoadL( R_WML_UNIT_MB );
    iMyMb = myMb->Alloc();
    delete myMb;
    }

// ---------------------------------------------------------
// CBrowserProgressIndicator::CBrowserProgressIndicator
// ---------------------------------------------------------
//
CBrowserProgressIndicator::CBrowserProgressIndicator(
        MApiProvider& aApiProvider ) : 
    iIsRunning( EFalse ),
    iApiProvider( aApiProvider ),
    iLastMaxEstimate( 0 ),
    iLastReceivedData( 0 )
	{
	}

// ---------------------------------------------------------
// CBrowserProgressIndicator::~CBrowserProgressIndicator
// ---------------------------------------------------------
//
CBrowserProgressIndicator::~CBrowserProgressIndicator()
	{
    delete iProgressData;
    delete iMyKb;
    delete iMyMb;
	}

// ---------------------------------------------------------
// CBrowserProgressIndicator::AddTransActIdL
// ---------------------------------------------------------
//
void CBrowserProgressIndicator::AddTransActIdL( TUint16 aId )
	{
    //Format each id at start point.
    AddProgressDataL(aId, 0, 0);
	}

// ---------------------------------------------------------
// CBrowserProgressIndicator::DeleteProgressDataItem
// ---------------------------------------------------------
//
void CBrowserProgressIndicator::DeleteProgressDataItem( TUint16 aId )
	{
    TKeyArrayFix key(0, ECmpTUint16);	
	TInt pos = 0;
	TProgressData data;
	data.iId = aId;
	TInt retVal = iProgressData->Find( data,key,pos );
    if ( retVal == KErrNone )
        {
        iProgressData->Delete( pos );
        }		

	}

// ---------------------------------------------------------
// CBrowserProgressIndicator::TransActIdAmount()
// ---------------------------------------------------------
//
TInt CBrowserProgressIndicator::TransActIdAmount() const
	{
    return iProgressData->Count();
	}

// ---------------------------------------------------------
// CBrowserProgressIndicator::TransActIdAmountCompleted()
// ---------------------------------------------------------
//
TInt CBrowserProgressIndicator::TransActIdAmountCompleted() const
    {
    TInt numOfCompleted( 0 );
    TInt i;
    for( i=0; i<TransActIdAmount(); ++i )
    {
        if( iProgressData->At( i ).iComplete )
            ++numOfCompleted;
    }
    return numOfCompleted;
    }

// ---------------------------------------------------------
// CBrowserProgressIndicator::ResetValues()
// ---------------------------------------------------------
//
void CBrowserProgressIndicator::ResetValues()
	{
	iProgressData->Reset();
    iProgressEstimate = 0;
	}

// ---------------------------------------------------------
// CBrowserProgressIndicator::ReceivedData()
// ---------------------------------------------------------
//
TUint32 CBrowserProgressIndicator::ReceivedData()
	{
	TUint32 recvdData = 0;
	for (TInt i = 0; i < iProgressData->Count();i++)
		{
		TProgressData data = iProgressData->At( i );
		recvdData += data.iRecvdData;
		}
	return recvdData;
	}

// ---------------------------------------------------------
// CBrowserProgressIndicator::EstimateMaxData()
// ---------------------------------------------------------
//
TUint32 CBrowserProgressIndicator::EstimateMaxData()
	{
	TUint32 maxData = 0;
    TInt count = 0;
    TUint32 estimation = 0;
	for ( TInt i = 0; i < iProgressData->Count();i++ )
		{
		TProgressData data = iProgressData->At( i );
        if( data.iMaxData )
            {
            maxData += data.iMaxData;
            count++;
            }
		}
     // estimate maximum data what will be received
     if ( count )
         {
         return estimation = ((maxData/count)* iProgressData->Count());
         }

	return estimation;

	}

// ---------------------------------------------------------
// CBrowserProgressIndicator::TransActId()
// ---------------------------------------------------------
//
TUint16 CBrowserProgressIndicator::TransActId( TInt aIndex )
	{
    return iProgressData->At( aIndex ).iId;
	}

// ---------------------------------------------------------
// CBrowserProgressIndicator::AddProgressDataL()
// ---------------------------------------------------------
//
void CBrowserProgressIndicator::AddProgressDataL( 
                     TUint16 aId, TUint32 aRecvdData, TUint32 aMaxData )
	{
	//Try to seek if current id exist in array
	TKeyArrayFix key(0, ECmpTUint16);
	TInt pos = 0;
	TProgressData data;
	data.iId = aId;
	TInt retVal = iProgressData->Find( data,key,pos );
	if ( retVal!=KErrNone ) //id was NOT found
		{
		//Add id, recvdData and maxData to array
		TProgressData data;
		data.iId = aId;
		data.iRecvdData = aRecvdData;		
        data.iMaxData = aMaxData;
        data.iComplete = EFalse;
        iProgressData->AppendL( data );
		}
	else
		{

        data = iProgressData->At( pos );
		if( ((data.iRecvdData != aRecvdData) && aRecvdData!=0 ) )
			{
			data.iRecvdData = aRecvdData;
            }
        if ( aMaxData!=0 ) 
            {
            data.iMaxData = aMaxData;
            }

        iProgressData->Delete( pos );
        iProgressData->InsertL( pos,data );	
		}
	}


// ---------------------------------------------------------
// CBrowserProgressIndicator::TransActIdCompleteL()
// ---------------------------------------------------------
//
void CBrowserProgressIndicator::TransActIdCompleteL( TUint16 aId )
    {
    TKeyArrayFix key( 0, ECmpTUint16 );	
	TInt pos = 0;
	TProgressData data;
	data.iId = aId;
	TInt retVal = iProgressData->Find( data,key,pos );
    if ( retVal == KErrNone && ! iProgressData->At(pos).iComplete )
        {
        data = iProgressData->At( pos );
        // we don't need this id anymore if both data values are empty
        if ( !data.iMaxData && !data.iRecvdData )
            {
            iProgressData->Delete( pos );
            }
        else 
            {
            data.iComplete = ETrue;
            if ( data.iMaxData > data.iRecvdData )
                {
                data.iRecvdData = data.iMaxData;
                }
            iProgressData->Delete( pos );
            iProgressData->InsertL( pos,data );
            }
       }
    }

// ---------------------------------------------------------
// CBrowserProgressIndicator::EnquireStatusL()
// ---------------------------------------------------------
//
void CBrowserProgressIndicator::EnquireStatusL()
	{
LOG_ENTERFN("EnquireStatusL");
    // Reset string storage.
	iPercentText.SetLength( 0 );
	iDataText.SetLength( 0 );
    iMBvalue.SetLength( 0 );

    TUint32 receivedData = ReceivedData();
    TUint32 maxEstimate = EstimateMaxData();
    
    if (receivedData == 0 || maxEstimate == 0)
        {
        // fake initial progress to reflect 5% downloaded and 0 kb received. 
        receivedData    =  5;
        maxEstimate     =  100;
        }

    //Calculate either kB-text or MB-text
    if ( ((receivedData)/KOnekB ) <= KkBLimit )
        {
        HBufC* kbUnit = HBufC::NewL( iMyKb->Length()+5 );
        TPtr ptr1( kbUnit->Des() );
        StringLoader::Format( ptr1, *iMyKb, -1, (receivedData)/KOnekB );
        AknTextUtils::DisplayTextLanguageSpecificNumberConversion(ptr1);
        iDataText.Copy( ptr1 );
        delete kbUnit;
        }
    else
        {
        
        TReal received( receivedData );
        TReal oneMB( KOneMB );
        TReal result( received / oneMB );
        // TUint32 result( receivedData / KOneMB );
        if ( result < KMBLimit )
            {
            // TUint32 res2( ( ( receivedData * 100 ) / KOneMB ) % 100 );
            // need to modify the format string too
            iMBvalue.Format( twoDecimals, result );
            }
        else if ( result < KMaxMBLimit )
            {
            // TUint32 res2( ( ( receivedData * 10 ) / KOneMB ) % 10 );
            // need to modify the format string too
            iMBvalue.Format( oneDecimal, result );
            }
        else if ( result > KMaxMBLimit)
            {
            TInt16 resultInt( 0 );
            Math::Int( resultInt, result );
            iMBvalue.Format( noDecimals, resultInt );
            }
        HBufC* mbUnit = HBufC::NewL( iMyMb->Length() + iMBvalue.Length() + 1 );
        TPtr ptr1( mbUnit->Des() );
        StringLoader::Format( ptr1, *iMyMb, -1, iMBvalue );
        AknTextUtils::DisplayTextLanguageSpecificNumberConversion(ptr1);
        iDataText.Copy( ptr1 );
        delete mbUnit;
        }
       
    // progress animation should be shown only in content view (not in bookmarks)
    if( iApiProvider.LastActiveViewId() == KUidBrowserContentViewId )
        {
        // Check all possible states.
        if ( !iInitialDownloadIndicatorState && maxEstimate <= 0 )
            {
            iInitialDownloadIndicatorState = ETrue;
            // The initial indicator state changed from EFalse to ETrue. Update it.
            iApiProvider.Display().UpdateFSDownloadInitialIndicator( ETrue );
            }
        else if ( !iInitialDownloadIndicatorState && 0 < maxEstimate )
            {
            //iInitialDownloadIndicatorState = EFalse; // Unnecessary statement.
            }
        else if ( iInitialDownloadIndicatorState && maxEstimate <= 0 )
            {
            //iInitialDownloadIndicatorState = ETrue; // Unnecessary statement.
            }
        else if ( iInitialDownloadIndicatorState && 0 < maxEstimate )
            {
            iInitialDownloadIndicatorState = EFalse;
            // The initial indicator state changed from ETrue to EFalse. Update it.
            iApiProvider.Display().UpdateFSDownloadInitialIndicator( EFalse );
            }

        iApiProvider.Display().UpdateFSProgressIndicator( maxEstimate, receivedData );

        // Render the downloaded data size.
        iApiProvider.Display().UpdateFSProgressDataL( iDataText );
        }
    else
        {
        // is this section really run?

        // Show how many percentage has been dowloaded
        if ( maxEstimate )
            {
            TReal received( receivedData );
            TReal maxEst( maxEstimate );
            TReal factor( 100 );
            TReal percentage( (received / maxEst) * factor );
            TInt16 percentInt( 0 );
            Math::Int( percentInt, percentage );
            if ( percentInt > KMaxPercentage )
                {
                percentInt = KMaxPercentage;
                }
            HBufC* percentUnit = 
                StringLoader::LoadLC( R_WML_UNIT_PERCENT, percentInt );
            iPercentText.Format( text, percentUnit, &iDataText );
            CleanupStack::PopAndDestroy( percentUnit ); //percentUnit

            //Update navi pane
            iApiProvider.UpdateNaviPaneL( iPercentText );
            }
        //Show amount of downloaded data (recvd data)
        //(if maximum data is not available)
        else
            {
            iApiProvider.UpdateNaviPaneL( iDataText );
            }
        }
	}

// ---------------------------------------------------------
// CBrowserProgressIndicator::StartL()
// ---------------------------------------------------------
//
void CBrowserProgressIndicator::StartL()
    {
    if ( !iIsRunning )
        {
        // clear progress bar data
        iLastMaxEstimate = 0;
        iLastReceivedData = 0;
                    
        // progress animation should be shown 
        // only in content view (not in bookmarks)
        if( iApiProvider.LastActiveViewId() == KUidBrowserContentViewId )
            {
            //Start progress bar
            NotifyProgress();
            
            iInitialDownloadIndicatorState = ETrue;
            iApiProvider.Display().UpdateFSDownloadInitialIndicator( ETrue );
            // Show initially 0 kB as downloaded size.
            HBufC* kbUnit = HBufC::NewL( iMyKb->Length()+5 );
            TPtr ptr1( kbUnit->Des() );
            StringLoader::Format( ptr1, *iMyKb, -1, 0 );
            iDataText.Copy( ptr1 );
            delete kbUnit;
            iApiProvider.Display().UpdateFSProgressDataL( iDataText );
            }

        iIsRunning = ETrue;
        }
    }

// ---------------------------------------------------------
// CBrowserProgressIndicator::StopL()
// ---------------------------------------------------------
//
void CBrowserProgressIndicator::StopL()
    {
    if( iApiProvider.LastActiveViewId() == KUidBrowserContentViewId )
        {
        ResetValues();
        if( iInitialDownloadIndicatorState )
            {
            iInitialDownloadIndicatorState = EFalse;
            iApiProvider.Display().
                UpdateFSDownloadInitialIndicator( EFalse );
            }
        iApiProvider.Display().RestoreTitleL();
        }
    iIsRunning = EFalse;
    }

// ---------------------------------------------------------
// CBrowserProgressIndicator::NotifyProgress()
// ---------------------------------------------------------
//
void CBrowserProgressIndicator::NotifyProgress()
    {
    // Unfortunately we could not eliminate TRAP. The Engine cannot 
    // support a leavable NotifyProgressL() function.
    TRAP_IGNORE( EnquireStatusL() );
    }

//  End of File
