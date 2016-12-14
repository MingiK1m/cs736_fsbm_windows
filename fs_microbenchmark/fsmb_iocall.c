#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include "fsmb_iocall.h"
#include "time_util.h"

#define FILENAME TEXT("testfile")
#define BUF_SIZE 1024*1024

void fsmb_io_system_call_benchmark(int block_size, int iter_count) {
	HANDLE file_handle;
	BOOL error_flag;
	DWORD dwRetBytes = 0;
	char * buf;
	double sec;

	buf = (char*)malloc(sizeof(char)*BUF_SIZE);
	for (int i = 0; i < BUF_SIZE; i++) {
		buf[i] = i % 2;
	}

	// Create file
	file_handle = CreateFile(FILENAME,	// filename
		GENERIC_READ | GENERIC_WRITE,	// rw
		0,								// dont share
		NULL,							// default security
		CREATE_ALWAYS,					// always create new file
		FILE_ATTRIBUTE_NORMAL,			// normal file
		NULL);							// no attr.

	if (file_handle == INVALID_HANDLE_VALUE) {
		printf("Failed to create file\n");
		exit(1);
	}

	printf("%d bytes writing time\n", block_size);
	for (int iter = 0; iter < iter_count; iter++) {
		TIMER_START();

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

		error_flag = FlushFileBuffers(file_handle);
		if (error_flag == FALSE) {
			printf("Failed to flush file\n");
			exit(1);
		}

		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		printf("%.9f\n", sec);
	}

	// reset the file pointer
	dwRetBytes = SetFilePointer(file_handle,
		0,
		NULL,
		FILE_BEGIN);
	if (dwRetBytes == INVALID_SET_FILE_POINTER) {
		printf("Failed to set file pointer\n");
		exit(1);
	}

	printf("%d bytes reading time\n", block_size);
	for (int iter = 0; iter < iter_count; iter++) {
		TIMER_START();

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

		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		printf("%.9f\n", sec);
	}


	// Close file
	CloseHandle(file_handle);

	error_flag = DeleteFile(FILENAME);
	if (error_flag == FALSE) {
		printf("Failed to delete file\n");
		exit(1);
	}
}