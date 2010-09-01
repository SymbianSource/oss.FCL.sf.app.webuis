/*
* Copyright (c) 2002-2005 Nokia Corporation and/or its subsidiary(-ies).
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

#include "BavpNPObject.h"
#include "BavpLogger.h"

#include <badesca.h>
#include <stdlib.h>
#include <string.h>
#include <npscript.h>

#include  "BavpPlugin.h"
#include  "BavpController.h"
#include  "BavpControllerVideo.h"
#include  "BavpControllerAudio.h"


const int NUM_METHOD_IDENTIFIERS = 5;
const int ID_PLAY = 0;
const int ID_STOP = 1;
const int ID_PAUSE = 2;
const int ID_FASTFORWARD = 3;
const int ID_REWIND = 4;
static NPIdentifier bavpMethodIdentifiers[NUM_METHOD_IDENTIFIERS];
static const NPUTF8 *bavpIdentifierNames[NUM_METHOD_IDENTIFIERS] = {
    "play",
    "stop",
    "pause",
    "fastforward",
    "rewind",
};

const int NUM_PROPERTY_IDENTIFIERS = 10;
const int ID_VOLUME = 0;
const int ID_POSITION = 1;
const int ID_DURATION = 2;
const int ID_STATE_CHANGED_CALLBACK = 3;
const int ID_STATE = 4;
const int ID_ISVIDEOCLIP = 5;
const int ID_ISSEEKABLE = 6;
const int ID_CLIPNAME = 7;
const int ID_FULLSCREENMODE = 8;
const int ID_MIMETYPE = 9;
static NPIdentifier bavpPropertyIdentifiers[NUM_PROPERTY_IDENTIFIERS];
static const NPUTF8 *bavpPropertyIdentifierNames[NUM_PROPERTY_IDENTIFIERS] = {
    "volume",
    "position",
    "duration",
    "statechangedcallback",
    "state",
    "isvideoclip",
    "isseekable",
    "clipname",
    "fullscreenmode",
    "mimetype",
};
    
static bool _initializedIdentifiers = false;    
static void initializeIdentifiers()
{
    if (!_initializedIdentifiers) {
        _initializedIdentifiers = true;
        NPN_GetStringIdentifiers (bavpPropertyIdentifierNames, NUM_PROPERTY_IDENTIFIERS, bavpPropertyIdentifiers);
        NPN_GetStringIdentifiers (bavpIdentifierNames, NUM_METHOD_IDENTIFIERS, bavpMethodIdentifiers);
    }
};


_LIT8(KNone,"None");
_LIT8(KBuffering,"Buffering");
_LIT8(KPlaying,"Playing");
_LIT8(KPlayComplete,"Playcomplete");
_LIT8(KStopped,"Stopped");
_LIT8(KPaused,"Paused");
_LIT8(KBadContent,"Badcontent");
_LIT8(KFastForwarding,"Fastforwarding");
_LIT8(KFFComplete,"Fastforward complete");
_LIT8(KRewinding,"Rewinding");
_LIT8(KRewindComplete,"Rewinding complete");
_LIT8(KInvalidState,"Invalid State");


static const TPtrC8 getState(int state)
{
    TPtrC8 ret;

    switch (state)
    {
        case EBavpNone:
            ret.Set(KNone);
            break;
            
        case EBavpBuffering:
            ret.Set(KBuffering);
            break;
            
        case EBavpPlaying:
            ret.Set(KPlaying);
            break;
            
        case EBavpPlayComplete:
            ret.Set(KPlayComplete);
            break;
            
        case EBavpStopped:
            ret.Set(KStopped);
            break;
            
        case EBavpPaused:
            ret.Set(KPaused);
            break;
            
        case EBavpBadContent:
            ret.Set(KBadContent);
            break;
            
        case EBavpFastForwarding:
            ret.Set(KFastForwarding);
            break;
            
        case EBavpFastForwardComplete:
            ret.Set(KFFComplete);
            break;
            
        case EBavpRewinding:
            ret.Set(KRewinding);
            break;
            
        case EBavpRewindComplete:
            ret.Set(KRewindComplete);
            break;
            
        default:
            ret.Set(KInvalidState);
            break;
    }
    return ret;
}

static void reportVolumeResults(int vol,CBavpController *ctlr)
{
    Log(ETrue,_L("SetProperty: Volume"));

    TInt setvol = ctlr->GetPlayerVolume();
    Log(ETrue,_L("Requested volume: "), vol);
    Log(ETrue,_L("Volume set to: "),setvol);

    
    if ( setvol == vol || setvol == KCRVolumeMax || setvol == KCRVolume0 )
    {
        Log(ETrue,_L("Result: Passed"));
    }
    else
    {
        Log(ETrue,_L("Result: Failed"));
    }
}

static void reportPositionResultsL(int pos, CBavpController *ctlr)
{
    Log(ETrue,_L("SetProperty: Position"));
    
    if ( !(ctlr->IsClipSeekable()) && !(ctlr->IsClipLocalMedia()) )
    {
        Log(ETrue,_L("Cannot set position for non-seekable and non-local media content"));
        return;
    }

    TTimeIntervalMicroSeconds ms = ctlr->getPositionL();
    TTimeIntervalMicroSeconds durationms = ctlr->Duration();
  	
    int setpos = (int)ms.Int64();
    int duration = (int)durationms.Int64();
    
    Log(ETrue,_L("Requested position: "), pos);
    Log(ETrue,_L("Position set to: "),setpos);
    
    if ( setpos == pos || setpos == duration || setpos == 0)
    {	
        Log(ETrue,_L("Result: Passed"));
    }
    else
    {
        Log(ETrue,_L("Result: Failed"));
    }	
}

static void reportPlayResultsL(int initState, CBavpController *ctlr)
{
    if ( ctlr->IsClipVideo())
    {
        TPtrC8 expectedState = getState(EBavpPlaying);
        TPtrC8 expectedState2 = getState(EBavpBuffering);
        TPtrC8 actualState = getState(ctlr->State());
    	
        if ( initState == EBavpStopped ||
             initState == EBavpPlayComplete ||
             initState == EBavpRewindComplete ||
             initState == EBavpFastForwardComplete ||
             initState == EBavpBadContent)
        {
            TTimeIntervalMicroSeconds ms = ctlr->getPositionL();
            int actualpos = (int)ms.Int64();

            Log(ETrue,_L("Expected Position: "), 0);
            Log(ETrue,_L("Actual Position: "),actualpos);
        	
            if ( actualpos != 0)
            {
                Log(ETrue,_L("Result: Position test failed"));
            }
            else
            {
                Log(ETrue,_L("Result:Position test passed"));
            }
        }
    	
        if ( ctlr->IsClipOnDemand() && initState == EBavpStopped )
        {
            Log(ETrue,_L8("Expected State: "), expectedState2);
            Log(ETrue,_L8("Actual State: "), actualState);
        }
        else
        {
            Log(ETrue,_L8("Expected State: "), expectedState);
            Log(ETrue,_L8("Actual State: "), actualState);	
        }
    	
        if ( actualState.Compare(expectedState) != 0 && actualState.Compare(expectedState2) != 0)
        {
            Log(ETrue,_L("Result: State test failed"));
        }
        else
        {
            Log(ETrue,_L("Result: State test passed"));
        }
    }
    else
    {
        TPtrC8 expectedState = getState(EBavpPlaying);
        TPtrC8 actualState = getState(ctlr->State());
    	
        Log(ETrue,_L8("Expected State: "), expectedState);
        Log(ETrue,_L8("Actual State: "), actualState);
    	
        if ( expectedState.Compare(actualState) != 0)
        {
            Log(ETrue,_L("Result: Failed"));
        }
        else
        {
            Log(ETrue,_L("Result: Passed"));
        }
    }
}

static void reportStopResultsL(CBavpController *ctlr)
{
    TPtrC8 expectedState = getState(EBavpStopped);
    TPtrC8 actualState = getState(ctlr->State());
    TTimeIntervalMicroSeconds ms = ctlr->getPositionL();
    int position = (int)ms.Int64();

    Log(ETrue,_L8("Expected State: "), expectedState);
    Log(ETrue,_L8("Actual State: "), actualState);
    Log(ETrue,_L("Current Position: "), position);
	
    if ( (position != 0) || (expectedState.Compare(actualState) != 0) )
    {
        Log(ETrue,_L("Result: Failed"));
    }
    else
    {
        Log(ETrue,_L("Result: Passed"));
    }
}

static void reportRewindResults(CBavpController *ctlr)
{
    if ( ctlr->IsClipVideo() )
    {
        if ( !(ctlr->IsClipSeekable()) && !(ctlr->IsClipLocalMedia()) )
        {
            Log(ETrue,_L("Result: Rewind is not applicable to non-seekable and non-local media content"));
        }
        else
        {
            TPtrC8 expectedState = getState(EBavpRewinding);
            TPtrC8 expectedState2 = getState(EBavpRewindComplete);
            TPtrC8 actualState = getState(ctlr->State());

            Log(ETrue,_L8("Expected State: "), expectedState);
            Log(ETrue,_L8("Actual State: "), actualState);

            if ( (expectedState.Compare(actualState) != 0) && (expectedState2.Compare(actualState) != 0) )
            {
                Log(ETrue,_L("Result: Failed"));
            }
            else
            {
                Log(ETrue,_L("Result: Passed"));
            }
        }
    }
    else
    {
        Log(ETrue,_L("Result: Rewind is not applicable to Audio"));
    }
	
}

static void reportFFResults(CBavpController *ctlr)
{
    if ( ctlr->IsClipVideo() )
    {
        if ( !(ctlr->IsClipSeekable()) && !(ctlr->IsClipLocalMedia()) )
        {
            Log(ETrue,_L("Result: Fastforward is not applicable to non-seekable and non-local media content"));
        }
        else
        {
            TPtrC8 expectedState = getState(EBavpFastForwarding);
            TPtrC8 expectedState2 = getState(EBavpFastForwardComplete);
            TPtrC8 actualState = getState(ctlr->State());

            Log(ETrue,_L8("Expected State: "), expectedState);
            Log(ETrue,_L8("Actual State: "), actualState);

            if ( (expectedState.Compare(actualState) != 0) && (expectedState2.Compare(actualState) != 0) )
            {
                Log(ETrue,_L("Result: Failed"));
            }
            else
            {
                Log(ETrue,_L("Result: Passed"));
            }
        }
    }
    else
    {
        Log(ETrue,_L("Result: Fastforward is not applicable to Audio"));
    }
	
}

static void reportPauseResultsL(CBavpController *ctlr)
{
    TBool seekablevideo = (ctlr->IsClipSeekable() && ctlr->IsClipVideo());
    TBool seekableaudio = (ctlr->IsClipSeekable() && !ctlr->IsClipVideo() 
            && (ctlr->State() == EBavpPaused || ctlr->State() == EBavpPlaying));

    if ( seekablevideo || seekableaudio )
    {
        TPtrC8 expectedState = getState(EBavpPaused);
        TPtrC8 actualState = getState(ctlr->State());

        Log(ETrue,_L8("Expected State: "), expectedState);
        Log(ETrue,_L8("Actual State: "), actualState);

        if ( (expectedState.Compare(actualState) != 0) )
        {
            Log(ETrue,_L("Result: Failed"));
        }
        else
        {
            Log(ETrue,_L("Result: Passed"));
        }
    	
    }
    else
    {
        reportStopResultsL(ctlr);
    }
}
    
static void BavpDesToNpvariant(TPtrC& string, NPVariant*& variant)
{
    char* newString = NULL;
    if (string.Length()) {
        newString = new char[string.Length()];
    }
    if (newString) {
        TPtr8 newStringPtr((unsigned char*)newString, 0, string.Length());
        newStringPtr.Copy(string);
        STRINGN_TO_NPVARIANT(newString, string.Length(), *variant);
    }
}

static void BavpDesToNpvariant(TPtrC8& string, NPVariant*& variant)
{
    char* newString = NULL;
    if (string.Length()) {
        newString = new char[string.Length()];
    }
    if (newString) {
        Mem::Copy(newString, string.Ptr(), string.Length());
        STRINGN_TO_NPVARIANT(newString, string.Length(), *variant);
    }
}
    
NPObject *BavpAllocate ()
{
    BavpNPObject *newInstance = (BavpNPObject *)User::AllocZ (sizeof(BavpNPObject));   
    
    return (NPObject *)newInstance;
}

void BavpInvalidate ()
{
    // Make sure we've released any remainging references to JavaScript
    // objects.
}

void BavpDeallocate (BavpNPObject *obj) 
{
    if (obj->stateChangedCallback)
        NPN_ReleaseObject((struct NPObject*)obj->stateChangedCallback);  
    User::Free ((void *)obj);
}    

bool BavpHasMethod(BavpNPObject */*obj*/, NPIdentifier name)
{
    int i;
    for (i = 0; i < NUM_METHOD_IDENTIFIERS; i++) {
        if (name == bavpMethodIdentifiers[i]){
            return true;
        }
    }
    return false;
}


