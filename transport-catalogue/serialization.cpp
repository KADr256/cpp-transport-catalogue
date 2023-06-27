#include "serialization.h"

void TransportCatalogue::Serialization::TransferColorIn(const SaveLoad::Color& color_in, svg::Color& color)
{
	if (color_in.name() == "none") {
		std::monostate result;
		color = std::move(result);
	}
	else if (color_in.name() == "rgb") {
		svg::Rgb result(color_in.red(), color_in.green(), color_in.blue());
		color = std::move(result);
	}
	else if (color_in.name() == "rgba") {
		svg::Rgba result(color_in.red(), color_in.green(), color_in.blue(), color_in.opacity());
		color = std::move(result);
	}
	else {
		color = color_in.name();
	}
}

void TransportCatalogue::Serialization::TransferColorOut(const svg::Color& color_out, SaveLoad::Color& color)
{
	switch (color_out.index())
	{
	case 0:
		color.set_name("none");
		break;
	case 1:
		color.set_name(std::get<std::string>(color_out));
		break;
	case 2:
		color.set_name("rgb");
		color.set_red(std::get<svg::Rgb>(color_out).red);
		color.set_blue(std::get<svg::Rgb>(color_out).blue);
		color.set_green(std::get<svg::Rgb>(color_out).green);
		break;
	case 3:
		color.set_name("rgba");
		color.set_red(std::get<svg::Rgba>(color_out).red);
		color.set_blue(std::get<svg::Rgba>(color_out).blue);
		color.set_green(std::get<svg::Rgba>(color_out).green);
		color.set_opacity(std::get<svg::Rgba>(color_out).opacity);
		break;
	}
}

void TransportCatalogue::Serialization::SaveSvgSettings(const SvgSetting& svg_set, SaveLoad::SvgSetting& svg_set_out)
{
	svg_set_out.set_width(svg_set.width);
	svg_set_out.set_height(svg_set.height);
	svg_set_out.set_padding(svg_set.padding);
	svg_set_out.set_line_width(svg_set.line_width);
	svg_set_out.set_stop_radius(svg_set.stop_radius);
	svg_set_out.set_bus_label_font_size(svg_set.bus_label_font_size);
	svg_set_out.set_bus_label_offset1(svg_set.bus_label_offset.first);
	svg_set_out.set_bus_label_offset2(svg_set.bus_label_offset.second);
	svg_set_out.set_stop_label_font_size(svg_set.stop_label_font_size);
	svg_set_out.set_stop_label_offset1(svg_set.stop_label_offset.first);
	svg_set_out.set_stop_label_offset2(svg_set.stop_label_offset.second);

	SaveLoad::Color color;
	TransferColorOut(svg_set.underlayer_color, *svg_set_out.mutable_underlayer_color());

	svg_set_out.set_underlayer_width(svg_set.underlayer_width);
	for (auto& el : svg_set.color_palette) {
		svg_set_out.add_color_palette();
		TransferColorOut(el, *svg_set_out.mutable_color_palette(svg_set_out.color_palette_size() - 1));
		color.Clear();
	}
}

void TransportCatalogue::Serialization::LoadSvgSettings(SvgSetting& svg_set, const SaveLoad::SvgSetting& svg_set_in)
{
	svg_set.width = svg_set_in.width();
	svg_set.height = svg_set_in.height();
	svg_set.padding = svg_set_in.padding();
	svg_set.line_width = svg_set_in.line_width();
	svg_set.stop_radius = svg_set_in.stop_radius();
	svg_set.bus_label_font_size = svg_set_in.bus_label_font_size();
	svg_set.bus_label_offset = { svg_set_in.bus_label_offset1(),svg_set_in.bus_label_offset2() };
	svg_set.stop_label_font_size = svg_set_in.stop_label_font_size();
	svg_set.stop_label_offset = { svg_set_in.stop_label_offset1(),svg_set_in.stop_label_offset2() };

	TransferColorIn(svg_set_in.underlayer_color(), svg_set.underlayer_color);

	svg_set.underlayer_width = svg_set_in.underlayer_width();

	for (int i = 0; i < svg_set_in.color_palette_size(); ++i) {
		svg::Color buf;
		TransferColorIn(svg_set_in.color_palette()[i], buf);
		svg_set.color_palette.push_back(std::move(buf));
	}
}

