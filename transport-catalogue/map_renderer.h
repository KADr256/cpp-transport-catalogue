#pragma once

#include "geo.h"
#include "domain.h"
#include "svg.h"
#include "transport_catalogue.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <optional>
#include <vector>

inline const double EPSILON = 1e-6;
inline bool IsZero(double value) {
	return std::abs(value) < EPSILON;
}

struct SvgSetting {
	double width;
	double height;
	double padding;
	double line_width;
	double stop_radius;
	int bus_label_font_size;
	std::pair<double, double> bus_label_offset;
	int stop_label_font_size;
	std::pair<double, double> stop_label_offset;
	svg::Color underlayer_color;
	double underlayer_width;
	std::vector<svg::Color> color_palette;
};

class SphereProjector {
public:
	template <typename PointInputIt>
	SphereProjector(PointInputIt points_begin, PointInputIt points_end,
		double max_width, double max_height, double padding)
		: padding_(padding)
	{
		if (points_begin == points_end) {
			return;
		}

		const auto [left_it, right_it] = std::minmax_element(
			points_begin, points_end,
			[](auto lhs, auto rhs) { return lhs.lng < rhs.lng; });
		min_lon_ = left_it->lng;
		const double max_lon = right_it->lng;

		const auto [bottom_it, top_it] = std::minmax_element(
			points_begin, points_end,
			[](auto lhs, auto rhs) { return lhs.lat < rhs.lat; });
		const double min_lat = bottom_it->lat;
		max_lat_ = top_it->lat;

		std::optional<double> width_zoom;
		if (!IsZero(max_lon - min_lon_)) {
			width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
		}

		std::optional<double> height_zoom;
		if (!IsZero(max_lat_ - min_lat)) {
			height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
		}

		if (width_zoom && height_zoom) {
			zoom_coeff_ = std::min(*width_zoom, *height_zoom);
		}
		else if (width_zoom) {
			zoom_coeff_ = *width_zoom;
		}
		else if (height_zoom) {
			zoom_coeff_ = *height_zoom;
		}
	}

	svg::Point operator()(geo::Coordinates coords) const {
		return {
			(coords.lng - min_lon_) * zoom_coeff_ + padding_,
			(max_lat_ - coords.lat) * zoom_coeff_ + padding_
		};
	}

private:
	double padding_;
	double min_lon_ = 0;
	double max_lat_ = 0;
	double zoom_coeff_ = 0;
};

TransportCatalogue::detail::MapData DataForMap(TransportCatalogue::Catalogue& catalogue);

class MapRenderer {
public:
	MapRenderer(SvgSetting& svg_set,TransportCatalogue::Catalogue& catalogue, std::ostream& out) :settings_(svg_set), data_(DataForMap(catalogue)),
		proj_(data_.coords_way.begin(), data_.coords_way.end(), settings_.width, settings_.height, settings_.padding) {
		PaintWays();
		PaintBusName();
		PaintStop();
		PaintStopName();
		doc_.Render(out);
	};

	void PaintWays();
	void PaintBusName();
	void PaintStop();
	void PaintStopName();
private:
	SvgSetting& settings_;
	TransportCatalogue::detail::MapData data_;
	svg::Document doc_;
	SphereProjector proj_;
};