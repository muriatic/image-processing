#include "command-line-parser.h"
#include "multithreader.cpp"


using namespace std;


int main(int argc, char** argv)
{
	commandLineParser cmdParser(argc, argv);

	ThreadHandler thread_handler(cmdParser);

	thread_handler.CreateThreads();

	string returnMessage = thread_handler.ConvertThreadResultsToString(';');

	cout << returnMessage << endl;

	return 0;
}