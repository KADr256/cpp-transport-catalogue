#include "map_renderer.h"

inline bool IsZero(double value) {
	return std::abs(value) < EPSILON;
}

void MapOut(SvgSetting& setting, TrancportCatalogue::detail::MapData& map, std::ostream& out) {
	std::vector<size_t> size_vec;
	std::deque <svg::Point> deq_point;
	svg::Document doc;
	SphereProjector sph_proj(map.coords_way.begin(), map.coords_way.end(), setting.width, setting.height, setting.padding);
	for (auto& el : map.coords_way) {
		deq_point.push_back(sph_proj(el));
	}

	size_t counter_a = 0;
	for (auto counter_b : map.coords_count) {
		svg::Polyline poly;
		poly.SetFillColor("none");
		poly.SetStrokeColor(setting.color_palette[counter_a++ % setting.color_palette.size()]);
		poly.SetStrokeWidth(setting.line_width);
		poly.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		poly.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		for (size_t i = 0; i < counter_b; ++i) {
			poly.AddPoint(deq_point.front());
			deq_point.pop_front();
		}
		doc.Add(std::move(poly));
	}

	counter_a = -1;
	std::string bus;
	for (auto& el : map.bus_name) {
		svg::Point point;
		if (bus != static_cast<std::string>(el.first)) {
			counter_a++;
		}
		bus = static_cast<std::string>(el.first);
		point = sph_proj(el.second);
		svg::Text text1;
		text1.SetPosition(point);
		text1.SetOffset({ setting.bus_label_offset.first, setting.bus_label_offset.second });
		text1.SetFontSize(setting.bus_label_font_size);
		text1.SetFontFamily("Verdana");
		text1.SetFontWeight("bold");
		text1.SetData(bus);
		text1.SetFillColor(setting.underlayer_color);
		text1.SetStrokeColor(setting.underlayer_color);
		text1.SetStrokeWidth(setting.underlayer_width);
		text1.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		text1.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		doc.Add(std::move(text1));

		svg::Text text2;
		text2.SetPosition(point);
		text2.SetOffset({ setting.bus_label_offset.first, setting.bus_label_offset.second });
		text2.SetFontSize(setting.bus_label_font_size);
		text2.SetFontFamily("Verdana");
		text2.SetFontWeight("bold");
		text2.SetData(bus);
		text2.SetFillColor(setting.color_palette[counter_a % setting.color_palette.size()]);
		doc.Add(std::move(text2));
	}

	for (auto& el : map.stop_name) {
		svg::Point point;
		point = sph_proj(el.second);
		svg::Circle stop;
		stop.SetCenter(point);
		stop.SetRadius(setting.stop_radius);
		stop.SetFillColor("white");
		doc.Add(stop);
	}

	for (auto& el : map.stop_name) {
		svg::Point point;
		point = sph_proj(el.second);
		svg::Text stop_under;
		svg::Text stop;
		stop_under.SetPosition(point);
		stop_under.SetOffset({ setting.stop_label_offset.first,setting.stop_label_offset.second });
		stop_under.SetFontSize(setting.stop_label_font_size);
		stop_under.SetFontFamily("Verdana");
		stop_under.SetData(static_cast<std::string>(el.first));
		stop_under.SetFillColor(setting.underlayer_color);
		stop_under.SetStrokeColor(setting.underlayer_color);
		stop_under.SetStrokeWidth(setting.underlayer_width);
		stop_under.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
		stop_under.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
		doc.Add(stop_under);

		stop.SetPosition(point);
		stop.SetOffset({ setting.stop_label_offset.first,setting.stop_label_offset.second });
		stop.SetFontSize(setting.stop_label_font_size);
		stop.SetFontFamily("Verdana");
		stop.SetData(static_cast<std::string>(el.first));
		stop.SetFillColor("black");
		doc.Add(stop);
	}

	doc.Render(out);
}