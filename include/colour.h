#pragma once

#include "types.h"
#include <string>
#include <math.h>

namespace colour{
const int WORD_LENGTH = 5;
const int BASE = 3;
const int GREEN_VALUE = 2;
const int YELLOW_VALUE = 1;
const int GREY_VALUE = 0;
const int MAX_VALUE = pow(BASE, WORD_LENGTH) - 1;
const std::string COLOUR_CHARS = "-yg";

struct Colours {
  int value;
  Colours(int value) : value(value) {}
  types::word_t getWord() {
    types::word_t response(WORD_LENGTH, ' ');
    int n = value;
    for (int i = 0; i < WORD_LENGTH; i++) {
      int colourAtI = n % BASE;
      n = n / BASE;
      response[i] = COLOUR_CHARS[colourAtI];
    }
    return response;
  }
};
}