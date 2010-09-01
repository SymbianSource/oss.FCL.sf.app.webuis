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
* Description:  AHLE client interface API definition.
 *
*/


// INCLUDE FILES
#include <e32std.h>
#include "AHLEInterface.h"



// ================= MEMBER FUNCTIONS =======================

CAHLEInterface* CAHLEInterface::NewL( TAHLENewType aNewType,
                                      const TDesC& aDatabase,
                                      TUint aPrimarySize,
                                      TUint aSecondarySize,
                                      TAHLEScore aAdaptationSpeed )
{
  CAHLEInterface* apiObject = new (ELeave) CAHLEInterface;
  CleanupStack::PushL( apiObject );
  apiObject->ConstructL( aNewType, aDatabase, aPrimarySize, aSecondarySize, aAdaptationSpeed );
  CleanupStack::Pop();
  return apiObject;
}

EXPORT_C CAHLEInterface* CAHLEInterface::NewL( const TDesC& aDatabase,
                                               TUint aPrimarySize,
                                               TUint aSecondarySize,
                                               TAHLEScore aAdaptationSpeed ){
  return NewL( EAHLENewAllArgs, aDatabase, aPrimarySize, aSecondarySize, aAdaptationSpeed );
}

EXPORT_C  CAHLEInterface* CAHLEInterface::NewL( const TDesC& aDatabase ){
  return NewL( EAHLENewDbOnlyArgs, aDatabase, 0, 0, 0);
}

EXPORT_C  CAHLEInterface* CAHLEInterface::NewL(){
  return NewL( EAHLENewNoArgs, KAHLEInterfaceDummyFile, 0, 0, 0);
}

EXPORT_C  CAHLEInterface* CAHLEInterface::NewL( TUint aPrimarySize ){
  return NewL( EAHLENewPrimarySizeOnlyArgs, KAHLEInterfaceDummyFile, aPrimarySize, 0, 0);
}


// -----------------------------------------------------------------------------
// CAHLEInterface::InitializeAHLE
// Creates the AHLE client if needed.
// Throws exception on failure. This is where the AHLE client finally gets created.
// We defer it as long as possible.
// -----------------------------------------------------------------------------
void CAHLEInterface::InitializeAHLEL()
{
  //If already done return quickly.
  if (iAHLE) return;

  TUint aPrimarySize = 0;
  TUint aSecondarySize = 0;
  TAHLEScore aAdaptationSpeed = 0;

  //Fire the correct contructor. It should map to the CAHLEInterface contructor.
  switch (iNewType)
    {
    case EAHLENewNoArgs:
      iAHLE = CAHLE::NewL();
      break;

    case EAHLENewAllArgs:
      iAHLE = CAHLE::NewL( *iDatabase, iPrimarySize, iSecondarySize, iAdaptationSpeed);
      break;

    case EAHLENewDbOnlyArgs:
      iAHLE = CAHLE::NewL( *iDatabase );
      break;

    case EAHLENewPrimarySizeOnlyArgs:
      //The actual value will be added by the initialization later. We
      //can't change the AHLE constructors.
      iDeferPrimarySize = iPrimarySize;
      iAHLE = CAHLE::NewL();
      break;

    default:
      iAHLE = CAHLE::NewL();
    }

  //Do the deferred reconfigure. It was not done earlier, the values
  //were cached for now. Note that the values could have been set in the
  //constructor as well as here. The trick is to keep track of which
  //ones to set here. If the deferred one was set use it. Otherwise use
  //the one just read in with GetConfiguration()

  iAHLE->GetConfigurationL( aPrimarySize, aSecondarySize, aAdaptationSpeed );
  iAHLE->ReconfigureL( iDeferPrimarySize     ? iDeferPrimarySize    : aPrimarySize,
                       iDeferSecondarySize   ? iDeferSecondarySize  : aSecondarySize,
                       iDeferAdaptationSpeed ? iDeferAdaptationSpeed: aAdaptationSpeed );
}

// -----------------------------------------------------------------------------
// Idle Callback Function
// Called when the thread is not busy. The code will be initialized
// now. If it was initialized explicitly bu other code than this call
// won't do very much. In any case this will always return false. It
// doesn't need to run a second time since it will run to completion.
// -----------------------------------------------------------------------------
TInt CAHLEInterface::AHLEInitializationCB(TAny* thisObj)
{
  ((CAHLEInterface*)thisObj)->InitializeAHLEL();
  return EFalse;
}



