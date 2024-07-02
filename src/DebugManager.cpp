#include "DebugManager.h"

DebugManager::DebugManager() {}

// make an color object to for instance get the color of red
String DebugManager::getColor(String color)
{
	String colorValue = "";
	if (color == "red")
	{
		colorValue = "\033[31m";
	}
	else if (color == "green")
	{
		colorValue = "\033[32m";
	}
	else if (color == "yellow")
	{
		colorValue = "\033[33m";
	}
	else if (color == "blue")
	{
		colorValue = "\033[34m";
	}
	else if (color == "magenta")
	{
		colorValue = "\033[35m";
	}
	else if (color == "cyan")
	{
		colorValue = "\033[36m";
	}
	else if (color == "white")
	{
		colorValue = "\033[37m";
	}
	return colorValue;
}

// make an an function to print the debug message to serial monitor. It should be "type: infoMessage" and an color value that defaults to white
void DebugManager::print(String type, String infoMessage, String color)
{
	// replace the color with the color void
	color = getColor(color);
	if (color == "")
	{
		color = getColor("white");
	}
	// serial print the message whith the color at the front
	Serial.printf("%s%s: %s%s", getColor(color), type.c_str(), infoMessage.c_str(), getColor("white"));
}
