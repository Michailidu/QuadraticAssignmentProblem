#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <algorithm>
#include <thread>
#include <mutex>
#include <vector>


using namespace std;

int number_of_items;
std::vector<std::vector<int>> flows;
std::vector<std::vector<int>> distances;
vector<int> best_permutation = std::vector<int>();
int best_distance = INT_MAX;
std::mutex m;

vector<int> ithPermutation(int i)
{
	int j, k = 0;
	vector<int> fact = vector<int>();
	vector<int> perm = vector<int>();

	fact.push_back(1);
	while (++k < number_of_items)
		fact.push_back(fact.at(k - 1) * k);

	for (k = 0; k < number_of_items; ++k)
	{
		perm.push_back(i / fact.at(number_of_items - 1 - k));
		i = i % fact.at(number_of_items - 1 - k);
	}

	for (k = number_of_items - 1; k > 0; --k)
		for (j = k - 1; j >= 0; --j)
			if (perm.at(j) <= perm.at(k))
				perm.at(k)++;

	return perm;
}

void readMatricesFromFile(const std::string& filename) {
	std::ifstream file(filename);

	if (file.is_open()) {
		std::string line;
		if (std::getline(file, line)) {
			std::istringstream iss(line);
			if (iss >> number_of_items) {
				flows.resize(number_of_items, std::vector<int>(number_of_items, 0));
				distances.resize(number_of_items, std::vector<int>(number_of_items, 0));

				std::getline(file, line);
				for (int i = 0; i < number_of_items; ++i) {
					if (std::getline(file, line)) {
						std::istringstream iss(line);
						for (int j = 0; j < number_of_items; ++j) {
							if (!(iss >> distances[i][j])) {
								std::cerr << "Error reading distances matrix at row " << i << ", column " << j << std::endl;
								return;
							}
						}
					}
				}

				std::getline(file, line);
				for (int i = 0; i < number_of_items; ++i) {
					if (std::getline(file, line)) {
						std::istringstream iss(line);
						for (int j = 0; j < number_of_items; ++j) {
							if (!(iss >> flows[i][j])) {
								std::cerr << "Error reading flows matrix at row " << i << ", column " << j << std::endl;
								return;
							}
						}
					}
				}
			}
			else {
				std::cerr << "Error reading the number of items." << std::endl;
			}
		}
		file.close();
	}
	else {
		std::cerr << "Unable to open the file: " << filename << std::endl;
	}
}

int get_permutation_distance(vector<int> permutation) {
	int distance = 0;
	for (int i = 0; i < number_of_items; i++)
	{
		for (int j = i; j < number_of_items; j++)
		{
			distance += distances[permutation.at(i)][permutation.at(j)] * flows[i][j] * 2;
			if (distance > best_distance)
				return -i;
		}

	}
	return distance;
}
int factorial(int n) {
	return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

void get_permutations_distances(int i_from, int i_to)
{
	vector<int> permutation = ithPermutation(i_from);

	for (int i = i_from; i < i_to; i++)
	{
		int d = get_permutation_distance(permutation);
		if (d < 0)
		{
			int prev = permutation[-d];
			while (i < i_to && permutation[-d] == prev) {
				std::next_permutation(permutation.begin() + 1, permutation.end());
				i++;
			}
		}
		else {
			std::lock_guard<std::mutex> lock(m);
			if (d < best_distance) {
				best_distance = d;
				best_permutation = permutation;
				std::cout << std::hash<std::thread::id>{}(std::this_thread::get_id()) << " - ";
				std::cout << best_distance << std::endl;
				for (int i = 0; i < best_permutation.size(); i++)
				{
					std::cout << best_permutation.at(i) << " ";
				}
				std::cout << std::endl;
			}
			std::next_permutation(permutation.begin() + 1, permutation.end());
		}
	}
}

bool read_tsp_file(const char* fname)
{
	readMatricesFromFile(fname);

	int number_of_permutations = factorial(number_of_items);
		int worker_cnt = thread::hardware_concurrency();
		int chunksize = std::ceil(static_cast<float>(number_of_permutations) / static_cast<float>(worker_cnt));
		vector<thread> workers;


		for (int id = 0; id < worker_cnt; ++id)
		{
			int i_from = id * chunksize;
			int i_to = (id + 1) * chunksize;
			i_to = i_to < number_of_permutations ? i_to : number_of_permutations;
			workers.push_back(thread(get_permutations_distances, i_from, i_to));
		}

		for (thread& worker : workers)
		{
			worker.join();
		}
		cout << best_distance << endl;
		for (int i = 0; i < best_permutation.size(); i++)
		{
			std::cout << best_permutation.at(i);
		}
		std::cout << std::endl;
	return true;
}

int main()
{
	read_tsp_file("C:\\Users\\sofia\\Documents\\vsb\\7S\\pa\\cviceni\\2\\input.dat");
	return 0;
}
