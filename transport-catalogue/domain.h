#pragma once

#include <string_view>
#include <set>
#include <unordered_map>
#include <vector>
#include <deque>
#include <variant>
#include <optional>

#include "geo.h"

namespace TransportCatalogue {
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

		struct RouteDataStorage {
			detail::RouteType type;
			std::deque<std::string_view> stations;
		};

		struct MapData {
			std::deque<geo::Coordinates> coords_way;
			std::vector<size_t> coords_count;
			std::deque <std::pair<std::string_view, geo::Coordinates>> bus_name;
			std::deque <std::pair<std::string_view, geo::Coordinates>> stop_name;
		};

		struct RoutingSettingsStorage {
			double bus_wait_time = 0;
			double bus_velocity = 0;
		};

		enum class EdgeDataType
		{
			Wait,
			Bus
		};

		struct EdgeData
		{
			EdgeDataType type;
			std::optional<std::string_view> stop_from;
			std::optional<int> stop_count;
			std::optional<std::string_view> bus;
			std::optional<double> time;
		};
	}
}