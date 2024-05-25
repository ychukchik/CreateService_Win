#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
//#include <algorithm>
//#include <stdexcept>
//#include <ctime>
//#include <iomanip>
//#include <sstream>
//#include <filesystem>
#include <windows.h>
//#include <cstring>
//#include <cstdlib>
//#include <sstream>
//#include <iomanip>
//#include <tchar.h>
//#include <zlib.h>
#include <zip.h>
#include <dirent.h>
//#include <filesystem>
//#include <sys/stat.h>
//#include <regex>
//namespace fs = std::filesystem;
using namespace std;

SERVICE_STATUS serviceStatus;
SERVICE_STATUS_HANDLE serviceStatusHandle;
//Структура SERVICE_STATUS используется для оповещения SCM о текущем статусе сервиса

#define SERVICE_NAME _T("MyService")
#define SERVICE_PATH _T("D:\\Univer_files\\5sem\\BCIT_2\\BCIT_2_1\\x64\\Debug\\BCIT_2_1.exe")
#define LOG_FILE "D:\\Univer_files\\5sem\\BCIT_2\\BCIT_2_1\\x64\\Debug\\log.txt"
#define CONFIG_FILE L"D:\\Univer_files\\5sem\\BCIT_2\\BCIT_2_1\\x64\\Debug\\config.txt"
#define ARCHIVE_FILE "D:\\test_bcit\\archive.zip"

void ServiceMain(int argc, char** argv);
int addLogMessage(const char* str);
void ControlHandler(DWORD request);
int InstallService();
int RemoveService();
int StartService();
int StopService();
void ServiceFileBackup();

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc - 1 == 0)
	{
		SERVICE_TABLE_ENTRY ServiceTable[1];
		ServiceTable[0].lpServiceName = (LPWSTR)SERVICE_NAME;
		ServiceTable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTION)ServiceMain;
		if (!StartServiceCtrlDispatcher(ServiceTable))
		{
			addLogMessage("Error: StartServiceCtrlDispatcher");
		}
	}
	else if (wcscmp(argv[argc - 1], _T("install")) == 0)
	{
		InstallService();
	}
	else if (wcscmp(argv[argc - 1], _T("remove")) == 0)
	{
		RemoveService();
	}
	else if (wcscmp(argv[argc - 1], _T("start")) == 0)
	{
		StartService();
	}
	else if (wcscmp(argv[argc - 1], _T("stop")) == 0)
	{
		StopService();
	}
}
// SERVICE_TABLE_ENTRY это структура, которая описывает точку входа для сервис
//менеджера, в данном случае вход будет происходить через функцию ServiceMain.
//Функция StartServiceCtrlDispatcher связывает сервис с SCM

void ServiceMain(int argc, char** argv)
{
	int i = 0;
	serviceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	serviceStatus.dwCurrentState = SERVICE_START_PENDING;
	serviceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_SHUTDOWN;
	serviceStatus.dwWin32ExitCode = 0;
	serviceStatus.dwServiceSpecificExitCode = 0;
	serviceStatus.dwCheckPoint = 0;
	serviceStatus.dwWaitHint = 0;
	serviceStatusHandle = RegisterServiceCtrlHandler(SERVICE_NAME, (LPHANDLER_FUNCTION)ControlHandler);
	if (serviceStatusHandle == (SERVICE_STATUS_HANDLE)0)
	{
		return;
	}

	serviceStatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(serviceStatusHandle, &serviceStatus);
	while (serviceStatus.dwCurrentState == SERVICE_RUNNING)
	{
		ServiceFileBackup();
		Sleep(1000);
	}
	return;
}
//Логика этой функции: Сначала регистрируется функция, которая будет
//обрабатывать управляющие запросы от SCM, например, запрос на остановку.Регистрация
//производится при помощи функции RegisterServiceCtrlHandler.При корректном запуске
//сервиса бэкапим файл.
//Для изменения статуса сервиса используется функция SetServiceStatus.

int addLogMessage(const char* str)
{
	errno_t err;
	FILE* log;
	if ((err = fopen_s(&log, LOG_FILE, "a+")) != 0)
	{
		return -1;
	}
	fprintf(log, ">> %s\n", str);
	fclose(log);
	return 0;
}

