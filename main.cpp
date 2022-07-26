#include <algorithm>
#include <cassert>
#include <future>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <thread>
#include <utility>



struct Tree {
  const vector<string> &answers, &guesses;
  const vector<ll> &answersI, &guessesI;
  const OutputCache &outputCache;
  Node root;
  Tree(const OutputCache &outputCache, const vector<string> &answers,
       const vector<string> &guesses, const vector<ll> &answersI,
       const vector<ll> &guessesI)
      : answers(answers), guesses(guesses), outputCache(outputCache),
        answersI(answersI), guessesI(guessesI) {}
  void search() { root = findBestNode(answersI, guessesI); }
  Node findBestNode(const vector<ll> &answersI,
                    const vector<ll> &guessesI) const {
    if (answersI.size() > 80) {
      return findBestNodeAsync(
          answersI, guessesI,
          rankGuesses(outputCache, answersI, guessesI, 15));
    } else {
      return findBestNodeSync(
          answersI, guessesI,
          rankGuesses(outputCache, answersI, guessesI, 15));
    }
  }

  Node findBestNodeSync(const vector<ll> &answersI, const vector<ll> &guessesI,
                        const ranked_guesses_t &rankedGuesses) const {
    Node bestNode;
    if (answersI.size() <= 2) {
      return searchGuess(answersI, guessesI, answersI[0]);
    }
    for (const auto &rankedGuess : rankedGuesses) {
      ll guess = rankedGuess.second;
      Node newNode = searchGuess(answersI, guessesI, guess);
      if (!bestNode.hasEV() || bestNode.ev > newNode.ev) {
        bestNode = newNode;
      }
    }
    return bestNode;
  }

  Node findBestNodeAsync(const vector<ll> &answersI, const vector<ll> &guessesI,
                         const ranked_guesses_t &rankedGuesses) const {
    Node bestNode;
    vector<future<Node>> newNodes;
    if (answersI.size() <= 2) {
      return searchGuess(answersI, guessesI, answersI[0]);
    }
    for (const auto &rankedGuess : rankedGuesses) {
      ll guess = rankedGuess.second;
      newNodes.push_back(
          async(launch::async, [this, answersI, guessesI, guess]() -> Node {
            return this->searchGuess(answersI, guessesI, guess);
          }));
    }
    for (auto &nodeFuture : newNodes) {
      const Node newNode = nodeFuture.get();
      if (!bestNode.hasEV() || bestNode.ev > newNode.ev) {
        bestNode = newNode;
      }
    }
    return bestNode;
  }

  Node searchGuess(const vector<ll> &answersI, const vector<ll> &guessesI,
                   const ll &guess) const {
    Node node(guess, answersI.size());
    unordered_map<ll, vector<ll>> answersIWithColour;
    for (const ll &answer : answersI) {
      ll colour = outputCache.find(answer, guess).value;
      answersIWithColour[colour].push_back(answer);
    }
    for (auto const &item : answersIWithColour) {
      if (item.first == FINISHED) {
        continue;
      }
      node.children[item.first] = findBestNode(item.second, guessesI);
    }
    node.updateEV();
    return node;
  }
};

int main() {
  vector<string> answers = read_file("words/answers.txt");
  vector<string> otherValidGuesses = read_file("words/guesses.txt");
  shuffle(answers.begin(), answers.end(), default_random_engine(0));
  shuffle(otherValidGuesses.begin(), otherValidGuesses.end(),
          default_random_engine(1));
  vector<string> guesses(answers);
  guesses.insert(guesses.end(), otherValidGuesses.begin(),
                 otherValidGuesses.end());
  cout << "Length of all valid answers: " << answers.size() << "\n";
  cout << "Length of all valid guesses: " << guesses.size() << "\n";
  OutputCache outputCache(answers, guesses);
  vector<ll> answersI(answers.size()), guessesI(guesses.size());
  iota(answersI.begin(), answersI.end(), 0);
  iota(guessesI.begin(), guessesI.end(), 0);
  cout << "\nTop first guesses according to heuristic:" << "\n";
  printRank(guesses, rankGuesses(outputCache, answersI, guessesI, 10));
  Tree tree(outputCache, answers, guesses, answersI, guessesI);
  tree.search();
  tree.root.updateEV();
  cout << "\nBest first guess: " << guesses[tree.root.guess] << "\n";
  cout << "Expected number of guesses: " << tree.root.ev << "\n";
}