// -----------------------------------------------------------------------------
// CAHLEInterface::CAHLEInterface
// -----------------------------------------------------------------------------
CAHLEInterface::CAHLEInterface()
{
  iAHLE = NULL;
  iDatabase = NULL;

  iPrimarySize = 0;
  iSecondarySize = 0;
  iAdaptationSpeed = 0;

  iDeferPrimarySize = 0;
  iDeferSecondarySize = 0;
  iDeferAdaptationSpeed = 0;
}


// -----------------------------------------------------------------------------
// CAHLEInterface::~CAHLEInterface
// C++ destructor
// -----------------------------------------------------------------------------
CAHLEInterface::~CAHLEInterface()
{
if(iIdle != NULL)
	{
	iIdle->Cancel();
	}
  delete iIdle;
  delete iAHLE;
  delete iDatabase;
}


// -----------------------------------------------------------------------------
// CAHLEInterface::IsConnected
// Check if the client has been connected to the server. Note that if the actual
// AHLE client has not been created yet we ARE considered connected. Otherwise we
// would need to connect almost immediately.
// -----------------------------------------------------------------------------
EXPORT_C TBool CAHLEInterface::IsConnected()
    {
      return (iAHLE ? iAHLE->IsConnected(): ETrue);
    }


// ---------------------------------------------------------
// CAHLEInterface::ConstructL
// Light weight constructor. Real work is deferred until later.
// An active idle object is created to do the construction when
// things are not busy. If it is done by a task that explicitly
// requires the AHLE client it will not need to run.
// ---------------------------------------------------------
void CAHLEInterface::ConstructL( TAHLENewType aNewType,
                                 const TDesC& aDatabase,
                                 TUint aPrimarySize,
                                 TUint aSecondarySize,
                                 TAHLEScore aAdaptationSpeed )
{
  iAHLE = NULL;
  iPrimarySize = aPrimarySize;
  iSecondarySize = aSecondarySize;
  iAdaptationSpeed = aAdaptationSpeed;
  iDatabase = aDatabase.Alloc();
  iNewType = aNewType;
  iIdle = CIdle::NewL(CActive::EPriorityIdle);
  iIdle->Start(TCallBack(AHLEInitializationCB, this));
}


// -----------------------------------------------------------------------------
// CAHLEInterface::SetObserverL
// -----------------------------------------------------------------------------
EXPORT_C void CAHLEInterface::SetObserverL( const MAHLEClientObserver* aClientObs )
{
  InitializeAHLEL();
  iAHLE->SetObserverL(aClientObs);
}


// ---------------------------------------------------------
// CAHLEInterface::Reconfigure
// ---------------------------------------------------------
EXPORT_C TInt CAHLEInterface::ReconfigureL( TUint aPrimarySize,
                                            TUint aSecondarySize,
                                            TAHLEScore aAdaptationSpeed )
{
  InitializeAHLEL();
  return iAHLE->ReconfigureL(aPrimarySize, aSecondarySize, aAdaptationSpeed );
}


// ---------------------------------------------------------
// CAHLEInterface::GetConfiguration
// ---------------------------------------------------------
EXPORT_C void CAHLEInterface::GetConfigurationL( TUint& aPrimarySize,
                                                 TUint& aSecondarySize,
                                                 TAHLEScore& aAdaptationSpeed )
{
  InitializeAHLEL();
  iAHLE->GetConfigurationL(aPrimarySize, aSecondarySize, aAdaptationSpeed );
}


// ----------------------------------------------------------------
// CAHLEInterface::GetParameters
// ----------------------------------------------------------------
EXPORT_C TInt CAHLEInterface::GetParameters( TAHLESortOrder& /* aOrder */ ) const
{
  return KErrNotSupported;
}


// ----------------------------------------------------------------
// CAHLEInterface::GetParameters
// ----------------------------------------------------------------
EXPORT_C TInt CAHLEInterface::GetParameters( TAHLESortOrder& /* aOrder */,
                                             TAny* /* aReserved */  ) const
{
  return KErrNotSupported;
}


// ----------------------------------------------------------------
// CAHLEInterface::SetParameters
// ----------------------------------------------------------------
EXPORT_C TInt CAHLEInterface::SetParameters( TAHLESortOrder /* aOrder */ )
{
  return KErrNotSupported;
}

EXPORT_C TInt CAHLEInterface::SetParameters( TAHLESortOrder /* aOrder */,
                                             TAny* /* aReserved */ )
{
  return KErrNotSupported;
}