void ControlHandler(DWORD request) // функция по обработке запросов
{ 
	switch (request)
	{
	case SERVICE_CONTROL_STOP:
		addLogMessage("Stopped.");
		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		return;
	case SERVICE_CONTROL_SHUTDOWN:
		addLogMessage("Shutdown.");
		serviceStatus.dwWin32ExitCode = 0;
		serviceStatus.dwCurrentState = SERVICE_STOPPED;
		SetServiceStatus(serviceStatusHandle, &serviceStatus);
		return;

	default:
		break;
	}
	SetServiceStatus(serviceStatusHandle, &serviceStatus);
	return;
}
//ControlHandler вызывается каждый раз, как SCM шлет запросы на изменение состояния сервиса.
//В основном ее используют для описания корректного завершения работы сервиса.

int InstallService()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	if (!hSCManager)
	{
		addLogMessage("Error: Can't open Service Control Manager");
		return -1;
	}

	SC_HANDLE hService = CreateService(
		hSCManager,
		SERVICE_NAME, // service name
		SERVICE_NAME, // show name
		SERVICE_ALL_ACCESS,
		SERVICE_WIN32_OWN_PROCESS,
		SERVICE_DEMAND_START,
		SERVICE_ERROR_NORMAL,
		SERVICE_PATH,
		NULL, NULL, NULL, NULL, NULL
	);
	if (!hService)
	{
		int err = GetLastError();
		switch (err)
		{
		case ERROR_ACCESS_DENIED:
			addLogMessage("Error: ERROR_ACCESS_DENIED");
			break;
		case ERROR_CIRCULAR_DEPENDENCY:
			addLogMessage("Error: ERROR_CIRCULAR_DEPENDENCY");
			break;
		case ERROR_DUPLICATE_SERVICE_NAME:
			addLogMessage("Error: ERROR_DUPLICATE_SERVICE_NAME");
			break;
		case ERROR_INVALID_HANDLE:
			addLogMessage("Error: ERROR_INVALID_HANDLE");
			break;
		case ERROR_INVALID_NAME:
			addLogMessage("Error: ERROR_INVALID_NAME");
			break;
		case ERROR_INVALID_PARAMETER:
			addLogMessage("Error: ERROR_INVALID_PARAMETER");
			break;
		case ERROR_INVALID_SERVICE_ACCOUNT:
			addLogMessage("Error: ERROR_INVALID_SERVICE_ACCOUNT");
			break;
		case ERROR_SERVICE_EXISTS:
			addLogMessage("Error: ERROR_SERVICE_EXISTS");
			break;
		default:
			addLogMessage("Error: Undefined");
		}
		CloseServiceHandle(hSCManager);
		return -1;
	}
	CloseServiceHandle(hService);

	CloseServiceHandle(hSCManager);
	addLogMessage("Success install service!");
	return 0;
}

int RemoveService()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
	{
		addLogMessage("Error: Can't open Service Control Manager");
		return -1;
	}
	SC_HANDLE hService = OpenService(hSCManager, SERVICE_NAME, SERVICE_STOP | DELETE);
	if (!hService)
	{
		addLogMessage("Error: Can't remove service");
		CloseServiceHandle(hSCManager);
		return -1;
	}

	DeleteService(hService);
	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);

	addLogMessage("Success remove service!");

	return 0;
}

int StartService()
{
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
	SC_HANDLE hService = OpenService(hSCManager, SERVICE_NAME, SERVICE_START);
	if (!StartService(hService, 0, NULL))
	{
		CloseServiceHandle(hSCManager);
		addLogMessage("Error: Can't start service");
		return -1;
	}

	CloseServiceHandle(hService);
	CloseServiceHandle(hSCManager);

	addLogMessage("Success start service!");

	return 0;
}

int StopService()
{
	// Открываем Service Control Manager и получаем его дескриптор
	SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
	if (!hSCManager)
	{
		addLogMessage("Error: Can't open Service Control Manager");
		return -1;
	}
	// Открываем сервис для остановки и удаления
	SC_HANDLE hService = OpenService(hSCManager, SERVICE_NAME, SERVICE_STOP | DELETE);
	if (!hService) 
	{
		addLogMessage("Error: Can't remove service");
		CloseServiceHandle(hSCManager); // Закрываем дескриптор Service Control Manager
		return -1;
	}
	
	// Отправляем команду остановки сервиса
	ControlService(hService, SERVICE_CONTROL_STOP, &serviceStatus);

	addLogMessage("Success stop service!");

	return 0;
}

