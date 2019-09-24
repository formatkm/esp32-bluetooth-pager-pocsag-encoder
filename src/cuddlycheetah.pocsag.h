#include <Arduino.h>

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#define SYNC 0x7CD215D8
#define IDLE 0x7A89C197
#define FRAME_SIZE 2
#define BATCH_SIZE 16
#define PREAMBLE_LENGTH 576

#define FLAG_ADDRESS 0x000000
#define FLAG_MESSAGE 0x100000

#define FLAG_TEXT_DATA 0x3
#define FLAG_NUMERIC_DATA 0x0

#define TEXT_BITS_PER_WORD 20
#define TEXT_BITS_PER_CHAR 7

#define NUMERIC_BITS_PER_WORD 20
#define NUMERIC_BITS_PER_DIGIT 4

#define CRC_BITS 10
#define CRC_GENERATOR 0b11101101001

uint32_t crc(uint32_t inputMsg);
uint32_t parity(uint32_t x);
uint32_t encodeCodeword(uint32_t msg);

uint32_t encodeASCII(uint32_t initial_offset, char* str, uint32_t strLen, uint32_t* out);
// Char Translationtable
char encodeDigit(char ch);
uint32_t encodeNumeric(uint32_t initial_offset, char* str, uint32_t strLen, uint32_t* out);
int addressOffset(int address);
void encodeTransmission(bool numeric, int address, int fb, char* message, uint32_t strLen, uint32_t* out);
size_t textMessageLength(int address, int numChars);
size_t numericMessageLength(int address, int numChars);