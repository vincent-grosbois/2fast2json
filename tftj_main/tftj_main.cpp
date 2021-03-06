#include <string>
#include <fstream>
#include <vector>
#include <iostream>
#include <sstream>

#include "../tftj/tftj.h"

int main(int argc, char *argv[])
{

	if (argc < 4)
	{
		std::cout << "Missing parameter";
		return -1;
	}

	std::string file(argv[1]);

	std::ifstream infile(file);

	std::string line;

	std::vector<std::string> keys;

	std::stringstream ss(argv[2]);
	std::string item;
	auto ba = std::back_inserter(keys);
	while (std::getline(ss, item, ',')) {
		*(ba++) = item;
	}

	//a.push_back("_id.$oid");

	//set 1 :
	/*a.push_back("a");
	a.push_back("a.[1]");
	a.push_back("a.[1].c");
	a.push_back("a.[2]");
	*/
	//set 2:
	//a.push_back("a.b");
	tftj::Query my_res(keys);

	tftj::OutputReader* out = new tftj::OutputCsv(keys, argv[3], true);

	bool useAvx2 = !((argc >= 6) && (std::string(argv[5]) == "no_avx2"));

	int training_num = (argc >= 5) ? std::stoi(argv[4]) : -1;

	training_num /= 2;

	std::cout << "training num: " << training_num << '\n';

	if (useAvx2)
	{
		std::cout << "avx2 ON\n\n";
	}
	else
	{
		std::cout << "avx2 OFF\n\n";
	}

	int i = 0;
	tftj::Parser parser(my_res, *out, useAvx2, training_num);

	while (infile.good())
	{
		std::getline(infile, line);
		if (line.empty())
			continue;
		out->newRecord(line);
		parser.parse(line);
		out->endRecord();
		++i;
	}
	out->endParsing();

	//std::cout << i << '\n';
	infile.close();


	return 0;
}