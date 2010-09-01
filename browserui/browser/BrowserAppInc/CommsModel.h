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
*      Declaration of class MCommsModel.
*
*
*/


#ifndef MCOMMS_MODEL_H
#define MCOMMS_MODEL_H

// FORWARD DECLARATION

// class CApListItem;
class CApListItemList;
class CActiveApDb;
class CCommsDatabase;

// CLASS DECLARATION

/**
* Observer mixin class for Comms Model changes.
* The main difference between being this observer and MActiveApDbObserver is
* that when an MActiveApDbObserver is notified, the database can be locked
* (so the notification is quite useless). An MCommsModelObserver is notified
* about the change only when the changes are successfully read by
* MCommsModelObserver - that is, in HandleCommsModelChangeL implementation
* you can safely read cached AP-s from CommsModel (and never get KErrLocked).
*/
class MCommsModelObserver
    {
    public:     // new methods

        /**
        * Handle change in comms model (AP-s). Derived classes must provide
        * this method.
        */
        virtual void HandleCommsModelChangeL() = 0;

    };

/**
* Comms model for the WML Browser.
* Provides CommsDb / AP database handling.
*/
class MCommsModel
	{
    public:     // new methods

        /**
        * Get Access Point Engine object.
        * @return Access Point Engine object.
        */
        virtual CActiveApDb& ApDb() const = 0;

        /**
        * Get CommsDb object.
        * @return CommsDb object.
        */
        virtual CCommsDatabase& CommsDb() const = 0;

        /**
        * Get a copy of access points in a list.
        * @return List of access points. The caller is responsible for
        * destroying the returned list.
        * @return List of access points.
        */
        virtual CApListItemList* CopyAccessPointsL() = 0;

        /**
        * Get the pointer to model's cached access points.
        * The owner of the list is the model. List contents may be updated
        * if database update occurs.
        * @return List of access points.
        */
        virtual const CApListItemList* AccessPointsL() = 0;

        /**
        * Refresh the cached access points.
        */
        virtual void RefreshAccessPointsL() = 0;

    public:     // observer support

        /**
        * Add an observer. Duplicates allowed.
        * @param aObserver The observer to add.
        */
        virtual void AddObserverL( MCommsModelObserver& aObserver ) = 0;

        /**
        * Remove an observer. Does nothing if not added / already removed.
        * @param aObserver The observer to remove.
        */
        virtual void RemoveObserver( MCommsModelObserver& aObserver ) = 0;

	};

#endif

// End of file