void TransportCatalogue::Serialization::SaveTransportCatalogue(const Catalogue& catalogue, SaveLoad::Catalogue& catalogue_out)
{
	SaveLoad::Words words_out;
	std::vector<std::string_view>  st_table;
	st_table.reserve(catalogue.st_words.size());
	std::vector<std::string_view>  bus_table;
	bus_table.reserve(catalogue.bus_words.size());
	{
		SaveLoad::Words words_out;
		for (auto& word : catalogue.st_words) {
			words_out.add_st_words(word);
			st_table.push_back(word);
		}
		for (auto& word : catalogue.bus_words) {
			words_out.add_bus_words(word);
			bus_table.push_back(word);
		}
		*catalogue_out.mutable_words() = std::move(words_out);
	}

	{
		SaveLoad::Bus bus_out;
		for (auto& bus : catalogue.bus_output) {
			bus_out.add_word_id_bus(std::distance(bus_table.begin(), std::find(bus_table.begin(), bus_table.end(), bus.first)));
			SaveLoad::RouteData route_data_out;
			route_data_out.set_stops_count(bus.second.stops_count);
			route_data_out.set_unique_stops_count(bus.second.unique_stops_count);
			route_data_out.set_lenght(bus.second.lenght);
			route_data_out.set_curvatur(bus.second.curvatur);
			if (bus.second.type == detail::RouteType::oneway) {
				route_data_out.set_type(false);
			}
			else {
				route_data_out.set_type(true);
			}
			for (auto& stop : bus.second.stations) {
				route_data_out.add_word_id_stop(std::distance(st_table.begin(), std::find(st_table.begin(), st_table.end(), stop)));
			}
			bus_out.add_route_data();
			*bus_out.mutable_route_data(bus_out.route_data_size() - 1) = route_data_out;
		}
		*catalogue_out.mutable_bus() = std::move(bus_out);
	}

	{
		SaveLoad::Stop stop_out;
		for (auto& stop : catalogue.stops) {
			stop_out.add_word_id_stop(std::distance(st_table.begin(), std::find(st_table.begin(), st_table.end(), stop.first)));
			SaveLoad::StopData stop_data_out;
			stop_data_out.set_lat(stop.second.coord.lat);
			stop_data_out.set_lng(stop.second.coord.lng);
			for (auto& bus : stop.second.buses) {
				stop_data_out.add_word_id_bus(std::distance(bus_table.begin(), std::find(bus_table.begin(), bus_table.end(), bus)));
			}
			stop_out.add_stop_data();
			*stop_out.mutable_stop_data(stop_out.stop_data_size() - 1) = stop_data_out;
		}
		*catalogue_out.mutable_stop() = std::move(stop_out);
	}
}

void TransportCatalogue::Serialization::LoadTransportCatalogue(Catalogue& catalogue, const SaveLoad::Catalogue& catalogue_in, std::vector<std::string_view>& bus_table, std::vector<std::string_view>& st_table)
{
	catalogue.st_words.clear();
	catalogue.bus_words.clear();
	catalogue.stops.clear();
	catalogue.bus_output.clear();

	st_table.reserve(catalogue_in.words().st_words_size());
	bus_table.reserve(catalogue_in.words().bus_words_size());

	for (int i = 0; i < catalogue_in.words().st_words_size(); ++i) {
		auto buf = catalogue.st_words.insert(catalogue_in.words().st_words()[i]);
		st_table.push_back(*buf.first);
	}

	for (int i = 0; i < catalogue_in.words().bus_words_size(); ++i) {
		auto buf = catalogue.bus_words.insert(catalogue_in.words().bus_words()[i]);
		bus_table.push_back(*buf.first);
	}

	for (int i = 0; i < catalogue_in.bus().word_id_bus_size(); ++i) {
		detail::RouteData route_data_in;
		route_data_in.stops_count = catalogue_in.bus().route_data()[i].stops_count();
		route_data_in.unique_stops_count = catalogue_in.bus().route_data()[i].unique_stops_count();
		route_data_in.lenght = catalogue_in.bus().route_data()[i].lenght();
		route_data_in.curvatur = catalogue_in.bus().route_data()[i].curvatur();
		if (catalogue_in.bus().route_data()[i].type()) {
			route_data_in.type = detail::RouteType::twoway;
		}
		else {
			route_data_in.type = detail::RouteType::oneway;
		}
		for (int j = 0; j < catalogue_in.bus().route_data()[i].word_id_stop_size(); ++j) {
			route_data_in.stations.push_back(st_table[catalogue_in.bus().route_data()[i].word_id_stop()[j]]);
		}
		catalogue.bus_output.insert({ bus_table[catalogue_in.bus().word_id_bus()[i]],route_data_in });
	}

	for (int i = 0; i < catalogue_in.stop().word_id_stop_size(); ++i) {
		detail::StopData stop_data_in;
		stop_data_in.coord.lat = catalogue_in.stop().stop_data()[i].lat();
		stop_data_in.coord.lng = catalogue_in.stop().stop_data()[i].lng();
		for (int j = 0; j < catalogue_in.stop().stop_data()[i].word_id_bus_size(); ++j) {
			stop_data_in.buses.insert(bus_table[catalogue_in.stop().stop_data()[i].word_id_bus()[j]]);
		}
		catalogue.stops.insert({ st_table[catalogue_in.stop().word_id_stop()[i]],stop_data_in });
	}
}

