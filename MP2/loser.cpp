#include <stdio.h>
#include <iostream>
#include <stdlib.h>
#include <dirent.h>
#include <stdint.h>
#include <string.h>
#include <string>
#include <vector>
#include <utility>
#include <algorithm>
#include <sys/time.h>
#include "loser.h"
#include "MD5.h"
using namespace std;

uint64_t get_timestamp()
{
	struct timeval tv;
	gettimeofday(&tv , NULL);
	return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

bool compare_commit(const Commit &commit_1 , const Commit &commit_2)
{
	return commit_1.timestamp < commit_2.timestamp;
}

bool compare_file(const File &file_1 , const File &file_2)
{
	return file_1.file_name < file_2.file_name;
}

bool compare_copied(const pair <string , string> &copied_1 , const pair <string , string> &copied_2)
{
	return (copied_1.first < copied_2.first) || (copied_1.first == copied_2.first && copied_1.second < copied_2.second);
}

void list_file(vector <string> &file_list)
{
	string file_name;
	struct dirent *entry;
	DIR *directory = opendir(".");

	while ((entry = readdir(directory)) != 0)
	{
		file_name = entry->d_name;
		if (file_name != "." && file_name != ".." && file_name != ".loser_record")
			file_list.push_back(file_name);
	}

	sort(file_list.begin() , file_list.end());

	closedir(directory);
	return;
}

void list_commit_size(FILE *loser_record , vector <uint32_t> &commit_size_list)
{
	uint32_t commit_size;

	fseek(loser_record , 0 , SEEK_SET);

	while (true)
	{
		fseek(loser_record , 6 * sizeof(uint32_t) , SEEK_CUR);
		if (!fread(&commit_size , sizeof(uint32_t) , 1 , loser_record))
			break;
		commit_size_list.push_back(commit_size);
		fseek(loser_record , commit_size - 7 * sizeof(uint32_t) , SEEK_CUR);
	}

	return;
}

uint32_t calculate_commit_size(Commit &commit)
{
	uint32_t commit_size = 7 * sizeof(uint32_t) + sizeof(uint64_t);

	for (uint32_t i = 0 ; i < commit.number_of_add ; i++)
		commit_size += sizeof(uint8_t) + strlen(commit.new_file[i].c_str()) * sizeof(char);

	for (uint32_t i = 0 ; i < commit.number_of_modify ; i++)
		commit_size += sizeof(uint8_t) + strlen(commit.modified[i].c_str()) * sizeof(char);

	for (uint32_t i = 0 ; i < commit.number_of_copy ; i++)
	{
		commit_size += sizeof(uint8_t) + strlen(commit.copied[i].first.c_str()) * sizeof(char);
		commit_size += sizeof(uint8_t) + strlen(commit.copied[i].first.c_str()) * sizeof(char);
	}

	for (uint32_t i = 0 ; i < commit.number_of_delete ; i++)
		commit_size += sizeof(uint8_t) + strlen(commit.deleted[i].c_str()) * sizeof(char);

	for (uint32_t i = 0 ; i < commit.number_of_file ; i++)
		commit_size += sizeof(uint8_t) + strlen(commit.file_list[i].c_str()) * sizeof(char) + LENGTH_OF_MD5 * sizeof(uint8_t);

	return commit_size;
}

void seek_commit(FILE *loser_record , uint32_t index)
{
	vector <uint32_t> commit_size_list;
	list_commit_size(loser_record , commit_size_list);
	uint32_t offset = 0;
	for (uint32_t i = 0 ; i < index - 1 ; i++)
		offset += commit_size_list[i];
	fseek(loser_record , offset , SEEK_SET);
	return;
}

uint32_t count_commit(FILE *loser_record)
{
	uint32_t number_of_commit = 0 , commit_size;

	fseek(loser_record , 0 , SEEK_SET);

	while (true)
	{
		fseek(loser_record , 6 * sizeof(uint32_t) , SEEK_CUR);
		if (!fread(&commit_size , sizeof(uint32_t) , 1 , loser_record))
			break;
		number_of_commit++;
		fseek(loser_record , commit_size - 7 * sizeof(uint32_t) , SEEK_CUR);
	}

	return number_of_commit;
}

void get_commit(FILE *loser_record , uint32_t index , Commit &commit)
{
	uint8_t file_name_size;
	char file_name_1[MAX_LENGTH_OF_FILE_NAME] = {0} , file_name_2[MAX_LENGTH_OF_FILE_NAME] = {0};
	uint8_t MD5[LENGTH_OF_MD5 + 1] = {0};

	seek_commit(loser_record , index);

	fread(&(commit.number_of_commit) , sizeof(uint32_t) , 1 , loser_record);
	fread(&(commit.number_of_file) , sizeof(uint32_t) , 1 , loser_record);
	fread(&(commit.number_of_add) , sizeof(uint32_t) , 1 , loser_record);
	fread(&(commit.number_of_modify) , sizeof(uint32_t) , 1 , loser_record);
	fread(&(commit.number_of_copy) , sizeof(uint32_t) , 1 , loser_record);
	fread(&(commit.number_of_delete) , sizeof(uint32_t) , 1 , loser_record);
	fread(&(commit.commit_size) , sizeof(uint32_t) , 1 , loser_record);

	for (uint32_t i = 0 ; i < commit.number_of_add ; i++)
	{
		memset(file_name_1 , 0 , MAX_LENGTH_OF_FILE_NAME);
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name_1 , sizeof(char) , file_name_size , loser_record);
		commit.new_file.push_back(string(file_name_1));
	}

	for (uint32_t i = 0 ; i < commit.number_of_modify ; i++)
	{
		memset(file_name_1 , 0 , MAX_LENGTH_OF_FILE_NAME);
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name_1 , sizeof(char) , file_name_size , loser_record);
		commit.modified.push_back(string(file_name_1));
	}

	for (uint32_t i = 0 ; i < commit.number_of_copy ; i++)
	{
		memset(file_name_1 , 0 , MAX_LENGTH_OF_FILE_NAME);
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name_1 , sizeof(char) , file_name_size , loser_record);
		memset(file_name_2 , 0 , MAX_LENGTH_OF_FILE_NAME);
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name_2 , sizeof(char) , file_name_size , loser_record);
		commit.copied.push_back(make_pair(string(file_name_1) , string(file_name_2)));
	}

	for (uint32_t i = 0 ; i < commit.number_of_delete ; i++)
	{
		memset(file_name_1 , 0 , MAX_LENGTH_OF_FILE_NAME);
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name_1 , sizeof(char) , file_name_size , loser_record);
		commit.deleted.push_back(string(file_name_1));
	}

	for (uint32_t i = 0 ; i < commit.number_of_file ; i++)
	{
		memset(file_name_1 , 0 , MAX_LENGTH_OF_FILE_NAME);
		fread(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fread(file_name_1 , sizeof(char) , file_name_size , loser_record);
		fread(MD5 , sizeof(uint8_t) , LENGTH_OF_MD5 , loser_record);
		commit.file_list.push_back(string(file_name_1));
		commit.MD5_list.push_back(string((char *)MD5));
	}

	fread(&(commit.timestamp) , sizeof(uint64_t) , 1 , loser_record);

	return;
}

