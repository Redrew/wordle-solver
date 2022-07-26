#pragma once

#include "types.h"
#include <string>
#include <fstream>
#include <iostream>


namespace io {
const types::words_t readFile(const std::string &filename) {
  types::words_t words;
  std::string line;
  std::ifstream file(filename);
  if (file.is_open()) {
    while (getline(file, line)) {
      words.push_back(line);
    }
    file.close();
  }
  return words;
}
void printRank(const types::words_t &guesses, types::ranked_guesses_t rankedGuesses) {
  std::sort(rankedGuesses.rbegin(), rankedGuesses.rend());
  for (const types::ranked_guess_t &guess : rankedGuesses) {
    std::cout << "Guess: " << guesses[guess.second] << ", Value: " << guess.first
         << "\n";
  }
}
}
