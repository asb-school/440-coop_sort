#ifndef _conf_h_856488445
#define _conf_h_856488445
/**
 * - Institution: Andrews University
 * - Instructor: Dr. Villafane
 *
 * - Item: Cross platform configuration definitions
 */

#if defined(_WIN64)
#define OS_WIN
#elif defined(__APPLE__)
#define OS_OSX
#elif defined(__linux__)
#define OS_LIN
#else
#error "Unknown operating system"
#endif

#endif // ifdef _conf_h_856488445