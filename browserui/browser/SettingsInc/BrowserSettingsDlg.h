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
*      Dialog for setting the gateway. It's used temporarily.
*     
*
*/


#ifndef CBrowserSETTINGSDIALOG_H
#define CBrowserSETTINGSDIALOG_H

//  INCLUDES

#include <AknForm.h>

// CLASS DECLARATION

/**
*  Dialog for setting the gateway's IP address
*/
class CBrowserSettingsDlg : public CAknDialog
	{
	public :	// construction
		/**
		* Two-phased constructor.
		*/
		static CBrowserSettingsDlg* NewL();

	protected :		// construction
		/**
		* constructor, that may leave.
		*/
		void ConstructL();

	protected :	// from CEikDialog
		/**
		* Set the gateway's IP address, if the user pressed OK.
		* @param aButtonId The ID of the pressed button.
		* @return May the dialog exit?
		*/
		TBool OkToExitL( TInt aButtonId ) ;
	};

#endif

// End of File