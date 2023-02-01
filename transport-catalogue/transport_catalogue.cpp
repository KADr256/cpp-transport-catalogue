#include "transport_catalogue.h"

using namespace TransportCatalogue;

void Catalogue::AddStop(const std::string& stop_name, const  geo::Coordinates stop_data) {
	std::string_view stop_name_sv = *st_words.insert(stop_name).first;
	stops.insert({ stop_name_sv,{stop_data,{},{}} });
}

void Catalogue::AddBus(const std::string& bus_name, const std::deque<std::string>& bus_way, const detail::RouteType type) {
	std::deque<std::string_view> bus_way_sv;

	std::string_view bus_name_sv = *bus_words.insert(bus_name).first;
	for (auto& stop : bus_way) {
		std::string_view stop_name_sv = *st_words.insert(stop).first;
		stops[stop_name_sv].buses.insert(bus_name_sv);
		bus_way_sv.push_back(stop_name_sv);
	}
	bus_output.insert({bus_name_sv,{0,0,0,0,type,bus_way_sv}});
}

void Catalogue::AddWay(const std::string& stop_name_start, const std::string& stop_name_end, int distant) {
	std::string_view stop_name_start_sv = *st_words.insert(stop_name_start).first;
	std::string_view stop_name_end_sv = *st_words.insert(stop_name_end).first;
	stops[stop_name_start_sv].another_ways.insert({ stop_name_end_sv, distant });
}

bool Catalogue::KnownStop(const std::string& stop_name) {
	return stops.count(stop_name);
}

detail::StopData& Catalogue::FindStopData(std::string_view stop_name) {
	return stops[stop_name];
}

detail::RouteData Catalogue::ProcessBusData(const std::string& bus_name) {
	if (!bus_output.count(bus_name)) {
		return { 0,0,-1,-1,detail::RouteType::oneway,{}};
	}
	return bus_output[bus_name];
}

void Catalogue::CalculBusDataOneway(detail::RouteData& storage) {
	++storage.stops_count;
	++storage.unique_stops_count;
	double geo_length = 0;
	auto iter = storage.stations.begin();
	std::string_view last_stop_sv = *iter;
	geo::Coordinates last_stop_coord = stops[last_stop_sv].coord;
	std::unordered_set<std::string_view> known_stop;

	known_stop.insert(last_stop_sv);

	while (iter + 1 != storage.stations.end()) {
		std::string_view now_stop_sv = *(iter + 1);
		geo::Coordinates now_stop_coord = stops[now_stop_sv].coord;

		if (stops[last_stop_sv].another_ways.count(now_stop_sv)) {
			storage.lenght += stops[last_stop_sv].another_ways[now_stop_sv];
			geo_length += ComputeDistance(last_stop_coord, now_stop_coord);
		}
		else if (stops[now_stop_sv].another_ways.count(last_stop_sv)) {
			storage.lenght += stops[now_stop_sv].another_ways[last_stop_sv];
			geo_length += ComputeDistance(last_stop_coord, now_stop_coord);
		}
		else {
			double buf = ComputeDistance(last_stop_coord, now_stop_coord);
			storage.lenght += buf;
			geo_length += buf;
		}

		++storage.stops_count;
		if (!known_stop.count(now_stop_sv)) {
			known_stop.insert(now_stop_sv);
			++storage.unique_stops_count;
		}
		last_stop_sv = now_stop_sv;
		last_stop_coord = now_stop_coord;
		++iter;
	}
	storage.curvatur = storage.lenght / geo_length;
}

void Catalogue::CalculBusDataTwoway(detail::RouteData& storage) {
	++storage.stops_count;
	++storage.unique_stops_count;
	double geo_length = 0;
	auto iter = storage.stations.begin();
	std::string_view last_stop_sv = *iter;
	geo::Coordinates last_stop_coord = stops[last_stop_sv].coord;
	std::unordered_set<std::string_view> known_stop;

	if (stops[last_stop_sv].another_ways.count(last_stop_sv)) {
		storage.lenght += stops[last_stop_sv].another_ways[last_stop_sv];
	}

	known_stop.insert(last_stop_sv);

	while (iter + 1 != storage.stations.end()) {
		std::string_view now_stop_sv = *(iter + 1);
		geo::Coordinates now_stop_coord = stops[now_stop_sv].coord;
		bool last_now_way = stops[last_stop_sv].another_ways.count(now_stop_sv);
		bool now_last_way = stops[now_stop_sv].another_ways.count(last_stop_sv);

		if (last_now_way && now_last_way) {
			storage.lenght += stops[last_stop_sv].another_ways[now_stop_sv];
			storage.lenght += stops[now_stop_sv].another_ways[last_stop_sv];
			geo_length += 2 * ComputeDistance(last_stop_coord, now_stop_coord);
		}
		else if (last_now_way) {
			storage.lenght += 2 * stops[last_stop_sv].another_ways[now_stop_sv];
			geo_length += 2 * ComputeDistance(last_stop_coord, now_stop_coord);
		}
		else if (now_last_way) {
			storage.lenght += 2 * stops[now_stop_sv].another_ways[last_stop_sv];
			geo_length += 2 * ComputeDistance(last_stop_coord, now_stop_coord);
		}
		else {
			double buf = 2 * ComputeDistance(last_stop_coord, now_stop_coord);
			storage.lenght += buf;
			geo_length += buf;
		}

		storage.stops_count += 2;
		if (!known_stop.count(now_stop_sv)) {
			known_stop.insert(now_stop_sv);
			++storage.unique_stops_count;
		}
		last_stop_sv = now_stop_sv;
		last_stop_coord = now_stop_coord;
		++iter;
	}

	if (stops[last_stop_sv].another_ways.count(last_stop_sv)) {
		storage.lenght += stops[last_stop_sv].another_ways[last_stop_sv];
	}

	storage.curvatur = storage.lenght / geo_length;
}

void TransportCatalogue::Catalogue::CalculateAll()
{
	for (auto& el : bus_output) {
		if (el.second.type == detail::RouteType::oneway) {
			CalculBusDataOneway(el.second);
		}
		else{
			CalculBusDataTwoway(el.second);
		}
	}
}