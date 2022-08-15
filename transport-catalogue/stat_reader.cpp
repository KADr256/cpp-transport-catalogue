#include "stat_reader.h"

using namespace TrancportCatalogue;

void output::Read(Catalogue& catalogue, std::istream& in, std::ostream& out) {
	size_t data_count;
	std::string text;
	std::string bus;
	std::getline(in, text);
	data_count = std::stoul(text);
	for (size_t i = 0; i < data_count; ++i) {
		std::getline(in, text);
		if (text[0] == 'B') {
			Bus(text, catalogue, out);
		}
		else {
			Stop(text, catalogue, out);
		}
	}
}

void output::Bus(std::string& text, Catalogue& catalogue, std::ostream& out) {
	std::string bus;
	bus = text.substr(4, text.size());
	auto data = catalogue.ProcessBusData(bus);
	if (data.lenght == -1) {
		out << text << ": not found" << std::endl;
	}
	else {
		out << text << ": " << data.stops_count << " stops on route, " << data.unique_stops_count << " unique stops, ";
		out.precision(6);
		out << data.lenght << " route length, " << data.curvatur << " curvature" << std::endl;
	}
}

void output::Stop(std::string& text, Catalogue& catalogue, std::ostream& out) {
	std::string stop;
	stop = text.substr(5, text.size());
	if (!catalogue.KnownStop(stop)) {
		out << "Stop " << stop << ": not found" << std::endl;
		return;
	}
	auto& buses = catalogue.FindStopData(stop).buses;
	if (buses.size() == 0) {
		out << "Stop " << stop << ": no buses" << std::endl;
	}
	else {
		out << "Stop " << stop << ": buses";
		for (auto& bus : buses) {
			out << " " << bus;
		}
		out << std::endl;
	}
}
