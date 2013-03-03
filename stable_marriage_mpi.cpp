#include <mpi.h>

#include <iostream>
#include <random>
#include <algorithm>
#include <vector>

int g_numProcs = 0;
int g_myId = 0;

std::vector<int> g_rank;

void initRank(std::vector<int> & rank);

int main(int argc, const char * argv[])
{
	MPI_Init(&argc, const_cast<char***>(&argv));

	MPI_Comm_size(MPI_COMM_WORLD,&g_numProcs);
	MPI_Comm_rank(MPI_COMM_WORLD,&g_myId);

	if(g_numProcs < 2)
		return 0; // Need at least 2 people to make a stable matching

	if(g_numProcs & 1)
		return 0; // A stable matching require the number of people to be even

	if(g_myId > g_numProcs / 2)
	{
		std::cout << "I'm nr " << g_myId << " and I'm a man" << std::endl;
	initRank(g_rank);

	}
	else
	{
		std::cout << "I'm nr " << g_myId << " and I'm a woman" << std::endl;
	initRank(g_rank);

	}
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
