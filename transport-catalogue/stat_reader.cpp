#include "stat_reader.h"

using namespace TrancportCatalogue;

void output::Read(Catalogue& catalogue) {
	size_t data_count;
	std::string text;
	std::string bus;
	std::getline(std::cin, text);
	data_count = std::stoul(text);
	for (size_t i = 0; i < data_count; ++i) {
		std::getline(std::cin, text);
		if (text[0] == 'B') {
			Bus(text, catalogue);
		}
		else {
			Stop(text, catalogue);
		}
	}
}

void output::Bus(std::string& text, Catalogue& catalogue) {
	std::string bus;
	bus = text.substr(4, text.size());
	auto data = catalogue.ProcessBusData(bus);
	if (data.lenght == -1) {
		std::cout << text << ": not found" << std::endl;
	}
	else {
		std::cout << text << ": " << data.stops_count << " stops on route, " << data.unique_stops_count << " unique stops, ";
		std::cout.precision(6);
		std::cout << data.lenght << " route length, " << data.curvatur << " curvature" << std::endl;
	}
}

void output::Stop(std::string& text, Catalogue& catalogue) {
	std::string stop;
	stop = text.substr(5, text.size());
	if (!catalogue.KnownStop(stop)) {
		std::cout << "Stop " << stop << ": not found" << std::endl;
		return;
	}
	auto& buses = catalogue.FindStopData(stop).buses;
	if (buses.size() == 0) {
		std::cout << "Stop " << stop << ": no buses" << std::endl;
	}
	else {
		std::cout << "Stop " << stop << ": buses";
		for (auto& bus : buses) {
			std::cout << " " << bus;
		}
		std::cout << std::endl;
	}
}
