//clang++ -g --std=c++11 -Werror -lcurl -pthread creatingimportance.cpp -o runme
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

int num_threads;
std::unordered_map <std::string,int> importance;
std::vector<std::string> urls;
auto startTime = std::chrono::system_clock::now();
int counter;
CURL* curls[24];
std::chrono::duration<double> elapsed_seconds;
typedef std::unordered_map<int, pthread_t> ThreadMap;
ThreadMap threads;
std::mutex urlsLock;
std::mutex importanceLock;

void save(int s){
    for (int i = 0; i < num_threads; i++) {
        pthread_cancel(threads[i]);
    }
    for (int i = 0; i < num_threads; i++) {
        curl_easy_cleanup(curls[i]);
    }
    elapsed_seconds = std::chrono::system_clock::now() - startTime;
    double duration = elapsed_seconds.count();
    std::cout << "\n" << counter << " urls were searched in " << duration
      << " seconds, good for " << counter/duration <<  " pages/second. " <<
      urls.size() << " urls to go.\n";
    startTime = std::chrono::system_clock::now();
    std::ofstream outfile("importance");
    for (auto it = importance.begin(); it != importance.end(); ++it) {
        outfile << it->first + "#" + std::to_string(it->second) << "\n";
    }
    outfile.close();
    std::ofstream urlfile("urls");
    for (int i = 0; i < urls.size(); i++) {
        urlfile << urls[i] << "\n";
    }
    urlfile.close();
    elapsed_seconds = std::chrono::system_clock::now() - startTime;
    printf("\nWriting to files took %g seconds.\n", elapsed_seconds.count());
    exit(0);
}

size_t writeFunction(void *ptr, size_t size, size_t nmemb, std::string* data) {
    data->append((char*) ptr, size * nmemb);
    return size * nmemb;
}

void getText(int id, CURL* curl) {
  std::string totalURL, page;
  char hrefChar[200];
  int i, j, pos;
  while (1) {
    urlsLock.lock();
    page = urls.back();
    urls.pop_back();
    counter++;
    urlsLock.unlock();
    totalURL = "https://en.wikipedia.org/w/api.php?action=query&titles="+page+
      "&prop=revisions&rvprop=content&format=json&formatversion=2";
    char charURL[totalURL.length()+1];
    for (i = 0; i < totalURL.length(); i++) {
        charURL[i] = totalURL[i];
    }
    charURL[i] = '\0';

    std::string response_string;
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    curl_easy_setopt(curl, CURLOPT_URL, &charURL);
    if (!curl_easy_perform(curl)) {
        for (i = 0; i < response_string.size()-10; i++) {
            if (response_string[i] == '[' && response_string[i-1] == '[') {
                j = i + 1;
                pos = 0;
                if (response_string.substr(j, 5) == "File:"){}
                else if (response_string.substr(j,6) == "Image:"){}
                else if (response_string.substr(j,9) == "Category:"){break;}
                else {
                    while ((response_string[j] != ']') && (response_string[j] != '|') && (response_string[j] != '#')) {
                        if (response_string[j] == ' ') {
                            hrefChar[pos] = '_';
                        }
                        else {
                            hrefChar[pos] = response_string[j];
                        }
                        j++;
                        pos++;
                    }
                    std::string hrefStr(hrefChar);
                    for (int y = 0; y < 200; y++) {
                        hrefChar[y] = '\0';
                    }
                    if (!(hrefStr.length() < 1) && hrefStr != "Main_Page") {
                        importanceLock.lock();
                        try {
                            importance.at(hrefStr) += 1;
                            importanceLock.unlock();
                        } catch (std::exception e) {
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
    } else {
        std::cout << "Bad link: " << charURL << " \n";
    }
    if (!urls.size()) {
        if (!id) {
            printf("Looks like it's all over, folks. GG.");
        }
        return;
    }
  }
}

int main() {
    std::cout << "Enter number of threads (max = 24): ";
    std::cin >> num_threads;
    startTime = std::chrono::system_clock::now();
    std::string line, page, refs;
    int pos;
    std::ifstream infile ("importance");
    if (infile.is_open()) {
        while(getline(infile,line)) {
            pos = line.find('#');
            page = line.substr(0,pos);
            refs = line.substr(pos+1,1);
            importance.emplace(page, stoi(refs));
        }
        infile.close();
    } else {
        std::cout << "no importance file found\n";
        importance = {{"Wolmirstedt_(Verwaltungsgemeinschaft)",0},{"NASA",0},
          {"The_Race_(Seinfeld)",0},{"Secretary_for_Petroleum",0}, {"Mango",0},
          {"Francisco_Trevino",0},{"Louis_d'Auvigny",0},{"Nathaniel_Uring",0},
          {"2008_European_Pairs_Speedway_Championship",0},{"Danfoss",0},
          {"Buena_High_School_(California",0}, {"Goniodromites",0},
          };
    }
    std::ifstream nextfile ("urls");
    if (nextfile.is_open()) {
        while(getline(nextfile,line)) {
            urls.push_back(line);
        }
        nextfile.close();
        page = urls.back();
        urls.pop_back();
    } else {
        std::cout << "no urls file found\n";
        urls.push_back("Mango");
        urls.push_back("NASA");
        urls.push_back("Goniodromites");
        urls.push_back("Danfoss");
        urls.push_back("The_Race_(Seinfeld)");
        urls.push_back("Secretary_for_Petroleum");
        urls.push_back("Francisco_Trevino");
        urls.push_back("Nathaniel_Uring");
        urls.push_back("Wolmirstedt_(Verwaltungsgemeinschaft)");
        urls.push_back("Buena_High_School_(California)");
        urls.push_back("Louis_d'Auvigny");
        urls.push_back("2008_European_Pairs_Speedway_Championship");
    }
    elapsed_seconds = std::chrono::system_clock::now() - startTime;
    std::cout << "Getting from files takes " << elapsed_seconds.count() <<
      " seconds.\n";
    counter = 0;
    signal(SIGINT, save);

    std::thread* actualThreads = new std::thread[num_threads];
    for (int i = 0; i < num_threads; i++){
        curls[i] = curl_easy_init();
        curl_easy_setopt(curls[i], CURLOPT_USERAGENT, "WikiRider");
        curl_easy_setopt(curls[i], CURLOPT_MAXREDIRS, 50L);
        curl_easy_setopt(curls[i], CURLOPT_WRITEFUNCTION, writeFunction);
        actualThreads[i] = std::thread(getText,i,curls[i]);
        threads[i] = actualThreads[i].native_handle();
    }
    startTime = std::chrono::system_clock::now();

    for (int i = 0; i < num_threads; i++) {
        actualThreads[i].join();
    }
    delete[] actualThreads;
    save(12);
    return 0;
}
