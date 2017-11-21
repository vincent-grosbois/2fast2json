// tftj_main.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <string>
#include <fstream>
#include <vector>
#include <iostream>

#include "../2fast2json/tftj/tftj.h"

int main(int argc, char *argv[])
{
	std::string file = "D:\\dev\\mison\\companies.json";

	std::ifstream infile(file);

	std::string line;


	std::vector<std::string> keys;
	keys.push_back("name");
	keys.push_back("acquisition.price_amount");
	keys.push_back("image.available_sizes[1]");
	keys.push_back("image.available_sizes[1][1]");
	keys.push_back("_id");
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

	OutputReader* out = new OutputCsv(keys, "myfile.txt", true);

	int i = 0;
	while (infile.good())
	{
		std::getline(infile, line);
		if (line.empty())
			continue;
		out->newRecord(line);
		tftj::parse(line, my_res, *out);
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