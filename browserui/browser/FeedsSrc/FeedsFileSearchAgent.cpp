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

#include "FeedsFileSearchAgent.h"
#include <fbs.h>

// EXTERNAL DATA STRUCTURES

// EXTERNAL FUNCTION PROTOTYPES

// CONSTANTS

// MACROS

// LOCAL CONSTANTS AND MACROS

// MODULE DATA STRUCTURES

// LOCAL FUNCTION PROTOTYPES

// FORWARD DECLARATIONS

// ============================= LOCAL FUNCTIONS ===============================


// ============================ MEMBER FUNCTIONS ===============================

// -----------------------------------------------------------------------------
// CFeedsFileSearchAgent::CFileSearchAgent
// C++ default constructor can NOT contain any code, that
// might leave.
// -----------------------------------------------------------------------------
//
CFeedsFileSearchAgent::CFeedsFileSearchAgent(MFeedsFileSearchAgentCallback& aCallback)
    : iCallback(&aCallback), iCancelSearch(EFalse)
    {
    }

// -----------------------------------------------------------------------------
// CFeedsFileSearchAgent::ConstructL
// Symbian 2nd phase constructor can leave.
// -----------------------------------------------------------------------------
//
void CFeedsFileSearchAgent::ConstructL()
    {
    TDriveList driveList;
	TChar driveLetter;
    
    // we'll keep two lists. one for the drives in the system, the 
	// other for the list of found files
    iDriveEntryList = new(ELeave) CDriveEntryList(KMaxDrives);
    iFileEntryList = new(ELeave) CFileEntryList(FEEDS_FILE_SEARCH_AGENT_MAX_RESULTS);
   	// Create a directory scan object.
    iScan = CDirScan::NewL(iFs);
    iLazyCaller = CIdle::NewL(CActive::EPriorityIdle );
			
	// Connect to fs for FS ops, etc
	User::LeaveIfError(iFs.Connect());
    	
	// populate list of drives
	GetDriveListL();
    }

// -----------------------------------------------------------------------------
// CFeedsFileSearchAgent::NewL
// Two-phased constructor.
// -----------------------------------------------------------------------------
//
CFeedsFileSearchAgent* CFeedsFileSearchAgent::NewL(MFeedsFileSearchAgentCallback& aCallback)
    {
	CFeedsFileSearchAgent* self = new( ELeave ) CFeedsFileSearchAgent(aCallback);

    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop();

    return self;
    }

// -----------------------------------------------------------------------------
// CFeedsFileSearchAgent::~CFeedsFileSearchAgent()
// Destructor
// -----------------------------------------------------------------------------
//
CFeedsFileSearchAgent::~CFeedsFileSearchAgent()
    {
    CancelSearch();
   	iFs.Close();
    delete iScan;
   	delete iLazyCaller;
	delete iDriveEntryList;
	delete iFileEntryList;
    }


// -----------------------------------------------------------------------------
// CFeedsFileSearchAgent::StartSearchingL
//
//
// -----------------------------------------------------------------------------
//
void CFeedsFileSearchAgent::StartSearchingL()
    {
	iSearchDriveIndex = -1;
	
   	// Return control to the CIdle
    // The searching work will be done in LazyCallBack.
    iLazyCaller->Start(TCallBack(CFeedsFileSearchAgent::LazyCallBack,this));

    }

// -----------------------------------------------------------------------------
// CFeedsFileSearchAgent::GetFileEntry
//
//
// -----------------------------------------------------------------------------
//
TFileEntry* CFeedsFileSearchAgent::GetFileEntry(const TInt aIndex)
	{
	// for a client, return a pointer to a file that was found given
	// it's index within the list of found files
	if(aIndex >= iFileEntryList->Count())
		{
		return NULL;
		}
	else
		{
		return &(iFileEntryList->At(aIndex));
		}
	}

