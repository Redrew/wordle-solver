#pragma once

#include "types.h"
#include "colour.h"
#include <unordered_map>

namespace node {
const types::ev_t NO_EV = -1;
const types::guess_t NO_GUESS = -1;

struct Node {
  int size;
  types::guess_t guess;
  types::ev_t ev = NO_EV;
  std::unordered_map<types::guess_t, Node> children;
  Node(types::guess_t guess = NO_GUESS, int size = 0) : guess(guess), size(size) {}
  bool hasEV() { return ev != NO_EV; }
  void updateEV() {
    ev = 1;
    for (auto &item : children) {
      if (item.first == colour::MAX_VALUE) {
        continue;
      } else if (!item.second.hasEV()) {
        item.second.updateEV();
      }
      double p = item.second.size / (double)size;
      ev += p * item.second.ev;
    }
  }
};
}
