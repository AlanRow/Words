// CountWordsFromFile.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <windows.h>
#include <list>
#include <map>
#include <chrono>

using namespace std;

char* ABET = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

class CStringComparator
{
public:

	/*
	A < B --> true
	*/
	bool operator()(const char* A, const char* B) const
	{
		while (true)
		{
			if (A[0] == B[0])
			{
				//A = B
				if (!A[0])
					return false;

				A++;
				B++;
			}
			else
			{
				return A[0] < B[0];
			}
		}
	}
};

template <class T>
class CMyAllocator
{
char * Buffer;
char * Free;
char * End;
public:
	typedef typename T value_type;

	CMyAllocator()
	{
		Buffer = (char*)malloc(1048576);
		char *Next = 0;
		End = Buffer + 1048576;
		Free = Buffer + sizeof(Next);
	}

	template <class U>
	CMyAllocator(const CMyAllocator<U> &V)
	{
		Buffer = (char*)malloc(1048576);
		char *Next = 0;
		End = Buffer + 1048576;
		Free = Buffer + sizeof(Next);
	}

	T* allocate(size_t Count) {
		if (Free + sizeof(T) * Count > End) {
			char * new_buf = (char*)malloc(1048576);
			void ** Next = (void **) new_buf;
			*Next = Buffer;
			Buffer = new_buf;
			End = Buffer + 1048576;

			char * to_write = Free;
			Free = Buffer + sizeof(T) * Count + sizeof(Next);

			return (T*)to_write;
		}
		else {
			Free += sizeof(T) * Count;
			return(T*)Free;
		}
	}
	
	void deallocate(T* V, size_t Count)
	{
		void ** Next = *Buffer;
		if (Free )
		free(V);
	}
/*
	T* allocate(size_t Count)
	{
		//printf("Allocate %d\n", (int)(Count * sizeof(T)));

		return (T*)malloc(sizeof(T) * Count);
	}

	void deallocate(T* V, size_t Count)
	{
		//printf("Free %d\n", (int)(Count * sizeof(T)));

		free(V);
	}*/
};

HANDLE open_for_read() {
	return CreateFile(TEXT("text.txt"), GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}


int read_file(char** text) {

	HANDLE file = open_for_read();

	if (INVALID_HANDLE_VALUE == file) {
		return -1;
	}

	LARGE_INTEGER size;
	GetFileSizeEx(file, &size);
	__int64 size_n = size.QuadPart;
	CloseHandle(file);

	file = open_for_read();
	unsigned long success_bytes;

	char* buffer = new char[size_n + 1];
	ReadFile(file, buffer, size_n, &success_bytes, NULL);

	if (size_n != success_bytes) {
		CloseHandle(file);
		cout << "Expected " << success_bytes << " bytes in file, but actually was "
			<< size.LowPart << "." << endl;
		getchar();
		return -1;
	}

	CloseHandle(file);

	buffer[size_n] = '\0';
	*text = buffer;
	return success_bytes + 1;
}

void parsewords(char* text) {

	while (*text != '\0') {
		char *abet_symb = ABET;
		bool is_sep = true;

		while (is_sep && *abet_symb != '\0') {
			if (*text == *abet_symb) {
				*text = tolower(*text);
				is_sep = false;
			}
			abet_symb++;
		}

		if (is_sep) {
			*text = '\0';
		}

		text++;
	}
}

int main(int argc, char* argv[])
{
	map<char*, size_t, CStringComparator, CMyAllocator<char*>> Map; // time ~ 2.7
	//map<char*, size_t> Map; // time ~ 3.4

	char* text;
	int length = read_file(&text);

	chrono::time_point<chrono::high_resolution_clock> start = chrono::high_resolution_clock::now();
	parsewords(text);

	char *p = text;

	while (p - text < length) {
		while (*p == '\0')
			p++;
		if (p - text >= length)
			break;

		auto It = Map.find(p);
		if (It == Map.end())
		{
			Map.insert(make_pair(p, 1));
		}
		else
		{
			It->second++;
		}

		while (*p != '\0')
			p++;
	}
	chrono::time_point<chrono::high_resolution_clock> end = chrono::high_resolution_clock::now();

	for (auto Entry : Map)
	{
		printf("Word %s, count %I64d\n", Entry.first, (uint64_t)Entry.second);
	}

	auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
	cout << endl << "Elapsed time: " << elapsed.count() << " ms" << endl;

	getchar();

	return 0;
}