// ---------------------------------------------------------
// CAHLEInterface::NewAccessL
// ---------------------------------------------------------
EXPORT_C TInt CAHLEInterface::NewAccessL( const TDesC& aItem,
                                          const TDesC& aItemName )
{
  InitializeAHLEL();
  return (iAHLE->NewAccessL(aItem, aItemName));
}


EXPORT_C void CAHLEInterface::NewAccessL( TRequestStatus& aStatus,
                                          const TDesC& aItem,
                                          const TDesC& aItemName )
{
  InitializeAHLEL();
  iAHLE->NewAccessL(aStatus, aItem, aItemName);
}


// ----------------------------------------------------------------
// CAHLEInterface::AdaptiveListL
// ----------------------------------------------------------------
EXPORT_C TInt CAHLEInterface::AdaptiveListL( CDesCArray&  aItems,
                                             CDesCArray&  aItemNames,
                                             const TInt  aSize,
                                             const TDesC& aMatch,
                                             const TAHLEState aState )
{
  InitializeAHLEL();
  return (iAHLE->AdaptiveListL( aItems, aItemNames, aSize, aMatch, aState ));
}


// ---------------------------------------------------------
// CAHLEInterface::OrderByScoreL
// ---------------------------------------------------------
//
EXPORT_C TInt CAHLEInterface::OrderByScoreL( CDesCArray& aItems,
                                             CDesCArray& aItemsSorted )
{
  InitializeAHLEL();
  return (iAHLE->OrderByScoreL( aItems, aItemsSorted ));
}

EXPORT_C TInt CAHLEInterface::OrderByScoreL( CDesCArray& /* aItems */,
                                             RArray<TInt>& /* aSortOrder */ )
{
  return KErrNotSupported;
}


// ---------------------------------------------------------
// CAHLEInterface::RemoveL
// ---------------------------------------------------------
EXPORT_C TInt CAHLEInterface::RemoveL( const TDesC& aItem )
{
  InitializeAHLEL();
  return (iAHLE->RemoveL( aItem ));
}

EXPORT_C void CAHLEInterface::RemoveL( const TDesC& aItem,
                                       TRequestStatus& aStatus )
{
  InitializeAHLEL();
  iAHLE->RemoveL( aItem, aStatus );
}

EXPORT_C void CAHLEInterface::RemoveL( const CDesCArray& aItems,
                                       TRequestStatus &aStatus)
{
  InitializeAHLEL();
  iAHLE->RemoveL( aItems, aStatus );
}

EXPORT_C TInt CAHLEInterface::RemoveL(const CDesCArray& aItems )
{
  InitializeAHLEL();
  return (iAHLE->RemoveL( aItems ));
}


// ---------------------------------------------------------
// CAHLEInterface::RemoveMatching
// ---------------------------------------------------------
EXPORT_C TInt CAHLEInterface::RemoveMatchingL( const TDesC& aMatch )
{
  InitializeAHLEL();
  return (iAHLE->RemoveMatchingL( aMatch ));
}

EXPORT_C void CAHLEInterface::RemoveMatchingL( const TDesC& aMatch,
                                               TRequestStatus& aStatus )
{
  InitializeAHLEL();
  iAHLE->RemoveMatchingL( aMatch, aStatus);
}


// ---------------------------------------------------------
// CAHLEInterface::Rename
// ---------------------------------------------------------
EXPORT_C TInt CAHLEInterface::RenameL( const TDesC& aItem,
                                       const TDesC& aNewName )
{
  InitializeAHLEL();
  return (iAHLE->RenameL( aItem, aNewName ));
}


// ---------------------------------------------------------
// CAHLEInterface::GetName
// ---------------------------------------------------------
//
EXPORT_C TInt CAHLEInterface::GetNameL( const TDesC& /* aItem */,
                                        TDesC& /* aName */ )
{
  return KErrNotSupported;
}


// ---------------------------------------------------------
// CAHLEInterface::Clear
// ---------------------------------------------------------
EXPORT_C TInt CAHLEInterface::Clear()
{
  //Exception may be thrown but is not expected. Return something
  //harmless in that case.
  TRAPD( error, InitializeAHLEL());
  return (error ? 0 : iAHLE->Clear());
}


// ---------------------------------------------------------
// CAHLEInterface::Flush
// ---------------------------------------------------------
EXPORT_C TInt CAHLEInterface::Flush()
{
  //Exception may be thrown but is not expected. Return something
  //harmless in that case.
  TRAPD( error, InitializeAHLEL());
  return (error ? 0 : iAHLE->Flush());
}

