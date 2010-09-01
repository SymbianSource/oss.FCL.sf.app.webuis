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
*     Handles white list settings in provisioning.
*
*/


#ifndef WHITELISTADAPTER_H
#define WHITELISTADAPTER_H

// INCLUDE FILES
	// User includes
#include "WhiteListAdapterDef.h"
	// System includes
#include <CWPAdapter.h>
#include <msvapi.h>
#include <MWPContextExtension.h>

// FORWARD DECLARATIONS

class CPushMtmSettings;
class CWPCharacteristic;
class CWPParameter;
class MWPPairVisitor;
class TWhiteListAdapterItem;

// CLASS DECLARATION

/**
* An ECOM adapter plug-in. It is responsible for maintaining data in a
* WAP Provisioning pushed message : we get appropriate physical proxy
* addresses from the message and store them in Push MTM settings. It is the
* Push Content Handler that will use these addresses to filter out
* unauthorized pushed SL messages.
* Note : although an adapter should always be prepared for displaying the
* processed and stored settings in the appropriate format, the stored items
* in this specific adapter will _never_ be displayed. Thus, ItemCount,
* SummaryTitle and SummaryText methods have been written considering this
* approach.
*/
class CWhiteListAdapter : public CWPAdapter, 
                          public MMsvSessionObserver, 
                          public MWPContextExtension
	{
	public :	// construction, destruction
		/**
		* Two-phased constructor.
		*/
		static CWhiteListAdapter* NewL();

		/**
		* Destructor.
		*/
		~CWhiteListAdapter();

	public :	// from CWPAdapter

        /**
		* Check the number of settings items.
		* @see CWPAdapter
		*/
		/*virtual*/ TInt ItemCount() const;

		/**
		* Returns the title of the nth summary line. 
		* @see CWPAdapter
		*/
		/*virtual*/ const TDesC16& SummaryTitle( TInt aIndex ) const;
		
		/**
		* Returns the text of the nth summary line. 
		* @see CWPAdapter
		*/
		/*virtual*/ const TDesC16& SummaryText( TInt aIndex ) const;
		
		/**
		* Saves one setting handled by the adapter.
		* @see CWPAdapter
		*/
		/*virtual*/ void SaveL( TInt aItem );

		/**
		* Returns ETrue if the adapter can set the settings as default.
		* @see CWPAdapter
		*/
		/*virtual*/ TBool CanSetAsDefault( TInt aItem ) const;

		/**
		* Adapter sets the settings as default.
		* @see CWPAdapter
		*/
		/*virtual*/ void SetAsDefaultL( TInt aItem );

		/**
		* Calls the pair visitor for each detail line of an item.
		* @see CWPAdapter
		*/
		/*virtual*/ TInt DetailsL( TInt aItem, MWPPairVisitor& aVisitor );

        /**
        * Returns a pointer to a context extension.
        * @param aExtension 
        *        Contains a pointer to MWPContextExtension if supported.
        * @return KErrNotSupported if not supported, KErrNone otherwise
        */
        /*virtual*/ TInt ContextExtension( MWPContextExtension*& aExtension );

    public: // From MWPVisitor (inheritance is through CWPAdapter)

        /**
		* Called for each characteristic found.
		* @see MWPVisitor
		*/
		/*virtual*/ void VisitL( CWPCharacteristic& aCharacteristic );
		
		/**
		* Called for each parameter found.
		* @see MWPVisitor
		*/
		/*virtual*/ void VisitL( CWPParameter& aParameter );

		/**
		* Called for each link to a logical proxy or access point.
		* @see MWPVisitor
		*/
		/*virtual*/ void VisitLinkL( CWPCharacteristic& aCharacteristic );

	public :	// from MMsvSessionObserver
		/**
		* Indicates an event has occurred.
		* @see MMsvSessionObserver
		*/
		/*virtual*/ void HandleSessionEventL( TMsvSessionEvent aEvent,
											TAny* aArg1,
											TAny* aArg2,
											TAny* aArg3 );

	private :	// construction
		/**
		* Constructor.
		*/
		CWhiteListAdapter();

		/**
		* Second-phase constructor.
		*/
		void ConstructL();

	private :	// internal new methods
		/**
		* This method is for interpreting a string as push support.
		* If the value of the string is '1', then push is enabled,
		* if it's equal to '0', then push is disabled, otherwise the proxy
		* does not care of push.
		* @param aPushSupport String to be processed.
		* @return TPushSupport enumeration value.
		*/
		TPushSupport ResolvePushSupport( const TDesC& aPushSupport );

    private: // From MWPContextExtension

        /**
        * Returns the data used for saving.
        * @param aIndex The index of the data
        * @return The data. Ownership is transferred.
        */
        const TDesC8& SaveDataL( TInt aIndex ) const;

        /**
        * Deletes a saved item.
        * @param aSaveData The data used for saving
        */
        void DeleteL( const TDesC8& aSaveData );

        /**
        * Returns the UID of the adapter.
        * @return UID
        */
        TUint32 Uid() const;

	private :	// data members

        RPointerArray<TWhiteListAdapterItem> iItems;	///< Physical proxies

        /**
        * These items are parts of the current logical proxy and have not
        * been added to the final item array (i.e. iItems array), because
        * further processing is needed.
        */
        RPointerArray<TWhiteListAdapterItem> iItemsToBeAdded;

		TPushSupport iLogicalProxyPushSupport;

		TBool iPhysicalProxyIsVisited;			///< In order to differentiate
												///< if we are visiting a
												///< ph. or a log. proxy

		TWhiteListAdapterItem* iCurrentItem;	///< Current physical proxy.
												///< Has.

		CMsvSession* iMsvSession;				///< Session to the Message
												///< Server. Has.
		CPushMtmSettings* iPushMtmSettings;		///< Push MTM Settings object.
												///< Has.
	};

#endif	// WHITELISTADAPTER_H

// End of file