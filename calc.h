#include "arduino.h"
#define SRATE    (50000L)
#define WTSIZE   (512L)
#define FRACBITS (6L)
#define HALF     (0x20)
#define DACRANGE (1024L)
extern const uint16_t octaveLookup[DACRANGE];
extern const uint8_t  sine[WTSIZE];
extern const uint8_t  ramp[WTSIZE];
extern const uint8_t  sq[WTSIZE];
extern const uint8_t  triangle[WTSIZE];