//void ServiceFileBackup()
//{
//	std::ifstream config(CONFIG_FILE);
//	if (!config.is_open())
//	{
//		addLogMessage("Error: cannot find configuration file");
//		return;
//	}
//
//	std::vector<std::string> src_paths;
//	std::string line;
//	while (std::getline(config, line))
//	{
//		src_paths.push_back(line);
//	}
//	config.close();
//
//	std::ofstream archive(ARCHIVE_FILE, std::ios::out | std::ios::binary);
//	if (!archive.is_open())
//	{
//		addLogMessage("Error: cannot create archive file");
//		return;
//	}
//
//	std::time_t now = std::time(nullptr);
//	std::tm local_time = *std::localtime(&now);
//	std::ostringstream archive_name;
//	archive_name << "backup_" << std::put_time(&local_time, "%Y%m%d%H%M%S") << ".dat";
//
//	archive << archive_name.str() << '\0';
//
//	for (const std::string& src_path : src_paths)
//	{
//		std::ifstream source(src_path, std::ios::in | std::ios::binary);
//		if (source.is_open())
//		{
//			std::string file_name = src_path.substr(src_path.find_last_of("\\/") + 1);
//			archive << file_name << '\0';
//			source.seekg(0, std::ios::end);
//			std::streampos file_size = source.tellg();
//			source.seekg(0, std::ios::beg);
//			archive.write(reinterpret_cast<const char*>(&file_size), sizeof(file_size));
//
//			archive << source.rdbuf();
//			source.close();
//		}
//		else
//		{
//			addLogMessage("Error: cannot open file for backup");
//		}
//	}
//
//	archive.close();
//	addLogMessage("Backup completed.");
//}

//void CompressFile(const std::string& sourceFile, const std::string& destinationFile) {
//	std::ifstream inFile(sourceFile, std::ios::binary);
//	std::ofstream outFile(destinationFile, std::ios::binary);
//
//	if (!inFile || !outFile) {
//		std::cerr << "Error: Unable to open files for compression" << std::endl;
//		return;
//	}
//
//	z_stream zstream;
//	zstream.zalloc = Z_NULL;
//	zstream.zfree = Z_NULL;
//	zstream.opaque = Z_NULL;
//
//	if (deflateInit(&zstream, Z_BEST_COMPRESSION) != Z_OK) {
//		std::cerr << "Error: deflateInit failed" << std::endl;
//		return;
//	}
//
//	const int bufferSize = 4096;
//	std::vector<unsigned char> inBuffer(bufferSize);
//	std::vector<unsigned char> outBuffer(bufferSize);
//
//	do {
//		inFile.read(reinterpret_cast<char*>(inBuffer.data()), bufferSize);
//		zstream.avail_in = static_cast<uInt>(inFile.gcount());
//		zstream.next_in = inBuffer.data();
//
//		do {
//			zstream.avail_out = bufferSize;
//			zstream.next_out = outBuffer.data();
//			deflate(&zstream, Z_NO_FLUSH);
//			outFile.write(reinterpret_cast<char*>(outBuffer.data()), bufferSize - zstream.avail_out);
//		} while (zstream.avail_out == 0);
//
//	} while (inFile);
//
//	do {
//		zstream.avail_out = bufferSize;
//		zstream.next_out = outBuffer.data();
//		deflate(&zstream, Z_FINISH);
//		outFile.write(reinterpret_cast<char*>(outBuffer.data()), bufferSize - zstream.avail_out);
//	} while (zstream.avail_out == 0);
//
//	deflateEnd(&zstream);
//
//	inFile.close();
//	outFile.close();
//}
//
//void ServiceFileBackup()
//{
//	std::string src_path = "source_file.txt";
//	std::string dst_path = "compressed_file.gz";
//
//	CompressFile(src_path, dst_path);
//}
// 

