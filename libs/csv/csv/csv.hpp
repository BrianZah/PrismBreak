#ifndef CSV_HPP
#define CSV_HPP

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <charconv>
#include <algorithm>
#include <limits>

namespace csv {
namespace {
  template<class T>
  inline T cast(const std::string& value);
  template<>
  inline float cast<float>(const std::string& value) {return std::stof(value);}
  template<>
  inline int cast<int>(const std::string& value) {return std::stoi(value);}
  template<>
  inline std::string cast<std::string>(const std::string& value) {return value;}
}
template<class A>
A toArray(const std::string& filename, const std::string& seperator = ",", const std::vector<int>& discardRows = {}, const std::vector<int>& discardCols = {}) {
  std::fstream file(filename, std::ios::in);
  if(!file.is_open())
    std::cerr << "[Error] (csv::getData) failed to open" << filename;

  std::string line;
  A array;
  std::cout << "[Info] (csv::getData) test1" << filename;
  using T = A::value_type;
  for(int row = 0; std::getline(file, line); ++row) {
    if(std::find(discardRows.begin(), discardRows.end(), row) != discardRows.end()) continue;
    for(int pos = 0, col = 0; pos < line.size(); ++col) {
      int nextpos;
      if(line.substr(pos, 1) == std::string("\"")) {
        nextpos = line.find("\"", ++pos);
        if(std::find(discardCols.begin(), discardCols.end(), col) == discardCols.end())
          array.emplace_back(cast<T>(line.substr(pos, nextpos-pos)));
        nextpos = line.find(seperator, nextpos);
      } else {
        nextpos = line.find(seperator, pos);
        if(std::find(discardCols.begin(), discardCols.end(), col) == discardCols.end())
          array.emplace_back(cast<T>(line.substr(pos, nextpos-pos)));
      }
      pos = (nextpos != line.npos) ? nextpos+1 : line.size();
    }
  }
  file.close();
  return array;
}

// constexpr bool ignore(const int& index, const int& min, const int& max, const std::vector<int>& discardIndex) {
//   return index < min || (0 < max && max < index) || std::find(discardIndex.begin(), discardIndex.end(), index) != discardIndex.end();
// }
// template<class A>
// A toArray(const std::string& filename, const std::string& seperator = ",",
//           const int& firstRow = 0, int& lastRow = -1,
//           const int& firstCol = 0, int& lastCol = -1,
//           const std::vector<int>& discardRows = {}, const std::vector<int>& discardCols = {}) {
//   std::fstream file(filename, std::ios::in);
//   if(not file.is_open())
//     throw std::string("[Error] (csv::toArray) failed to open" + filename);
//
//   std::string line;
//   A array;
//   using T = A::value_type;
//   for(int row = 0; std::getline(file, line); ++row) {
//     if(ignore(row, firstRow, lastRow, discardRows)) continue;
//     std::string elem;
//     for(int pos = 0, col = 0; pos < line.size(); ++col) {
//       int nextpos;
//       if(line.substr(pos, 1) == std::string("\"")) {
//         nextpos = line.find("\"", ++pos);
//         if(not ignore(col, firstCol, lastCol, discardCols))
//           array.emplace_back(cast<T>(line.substr(pos, nextpos-pos)));
//         nextpos = line.find(seperator, nextpos);
//       } else {
//         nextpos = line.find(seperator, pos);
//         if(not ignore(col, firstCol, lastCol, discardCols))
//           array.emplace_back(cast<T>(line.substr(pos, nextpos-pos)));
//       }
//       pos = (nextpos != line.npos) ? nextpos+1 : line.size();
//     }
//   }
//   file.close();
//   return array;
// }
constexpr bool ignore(const int& index, const std::vector<int>& discardIndex) {
  return std::find(discardIndex.begin(), discardIndex.end(), index) != discardIndex.end();
}

inline bool getElement(std::string& elem, const std::string& line, int& pos, const std::string& seperator) {
  if(pos == line.npos) return false;
  int nextpos;
  if(line.substr(pos, 1) == std::string("\"")) {
    nextpos = line.find("\"", ++pos);
    elem = line.substr(pos, nextpos-pos);
    nextpos = line.find(seperator, nextpos);
  } else {
    nextpos = line.find(seperator, pos);
    elem = line.substr(pos, nextpos-pos);
  }
  pos = (nextpos != line.npos) ? nextpos+1 : line.npos;
  return true;
}
inline bool ignoreElement(const std::string& line, int& pos, const std::string& seperator) {
  if(pos == line.npos) return false;
  int nextpos = (line.substr(pos, 1) == std::string("\"")) ? line.find("\"", ++pos) : pos;
  nextpos = line.find(seperator, nextpos);

  pos = (nextpos != line.npos) ? nextpos+1 : line.npos;
  return true;
}
template<class A = std::vector<float>>
std::tuple<A, int, std::vector<std::string>, std::vector<std::string>> getTable(const std::string& filename, const std::string& seperator = ",",
          const bool& hasColNames = true, const bool& hasRowNames = true,
          const std::vector<int>& discardRows = {}, const std::vector<int>& discardCols = {}) {
  std::fstream file(filename, std::ios::in);
  if(not file.is_open())
    throw std::string("[Error] (csv::toArray) failed to open" + filename);

  int rows = 0, cols = 0, numColNames = 0, colsDiff = 0;
  std::string line, elem;
  if(hasColNames) {
    if(std::getline(file, line)) {
      for(int pos = 0, col = 0; ignoreElement(line, pos, seperator); ++col) {
        --colsDiff;
        if(not ignore(col, discardCols)) ++numColNames;
      }
    }
  }
  if(std::getline(file, line)) {
    if(not ignore(0, discardRows)) ++rows;
    int pos = 0;
    if(hasRowNames) {
      ignoreElement(line, pos, seperator);
      ++colsDiff;
    }
    for(int col = 0; ignoreElement(line, pos, seperator); ++col) {
      ++colsDiff;
      if(not ignore(col, discardCols)) ++cols;
    }
  }
  colsDiff = hasColNames && hasRowNames ? colsDiff : 0;
  if(colsDiff < 0 || 1 < colsDiff) throw std::string("[Error] (csv::getTable) no valid format" + filename);

  for(int row = 1; file.ignore(std::numeric_limits<std::streamsize>::max(), file.widen('\n')); ++row) {
    if(not ignore(row, discardRows)) ++rows;
  }
  // file.ignore(...) counts last empty line in, therefore:
  rows = std::max(0, rows-1);
  file.close();

  file.open(filename, std::ios::in);
  if(not file.is_open())
    throw std::string("[Error] (csv::toArray) failed to open" + filename);

  std::vector<std::string> colNames(cols);
  std::string cornerName;
  int colOffset = (hasRowNames && 0 == colsDiff) ? 1 : 0;
  if(hasColNames) {
    if(std::getline(file, line)) {
      int pos = 0;
      if(hasRowNames && 0 == colsDiff) {
        if(getElement(elem, line, pos, seperator)) cornerName = elem;
      }
      for(int col = 0;; ++col) {
        if(ignore(col, discardCols)) {
          if(ignoreElement(line, pos, seperator)) continue;
        } else if(getElement(elem, line, pos, seperator)) {
          colNames[col] = elem;
          continue;
        }
        break;
      }
    }
  }

  std::vector<std::string> rowNames(rows);
  A array(rows*cols);
  using T = A::value_type;
  for(int row = 0;; ++row) {
    if(ignore(row, discardRows)) {
      if(file.ignore()) continue;
      else break;
    }
    if(not std::getline(file, line)) break;
    int pos = 0;
    if(hasRowNames) {
      getElement(elem, line, pos, seperator);
      rowNames[row] = elem;
    }
    for(int col = 0;; ++col) {
      if(ignore(col, discardCols)) {
        if(ignoreElement(line, pos, seperator)) continue;
      } else if(getElement(elem, line, pos, seperator)) {
        array[row*cols + col] = cast<T>(elem);
        continue;
      }
      break;
    }
  }
  file.close();
  return {array, cols, colNames, rowNames};
}
} // namespace csv

#endif//CSV_HPP
