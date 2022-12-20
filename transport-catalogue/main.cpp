#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"
#include <fstream>

int main() {
	/*
	std::ifstream in("input.json");
	std::streambuf* cinbuf = std::cin.rdbuf();
	std::cin.rdbuf(in.rdbuf());

	std::ofstream out("stdout.txt");
	std::streambuf* coutbuf = std::cout.rdbuf();
	std::cout.rdbuf(out.rdbuf());

	std::ofstream out_err("stderr.txt");
	std::streambuf* cerrbuf = std::cerr.rdbuf();
	std::cerr.rdbuf(out_err.rdbuf());
	*/
	TransportCatalogue::Catalogue catalogue = TransportCatalogue::Catalogue();
	TransportCatalogue::json_processing::JSONReader json_read(catalogue, std::cin, std::cout);
}