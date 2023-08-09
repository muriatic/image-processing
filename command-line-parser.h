#ifndef COMMAND_LINE_PROCESSOR_H
#define COMMAND_LINE_PROCESSOR_H


#include <vector>
#include <iostream>
#include <string>


using namespace std;


class commandLineParser
{
	public:
		vector <string> names;
		vector <string> imageNames;
		string sourceImageDirectory;
		string finalizedImageDirectory;
		int threads;

		commandLineParser(int argc, char** argv);

		string PrintValidArguments();

		vector <string> SplitString(string rawString, char delimiter);

	private:
		vector<string> argumentsVector = {
			"-names",
			"-imageNames",
			"-sourceDir",
			"-finalDir",
			"-threads"
		};

		vector <string> arguments;

		vector <string> values;

		vector <unsigned short int> argOrder;
};


#endif // !1