//#define COM_LINE_BEG "\"C:\\Program Files\\7-Zip\\7z.exe\" u -tzip -mx=0 "
//#define COM_LINE_END " -r"
//
//void ServiceFileBackup()
//{
//	ifstream in(CONFIG_FILE);
//	if (!in.is_open())
//	{
//		printf("Error: cannot find configuration file");
//		return;
//	}
//	else
//	{
//
//		ifstream in(CONFIG_FILE);
//		string src_path, temp_line, com_line, dst_path, mask;
//		getline(in, src_path);
//		getline(in, dst_path);
//		getline(in, mask);
//		while (mask.size())
//		{
//			com_line = COM_LINE_BEG + dst_path + " " + src_path + mask + COM_LINE_END;
//			system(com_line.c_str());
//			mask = "";
//			getline(in, mask);
//		}
//		in.close();
//	}
//}

//#define MAX_DIR_LENGTH 512
//#define MAX_MASK_LENGTH 64
//
//void createArchive(const wchar_t* sourceDir, const wchar_t* backupDir, const wchar_t* mask) {
//	addLogMessage("Start createArchive()");
//	WIN32_FIND_DATAW findFileData;
//	wchar_t searchPath[MAX_DIR_LENGTH];
//	swprintf(searchPath, MAX_DIR_LENGTH, L"%s\\%s", sourceDir, mask);
//	HANDLE hFind = FindFirstFileW(searchPath, &findFileData);
//
//	if (hFind == INVALID_HANDLE_VALUE) {
//		return;
//	}
//
//	do {
//		if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
//			wchar_t filePath[MAX_DIR_LENGTH];
//			swprintf(filePath, MAX_DIR_LENGTH, L"%s\\%s", sourceDir, findFileData.cFileName);
//
//			// Open the file for reading
//			FILE* file = _wfopen(filePath, L"rb");
//			if (file != NULL) {
//				fseek(file, 0, SEEK_END);
//				long fileLength = ftell(file);
//				fseek(file, 0, SEEK_SET);
//
//				uLongf compress_buff_size = compressBound(fileLength);
//				Bytef* compress_buff = (Bytef*)malloc(compress_buff_size);
//				if (compress_buff == NULL) {
//					fclose(file);
//					continue;
//				}
//
//				uLongf compressed_size = compress_buff_size;
//
//				// Read the file into a buffer
//				if (fread(compress_buff, 1, fileLength, file) != fileLength) {
//					addLogMessage("Failed to read file data.");
//					fclose(file);
//					free(compress_buff);
//					continue;
//				}
//
//				int res = compress(compress_buff, &compressed_size, compress_buff, fileLength);
//				if (res != Z_OK) {
//					addLogMessage("compress failed");
//					fclose(file);
//					free(compress_buff);
//					continue;
//				}
//
//				// Create a unique filename for the backup
//				wchar_t archiveName[MAX_DIR_LENGTH];
//				swprintf(archiveName, MAX_DIR_LENGTH, L"%s\\backup_%s_%s.zip", backupDir, mask, findFileData.cFileName);
//
//				// Open the output file for writing
//				FILE* archiveFile = _wfopen(archiveName, L"wb");
//				if (archiveFile == NULL) {
//					addLogMessage("Failed to create archive file.");
//					fclose(file);
//					free(compress_buff);
//					continue;
//				}
//
//				// Write the compressed data to the archive
//				fwrite(compress_buff, 1, compressed_size, archiveFile);
//
//				fclose(archiveFile);
//				fclose(file);
//				free(compress_buff);
//			}
//		}
//	} while (FindNextFileW(hFind, &findFileData) != 0);
//
//	FindClose(hFind);
//	addLogMessage("Finished createArchive()");
//}
//
//void ServiceFileBackup() {
//	FILE* configFile = _wfopen(CONFIG_FILE, L"r");
//	if (configFile == NULL) {
//		addLogMessage("Failed to open config file");
//		return;
//	}
//
//	wchar_t sourceDir[MAX_DIR_LENGTH];
//	wchar_t backupDir[MAX_DIR_LENGTH];
//	wchar_t mask[MAX_MASK_LENGTH];
//
//	if (fgetws(sourceDir, sizeof(sourceDir) / sizeof(wchar_t), configFile) == NULL ||
//		fgetws(backupDir, sizeof(backupDir) / sizeof(wchar_t), configFile) == NULL) {
//		addLogMessage("Failed to read source and backup directories from the config file.");
//		fclose(configFile);
//		return;
//	}
//
//	sourceDir[wcslen(sourceDir) - 1] = L'\0';
//	backupDir[wcslen(backupDir) - 1] = L'\0';
//
//	while (fgetws(mask, sizeof(mask) / sizeof(wchar_t), configFile) != NULL) {
//		mask[wcslen(mask) - 1] = L'\0';
//		createArchive(sourceDir, backupDir, mask);
//	}
//
//	fclose(configFile);
//
//	addLogMessage("ServiceFileBackup finished!");
//}

