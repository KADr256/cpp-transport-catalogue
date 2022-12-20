#pragma once
#include <memory>
#include <map>

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

namespace TransportCatalogue {
	namespace transport_router {
		class TransportRouter
		{
		public:
			inline TransportRouter(TransportCatalogue::Catalogue& cat, detail::RoutingSettingsStorage rss);

			auto BuildRoute(std::string_view from, std::string_view to) {
				return router.value().BuildRoute(stop_id_[from] * 2, stop_id_[to] * 2);
			}

			const detail::EdgeData& AccessEdgeData(size_t edge_id) {
				return edges_[edge_id];
			}

			TransportCatalogue::Catalogue& catalogue;
			detail::RoutingSettingsStorage routing_settings;
			graph::DirectedWeightedGraph<double> graph_tr;

			std::optional<graph::Router<double>> router;
		private:
			std::vector<detail::EdgeData> edges_;
			std::map<std::string_view, size_t> stop_id_;
		};

		TransportCatalogue::transport_router::TransportRouter::TransportRouter(TransportCatalogue::Catalogue& cat, detail::RoutingSettingsStorage rss) : catalogue(cat), routing_settings(rss), graph_tr(catalogue.stops.size() * 2) // Перенос в юсзз не удался
		{
			edges_.reserve(catalogue.stops.size() * catalogue.bus.size());
			size_t counter = 0;
			for (auto& el : catalogue.stops) {
				stop_id_.insert({ el.first,counter++ });
				graph_tr.AddEdge(graph::Edge<double>{ stop_id_[el.first] * 2, stop_id_[el.first] * 2 + 1, routing_settings.bus_wait_time});
				edges_.push_back({ detail::EdgeDataType::Wait,el.first,std::nullopt,std::nullopt,routing_settings.bus_wait_time });
			}
			for (auto& el1 : catalogue.bus) {
				for (auto iter1 = el1.second.stations.begin(); iter1 != el1.second.stations.end(); ++iter1) {
					double dist = 0.0;
					int depth = 0;
					if (iter1 + 1 != el1.second.stations.end()) {
						auto last_iter = iter1;
						for (auto iter2 = iter1 + 1; iter2 != el1.second.stations.end(); ++iter2) {
							++depth;
							auto& stop_data1 = catalogue.FindStopData(*last_iter);
							auto& stop_data2 = catalogue.FindStopData(*iter2);
							if (stop_data1.another_ways.count(*iter2)) {
								dist += stop_data1.another_ways[*iter2];
							}
							else if (stop_data2.another_ways.count(*last_iter)) {
								dist += stop_data2.another_ways[*last_iter];
							}
							else {
								dist += geo::ComputeDistance(stop_data1.coord, stop_data2.coord);
							}
							double copy = dist / routing_settings.bus_velocity;
							graph_tr.AddEdge(graph::Edge<double>{ stop_id_[*iter1] * 2 + 1, stop_id_[*iter2] * 2, copy});
							edges_.push_back({ detail::EdgeDataType::Bus,std::nullopt,depth,el1.first,copy });
							last_iter = iter2;
						}
					}
				}
				if (el1.second.type == detail::RouteType::twoway) {
					for (auto iter3 = el1.second.stations.rbegin(); iter3 != el1.second.stations.rend(); ++iter3) {
						double dist = 0.0;
						int depth = 0;
						if (iter3 + 1 != el1.second.stations.rend()) {
							auto last_iter = iter3;
							for (auto iter4 = iter3 + 1; iter4 != el1.second.stations.rend(); ++iter4) {
								++depth;
								auto& stop_data1 = catalogue.FindStopData(*last_iter);
								auto& stop_data2 = catalogue.FindStopData(*iter4);
								if (stop_data1.another_ways.count(*iter4)) {
									dist += stop_data1.another_ways[*iter4];
								}
								else if (stop_data2.another_ways.count(*last_iter)) {
									dist += stop_data2.another_ways[*last_iter];
								}
								else {
									dist += geo::ComputeDistance(stop_data1.coord, stop_data2.coord);
								}
								double copy = dist / routing_settings.bus_velocity;
								graph_tr.AddEdge(graph::Edge<double>{ stop_id_[*iter3] * 2 + 1, stop_id_[*iter4] * 2, copy});
								edges_.push_back({ detail::EdgeDataType::Bus,std::nullopt,depth,el1.first,copy });
								last_iter = iter4;
							}
						}
					}
				}
			}
			router.emplace(graph_tr);
		}
	}
}