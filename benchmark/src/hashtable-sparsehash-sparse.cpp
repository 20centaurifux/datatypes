#include <fstream>
#include <iostream>
#include <string>
#include <sparsehash/sparse_hash_map>

/*
 * compile with:
 * $ g++ -O2 -std=c++11 hashtable-sparsehash-sparse.cpp -o hashtable-sparsehash-sparse
 */

using namespace std;

static void
benchmark()
{
	ifstream file;

	file.open("words.txt");

	if(file.is_open())
	{
		google::sparse_hash_map<string, size_t> table;

		string word;

		while(file >> word)
		{
			auto iter = table.find(word);

			if(iter == table.end())
			{
				table[word] = 1;
			}
			else
			{
				iter->second++;
			}
		}

		cout << table.size() << std::endl;

		size_t sum = 0;

		for(auto& kv : table)
		{
			sum += kv.second;
		}

		cout << sum << std::endl;
	}
}

int
main(void)
{
	clock_t start = clock();

	benchmark();

	float seconds = (float)(clock() - start) / CLOCKS_PER_SEC;

	cout << seconds << std::endl;

	return 0;
}

