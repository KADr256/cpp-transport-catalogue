#include "map_renderer.h"

TransportCatalogue::detail::MapData DataForMap(TransportCatalogue::Catalogue& catalogue) {
	TransportCatalogue::detail::MapData result;
	std::set<std::string_view> buf_bus;
	std::set<std::string_view> buf_stop;
	for (auto& bus : catalogue.bus) {
		if (bus.second.stations.empty() == false) {
			buf_bus.insert(bus.first);
		}
	}
	for (auto& bus : buf_bus) {
		auto iter = catalogue.bus.find(bus);

		result.bus_name.push_back(std::pair(bus, catalogue.FindStopData((*iter).second.stations[0]).coord));

		size_t coord_count = (*iter).second.stations.size();
		for (auto& station_sv : (*iter).second.stations) {
			auto& coord = catalogue.FindStopData(station_sv).coord;
			result.coords_way.push_back(coord);
		}
		if ((*iter).second.type == TransportCatalogue::detail::RouteType::twoway) {
			size_t station_count = coord_count;

			size_t last_pos = coord_count - 1;
			if ((*iter).second.stations[last_pos] != (*iter).second.stations[0]) {
				result.bus_name.push_back(std::pair(bus, catalogue.FindStopData((*iter).second.stations[last_pos]).coord));
			}

			for (size_t i = 1; i < station_count; ++i) {
				auto& station_sv = (*iter).second.stations[station_count - i - 1];
				auto& coord = catalogue.FindStopData(station_sv).coord;
				result.coords_way.push_back(coord);
			}
			coord_count = coord_count * 2 - 1;
		}
		result.coords_count.push_back(coord_count);
	}

	for (auto& stop : catalogue.stops) {
		if (stop.second.buses.empty() == false) {
			buf_stop.insert(stop.first);
		}
	}

	for (auto& stop : buf_stop) {
		auto iter = catalogue.stops.find(stop);
		result.stop_name.push_back(std::pair(stop, (*iter).second.coord));
	}

	return result;
}

void MapRenderer::PaintWays()
{
	std::deque <svg::Point> deq_point;
	for (auto& el : data_.coords_way) {
		deq_point.push_back(proj_(el));
	}
	size_t counter_a = 0;
	for (auto counter_b : data_.coords_count) {
		svg::Polyline poly;
		poly.SetFillColor("none");
		poly.SetStrokeColor(settings_.color_palette[counter_a++ % settings_.color_palette.size()]);
		poly.SetStrokeWidth(settings_.line_width);
		poly.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		poly.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		for (size_t i = 0; i < counter_b; ++i) {
			poly.AddPoint(deq_point.front());
			deq_point.pop_front();
		}
		doc_.Add(std::move(poly));
	}
}

void MapRenderer::PaintBusName() {
	int counter_a = -1;
	std::string bus;
	for (auto& el : data_.bus_name) {
		svg::Point point;
		if (bus != static_cast<std::string>(el.first)) {
			counter_a++;
		}
		bus = static_cast<std::string>(el.first);
		point = proj_(el.second);
		svg::Text text1;
		text1.SetPosition(point);
		text1.SetOffset({ settings_.bus_label_offset.first, settings_.bus_label_offset.second });
		text1.SetFontSize(settings_.bus_label_font_size);
		text1.SetFontFamily("Verdana");
		text1.SetFontWeight("bold");
		text1.SetData(bus);
		text1.SetFillColor(settings_.underlayer_color);
		text1.SetStrokeColor(settings_.underlayer_color);
		text1.SetStrokeWidth(settings_.underlayer_width);
		text1.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		text1.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		doc_.Add(std::move(text1));

		svg::Text text2;
		text2.SetPosition(point);
		text2.SetOffset({ settings_.bus_label_offset.first, settings_.bus_label_offset.second });
		text2.SetFontSize(settings_.bus_label_font_size);
		text2.SetFontFamily("Verdana");
		text2.SetFontWeight("bold");
		text2.SetData(bus);
		text2.SetFillColor(settings_.color_palette[counter_a % settings_.color_palette.size()]);
		doc_.Add(std::move(text2));
	}
}

void MapRenderer::PaintStop()
{
	for (auto& el : data_.stop_name) {
		svg::Point point;
		point = proj_(el.second);
		svg::Circle stop;
		stop.SetCenter(point);
		stop.SetRadius(settings_.stop_radius);
		stop.SetFillColor("white");
		doc_.Add(stop);
	}
}

void MapRenderer::PaintStopName()
{
	for (auto& el : data_.stop_name) {
		svg::Point point;
		point = proj_(el.second);
		svg::Text stop_under;
		svg::Text stop;
		stop_under.SetPosition(point);
		stop_under.SetOffset({ settings_.stop_label_offset.first,settings_.stop_label_offset.second });
		stop_under.SetFontSize(settings_.stop_label_font_size);
		stop_under.SetFontFamily("Verdana");
		stop_under.SetData(static_cast<std::string>(el.first));
		stop_under.SetFillColor(settings_.underlayer_color);
		stop_under.SetStrokeColor(settings_.underlayer_color);
		stop_under.SetStrokeWidth(settings_.underlayer_width);
		stop_under.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		stop_under.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		doc_.Add(stop_under);

		stop.SetPosition(point);
		stop.SetOffset({ settings_.stop_label_offset.first,settings_.stop_label_offset.second });
		stop.SetFontSize(settings_.stop_label_font_size);
		stop.SetFontFamily("Verdana");
		stop.SetData(static_cast<std::string>(el.first));
		stop.SetFillColor("black");
		doc_.Add(stop);
	}
}