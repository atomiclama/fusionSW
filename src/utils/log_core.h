/*
 * log.h
 *
 *  Created on: 21 Dec 2015
 *      Author: gb7beda1
 */

#ifndef LIB_INT_LOG_H_LOG_CORE_H_
#define LIB_INT_LOG_H_LOG_CORE_H_

#ifdef __cplusplus
extern "C" {
#endif



typedef enum {
    LOG_ERROR, LOG_WARN, LOG_INFO, LOG_DEBUG, LOG_ALL
} LOG_LEVEL;

typedef void (*LOGGER)(const char * msg);


void log_init(LOG_LEVEL level);
void log_setLogLevel(LOG_LEVEL level);
void log_msg(LOG_LEVEL level, const char *msg, ...);

// register a function to perform logging to a particular device.
int log_registerLogger(LOGGER);

#ifdef __cplusplus
}
#endif
#endif /* LIB_INT_LOG_H_LOG_CORE_H_ */