bool BavpInvokeL(BavpNPObject *obj, NPIdentifier name, const NPVariant */* args */, uint32_t /* argCount */, NPVariant *result)
{
    if (!obj->plugin) {
        NULL_TO_NPVARIANT(*result);
        return true;    
    }    

    CBavpController *ctlr = obj->plugin->Controller();
    
    if (!ctlr)
    {
        NULL_TO_NPVARIANT(*result);
        return true; 
    }
    
    int initState = ctlr->State();
    Log( ETrue, _L("\n"));
    Log( ETrue, _L8("Initial state:"), getState(initState));

    if (name == bavpMethodIdentifiers[ID_PLAY]) 
    {
        Log( ETrue, _L("Method Invoked: Play"));
        if ( ctlr->State() != EBavpPlaying )
        {
            ctlr->PlayL();
        }
        VOID_TO_NPVARIANT(*result);
        reportPlayResultsL(initState,ctlr);
    }
    else if (name == bavpMethodIdentifiers[ID_STOP]) 
    {
        Log( ETrue, _L("Method Invoked: Stop"));
        if ( ctlr->State() != EBavpStopped )
        {
            ctlr->Stop();
        }
        VOID_TO_NPVARIANT(*result);
        reportStopResultsL(ctlr);
    }
    else if (name == bavpMethodIdentifiers[ID_PAUSE]) 
    {
        Log( ETrue, _L("Method Invoked: Pause"));
        if ( ctlr->State() != EBavpPaused )
        {
            ctlr->PauseL();
        }
        VOID_TO_NPVARIANT(*result);
        reportPauseResultsL(ctlr);
    }  
    else if (name == bavpMethodIdentifiers[ID_FASTFORWARD]) 
    {
        Log( ETrue, _L("Method Invoked: FastForward"));
        if ( ctlr->State() == EBavpRewinding )
        {
            ctlr->PauseL();
        }

        if ( ctlr->State() != EBavpFastForwarding )
        {
            ctlr->FastForwardL();
        }
        VOID_TO_NPVARIANT(*result);
        reportFFResults(ctlr);
    }  
    else if (name == bavpMethodIdentifiers[ID_REWIND]) 
    {
        Log( ETrue, _L("Method Invoked: Rewind"));
        if ( ctlr->State() == EBavpFastForwarding )
        {
            ctlr->PauseL();
        }

        if ( ctlr->State() != EBavpRewinding )
        {
            ctlr->RewindL();
        }
        VOID_TO_NPVARIANT(*result);
        reportRewindResults(ctlr);
    }
    else
    {
        Log( ETrue, _L("Unknown method invoked"));
        return false;
    }  
       
    return true;
}
    
