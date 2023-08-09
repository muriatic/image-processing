#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <regex>
#include "command-line-parser.h"


using namespace std;


commandLineParser::commandLineParser(int argc, char** argv)
{
	bool nextIsValue = false;
	for (int i = 1; i < argc; ++i)
	{
		auto pos = find(argumentsVector.begin(), argumentsVector.end(), argv[i]);

		// arg is in list
		if (pos != argumentsVector.end())
		{
			int idx = pos - argumentsVector.begin();
			argOrder.push_back(idx);
			arguments.push_back(argv[i]);
			nextIsValue = true;
		}
		else if (nextIsValue == true)
		{
			values.push_back(argv[i]);
			nextIsValue = false;
		}
	}

	// go through list of arguments and check if they are in the list of valid args
	for (int i = 0; i < arguments.size(); i++)
	{
		// check if particular argument is doesn't even exist
		if (find(argumentsVector.begin(), argumentsVector.end(), arguments[i]) == argumentsVector.end())
		{
			string errorMessage = "Argument: " + arguments[i] + " is not a valid argument for this program. These are valid arguments: \n";
			errorMessage += PrintValidArguments();

			cout << errorMessage;

			throw invalid_argument(errorMessage);
		}
	}

	// go through list of required arguments and check if those are in the argument list
	for (int i = 0; i < argumentsVector.size(); i++)
	{
		// check if particular argument is doesn't even exist
		if (find(arguments.begin(), arguments.end(), argumentsVector[i]) == arguments.end())
		{
			string errorMessage = "Argument: " + argumentsVector[i] + " is not found in the provided arguments for this program. These are required arguments: \n";
			errorMessage += PrintValidArguments();

			cout << errorMessage;

			throw invalid_argument(errorMessage);
		}
	}

	vector <string> commands;
	for (int i = 0; i < values.size(); i++)
	{
		auto pos = find(argOrder.begin(), argOrder.end(), i);
		int idx = pos - argOrder.begin();
		commands.push_back(values[idx]);
	}

	names = SplitString(commands[0], ';');
	imageNames = SplitString(commands[1], ';');
	sourceImageDirectory = commands[2];
	finalizedImageDirectory = commands[3];
	threads = stoi(commands[4]);

	if (names.size() != imageNames.size())
	{
		string errorMessage = "Different number of names and imageNames, ensure they are the same, they should be split by ';'. # of names found: " + to_string(names.size()) + ". # of imageNames found: " + to_string(imageNames.size()) + ".";

		cout << errorMessage;

		throw invalid_argument(errorMessage);
	}
}


string commandLineParser::PrintValidArguments() 
{
	string validArguments;
	for (int i = 0; i < argumentsVector.size(); i++)
	{
		validArguments += argumentsVector[i] + "\n";
	}

	return validArguments;
}


vector <string> commandLineParser::SplitString(string rawString, char delimiter)
{
	vector <string> delimitedStrings;

	std::stringstream strStream(rawString);
	string token;

	while (std::getline(strStream, token, delimiter)) {
		token = regex_replace(token, regex("^ +| +$|( ) +"), "$1");
		delimitedStrings.push_back(token);
	}

	return delimitedStrings;
}