void print_commit(Commit &commit)
{
	vector <string> tracked;

	cout << "# commit " << commit.number_of_commit << "\n";

	cout << "[new_file]\n";
	for (uint32_t i = 0 ; i < commit.number_of_add ; i++)
	{
		cout << commit.new_file[i] << "\n";
		tracked.push_back(commit.new_file[i]);
	}

	cout << "[modified]\n";
	for (uint32_t i = 0 ; i < commit.number_of_modify ; i++)
	{
		cout << commit.modified[i] << "\n";
		tracked.push_back(commit.modified[i]);
	}

	cout << "[copied]\n";
	for (uint32_t i = 0 ; i < commit.number_of_copy ; i++)
		cout << commit.copied[i].first << " => " << commit.copied[i].second << "\n";

	cout << "[deleted]\n";
	for (uint32_t i = 0 ; i < commit.number_of_delete ; i++)
		cout << commit.deleted[i] << "\n";

	cout << "(MD5)\n";
	for (uint32_t i = 0 ; i < commit.number_of_file ; i++)
		if (find(tracked.begin() , tracked.end() , commit.file_list[i]) != tracked.end())
		{
			cout << commit.file_list[i] << " ";
			uint8_t MD5[LENGTH_OF_MD5 + 1] = {0};
			strcpy((char *)MD5 , commit.MD5_list[i].c_str());
			for (uint8_t j = 0 ; j < LENGTH_OF_MD5 ; j++)
				printf("%02x" , MD5[j]);
			cout << "\n";
		}

	cout << "(timestamp)\n";
	cout << commit.timestamp << "\n";

	return;
}