// -----------------------------------------------------------------------------
// CFeedsFileSearchAgent::GetDriveListL
//
//
// -----------------------------------------------------------------------------
//
void CFeedsFileSearchAgent::GetDriveListL()
	{
	TDriveList driveList;
   	
   	// Setup List of drives
   	// DriveList will return an array of 26 bytes.
   	// nonzero entries means there is media present in 
   	// that drive letter. 
    User::LeaveIfError(iFs.DriveList(driveList));
    for (TInt i=0; i<KMaxDrives; i++)
		{
    	if (driveList[i])
        	{
        	TDriveEntry driveEntry;
        	
        	// Assign a drive letter to the drive entry.
        	// In the returned list of drives, index 0 corresponds to A:\,
        	// 1 to B:\, etc.
            driveEntry.iLetter = 'A' + i;
            
            // map the letter back to the drive number. This should be the 
            // same as "i"
            iFs.CharToDrive(driveEntry.iLetter, driveEntry.iNumber);
            
 			// Add to the list of drives that we keep
 			iDriveEntryList->AppendL(driveEntry);
            }
        }
	}
	
// -----------------------------------------------------------------------------
// CFeedsFileSearchAgent::DoSearchFilesL
//
// Searches a given path or paths for a filename and add it to this class' list
// of found files. The filename can contain wildcards.
// -----------------------------------------------------------------------------
//	
TInt CFeedsFileSearchAgent::DoSearchFiles(const TDesC& aFileName, const TDesC& aPath)
	{
	TFindFile fileFinder(iFs);
	CDir* dir = NULL;
	
	// given a semicolon-separated lists of paths to search in,
	// look for the filename (that can include wildcards) found in that path or paths
	// This first search will allocate memory for dir and will contain the list of
	// files found within the first path given
	TInt err = fileFinder.FindWildByPath(aFileName, &aPath, dir);
	
	while(err == KErrNone)
		{
		// For the current path given,
		// loop through list of found items within that path 
		for (TInt i=0; i<dir->Count(); i++)
			{
			TEntry entry = (*dir)[i];

			// create and setup TParse object, useful for later extracting parts
			// of the found files used by clients of this class
			TParse parsedName;
			parsedName.Set(entry.iName, &fileFinder.File(), NULL);
			
			// as a sanity check,
			// make sure the drive, path are valid and
			// that the drive found matches the path given
			if(parsedName.Drive().Length() && aPath.Length() && parsedName.Drive()[0] == aPath[0])
				{
				// the wildcard search may have found directories that match
				// exlude these from our list because we use this function to 
				// record files only 
				if(!entry.IsDir())
					{
					// Create a fileEntry based upon the found file
					// FullName will be the complete path specifier for the file
					// while entry will be just the name within the directory					
					TFileEntry fileEntry;
					fileEntry.iPath = parsedName.FullName();
					fileEntry.iEntry = entry;
					
					// copy this entry into to our list of found files
					TRAP(err, iFileEntryList->AppendL(fileEntry));
					
					// If we have exceeded our maximum number of possible entries,
					// or we're cancelling an ongoing search
					// then stop adding to the list
					if(iFileEntryList->Count() >= FEEDS_FILE_SEARCH_AGENT_MAX_RESULTS ||
						iCancelSearch)	
						{
						// cleanup dir
						if(dir != NULL)
							{
							delete(dir);
							dir = NULL;
							}
						return err;
						}
					}
				}
			}
			
			//
			// We have completed looking at the results for this dir. Look
			// At the results where we left off, for the next path given. 
			// If there is no next path, KErrNotFound will be returned, we'll 
			// complete the search cycle.
			//
			if(dir != NULL)
				{				
				delete(dir);
				dir = NULL;
				}
			err = fileFinder.FindWild(dir);
		}

		// Cleanup the final instance of dir that has been allocated
		if(dir != NULL)
			{			
			delete(dir);
			dir = NULL;
			}
		return err;
	}
	
// -----------------------------------------------------------------------------
// CFeedsFileSearchAgent::DoSearchFilesRecursive
//
// Starting from a given path, search the filesystem recursively for a given
// filename. The filename may contain wildcards.
// -----------------------------------------------------------------------------
//	
	
