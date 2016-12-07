#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

#include "fsmb_smallfile.h"
#include "time_util.h"

#define BUF_SIZE 1024*1024 // 1mb
#define NAME_BUF_SIZE 1024
#define FILE_COUNT 10000 // 10,000

/*
 * Design
 * 1. Create/open 10,000 files, write 1 block and close
 * 2. Open and read files.
 * 3. Delete files.
 *
 */

void fsmb_smallfile_benchmark(long block_size, int count){
	HANDLE file_handle;
	BOOL error_flag;
	DWORD dwRetBytes = 0;
	char * buf;
    TCHAR filepath_r[NAME_BUF_SIZE];
    double sec;

	buf = (char*)malloc(sizeof(char)*BUF_SIZE);

    printf("micro benchmarking for lots of small files RW\n");
	printf("Opn/Wrt | Opn/Rd | Del\n");

	for (int cnt = 0; cnt < count; cnt++) {
		/*** First phase ***/
		TIMER_START();
		for (int i = 0; i < FILE_COUNT; i++) {
			//TODO: check sprintf
			swprintf(filepath_r, NAME_BUF_SIZE, TEXT("small_%d_%d"), cnt, i);

			// Create file
			file_handle = CreateFile(filepath_r,	// filename
				GENERIC_WRITE,					// write only
				0,								// dont share
				NULL,							// default security
				CREATE_ALWAYS,					// always create new file
				FILE_ATTRIBUTE_NORMAL,			// normal file
				NULL);							// no attr.

			if (file_handle == INVALID_HANDLE_VALUE) {
				printf("Failed to create file\n");
				exit(1);
			}

			// Write data
			error_flag = WriteFile(file_handle,	// file handle to write
				buf,							// write buffer
				block_size,						// how many bytes
				&dwRetBytes,				// number of bytes
				NULL);							// no overlapped structrue

			if (error_flag == FALSE || dwRetBytes != block_size) {
				printf("Failed to write a block\n");
				exit(1);
			}

			// Close file
			CloseHandle(file_handle);
		}
		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		printf("%f \t", sec);

		/*** Second phase ***/
		TIMER_START();
		for (int i = 0; i < FILE_COUNT; i++) {
			swprintf(filepath_r, NAME_BUF_SIZE, TEXT("small_%d_%d"), cnt, i);

			// Create file
			file_handle = CreateFile(filepath_r,	// filename
				GENERIC_READ,					// read only
				0,								// dont share
				NULL,							// default security
				OPEN_EXISTING,					// only use existsing file
				FILE_ATTRIBUTE_NORMAL,			// normal file
				NULL);							// no attr.

			if (file_handle == INVALID_HANDLE_VALUE) {
				printf("Failed to open file\n");
				exit(1);
			}

			// Read data
			error_flag = ReadFile(file_handle,	// file handle to write
				buf,							// read buffer
				block_size,						// how many bytes
				&dwRetBytes,				// number of bytes
				NULL);							// no overlapped structrue

			if (error_flag == FALSE || dwRetBytes != block_size) {
				printf("Failed to read a block\n");
				exit(1);
			}

			// Close file
			CloseHandle(file_handle);
		}
		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		printf("%f \t", sec);

		/*** Third phase ***/
		TIMER_START();
		for (int i = 0; i < FILE_COUNT; i++) {
			swprintf(filepath_r, NAME_BUF_SIZE, TEXT("small_%d_%d"), cnt, i);

			error_flag = DeleteFile(filepath_r);
			if (error_flag == FALSE) {
				printf("Failed to delete file\n");
				exit(1);
			}
		}
		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		printf("%f\n", sec);
	}

	free(buf);
}
