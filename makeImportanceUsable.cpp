#include <iostream>
#include <fstream>
#include <queue>

class CompareImportance {
  public:
    bool operator()(std::pair<int, std::string> lhs, std::pair<int, std::string> rhs) {
      return lhs.first < rhs.first;
    }
};

inline void replaceAll(std::string& str, const std::string& from, const std::string& to) {
  size_t start_pos = 0;
  while((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length(); 
  }
}

inline std::string addCommas(int val) {
  std::string withCommas = std::to_string(val);
  int pos = withCommas.length() - 3;
  while (pos > 0) {
    withCommas.insert(pos, ",");
    pos -= 3;
  }
  return withCommas;
};

int main() {
  std::priority_queue<std::pair<int, std::string>, std::vector<std::pair<int, std::string>>, CompareImportance> data;
  std::ifstream infile ("data/importance.txt");

  std::string line, page, refs;
  int pos;
  int totalRefs = 0;
  if (infile.is_open()) {
    while(getline(infile,line)) {
      pos = line.find('#');
      page = line.substr(0, pos);
      refs = line.substr(pos+1, std::string::npos);
      totalRefs += stoi(refs);
      data.push(std::pair<int, std::string>(stoi(refs), page));
    }
  }

  int totalPages = data.size();
  std::pair<int, std::string> temp;

  for (int i = 0; i < 20; i++) {
    if (data.empty()) {
      break;
    }
    temp = data.top();
    data.pop();
    page = temp.second;
    replaceAll(page, "_", " ");
    std::cout << page << ": " << addCommas(temp.first) << std::endl;
  }

  std::cout << "\n\n# of total references to other pages: " << addCommas(totalRefs) << std::endl;

  std::cout << "# of pages referenced: " << addCommas(totalPages) << std::endl;

  double avg = (double) totalRefs / totalPages;
  std::cout << "Average # of references to each page: " << avg << std::endl;

  return 0;
}
