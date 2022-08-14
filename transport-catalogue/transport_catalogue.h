#pragma once

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <string_view>
#include <deque>

#include "geo.h"

namespace TrancportCatalogue {
	namespace detail {
		enum class RouteType {
			oneway,
			twoway
		};

		struct RouteDataOutput {
			size_t stops_count = 0;
			size_t unique_stops_count = 0;
			double lenght = 0;
			double curvatur = 0;
		};

		struct StopData {
			geo::Coordinates coord;
			std::set<std::string_view> buses;
			std::unordered_map<std::string_view, int> another_ways;
		};
	}

	class Catalogue {
	public:

		void AddStop(std::string& stop_name, geo::Coordinates stop_data);

		void AddBus(std::string& bus_name, std::deque<std::string>& bus_way, detail::RouteType type);

		void AddWay(std::string& stop_name_start, std::string& stop_name_end, int distant);

		bool KnownStop(std::string& stop_name);

		detail::StopData& FindStopData(std::string& stop_name);

		detail::RouteDataOutput ProcessBusData(std::string& bus_name);

		std::unordered_set<std::string> words_;
	private:

		struct RouteDataStorage {
			detail::RouteType type;
			std::deque<std::string_view> stations;
		};

		detail::RouteDataOutput CalculBusDataOneway(RouteDataStorage& storage);

		detail::RouteDataOutput CalculBusDataTwoway(RouteDataStorage& storage);

		std::unordered_map <std::string_view, detail::StopData> stops_;
		std::unordered_map <std::string_view, RouteDataStorage> bus_;
		std::unordered_map <std::string_view, detail::RouteDataOutput> bus_output_;
	};
}