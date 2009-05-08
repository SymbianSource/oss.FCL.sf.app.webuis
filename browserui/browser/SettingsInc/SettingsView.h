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
*     View of the information about the active settings
*     
*
*/


#ifndef SETTINGS_VIEW_H
#define SETTINGS_VIEW_H

// INCLUDES

// #include <eiklbo.h>
// #include <ActiveApDb.h>
#include "BrowserAppViewBase.h"
#include "CommsModel.h"

// FORWARD DECLARATION

class CSettingsContainer;
class MApiProvider;

// CLASS DECLARATION

/**
*  View of settings info
*/
class CBrowserSettingsView: public CBrowserViewBase,
						    public MCommsModelObserver
{
	public:		// construction

        /**
        * Two-phased constructor
        * @param aApiProvider The API provider.
        */
		static CBrowserSettingsView* NewLC( MApiProvider& aApiProvider );

        /**
        * Two-phased constructor
        * @param aApiProvider The API provider.
        */
		static CBrowserSettingsView* NewL( MApiProvider& aApiProvider );

        /**
        * Destructor.
        */
		virtual ~CBrowserSettingsView();

	public:     // From MCbaSetter (CWmlBrowserViewBase)
        
        /**
        * Return command set id, to be displayed.
        * @return The command set's resource id.
        */
        TInt CommandSetResourceIdL();

		/**
		* Set command set lsk,rsk,msk dynamically via pointers.
		* Derived classes should implement, though it can be empty.
		* If it does nothing, empty softkeys will be assigned
		* @since 5.0
		*/
		void CommandSetResourceDynL(TSKPair& /*lsk*/, TSKPair& /*rsk*/, TSKPair& /*msk*/) {};
			
	public:		// From	MCommsModelObserver

        /**
        * Handle change in comms model (AP-s).
        */
        void HandleCommsModelChangeL();

	public:		// from CAknView (CWmlBrowserViewBase)

        /**
        * @see CAknView
        */
		void HandleCommandL(TInt aCommand);

        /**
        * @see CAknView
        */
		TUid Id() const;
		
		/**
        * Updates the options softkey to reflect the command set.
        *
        * @since 3.2
        * @param aCommandSet The new "command set".
        * @return void.
        */
		void UpdateCbaL(TInt aCommandSet);
		

	protected:	// from CAknView

        /**
        * @see CAknView
        */
		void DoActivateL(const TVwsViewId& aPrevViewId,TUid aCustomMessageId,const TDesC8& aCustomMessage);


        /**
        * @see CAknView
        */
		void DoDeactivate();

        /**
        * @see CAknView
        */
		void HandleClientRectChange();
		
		/**
		* @see CAknView
		*/
		void  HandleForegroundEventL (TBool aForeground);

	private: // MEikMenuObserver

		void DynInitMenuPaneL( TInt aResourceId, CEikMenuPane* aMenuPane );

	private:	// destruction

        /**
        * Constructor.
        * @param aApiProvider The API provider.
        */
		CBrowserSettingsView( MApiProvider& aApiProvider );

        /**
        * SymbianOS default constructor.
        */
		void ConstructL();

	private:	// members	

		CSettingsContainer*	iContainer;

        TVwsViewId		    iLastViewId; // The view to switch after closing this
};
#endif  // SETTINGS_VIEW_H

// End of file