//void ServiceFileBackup()
//{
//	const std::string sourceDirectory = "D:\\bcit_from";
//	const std::string outputZipFile = "D:\\bcit_to.zip";
//
//	std::vector<std::string> filesToCompress;
//
//	// Получение списка файлов в директории
//	WIN32_FIND_DATAW findFileData;
//	std::wstring sourceDirectoryWide = L"D:\\bcit_from";
//	std::wstring searchPath = sourceDirectoryWide + L"\\*";
//	HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findFileData);
//
//
//
//	if (hFind != INVALID_HANDLE_VALUE) {
//		do {
//			if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
//				filesToCompress.push_back(std::string(findFileData.cFileName, findFileData.cFileName + wcslen(findFileData.cFileName)));
//			}
//		} while (FindNextFile(hFind, &findFileData) != 0);
//		FindClose(hFind);
//	}
//
//	if (filesToCompress.empty()) {
//		std::cout << "No files to compress in the source directory." << std::endl;
//		return;
//	}
//
//	// Открытие архива ZIP
//	gzFile zipFile = gzopen(outputZipFile.c_str(), "wb");
//	if (!zipFile) {
//		std::cerr << "Failed to open the output ZIP file." << std::endl;
//		return;
//	}
//
//	// Сжатие и добавление файлов в архив
//	for (const std::string& fileName : filesToCompress) {
//		std::string filePath = sourceDirectory + "\\" + fileName;
//		gzFile sourceFile = gzopen(filePath.c_str(), "rb");
//		if (!sourceFile) {
//			std::cerr << "Failed to open the source file: " << fileName << std::endl;
//			gzclose(zipFile);
//			return;
//		}
//
//		// Чтение и сжатие содержимого файла
//		char buffer[1024];
//		int bytesRead = 0;
//		while ((bytesRead = gzread(sourceFile, buffer, sizeof(buffer))) > 0) {
//			gzwrite(zipFile, buffer, bytesRead);
//		}
//
//		gzclose(sourceFile);
//	}
//
//	gzclose(zipFile);
//
//	std::cout << "Compression completed. Output ZIP file: " << outputZipFile << std::endl;
//}

//void ServiceFileBackup() {
//	const std::string sourceDirectory = "D:\\bcit_from";
//	const std::string outputZipFile = "D:\\bcit_to.zip";
//
//	std::vector<std::string> filesToCompress;
//
//	// Получение списка файлов в директории
//	for (const auto& entry : fs::directory_iterator(sourceDirectory)) {
//		if (!entry.is_directory()) {
//			filesToCompress.push_back(entry.path().string());
//		}
//	}
//
//	if (filesToCompress.empty()) {
//		std::cout << "No files to compress in the source directory." << std::endl;
//		return;
//	}
//
//	// Открытие архива ZIP
//	gzFile zipFile = gzopen(outputZipFile.c_str(), "wb");
//	if (!zipFile) {
//		std::cerr << "Failed to open the output ZIP file." << std::endl;
//		return;
//	}
//
//	// Сжатие и добавление файлов в архив
//	for (const std::string& filePath : filesToCompress) {
//		gzFile sourceFile = gzopen(filePath.c_str(), "rb");
//		if (!sourceFile) {
//			std::cerr << "Failed to open the source file: " << filePath << std::endl;
//			gzclose(zipFile);
//			return;
//		}
//
//		// Чтение и сжатие содержимого файла
//		char buffer[1024];
//		int bytesRead = 0;
//		while ((bytesRead = gzread(sourceFile, buffer, sizeof(buffer))) > 0) {
//			gzwrite(zipFile, buffer, bytesRead);
//		}
//
//		gzclose(sourceFile);
//	}
//
//	gzclose(zipFile);
//
//	std::cout << "Compression completed. Output ZIP file: " << outputZipFile << std::endl;
//}

