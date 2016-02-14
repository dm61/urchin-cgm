#include "app_keys.h"
#include "config.h"
#include "graph_element.h"
#include "layout.h"
#include "preferences.h"
#include "staleness.h"

static void plot_point(int x, int y, GContext *ctx) {
  graphics_fill_rect(ctx, GRect(x, y, GRAPH_POINT_SIZE, GRAPH_POINT_SIZE), 0, GCornerNone);
}

static void plot_tick(int x, int bottom_y, GContext *ctx) {
  graphics_fill_rect(ctx, GRect(x, bottom_y - BOLUS_TICK_HEIGHT, BOLUS_TICK_WIDTH, BOLUS_TICK_HEIGHT), 0, GCornerNone);
}

static int bg_to_y(int height, int bg) {
  // Graph lower bound, graph upper bound
  int graph_min = get_prefs()->bottom_of_graph;
  int graph_max = get_prefs()->top_of_graph;
  return (float)height - (float)(bg - graph_min) / (float)(graph_max - graph_min) * (float)height + 0.5f;
}

static int bg_to_y_for_point(int height, int bg) {
  int min = 0;
  int max = height - GRAPH_POINT_SIZE;

  int y = (float)bg_to_y(height, bg) - GRAPH_POINT_SIZE / 2.0f + 0.5f;
  if (y < min) {
    return min;
  } else if (y > max) {
    return max;
  } else {
    return y;
  }
}

static void graph_update_proc(Layer *layer, GContext *ctx) {
  int i, x, y;
  GSize size = layer_get_bounds(layer).size;

  GraphData *data = layer_get_data(layer);
  graphics_context_set_stroke_color(ctx, data->color);
  graphics_context_set_fill_color(ctx, data->color);
  int padding = graph_staleness_padding();

  // SGVs
  for(i = 0; i < data->count; i++) {
    // XXX: JS divides by 2 to fit into 1 byte
    int bg = data->sgvs[i] * 2;
    if(bg == 0) {
      continue;
    }
    x = size.w - GRAPH_POINT_SIZE * (1 + i + padding);
    y = bg_to_y_for_point(size.h, bg);
    plot_point(x, y, ctx);
  }

  // Boluses
  for(i = 0; i < data->count; i++) {
    bool bolus = data->boluses[i];
    if (bolus) {
      x = size.w - GRAPH_POINT_SIZE * (1 + i + padding);
      plot_tick(x, size.h, ctx);
    }
  }

  // Target range bounds
  uint16_t limits[2] = {get_prefs()->top_of_range, get_prefs()->bottom_of_range};
  bool is_top[2] = {true, false};
  for(i = 0; i < (int)ARRAY_LENGTH(limits); i++) {
    y = bg_to_y(size.h, limits[i]);
    for(x = 0; x < size.w; x += 2) {
      graphics_draw_pixel(ctx, GPoint(x + 1, y - 1));
      graphics_draw_pixel(ctx, GPoint(x, y));
      graphics_draw_pixel(ctx, GPoint(x + 1, y + 1));
      // Draw bounds symmetrically, on the inside of the range
      if (is_top[i]) {
        graphics_draw_pixel(ctx, GPoint(x, y + 2));
      } else {
        graphics_draw_pixel(ctx, GPoint(x, y - 2));
      }
    }
  }

  // Horizontal gridlines
  int h_gridline_frequency = get_prefs()->h_gridlines;
  if (h_gridline_frequency > 0) {
    int graph_min = get_prefs()->bottom_of_graph;
    int graph_max = get_prefs()->top_of_graph;
    for(int g = 0; g < graph_max; g += h_gridline_frequency) {
      if (g <= graph_min || g == limits[0] || g == limits[1]) {
        continue;
      }
      y = bg_to_y(size.h, g);
      for(x = 2; x < size.w; x += 8) {
        graphics_draw_line(ctx, GPoint(x, y), GPoint(x + 1, y));
      }
    }
  }
}

GraphElement* graph_element_create(Layer *parent) {
  GRect bounds = element_get_bounds(parent);

  Layer* graph_layer = layer_create_with_data(
    GRect(0, 0, bounds.size.w, bounds.size.h),
    sizeof(GraphData)
  );
  ((GraphData*)layer_get_data(graph_layer))->color = element_fg(parent);
  ((GraphData*)layer_get_data(graph_layer))->sgvs = malloc(GRAPH_MAX_SGV_COUNT * sizeof(char));
  ((GraphData*)layer_get_data(graph_layer))->boluses = malloc(GRAPH_MAX_SGV_COUNT * sizeof(char));
  layer_set_update_proc(graph_layer, graph_update_proc);
  layer_add_child(parent, graph_layer);

  ConnectionStatusComponent *conn_status = connection_status_component_create(parent, 1, 1);

  GraphElement *el = malloc(sizeof(GraphElement));
  el->graph_layer = graph_layer;
  el->conn_status = conn_status;
  return el;
}

void graph_element_destroy(GraphElement *el) {
  free(((GraphData*)layer_get_data(el->graph_layer))->sgvs);
  free(((GraphData*)layer_get_data(el->graph_layer))->boluses);
  layer_destroy(el->graph_layer);
  connection_status_component_destroy(el->conn_status);
  free(el);
}

void graph_element_update(GraphElement *el, DictionaryIterator *data) {
  int count = dict_find(data, APP_KEY_SGV_COUNT)->value->int32;
  count = count > GRAPH_MAX_SGV_COUNT ? GRAPH_MAX_SGV_COUNT : count;
  ((GraphData*)layer_get_data(el->graph_layer))->count = count;
  memcpy(
    ((GraphData*)layer_get_data(el->graph_layer))->sgvs,
    (char*)dict_find(data, APP_KEY_SGVS)->value->cstring,
    count * sizeof(char)
  );
  memcpy(
    ((GraphData*)layer_get_data(el->graph_layer))->boluses,
    (char*)dict_find(data, APP_KEY_BOLUSES)->value->cstring,
    count * sizeof(char)
  );
  layer_mark_dirty(el->graph_layer);
  connection_status_component_refresh(el->conn_status);
}

void graph_element_tick(GraphElement *el) {
  connection_status_component_refresh(el->conn_status);
}
