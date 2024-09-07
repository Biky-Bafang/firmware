#include <string>
#include <vector>
#include <dirent.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <stdint.h>

// make an vector with through which folders it should go through
std::vector<std::string> folders = {"../data/flows/core", "../data/flows/custom"};

struct Variable
{
	std::string name;
	std::string type;  // Example: "integer", "string", "boolean"
	std::string value; // Current value of the variable (stored as a string for simplicity)
	bool persist;	   // Whether the variable should be persisted across reboots
};

struct flowData
{
	std::string id;					   // ID (filename of the Lua file)
	std::string trigger_type;		   // Trigger type
	std::vector<uint8_t> trigger_data; // Hex trigger code (e.g., 0x11, 0x52)
	std::string lua_code;			   // Lua code as a string
	std::vector<Variable> variables;   // Variables needed for the Lua code
};
struct file
{
	std::string name;
	std::string path;
	std::string content;
};

std::vector<file> read_files(std::string path)
{
	std::vector<file> files;
	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir(path.c_str())) != NULL)
	{
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL)
		{
			std::string name = ent->d_name;
			if (name.find(".lua") != std::string::npos)
			{
				std::string file_path = path + "/" + name;
				std::ifstream file(file_path);
				std::string content((std::istreambuf_iterator<char>(file)),
									std::istreambuf_iterator<char>());
				files.push_back({name, file_path, content});
			}
		}
		closedir(dir);
	}
	else
	{
		/* could not open directory */
		perror("");
	}
	return files;
}

int main()
{
	std::vector<flowData> flows;
	for (std::string folder : folders)
	{
		std::vector<file> files = read_files(folder);
		for (file f : files)
		{
			// put it inside the flowData struct
			flowData flow;
			flow.id = f.name;
			flow.trigger_type = "trigger_type";
			flow.trigger_data = {0x11, 0x52};
			flow.variables = {{"variable1", "integer", "0", true}, {"variable2", "string", "hello", false}};
			flow.lua_code = f.content;
			flows.push_back(flow);

			// dump the flowData struct to a <FILENAME>.bin inside the same dir
			std::ofstream out(f.path.substr(0, f.path.size() - 4) + ".bin", std::ios::binary);
			out.write((char *)&flow, sizeof(flow));
			out.close();

			// Success message
			std::cout << "Successfully generated " << f.path.substr(0, f.path.size() - 4) + ".bin" << std::endl;
		}
	}
	return 0;
}
