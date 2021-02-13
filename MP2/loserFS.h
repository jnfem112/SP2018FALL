#ifndef LOSERFS_H
#define LOSERFS_H

#include <string>
#include <vector>
using namespace std;

#define BUFFER_SIZE 1024

struct Config
{
	string name;
	int number_of_peer;
	vector <string> peers;
	string repo;
};

typedef struct Config Config;

bool read_config(string file_name , Config &config);
bool create_socket(Config &config , int &server_socket , vector <int> &peer_socket);
bool connect_socket(Config &config , vector <int> &peer_socket , int index);
void close_socket(Config &config , int server_socket , vector <int> &peer_socket , vector <int> &client_socket);
void read_command(string &command , vector <string> &arguments);
bool check_file_exist(string file_name);
int simple_copy(string source , string destination);

#endif