TInt CFeedsFileSearchAgent::DoSearchFilesRecursive(const TDesC& aFileName)
    {
    TInt err(KErrNone);
    CDir* dir = NULL;

    // Find the entries from the next directory in the hierarchy
    // or NULL/error if there are none
    TRAP(err, iScan->NextL(dir));

	// done, break out of loop
    if(!dir || (err != KErrNone) )
        {
        return KErrGeneral;
        }

	// loop through the number of found directories
	for(TInt i=0; i < dir->Count(); i++)
        {
        TEntry entry = (*dir)[i];

        // The search will find directories as well as files. We're
        // only interested here in directories.
        if(entry.IsDir())
            {
            // From the scan, get the full path including the drive letter
            // being scanned.
            TFileName path(iScan->FullPath());

            if(path.Length())
                {
                // If valid, append a slash to the end of the directory, 
                // and then search for the desired filename given this path
                path.Append(entry.iName);
                path.Append(_L("\\"));
                DoSearchFiles(aFileName,path);
                }
            }
        }

    // Delete the dir that was allocated in this iteration
    if(dir != NULL)
        {
        delete(dir);
        dir = NULL;
        }

    return err;
    }

// -----------------------------------------------------------------------------
// CFeedsFileSearchAgent::LazyCallBack
//
// This function calls StartSearchFile instead of calling it from StartSearchingL function
// To ensure it doesn't called if user has cancelled the request
// -----------------------------------------------------------------------------
//
TBool CFeedsFileSearchAgent::LazyCallBack(TAny* aPtr)
    {
    CFeedsFileSearchAgent* ptr = (CFeedsFileSearchAgent*)aPtr;
    // if we've cancelled the search
    // then stop adding to the list
    if(!ptr->iCancelSearch)
        {
        return ptr->StartSearchFile();    
        }
    else
        {
        return EFalse;
        }    
    }

// -----------------------------------------------------------------------------
// CFeedsFileSearchAgent::StartSearchFile
//
// starts the search of *.OPML Files.
// -----------------------------------------------------------------------------
//
TBool CFeedsFileSearchAgent::StartSearchFile()
    {
    // Quit looping and don't try to scan if we've already exceeded the number of 
    // possible file entries
    // or if we've cancelled the search
    if(iFileEntryList->Count() >= FEEDS_FILE_SEARCH_AGENT_MAX_RESULTS ||
            iCancelSearch)
        {
        TRAP_IGNORE(iCallback->FeedsFileSearchCompleteL( iFileEntryList->Count() ));
        return EFalse;	
        }
    TInt retVal = KErrGeneral;
    if(iSearchDriveIndex != -1)
        {
        retVal = DoSearchFilesRecursive( _L("*.opml"));
        }
    if(retVal != KErrNone)
        {
        iSearchDriveIndex++;
        if(iSearchDriveIndex < iDriveEntryList->Count())
            {
            TDriveEntry driveEntry = iDriveEntryList->At(iSearchDriveIndex);
            TBuf<5> driveRoot;

            driveRoot.Append(driveEntry.iLetter);
            driveRoot.Append(_L(":\\"));    	

            // Search the base of the drive and also search recursively
            // through it's folder hierarchy.
            DoSearchFiles(_L("*.opml"),driveRoot);
            TRAP_IGNORE(iScan->SetScanDataL(driveRoot, KEntryAttDir|KEntryAttMatchMask, ESortByName | EAscending | EDirsFirst));
            return ETrue;
            }
        else
            {
            TRAP_IGNORE(iCallback->FeedsFileSearchCompleteL( iFileEntryList->Count() ));
            return EFalse;
            }
        }
    else
        {
        return ETrue;
        }
    }

// -----------------------------------------------------------------------------
// CFeedsFileSearchAgent::CancelSearch.
//
// Cancel the ongoing search.
// -----------------------------------------------------------------------------
//	    
void CFeedsFileSearchAgent::CancelSearch()
    {
    if(iLazyCaller->IsActive())
        {
        iLazyCaller->Cancel();
        }
    iCancelSearch = ETrue;   
    }

// ========================== OTHER EXPORTED FUNCTIONS =========================


//  End of File
