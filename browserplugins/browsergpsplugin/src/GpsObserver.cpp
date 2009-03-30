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
*
*/


// INCLUDE FILES
#include <e32std.h>
#include <e32cmn.h>
#include "GpsObserver.h"
#include "GpsPlugin.h"

// for debug #define BROWSER_GPS_PLUGIN_LOG

#ifdef BROWSER_GPS_PLUGIN_LOG

#include <flogger.h>

// output goes in c:\Logs\gps  (create this dir manually if it doesn't exist)

_LIT(LOG_DIR, "gps");
_LIT(LOG_NAME, "gps.log");

TBuf16<256> formatBuf;
_LIT16(formatInt,"[%d]");

#define LOG_E \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, _L("E")); \
  formatBuf.Format(formatInt, e); \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, formatBuf)
#define LOG_CONNECT \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, _L("RPositionerServer.Connect"))
#define LOG_NUMMODULES \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, _L("NUMMODULES")); \
  formatBuf.Format(formatInt, numberOfModules); \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, formatBuf)
#define LOG_MODULE \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, _L("MODULE")); \
  formatBuf.Format(formatInt, i); \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, formatBuf)
#define LOG_SATMODULE \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, _L("SATELLITE MODULE")); \
  formatBuf.Format(formatInt, i); \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, formatBuf)
#define LOG_OPEN \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, _L("RPositioner.Open"))
#define LOG_OPTIONS \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, _L("RPositioner.SetUpdateOptions"))
#define LOG_REQUESTOR \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, _L("RPositioner.SetRequestor"))
#define LOG_REQUESTFIELD \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, _L("HPositionGenericInfo.SetRequestedField"))
#define LOG_POSITIONUPDATE \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, _L("RPositioner.NotifyPositionUpdate"))
#define LOG_RESPONSE \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, _L("response"))
#define LOG_GETPOSITION \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, _L("HPositionGenericInfo.GetPosition"))
#define LOG_LATLNGALT \
  int lat = iPosition.Latitude(); \
  int lon = iPosition.Longitude(); \
  int alt = iPosition.Altitude(); \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, formatBuf); \
  formatBuf.Format(formatInt, lat); \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, formatBuf); \
  formatBuf.Format(formatInt, lon); \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, formatBuf); \
  formatBuf.Format(formatInt, alt); \
  RFileLogger::Write(LOG_DIR, LOG_NAME, EFileLoggingModeAppend, formatBuf)


#else

#define LOG_E
#define LOG_CONNECT
#define LOG_NUMMODULES
#define LOG_MODULE
#define LOG_SATMODULE
#define LOG_OPEN
#define LOG_OPTIONS
#define LOG_REQUESTOR
#define LOG_REQUESTFIELD
#define LOG_POSITIONUPDATE
#define LOG_RESPONSE
#define LOG_GETPOSITION
#define LOG_LATLNGALT

