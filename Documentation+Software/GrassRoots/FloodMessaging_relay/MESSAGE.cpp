
// Might be a better way to do this but had to
// integrate two disparate collections of code.

// Adds Arduino's capabilities.
// https://stackoverflow.com/questions/10612385/strings-in-c-class-file-for-arduino-not-compiling
// Needed for uint8_t type specifier.
#include <Arduino.h>

// Message contents
uint8_t MESSAGE[256]; // max length of LoRa messages, cell indexes = 0..255
