#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <vector>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "loserFS.h"
using namespace std;

bool read_config(string file_name , Config &config)
{
	ifstream config_file(file_name);
	string line;

	if (!config_file.is_open())
		return false;

	getline(config_file , line);
	line = line.substr(line.find("=") + 1);
	istringstream input_1(line);
	input_1 >> config.name;
	config.name = "/tmp/mp2-" + config.name + ".sock";

	getline(config_file , line);
	line = line.substr(line.find("=") + 1);
	istringstream input_2(line);
	string peer;
	config.number_of_peer = 0;
	while (input_2 >> peer)
	{
		peer = "/tmp/mp2-" + peer + ".sock";
		config.peers.push_back(peer);
		config.number_of_peer++;
	}

	getline(config_file , line);
	line = line.substr(line.find("=") + 1);
	istringstream input_3(line);
	input_3 >> config.repo;

	if (access(config.repo.c_str() , W_OK) == -1)
		return false;

	return true;
}

bool create_socket(Config &config , int &server_socket , vector <int> &peer_socket)
{
	struct sockaddr_un address;

	unlink(config.name.c_str());
	
	if ((server_socket = socket(AF_UNIX , SOCK_STREAM , 0)) == -1)
		return false;

	memset(&address , 0 , sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path , config.name.c_str() , sizeof(address.sun_path) - 1);

	if (bind(server_socket , (const struct sockaddr *)&address , sizeof(struct sockaddr_un)) == -1)
		return false;

	if (listen(server_socket , 20) == -1)
		return false;

	for (int i = 0 ; i < config.number_of_peer ; i++)
		if ((peer_socket[i] = socket(AF_UNIX , SOCK_STREAM , 0)) == -1)
			return false;

	return true;
}

bool connect_socket(Config &config , vector <int> &peer_socket , int index)
{
	struct sockaddr_un address;
	memset(&address , 0 , sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	strncpy(address.sun_path , config.peers[index].c_str() , sizeof(address.sun_path) - 1);
	return connect(peer_socket[index] , (const struct sockaddr *)&address , sizeof(struct sockaddr_un)) != -1;
}

void close_socket(Config &config , int server_socket , vector <int> &peer_socket , vector <int> &client_socket)
{
	int number_of_client = client_socket.size();
	for (int i = 0 ; i < number_of_client ; i++)
		close(client_socket[i]);

	for (int i = 0 ; i < config.number_of_peer ; i++)
		close(peer_socket[i]);

	close(server_socket);
	unlink(config.name.c_str());

	return;
}

void read_command(string &command , vector <string> &arguments)
{
	string line;
	getline(cin , line);
	istringstream input(line);
	input >> command;
	arguments.clear();
	string argument;
	while (input >> argument)
		arguments.push_back(argument);
	return;
}

bool check_file_exist(string file_name)
{
	struct stat temp;
	if (file_name[0] == '@')
		file_name = file_name.substr(1);
	return stat(file_name.c_str() , &temp) == 0;
}

int simple_copy(string source , string destination)
{
	if (source[0] == '@' && !check_file_exist(source))
		return 2;
	else
	{
		FILE *in_file;

		if (source[0] == '@')
			in_file = fopen(source.substr(1).c_str() , "rb");
		else
			in_file = fopen(source.c_str() , "rb");

		if (!in_file)
			return 0;

		FILE *out_file;
		char buffer[BUFFER_SIZE + 1] = {0};
		int byte;

		if (destination[0] == '@')
			out_file = fopen(destination.substr(1).c_str() , "wb");
		else
			out_file = fopen(destination.c_str() , "wb");

		while ((byte = fread(buffer , sizeof(char) , BUFFER_SIZE , in_file)) > 0)
			fwrite(buffer , sizeof(char) , byte , out_file);

		fclose(in_file);
		fclose(out_file);
	}

	return 1;
}