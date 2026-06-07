/* Public domain
 *
 * Just some useful macros collection.
 *
 * Usage:
 *     size_t LENGTH(ARR): Returns length of 'ARR'
 */
#ifndef UTILSH_MACROS_H
#define UTILSH_MACROS_H

#define LENGTH(ARR) (sizeof(ARR) / sizeof((ARR)[0]))

#endif
