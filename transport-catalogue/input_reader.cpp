#include "input_reader.h"

using namespace TrancportCatalogue;

void input::line::Buses(Catalogue& cataloge, std::unordered_set<std::string>& bus_wait) {
	for (auto& text : bus_wait) {
		std::string_view text_sv = text;
		std::string station_last;
		size_t pos;

		std::string bus;
		std::deque<std::string> bus_way;
		detail::RouteType type;

		text_sv.remove_prefix(4);
		bus = text_sv.substr(0, text_sv.find_first_of(':'));
		text_sv.remove_prefix(bus.size() + 1);

		pos = text_sv.find_first_not_of(' ');
		station_last = text_sv.substr((pos), text_sv.find_first_of(">-", pos) - pos - 1);
		text_sv.remove_prefix(station_last.size() + pos + 1);
		bus_way.push_back(station_last);

		if (text_sv[0] == '-') {
			type = detail::RouteType::twoway;
		}
		else {
			type = detail::RouteType::oneway;
		}

		while (!text_sv.empty()) {
			text_sv.remove_prefix(1);
			pos = text_sv.find_first_not_of(' ');
			station_last = text_sv.substr((pos), text_sv.find_first_of(">-", pos) - pos - 1);
			text_sv.remove_prefix(station_last.size() + pos);
			bus_way.push_back(std::move(station_last));
			if (text_sv.size() > 1) {
				text_sv.remove_prefix(1);
			}
		}

		cataloge.AddBus(bus, bus_way, type);
	}
}

void input::Read(Catalogue& cataloge,std::istream& in) {
	size_t data_count;
	std::string text;
	std::unordered_set<std::string> bus_wait;
	std::getline(in, text);

	data_count = std::stoul(text);
	for (size_t count = 0; count < data_count; ++count) {
		std::getline(in, text);
		if (text[0] == 'B') {
			bus_wait.insert(move(text));
		}
		else {
			line::Stop(cataloge, text);
		}
	}
	line::Buses(cataloge, bus_wait);
}

void input::line::Stop(Catalogue& catalog, std::string& text) {
	std::string_view text_sv = text;
	text_sv.remove_prefix(5);
	size_t pos = 0;
	std::string stop_name;
	std::string stop_name_dest;
	std::string buf;
	geo::Coordinates stop_coord;

	stop_name = text_sv.substr(0, text_sv.find_first_of(':'));
	text_sv.remove_prefix(stop_name.size() + 1);

	pos = text_sv.find_first_not_of(' ');
	buf = text_sv.substr((pos), text_sv.find_first_of(',', pos) - pos);
	text_sv.remove_prefix(buf.size() + pos + 1);
	stop_coord.lat = std::stod(buf);

	pos = text_sv.find_first_not_of(' ');
	buf = text_sv.substr(pos, text_sv.find_first_of(' ', pos) - pos);
	text_sv.remove_prefix(buf.size() + pos);
	stop_coord.lng = (std::stod(buf));
	catalog.AddStop(stop_name, stop_coord);

	if (!text_sv.empty()) {
		text_sv.remove_prefix(1);
	}

	while (!text_sv.empty()) {
		std::string distant_s;
		int distant;

		distant_s = text_sv.substr(0, text_sv.find_first_not_of("0123456789"));
		text_sv.remove_prefix(distant_s.size() + 5);
		distant = std::stoi(distant_s);
		stop_name_dest = text_sv.substr(0, text_sv.find_first_of(','));
		text_sv.remove_prefix(stop_name_dest.size());
		if (!text_sv.empty()) {
			text_sv.remove_prefix(2);
		}
		catalog.AddWay(stop_name, stop_name_dest, distant);
	}
}
