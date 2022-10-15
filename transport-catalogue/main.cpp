#include "transport_catalogue.h"
#include "request_handler.h"
#include "json_reader.h"
#include <fstream>

int main() {
	TransportCatalogue::Catalogue catalogue = TransportCatalogue::Catalogue();
	TransportCatalogue::json_processing::JSONReader json_read(catalogue, std::cin, std::cout);
}