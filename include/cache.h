#pragma once

#include "types.h"
#include "colour.h"
#include "wordle.h"
#include <vector>

struct OutputCache {
  std::vector<colour::Colours> outputCache;
  int nAnswers, nGuesses;
  OutputCache(const types::words_t &answers, const types::words_t &guesses) {
    outputCache.reserve(answers.size() * guesses.size());
    nAnswers = answers.size();
    nGuesses = guesses.size();
    for (types::answer_t answerI = 0; answerI < nAnswers; answerI++) {
      const types::word_t &answer = answers[answerI];
      for (types::guess_t guessI = 0; guessI < nGuesses; guessI++) {
        const types::word_t &guess = guesses[guessI];
        outputCache[answerI * nGuesses + guessI] =
            wordle::Wordle(answer).query(guess);
      }
    }
  }
  colour::Colours find(const types::answer_t &answerI, const types::guess_t &guessI) const {
    return outputCache[answerI * nGuesses + guessI];
  }
};