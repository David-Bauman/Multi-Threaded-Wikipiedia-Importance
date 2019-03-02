#include <iostream>
#include <unordered_map>
#include <fstream>
#include <vector>
#include <signal.h>
#include <curl/curl.h>
#include <string>
#include <cstring>
#include <thread>
#include <mutex>
#include <math.h>

int numThreads, counter;
std::unordered_map <std::string, int> importance;
std::vector<std::string> urls;
CURL* curls[100];
std::chrono::duration<double> elapsedSeconds;
std::unordered_map<int, pthread_t> threads;
std::mutex urlsLock, importanceLock;
auto startTime = std::chrono::system_clock::now();

inline std::string addCommas(int val) {
  std::string withCommas = std::to_string(val);
  int pos = withCommas.length() - 3;
  while (pos > 0) {
    withCommas.insert(pos, ",");
    pos -= 3;
  }
  return withCommas;
};

void save(int s) {
    for (int i = 0; i < numThreads; i++) {
        pthread_cancel(threads[i]);
    }
    for (int i = 0; i < numThreads; i++) {
        curl_easy_cleanup(curls[i]);
    }
    elapsedSeconds = std::chrono::system_clock::now() - startTime;
    double duration = elapsedSeconds.count();

    std::cout << "\n" << addCommas(counter) << " urls were searched in " << round(duration * 100) / 100
      << " seconds, good for " << round(100*counter/duration) / 100 <<  " pages/second. " <<
      addCommas(urls.size()) << " urls to go." << std::endl;

    startTime = std::chrono::system_clock::now();
    std::ofstream outfile("data/importance.txt");
    for (auto it = importance.begin(); it != importance.end(); ++it) {
        outfile << it->first + "#" + std::to_string(it->second) << "\n";
    }
    outfile.close();
    std::ofstream urlfile("data/urls.txt");
    for (int i = 0; i < urls.size(); i++) {
        urlfile << urls[i] << "\n";
    }
    urlfile.close();
    elapsedSeconds = std::chrono::system_clock::now() - startTime;
    printf("\nWriting to files took %g seconds.\n", elapsedSeconds.count());
    exit(s);
}

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string * data) {
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}

void getText(int id, CURL* curl) {
  std::string totalURL, page, responseString, hrefStr;
  int i, j, pos, res, threadTotal;
  threadTotal = 0;
  char temp;
  while (1) {
    urlsLock.lock();
    if (urls.size() == 0) {
        elapsedSeconds = std::chrono::system_clock::now() - startTime;
        std::cout << "Thread #" << id << " reporting in. My watch has ended after "
         << round(elapsedSeconds.count() * 100) / 100 << " seconds and " 
         << threadTotal << " successful searches." << std::endl;
        urlsLock.unlock();
        return;
    }
    page = urls.back();
    urls.pop_back();
    counter++;
    urlsLock.unlock();
    responseString = "";
    totalURL = "https://en.wikipedia.org/w/api.php?action=query&titles="+page+
      "&prop=revisions&rvprop=content&format=json&formatversion=2";
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseString);
    curl_easy_setopt(curl, CURLOPT_URL, totalURL.c_str());
    res = curl_easy_perform(curl);
    if (res == 0) {
        for (i = 0; i < responseString.size()-10; i++) {
            if (responseString[i] == '[' && responseString[i-1] == '[') {
                j = i + 1;

                if (responseString.substr(j, 5) == "File:"){continue;}
                else if (responseString.substr(j, 6) == "Image:"){continue;}
                else if (responseString.substr(j, 8) == "Special:"){continue;}
                else if (responseString.substr(j, 9) == "Category:"){break;}
                else {
                    hrefStr = "";
                    temp = responseString[j];
                    while ((temp != ']') && (temp != '|') && (temp != '#')) {
                        if (temp == ' ') {
                            hrefStr += '_';
                        } else {
                            hrefStr += temp;
                        }
                        j++;
                        temp = responseString[j];
                    }
                    if (hrefStr.length() > 0 && hrefStr != "Main_Page" && hrefStr.find(':') == std::string::npos) {
                        importanceLock.lock();
                        if (importance.count(hrefStr) == 1) {
                            importance.at(hrefStr) += 1;
                            importanceLock.unlock();
                        } else {
                            importance[hrefStr] = 1;
                            importanceLock.unlock();
                            urlsLock.lock();
                            urls.push_back(hrefStr);
                            urlsLock.unlock();
                        }
                    }
                }
            }
        }
        threadTotal++;
    } else {
        std::cout << "Bad link: " << totalURL << " gives an error code of " << res << " \n";
    }
  }
}

int main() {
    std::cout << "Enter number of threads (max 100): ";
    std::cin >> numThreads;
    if (numThreads > 100 || numThreads < 1) {
        std::cout << "No funny stuff - 100 threads maximum, 1 thread minimum." << std::endl;
        exit(1);
    }
    startTime = std::chrono::system_clock::now();
    std::string line, page, refs;
    int pos;

    std::ifstream importanceFile ("data/importance.txt");
    if (importanceFile.is_open()) {
        while(getline(importanceFile, line)) {
            pos = line.find('#');
            page = line.substr(0,pos);
            refs = line.substr(pos+1,1);
            importance.emplace(page, stoi(refs));
        }
        importanceFile.close();
    } else {
        std::cout << "no importance file found" << std::endl;
        std::ifstream infile ("data/initialUrls.txt");
        if (!infile.is_open())
            throw std::runtime_error("Couldn't find any initial urls.");
        while (getline(infile, line) && urls.size() < (numThreads + 5)) {
            importance.emplace(line, 0);
            urls.push_back(line);
        }
    }

    std::ifstream urlsFile ("data/urls.txt");
    if (urlsFile.is_open()) {
        while(getline(urlsFile, line)) {
            urls.push_back(line);
        }
        urlsFile.close();
    } else {
        std::cout << "no urls file found" << std::endl;
    }

    elapsedSeconds = std::chrono::system_clock::now() - startTime;
    std::cout << "Getting from files takes " << elapsedSeconds.count() <<
      " seconds.\n";
    counter = 0;
    signal(SIGINT, save);

    std::thread* actualThreads = new std::thread[numThreads];
    for (int i = 0; i < numThreads; i++){
        curls[i] = curl_easy_init();
        curl_easy_setopt(curls[i], CURLOPT_USERAGENT, "WikiRider");
        curl_easy_setopt(curls[i], CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curls[i], CURLOPT_WRITEFUNCTION, writeFunction);
        actualThreads[i] = std::thread(getText,i,curls[i]);
        threads[i] = actualThreads[i].native_handle();
    }
    std::cout << "All threads started" << std::endl;
    startTime = std::chrono::system_clock::now();

    for (int i = 0; i < numThreads; i++) {
        actualThreads[i].join();
    }
    save(0);
}
