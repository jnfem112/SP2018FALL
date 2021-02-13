#ifndef CSIE_BOX_H
#define CSIE_BOX_H

#include <string>
#include <queue>
using namespace std;

#define BUFFER_SIZE 1024

struct Config
{
	string fifo_path , directory_name;
};

struct Event
{
	string event_type , file_name , file_type;
};

typedef struct Config Config;
typedef struct Event Event;

bool read_config(string file_name , Config &config);
bool create_fifo(Config &config);
void remove_fifo(Config &config);
bool check_file_exist(string file_name);
void empty_directory(string directory_name , int remove_root);
void list_directory(string directory_name , int list_root , queue <Event> &event_list);
void watch_directory(string directory_name , int inotify_fd , vector <int> &wds , vector <string> &wdirs);
void watch_all_directory(Config &config , int inotify_fd , vector <int> &wds , vector <string> &wdirs , int watch_root);
void ignore_directory(string directory_name , int inotify_fd , vector <int> &wds , vector <string> &wdirs);
void ignore_all_directory(Config &config , int inotify_fd , vector <int> &wds , vector <string> &wdirs , int ignore_root);

#endif