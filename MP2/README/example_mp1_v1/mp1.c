#include "md5.h"
#include <openssl/md5.h>

#include <dirent.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

char kRecordingFile[] = ".loser_record";

typedef struct {
    uint32_t nCommit;
    uint32_t nFile;
    uint32_t nAdd;
    uint32_t nModify;
    uint32_t nCopy;
    uint32_t nDel;
    uint32_t commitSize;
} CommitHeader;

typedef struct {
    uint8_t nFileName;
    char fileName[256];
    uint8_t MD5[16];
} FileStruct;

int initOffsets(FILE *fp, int32_t *commitsOffset) {
    if (fp == NULL) return 0;
    int totleCommits = 0;
    commitsOffset[0] = 0;
    int32_t offset = 0;
    while (1) {
        CommitHeader commit;
        fseek(fp, offset, SEEK_SET);
        if (fread(&commit, sizeof(CommitHeader), 1, fp) != 1)
            break;
        offset += commit.commitSize;
        totleCommits++;
        commitsOffset[totleCommits] = offset;
    }
    return totleCommits;
}

CommitHeader getRecord(FILE *fp, int32_t *commitsOffset, int commit_id) {
    fseek(fp, commitsOffset[commit_id], SEEK_SET);
    CommitHeader commit;
    fread(&commit, sizeof(CommitHeader), 1, fp);
    return commit;
}

uint8_t getFileName(FILE *fp, char *fileName) {
    uint8_t nFileName;
    fread(&nFileName, sizeof(uint8_t), 1, fp);
    fread(fileName, sizeof(char), nFileName, fp);
    fileName[nFileName] = 0;
    return nFileName;
}

void getFileMd5(char *fileName, uint8_t *dest) {
    FILE *fp = fopen(fileName, "rb");
    fseek(fp, 0, SEEK_END);
    int fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    char *string = malloc(fsize);
    fread(string, fsize, 1, fp);
    fclose(fp);

    str_to_md5(string, fsize, dest);
}

int findFileByName(FileStruct *fileList, int len, char *targetName) {
    for (int i = 0; i < len; i++) {
        if (strcmp(fileList[i].fileName, targetName) == 0)
            return i;
    }
    return -1;
}

int findSameMd5(FileStruct *fileList, int len, uint8_t *MD5) {
    for (int i = 0; i < len; i++) {
        if (memcmp(fileList[i].MD5, MD5, 16) == 0)
            return i;
    }
    return -1;
}

FileStruct oldFiles[1000];
int matchedOldFiles[1000];

FileStruct allFiles[1000];
int addFiles[1000];
int modifyFiles[1000];
int copyFiles[1000];
int copyFrom[1000];

