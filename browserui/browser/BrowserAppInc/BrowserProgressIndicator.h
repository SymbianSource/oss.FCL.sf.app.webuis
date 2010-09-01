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
*      Helper class to put animation and progress indicator to contextPane.
*
*
*/


#ifndef BROWSERPROGRESSINDICATOR_H
#define BROWSERPROGRESSINDICATOR_H

// INCLUDES
#include <E32BASE.H>

// FORWARD DECLARATIONS
//class CBrowserAnimation;
class MApiProvider;

// DATA TYPES

// Progressbar data struct.
struct TProgressData
    {
    TUint16 iId;        // Id of the transaction
    TUint32 iRecvdData; // Amount of received data
    TUint32 iMaxData;   // Max amount of data
    TBool   iComplete;  // True if TransAct Id downloaded otherwise False.
    };


// CLASS DECLARATION
/**
*  Encapsulates the animation feature of the browser.
*
*  @lib Browser.app
*  @since Series60 1.2
*/
class CBrowserProgressIndicator : public CBase
    {
    public:  // Constructors and destructor

        /**
        * Two-phased constructor.
        * @param aApiProvider Reference to API provider
        * @return Pointer to instance of CBrowserAnimation
        */
        static CBrowserProgressIndicator* CBrowserProgressIndicator::NewL( MApiProvider& aApiProvider );
        /**
        * Constructor.
        * @param aApiProvider Reference to API provider
        */
        CBrowserProgressIndicator( MApiProvider& aApiProvider );

       /**
        * Destructor.
        */
        virtual ~CBrowserProgressIndicator();

    public: // New functions

        /**
        * Starts animation and progress bar.
        * @since Series60 1.2
        * @param aBearerType 
        */
        void StartL();

        /**
        * Stops animation and progress bar.
        * @since Series60 1.2
        */
        void StopL();

        /**
        * Adds transaction id to array.
        * @since Series60 1.2
        * @param aId Id of the transaction
        */
        void AddTransActIdL( TUint16 aId );

        /**
        * Notify that new data has arrived to the loaders.
        * @since Series60_version 2.5
        * @return none
        */
        void NotifyProgress();

        /**
        * Saves maximum and received data with a transction id. 
        * @since Series60 1.2
        * @param aId Id of the transaction
        * @param aRecvdData Amount of data we have received with current transaction Id
        * @param aMaxData Max size of the transaction
        */
        void AddProgressDataL( TUint16 aId, TUint32 aRecvdData, TUint32 aMaxData );

        /**
        * Marks particular transaction id to complete status
        * @since Series60 2.0
        * @param aId Id of the transaction
        */
        void TransActIdCompleteL( TUint16 aId );

        /**
        * Gives amount of transaction ids. 
        * @since Series60 1.2
        * @return Amount of transaction ids
        */
        TInt TransActIdAmount() const;

        /**
        * Gives amount of completed transaction ids. 
        * @since Series60 1.2
        * @return Amount of transaction ids
        */
        TInt TransActIdAmountCompleted() const;

    protected:
        /**
        * Two-phase constructor.
        */
        void ConstructL();

    private:
        /**
        * Enquires transaction statuses. Calculates and generates progress 
        * text and progress bar.
        * @since Series60 1.2
        * @return None.
        */
        void EnquireStatusL();

        /**
        * Deletes progress data item according to aId .
        * @since Series60 1.2
        * @param aId Transaction id
        */
        void DeleteProgressDataItem( TUint16 aId );
		
        /**
        * Resets all saved transaction ids, maximum data and received data.
        * @since Series60 1.2
        */
        void ResetValues();
		
        /**
        * Returns total amount of received data.
        * @since Series60 1.2
        * @return Number of received data
        */
        TUint32 ReceivedData();
		
        /**
        * All transaction ids won't give max data info at the same time.
        * Calculation is needed to get estimated maxData of page.
        * @since Series60 1.2
        * @return Number of maximum data
        */
        TUint32 EstimateMaxData();
		
        /**
        * Returns transaction id of current index from array.
        * @since Series60 1.2
        * @param aIndex Index of array
        * @return Transaction id
        */
        TUint16 TransActId( TInt aIndex );

    private:    // Data

        TBool iIsRunning;                   // Animation is running
        MApiProvider& iApiProvider;         // Api provider

        CArrayFixFlat<TProgressData>* iProgressData;
        TBool iInitialDownloadIndicatorState; // initial download (globe) animation in Full Screen pane
        TReal iProgressEstimate;

        /** 
        * Temporary storage for strings. Each must be reseted at 
        * the beginning of EnquireStatusL!
        */
        TBuf<20> iPercentText;
        TBuf<15> iDataText;
        TBuf<5>  iMBvalue;

        const TDesC* iMyKb;
        const TDesC* iMyMb;

        TInt iLastMaxEstimate;
        TInt iLastReceivedData;
    };

#endif

// End of File
