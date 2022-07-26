#pragma once

#include "colour.h"
#include "types.h"
#include "utils.h"
#include <math.h>
#include <vector>

namespace wordle {
const int NUM_LETTERS = 26;

struct Wordle {
  types::word_t answer;
  Wordle(const types::word_t &answer) : answer(answer) {}
  colour::Colours query(const types::word_t &guess) {
    std::vector<int> letterCount(NUM_LETTERS);
    int value = 0;
    for (int i = 0; i < colour::WORD_LENGTH; i++) {
      int answerI = utils::charToInt(answer[i]);
      if (answer[i] == guess[i]) {
        value += colour::GREEN_VALUE * pow(colour::BASE, i);
      } else {
        letterCount[answerI]++;
      }
    }
    for (int i = 0; i < colour::WORD_LENGTH; i++) {
      int guessI = utils::charToInt(guess[i]);
      if (answer[i] != guess[i] && letterCount[guessI] > 0) {
        letterCount[guessI]--;
        value += colour::YELLOW_VALUE * pow(colour::BASE, i);
      }
    }
    return colour::Colours(value);
  }
};
}
