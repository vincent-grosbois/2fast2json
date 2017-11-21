// tftj_main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
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
	Query my_res(keys);

	OutputReader* out = new OutputCsv(keys, argv[3], true);

	bool useAvx2 = argc == 4;

	int i = 0;
	while (infile.good())
	{
		std::getline(infile, line);
		if (line.empty())
			continue;
		out->newRecord(line);
		tftj::parse(line, my_res, *out, useAvx2);
		out->endRecord();
		++i;
	}
	out->endParsing();

	std::cout << tftj::c << '\n';
	std::cout << tftj::aa << '\n';
	std::cout << i << '\n';
	infile.close();


	return 0;
}