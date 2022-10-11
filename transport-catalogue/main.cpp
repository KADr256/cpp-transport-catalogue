#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"
#include <fstream>

int main() {
	TrancportCatalogue::Catalogue catalogue = TrancportCatalogue::Catalogue();
	auto document = TrancportCatalogue::json_processing::Read(std::cin);
	TrancportCatalogue::json_processing::input::Process(catalogue, document);
	TrancportCatalogue::json_processing::output::Process(catalogue, document, std::cout);
}