// Проверка, соответствует ли имя файла заданным шаблонам
bool isMatchingTemplate(const std::string& fileName, const std::vector<std::string>& templates) {
	for (const std::string& tmp : templates) {
		bool isMatching = true;
		size_t templatePos = 0;
		size_t fileNamePos = 0;
		while (templatePos < tmp.length() && fileNamePos < fileName.length()) {
			if (tmp[templatePos] == '*') {
				if (templatePos == tmp.length() - 1 || tmp[templatePos + 1] == '.') {
					// Звездочка с точкой означает, что нужно сравнивать расширение файла
					size_t dotPos = fileName.find_last_of(".");
					if (dotPos == std::string::npos) {
						isMatching = false;
						break;
					}
					fileNamePos = dotPos + 1;
				}
				else {
					// Звездочка без точки - сравнивать оставшуюся часть имени файла
					templatePos++;
					while (fileNamePos < fileName.length() && fileName[fileNamePos] != tmp[templatePos]) {
						fileNamePos++;
					}
				}
			}
			else if (tmp[templatePos] == '?') {
				// Вопросительный знак означает любой символ
				templatePos++;
				fileNamePos++;
			}
			else if (tmp[templatePos] != fileName[fileNamePos]) {
				isMatching = false;
				break;
			}
			else {
				templatePos++;
				fileNamePos++;
			}
		}
		if (isMatching && templatePos == tmp.length() && fileNamePos == fileName.length()) {
			return true;
		}
	}
	return false;
}


void ServiceFileBackup()
{
	/*const std::string sourceDirectory = "D:\\bcit_from";
	const std::string outputZipFile = "D:\\bcit_to.zip";*/

	std::ifstream configFile(CONFIG_FILE);
	if (!configFile.is_open())
	{
		std::cerr << "Failed to open the config file." << std::endl;
		return;
	}

	// Читаем sourceDirectory и outputZipFile из файла
	std::string sourceDirectory, outputZipFile;
	std::getline(configFile, sourceDirectory);
	std::getline(configFile, outputZipFile);

	configFile.close();

	std::vector<std::string> filesToCompress;
	// Получение списка файлов в директории
	DIR* dir;
	struct dirent* entry;
	if ((dir = opendir(sourceDirectory.c_str())) != nullptr)
	{
		while ((entry = readdir(dir)) != nullptr)
		{
			if (entry->d_type == DT_REG)
			{
				filesToCompress.push_back(entry->d_name);
			}
		}
		closedir(dir);
	}
	else
	{
		perror("Failed to open the source directory");
		return;
	}


	////// Чтение шаблонов из конфигурационного файла
	//std::vector<std::string> templates;
	//std::string templateLine;
	//while (std::getline(configFile, templateLine))
	//{
	//	templates.push_back(templateLine);
	//}

	////// Получение списка файлов в директории и добавление подходящих файлов
	//DIR* dir;
	//struct dirent* entry;
	//if ((dir = opendir(sourceDirectory.c_str())) != nullptr)
	//{
	//	while ((entry = readdir(dir)) != nullptr)
	//	{
	//		if (entry->d_type == DT_REG)
	//		{
	//			std::string fileName = entry->d_name;
	//			if (isMatchingTemplate(fileName, templates))
	//			{
	//				filesToCompress.push_back(fileName);
	//			}
	//		}
	//	}
	//	closedir(dir);
	//}
	//else
	// {
	//	perror("Failed to open the source directory");
	//	return;
	//}

	if (filesToCompress.empty())
	{
		std::cout << "No files to compress in the source directory." << std::endl;
		return;
	}

	// Открытие архива ZIP
	struct zip* archive = zip_open(outputZipFile.c_str(), ZIP_CREATE | ZIP_TRUNCATE, nullptr);
	if (!archive)
	{
		std::cerr << "Failed to open the output ZIP file." << std::endl;
		return;
	}

	// Сжатие и добавление файлов в архив
	for (const std::string& fileName : filesToCompress)
	{
		std::string filePath = sourceDirectory + "\\" + fileName;

		struct zip_source* source = zip_source_file(archive, filePath.c_str(), 0, -1);
		if (!source)
		{
			std::cerr << "Failed to open the source file: " << filePath << std::endl;
			zip_close(archive);
			return;
		}

		zip_file_add(archive, fileName.c_str(), source, ZIP_FL_OVERWRITE);
	}

	zip_close(archive);

	std::cout << "Compression completed. Output ZIP file: " << outputZipFile << std::endl;
}