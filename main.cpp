#include <algorithm>
#include <cassert>
#include <fstream>
#include <future>
#include <iostream>
#include <limits>
#include <map>
#include <math.h>
#include <numeric>
#include <random>
#include <string>
#include <thread>
#include <unordered_map>
#include <utility>
#include <vector>

#define NUM_LETTERS 26
#define NUM_COLOUR_COMBINATIONS 243
#define FINISHED 242
#define NO_EV -1
#define NO_GUESS -1
#define ll long long

using namespace std;

typedef pair<double, ll> ranked_guess_t;
typedef vector<ranked_guess_t> ranked_guesses_t;

const ll charToInt(const char &c) { return c - 'a'; }

const vector<string> read_file(const string &filename) {
  vector<string> words;
  string line;
  ifstream file(filename);
  if (file.is_open()) {
    while (getline(file, line)) {
      words.push_back(line);
    }
    file.close();
  }
  return words;
}

struct Colours {
  ll value;
  Colours(ll value) : value(value) {}
  string getString() {
    string response = "----- ";
    string allColours = "-yg";
    ll n = value;
    for (ll i = 0; i < 5; i++) {
      ll colourAtI = n % 3;
      n = n / 3;
      response[i] = allColours[colourAtI];
    }
    return response;
  }
};

struct Wordle {
  string answer;
  Wordle(const string &answer) : answer(answer) {}
  Colours query(const string &guess) {
    vector<ll> letterCount(NUM_LETTERS);
    ll value = 0;
    for (ll i = 0; i < 5; i++) {
      ll answerLetter = charToInt(answer[i]);
      if (answer[i] == guess[i]) {
        value += 2 * pow(3, i);
      } else {
        letterCount[answerLetter]++;
      }
    }
    for (ll i = 0; i < 5; i++) {
      ll guessLetter = charToInt(guess[i]);
      if (answer[i] != guess[i] && letterCount[guessLetter] > 0) {
        letterCount[guessLetter]--;
        value += 1 * pow(3, i);
      }
    }
    return Colours(value);
  }
};

struct ColoursLookup {
  vector<Colours> coloursLookup;
  ll nAnswers, nGuesses;
  ColoursLookup(const vector<string> &answers, const vector<string> &guesses) {
    coloursLookup.reserve(answers.size() * guesses.size());
    nAnswers = answers.size();
    nGuesses = guesses.size();
    for (ll answerI = 0; answerI < nAnswers; answerI++) {
      const string &answer = answers[answerI];
      for (ll guessI = 0; guessI < nGuesses; guessI++) {
        const string &guess = guesses[guessI];
        coloursLookup[answerI * nGuesses + guessI] =
            Wordle(answer).query(guess);
      }
    }
  }
  Colours find(const ll &answerI, const ll &guessI) const {
    return coloursLookup[answerI * nGuesses + guessI];
  }
};

double estimateGuessValue(const ColoursLookup &coloursLookup,
                          const vector<ll> &answers, const ll &guess) {
  unordered_map<ll, ll> coloursCount;
  double dn = 1 / (double)answers.size();
  double entropy = 0;
  for (const ll &answer : answers) {
    coloursCount[coloursLookup.find(answer, guess).value]++;
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

ranked_guesses_t rankGuesses(const ColoursLookup &coloursLookup,
                             const vector<ll> &answers,
                             const vector<ll> &guesses, const ll &k) {
  ranked_guesses_t values;

  values.reserve(answers.size());
  for (const ll &guess : guesses) {
    values.push_back(ranked_guess_t(
        estimateGuessValue(coloursLookup, answers, guess), guess));
  }
  nth_element(values.begin(), values.end() - k, values.end());
  ranked_guesses_t topk(values.end() - k, values.end());
  return topk;
}

void printRank(const vector<string> &guesses, ranked_guesses_t rankedGuesses) {
  sort(rankedGuesses.rbegin(), rankedGuesses.rend());
  for (const ranked_guess_t &guess : rankedGuesses) {
    cout << "Guess: " << guesses[guess.second] << ", Value: " << guess.first
         << "\n";
  }
}

struct Node {
  ll guess, size;
  unordered_map<ll, Node> children;
  double ev = NO_EV;
  Node(ll guess = NO_GUESS, ll size = 0) : guess(guess), size(size) {}
  bool hasEV() { return ev != NO_EV; }
  void updateEV() {
    ev = 1;
    for (auto &item : children) {
      if (item.first == FINISHED) {
        continue;
      } else if (!item.second.hasEV()) {
        item.second.updateEV();
      }
      double p = item.second.size / (double)size;
      ev += p * item.second.ev;
    }
  }
};

struct Tree {
  const vector<string> &answers, &guesses;
  const vector<ll> &answersI, &guessesI;
  const ColoursLookup &coloursLookup;
  Node root;
  Tree(const ColoursLookup &coloursLookup, const vector<string> &answers,
       const vector<string> &guesses, const vector<ll> &answersI,
       const vector<ll> &guessesI)
      : answers(answers), guesses(guesses), coloursLookup(coloursLookup),
        answersI(answersI), guessesI(guessesI) {}
  void search() { root = findBestNode(answersI, guessesI); }
  Node findBestNode(const vector<ll> &answersI,
                    const vector<ll> &guessesI) const {
    if (answersI.size() == answers.size()) {
      return findBestNodeAsync(
          answersI, guessesI,
          rankGuesses(coloursLookup, answersI, guessesI, 10));
    } else {
      return findBestNodeSync(
          answersI, guessesI,
          rankGuesses(coloursLookup, answersI, guessesI, 1));
    }
  }

  Node findBestNodeSync(const vector<ll> &answersI, const vector<ll> &guessesI,
                        const ranked_guesses_t &rankedGuesses) const {
    Node bestNode;
    if (answersI.size() == 0) {
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
    if (answersI.size() == 0) {
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
      ll colour = coloursLookup.find(answer, guess).value;
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
  cout << "Length of answers: " << answers.size() << "\n";
  cout << "Length of guesses: " << guesses.size() << "\n";
  ColoursLookup coloursLookup(answers, guesses);
  vector<ll> answersI(answers.size()), guessesI(guesses.size());
  iota(answersI.begin(), answersI.end(), 0);
  iota(guessesI.begin(), guessesI.end(), 0);
  printRank(guesses, rankGuesses(coloursLookup, answersI, guessesI, 10));
  Tree tree(coloursLookup, answers, guesses, answersI, guessesI);
  tree.search();
  tree.root.updateEV();
  cout << "Guess: " << guesses[tree.root.guess] << "\n";
  cout << "EV: " << tree.root.ev << "\n";
}