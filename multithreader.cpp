#include <thread>
#include <vector>
#include <string>
#include "command-line-parser.h"
#include "image-processing.cpp"


using namespace std;


class ThreadHandler
{
	public:
		vector <string> totalNames;
		vector <string> totalImageNames;
		string sourceImageDirectory;
		string finalizedImageDirectory;
		int threads;

		// create vector of vector s
		vector<vector <string>> nameChunks;
		vector<vector <string>> imageChunks;

		vector <vector<string>> threadResults;

		ThreadHandler(commandLineParser cmdParser)
		{
			totalNames = cmdParser.names;
			totalImageNames = cmdParser.imageNames;
			sourceImageDirectory = cmdParser.sourceImageDirectory;
			finalizedImageDirectory = cmdParser.finalizedImageDirectory;
			threads = cmdParser.threads;

			ValidateNumberOfThreads(threads);

			NameAndImageChunks();
		}

		void ValidateNumberOfThreads(int t)
		{
			const auto numberOfThreads = std::thread::hardware_concurrency();

			bool validNumberOfThreads = false;

			if (numberOfThreads > t)
			{
				validNumberOfThreads = true;
			}

			if (!validNumberOfThreads)
			{
				string errorMessage = "Number of CPU threads exceeded, halting execution\nYou requested: " + to_string(t) + " threads; this is equal to or exceeds the " + to_string(numberOfThreads) + " threads available";
				
				cout << errorMessage << endl;

				throw exception();
			}

		}

		// create threads and return results
		void CreateThreads()
		{
			std::vector <std::thread> ThreadVector;
			std::vector<std::vector<std::string>> threadReturns(nameChunks.size());

			for (int i = 0; i < nameChunks.size(); i++)
			{
				ThreadVector.emplace_back([&, i]() { // capture by reference
					threadReturns[i] = SubProcessImages(nameChunks[i], imageChunks[i], sourceImageDirectory, finalizedImageDirectory); // Pass by reference here, make sure the object lifetime is correct
					});
			}

			for (std::thread& t : ThreadVector)
			{
				t.join();
			}

			threadResults = threadReturns;
		}


		// converts multilayered threadResults into a single string split by delimiters
		string ConvertThreadResultsToString(char delimiter = ';')
		{
			string resultsAsString;
			for (int i = 0; i < threadResults.size(); i++)
			{
				for (int j = 0; j < threadResults[i].size(); j++)
				{
					resultsAsString += threadResults[i][j];
					resultsAsString += delimiter;
				}

			}

			return resultsAsString;
		}


	private:
		// process images and intereact with ImageProcessor
		vector<string> SubProcessImages(vector <string> names, vector <string> imageNames, string sourceImageDirectory, string finalizedImageDirectory)
		{
			vector<string> saveFileNames;
			for (int i = 0; i < names.size(); i++)
			{
				ImageProcessor img_proc(names[i], imageNames[i], sourceImageDirectory, finalizedImageDirectory);

				img_proc.CenterFace();

				saveFileNames.push_back(img_proc.saveFileName);
			}

			return saveFileNames;
		}


		// creates name and image chunks
		void NameAndImageChunks()
		{
			vector <int> chunkSizes = CreateChunks(totalNames.size(), threads);

			// this keeps track of where the last chunk ended
			unsigned int lastIDX = 0;
			for (int i = 0; i < chunkSizes.size(); i++)
			{
				// where last chunk ended
				unsigned int start = lastIDX;

				// start + the length of this chunk minus 1 (starting idx of 0)
				unsigned int end = start + chunkSizes[i] - 1;

				// this is the ending but to ensure it starts on the next digit we add 1
				lastIDX = end + 1;

				vector <string> chunkOfNames = CreateSubChunk(start, end, totalNames);
				vector <string> chunkOfImages = CreateSubChunk(start, end, totalImageNames);


				nameChunks.push_back(chunkOfNames);
				imageChunks.push_back(chunkOfImages);

			}
		}


		// creates vector <string> from string vector given start and end (can exclude the end with endInclusive = false)
		vector <string> CreateSubChunk(int start, int end, vector <string> StringVector, bool endInclusive = true)
		{
			vector <string> SubChunk;
			// if we're not including the end, just decrement by 1
			if (!endInclusive)
			{
				end--;
			}

			for (int i = start; i <= end; i++)
			{
				string str = StringVector[i];
				SubChunk.push_back(str);
			}

			return SubChunk;
		}


		// integer overload of vector <string> CreateSubChunk()
		vector <int> CreateSubChunk(int start, int end, vector <int> IntVector, bool endInclusive = true)
		{
			vector <int> SubChunk;
			// if we're not including the end, just decrement by 1
			if (!endInclusive)
			{
				end--;
			}

			for (int i = start; i <= end; i++)
			{
				int integer = IntVector[i];
				SubChunk.push_back(integer);
			}

			return SubChunk;
		}


		// divide length into n number of chunks
		vector <int> CreateChunks(int length, int n)
		{
			vector<int> chunkSizes;

			int quotient = length / n;
			int remainder = length % n;

			if (remainder > 0)
			{
				for (int i = 0; i < n; i++)
				{
					int addition = 0;

					if (remainder > 0)
					{
						addition = 1;
						remainder--;
					}

					int chunkSize = quotient + addition;

					chunkSizes.push_back(chunkSize);
				}
			}
			else
			{
				for (int i = 0; i < n; i++)
				{
					chunkSizes.push_back(quotient);
				}
			}

			return chunkSizes;
		}
};