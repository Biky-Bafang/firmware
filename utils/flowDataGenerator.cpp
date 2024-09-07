#include <string>
#include <vector>
#include <dirent.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
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
int main()
{
}