void TransportCatalogue::Serialization::SaveTransportRouter(const Catalogue& catalogue, const transport_router::TransportRouter& tr_router, SaveLoad::TransportRouter& tr_router_out, const detail::RoutingSettingsStorage rss)
{
	std::vector<std::string_view>  st_table(catalogue.st_words.begin(), catalogue.st_words.end());
	std::vector<std::string_view>  bus_table(catalogue.bus_words.begin(), catalogue.bus_words.end());

	{
		SaveLoad::DirectedWeightedGraph dwgraph;
		for (auto& el1 : tr_router.graph_tr.edges) {
			SaveLoad::Edge edge_out;
			edge_out.set_from(el1.from);
			edge_out.set_to(el1.to);
			edge_out.set_weight(el1.weight);
			*dwgraph.add_edges() = edge_out;
		}
		for (auto& el1 : tr_router.graph_tr.incidence_lists) {
			SaveLoad::IncedenceListL1 inc_list_l1;
			for (auto& el2 : el1) {
				inc_list_l1.add_edge_id(el2);
			}
			*dwgraph.add_incedence_list() = inc_list_l1;
		}
		*tr_router_out.mutable_graph() = std::move(dwgraph);
	}

	{
		SaveLoad::Router router;
		for (auto& el1 : tr_router.router.value().routes_internal_data_) {
			SaveLoad::RoutesInternalDataL1 ridl1;
			for (auto& el2 : el1) {
				if (el2.has_value()) {
					ridl1.add_opt(true);
					SaveLoad::RouteInternalData data;
					data.set_weight(el2.value().weight);
					if (el2.value().prev_edge.has_value()) {
						data.set_opt(true);
						data.set_prev_edge(el2.value().prev_edge.value());
					}
					else {
						data.set_opt(false);
					}
					*ridl1.add_data_l1() = std::move(data);
				}
				else {
					ridl1.add_opt(false);
					ridl1.add_data_l1();
				}
			}
			*router.add_data_l2() = ridl1;
		}
		*tr_router_out.mutable_router() = std::move(router);
	}

	{
		for (auto& el1 : tr_router.edges) {
			SaveLoad::EdgeData edge_data;
			if (el1.type == detail::EdgeDataType::Wait) {
				edge_data.set_type(false);
				edge_data.set_stop_from_id(std::distance(st_table.begin(), std::find(st_table.begin(), st_table.end(), el1.stop_from.value())));
				edge_data.set_time(el1.time.value());
			}
			else {
				edge_data.set_type(true);
				edge_data.set_bus_id(std::distance(bus_table.begin(), std::find(bus_table.begin(), bus_table.end(), el1.bus.value())));
				edge_data.set_stop_count(el1.stop_count.value());
				edge_data.set_time(el1.time.value());
			}
			*tr_router_out.add_edges() = std::move(edge_data);
		}
	}

	for (auto& el1 : tr_router.stop_id) {
		tr_router_out.add_stop_id(std::distance(st_table.begin(), std::find(st_table.begin(), st_table.end(), el1.first)));
		tr_router_out.add_counter(el1.second);
	}

	tr_router_out.set_bus_wait_time(rss.bus_wait_time);
	tr_router_out.set_bus_velocity(rss.bus_velocity);
}

