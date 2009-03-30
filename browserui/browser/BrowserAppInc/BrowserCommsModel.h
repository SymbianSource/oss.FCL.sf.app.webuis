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
* Description:  Declaration of class CBrowserCommsModel.
*
*
*/


#ifndef BROWSER_COMMS_MODEL_H
#define BROWSER_COMMS_MODEL_H

//  INCLUDES

#include <e32base.h>
#include <ActiveApDb.h>
#include "CommsModel.h"

// FORWARD DECLARATION

// class CApListItem;
class CApListItemList;

// CLASS DECLARATION

/**
*  Comms model for the Browser.
*  Provides CommsDb / AP database handling. Because querying AP-s is slow, but
*  there should not be many of them, this class keeps a cached copy of AP-s.
*  AP db is observed, and the cached list of AP-s is kept up to date.
*  However, if database changes but we cannot get new data (database locked
*  etc., the cached list is kept (until we can get the new data successfully)).
*
*  @lib Browser.app
*  @since Series 60 1.2
*/
class CBrowserCommsModel:
                public CBase, public MCommsModel, public MActiveApDbObserver
	{
	public:     // construction

        /**
        * Two-phased constructor. Leaves on failure.
        * @return The constructed model
        */
        static CBrowserCommsModel* NewL();

        /**
        * Destructor.
        */
        virtual ~CBrowserCommsModel();

	protected:  // construction

        /**
        * Constructor.
        */
		CBrowserCommsModel();

        /**
        * Second phase constructor. Leaves on failure.
        */
		void ConstructL();

    public:     // from MCommsModel

        /**
        * Get Access Point Engine object.
        * @since Series 60 1.2
        * @return Access Point Engine object
        */
        CActiveApDb& ApDb() const;

        /**
        * Get CommsDb object.
        * @since Series 60 1.2
        * @return CommsDb object
        */
        CCommsDatabase& CommsDb() const;

        /**
        * Get a copy of access points in a list.
        * The caller is responsible for destroying the returned list.
        * @since Series 60 1.2
        * @return List of access points.
        */
        CApListItemList* CopyAccessPointsL();

        /**
        * Get pointer to model's cached access points.
        * The owner of the list is the model. List contents may be updated
        * if database update occurs.
        * @since Series 60 1.2
        * @return List of access points.
        */
        const CApListItemList* AccessPointsL();

        /**
        * Refresh the cached access points.
        */
        void RefreshAccessPointsL();

        /**
        * Add an observer. Duplicates allowed.
        * @since Series 60 1.2
        * @param aObserver The observer to add.
        */
        void AddObserverL( MCommsModelObserver& aObserver );

        /**
        * Remove an observer. Does nothing if not added / already removed.
        * @since Series 60 1.2
        * @param aObserver The observer to remove.
        */
        void RemoveObserver( MCommsModelObserver& aObserver );

    public:     // from MActiveApDbObserver

        /**
        * Handle database event (refresh cached AP list on change).
        * @since Series 60 1.2
        * @param aEvent Database-related event.
        */
        void HandleApDbEventL( MActiveApDbObserver::TEvent aEvent );

    private:    // new methods

        /**
        * Get all access points (implementation detail). If database is
        * inaccessible, old data remains and update becomes pending.
        * @since Series 60 1.2
        */
        void GetAccessPointsL();

        /**
        * Get all access points (implementation detail).
        * @since Series 60 1.2
        * @param aList Append access points to this list
        */
        void DoGetAccessPointsL( CApListItemList& aList ) const;

        /**
        * Successfully got new AP data. Notify observers.
        * @since Series 60 1.2
        */
        void NotifyObserversL();

    private:    // data

        /**
        * Access Point database.
        */
        CActiveApDb* iDb;

        /**
        * Cached AP list. Since querying the AP Engine is expensive (slow),
        * we maintain a cached list of AP-s for quick access. Hopefully
        * there are not too many of them.
        */
        CApListItemList* iCachedApList;

        /**
        * Array of observer pointers. Array owned, contents not.
        */
        CArrayPtrFlat<MCommsModelObserver>* iObservers;

        /**
        * ETrue if database has changed, but could not get new data yet.
        */
        TBool iUpdatePending;
        
        
        /**
        *Etrue if referesh needs to be done on next access to cached data
        *Added as a result of performance tuning
        */
        TBool iDelayedRefresh;
	};

#endif

// End of file
