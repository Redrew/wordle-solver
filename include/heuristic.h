#pragma once

#include "cache.h"
#include "types.h"

namespace heuristic {
types::ev_t estimateGuessValue(const cache::OutputCache &outputCache,
                          const types::answers_t &answers, const types::guess_t &guess) {
  std::unordered_map<types::answer_t, types::guess_t> coloursCount;
  types::ev_t dn = 1 / (types::ev_t)answers.size();
  types::ev_t entropy = 0;
  for (const ll &answer : answers) {
    coloursCount[outputCache.find(answer, guess).value]++;
  }
  for (const auto &item : coloursCount) {
    ll c = item.second;
    if (item.first == FINISHED) {
      entropy -= c * dn * log(c * dn);
    } else {
      entropy -= c * dn * log((c + 1) * dn);
    }
  }
  return entropy;
}

ranked_guesses_t rankGuesses(const OutputCache &outputCache,
                             const types::answers_t &answers,
                             const types::guesses_t &guesses, const ll &k) {
  ranked_guesses_t values;

  values.reserve(answers.size());
  for (const ll &guess : guesses) {
    values.push_back(ranked_guess_t(
        estimateGuessValue(outputCache, answers, guess), guess));
  }
  nth_element(values.begin(), values.end() - k, values.end());
  ranked_guesses_t topk(values.end() - k, values.end());
  return topk;
}
}