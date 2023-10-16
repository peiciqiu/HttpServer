#include "./WordIndex.h"
#include <algorithm>

namespace searchserver {

WordIndex::WordIndex() {
  unique_words = 0;
}

size_t WordIndex::num_words() {
  return word_index_map.size();
}

void WordIndex::record(const string& word, const string& doc_name) {
    if (word_index_map.count(word) == 0) {
      word_index_map.insert(std::make_pair(word, std::unordered_map<std::string, int>()));
      word_index_map[word].insert(std::make_pair(doc_name,1));
      unique_words++;
    } else {
      if (word_index_map[word].count(doc_name) == 0) {
        word_index_map[word].insert(std::make_pair(doc_name,1));
      } else {
        word_index_map[word][doc_name]++;
      }
    
    }

    if (doc_index_map.count(doc_name) == 0) {
      doc_index_map.insert(std::make_pair(doc_name, std::unordered_set<std::string>()));
    }
    doc_index_map[doc_name].insert(word);

}

vector<Result> WordIndex::lookup_word(const string& word) {
  vector<Result> result;
  std::unordered_map<std::string, int> word_map;
  if (word_index_map.count(word) != 0) {
    word_map = word_index_map[word];
  } else {
    return result;
  }

  for (auto const& pair: word_map) {
      result.push_back(Result(pair.first, pair.second));
  }

  sort(result.begin(),result.end());

  return result;
}

vector<Result> WordIndex::lookup_query(const vector<string>& query) {

  vector<Result> results;
  for (auto const& kv: doc_index_map) {
      auto& doc_name = kv.first;
      auto& doc_word_set = kv.second;
      bool contains_all_words = true;
      for (auto const& word: query) {
        if (doc_word_set.find(word) == doc_word_set.end()) {
          contains_all_words = false;
        }
      }

      if (contains_all_words) {
        int rank = 0;
        for (auto const& word: query) {
          rank += word_index_map[word][doc_name];
        }
        results.push_back(Result(doc_name, rank));
      }
  }

  sort(results.begin(),results.end());


  return results;
}

}  // namespace searchserver