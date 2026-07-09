/**
 * NOTE:
 * THESE ARE ONLY HERE TO GET CERTAIN VERSIONS OF
 * STM32CUBEIDE TO SHUT UP. THEY DONT DO ANYTHING AND
 * ARE NOT SUPPOSED TO.
 */

/**********     INCLUDES        **********/

/**********     TYPEDEFS         **********/

/**********		DEFINES		**********/

/**********		EXTERNAL VARIABLE DEFINITIONS		**********/

/**********		STATIC VARIABLES		**********/

/**********		STATIC FUNCTION DECLRATIONS		**********/

/**********		STATIC FUNCTION DEFINITIONS		**********/

/**********		GLOBAL FUNCTION DEFINITIONS		**********/
__attribute__((weak)) int _lseek(void) {
    return 0;
}

__attribute__((weak)) int _close(void) {
    return 0;
}

__attribute__((weak)) int _fstat(void) {
    return 0;
}

__attribute__((weak)) int _getpid(void) {
    return 0;
}

__attribute__((weak)) int _isatty(void) {
    return 0;
}

__attribute__((weak)) int _kill(void) {
    return 0;
}

__attribute__((weak)) int _read(void) {
    return 0;
}

__attribute__((weak)) int _write(void) {
    return 0;
}
