#include "transport_catalogue.h"

using namespace TrancportCatalogue;

void Catalogue::AddStop(const std::string& stop_name, const  geo::Coordinates stop_data) {
	std::string_view stop_name_sv = *words.insert(stop_name).first;
	stops.insert({ stop_name_sv,{stop_data,{},{}} });
}

void Catalogue::AddBus(const std::string& bus_name, const std::deque<std::string>& bus_way, const detail::RouteType type) {
	std::deque<std::string_view> bus_way_sv;

	std::string_view bus_name_sv = *words.insert(bus_name).first;
	for (auto& stop : bus_way) {
		std::string_view stop_name_sv = *words.insert(stop).first;
		stops[stop_name_sv].buses.insert(bus_name_sv);
		bus_way_sv.push_back(stop_name_sv);
	}
	bus.insert({ bus_name_sv,{type,bus_way_sv} });
}

void Catalogue::AddWay(const std::string& stop_name_start, const std::string& stop_name_end, int distant) {
	std::string_view stop_name_start_sv = *words.insert(stop_name_start).first;
	std::string_view stop_name_end_sv = *words.insert(stop_name_end).first;
	stops[stop_name_start_sv].another_ways.insert({ stop_name_end_sv, distant });
}

bool Catalogue::KnownStop(const std::string& stop_name) {
	return stops.count(stop_name);
}

detail::StopData& Catalogue::FindStopData(std::string_view stop_name) {
	return stops[stop_name];
}

detail::RouteDataOutput Catalogue::ProcessBusData(const std::string& bus_name) {
	if (!bus.count(bus_name)) {
		return { 0,0,-1,-1 };
	}
	if (bus_output_.count(bus_name)) {
		return bus_output_[bus_name];
	}
	auto& buf = bus[bus_name];
	if (buf.type == detail::RouteType::oneway) {
		return CalculBusDataOneway(buf);
	}
	return CalculBusDataTwoway(buf);
}

detail::RouteDataOutput Catalogue::CalculBusDataOneway(detail::RouteDataStorage& storage) {
	detail::RouteDataOutput data = { 1,1,0,0 };
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
			data.lenght += stops[last_stop_sv].another_ways[now_stop_sv];
			geo_length += ComputeDistance(last_stop_coord, now_stop_coord);
		}
		else if (stops[now_stop_sv].another_ways.count(last_stop_sv)) {
			data.lenght += stops[now_stop_sv].another_ways[last_stop_sv];
			geo_length += ComputeDistance(last_stop_coord, now_stop_coord);
		}
		else {
			double buf = ComputeDistance(last_stop_coord, now_stop_coord);
			data.lenght += buf;
			geo_length += buf;
		}

		++data.stops_count;
		if (!known_stop.count(now_stop_sv)) {
			known_stop.insert(now_stop_sv);
			++data.unique_stops_count;
		}
		last_stop_sv = now_stop_sv;
		last_stop_coord = now_stop_coord;
		++iter;
	}
	data.curvatur = data.lenght / geo_length;

	return data;
}

detail::RouteDataOutput Catalogue::CalculBusDataTwoway(detail::RouteDataStorage& storage) {
	detail::RouteDataOutput data = { 1,1,0,0 };
	double geo_length = 0;
	auto iter = storage.stations.begin();
	std::string_view last_stop_sv = *iter;
	geo::Coordinates last_stop_coord = stops[last_stop_sv].coord;
	std::unordered_set<std::string_view> known_stop;

	if (stops[last_stop_sv].another_ways.count(last_stop_sv)) {
		data.lenght += stops[last_stop_sv].another_ways[last_stop_sv];
	}

	known_stop.insert(last_stop_sv);

	while (iter + 1 != storage.stations.end()) {
		std::string_view now_stop_sv = *(iter + 1);
		geo::Coordinates now_stop_coord = stops[now_stop_sv].coord;
		bool last_now_way = stops[last_stop_sv].another_ways.count(now_stop_sv);
		bool now_last_way = stops[now_stop_sv].another_ways.count(last_stop_sv);

		if (last_now_way && now_last_way) {
			data.lenght += stops[last_stop_sv].another_ways[now_stop_sv];
			data.lenght += stops[now_stop_sv].another_ways[last_stop_sv];
			geo_length += 2 * ComputeDistance(last_stop_coord, now_stop_coord);
		}
		else if (last_now_way) {
			data.lenght += 2 * stops[last_stop_sv].another_ways[now_stop_sv];
			geo_length += 2 * ComputeDistance(last_stop_coord, now_stop_coord);
		}
		else if (now_last_way) {
			data.lenght += 2 * stops[now_stop_sv].another_ways[last_stop_sv];
			geo_length += 2 * ComputeDistance(last_stop_coord, now_stop_coord);
		}
		else {
			double buf = 2 * ComputeDistance(last_stop_coord, now_stop_coord);
			data.lenght += buf;
			geo_length += buf;
		}

		data.stops_count += 2;
		if (!known_stop.count(now_stop_sv)) {
			known_stop.insert(now_stop_sv);
			++data.unique_stops_count;
		}
		last_stop_sv = now_stop_sv;
		last_stop_coord = now_stop_coord;
		++iter;
	}

	if (stops[last_stop_sv].another_ways.count(last_stop_sv)) {
		data.lenght += stops[last_stop_sv].another_ways[last_stop_sv];
	}

	data.curvatur = data.lenght / geo_length;

	return data;
}

