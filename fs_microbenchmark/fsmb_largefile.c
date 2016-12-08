#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <time.h>
#include "fsmb_largefile.h"
#include "time_util.h"

#define BUF_SIZE 1024*1024 // 1MB, because the maximum size of block unit is 1MB
#define NAME_BUF_SIZE 1024
#define LARGE_FILE_SIZE 1024*1024*1024 // 4GB, TODO: TBD

/*
 * Largefile microbenchmark Design
 *
 * 1. create/open/write 1 block(various length)
 *    repetitively until reaching the pre-defined file size.
 * 2. read sequentially block-by-block
 * 3. lseek() random access, read 1 block
 * 4. lseek() random access, rewrite 1 block
 * 5. delete file
 *
 */

unsigned long long ulrand(){
	unsigned long long ret = 0;

	for (int i = 0; i < 5; ++i) {
		ret = (ret << 15) | (rand() & 0x7FFF);
	}

	return ret & 0xFFFFFFFFFFFFFFFFULL;
}

void fsmb_largefile_benchmark(int block_size, int count){
	HANDLE file_handle;
	BOOL error_flag;
	DWORD dwRetBytes = 0;
	char * buf;
	TCHAR filepath_r[NAME_BUF_SIZE];
	double sec;

	unsigned long long tot_file_size, read_file_size, tmp;
	unsigned long long *rand_seq;
	unsigned long long block_count = LARGE_FILE_SIZE/block_size;

	buf = (char*)malloc(sizeof(char)*BUF_SIZE);
	rand_seq = (unsigned long long*)malloc(block_count * sizeof(unsigned long long));

    printf("micro benchmarking for one big file RW\n");
    printf("Opn/Seq Wrt | Seq Read | Ran Read | Ran Wrt | Del\n");

	for (int iter = 0; iter < count; iter++) {
		/*** Rand value set ***/
		srand(time(NULL)); // set seed for random function

		// init
		for(unsigned long i=0;i<block_count;i++){
			rand_seq[i] = i;
		}

		// shuffle
		for (unsigned long long i=block_count - 1; i > 0; i--) {
			unsigned long long j = ulrand() % (i + 1);

			tmp = rand_seq[i];
			rand_seq[i] = rand_seq[j];
			rand_seq[j] = tmp;
		}


		/*** First phase ***/
		swprintf(filepath_r, NAME_BUF_SIZE, TEXT("large_%d"), iter);
		tot_file_size = 0;

		TIMER_START();

		// Create file
		file_handle = CreateFile(filepath_r,	// filename
			GENERIC_READ | GENERIC_WRITE,					// write only
			0,								// dont share
			NULL,							// default security
			CREATE_ALWAYS,					// always create new file
			FILE_ATTRIBUTE_NORMAL,			// normal file
			NULL);							// no attr.

		if (file_handle == INVALID_HANDLE_VALUE) {
			printf("Failed to create file\n");
			exit(1);
		}

		while (tot_file_size < LARGE_FILE_SIZE) {
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

			tot_file_size += dwRetBytes;
		}

		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		printf("%f \t", sec);

		/*** Second phase ***/
		TIMER_START();

		read_file_size = 0;
		while (tot_file_size > read_file_size) {
			// Read data
			error_flag = ReadFile(file_handle,	// file handle to read
				buf,							// read buffer
				block_size,						// how many bytes
				&dwRetBytes,				// number of bytes
				NULL);							// no overlapped structrue

			if (error_flag == FALSE || dwRetBytes != block_size) {
				printf("Failed to read a block\n");
				exit(1);
			}
			read_file_size += dwRetBytes;
		}
		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		printf("%f \t", sec);

		/*** Third phase ***/
		TIMER_START();

		for (unsigned long long i = 0; i < block_count; i++) {
			dwRetBytes = SetFilePointer(file_handle,
				rand_seq[i] * block_size,
				NULL,
				FILE_BEGIN);
			if(dwRetBytes == INVALID_SET_FILE_POINTER) {
				printf("Failed to set file pointer\n");
				exit(1);
			}

			// Read data
			error_flag = ReadFile(file_handle,	// file handle to read
				buf,							// read buffer
				block_size,						// how many bytes
				&dwRetBytes,				// number of bytes
				NULL);							// no overlapped structrue

			if (error_flag == FALSE || dwRetBytes != block_size) {
				printf("Failed to read a block %ld %d\n", dwRetBytes, block_size);
				exit(1);
			}
		}

		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		printf("%f \t", sec);

		/*** Fourth phase ***/
		TIMER_START();

		for (unsigned long long i = 0; i < block_count; i++) {
			dwRetBytes = SetFilePointer(file_handle,
				rand_seq[i] * block_size,
				NULL,
				FILE_BEGIN);
			if (dwRetBytes == INVALID_SET_FILE_POINTER) {
				printf("Failed to set file pointer\n");
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
		}

		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		printf("%f \t", sec);


		/*** Fifth phase ***/
		// Close file to delete
		CloseHandle(file_handle);

		TIMER_START();

		error_flag = DeleteFile(filepath_r);
		if (error_flag == FALSE) {
			printf("Failed to delete file\n");
			exit(1);
		}

		TIMER_END();

		sec = TIMER_ELAPSE_SEC();

		printf("%f \n", sec);
	}

	free(rand_seq);
}