void TransportCatalogue::Serialization::LoadTransportRouter(Catalogue& catalogue, std::optional<transport_router::TransportRouter>& tr_router, const SaveLoad::TransportRouter& tr_router_in, std::vector<std::string_view>& bus_table, std::vector<std::string_view>& stop_table)
{
	std::vector <graph::Edge<double>> edges;
	for (int i = 0; i < tr_router_in.graph().edges_size(); ++i) {
		auto& edge_in = tr_router_in.graph().edges()[i];
		graph::Edge<double> edge{ static_cast<size_t>(edge_in.from()),static_cast<size_t>(edge_in.to()),edge_in.weight() };
		edges.push_back(std::move(edge));
	}
	std::vector <std::vector<size_t>> incidence_lists;
	for (int i = 0; i < tr_router_in.graph().incedence_list_size(); ++i) {
		std::vector<size_t> incidence_lists1;
		for (int j = 0; j < tr_router_in.graph().incedence_list()[i].edge_id_size(); ++j) {
			incidence_lists1.push_back(tr_router_in.graph().incedence_list()[i].edge_id()[j]);
		}
		incidence_lists.push_back(std::move(incidence_lists1));
	}
	graph::DirectedWeightedGraph<double> graph_in(std::move(edges), std::move(incidence_lists));


	std::vector<std::vector<std::optional<graph::Router<double>::RouteInternalData>>> routes_internal_data;
	for (int i = 0; i<tr_router_in.router().data_l2_size();++i) {
		auto& place1 = tr_router_in.router().data_l2();
		std::vector<std::optional<graph::Router<double>::RouteInternalData>> routes_data1;
		for (int j = 0; j < place1[i].opt_size();++j) {
			if (place1[i].opt()[j]) {
				std::optional<graph::Router<double>::RouteInternalData> routes_data2;
				if (place1[i].data_l1()[j].opt()) {
					routes_data2 = {place1[i].data_l1()[j].weight(),{place1[i].data_l1()[j].prev_edge()}};
				}
				else {
					routes_data2 = {place1[i].data_l1()[j].weight(),{}};
				}
				routes_data1.push_back(std::move(routes_data2));
			}
			else {
				routes_data1.push_back({});
			}
		}
		routes_internal_data.push_back(std::move(routes_data1));
	}

	std::vector<detail::EdgeData> edge_data_vec;
	for (int i = 0; i < tr_router_in.edges_size();++i) {
		if (tr_router_in.edges()[i].type()) {
			detail::EdgeData edge_data;
			edge_data.type = detail::EdgeDataType::Bus;
			edge_data.bus = bus_table[tr_router_in.edges()[i].bus_id()];
			edge_data.stop_count = tr_router_in.edges()[i].stop_count();
			edge_data.time = tr_router_in.edges()[i].time();
			edge_data_vec.push_back(edge_data);
		}
		else {
			detail::EdgeData edge_data;
			edge_data.type = detail::EdgeDataType::Wait;
			edge_data.stop_from = stop_table[tr_router_in.edges()[i].stop_from_id()];
			edge_data.time = tr_router_in.edges()[i].time();
			edge_data_vec.push_back(edge_data);
		}
	}

	std::map<std::string_view, size_t> stop_id_map;
	for (int i = 0; i < tr_router_in.stop_id_size(); ++i) {
		stop_id_map[stop_table[tr_router_in.stop_id()[i]]] = tr_router_in.counter()[i];
	}

	detail::RoutingSettingsStorage rss({ tr_router_in.bus_wait_time(),tr_router_in.bus_velocity() });

	tr_router.emplace(catalogue, rss, graph_in, edge_data_vec, stop_id_map, routes_internal_data);
}

//
void TransportCatalogue::Serialization::SaveAll(std::ostream& out, const TransportCatalogue::Catalogue& catalogue, const SvgSetting& svg_set, const transport_router::TransportRouter& tr_router, const detail::RoutingSettingsStorage rss)
{
	SaveLoad::TransportCatalogue tr_out;
	SaveTransportCatalogue(catalogue, *tr_out.mutable_catalogue());
	SaveSvgSettings(svg_set, *tr_out.mutable_svg_set());
	SaveTransportRouter(catalogue, tr_router, *tr_out.mutable_tr_router(),rss);
	tr_out.SerializePartialToOstream(&out);
}
//
void TransportCatalogue::Serialization::LoadAll(std::istream& in, TransportCatalogue::Catalogue& catalogue, SvgSetting& svg_set, std::optional<transport_router::TransportRouter>& tr_router)
{
	std::vector<std::string_view>  st_table;
	std::vector<std::string_view>  bus_table;
	SaveLoad::TransportCatalogue tr_in;
	tr_in.ParseFromIstream(&in);
	LoadTransportCatalogue(catalogue, tr_in.catalogue(),bus_table,st_table);
	LoadSvgSettings(svg_set, tr_in.svg_set());
	LoadTransportRouter(catalogue, tr_router, tr_in.tr_router(),bus_table,st_table);
}