int main(int argc, char* argv[])
{
    int totleCommits;
    int commitsOffset[1005];

    if (strcmp(argv[1], "status") == 0 || strcmp(argv[1], "commit") == 0) {
        chdir(argv[2]);
        FILE *fp = fopen(kRecordingFile, "rb");
        totleCommits = initOffsets(fp, commitsOffset);

        uint32_t commitLen = 7*4;
        int nOldFiles = 0;
        if (totleCommits > 0) {
            CommitHeader lastCommit = getRecord(fp, commitsOffset, totleCommits-1);
            nOldFiles = lastCommit.nFile;
            int fileLogs = lastCommit.nAdd +
                lastCommit.nModify +
                lastCommit.nCopy*2 +
                lastCommit.nDel;
            for (int i = 0; i < fileLogs; i++) {
                char tmpBuf[256];
                getFileName(fp, tmpBuf);
            }
            for (int i = 0; i < nOldFiles; i++) {
                oldFiles[i].nFileName = getFileName(fp, oldFiles[i].fileName);
                fread(oldFiles[i].MD5, sizeof(uint8_t), 16, fp);
                matchedOldFiles[i] = 0;
                commitLen += 1 + oldFiles[i].nFileName;
            }
        }

        struct dirent **namelist;
        struct dirent *ent;
        uint32_t nAllFiles = 0;
        uint32_t nAddFiles = 0;
        uint32_t nModifyFiles = 0;
        uint32_t nCopyFiles = 0;
        uint32_t nDelFiles = nOldFiles;
        int n = scandir("./", &namelist, NULL, alphasort);
        for (int i = 0; i < n; i++) {
            ent = namelist[i];
            if (strcmp(ent->d_name, kRecordingFile) == 0)
                continue;
            if (strcmp(ent->d_name, ".") == 0)
                continue;
            if (strcmp(ent->d_name, "..") == 0)
                continue;
            uint8_t fileNameLen = strlen(ent->d_name);
            allFiles[nAllFiles].nFileName = fileNameLen;
            strcpy(allFiles[nAllFiles].fileName, ent->d_name);
            getFileMd5(ent->d_name, allFiles[nAllFiles].MD5);
            commitLen += 1 + fileNameLen + 16;

            int fileIndex = findFileByName(oldFiles, nOldFiles, ent->d_name);
            if (fileIndex >= 0) {
                if (memcmp(oldFiles[fileIndex].MD5,
                            allFiles[nAllFiles].MD5, 16) != 0) {
                    modifyFiles[nModifyFiles] = nAllFiles;
                    nModifyFiles++;
                    commitLen += 1 + fileNameLen;
                }
                matchedOldFiles[fileIndex] = 1;
                nDelFiles--;
                commitLen -= 1 + oldFiles[fileIndex].nFileName;
            } else {
                int sameMd5Index = findSameMd5(oldFiles, nOldFiles,
                        allFiles[nAllFiles].MD5);
                if (sameMd5Index >= 0) {
                    copyFiles[nCopyFiles] = nAllFiles;
                    copyFrom[nCopyFiles] = sameMd5Index;
                    nCopyFiles++;
                    commitLen += 1 + fileNameLen;
                    commitLen += 1 + oldFiles[sameMd5Index].nFileName;
                } else {
                    addFiles[nAddFiles] = nAllFiles;
                    nAddFiles++;
                    commitLen += 1 + fileNameLen;
                }
            }
            nAllFiles++;
        }
        if (strcmp(argv[1], "status") == 0) {
            printf("[new_file]\n");
            for (int i = 0; i < nAddFiles; i++)
                printf("%s\n", allFiles[addFiles[i]].fileName);
            printf("[modified]\n");
            for (int i = 0; i < nModifyFiles; i++)
                printf("%s\n", allFiles[modifyFiles[i]].fileName);
            printf("[copied]\n");
            for (int i = 0; i < nCopyFiles; i++) {
                printf("%s => %s\n",
                        oldFiles[copyFrom[i]].fileName,
                        allFiles[copyFiles[i]].fileName);
            }
            printf("[deleted]\n");
            for (int i = 0; i < nOldFiles; i++) {
                if (!matchedOldFiles[i]) {
                    printf("%s\n", oldFiles[i].fileName);
                }
            }
        } else {
            FILE *fRecord = fopen(kRecordingFile, "ab");
            CommitHeader newCommit = {
                .nCommit = totleCommits+1,
                .nFile = nAllFiles,
                .nAdd = nAddFiles,
                .nModify = nModifyFiles,
                .nCopy = nCopyFiles,
                .nDel = nDelFiles,
                .commitSize = commitLen};
            fwrite(&newCommit, sizeof(CommitHeader), 1, fRecord);
            for (int i = 0; i < nAddFiles; i++) {
                uint8_t fileNameLen = allFiles[addFiles[i]].nFileName;
                fwrite(&fileNameLen, sizeof(uint8_t), 1, fRecord);
                fwrite(allFiles[addFiles[i]].fileName, sizeof(char),
                       fileNameLen, fRecord);
            }
            for (int i = 0; i < nModifyFiles; i++) {
                uint8_t fileNameLen = allFiles[modifyFiles[i]].nFileName;
                fwrite(&fileNameLen, sizeof(uint8_t), 1, fRecord);
                fwrite(allFiles[modifyFiles[i]].fileName, sizeof(char),
                       fileNameLen, fRecord);
            }
            for (int i = 0; i < nCopyFiles; i++) {
                uint8_t fileNameLen = oldFiles[copyFrom[i]].nFileName;
                fwrite(&fileNameLen, sizeof(uint8_t), 1, fRecord);
                fwrite(oldFiles[copyFrom[i]].fileName, sizeof(char),
                       fileNameLen, fRecord);
                fileNameLen = allFiles[copyFiles[i]].nFileName;
                fwrite(&fileNameLen, sizeof(uint8_t), 1, fRecord);
                fwrite(allFiles[copyFiles[i]].fileName, sizeof(char),
                       fileNameLen, fRecord);
            }
            for (int i = 0; i < nOldFiles; i++) {
                if (!matchedOldFiles[i]) {
                    uint8_t fileNameLen = oldFiles[i].nFileName;
                    fwrite(&fileNameLen, sizeof(uint8_t), 1, fRecord);
                    fwrite(oldFiles[i].fileName, sizeof(char),
                           fileNameLen, fRecord);
                }
            }
            for (int i = 0; i < nAllFiles; i++) {
                uint8_t fileNameLen = allFiles[i].nFileName;
                fwrite(&fileNameLen, sizeof(uint8_t), 1, fRecord);
                fwrite(allFiles[i].fileName, sizeof(char),
                       fileNameLen, fRecord);
                fwrite(allFiles[i].MD5, sizeof(uint8_t), 16, fRecord);
            }
        }
    }
    else if (strcmp(argv[1], "log") == 0) {
        chdir(argv[3]);
        FILE *fp = fopen(kRecordingFile, "rb");
        totleCommits = initOffsets(fp, commitsOffset);

        int commitsToPrint = atoi(argv[2]);
        for (int i = 0; i < commitsToPrint && totleCommits > i; i++) {
            if (i > 0)
                printf("\n");
            CommitHeader commit = getRecord(fp, commitsOffset, totleCommits-i-1);
            printf("# commit %u\n", commit.nCommit);
            printf("[new_file]\n");
            for (int i = 0; i < commit.nAdd; i++) {
                char nameBuf[256];
                getFileName(fp, nameBuf);
                printf("%s\n", nameBuf);
            }
            printf("[modified]\n");
            for (int i = 0; i < commit.nModify; i++) {
                char nameBuf[256];
                getFileName(fp, nameBuf);
                printf("%s\n", nameBuf);
            }
            printf("[copied]\n");
            for (int i = 0; i < commit.nCopy; i++) {
                char nameBufFrom[256];
                char nameBufTo[256];
                getFileName(fp, nameBufFrom);
                getFileName(fp, nameBufTo);
                printf("%s => %s\n", nameBufFrom, nameBufTo);
            }
            printf("[deleted]\n");
            for (int i = 0; i < commit.nDel; i++) {
                char nameBuf[256];
                getFileName(fp, nameBuf);
                printf("%s\n", nameBuf);
            }
            printf("(MD5)\n");
            for (int i = 0; i < commit.nFile; i++) {
                char nameBuf[256];
                uint8_t MD5Buf[16];
                getFileName(fp, nameBuf);
                fread(MD5Buf, sizeof(uint8_t), 16, fp);
                printf("%s ", nameBuf);
                for (int j = 0; j < 16; j++) {
                    printf("%02x", MD5Buf[j]);
                }
                printf("\n");
            }
        }
    }
}
