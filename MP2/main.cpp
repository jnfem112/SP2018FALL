#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <vector>
#include <sys/select.h>
#include <sys/socket.h>
#include "loserFS.h"
#include "loser.h"
using namespace std;

int main(int argc , char **argv)
{
	Config config;

	// cout << "read config\n";
	if (!read_config(argv[1] , config))
		return EXIT_FAILURE;

	// cout << "change directory\n";
	chdir(config.repo.c_str());

	bool finish = false;
	string command;
	vector <string> arguments(2);
	string source , destination;
	vector <string> client_command(0) , client_source(0);
	int status = 0;
	vector <int> peer_status(config.number_of_peer , 0) , client_status(0);
	int byte;
	char buffer_1[BUFFER_SIZE + 1] = {0} , buffer_2[BUFFER_SIZE + 1] = {0};
	int target;
	vector <FILE *> in_file(0) , out_file(config.number_of_peer);

	int server_socket;
	vector <int> peer_socket(config.number_of_peer) , client_socket(0);
	vector <bool> connected(config.number_of_peer , false);

	// cout << "create server socket\n";
	if (!create_socket(config , server_socket , peer_socket))
		return EXIT_FAILURE;

	int max_fd = sysconf(_SC_OPEN_MAX);
	fd_set read_set , write_set;
	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_SET(STDIN_FILENO , &read_set);
	FD_SET(server_socket , &read_set);

	while (!finish)
	{
		for (int i = 0 ; i < config.number_of_peer ; i++)
			if (!connected[i] && connect_socket(config , peer_socket , i))
			{
				// cout << "connect peer socket\n";
				connected[i] = true;
				FD_SET(peer_socket[i] , &read_set);
				FD_SET(peer_socket[i] , &write_set);
			}

		fd_set working_read_set , working_write_set;
		memcpy(&working_read_set , &read_set , sizeof(fd_set));
		memcpy(&working_write_set , &write_set , sizeof(fd_set));

		if (select(max_fd , &working_read_set , &working_write_set , NULL , NULL) == 0)
			continue;

		if (FD_ISSET(STDIN_FILENO , &working_read_set))
		{
			read_command(command , arguments);
			status = 1;
		}

		if (FD_ISSET(server_socket , &working_read_set))
		{
			int fd = accept(server_socket , NULL , NULL);
			// cout << "accept client socket\n";
			client_socket.push_back(fd);
			client_command.push_back("");
			client_source.push_back("");
			client_status.push_back(0);
			in_file.push_back(0);
			FD_SET(fd , &read_set);
			FD_SET(fd , &write_set);
		}

		for (int i = 0 ; i < config.number_of_peer ; i++)
		{
			if (FD_ISSET(peer_socket[i] , &working_read_set))
			{
				recv(peer_socket[i] , buffer_1 , BUFFER_SIZE , 0);

				if (strcmp(buffer_1 , "success") == 0)
				{
					if (command == "list" || command == "history")
					{
						peer_status[i] = 3;
						out_file[i] = fopen((".loser_record_" + to_string(i)).c_str() , "wb");
					}
					else if (command == "cp" || command == "mv")
					{
						// cout << "find : " << config.peers[i] << "\n";
						status = 3;
						peer_status[i] = 3;
						target = peer_socket[i];
						if (destination[0] == '@')
							out_file[0] = fopen(destination.substr(1).c_str() , "wb");
						else
							out_file[0] = fopen(destination.c_str() , "wb");
					}
					else if (command == "rm")
					{
						// cout << "success\n";
						status = 0;
					}
					else if (command == "exit")
					{
						FD_CLR(peer_socket[i] , &read_set);
						FD_CLR(peer_socket[i] , &write_set);
						connected[i] = false;
						peer_status[i] = 2;
					}
				}
				else if (strcmp(buffer_1 , "fail") == 0)
				{
					peer_status[i] = 2;
				}
				else
				{
					if (command == "list" || command == "history")
					{
						byte = atoi(buffer_1);

						if (byte != 0)
						{
							recv(peer_socket[i] , buffer_1 , BUFFER_SIZE , 0);
							fwrite(buffer_1 , sizeof(char) , byte , out_file[i]);
						}
						else
						{
							fclose(out_file[i]);
							// cout << "receive .loser_record from " << config.peers[i] << "\n";
							peer_status[i] = 2;
						}
					}
					else if (peer_socket[i] == target)
					{
						byte = atoi(buffer_1);

						if (byte != 0)
						{
							recv(peer_socket[i] , buffer_1 , BUFFER_SIZE , 0);
							fwrite(buffer_1 , sizeof(char) , byte , out_file[0]);
						}
						else
						{
							fclose(out_file[0]);
							cout << "success\n";

							if (destination[0] == '@')
							{
								// cout << "commit\n";
								__commit__();
							}

							status = 0;
						}
					}
				}
			}

			if (FD_ISSET(peer_socket[i] , &working_write_set))
			{
				if (status == 2 && peer_status[i] == 0)
				{
					// cout << "to : " << config.peers[i] << "\n";

					strcpy(buffer_1 , command.c_str());
					send(peer_socket[i] , buffer_1 , BUFFER_SIZE , 0);
					// cout << "send : " << buffer_1 << "\n";

					if (command == "cp" || command == "mv" || command == "rm")
					{
						strcpy(buffer_1 , source.c_str());
						send(peer_socket[i] , buffer_1 , BUFFER_SIZE , 0);
						// cout << "send : " << buffer_1 << "\n";
					}
					else if (command == "exit")
					{
						strcpy(buffer_1 , config.name.c_str());
						send(peer_socket[i] , buffer_1 , BUFFER_SIZE , 0);
						// cout << "send : " << buffer_1 << "\n";
					}

					peer_status[i] = 1;
				}
			}
		}

		int number_of_client = client_socket.size();
		for (int i = 0 ; i < number_of_client ; i++)
		{
			if (FD_ISSET(client_socket[i] , &working_read_set))
			{
				recv(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);
				// cout << "recv : " << buffer_1 << "\n";
				client_command[i] = buffer_1;

				if (client_command[i] == "list" || client_command[i] == "history")
				{
					client_source[i] = ".loser_record";
				}
				else if (client_command[i] == "cp" || client_command[i] == "mv" || client_command[i] == "rm" || client_command[i] == "exit")
				{
					recv(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);
					// cout << "recv : " << buffer_1 << "\n";
					client_source[i] = buffer_1;
				}

				client_status[i] = 1;
			}

			if (FD_ISSET(client_socket[i] , &working_write_set))
			{
				if ((client_command[i] == "list" || client_command[i] == "history" || client_command[i] == "cp" || client_command[i] == "mv") && client_status[i] == 1)
				{
					if (check_file_exist(client_source[i]))
					{
						// cout << client_source[i] << " exist\n";
						strcpy(buffer_1 , "success");
						send(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);
						client_status[i] = 2;
						if (client_source[i][0] == '@')
							in_file[i] = fopen(client_source[i].substr(1).c_str() , "rb");
						else
							in_file[i] = fopen(client_source[i].c_str() , "rb");
					}
					else
					{
						// cout << client_source[i] << " does not exist\n";
						strcpy(buffer_1 , "fail");
						send(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);
						client_status[i] = 0;
					}
				}
				else if ((client_command[i] == "list" || client_command[i] == "history" || client_command[i] == "cp" || client_command[i] == "mv") && client_status[i] == 2)
				{
					byte = fread(buffer_2 , sizeof(char) , BUFFER_SIZE , in_file[i]);
					sprintf(buffer_1 , "%d" , byte);
					send(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);

					if (byte > 0)
					{
						send(client_socket[i] , buffer_2 , BUFFER_SIZE , 0);
					}
					else
					{
						fclose(in_file[i]);

						if (client_command[i] == "mv")
						{
							remove(client_source[i].substr(1).c_str());

							// cout << "commit\n";
							__commit__();
						}

						client_status[i] = 0;
					}
				}
				else if (client_command[i] == "rm" && client_status[i] == 1)
				{
					if (check_file_exist(client_source[i]))
					{
						// cout << client_source[i] << " exist\n";
						remove(client_source[i].substr(1).c_str());

						// cout << "commit\n";
						__commit__();

						strcpy(buffer_1 , "success");
					}
					else
					{
						// cout << client_source[i] << " does not exist\n";

						strcpy(buffer_1 , "fail");
					}

					send(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);
					client_status[i] = 0;
				}
				else if (client_command[i] == "exit" && client_status[i] == 1)
				{
					strcpy(buffer_1 , "success");
					send(client_socket[i] , buffer_1 , BUFFER_SIZE , 0);

					// cout << "close client socket\n";
					FD_CLR(client_socket[i] , &read_set);
					FD_CLR(client_socket[i] , &write_set);
					close(client_socket[i]);
					client_socket[i] = -1;

					for (int j = 0 ; j < config.number_of_peer ; j++)
						if (config.peers[j] == client_source[i])
						{
							FD_CLR(peer_socket[j] , &read_set);
							FD_CLR(peer_socket[j] , &write_set);
							// cout << "reset peer socket\n";
							while (check_file_exist(config.peers[j]));
							close(peer_socket[j]);
							peer_socket[j] = socket(AF_UNIX , SOCK_STREAM , 0);
							connected[j] = false;
						}

					client_status[i] = 0;
				}
			}
		}

		vector <int> temp_client_socket , temp_client_status;
		vector <string> temp_client_command , temp_client_source;

		for (int i = 0 ; i < number_of_client ; i++)
			if (client_socket[i] != -1)
			{
				temp_client_socket.push_back(client_socket[i]);
				temp_client_command.push_back(client_command[i]);
				temp_client_source.push_back(client_source[i]);
				temp_client_status.push_back(client_status[i]);
			}

		client_socket = temp_client_socket;
		client_command = temp_client_command;
		client_source = temp_client_source;
		client_status = temp_client_status;

		if ((command == "list" || command == "history") && status == 1)
		{
			if (command == "history" && arguments.size() == 0)
			{
				__log__();
				status = 0;
			}
			else
			{
				// cout << "collect .loser_record\n";
				status = 2;
				for (int i = 0 ; i < config.number_of_peer ; i++)
					peer_status[i] = 0;
			}
		}
		if ((command == "list" || command == "history") && status == 2)
		{
			status = 3;
			for (int i = 0 ; i < config.number_of_peer && status == 3 ; i++)
				if (connected[i] && peer_status[i] != 2)
					status = 2;
		}
		if ((command == "list" || command == "history") && status == 3)
		{
			if (command == "list")
				__list__(config.number_of_peer);
			else
				__logs__(config.number_of_peer);

			for (int i = 0 ; i < config.number_of_peer ; i++)
				remove((".loser_record_" + to_string(i)).c_str());

			status = 0;
		}
		if ((command == "cp" || command == "mv") && status == 1)
		{
			source = arguments[0];
			destination = arguments[1];

			int ret = simple_copy(source , destination);
			if (ret == 0)
			{
				cout << "fail\n";
				status = 0;
			}
			else if (ret == 1)
			{
				if (command == "mv")
				{
					if (source[0] == '@')
						remove(source.substr(1).c_str());
					else
						remove(source.c_str());
				}

				cout << "success\n";

				if (destination[0] == '@' || (command == "mv" && source[0] == '@'))
				{
					// cout << "commit\n";
					__commit__();
				}

				status = 0;
			}
			else
			{
				// cout << "start remote transfer\n";
				status = 2;
				for (int i = 0 ; i < config.number_of_peer ; i++)
					peer_status[i] = 0;
			}
		}
		if ((command == "cp" || command == "mv") && status == 2)
		{
			bool fail = true;
			for (int i = 0 ; i < config.number_of_peer && fail ; i++)
				if (connected[i] && peer_status[i] != 2)
					fail = false;

			if (fail)
			{
				cout << "fail\n";
				status = 0;
			}
		}
		if (command == "rm" && status == 1)
		{
			source = arguments[0];

			if (check_file_exist(source))
			{
				remove(source.substr(1).c_str());
				// cout << "success\n";

				// cout << "commit\n";
				__commit__();

				status = 0;
			}
			else
			{
				status = 2;
				for (int i = 0 ; i < config.number_of_peer ; i++)
					peer_status[i] = 0;
			}
		}
		if (command == "rm" && status == 2)
		{
			bool fail = true;
			for (int i = 0 ; i < config.number_of_peer && fail ; i++)
				if (connected[i] && peer_status[i] != 2)
					fail = false;

			if (fail)
			{
				// cout << "fail\n";
				status = 0;
			}
		}
		if (command == "exit" && status == 1)
		{
			status = 2;
			for (int i = 0 ; i < config.number_of_peer ; i++)
				peer_status[i] = 0;
		}
		if (command == "exit" && status == 2)
		{
			finish = true;
			for (int i = 0 ; i < config.number_of_peer && finish ; i++)
				if (connected[i] && peer_status[i] != 2)
					finish = false;
		}
	}

	// cout << "close all socket\n";
	close_socket(config , server_socket , peer_socket , client_socket);
	cout << "bye\n";
	return EXIT_SUCCESS;
}
