// MIT License
// Copyright © 2025 W.M.R Jap-A-Joe

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is furnished to do
// so.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "stw_runtime.hpp"
#include "stw_platform.hpp"

#if defined(STW_PLATFORM_WINDOWS)
	#include <windows.h>
#endif

#if defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
	#include <dlfcn.h>
	#include <unistd.h>
	#include <limits.h>
	#include <sys/wait.h>
#endif

#include <filesystem>
#include <iostream>
#include <cstring>
#include <stdexcept>
#include <memory>

namespace stw::runtime
{
	void *load_library(const std::string &filePath)
	{
		if (!std::filesystem::exists(std::filesystem::path(filePath)))
		{
			std::cerr << "File not found: " << filePath << '\n';
			return nullptr;
		}

		void *moduleHandle = nullptr;

#if defined(STW_PLATFORM_WINDOWS)
		moduleHandle = (void *)LoadLibrary(filePath.c_str());
		if (!moduleHandle)
			std::cerr << "Failed to load plugin: " << filePath << '\n';
#elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
		moduleHandle = dlopen(filePath.c_str(), RTLD_LAZY);
		if (!moduleHandle)
		{
			char *error = dlerror();
			std::cerr << "Failed to load plugin: " << filePath << ". Error: " << error << '\n';
		}
#endif

		return moduleHandle;
	}

	void unload_library(void *libraryHandle)
	{
		if (!libraryHandle)
			return;
#if defined(STW_PLATFORM_WINDOWS)
		FreeLibrary((HINSTANCE)libraryHandle);
#elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
		dlclose(libraryHandle);
#endif
	}

	void *get_symbol(void *libraryHandle, const std::string &symbolName)
	{
		if (!libraryHandle)
			return nullptr;

		void *s = nullptr;

#if defined(STW_PLATFORM_WINDOWS)
		s = (void *)GetProcAddress((HINSTANCE)libraryHandle, symbolName.c_str());
		if (s == nullptr)
			std::cerr << "Error: undefined symbol: " << symbolName << '\n';
#elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
		dlerror(); /* clear error code */
		s = dlsym(libraryHandle, symbolName.c_str());
		char *error = dlerror();

		if (error != nullptr)
			std::cerr << "Error: " << error << '\n';
#endif

		return s;
	}

	bool find_library_path(const std::string &libraryName, std::string &libraryPath)
	{
#if defined(STW_PLATFORM_WINDOWS)
		static char result[4096]; // Static buffer to hold result
		DWORD res = SearchPath(nullptr, libraryName.c_str(), nullptr, MAX_PATH, result, nullptr);
		if (res == 0)
			return false;
		int len = strlen(result);
		char *outputPath = new char[len + 1];
		std::memcpy(outputPath, result, len);
		outputPath[len] = '\0';
		libraryPath = std::string(outputPath);
		delete[] outputPath;
		return true;
#elif defined(STW_PLATFORM_LINUX)
		// Prepare the command to search the library
		char cmd[256];
		snprintf(cmd, sizeof(cmd), "ldconfig -p 2>/dev/null | grep %s", libraryName.c_str());

		FILE *pipe = popen(cmd, "r");

		if (!pipe)
		{
			std::cerr << "popen() failed\n";
			return false;
		}

		static char result[4096]; // Static buffer to hold result

		while (fgets(result, sizeof(result), pipe) != NULL)
		{
			// Find the path after the "=>" symbol
			char *pos = strstr(result, "=>");
			if (pos != NULL)
			{
				pos += 2; // Move pointer to the path
				// Trim whitespace
				while (*pos == ' ')
					pos++;
				// Remove newline character
				char *newline = strchr(pos, '\n');
				if (newline)
					*newline = '\0';
				pclose(pipe);
				int len = strlen(pos);
				char *outputPath = new char[len + 1];
				std::memcpy(outputPath, pos, len);
				outputPath[len] = '\0';
				libraryPath = std::string(outputPath);
				delete[] outputPath;
				return true;
			}
		}

		pclose(pipe);
		return false;
#elif defined(STW_PLATFORM_MAC)
		return false;
#else
		return false;
#endif
	}

	void set_current_working_directory(const std::string &directoryPath)
	{
#if defined(STW_PLATFORM_WINDOWS)
		if (!SetCurrentDirectory(directoryPath.c_str()))
			throw std::runtime_error("Failed to change directory: " + directoryPath);
#elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
		if (chdir(directoryPath.c_str()) != 0)
			throw std::runtime_error("Failed to change directory: " + directoryPath);
#endif
	}

	std::string get_current_working_directory()
	{
		constexpr size_t PATH_MAX_LENGTH = 4096;
		char buffer[PATH_MAX_LENGTH] = {0};
#if defined(STW_PLATFORM_WINDOWS)
		if (!GetCurrentDirectory(PATH_MAX_LENGTH, buffer))
			throw std::runtime_error("Failed to get current directory");
#elif defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
		if (getcwd(buffer, sizeof(buffer)) == nullptr)
			throw std::runtime_error("Failed to get current directory");
#endif
		return std::string(buffer);
	}

	bool run_command(const std::string &cmd, const std::vector<std::string> &args, std::string &output)
	{
		//Source: https://gist.github.com/meritozh/f0351894a2a4aa92871746bf45879157
		std::string command = cmd;
#if defined(STW_PLATFORM_LINUX) || defined(STW_PLATFORM_MAC)
		for(const auto &arg : args)
		{
			command += " " + arg;
		}

		command += " 2>&1"; // Add redirection to capture stderr

		std::shared_ptr<FILE> pipe(popen(command.c_str(), "r"), pclose);
		
		if (!pipe) return 
			false;
		
		char buffer[128];
		
		while (!feof(pipe.get())) 
		{
			if (fgets(buffer, 128, pipe.get()) != NULL)
				output += buffer;
		}
		return true;
#else
		return false;
#endif
	}
}