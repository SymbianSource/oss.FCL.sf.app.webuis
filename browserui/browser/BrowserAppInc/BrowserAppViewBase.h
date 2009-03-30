/*
* Copyright (c) 2007 Nokia Corporation and/or its subsidiary(-ies).
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
*      Declaration of class CBrowserAppViewBase.
*      
*
*/


#ifndef BROWSER_APP_VIEW_BASE_H
#define BROWSER_APP_VIEW_BASE_H

// INCLUDE FILES
#include <aknview.h>

// FORWARD DECLARATION

class MApiProvider;

// CLASS DECLARATION

/**
*  Class to encapsulate a softkey pair made up of an Command ID and Resource qtn
*/
class TSKPair
	{
public:
    /**
    * Constructor.
    * @param NONE. command and qtn are set to default.
    */
	TSKPair();

    /**
    * Constructor.
    * @param Command and Resource IDs
    */
	TSKPair(TInt aId,TInt aRsc);
	
	/**
	* check to see if command/qtn are not currently set to the default assignment
	*/
	TBool IsAssigned();
	
	/**
	* define an == operator to check pair equality
	* @param TSK pair
	*/
	TBool operator==(TSKPair aSKPair);
	
	/**
	* define an = operator for assignment
	* @param TSK pair
	*/
	void operator=(TSKPair aSKPair);
	
	/**
	* a function to set the pair directly
	* @param Command and Resource Ids
	*/
	void setPair(TInt aId, TInt aRsc);
	
	/**
	* retrieve the command ID
	*/
	TInt id() { return iCmdId; }
	
	/**
	* retrieve the qtn resource ID
	*/
	TInt qtn() { return iQtnRsc; }
private:

	/**
	* private variables to hold command and resource ids
	*/
	TInt iCmdId;
	TInt iQtnRsc;
	};

/**
*  Base class all of the browser's views. Allows CBA setting and holds pointer
*  to API provider.
*  @lib Browser.app
*  @since Series 60 1.2
*/
class CBrowserViewBase : public CAknView
    {
    public:     // construction

        /**
        * Constructor.
        * @param aApiProvider The API provider
        */
        CBrowserViewBase( MApiProvider& aApiProvider );

    public:     // new methods

        /**
        * Get the API provider.
        * @since Series 60 1.2
        * @return The API provider
        */
        MApiProvider& ApiProvider();

        /**
        * Update the view's CBA, according to its current state.
        * This will call the view's CommandSetResourceIdL to get the
        * command set's resource id and then change the command set to that.
        * Note, there is no Push/Pop of CBA-s! The CBA which changes is always
        * that of the view; that is, if there is another CBA Push()-ed on top
        * of it (e.g. dialogs etc.), that will be unchanged.
        * @since Series 60 1.2
        */
        void UpdateCbaL();

        /**
        * Switch cba visibility, checking landscape state
        * @param aVisible Visibility
        */
        void MakeCbaVisible(TBool aVisible);

        /**
        * Update goto pane, if it is present in the view.
        * @since Series 60 1.2
        */
        virtual void UpdateGotoPaneL();

        /**
        * Clear CBA.
        * @since Series 60 1.2
        */
        void ClearCbaL();

        /**
        * Return command set id, to be displayed.
        * Derived classes should implement this method.
        * @since Series 60 1.2
        * @return
        *   - 0 if the command set should not be changed\n
        *   - Otherwise, the desired command set's resource id
        */
        virtual TInt CommandSetResourceIdL() = 0;

		/**
		* Set command set lsk,rsk,msk dynamically via pointers.
		* Derived classes should implement, though it can be empty.
		* If it does nothing, empty softkeys will be assigned
		* @since 5.0
		*/
		virtual void CommandSetResourceDynL(TSKPair &lsk, TSKPair &rsk, TSKPair &msk) = 0;

        /**
        * Handles client rect changes
        */
        virtual void HandleClientRectChange() = 0;
        
        /**
        * Gets the id of the previous active view.
        */
        inline TUid GetPreviousViewID()
            { return iPreviousViewID; }
                
        /**
        * Sets the id of the previous active view.
        * @param aUid UID of previous view
        */
        inline void SetPreviousViewID(TUid aUid)
            { iPreviousViewID = aUid; }
	
		/**
		* gets the current view's id
		*/
		virtual TUid Id() const = 0;
                
    //private:
	protected:

        MApiProvider& iApiProvider; // API provider
        
        // Id of the view which was active before this view.
        TUid iPreviousViewID;
        
     
    private:
    	// For each child view class, track
    	// the last command set, dynamic lsk,rsk,msk settings
    	// to update CBA as little as possible
    	TInt iLastCommandSet;
    	TSKPair iLastLSK;
    	TSKPair iLastRSK;
    	TSKPair iLastMSK;
    };

#endif// End of File