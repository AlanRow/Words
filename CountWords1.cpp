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
//int BUFSIZE = 10000;

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

class BuffAllocator {
	char * free;
	char * buf;
	char * end;
	int bufsize;

public:
	BuffAllocator() : BuffAllocator(200000) {}

	BuffAllocator(int size) {
		bufsize = size;
		cout << "Allocator created!" << endl;

		buf = (char*)malloc(bufsize);
		*(void **)buf = 0;
		
		free = buf + sizeof(void*);
	}

	void * allocate(size_t Count) {
		if (free + Count > end) {
			cout << "Buffer created!" << endl;
			char * new_buf = (char*)malloc(bufsize);//  1048576
			*(void **)new_buf = buf;
			buf = new_buf;

			end = new_buf + bufsize;
			free = new_buf + sizeof(char *);
		}

		char * to_write = free;
		free += Count;

		return to_write;
	}

	BuffAllocator::~BuffAllocator() {
		void* p = *(void **)buf;
		char* cur = buf;

		while (p != 0) {
			std::free(cur);
			cout << "Buffer deleted!" << endl;
			cur = (char*)p;
			p = *(void **)cur;
		}
	}
};

class CBuddyAllocator {
private:
	list<char*> free_blocks[20];

public:
	CBuddyAllocator() {
		for (int i = 0; i < 20; i++)
			free_blocks[i] = list<char*>();

		char* p = (char*)malloc(2097152);

		free_blocks[19].emplace_back(p);
	}

	void * allocate(size_t Count) {
		int level = (int) ceil(log2(Count)) - 2;
		
		int cur_level = level;
		while (!free_blocks[cur_level].empty()) {
			cur_level++;
		}

		if (cur_level == level) {
			char* p = free_blocks[level].front();
			free_blocks[level].remove(p);

			return p;
		}
		else {
			char* p = free_blocks[cur_level].front();
			free_blocks[cur_level].remove(p);
			cur_level--;
			while (cur_level >= level) {
				int shift = 1 << (2 + cur_level);
				char* cur_p = p + shift;
				free_blocks[cur_level].emplace_back(cur_p);
				cur_level--;
			}
			return p;
		}
	}
};

CBuddyAllocator BUDDY_ALLOCATOR = CBuddyAllocator();

template <class T>
class CMyAllocator {

public:
	typedef typename T value_type;

	CMyAllocator()
	{
	}

	template <class U>
	CMyAllocator(const CMyAllocator<U> &V)
	{
	}

	T* allocate(size_t Count) {
		return (T*)BUDDY_ALLOCATOR.allocate(Count * sizeof(T));
	}

	void deallocate(T* V, size_t Count)
	{
	}
};

BuffAllocator ALLOCATOR = BuffAllocator();

/*template <class T>
class CMyAllocator
{
public:
	typedef typename T value_type;



	CMyAllocator()
	{
	}

	template <class U>
	CMyAllocator(const CMyAllocator<U> &V)
	{
	}

	T* allocate(size_t Count) {
		return (T*)ALLOCATOR.allocate(Count * sizeof(T));
	}

	void deallocate(T* V, size_t Count)
	{
	}
};*/

HANDLE open_for_read() {
	return CreateFile(TEXT("text.txt"), GENERIC_READ, 0, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
}

/*template <class T>
class CMyAllocator
{
public:
	typedef typename T value_type;
	CMyAllocator()
	{
	}
	template <class U>
	CMyAllocator(const CMyAllocator<U> &V)
	{
	}
	T* allocate(size_t Count)
	{
		//printf("Allocate %d\n", (int)(Count * sizeof(T)));
		return (T*)malloc(sizeof(T) * Count);
	}
	void deallocate(T* V, size_t Count)
	{
		//printf("Free %d\n", (int)(Count * sizeof(T)));
		free(V);
	}
};*/
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

void abetinit(int* abet) {
	for (int i = 0; i < 256; i++) {
		abet[i] = 0;
	}


	char *abet_symb = ABET;

	while (*abet_symb != '\0') {
		abet[(int)*abet_symb] = 1;
		abet_symb++;
	}
}

void parsewords(char* text, int* abet) {

	while (*text != '\0') {
		//char *abet_symb = ABET;

		if (abet[(int)*text] == 1) {
			*text = tolower(*text);
		}
		else {
			*text = '\0';
		}
		text++;
	}
}

int main(int argc, char* argv[])
{ // time ~ 2.7
  //map<char*, size_t> Map; // time ~ 3.4

  //alphabet initialize (int array: 0 - not a letter, 1 - letter)
	int* abet = new int[256];
	abetinit(abet);

	char* text;
	int length = read_file(&text);

	double time = 0;

	//for (int i = 0; i < 1; i++) {
		chrono::time_point<chrono::high_resolution_clock> start = chrono::high_resolution_clock::now();
		parsewords(text, abet);

		map<char*, size_t, CStringComparator, CMyAllocator<char*>> Map;

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
		auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start);
		time += elapsed.count();
	//}

	for (auto Entry : Map)
	{
		//printf("Word %s, count %I64d\n", Entry.first.c_str(), (uint64_t)Entry.second);
		printf("Word %s, count %d\n", Entry.first, Entry.second);
	}

	//BuffAllocator buf = BuffAllocator();
	//delete &buf;

	cout << endl << "Elapsed time: " << time << " ms" << endl;

	getchar();
	//delete &ALLOCATOR;
	return 0;
}