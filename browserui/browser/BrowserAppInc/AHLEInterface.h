/*
* Copyright (c) 2006 Nokia Corporation and/or its subsidiary(-ies).
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
* Description:  This file contains the header file of the CAHLEInterface class.
 *
*/


// This is a wrapper class for the AHLE client. Its intent is to defer
// the construction until after the browser startup is
// complete. Documentation is not duplicated. Only new functions or
// those modified are described here. See AHLE.h for the detailed
// descriptions.


#ifndef AHLEINTERFACE_H
#define AHLEINTERFACE_H

#include <AHLE.h>

_LIT(KAHLEInterfaceDummyFile, "dummy_file");

class CAHLEInterface: public CBase, public MAHLEClientAPI
    {
    public:

    enum TAHLENewType
      {
        EAHLENewNoArgs,
        EAHLENewAllArgs,
        EAHLENewDbOnlyArgs,
        EAHLENewPrimarySizeOnlyArgs
      };

    virtual ~CAHLEInterface();

    /** Engine start. */
    IMPORT_C static CAHLEInterface* NewL();
    IMPORT_C static CAHLEInterface* NewL( const TDesC& aDatabase );
    IMPORT_C static CAHLEInterface* NewL( TUint aPrimarySize );
    IMPORT_C static CAHLEInterface* NewL( const TDesC& aDatabase,
                                          TUint aPrimarySize,
                                          TUint aSecondarySize,
                                          TAHLEScore aAdaptationSpeed );


    /** Check if the client has been connected to the server */
    IMPORT_C TBool IsConnected();

    /** Set Observer */
    IMPORT_C void SetObserverL( const MAHLEClientObserver* aObserver );

    /** Engine reconfiguration. */
    IMPORT_C TInt ReconfigureL( TUint aPrimarySize,
                                TUint aSecondarySize,
                                TAHLEScore aAdaptationSpeed );

    /** Current engine configuration. */
    IMPORT_C void GetConfigurationL( TUint& aPrimarySize,
                                     TUint& aSecondarySize,
                                     TAHLEScore& aAdaptationSpeed );

    /** Get adaptive list parameters. */
    IMPORT_C TInt GetParameters( TAHLESortOrder& aOrder ) const;
    IMPORT_C TInt GetParameters( TAHLESortOrder& aOrder,
                                 TAny* aReserved ) const;


    /** Set adaptive list parameters. */
    IMPORT_C TInt SetParameters( TAHLESortOrder aOrder );
    IMPORT_C TInt SetParameters( TAHLESortOrder aOrder,
                                 TAny* aReserved );


    ////////////////////////////////////////////////////////////////
    // ADAPTIVE LIST METHODS
    ////////////////////////////////////////////////////////////////

    /** Logging of new access. Synchronous. Used for string data. */
    IMPORT_C TInt NewAccessL( const TDesC& aItem,
                              const TDesC& aItemName );

    /** Asynchronous version of NewAccessL(). */
    IMPORT_C void NewAccessL( TRequestStatus& aStatus,
                              const TDesC& aItem,
                              const TDesC& aItemName );

   /** Get adaptive list. Here the user has the option of selecting sites/groups  */
    IMPORT_C TInt AdaptiveListL( CDesCArray&  aItems,
                                 CDesCArray&  aItemNames,
                                 const TInt  aSize,
                                 const TDesC& aMatch,
                                 const TAHLEState aState );

    /**
     * Sorts items by their scores. Used e.g. in Favorite
     * Links in Page feature within browser application.
     */
    IMPORT_C TInt OrderByScoreL( CDesCArray& aItems, CDesCArray& aItemsSorted );
    IMPORT_C TInt OrderByScoreL( CDesCArray& aItems, RArray<TInt>& aItemsSorted );


    /** Remove item(s). */
    IMPORT_C TInt RemoveL( const TDesC& aItem );
    IMPORT_C void RemoveL( const TDesC& aItem, TRequestStatus& aStatus );
    IMPORT_C TInt RemoveL( const CDesCArray& aItems );
    IMPORT_C void RemoveL( const CDesCArray& aItems, TRequestStatus& aStatus );


    /** Remove matching items. */
    IMPORT_C TInt RemoveMatchingL( const TDesC& aMatch );
    IMPORT_C void RemoveMatchingL( const TDesC& aMatch, TRequestStatus& aStatus );


    /**  Rename item. */
    IMPORT_C TInt RenameL( const TDesC& aItem, const TDesC& aNewName );


    /** Get item name. */
    IMPORT_C TInt GetNameL( const TDesC& aItem, TDesC& aName );


    /** Clear all adaptive list items. */
    IMPORT_C TInt Clear();


    /** Flush cached items to persistent storage. */
    IMPORT_C TInt Flush();


    private:

    CAHLEInterface();

    static TInt AHLEInitializationCB(TAny* thisObj);


    void ConstructL( TAHLENewType aNewType,
                     const TDesC& aDatabase,
                     TUint aPrimarySize,
                     TUint aSecondarySize,
                     TAHLEScore aAdaptationSpeed );

     static CAHLEInterface* NewL( TAHLENewType aNewType,
                                  const TDesC& aDatabase,
                                  TUint aPrimarySize,
                                  TUint aSecondarySize,
                                  TAHLEScore aAdaptationSpeed );

    // This final section contains the modifications from the CAHLE
    // class. The only new method facilitates lazy construction. If not
    // needed it will still work.

    private:

    /**
     * AHLE Engine Initialization
     * Performs the deferred loading of the AHLE dll and its
     * initialization.  Throws an exception on failure. Does nothing if
     * already initialized.
     */
    void InitializeAHLEL();


    //This private data allows the actual construction of the AHLE
    //object to be deferred until actually needed. This will defer the
    //loading of the server dll.

    private:

    CAHLE* iAHLE;
    TUint iPrimarySize;
    TUint iSecondarySize;
    TAHLEScore iAdaptationSpeed;
    TUint iDeferPrimarySize;
    TUint iDeferSecondarySize;
    TAHLEScore iDeferAdaptationSpeed;
    HBufC* iDatabase;
    TAHLENewType iNewType;
    CIdle* iIdle;
    };

#endif /*  AHLEINTERFACE_H */
