#pragma once

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <string_view>
#include <deque>

#include "domain.h"

namespace TransportCatalogue {
	class Catalogue {
	public:
		void AddStop(const std::string& stop_name, const  geo::Coordinates stop_data);

		void AddBus(const std::string& bus_name, const  std::deque<std::string>& bus_way, const  detail::RouteType type);

		void AddWay(const std::string& stop_name_start, const  std::string& stop_name_end, int distant);

		bool KnownStop(const std::string& stop_name);

		detail::StopData& FindStopData(std::string_view stop_name);

		detail::RouteDataOutput ProcessBusData(const std::string& bus_name);

		std::unordered_set<std::string> words;
		std::unordered_map <std::string_view, detail::RouteDataStorage> bus;
		std::unordered_map <std::string_view, detail::StopData> stops;
	private:
		detail::RouteDataOutput CalculBusDataOneway(detail::RouteDataStorage& storage);

		detail::RouteDataOutput CalculBusDataTwoway(detail::RouteDataStorage& storage);

		std::unordered_map <std::string_view, detail::RouteDataOutput> bus_output_;
	};
}