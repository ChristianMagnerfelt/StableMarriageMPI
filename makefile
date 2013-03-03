all: stable_marriage_mpi.cpp
	mpicxx -o StableMarriageMPI stable_marriage_mpi.cpp -std=c++0x
clean:
	rm StableMarriageMPI
