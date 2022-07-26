#pragma once

#include <string>
#include <vector>

namespace types {
using word_t = std::string;
using words_t = std::vector<word_t>;
using answer_t = int;
using guess_t = int;
using answers_t = std::vector<answer_t>;
using guesses_t = std::vector<guess_t>;
using ev_t = double;

typedef std::pair<double, guess_t> ranked_guess_t;
typedef std::vector<ranked_guess_t> ranked_guesses_t;
}