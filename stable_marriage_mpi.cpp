#include <mpi.h>

#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>
#include <vector>
#include <iterator>

int g_numProcs = 0;
int g_myId = 0;
int g_numAccepted = 0;

std::vector<int> g_rank;

void initRank(std::vector<int> & rank);
void printRank(std::ostream & out);

int main(int argc, const char * argv[])
{

	MPI_Init(&argc, const_cast<char***>(&argv));

	MPI_Comm_size(MPI_COMM_WORLD,&g_numProcs);
	MPI_Comm_rank(MPI_COMM_WORLD,&g_myId);

	std::ostringstream fileName;
	fileName << "output/";
	fileName << g_myId;
	fileName << ".out";
	std::ofstream out (fileName.str().data());

	if(g_numProcs < 2)
		return 0; // Need at least 2 people to make a stable matching

	if(g_numProcs & 1)
		return 0; // A stable matching require the number of people to be even

	if(g_myId >= g_numProcs / 2)
	{
		out << "I'm nr " << g_myId << " and I'm a man" << std::endl;
		initRank(g_rank);
		printRank(out);

		int buffer [2];
		bool matched;
		MPI_Status status;

		for(std::size_t i = 0; i < g_rank.size(); ++i)
		{
			matched = false;
			buffer[0] = 1;
			buffer[1] = g_myId;
			MPI_Ssend(buffer, 2, MPI_INT, g_rank[i], 0, MPI_COMM_WORLD);
			out << "Sent request to " << g_rank[i] << std::endl;
			while(matched)
			{
				MPI_Recv(buffer, 2, MPI_INT, g_rank[i], MPI_ANY_TAG, MPI_COMM_WORLD, &status);
				if(buffer[0] == 0)
				{
					++g_numAccepted;
					if(g_numAccepted == g_rank.size())
					{
						out.close();
						return 0;
					}
				}
				else if(buffer[0] == 1)
				{
					if(buffer[1] == 0)
					{
						matched = false;
						out << "Rejected by " << status.MPI_SOURCE << std::endl;
					}
					else
					{
						out << "Accepted by " << status.MPI_SOURCE << std::endl;
					}
				}
				else
				{
					out << "Error in buffer pos 0" << std::endl;
				}
			}
		}
		out << "Error : process " << g_myId << " was not matched" << std::endl; 
	}
	else
	{
		out << "I'm nr " << g_myId << " and I'm a woman" << std::endl;
		initRank(g_rank);
		printRank(out);

		int buffer [2];
		int current;
		int offer;
		MPI_Status status;

		while(true)		
		{
			MPI_Recv(buffer, 2, MPI_INT, g_myId, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
			out << "Recieved msg from " << status.MPI_SOURCE << std::endl;
			if(buffer[0] == 0)
			{
				++g_numAccepted;
				if(g_numAccepted == g_rank.size())
				{
					out.close();
					return 0;
				}
			}
			else if(buffer[0] == 1)
			{
				if(current < 0)
				{
					current = buffer[1];
					buffer[0] = 1;
					buffer[1] = 1;
					MPI_Ssend(buffer, 2, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
					buffer[0] = 0;
					// TODO broadcast
					out << "Accepted man " << current << " for the first time" << std::endl;
					continue;
				}
				offer = buffer[1];
				if(0 < std::distance(std::find(g_rank.begin(), g_rank.end(), current), 
					std::find(g_rank.begin(), g_rank.end(), offer)))
				{
					buffer[0] = 1;
					buffer[1] = 1;
					MPI_Ssend(buffer, 2, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
					buffer[0] = 1;
					buffer[1] = 0;
					MPI_Ssend(buffer, 2, MPI_INT, current, 0, MPI_COMM_WORLD);
					out << "Rejected current " << current << " and accepted " << offer << std::endl;
				}
				else
				{
					buffer[0] = 1;
					buffer[1] = 0;
					MPI_Ssend(buffer, 2, MPI_INT, status.MPI_SOURCE, 0, MPI_COMM_WORLD);
				}
			}
			else
			{
				out << "Error in buffer pos 0" << std::endl;
			}			
		}
	}
	out << "Error" << std::endl;
	out.close();
	MPI_Finalize();
	return 0;
}	
void initRank(std::vector<int> & rank)
{
	// Setrank equal to the number of men/women	
	rank.resize(g_numProcs / 2);

	// Generate indexes
	int count = 0;		 
	std::generate(rank.begin(), rank.end(), [&count]()
	{
		return count++;
	});

	// Shuffle ranks non-deterministally
	std::shuffle(rank.begin(), rank.end(), std::random_device());
}
void printRank(std::ostream & out)
{
	std::for_each(g_rank.begin(), g_rank.end(), [&out](int & value)
	{
		out << value << " ";
	});
	out << std::endl; 
}