void write_commit(Commit &commit)
{
	FILE *loser_record;
	uint32_t number_of_commit;
	uint8_t file_name_size;
	char file_name[MAX_LENGTH_OF_FILE_NAME] = {0};
	uint8_t MD5[LENGTH_OF_MD5 + 1] = {0};

	loser_record = fopen(".loser_record" , "ab");

	fwrite(&(commit.number_of_commit) , sizeof(uint32_t) , 1 , loser_record);
	fwrite(&(commit.number_of_file) , sizeof(uint32_t) , 1 , loser_record);
	fwrite(&(commit.number_of_add) , sizeof(uint32_t) , 1 , loser_record);
	fwrite(&(commit.number_of_modify) , sizeof(uint32_t) , 1 , loser_record);
	fwrite(&(commit.number_of_copy) , sizeof(uint32_t) , 1 , loser_record);
	fwrite(&(commit.number_of_delete) , sizeof(uint32_t) , 1 , loser_record);
	fwrite(&(commit.commit_size) , sizeof(uint32_t) , 1 , loser_record);

	for (uint32_t i = 0 ; i < commit.number_of_add ; i++)
	{
		strcpy(file_name , commit.new_file[i].c_str());
		file_name_size = strlen(file_name);
		fwrite(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fwrite(file_name , sizeof(char) , file_name_size , loser_record);
	}

	for (uint32_t i = 0 ; i < commit.number_of_modify ; i++)
	{
		strcpy(file_name , commit.modified[i].c_str());
		file_name_size = strlen(file_name);
		fwrite(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fwrite(file_name , sizeof(char) , file_name_size , loser_record);
	}

	for (uint32_t i = 0 ; i < commit.number_of_copy ; i++)
	{
		strcpy(file_name , commit.copied[i].first.c_str());
		file_name_size = strlen(file_name);
		fwrite(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fwrite(file_name , sizeof(char) , file_name_size , loser_record);

		strcpy(file_name , commit.copied[i].second.c_str());
		file_name_size = strlen(file_name);
		fwrite(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fwrite(file_name , sizeof(char) , file_name_size , loser_record);
	}

	for (uint32_t i = 0 ; i < commit.number_of_delete ; i++)
	{
		strcpy(file_name , commit.deleted[i].c_str());
		file_name_size = strlen(file_name);
		fwrite(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fwrite(file_name , sizeof(char) , file_name_size , loser_record);
	}

	for (uint32_t i = 0 ; i < commit.number_of_file ; i++)
	{
		strcpy(file_name , commit.file_list[i].c_str());
		file_name_size = strlen(file_name);
		fwrite(&file_name_size , sizeof(uint8_t) , 1 , loser_record);
		fwrite(file_name , sizeof(char) , file_name_size , loser_record);

		getFileMd5(file_name , (char *)MD5);
		fwrite(MD5 , sizeof(uint8_t) , LENGTH_OF_MD5 , loser_record);
	}

	fwrite(&(commit.timestamp) , sizeof(uint64_t) , 1 , loser_record);

	fclose(loser_record);
	return;
}

void classify(Commit &commit , Commit &previous_commit)
{
	for (uint32_t i = 0 ; i < commit.number_of_file ; i++)
	{
		string original_file_name;
		char MD5_1[LENGTH_OF_MD5 + 1] = {0} , MD5_2[LENGTH_OF_MD5 + 1] = {0};
		bool find_file_name = false , find_MD5 = false;

		getFileMd5(commit.file_list[i].c_str() , MD5_1);

		for (uint32_t j = 0 ; j < previous_commit.number_of_file && !find_file_name ; j++)
		{
			strcpy(MD5_2 , previous_commit.MD5_list[j].c_str());

			if (previous_commit.file_list[j] == commit.file_list[i])
			{
				find_file_name = true;
				find_MD5 = same_MD5(MD5_1 , MD5_2);
			}

			if (same_MD5(MD5_1 , MD5_2))
			{
				find_MD5 = true;

				if (original_file_name == "" || previous_commit.file_list[j] < original_file_name)
					original_file_name = previous_commit.file_list[j];
			}
		}

		if (find_file_name)
		{
			if (!find_MD5)
				commit.modified.push_back(commit.file_list[i]);
		}
		else
		{
			if (!find_MD5)
				commit.new_file.push_back(commit.file_list[i]);
			else
				commit.copied.push_back(make_pair(original_file_name , commit.file_list[i]));
		}
	}

	for (uint32_t i = 0 ; i < previous_commit.number_of_file ; i++)
		if (find(commit.file_list.begin() , commit.file_list.end() , previous_commit.file_list[i]) == commit.file_list.end())
			commit.deleted.push_back(previous_commit.file_list[i]);

	return;
}

void __status__()
{
	uint32_t number_of_commit;
	Commit commit , previous_commit;

	list_file(commit.file_list);
	commit.number_of_file = commit.file_list.size();

	FILE *loser_record = fopen(".loser_record" , "rb");

	if (!loser_record)
	{
		for (uint32_t i = 0 ; i < commit.number_of_file ; i++)
			commit.new_file.push_back(commit.file_list[i]);
	}
	else
	{
		number_of_commit = count_commit(loser_record);
		get_commit(loser_record , number_of_commit , previous_commit);
		classify(commit , previous_commit);

		fclose(loser_record);
	}

	sort(commit.new_file.begin() , commit.new_file.end());
	sort(commit.modified.begin() , commit.modified.end());
	sort(commit.copied.begin() , commit.copied.end() , compare_copied);
	sort(commit.deleted.begin() , commit.deleted.end());

	commit.number_of_add = commit.new_file.size();
	commit.number_of_modify = commit.modified.size();
	commit.number_of_copy = commit.copied.size();
	commit.number_of_delete = commit.deleted.size();

	cout << "[new_file]\n";
	for (uint32_t i = 0 ; i < commit.number_of_add ; i++)
		cout << commit.new_file[i] << "\n";

	cout << "[modified]\n";
	for (uint32_t i = 0 ; i < commit.number_of_modify ; i++)
		cout << commit.modified[i] << "\n";

	cout << "[copied]\n";
	for (uint32_t i = 0 ; i < commit.number_of_copy ; i++)
		cout << commit.copied[i].first << " => " << commit.copied[i].second << "\n";

	cout << "[deleted]\n";
	for (uint32_t i = 0 ; i < commit.number_of_delete ; i++)
		cout << commit.deleted[i] << "\n";

	return;
}

void __commit__()
{
	uint32_t number_of_commit = 0;
	Commit commit , previous_commit;

	list_file(commit.file_list);
	commit.number_of_file = commit.file_list.size();

	FILE *loser_record = fopen(".loser_record" , "rb");

	if (!loser_record)
	{
		for (uint32_t i = 0 ; i < commit.number_of_file ; i++)
			commit.new_file.push_back(commit.file_list[i]);
	}
	else
	{
		number_of_commit = count_commit(loser_record);
		get_commit(loser_record , number_of_commit , previous_commit);
		classify(commit , previous_commit);

		fclose(loser_record);
	}

	sort(commit.new_file.begin() , commit.new_file.end());
	sort(commit.modified.begin() , commit.modified.end());
	sort(commit.copied.begin() , commit.copied.end() , compare_copied);
	sort(commit.deleted.begin() , commit.deleted.end());

	commit.number_of_commit = number_of_commit + 1;
	commit.number_of_add = commit.new_file.size();
	commit.number_of_modify = commit.modified.size();
	commit.number_of_copy = commit.copied.size();
	commit.number_of_delete = commit.deleted.size();
	commit.commit_size = calculate_commit_size(commit);
	commit.timestamp = get_timestamp();

	write_commit(commit);

	return;
}

void __log__()
{
	FILE *loser_record = fopen(".loser_record" , "rb");

	if (!loser_record)
		return;

	uint32_t number_of_commit = count_commit(loser_record);
	for (uint32_t index = 1 ; index <= number_of_commit ; index++)
	{
		Commit commit;
		get_commit(loser_record , index , commit);
		print_commit(commit);
		if (index != number_of_commit)
			cout << "\n";
	}
	
	fclose(loser_record);
	return;
}

void __logs__(uint32_t number_of_peer)
{
	FILE *loser_record;
	uint32_t number_of_commit;
	vector <Commit> commit_list;

	if ((loser_record = fopen(".loser_record" , "rb")) != 0)
	{
		number_of_commit = count_commit(loser_record);
		for (uint32_t index = 1 ; index <= number_of_commit ; index++)
		{
			Commit commit;
			get_commit(loser_record , index , commit);
			commit_list.push_back(commit);
		}

		fclose(loser_record);
	}

	for (uint32_t i = 0 ; i < number_of_peer ; i++)
		if ((loser_record = fopen((".loser_record_" + to_string(i)).c_str() , "rb")) != 0)
		{
			number_of_commit = count_commit(loser_record);
			for (uint32_t index = 1 ; index <= number_of_commit ; index++)
			{
				Commit commit;
				get_commit(loser_record , index , commit);
				commit_list.push_back(commit);
			}

			fclose(loser_record);
		}

	sort(commit_list.begin() , commit_list.end() , compare_commit);

	number_of_commit = commit_list.size();
	for (uint32_t i = 0 ; i < number_of_commit ; i++)
	{
		commit_list[i].number_of_commit = i + 1;
		print_commit(commit_list[i]);
		if (i != number_of_commit - 1)
			cout << "\n";
	}

	return;
}

void __list__(uint32_t number_of_peer)
{
	FILE *loser_record;
	uint32_t number_of_commit , number_of_file;
	vector <Commit> commit_list;
	vector <File> logical_view;

	if ((loser_record = fopen(".loser_record" , "rb")) != 0)
	{
		number_of_commit = count_commit(loser_record);
		for (uint32_t index = 1 ; index <= number_of_commit ; index++)
		{
			Commit commit;
			get_commit(loser_record , index , commit);
			commit_list.push_back(commit);
		}

		fclose(loser_record);
	}

	for (uint32_t i = 0 ; i < number_of_peer ; i++)
		if ((loser_record = fopen((".loser_record_" + to_string(i)).c_str() , "rb")) != 0)
		{
			number_of_commit = count_commit(loser_record);
			for (uint32_t index = 1 ; index <= number_of_commit ; index++)
			{
				Commit commit;
				get_commit(loser_record , index , commit);
				commit_list.push_back(commit);
			}

			fclose(loser_record);
		}

	sort(commit_list.begin() , commit_list.end() , compare_commit);

	number_of_commit = commit_list.size();
	for (uint32_t i = 0 ; i < number_of_commit ; i++)
	{
		for (uint32_t j = 0 ; j < commit_list[i].number_of_add ; j++)
		{
			File file;

			file.file_name = commit_list[i].new_file[j];
			for (uint32_t k = 0 ; k < commit_list[i].number_of_file ; k++)
				if (commit_list[i].file_list[k] == file.file_name)
				{
					file.MD5 = commit_list[i].MD5_list[k];
					break;
				}

			logical_view.push_back(file);
		}

		for (uint32_t j = 0 ; j < commit_list[i].number_of_modify ; j++)
		{
			number_of_file = logical_view.size();
			for (uint32_t k = 0 ; k < number_of_file ; k++)
				if (logical_view[k].file_name == commit_list[i].modified[j])
				{
					for (uint32_t l = 0 ; l < commit_list[i].number_of_file ; l++)
						if (commit_list[i].file_list[l] == logical_view[k].file_name)
						{
							logical_view[k].MD5 = commit_list[i].MD5_list[l];
							break;
						}

					break;
				}
		}

		for (uint32_t j = 0 ; j < commit_list[i].number_of_copy ; j++)
		{
			File file;

			file.file_name = commit_list[i].copied[j].second;
			for (uint32_t k = 0 ; k < commit_list[i].number_of_file ; k++)
				if (commit_list[i].file_list[k] == file.file_name)
				{
					file.MD5 = commit_list[i].MD5_list[k];
					break;
				}

			logical_view.push_back(file);
		}

		for (uint32_t j = 0 ; j < commit_list[i].number_of_delete ; j++)
		{
			number_of_file = logical_view.size();
			for (uint32_t k = 0 ; k < number_of_file ; k++)
				if (logical_view[k].file_name == commit_list[i].deleted[j])
				{
					logical_view.erase(logical_view.begin() + k);
					break;
				}
		}
	}

	sort(logical_view.begin() , logical_view.end() , compare_file);

	number_of_file = logical_view.size();
	for (uint32_t i = 0 ; i < number_of_file ; i++)
		cout << logical_view[i].file_name << "\n";
	cout << "(MD5)\n";
	for (uint32_t i = 0 ; i < number_of_file ; i++)
	{
		cout << logical_view[i].file_name << " ";
		uint8_t MD5[LENGTH_OF_MD5 + 1] = {0};
		strcpy((char *)MD5 , logical_view[i].MD5.c_str());
		for (uint8_t j = 0 ; j < LENGTH_OF_MD5 ; j++)
			printf("%02x" , MD5[j]);
		cout << "\n";
	}

	return;
}