bool BavpHasProperty(BavpNPObject */*obj*/, NPIdentifier name)
{
    int i;
    for (i = 0; i < NUM_PROPERTY_IDENTIFIERS; i++) 
    {
        if (name == bavpPropertyIdentifiers[i]){
            return true;
        }
    }
    return false;
}    
    
bool BavpGetPropertyL (BavpNPObject *obj, NPIdentifier name, NPVariant *variant)
{
    if (!obj->plugin || !(obj->plugin->Controller())){
        NULL_TO_NPVARIANT(*variant);
        return false;    
    }
    
    
    if (name == bavpPropertyIdentifiers[ID_VOLUME]) 
    {
        INT32_TO_NPVARIANT(obj->plugin->Controller()->GetPlayerVolume(), *variant);
    } 

    else if (name == bavpPropertyIdentifiers[ID_POSITION]) 
    {
        TTimeIntervalMicroSeconds ms = obj->plugin->Controller()->getPositionL();
        int time = (int)ms.Int64();
        INT32_TO_NPVARIANT(time, *variant);
    	
    } 

    else if (name == bavpPropertyIdentifiers[ID_DURATION]) 
    {
        TTimeIntervalMicroSeconds ms = obj->plugin->Controller()->Duration();
        int time = (int)ms.Int64();
        INT32_TO_NPVARIANT(time, *variant);
    } 

    else if (name == bavpPropertyIdentifiers[ID_STATE_CHANGED_CALLBACK]) 
    {
        if (obj->stateChangedCallback)
            OBJECT_TO_NPVARIANT(obj->stateChangedCallback, *variant);  
        else
            NULL_TO_NPVARIANT(*variant);
    }
    else if (name == bavpPropertyIdentifiers[ID_STATE])
    {
        TPtrC8 state = getState((TInt)(obj->plugin->Controller()->State()));
        BavpDesToNpvariant(state, variant);
    }
    else if (name == bavpPropertyIdentifiers[ID_ISVIDEOCLIP])
    {
        BOOLEAN_TO_NPVARIANT(obj->plugin->Controller()->IsClipVideo(), *variant);
    }
    else if (name == bavpPropertyIdentifiers[ID_ISSEEKABLE])
    {
        BOOLEAN_TO_NPVARIANT(obj->plugin->Controller()->IsClipSeekable(), *variant);
    }
    else if (name == bavpPropertyIdentifiers[ID_CLIPNAME])
    {
        const HBufC& name = obj->plugin->Controller()->ClipName();
        
        TInt lastSlashPos = name.LocateReverse( '/' );
        //retrieve the clip name
        if( lastSlashPos == KErrNotFound ) {
            lastSlashPos = 0;
        }

        TInt len = name.Length()-lastSlashPos-1;
        TPtrC namePtr( name.Right(len));
        BavpDesToNpvariant(namePtr, variant);
    }
    else if (name == bavpPropertyIdentifiers[ID_FULLSCREENMODE])
    {
        BOOLEAN_TO_NPVARIANT(obj->plugin->Controller()->IsClipFullScreen(), *variant);
    }
    else if (name == bavpPropertyIdentifiers[ID_MIMETYPE])
    {
        const HBufC8& name = obj->plugin->Controller()->MimeType();
        TPtrC8 namePtr(name);
        BavpDesToNpvariant(namePtr, variant);
    }
    else
    {
        return false;
    }
    
    return true;
   
}


