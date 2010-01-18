/*
* This file enables or disables the logger 
* functionality in Browser Application.
* File included into mmp files.
*/

// define this macro to enable browser log
// #define __BROWSER_LOG_ENABLE

// define this macro to enable browser performance log
//#define __BROWSER_PERF_LOG_ENABLE


// debug mode always activates log
#if defined( __BROWSER_LOG_ENABLE ) || defined( _DEBUG )
	MACRO I__BROWSER_LOG_ENABLED
	#define I__BROWSER_LOG_ENABLED
#endif

#if defined( __BROWSER_PERF_LOG_ENABLE )
	MACRO I__BROWSER_PERF_LOG_ENABLED
	#define I__BROWSER_PERF_LOG_ENABLED
#endif
