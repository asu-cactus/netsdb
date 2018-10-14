
/*
 * LogLevel.h
 *
 *  Created on: Jul 1, 2016
 *      Author: kia
 */


// Following the same concept of logging as log4j

// OFF	The highest possible rank and is intended to turn off logging.
// FATAL	Severe errors that cause premature termination.
// ERROR	Other runtime errors or unexpected conditions.
// WARN	    Use of deprecated APIs, poor use of API, 'almost' errors.
// INFO	    Interesting runtime events (startup/shutdown).
// DEBUG	Detailed information on the flow through the system.
// TRACE	Most detailed information.

#ifndef SRC_UTILITIES_HEADERS_LOGLEVEL_H_
#define SRC_UTILITIES_HEADERS_LOGLEVEL_H_

enum LogLevel { OFF, FATAL, ERROR, WARN, INFO, DEBUG, TRACE };

#endif /* SRC_UTILITIES_HEADERS_LOGLEVEL_H_ */
