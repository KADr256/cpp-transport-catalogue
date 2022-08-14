#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"
#include <fstream>

int main() {
	/*
	std::ifstream in("tsC_case1_input.txt");
	std::streambuf* cinbuf = std::cin.rdbuf();
	std::cin.rdbuf(in.rdbuf());
	*/
	TrancportCatalogue::Catalogue catalogue= TrancportCatalogue::Catalogue();
	TrancportCatalogue::input::Read(catalogue);
	TrancportCatalogue::output::Read(catalogue);
 }