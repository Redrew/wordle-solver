#include <algorithm>
#include <cassert>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <math.h>
#include <numeric>
#include <random>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#define NUM_LETTERS 26
#define NUM_COLOUR_COMBINATIONS 243
#define FINISHED 242
#define NO_EV -1
#define ll long long

using namespace std;

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
  vector<ll> coloursCount(NUM_COLOUR_COMBINATIONS);
  double n = answers.size();
  double entropy = 0;
  for (const ll &answer : answers) {
    coloursCount[coloursLookup.find(answer, guess).value]++;
  }
  for (ll colours = 0; colours < NUM_COLOUR_COMBINATIONS; colours++) {
    ll count = coloursCount[colours];
    double logp, p = count / n;
    if (colours != FINISHED) {
      logp = log((count + 1) / n);
    } else {
      logp = log(p);
    }
    if (p != 0) {
      entropy += -p * logp;
    }
  }
  return entropy;
}

vector<pair<double, ll>> rankGuesses(const ColoursLookup &coloursLookup,
                                     const vector<ll> &answers,
                                     const vector<ll> &guesses, const ll &k) {
  vector<pair<double, ll>> values;

  values.reserve(answers.size());
  for (const ll &guess : guesses) {
    values.push_back(
        pair(estimateGuessValue(coloursLookup, answers, guess), guess));
  }
  nth_element(values.begin(), values.end() - k, values.end());
  vector<pair<double, ll>> topk(values.end() - k, values.end());
  return topk;
}

void printRank(const vector<string> &guesses,
               vector<pair<double, ll>> rankedGuesses) {
  sort(rankedGuesses.rbegin(), rankedGuesses.rend());
  for (const pair<double, ll> &guess : rankedGuesses) {
    cout << "Guess: " << guesses[guess.second] << ", Value: " << guess.first
         << "\n";
  }
}

struct Node {
  ll guess, size;
  unordered_map<ll, Node *> children;
  double ev = NO_EV;
  Node(ll guess, ll size) : guess(guess), size(size) {}
  void updateEV() {
    ev = 1;
    for (const auto &item : children) {
      if (item.first == FINISHED) {
        continue;
      } else if (item.second->ev == NO_EV) {
        item.second->updateEV();
      }
      double p = item.second->size / (double)size;
      ev += p * item.second->ev;
    }
  }
};

struct Tree {
  const vector<string> &answers, &guesses;
  const vector<ll> &answersI, &guessesI;
  const ColoursLookup &coloursLookup;
  Node *root;
  Tree(const ColoursLookup &coloursLookup, const vector<string> &answers,
       const vector<string> &guesses, const vector<ll> &answersI,
       const vector<ll> &guessesI)
      : answers(answers), guesses(guesses), coloursLookup(coloursLookup),
        answersI(answersI), guessesI(guessesI) {}
  void search() { search(root, answersI, guessesI); }
  void search(const ll &guess) { search(root, answersI, guessesI, guess); }
  void search(Node *&node, const vector<ll> &answersI,
              const vector<ll> &guessesI) {
    ll k = 1;
    if (answersI.size() > 100) {
        k = 10;
    }
    vector<pair<double, ll>> topK =
        rankGuesses(coloursLookup, answersI, guessesI, k);
    sort(topK.rbegin(), topK.rend());
    if (k > 1) {
        cout << "Start search\n";
    }
    for (const auto &item : topK) {
      ll guess = item.second;
      search(node, answersI, guessesI, guess);
      if (k > 1) {
        cout << "Guess: " << guesses[guess] << ", Ent: " << item.first << ", EV: " << node->ev << "\n";
      }
    }
    if (k > 1) {
        cout << "End search\n";
    }
  }
  void search(Node *&node, const vector<ll> &answersI,
              const vector<ll> &guessesI, const ll &guess) {
    Node *new_node = new Node(guess, answersI.size());
    unordered_map<ll, vector<ll>> answersIWithColour;
    for (const ll &answer : answersI) {
      ll colour = coloursLookup.find(answer, guess).value;
      answersIWithColour[colour].push_back(answer);
    }
    for (auto const &item : answersIWithColour) {
      if (item.first == FINISHED) {
        continue;
      }
      search(new_node->children[item.first], item.second, guessesI);
    }
    new_node->updateEV();
    if (node == nullptr || node->ev > new_node->ev) {
      node = new_node;
    }
  }
};

int main() {
  vector<string> answers = read_file("words/answers.txt");
  vector<string> guesses = read_file("words/guesses.txt");
  shuffle(answers.begin(), answers.end(), default_random_engine(0));
  shuffle(guesses.begin(), guesses.end(), default_random_engine(1));
  guesses.insert(guesses.end(), answers.begin(), answers.end());
  cout << "Length of answers: " << answers.size() << "\n";
  cout << "Length of guesses: " << guesses.size() << "\n";
  ColoursLookup coloursLookup(answers, guesses);
  vector<ll> answersI(answers.size()), guessesI(guesses.size());
  iota(answersI.begin(), answersI.end(), 0);
  iota(guessesI.begin(), guessesI.end(), 0);
  //   vector<pair<double, ll>> rankedGuesses = rankGuesses(coloursLookup,
  //   answersI, guessesI, 10); for (const auto &item: rankedGuesses) {
  //     const ll &guess = item.second;
  //     cout << "First Guess: " << guesses[guess] << "\n";
  //     cout << "Entropy: " << item.first << "\n";
  //     Tree tree(coloursLookup, answers, guesses, answersI, guessesI);
  //     tree.search(guess);
  //     tree.root->updateEV();
  //     cout << "EV: " << tree.root->ev << "\n";
  //   }
  Tree tree(coloursLookup, answers, guesses, answersI, guessesI);
  tree.search();
  tree.root->updateEV();
  cout << "Guess: " << guesses[tree.root->guess] << "\n";
  cout << "EV: " << tree.root->ev << "\n";
}