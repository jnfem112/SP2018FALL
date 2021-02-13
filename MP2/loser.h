#ifndef LOSER_H
#define LOSER_H

#include <stdint.h>
#include <string>
#include <vector>
#include <utility>
using namespace std;

#define MAX_LENGTH_OF_FILE_NAME 256

struct Commit
{
	uint32_t number_of_commit , number_of_file , number_of_add , number_of_modify , number_of_copy , number_of_delete , commit_size;
	vector <string> new_file , modified , deleted;
	vector <pair <string , string> > copied;
	vector <string> file_list , MD5_list;
	uint64_t timestamp;
};

struct File
{
	string file_name , MD5;
};

typedef struct Commit Commit;
typedef struct File File;

uint64_t get_timestamp();
bool compare_commit(const Commit &commit_1 , const Commit &commit_2);
bool compare_file(const File &file_1 , const File &file_2);
bool compare_copied(const pair <string , string> &copied_1 , const pair <string , string> &copied_2);
void list_file(vector <string> &file_list);
void list_commit_size(FILE *loser_record , vector <uint32_t> &commit_size_list);
uint32_t calculate_commit_size(Commit &commit);
void seek_commit(FILE *loser_record , uint32_t index);
uint32_t count_commit(FILE *loser_record);
void get_commit(FILE *loser_record , uint32_t index , Commit &commit);
void print_commit(Commit &commit);
void write_commit(Commit &commit);
void classify(Commit &commit , Commit &previous_commit);
void __status__();
void __commit__();
void __log__();
void __logs__(uint32_t number_of_peer);
void __list__(uint32_t number_of_peer);

#endif
