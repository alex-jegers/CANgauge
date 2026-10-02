
#ifndef _VERSION_H_
#define _VERSION_H_

#ifdef __cplusplus
extern "C" {
#endif

/**********     INCLUDES        **********/

/**********     TYPEDEFS         **********/

/**********     DEFINES      **********/
#ifdef RELEASE
	#define BUILD_TYPE_STR		"Release"
#elif DEBUG
	#define BUILD_TYPE_STR		"Debug"
#elif DEBUG_OFAST
	#define BUILD_TYPE_STR		"Dbg Ofast"
#endif

#define VERSION					"v0.1.16 10/01/26 08:56AM"
/**********     GLOBAL VARIABLE DECLRATIONS     **********/

/**********		GLOBAL FUNCTION DECLRATIONS		**********/
/**
 * name:
 *      desc:
 *      params:
 *      returns:
 */




#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif  //_VERSION_H_
