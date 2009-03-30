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
* Description:  File search utility active agent for use within browser ui
*
*/



#ifndef CFEEDSFILESEARCHAGENT_H
#define CFEEDSFILESEARCHAGENT_H

//  INCLUDES

#include <e32base.h>
#include <f32file.h>

// CONSTANTS

// MACROS

// FUNCTION PROTOTYPES
#define FEEDS_FILE_SEARCH_AGENT_MAX_RESULTS 64

// FORWARD DECLARATIONS

// CLASS DECLARATION

// DATA TYPES

// CLASS DECLARATION

/**
* Encapsulate drive entry info
* @since 3.2
*/
class TDriveEntry
	{
public:
    TChar           iLetter;
    TInt            iNumber;
	};

/**
* Encapsulate File entry info
* @since 3.2
*/
class TFileEntry
	{
public:
    TFileName       iPath;
    TEntry          iEntry;
	};

typedef CArrayFixSeg<TDriveEntry> CDriveEntryList;
typedef CArrayFixSeg<TFileEntry> CFileEntryList;

/**
* A Callback mixin for search Agent
* @since 3.2
*/

class MFeedsFileSearchAgentCallback
	{
	public:
		virtual void FeedsFileSearchCompleteL(TInt aCount) = 0;
	};

// CLASS DECLARATION

/**
* An Active Agent used to search the filesystem for feeds files
* @since 3.2
*/

class CFeedsFileSearchAgent : public CBase
	{
	public: // Constructor & destructor

        /**
        * Two-phased constructor.
        */
        static CFeedsFileSearchAgent* NewL(MFeedsFileSearchAgentCallback& aCallback);
        
        /**
        * Destructor.
        */
        virtual ~CFeedsFileSearchAgent();
	
	public: // New Functions
		
		/**
		* Once the search agent has been setup via its constructor, begin
		* the search process
		* @since 3.2
		* @return void
		*/	
		void StartSearchingL();
		
		/**
		* After the search has completed, allows client to retrieve a 
		* File entry pointer corresponding to the passed-in index
		* @since 3.2
		* @return File entry pointer or NULL if invalid index
		*/
		TFileEntry* GetFileEntry(const TInt aIndex);
		
	private: // Search Functions
	
		/**
		* Populate the list of available drives on the phone within this class
		* @since 3.2
		*/
		void GetDriveListL();
		
		/**
		* Given a directory, search for a wildcard-included filename within
		* that directory
		* @since 3.2
		* @param a filename, a path
		*/ 
		TInt DoSearchFiles(const TDesC& aFileName, const TDesC& aPath);
		
		/**
		* Given a directory, search recursively for a wildcard-included filename
		* within the hierarchy below that directory
		* @since 3.2
		* @param a filename, a path
		*/
		TInt DoSearchFilesRecursive(const TDesC& aFileName);
	
	private:
		
        /**
        * C++ constructor.
        */
        CFeedsFileSearchAgent(MFeedsFileSearchAgentCallback& aCallback);

        /**
        * By default Symbian 2nd phase constructor is private.
        */
        void ConstructL();        

    public:
        /**
         * This method is a call back;
         *
         * @since 7.1
         * @return The TBool.
         */
        static TBool LazyCallBack(TAny* aPtr);

         /**
         * This method Cancels the ongoing search;
         *
         * @since 7.1
         * @return Void.
         */
        void CancelSearch();

        /**
         * This method Starts the search;
         *
         * @since 7.1
         * @return TBool.
         */
        TBool StartSearchFile();

    private:    // Data

		RFs									iFs;
		
        MFeedsFileSearchAgentCallback*		iCallback;           // not owned (callee owns)
        CDriveEntryList* 		          	iDriveEntryList;     // owned

        TInt								iSearchDriveIndex;

        CFileEntryList*						iFileEntryList;		 // owned
        TBool								iCancelSearch;
        CIdle*                              iLazyCaller;
        CDirScan*                           iScan;
	};
	
#endif      // CFEEDSFILESEARCHAGENT_H

// End of File
