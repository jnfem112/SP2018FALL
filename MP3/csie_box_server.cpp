#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <string>
#include <queue>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/inotify.h>
#include "csie_box.h"
using namespace std;

int status = 0;
Config config;
int fifo_server_to_client = -1 , fifo_client_to_server = -1;

void signal_handler(int signal)
{
	if (signal == SIGINT)
	{
		if (status != 0)
		{
			status = 3;
		}
		else
		{
			// cout << "(server) close fifo\n";
			if (fifo_server_to_client != -1)
				close(fifo_server_to_client);
			if (fifo_client_to_server != -1)
				close(fifo_client_to_server);

			// cout << "(server) remove fifo\n";
			remove_fifo(config);

			exit(EXIT_SUCCESS);
		}
	}

	return;
}

int main(int argc , char **argv)
{
	struct sigaction action;
	action.sa_flags = 0;
	action.sa_handler = signal_handler;
	sigaction(SIGINT , &action , NULL);

	// cout << "(server) read config\n";
	if (!read_config(argv[1] , config))
		return EXIT_FAILURE;

	// cout << "(server) create fifo\n";
	if (!create_fifo(config))
		return EXIT_FAILURE;

	// cout << "(server) open fifo\n";
	fifo_server_to_client = open((config.fifo_path + "/server_to_client.fifo").c_str() , O_WRONLY);
	fifo_client_to_server = open((config.fifo_path + "/client_to_server.fifo").c_str() , O_RDONLY);

	int max_fd = sysconf(_SC_OPEN_MAX);
	fd_set read_set , write_set;
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_SET(fifo_server_to_client , &write_set);
	FD_SET(fifo_client_to_server , &read_set);

	int inotify_fd = -1 , wd;
	struct inotify_event *wevent;
	vector <int> wds;
	vector <string> wdirs;

	int finish = 1;
	Event event;
	queue <Event> event_list;
	string event_type , file_type , file_name;
	int byte;
	char buffer_1[BUFFER_SIZE + 1] = {0} , buffer_2[BUFFER_SIZE + 1] = {0};
	int in_file = -1 , out_file = -1;

	list_directory(config.directory_name , 0 , event_list);

	while (true)
	{
		if (status == 3 || status == 7)
		{
			if (FD_ISSET(inotify_fd , &read_set))
			{
				FD_CLR(inotify_fd , &read_set);

				while (!event_list.empty())
					event_list.pop();

				// cout << "(server) ignore all : " << config.directory_name << "\n";
				ignore_all_directory(config , inotify_fd , wds , wdirs , 0);
			}

			if (in_file == -1)
				status++;
		}

		if (status == 6)
			break;

		fd_set working_read_set , working_write_set;
		memcpy(&working_read_set , &read_set , sizeof(fd_set));
		memcpy(&working_write_set , &write_set , sizeof(fd_set));

		if (select(max_fd , &working_read_set , &working_write_set , NULL , NULL) == 0)
			continue;

		if (FD_ISSET(fifo_client_to_server , &working_read_set))
		{
			memset(buffer_1 , 0 , BUFFER_SIZE);
			read(fifo_client_to_server , buffer_1 , BUFFER_SIZE);

			if (strcmp(buffer_1 , "create") == 0 || strcmp(buffer_1 , "modify") == 0)
			{
				// cout << "(server) recv : " << buffer_1 << "\n";

				memset(buffer_1 , 0 , BUFFER_SIZE);
				read(fifo_client_to_server , buffer_1 , BUFFER_SIZE);
				// cout << "(server) recv : " << buffer_1 << "\n";
				file_name = config.directory_name + "/" + buffer_1;

				memset(buffer_1 , 0 , BUFFER_SIZE);
				read(fifo_client_to_server , buffer_1 , BUFFER_SIZE);
				// cout << "(server) recv : " << buffer_1 << "\n";
				file_type = buffer_1;

				if (file_type == "file")
				{
					out_file = open(file_name.c_str() , O_CREAT | O_TRUNC | O_WRONLY | O_SYNC , 0664);
					finish = 0;
				}
				else if (file_type == "link")
				{
					memset(buffer_1 , 0 , BUFFER_SIZE);
					read(fifo_client_to_server , buffer_1 , BUFFER_SIZE);
					symlink(buffer_1 , file_name.c_str());
					finish = 1;
				}
				else if (file_type == "directory")
				{
					mkdir(file_name.c_str() , 0755);

					// cout << "(server) watch : " << file_name << "\n";
					watch_directory(file_name , inotify_fd , wds , wdirs);

					finish = 1;
				}
			}
			else if (strcmp(buffer_1 , "delete") == 0)
			{
				// cout << "(server) recv : " << buffer_1 << "\n";

				memset(buffer_1 , 0 , BUFFER_SIZE);
				read(fifo_client_to_server , buffer_1 , BUFFER_SIZE);
				// cout << "(server) recv : " << buffer_1 << "\n";
				file_name = config.directory_name + "/" + buffer_1;

				memset(buffer_1 , 0 , BUFFER_SIZE);
				read(fifo_client_to_server , buffer_1 , BUFFER_SIZE);
				// cout << "(server) recv : " << buffer_1 << "\n";
				file_type = buffer_1;

				if (file_type == "file")
				{
					remove(file_name.c_str());
				}
				else if (file_type == "link")
				{
					unlink(file_name.c_str());
				}
				else if (file_type == "directory")
				{
					// cout << "(server) ignore : " << file_name << "\n";
					ignore_directory(file_name , inotify_fd , wds , wdirs);

					empty_directory(file_name , 1);
				}

				finish = 1;
			}
			else if (strcmp(buffer_1 , "exit") == 0)
			{
				// cout << "(server) recv : " << buffer_1 << "\n";
				status = 7;
			}
			else if (strcmp(buffer_1 , "success") == 0)
			{
				// cout << "(server) recv : " << buffer_1 << "\n";
				status = 6;
			}
			else
			{
				byte = atoi(buffer_1);

				if (byte != 0)
				{
					memset(buffer_1 , 0 , BUFFER_SIZE);
					read(fifo_client_to_server , buffer_1 , BUFFER_SIZE);
					write(out_file , buffer_1 , byte);
				}
				else
				{
					close(out_file);
					finish = 1;
				}
			}
		}

		if (FD_ISSET(fifo_server_to_client , &working_write_set))
		{
			if (status == 0)
			{
				strcpy(buffer_1 , "sync start");
				write(fifo_server_to_client , buffer_1 , BUFFER_SIZE);
				// cout << "(server) send : " << buffer_1 << "\n";
				status = 1;
			}
			else if (status == 4)
			{
				strcpy(buffer_1 , "exit");
				write(fifo_server_to_client , buffer_1 , BUFFER_SIZE);
				// cout << "(server) send : " << buffer_1 << "\n";
				status = 5;
			}
			else if (status == 8)
			{
				status = 0;

				strcpy(buffer_1 , "success");
				write(fifo_server_to_client , buffer_1 , BUFFER_SIZE);
				// cout << "(server) send : " << buffer_1 << "\n";

				// cout << "(server) close fifo\n";
				close(fifo_server_to_client);
				close(fifo_client_to_server);
				FD_CLR(fifo_server_to_client , &write_set);
				FD_CLR(fifo_client_to_server , &read_set);
				fifo_server_to_client = -1;
				fifo_client_to_server = -1;
				// cout << "(server) remove fifo\n";
				remove_fifo(config);

				// cout << "(server) create fifo\n";
				if (!create_fifo(config))
					return EXIT_FAILURE;
				// cout << "(server) open fifo\n";
				fifo_server_to_client = open((config.fifo_path + "/server_to_client.fifo").c_str() , O_WRONLY);
				fifo_client_to_server = open((config.fifo_path + "/client_to_server.fifo").c_str() , O_RDONLY);
				FD_SET(fifo_server_to_client , &write_set);
				FD_SET(fifo_client_to_server , &read_set);

				list_directory(config.directory_name , 0 , event_list);
			}
			else
			{
				if (in_file == -1)
				{
					if (event_list.size() != 0)
					{
						event = event_list.front();
						event_list.pop();

						if (status == 1)
							strcpy(buffer_1 , "sync");
						else if (status == 2)
							strcpy(buffer_1 , event.event_type.c_str());
						write(fifo_server_to_client , buffer_1 , BUFFER_SIZE);
						// cout << "(server) send : " << buffer_1 << "\n";

						strcpy(buffer_1 , event.file_name.substr(config.directory_name.size() + 1).c_str());
						write(fifo_server_to_client , buffer_1 , BUFFER_SIZE);
						// cout << "(server) send : " << buffer_1 << "\n";

						strcpy(buffer_1 , event.file_type.c_str());
						write(fifo_server_to_client , buffer_1 , BUFFER_SIZE);
						// cout << "(server) send : " << buffer_1 << "\n";

						if (event.file_type == "file")
						{
							in_file = open(event.file_name.c_str() , O_RDONLY);
						}
						else if (event.file_type == "link")
						{
							readlink(event.file_name.c_str() , buffer_1 , BUFFER_SIZE);
							write(fifo_server_to_client , buffer_1 , BUFFER_SIZE);
						}
					}
					else
					{ 
						if (status == 1)
						{
							strcpy(buffer_1 , "sync end");
							write(fifo_server_to_client , buffer_1 , BUFFER_SIZE);
							// cout << "(server) send : " << buffer_1 << "\n";
							status = 2;

							inotify_fd = inotify_init();
							FD_SET(inotify_fd , &read_set);

							// cout << "(server) watch all : " << config.directory_name << "\n";
							watch_all_directory(config , inotify_fd , wds , wdirs , 1);
						}
					}
				}
				else
				{
					memset(buffer_2 , 0 , BUFFER_SIZE);
					byte = read(in_file , buffer_2 , BUFFER_SIZE);
					sprintf(buffer_1 , "%d" , byte);

					write(fifo_server_to_client , buffer_1 , BUFFER_SIZE);

					if (byte > 0)
					{
						write(fifo_server_to_client , buffer_2 , BUFFER_SIZE);
					}
					else
					{
						close(in_file);
						in_file = -1;
					}
				}
			}
		}

		if (FD_ISSET(inotify_fd , &working_read_set))
		{
			memset(buffer_1 , 0 , BUFFER_SIZE);
			byte = read(inotify_fd , buffer_1 , BUFFER_SIZE);
			for (int offset = 0 ; offset < byte ; offset += sizeof(struct inotify_event))
			{
				wevent = (struct inotify_event *)(buffer_1 + offset);

				if (wevent->len == 0)
					continue;

				int number_of_directory = wds.size();
				for (int i = 0 ; i < number_of_directory ; i++)
					if (wds[i] == wevent->wd)
					{
						event.file_name = wdirs[i] + "/" + wevent->name;
						break;
					}

				if (event.file_name == file_name)
					continue;

				if (wevent->mask & IN_CREATE)
				{
					// cout << "(server) create : " << event.file_name << "\n";
					event.event_type = "create";

				}
				if (wevent->mask & IN_MODIFY)
				{
					// cout << "(server) modify : " << event.file_name << "\n";
					event.event_type = "modify";
				}
				if (wevent->mask & IN_DELETE)
				{
					// cout << "(server) delete : " << event.file_name << "\n";
					event.event_type = "delete";
				}

				struct stat file_status;
				lstat(event.file_name.c_str() , &file_status);
				if (S_ISREG(file_status.st_mode))
				{
					event.file_type = "file";
				}
				else if (S_ISLNK(file_status.st_mode))
				{
					event.file_type = "link";
				}
				else if (S_ISDIR(file_status.st_mode))
				{
					// cout << "(server) " << event.file_name << " is a directory\n";
					event.file_type = "directory";
					// cout << "(server) watch : " << event.file_name << "\n";
					watch_directory(event.file_name , inotify_fd , wds , wdirs);
				}

				event_list.push(event);
			}
		}

		if (finish)
			file_name = "";
	}

	// cout << "(server) close fifo\n";
	close(fifo_server_to_client);
	close(fifo_client_to_server);

	// cout << "(server) remove fifo\n";
	remove_fifo(config);

	return EXIT_SUCCESS;
}