#endif


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CGpsObserver::NewL
// Symbian 2-phase constructor.
// -----------------------------------------------------------------------------
CGpsObserver* CGpsObserver::NewL( CGpsPlugin* aPlugin )
    {
    CGpsObserver* self = new (ELeave) CGpsObserver( aPlugin );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

// -----------------------------------------------------------------------------
// CGpsObserver::CGpsObserver
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
CGpsObserver::CGpsObserver( CGpsPlugin* aPlugin )
    : CActive( EPriorityStandard ),
      iGpsPlugin( aPlugin )
    {
    CActiveScheduler::Add( this );
    }

// -----------------------------------------------------------------------------
// CGpsObserver::ConstructL
// 2nd-phase constructor.
// -----------------------------------------------------------------------------
void CGpsObserver::ConstructL()
    {
    TInt e; // for error codes

    // 1. connect to the position server
    e = iServer.Connect();
    LOG_E;
    LOG_CONNECT;
    User::LeaveIfError( e );

    // 2. find the satellite module (see the BLID application source
    // code CSattelliteEng.cpp)
    TUint numberOfModules = 0;
    e = iServer.GetNumModules( numberOfModules );
    LOG_E;
    LOG_NUMMODULES;
    User::LeaveIfError( e );

    TPositionModuleInfo moduleInfo;
    TInt i;
    for ( i = 0; i < numberOfModules ; i++ )
        {
        LOG_MODULE;
        e = iServer.GetModuleInfoByIndex( i, moduleInfo );
        LOG_E;
        if ( ( e == KErrNone ) &&
             ( moduleInfo.Capabilities()
               & TPositionModuleInfo::ECapabilitySatellite ) ) /* bit test */
            {
            break;
            }
        }

    if ( i == numberOfModules )
        {
        // leave if no GPS
        User::Leave( KErrNotFound );
        }

    // 3. open the positioner with the satellite module
    LOG_SATMODULE;
    TPositionModuleId moduleId = moduleInfo.ModuleId();
    e = iPositioner.Open( iServer, moduleId );
    LOG_E;
    User::LeaveIfError( e );
    LOG_OPEN;

    // 4. set the requestor on the positioner (something for security)
    iPositioner.SetRequestor( CRequestor::ERequestorService,
                              CRequestor::EFormatApplication,
                              _L("BrowserGpsPlugin")/**srvName*/ );
    LOG_REQUESTOR;

    // 5. request position notification
    iPositioner.NotifyPositionUpdate( iPositionInfo, iStatus );
    SetActive();
    LOG_POSITIONUPDATE;
    }

// -----------------------------------------------------------------------------
// CGpsObserver::~CGpsObserver()
// Destructor
// -----------------------------------------------------------------------------
CGpsObserver::~CGpsObserver()
    {
    Cancel();
    iPositioner.Close();
    iServer.Close();

    delete iEventHandler;
    }

// -----------------------------------------------------------------------------
// CSystemInfoBatteryObserver::RequestNotification
// -----------------------------------------------------------------------------
void CGpsObserver::RequestNotificationL( const NPString& aEvent )
    {
    // remove and free current handler
    HBufC8* handler = iEventHandler;
    iEventHandler = NULL;
    delete handler;

    //store the event handler function name
    TInt length = aEvent.UTF8Length;
    handler = HBufC8::NewL( length+1 );

    TPtr8 tptr = handler->Des();
    tptr.Copy( (const TUint8 *)aEvent.UTF8Characters, length );

    //Remove any arguments passed in the with the function name
    TInt pos = handler->LocateReverse( TChar('(') );
    if ( pos != KErrNotFound )
        {
        length = pos;
        }

    tptr.SetLength( length );
    tptr.ZeroTerminate();

    iEventHandler = handler;

    // monitoring is already active
    }

// -----------------------------------------------------------------------------
// CSystemInfoBatteryObserver::RunL
// -----------------------------------------------------------------------------
void CGpsObserver::RunL()
    {
    // if status ok and exists, update values and invoke callback
    if ( iStatus == KErrNone )
       {
       // update saved values
       iPositionInfo.GetPosition( iPosition );
       LOG_GETPOSITION;
       LOG_LATLNGALT;

       iTickCount++;
        if ( iEventHandler )
           {
           iGpsPlugin->InvokeCallback( *iEventHandler, NULL, 0 );
           }
       }

    // request next update
    iPositioner.NotifyPositionUpdate( iPositionInfo, iStatus );
    SetActive();
    }

// -----------------------------------------------------------------------------
// CGpsObserver::RunError
// -----------------------------------------------------------------------------
TInt CGpsObserver::RunError( TInt /*aError*/ )
    {
    return KErrNone;
    }

// -----------------------------------------------------------------------------
// CGpsObserver::DoCancel
// -----------------------------------------------------------------------------
void CGpsObserver::DoCancel()
    {
    iPositioner.CancelRequest( EPositionerNotifyPositionUpdate );
    }

// -----------------------------------------------------------------------------
// CGpsObserver::Latitude
// -----------------------------------------------------------------------------
TReal CGpsObserver::Latitude() const
    {
    return iPosition.Latitude();
    }

// -----------------------------------------------------------------------------
// CGpsObserver::Longitude
// -----------------------------------------------------------------------------
TReal CGpsObserver::Longitude() const
    {
    return iPosition.Longitude();
    }

// -----------------------------------------------------------------------------
// CGpsObserver::Altitude
// -----------------------------------------------------------------------------
TReal CGpsObserver::Altitude() const
    {
    return iPosition.Altitude();
    }

// -----------------------------------------------------------------------------
// CGpsObserver::HorizontalAccuracy
// -----------------------------------------------------------------------------
TReal CGpsObserver::HorizontalAccuracy() const
    {
    return iPosition.HorizontalAccuracy();
    }

// -----------------------------------------------------------------------------
// CGpsObserver::VerticalAccuracy
// -----------------------------------------------------------------------------
TReal CGpsObserver::VerticalAccuracy() const
    {
    return iPosition.VerticalAccuracy();
    }

// -----------------------------------------------------------------------------
// CGpsObserver::TIckCount
// -----------------------------------------------------------------------------
TInt CGpsObserver::TickCount() const
    {
    return iTickCount;
    }

//  End of File
