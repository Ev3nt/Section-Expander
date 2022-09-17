#include <iostream>
#include <windows.h>
#include <string>
#include <fstream>

#define ALIGN_DOWN(x, align) (x & ~(align - 1))
#define ALIGN_UP(x, align) ((x & (align - 1)) ? ALIGN_DOWN(x, align) + align : x) 

BOOL main(int argc, LPCSTR argv[]) {
	std::string name;

	if (argc > 1) {
		name = argv[1];
	} 
	else {
		char buffer[MAX_PATH] = { 0 };
		std::cout << "Type file name: ";
		std::cin.getline(buffer, sizeof(buffer));

		GetFullPathName(buffer, sizeof(buffer), buffer, NULL);

		name = buffer;
	}

	std::ifstream file(name, std::ios::binary);
	if (!file) {
		std::cout << "File doesn't exist." << std::endl;
		system("pause");

		return FALSE;
	}

	std::string buffer((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	size_t lenght = buffer.size();
	file.close();

	PIMAGE_DOS_HEADER dos_header = (PIMAGE_DOS_HEADER)buffer.data();
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE) {
		std::cout << "File has not DOS signature." << std::endl;

		system("pause");

		return FALSE;
	}

	PIMAGE_NT_HEADERS nt_header = PIMAGE_NT_HEADERS((DWORD)buffer.data() + dos_header->e_lfanew);
	if (nt_header->Signature != IMAGE_NT_SIGNATURE) {
		std::cout << "File has not NT signature." << std::endl;

		system("pause");

		return FALSE;
	}

	char section_name[8] = { 0 };
	std::cout << "Type section name [8 symbols]: ";
	std::cin.getline(section_name, 8);

	int section_size;
	std::cout << "Type section size: ";
	std::cin >> section_size;
	
	PIMAGE_SECTION_HEADER section = &IMAGE_FIRST_SECTION(nt_header)[nt_header->FileHeader.NumberOfSections];
	PIMAGE_SECTION_HEADER last_section = &IMAGE_FIRST_SECTION(nt_header)[nt_header->FileHeader.NumberOfSections - 1];
	ZeroMemory(section, sizeof(IMAGE_SECTION_HEADER));
	CopyMemory(section->Name, section_name, sizeof(section_name));

	section->VirtualAddress = ALIGN_UP(last_section->Misc.VirtualSize, nt_header->OptionalHeader.SectionAlignment) + last_section->VirtualAddress;
	section->Misc.VirtualSize = ALIGN_UP(section_size, nt_header->OptionalHeader.SectionAlignment);
	section->PointerToRawData = ALIGN_UP(last_section->SizeOfRawData, nt_header->OptionalHeader.FileAlignment) + last_section->PointerToRawData;
	section->SizeOfRawData = ALIGN_UP(section_size, nt_header->OptionalHeader.FileAlignment);
	section->Characteristics = IMAGE_SCN_CNT_CODE | IMAGE_SCN_MEM_EXECUTE | IMAGE_SCN_MEM_WRITE | IMAGE_SCN_MEM_READ | IMAGE_SCN_CNT_INITIALIZED_DATA;

	nt_header->OptionalHeader.SizeOfImage = section->VirtualAddress + section->Misc.VirtualSize;
	nt_header->FileHeader.NumberOfSections++;

	lenght += section->Misc.VirtualSize;
	buffer.resize(lenght);

	dos_header = (PIMAGE_DOS_HEADER)buffer.data();
	nt_header = PIMAGE_NT_HEADERS((DWORD)buffer.data() + dos_header->e_lfanew);
	
	size_t dot_position = name.find_last_of('.');
	std::ofstream output((dot_position != std::string::npos ? name.substr(0, dot_position) : name) + " [Expanded]" + (dot_position != std::string::npos ? name.substr(dot_position, name.size() - dot_position) : ""), std::ios::binary);
	output.write(buffer.data(), lenght);
	output.close();

	system("cls");

	std::cout << "Sections number: " << nt_header->FileHeader.NumberOfSections << std::endl
		<< "----------------------------------------------------------------" << std::endl;

	section = IMAGE_FIRST_SECTION(nt_header);
	for (int i = 0; i < nt_header->FileHeader.NumberOfSections; i++, section++) {
		std::cout	<< section->Name << ":" << std::endl
					<< "Virtual Address - " << (LPVOID)section->VirtualAddress << std::endl
					<< "Virtual Size - " << section->Misc.VirtualSize << " = 0x" << (LPVOID)section->Misc.VirtualSize << std::endl
					<< "Pointer to Raw Data - " << (LPVOID)section->PointerToRawData << std::endl
					<< "Size of Raw Data - " << section->SizeOfRawData << " = 0x" << (LPVOID)section->SizeOfRawData << std::endl
					<< "Characteristics - " << (LPVOID)section->Characteristics << std::endl
					<< "----------------------------------------------------------------" << std::endl;
	}

	system("pause");

	return FALSE;
}