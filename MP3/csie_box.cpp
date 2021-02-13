#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <unistd.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include <queue>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/inotify.h>
#include "csie_box.h"
using namespace std;

bool read_config(string file_name , Config &config)
{
	if (access(file_name.c_str() , R_OK) == -1)
		return false;

	ifstream config_file(file_name);
	string line;

	getline(config_file , line);
	line = line.substr(line.find("=") + 1);
	istringstream input_1(line);
	input_1 >> config.fifo_path;

	getline(config_file , line);
	line = line.substr(line.find("=") + 1);
	istringstream input_2(line);
	input_2 >> config.directory_name;

	return true;
}

bool create_fifo(Config &config)
{
	if (!check_file_exist(config.fifo_path + "/server_to_client.fifo") && mkfifo((config.fifo_path + "/server_to_client.fifo").c_str() , 0777) == -1)
		return false;
	if (!check_file_exist(config.fifo_path + "/client_to_server.fifo") && mkfifo((config.fifo_path + "/client_to_server.fifo").c_str() , 0777) == -1)
		return false;
	return true;
}

void remove_fifo(Config &config)
{
	remove((config.fifo_path + "/server_to_client.fifo").c_str());
	remove((config.fifo_path + "/client_to_server.fifo").c_str());
	return;
}

bool check_file_exist(string file_name)
{
	struct stat file_status;
	return lstat(file_name.c_str() , &file_status) == 0;
}

void empty_directory(string directory_name , int remove_root)
{
	if (!check_file_exist(directory_name))
		return;

	DIR *directory = opendir(directory_name.c_str());
	struct dirent *entry;
	string file_name;
	struct stat file_status;

	while ((entry = readdir(directory)) != 0)
	{
		if (strcmp(entry->d_name , ".") == 0 || strcmp(entry->d_name , "..") == 0)
			continue;

		file_name = directory_name + "/" + entry->d_name;
		lstat(file_name.c_str() , &file_status);
		if (S_ISREG(file_status.st_mode))
			remove(file_name.c_str());
		else if (S_ISLNK(file_status.st_mode))
			unlink(file_name.c_str());
		else if (S_ISDIR(file_status.st_mode))
			empty_directory(file_name , remove_root + 1);
	}

	if (remove_root)
		remove(directory_name.c_str());

	return;
}

void list_directory(string directory_name , int list_root , queue <Event> &event_list)
{
	DIR *directory = opendir(directory_name.c_str());
	struct dirent *entry;
	Event event;
	struct stat file_status;

	while ((entry = readdir(directory)) != 0)
	{
		if (strcmp(entry->d_name , ".") == 0 || strcmp(entry->d_name , "..") == 0)
			continue;

		event.file_name = directory_name + "/" + entry->d_name;
		lstat(event.file_name.c_str() , &file_status);
		if (S_ISREG(file_status.st_mode))
		{
			event.file_type = "file";
			event_list.push(event);
		}
		else if (S_ISLNK(file_status.st_mode))
		{
			event.file_type = "link";
			event_list.push(event);
		}
		else if (S_ISDIR(file_status.st_mode))
		{
			event.file_type = "directory";
			event_list.push(event);
			list_directory(event.file_name , list_root + 1 , event_list);
		}
	}

	return;
}

void watch_directory(string directory_name , int inotify_fd , vector <int> &wds , vector <string> &wdirs)
{
	int wd = inotify_add_watch(inotify_fd , directory_name.c_str() , IN_CREATE | IN_MODIFY | IN_DELETE);
	wds.push_back(wd);
	wdirs.push_back(directory_name);
	return;
}

void watch_all_directory(Config &config , int inotify_fd , vector <int> &wds , vector <string> &wdirs , int watch_root)
{
	if (watch_root)
		watch_directory(config.directory_name , inotify_fd , wds , wdirs);

	queue <Event> event_list;
	list_directory(config.directory_name , 0 , event_list);
	while (!event_list.empty())
	{
		Event event = event_list.front();
		event_list.pop();
		if (event.file_type == "directory")
			watch_directory(event.file_name , inotify_fd , wds , wdirs);
	}

	return;
}

void ignore_directory(string directory_name , int inotify_fd , vector <int> &wds , vector <string> &wdirs)
{
	int number_of_directory = wdirs.size();
	for (int i = 0 ; i < number_of_directory ; i++)
		if (wdirs[i] == directory_name)
		{
			inotify_rm_watch(inotify_fd , wds[i]);
			wds.erase(wds.begin() + i);
			wdirs.erase(wdirs.begin() + i);
			break;
		}

	return;
}

void ignore_all_directory(Config &config , int inotify_fd , vector <int> &wds , vector <string> &wdirs , int ignore_root)
{
	if (ignore_root)
		ignore_directory(config.directory_name , inotify_fd , wds , wdirs);

	queue <Event> event_list;
	list_directory(config.directory_name , 0 , event_list);
	while (!event_list.empty())
	{
		Event event = event_list.front();
		event_list.pop();
		if (event.file_type == "directory")
			ignore_directory(event.file_name , inotify_fd , wds , wdirs);
	}

	return;
}