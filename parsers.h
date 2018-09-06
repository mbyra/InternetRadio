//
// Created by marcin on 06.09.18.
//

#ifndef INTERNETRADIOSIK_PARSERS_H
#define INTERNETRADIOSIK_PARSERS_H


// Prints diagnostic data to stdout.
void diagnoseSender();

// Checks if string contains unsigned integer. If so, returns it.
// Otherwise, prints error message and exits with code 1.
unsigned parse_unsigned(const char* s);

// Checks if string contains unsigned 16 bit integer. If so, returns it.
// Otherwise, prints error message and exits with code 1.
unsigned parse_unsigned_16_bit(const char* s);

// Parses all arguments from cli and saves them into global variables. Some
// variables have default value.
void parseSender(int argc, char **argv);


// Prints diagnostic data to stdout.
void diagnoseReceiver();

// Parses all arguments from cli and saves them into global variables. Some
// variables have default value.
void parseReceiver(int argc, char **argv);


#endif //INTERNETRADIOSIK_PARSERS_H
