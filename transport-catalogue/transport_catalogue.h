#pragma once

#include <unordered_map>
#include <unordered_set>
#include <set>
#include <string>
#include <string_view>
#include <deque>
#include <vector>

#include "domain.h"
#include <transport_catalogue.pb.h>//

namespace TransportCatalogue {
	class Catalogue {
	public:
		void AddStop(const std::string& stop_name, const  geo::Coordinates stop_data);

		void AddBus(const std::string& bus_name, const  std::deque<std::string>& bus_way, const  detail::RouteType type);

		void AddWay(const std::string& stop_name_start, const  std::string& stop_name_end, int distant);

		bool KnownStop(const std::string& stop_name);

		detail::StopData& FindStopData(std::string_view stop_name);

		detail::RouteData ProcessBusData(const std::string& bus_name);

		void CalculateAll();

		std::unordered_set<std::string> st_words;
		std::unordered_set<std::string> bus_words;

		std::unordered_map <std::string_view, detail::StopData> stops;
		std::unordered_map <std::string_view, detail::RouteData> bus_output;
	private:
		void CalculBusDataOneway(detail::RouteData& storage);

		void CalculBusDataTwoway(detail::RouteData& storage);
	};
}