bool BavpSetPropertyL (BavpNPObject *obj, NPIdentifier name, NPVariant *variant)
{
    if (!obj->plugin)
        return false;

    CBavpController *ctlr = obj->plugin->Controller();
    
    if (!ctlr)
        return false;
    
    Log(ETrue,_L("\n"));

    if (name == bavpPropertyIdentifiers[ID_VOLUME]) 
    {
        int vol = 0;

        if (NPVARIANT_IS_DOUBLE(*variant))
            vol = (int)NPVARIANT_TO_DOUBLE(*variant);
        else if (NPVARIANT_IS_INT32(*variant))
            vol = NPVARIANT_TO_INT32(*variant); 
        else
        {
            Log(ETrue,_L("SetProperty: Not a valid value for volume"));
            return false;
        }
            
        ctlr->SetPlayerVolume(vol);
        reportVolumeResults(vol,ctlr);
    } 

    else if (name == bavpPropertyIdentifiers[ID_POSITION]) 
    {
        int pos = 0;

        if (NPVARIANT_IS_DOUBLE(*variant))
            pos = (int)NPVARIANT_TO_DOUBLE(*variant);
        else if (NPVARIANT_IS_INT32(*variant))
            pos = NPVARIANT_TO_INT32(*variant); 
        else
        {
            Log(ETrue,_L("SetProperty: Not a valid value for position"));
            return false;
        }
            
        TInt64 ms(pos);
        CBavpController *ctlr = obj->plugin->Controller();
        int state = ctlr->State();

        if ( state != EBavpRewinding && state != EBavpFastForwarding )
        {
            ctlr->setPositionL(TTimeIntervalMicroSeconds(ms)); 
        }
        else
        {
            ctlr->Stop();
            ctlr->setPositionL(TTimeIntervalMicroSeconds(ms));
        }

        reportPositionResultsL(pos,ctlr);
    } 

    else if (name == bavpPropertyIdentifiers[ID_STATE_CHANGED_CALLBACK]) 
    {
        //int count = 0;
        if (NPVARIANT_IS_NULL(*variant)) 
        {
            if (obj->stateChangedCallback) 
            {
                NPN_ReleaseObject((struct NPObject*)obj->stateChangedCallback);  
                obj->stateChangedCallback = 0;
            }            
        } 
        else if (NPVARIANT_IS_OBJECT(*variant)) 
        {
            NPObject* callback = NPVARIANT_TO_OBJECT(*variant);            
            if (obj->stateChangedCallback) 
            {
                NPN_ReleaseObject((struct NPObject*)obj->stateChangedCallback);  
                obj->stateChangedCallback = 0;
            }
            NPN_RetainObject((struct NPObject*)callback);            
            obj->stateChangedCallback = callback;
        }
        return true;  
    }

    else if (name == bavpPropertyIdentifiers[ID_FULLSCREENMODE]) 
    {
        int   pos = 0;
        TBool newmode, needtochange;
        CBavpController *ctlr = obj->plugin->Controller();

        if ( !(ctlr->IsClipVideo()) )
        {
            Log(ETrue,_L("Fullscreenmode not applicable for audio"));
            return false;
        }
        	
        if (NPVARIANT_IS_INT32(*variant))
        {
            pos = NPVARIANT_TO_INT32(*variant); 
        }
        else if (NPVARIANT_IS_DOUBLE(*variant))
        {
            pos = (int)NPVARIANT_TO_DOUBLE(*variant);
        }
        else if (NPVARIANT_IS_BOOLEAN(*variant))
        {
            pos = NPVARIANT_TO_BOOLEAN(*variant) ? 1 : 0;
        }
        
        else
        {
            Log(ETrue,_L("SetProperty: Not a valid value for fullscreenmode"));
            return false;
        }

        Log(ETrue,_L("SetProperty: Fullscreenmode"));
            
        newmode = (pos == 1);
        needtochange = (ctlr->IsClipFullScreen() != newmode);

        if ( ctlr->IsClipFullScreen() )
        {
            Log(ETrue,_L("Current mode: Fullscreen"));
        }
        else
        {
            Log(ETrue,_L("Current mode: Normalscreen"));
        }

        if ( newmode )
        {
            Log(ETrue,_L("Requested mode: Fullscreen"));
        }
        else
        {
            Log(ETrue,_L("Requested mode: Normalscreen"));
        }

         
        if ( !needtochange )
        {
            Log(ETrue,_L("No need for mode change"));
            return false;
        }

        ((CBavpControllerVideo *)ctlr)->HandleCommandL(EBavpCmdPlayFullScreen);   
    }
    else
    {
        return false;
    } 
    
    return true; 
}

      
static NPClass _BavpNPClass = { 
    0,
    (NPAllocateFunctionPtr) BavpAllocate, 
    (NPDeallocateFunctionPtr) BavpDeallocate, 
    (NPInvalidateFunctionPtr) BavpInvalidate,
    (NPHasMethodFunctionPtr) BavpHasMethod,
    (NPInvokeFunctionPtr) BavpInvokeL,
    (NPInvokeDefaultFunctionPtr) 0,
    (NPHasPropertyFunctionPtr) BavpHasProperty,
    (NPGetPropertyFunctionPtr) BavpGetPropertyL,
    (NPSetPropertyFunctionPtr) BavpSetPropertyL,
    (NPRemovePropertyFunctionPtr) 0
};
static NPClass *BavpNPClass = &_BavpNPClass;


BavpNPObject* BavpNPObject_new(NPP instance)
{
    initializeIdentifiers();
    BavpNPObject *self = (BavpNPObject *)NPN_CreateObject (instance, BavpNPClass);  
    return self;
}

void BavpNPObject_stateChanged(BavpNPObject* obj, char* state)
{
    if (!obj || !obj->stateChangedCallback)
        return;
    NPVariant arg;
    NPVariant res;    
    STRINGZ_TO_NPVARIANT(state, arg);
    NPN_InvokeDefault(obj->nppInstance, obj->stateChangedCallback, &arg, 1, &res);
}
