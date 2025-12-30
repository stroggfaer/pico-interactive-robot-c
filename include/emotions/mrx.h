#ifndef MRX_H
#define MRX_H

#include <cstdint>

// Matrix dimensions
const int MATRIX_ROWS = 12;
const int MATRIX_COLS = 12;

// Matrix type definition
using Matrix12x12 = const uint8_t[MATRIX_ROWS][MATRIX_COLS];

// Angry expressions
extern Matrix12x12 ANGRY_CLOSED_MOUTH;
extern Matrix12x12 ANGRY_CLOSED;
extern Matrix12x12 ANGRY_OPEN_MOUTH;

// Neutral expressions
extern Matrix12x12 NEUTRAL_NO_BLINK;
extern Matrix12x12 NEUTRAL_BLINK;
extern Matrix12x12 NEUTRAL_YAWN;
extern Matrix12x12 NEUTRAL_SLEEP;
extern Matrix12x12 NEUTRAL_HALF_BLINK;
extern Matrix12x12 NEUTRAL_CIRCLE;

// Smile expressions
extern Matrix12x12 SMILE;
extern Matrix12x12 SMILE_A;
extern Matrix12x12 SMILE_B;

// Love smile expressions
extern Matrix12x12 SMILE_LOVE;
extern Matrix12x12 SMILE_LOVE_A;
extern Matrix12x12 SMILE_LOVE_B;

// Embarrassed expression
extern Matrix12x12 EMBARRASSED;

// Surprise expression
extern Matrix12x12 SURPRISE;

// Sad expressions
extern Matrix12x12 SAD;
extern Matrix12x12 SAD_A;

// Happy expressions
extern Matrix12x12 HAPPY;
extern Matrix12x12 HAPPY_CIRCLE;

// Scary expressions
extern Matrix12x12 SCARY_A;
extern Matrix12x12 SCARY_B;
extern Matrix12x12 SCARY_C;
extern Matrix12x12 SCARY_D;

// Talking expressions
extern Matrix12x12 TALKING_A;
extern Matrix12x12 TALKING_B;

// Tricky talking expressions
extern Matrix12x12 TALKING_TRICKY_A;
extern Matrix12x12 TALKING_TRICKY_B;

// Tricky smile expressions
extern Matrix12x12 SMILE_TRICKY_A;
extern Matrix12x12 SMILE_TRICKY_B;

#endif // MRX_H