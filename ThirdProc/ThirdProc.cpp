#include <iostream>
#include <windows.h>
#include <conio.h>
#include <stdio.h>
#include <tchar.h>

#define CRITICAL_SECTION_NAME TEXT("OLD_SECTION")
#define SHARED_MEMORY_NAME TEXT("SOMETHING")
#define N 3

void freeResources(HANDLE* mapping, LPCTSTR* buff);

int main()
{
    size_t sizeOfSharedMemory = sizeof(int) * N;
    size_t sizeOfSection = sizeof(CRITICAL_SECTION);
    LPCTSTR bufForSharedMemory;
    HANDLE mappingForCrSection, mappingForMemory;
    CRITICAL_SECTION* crSection;

    mappingForCrSection = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, CRITICAL_SECTION_NAME);// CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeOfSection, CRITICAL_SECTION_NAME);
    if (mappingForCrSection == NULL) {
        return -1;
    }
    crSection = (CRITICAL_SECTION*)MapViewOfFile(mappingForCrSection, FILE_MAP_ALL_ACCESS, 0, 0, sizeOfSection);
    if (crSection == NULL) {
        freeResources(&mappingForCrSection, NULL);
        return NULL;
    }


    mappingForMemory = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);// CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeOfSharedMemory, SHARED_MEMORY_NAME);
    if (mappingForMemory == NULL) {
        freeResources(&mappingForCrSection, NULL);
        UnmapViewOfFile(crSection);
        _getch();
        return -1;
    }
    bufForSharedMemory = (LPCTSTR)MapViewOfFile(mappingForMemory, FILE_MAP_ALL_ACCESS, 0, 0, sizeOfSharedMemory);
    if (bufForSharedMemory == NULL) {
        freeResources(&mappingForCrSection, NULL);
        UnmapViewOfFile(crSection);
        freeResources(&mappingForMemory, NULL);
        _getch();
        return -1;
    }

    bool fl = true;
    while (fl) {
        if (TryEnterCriticalSection(crSection)) {
            for (int i = 0; i < N; i++) {
                std::cout << "ThirdProc read " << ((int*)bufForSharedMemory)[i] << std::endl << std::flush;
                ((int*)bufForSharedMemory)[i] *= 3;
                std::cout << "ThirdProc wrote " << ((int*)bufForSharedMemory)[i] << std::endl << std::flush;
                Sleep(500);
            }
            LeaveCriticalSection(crSection);
            fl = false;
        }
    }

    freeResources(&mappingForCrSection, NULL);
    UnmapViewOfFile(crSection);
    freeResources(&mappingForMemory, &bufForSharedMemory);
    _getch();
    return 0;
}

void freeResources(HANDLE* mapping, LPCTSTR* buff) {
    if (buff != NULL)
        UnmapViewOfFile(*buff);
    if (mapping != NULL)
        CloseHandle(*mapping);
}