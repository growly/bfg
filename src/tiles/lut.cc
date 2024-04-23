#include "lut.h"

#include <set>

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include "../poly_line_cell.h"
#include "../poly_line_inflator.h"
#include "../routing_grid.h"
#include "../layout.h"
#include "../geometry/rectangle.h"
#include "../geometry/shape_collection.h"
#include "../atoms/sky130_buf.h"
#include "../atoms/sky130_dfxtp.h"
#include "../atoms/sky130_mux.h"
#include "../atoms/sky130_hd_mux2_1.h"
#include "../atoms/sky130_tap.h"
#include "../routing_via_info.h"
#include "../row_guide.h"

namespace bfg {
namespace tiles {


namespace {

std::string DescribePorts(const std::set<geometry::Port*> &ports) {
  std::vector<geometry::Port*> sorted_ports(ports.begin(), ports.end());
  std::sort(sorted_ports.begin(), sorted_ports.end(),
            [](geometry::Port *lhs, geometry::Port *rhs) {
              if (lhs->centre().x() == rhs->centre().x()) {
                return lhs->centre().y() < rhs->centre().y();
              }
              return lhs->centre().x() < rhs->centre().x();
            });
  std::vector<std::string> port_descriptions;
  for (geometry::Port *port : sorted_ports) {
    port_descriptions.push_back(
        absl::StrFormat("(%d, %d)", port->centre().x(), port->centre().y()));
  }
  return absl::StrJoin(port_descriptions, ", ");
}

}  // namespace

const Lut::LayoutConfig *Lut::GetLayoutConfiguration(size_t lut_size) {
  size_t num_configurations =
      sizeof(kLayoutConfigurations) / sizeof(kLayoutConfigurations[0]);
  for (size_t i = 0; i < num_configurations; ++i ){
    if (kLayoutConfigurations[i].first == lut_size) {
      return &kLayoutConfigurations[i].second;
    }
  }
  LOG(FATAL) << "No layout configuration for LUT size: " << lut_size;
  return nullptr;
}

bfg::Cell *Lut::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<bfg::Cell> lut_cell(new bfg::Cell("lut"));
  std::unique_ptr<bfg::Layout> layout(new bfg::Layout(db));
  std::unique_ptr<bfg::Circuit> circuit(new bfg::Circuit());

  const Lut::LayoutConfig layout_config =
      *Lut::GetLayoutConfiguration(lut_size_); 
  constexpr int64_t kMuxSize = 8;

  int64_t buf_y_pos = 0;

  bfg::atoms::Sky130Tap::Parameters tap_params = {
    .height_nm = 2720,
    .width_nm = 460
  };
  bfg::atoms::Sky130Tap tap_generator(tap_params, design_db_);
  bfg::Cell *tap_cell = tap_generator.GenerateIntoDatabase(
      "lut_dfxtp_tap_template");

  // We arrange the flip-flops in banks to flank the central mux arrangements,
  // and in such as way as to minimise the longest distance between the output
  // of one and the input of the next, for the scan chain.
  //
  // Let the flip flop cell be oriented left to right with input left and output
  // right, and denote this direction -->; then the arrangement is:
  //
  // +-----+-----+             +-----+-----+
  // | --> | --> | ----------> | --> | --> |
  // +-----+-----+             +-----+-----+
  // | <-- | <-- |             | <-- | <-- |
  // +-----+-----+             +-----+-----+
  // | --> | --> |             | --> | --> |
  // +-----+-----+             +-----+-----+
  // | <-- | <-- |             | <-- | <-- |
  // +-----+-----+             +-----+-----+
  //           ^                v
  //           scan in          scan out
  //
  // (the bottom-most row is rotated)
  //
  // Each of the left and right groups of memories is called a bank. Banks are
  // created in a separate layout object and then copied into the main layout
  // for convenience. This has the *in*convenient side effect of requiring us to
  // map instances from the originating layouts into the main layout.

  std::vector<Bank> banks;

  int64_t max_row_height = 0;
  for (size_t b = 0; b < layout_config.num_banks; ++b) {
    banks.push_back(Bank());
    Bank &bank = banks.back();
    bank.layout.reset(new bfg::Layout(db));

    int64_t y_pos = 0;
    size_t num_memories = 0;

    for (size_t j = 0; j < layout_config.bank_rows; j++) {
      size_t row_width = 0;

      bank.rows.push_back(RowGuide(
          {0, y_pos},     // Row lower-left point.
          bank.layout.get(),
          nullptr,        // FIXME
          design_db_));
      RowGuide &row = bank.rows.back();

      bank.memory_names.emplace_back();
      std::vector<std::string> &bank_memories = bank.memory_names.back();

      // Rotate j = 1, 3, 5, ...
      bool rotate_this_row = layout_config.rotate_first_row ?
          j % 2 == 0 : j % 2 != 0;
      row.set_rotate_instances(rotate_this_row);

      row.set_tap_cell(*tap_cell);
      
      for (size_t i = 0; i < layout_config.bank_columns; i++) {
        std::string instance_name = absl::StrFormat(
            "lut_dfxtp_%d_%d", b, num_memories);
        std::string cell_name = absl::StrCat(instance_name, "_template");
        bfg::atoms::Sky130Dfxtp::Parameters params;
        bfg::atoms::Sky130Dfxtp generator(params, design_db_);
        bfg::Cell *cell = generator.GenerateIntoDatabase(cell_name);
        //cell->layout()->ResetOrigin();

        geometry::Instance *installed = nullptr;
        if (rotate_this_row) {
          installed = row.InstantiateFront(instance_name, cell->layout());
          bank_memories.insert(bank_memories.begin(), instance_name);
        } else {
          installed = row.InstantiateBack(instance_name, cell->layout());
          bank_memories.push_back(instance_name);
        }

        ++num_memories;
      }

      row.Place();

      max_row_height = std::max(
          max_row_height, static_cast<int64_t>(row.Height()));

      y_pos += static_cast<int64_t>(row.Height());

      bank.layout->SavePoint(absl::StrCat("row_", j, "_lr"),
                             geometry::Point(row_width, y_pos));
    }
  }

  LOG_IF(FATAL, banks.size() < 1) << "Expected at least 1 bank by this point.";

  banks[0].layout->MoveTo(geometry::Point(0, 0));
  layout->AddLayout(*banks[0].layout, "bank_0");
  geometry::Rectangle left_bounds = layout->GetBoundingBox();

  bfg::atoms::Sky130Mux::Parameters mux_params;
  bfg::atoms::Sky130Mux mux(mux_params, design_db_);
  bfg::Cell *mux_cell = mux.GenerateIntoDatabase("sky130_mux");

  std::vector<geometry::Instance*> mux_order;

  size_t num_muxes = (1 << lut_size_) / kMuxSize;
  {
    // Muxes are positioned like so:
    //
    // | 4-LUT | 5-LUT | 6-LUT
    //                 
    // |   x   |   x   |   x x
    // | x     | x     | x     x
    // |       |   x   |   x x
    // |       | x     | x     x
    //
    // The number of columns is defined in the LayoutConfig struct in
    // kLayoutConfigurations. Here we must compute the position based on where
    // they are in this chain.
    size_t column_select = 0;
    int64_t column_spacing = 300;
    int64_t row_spacing = -500;
    int64_t mux_height = mux_cell->layout()->GetBoundingBox().Height();
    int64_t mux_width = mux_cell->layout()->GetBoundingBox().Width();
    int64_t effective_mux_height = mux_height + row_spacing;
    int64_t effective_mux_width = mux_width + column_spacing;

    int64_t x_pos = static_cast<int64_t>(
        left_bounds.Width()) + layout_config.mux_area_padding;
    int64_t y_offset = -(mux_height - 2 * max_row_height) / 2;
    int64_t y_pos = y_offset;

    size_t p = 0;
    for (size_t i = 0; i < num_muxes; ++i) {
      std::string mux_name = absl::StrCat("mux_", i);
      circuit->AddInstance(mux_name, mux_cell->circuit());
      int64_t x_shift = column_select * effective_mux_width;
      geometry::Instance geo_instance(
          mux_cell->layout(), geometry::Point {
              x_pos + static_cast<int64_t>(column_select * effective_mux_width),
              y_pos
          });
      geo_instance.set_name(mux_name);
      geometry::Instance *instance = layout->AddInstance(geo_instance);
      mux_order.push_back(instance);
      if (p < layout_config.mux_area_rows - 1) {
        y_pos += effective_mux_height;
        ++p;
      } else {
        column_select += 1;   // Alternate the alternation.
        x_pos += 2 * (effective_mux_width);
        y_pos = y_offset;
        p = 0;
      }

      // Alternates between 0 and 1.
      column_select = (column_select + 1) % 2;
    }
  }

  {
    // NOTE(aryap): Can only gracefully deal with two banks.
    int64_t x_pos =
        layout->GetBoundingBox().Width() + layout_config.mux_area_padding;
    for (size_t i = 1; i < banks.size(); ++i) {
      banks[i].layout->MoveTo(geometry::Point(x_pos, 0));
      layout->AddLayout(*banks[i].layout, absl::StrCat("bank_", i));
      x_pos += banks[i - 1].layout->GetBoundingBox().Width();
    }
  }

  std::unordered_map<std::string, geometry::Instance *const>
      all_instances_by_name;
  layout->GetInstancesByName(&all_instances_by_name);

  std::set<geometry::Port*> all_mux_ports;
  for (geometry::Instance *mux : mux_order) {
    mux->GetInstancePorts(&all_mux_ports);
  }

  {
    // Now that the banks prototype layouts are copied into the main layout, map
    // the memory instances by name into the actual objects.
    for (size_t b = 0; b < banks.size(); ++b) {
      Bank &bank = banks[b];
      for (size_t j = 0; j < bank.memory_names.size(); ++j) {
        std::vector<std::string> &row = bank.memory_names[j];
        bank.memories.emplace_back();
        for (size_t i = 0; i < row.size(); ++i) {
          const std::string &name = row[i];
          auto it = all_instances_by_name.find(name);
          LOG_IF(FATAL, it == all_instances_by_name.end())
              << "Could not find memory \"" << name << "\" in main layout";
          geometry::Instance *memory = it->second;
          // LOG(INFO) << "row " << j << ", col " << i << ": " << name
          //           << " -> " << memory;
          bank.memories.back().push_back(memory);
        }
      }
    }
  }

  // The required scan chain connections are enumerated in (source, sink) pairs
  // given by the names of the instances to be connected.
  std::set<std::pair<std::string, std::string>> scan_chain_pairs;
  {
    geometry::Instance *end_of_last_bank = nullptr;
    for (size_t b = 0; b < banks.size(); ++b) {
      Bank &bank = banks[b];

      for (size_t j = 0; j < bank.memories.size(); ++j) {
        bool rotate_this_row = layout_config.rotate_first_row ?
            j % 2 == 0 : j % 2 != 0;
        std::vector<geometry::Instance*> &row = bank.memories[j];

        // Connect flip flops next to each other in each row:
        for (size_t i = 0; i < row.size() - 1; ++i) {
          geometry::Instance *memory = row[i];
          geometry::Instance *next_memory = row[i + 1];

          // If the row is rotated, the direction of connection is reversed:
          if (rotate_this_row) {
            std::swap(memory, next_memory);
          }

          scan_chain_pairs.insert({memory->name(), next_memory->name()});
        }

        if (j == 0)
          continue;

        // There are also connections between rows, which depend on which rows
        // are rotated and which bank we're in (left or right):
        //    row[rotate_this_row ? 0 : row.size() - 1];
        std::vector<geometry::Instance*> &last_row = bank.memories[j - 1];

        geometry::Instance *start_of_this_row =
            rotate_this_row ? row.back() : row.front();
        geometry::Instance *end_of_this_row =
            rotate_this_row ? row.front() : row.back();
        geometry::Instance *start_of_last_row =
            rotate_this_row ? last_row.front() : last_row.back();
        geometry::Instance *end_of_last_row =
            rotate_this_row ? last_row.back() : last_row.front();
        if (b == 0) {
          scan_chain_pairs.insert(
              {end_of_last_row->name(), start_of_this_row->name()});
        } else {
          scan_chain_pairs.insert(
              {end_of_this_row->name(), start_of_last_row->name()});
        }

        if (j == bank.memories.size() - 1) {
          if (end_of_last_bank) {
            scan_chain_pairs.insert(
                {end_of_last_bank->name(), start_of_this_row->name()});
          }

          end_of_last_bank = end_of_this_row;
        }
      }
    }
  }

  {
    // Add input buffers. We need one buffer per LUT selector input, i.e. k
    // buffers for a k-LUT.
    int64_t buf_width = 0;
    RowGuide &upper_row = banks[0].rows[3];
    geometry::Point start_position = upper_row.LowerRight();
    for (size_t i = 0; i < lut_size_ - 1; ++i) {
      std::string instance_name = absl::StrFormat("buf_%d", i);
      std::string cell_name = absl::StrCat(instance_name, "_template");
      atoms::Sky130Buf::Parameters buf_params = {
        .width_nm = 1380,
        .height_nm = 2720,
        .nfet_0_width_nm = 520,
        .nfet_1_width_nm = 520,
        .pfet_0_width_nm = 790,
        .pfet_1_width_nm = 790
      };
      atoms::Sky130Buf buf_generator(buf_params, design_db_);
      bfg::Cell *buf_cell = buf_generator.GenerateIntoDatabase(cell_name);
      buf_cell->layout()->ResetY();
      geometry::Instance *instance = layout->AddInstance(
          geometry::Instance (
              buf_cell->layout(),
              start_position + geometry::Point(buf_width, 0)
              //geometry::Point {
              //-200, -200
              //}
          )
      );
      instance->set_rotation_degrees_ccw(upper_row.RotationDegreesCCW());
      geometry::Rectangle bounding_box = buf_cell->layout()->GetTilingBounds();
      buf_width += bounding_box.Width();
    }
    RowGuide &lower_row = banks[1].rows[0];
    start_position = lower_row.lower_left();
    for (size_t i = 0; i < 1; ++i) {
      std::string instance_name = absl::StrFormat("hd_mux2_1_%d", i);
      std::string cell_name = absl::StrCat(instance_name, "_template");
      atoms::Sky130HdMux21 active_mux2_generator({}, design_db_);
      bfg::Cell *active_mux2_cell = active_mux2_generator.GenerateIntoDatabase(
          cell_name);
      active_mux2_cell->layout()->ResetY();
      geometry::Rectangle bounding_box =
          active_mux2_cell->layout()->GetTilingBounds();
      buf_width += bounding_box.Width();
      geometry::Instance *instance = layout->AddInstance(
          geometry::Instance (
              active_mux2_cell->layout(),
              start_position - geometry::Point(buf_width, 0)
              //geometry::Point {
              //-200, -200
              //}
          )
      );
      instance->set_rotation_degrees_ccw(lower_row.RotationDegreesCCW());
    }
  }

  geometry::Rectangle pre_route_bounds = layout->GetBoundingBox();
  LOG(INFO) << "Pre-routing bounds: " << pre_route_bounds;

  RoutingGrid routing_grid(db);
  
  // The alt routing grid is used for cases where we want to switch routing
  // direction. Since the grids don't share state, they must be used
  // independently with care that their routes do not produce conflicts.
  RoutingGrid alt_routing_grid(db);

  // Set every property the RoutingGrid needs.
  //bfg::RoutingLayerInfo li_layer_info;
  //db.GetRoutingLayerInfo("li.drawing", &li_layer_info);
  //li_layer_info.direction = bfg::RoutingTrackDirection::kTrackHorizontal;
  //li_layer_info.area = pre_route_bounds;
  //li_layer_info.offset = 50;

  bfg::RoutingLayerInfo met1_layer_info;
  db.GetRoutingLayerInfo("met1.drawing", &met1_layer_info);
  met1_layer_info.direction = bfg::RoutingTrackDirection::kTrackHorizontal;
  met1_layer_info.area = pre_route_bounds;
  // TODO(aryap): If we want y = 735 to be on the grid, and we know the offset
  // is relative to the pre_route_bounds lower-left y = -600, 
  // (735 - (-190)) / 340 (the pitch) = 3.9265
  //    offset = .3.9265 * 340
  //           = 315
  met1_layer_info.offset = 330;

  bfg::RoutingLayerInfo met2_layer_info;
  db.GetRoutingLayerInfo("met2.drawing", &met2_layer_info);
  met2_layer_info.direction = bfg::RoutingTrackDirection::kTrackVertical;
  met2_layer_info.area = pre_route_bounds;
  met2_layer_info.offset = 50;

  // TODO(aryap): Store connectivity information (which layers connect through
  // which vias) in the PhysicalPropertiesDatabase's via_layers_.
  bfg::RoutingViaInfo routing_via_info =
      db.GetRoutingViaInfo("met1.drawing", "met2.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid.AddRoutingViaInfo(
      met1_layer_info.layer, met2_layer_info.layer, routing_via_info);
  //alt_routing_grid.AddRoutingViaInfo(
  //    met1_layer_info.layer, met2_layer_info.layer, routing_via_info);

  routing_via_info = db.GetRoutingViaInfo("li.drawing", "met1.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid.AddRoutingViaInfo(
      met1_layer_info.layer, db.GetLayer("li.drawing"), routing_via_info);
  //alt_routing_grid.AddRoutingViaInfo(
  //    met1_layer_info.layer, db.GetLayer("li.drawing"), routing_via_info);

  routing_via_info = db.GetRoutingViaInfo("met2.drawing", "met3.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid.AddRoutingViaInfo(
      db.GetLayer("met3.drawing"), met2_layer_info.layer, routing_via_info);
  //alt_routing_grid.AddRoutingViaInfo(
  //    db.GetLayer("met3.drawing"), met2_layer_info.layer, routing_via_info);

  //routing_grid.AddRoutingLayerInfo(li_layer_info);
  routing_grid.AddRoutingLayerInfo(met1_layer_info);
  routing_grid.AddRoutingLayerInfo(met2_layer_info);

  routing_grid.ConnectLayers(met1_layer_info.layer, met2_layer_info.layer);

  // Swap direction for the alt routing grid:
  ////std::swap(met1_layer_info.direction, met2_layer_info.direction);
  //alt_routing_grid.AddRoutingLayerInfo(met1_layer_info);
  //alt_routing_grid.AddRoutingLayerInfo(met2_layer_info);
  //alt_routing_grid.ConnectLayers(
  //    met1_layer_info.layer, met2_layer_info.layer);

  {
    // Add blockages from all existing shapes.
    geometry::ShapeCollection shapes;
    layout->GetShapesOnLayer(db.GetLayer("met1.drawing"), &shapes);
    // LOG(INFO) << "met1 shapes: \n" << shapes.Describe();
    routing_grid.AddBlockages(shapes);
    //alt_routing_grid.AddBlockages(
    //    shapes, db.Rules("met1.drawing").min_separation);
  }
  {
    geometry::ShapeCollection shapes;
    layout->GetShapesOnLayer(db.GetLayer("met2.drawing"), &shapes);
    // LOG(INFO) << "met2 shapes: \n" << shapes.Describe();
    routing_grid.AddBlockages(shapes);
    //alt_routing_grid.AddBlockages(
    //    shapes, db.Rules("met2.drawing").min_separation);
  }

  // Debug only.
  //routing_grid.ExportVerticesAsSquares("areaid.frame", false, layout.get());

  // Connect the weird scan chain jumps across VDD/VSS met1 rails. Use layout
  // config to deduce where this ought to occur (instead of looking at it).
  //
  // Here I assume that the cells have a left-side input and a right-side output
  // when oriented normally (no rotation). This will change for different memory
  // types.
  //
  // On a bank that looks like this, we want to add jogs over the horizontal
  // rails:
  //            bank 0                    bank 1
  //         +-----+-----+             +-----+-----+
  //  3      |+ -> | --> | ----------> | --> | -> +|
  //         +|----+-----+             +-----+----| <--- jog
  //  2      |+ -- | <-- |             | <-- | <- +|
  //         +-----+-----+             +-----+-----+
  //  1      |+ -> | --> |             | --> | -> +|
  //         +|----+-----+             +-----+----| <--- jog
  //  0   /-> + -- | <-- |             | <-- | <- +|
  //      |  +-----+-----+             +-----+-----+
  //      jog   0     1                   0     1
  //
  //  For bank 0 we are tying together memories (row, col):
  //    (0, 0) and (1, 0), (2, 0) and (3, 0), ...
  //
  //  For bank 1 we are tying together memories (row, col):
  //    (0, 1) and (1, 1), (2, 1) and (3, 1), ...
  //
  // TODO(aryap): Why not just determine that these particular scan connections
  // require the alternate routing mode at the time of determining the scan
  // connections above? Seems like repeated logic :/
  std::map<geometry::Instance*, std::string> memory_output_net_names;
  {
    for (size_t b = 0; b < layout_config.num_banks; ++b) {
      const auto &memories = banks[b].memories;
      for (size_t j = layout_config.rotate_first_row ? 0 : 1;
           j < memories.size();
           j += 2) {
        // For the left bank (b == 0) pick the minimum (left) column, for the
        // right bank (b == 1) pick the maximum (right) column:
        size_t i = b == 0 ? 0 : memories[j].size() - 1;

        geometry::Instance *source = memories[j][i];
        geometry::Instance *sink = memories[j + 1][i];

        if (b != 0) {
          // The right hand bank has source and sink swapped, which matters when
          // looking up the right port on the instances below:
          std::swap(source, sink);
        }
        
        std::set<geometry::Port*> ports;
        source->GetInstancePorts("Q", &ports);
        geometry::Port *start = *ports.begin();
        ports.clear();

        sink->GetInstancePorts("D", &ports);
        geometry::Port *end = *ports.begin();

        std::string net_name = absl::StrCat(source->name(), "_Q");
        memory_output_net_names[source] = net_name;

        routing_grid.AddRouteBetween(*start, *end, all_mux_ports, net_name);

        LOG(INFO) << "b=" << b << ", j=" << j << ", i=" << i << " "
                  << source->name() << " -> " << sink->name()
                  << " start port: " << *start << " end: " << *end;

        scan_chain_pairs.erase({source->name(), sink->name()});
      }
    }
  }

  std::unique_ptr<bfg::Layout> grid_layout;
  // Now that we're done with the alt routing grid, add its shapes to the main
  // layout and update the regular routing grid with the alternate one's shapes
  // as blockages.
  //std::unique_ptr<bfg::Layout> grid_layout(alt_routing_grid.GenerateLayout());
  //layout->AddLayout(*grid_layout, "routing_alt");

  // TODO(aryap): This seems to be a common enough op to factor out. Would be 1
  // step nicer with GetShapesOnLayer returning the ShapeCollection directly,
  // and would be 2 steps nicer with RoutingGrid having the facility to
  // automatically add blockages from a Layout (for all the layers it itself
  // uses for routing).
  //{
  //  // Add blockages from all existing shapes.
  //  geometry::ShapeCollection shapes;
  //  grid_layout->GetShapesOnLayer(db.GetLayer("met1.drawing"), &shapes);
  //  LOG(INFO) << "met1 shapes: \n" << shapes.Describe();
  //  routing_grid.AddBlockages(shapes, db.Rules("met1.drawing").min_separation);
  //}
  //{
  //  geometry::ShapeCollection shapes;
  //  grid_layout->GetShapesOnLayer(db.GetLayer("met2.drawing"), &shapes);
  //  LOG(INFO) << "met2 shapes: \n" << shapes.Describe();
  //  routing_grid.AddBlockages(shapes, db.Rules("met2.drawing").min_separation);
  //}

  // Connect the scan chain.
  for (const auto &pair : scan_chain_pairs) {
    LOG(INFO) << "Adding scan routes for pair "
              << pair.first << ", " << pair.second;

    auto it = all_instances_by_name.find(pair.first);
    LOG_IF(FATAL, it == all_instances_by_name.end())
        << "Could not source memory \"" << name << "\" in main layout";
    geometry::Instance *source = it->second;

    it = all_instances_by_name.find(pair.second);
    LOG_IF(FATAL, it == all_instances_by_name.end())
        << "Could not sink memory \"" << name << "\" in main layout";
    geometry::Instance *sink = it->second;

    std::set<geometry::Port*> ports;
    source->GetInstancePorts("Q", &ports);
    geometry::Port *start = *ports.begin();
    ports.clear();

    sink->GetInstancePorts("D", &ports);
    geometry::Port *end = *ports.begin();

    std::string net_name = absl::StrCat(source->name(), "_Q");
    memory_output_net_names[source] = net_name;

    routing_grid.AddRouteBetween(*start, *end, all_mux_ports, net_name);
  }

  // Connect memory outputs to the muxes in order:
  // - Connect the closest output with

  // Connect flip-flops to mux.

  // TODO(aryap): The mux should tell us this!
  std::vector<std::string> mux_input_order = {
    "input_1",
    "input_0",
    "input_2",
    "input_3",
    "input_7",
    "input_6",
    "input_4",
    "input_5",
  };

  // Connect the 8:1 mux inputs to each other
  {
    for (size_t i = 0; i < num_muxes; ++i) {
      for (size_t j = 0; j < kMuxSize; ++j) {
        geometry::Instance *mux = mux_order[i];
        std::set<geometry::Port*> ports;
        mux->GetInstancePorts(mux_input_order[j], &ports);
      }
    }
  }

  // Auto-route order:
  std::vector<geometry::Instance*> auto_route_order = {
    banks[0].memories[0][0],
    banks[0].memories[0][1],
    banks[0].memories[1][0],
    banks[0].memories[1][1],
    banks[0].memories[2][0],
    banks[0].memories[2][1],
    banks[0].memories[3][0],
    banks[0].memories[3][1],
    banks[1].memories[0][0],
    banks[1].memories[0][1],
    banks[1].memories[1][0],
    banks[1].memories[1][1],
    banks[1].memories[2][0],
    banks[1].memories[2][1],
    banks[1].memories[3][0],
    banks[1].memories[3][1]
  };

  for (size_t i = 0; i < auto_route_order.size(); ++i) {
    geometry::Instance *memory = auto_route_order[i];
    geometry::Instance *mux = mux_order[i / kMuxSize];
    std::string input_name = mux_input_order[i % kMuxSize];

    std::set<geometry::Port*> all_other_mux_ports(
        all_mux_ports.begin(), all_mux_ports.end());

    // Heuristically determine which mux port to use based on which which is
    // closest to the memory output, even if we're routing to the memory output
    // net instead of the port specifically.
    std::set<geometry::Port*> memory_ports;
    memory->GetInstancePorts("Q", &memory_ports);
    geometry::Port *memory_output = *memory_ports.begin();

    std::set<geometry::Port*> mux_ports_on_net;
    mux->GetInstancePorts(input_name, &mux_ports_on_net);

    geometry::Port *mux_port = mux->GetNearestPortNamed(*memory_output,
                                                        input_name);
    if (!mux_port) {
      continue;
    }
    LOG_IF(FATAL, mux_ports_on_net.find(mux_port) == mux_ports_on_net.end())
        << "Nearest port named " << input_name
        << " did not appear in list of all ports for same name";

    // Do not avoid any ports with the given name, since presumably they are
    // connectible.
    for (geometry::Port *port : mux_ports_on_net) {
      all_other_mux_ports.erase(port);
    }

    while (mux_port) {
      LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
                << " avoiding " << DescribePorts(all_other_mux_ports);

      bool path_found = false;
      auto named_output_it = memory_output_net_names.find(memory);
      if (named_output_it == memory_output_net_names.end()) {
        std::string net_name = absl::StrCat(memory->name(), "_Q");
        memory_output_net_names[memory] = net_name;
        LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
                  << " to " << memory->name();
        path_found = routing_grid.AddRouteBetween(
            *mux_port, *memory_output, all_other_mux_ports, net_name);
      } else {
        const std::string &target_net = named_output_it->second;
        LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
                  << " to net " << target_net;
        path_found = routing_grid.AddRouteToNet(
            *mux_port, target_net, all_other_mux_ports);
      }
      if (path_found) {
        break;
      }
      mux_ports_on_net.erase(mux_port);
      mux_port = mux_ports_on_net.empty() ? nullptr : *mux_ports_on_net.begin();
    }
  }

  //size_t j = 0;
  //while (j < flip_flops.size()) {
  //  for (size_t k = 0; k < kMuxSize && j < flip_flops.size(); ++k) {
  //    std::set<geometry::Port*> ff_ports;
  //    geometry::Instance *ff = flip_flops[j];
  //    ff->GetInstancePorts("D", &ff_ports);
  //    geometry::Port *start = *ff_ports.begin();

  //    std::set<geometry::Port*> ports;
  //    size_t mux_index = j / kMuxSize;
  //    geometry::Instance *mux = mux_order[mux_index];
  //    mux->GetInstancePorts(mux_input_order[k], &ports);

  //    if (ports.empty())
  //      continue;

  //    std::string net_name = absl::StrCat("net_", j, "_", k);

  //    geometry::Port *first_port = *ports.begin();
  //    LOG(INFO) << "Adding routes for (ff, mux) = (" << j << ", " << mux_index << ")";
  //    routing_grid.AddRouteBetween(*start, *first_port, net_name);

  //    //auto it = ports.begin();
  //    //it++;  // Skip first port.
  //    //while (it != ports.end()) {
  //    //  geometry::Port *port = *it;
  //    //  LOG(INFO) << "Adding routes for (ff, mux) = (" << j << ", "
  //    //            << mux_index << ")" << " to net " << net_name;
  //    //  // HACK HACK HACK
  //    //  start->set_layer(db.GetLayer("met1.drawing"));
  //    //  port->set_layer(db.GetLayer("met1.drawing"));
  //    //  routing_grid.AddRouteToNet(*port, net_name);
  //    //  it++;
  //    //}
  //    j++;
  //  }
  //}

  // Debug only.
  routing_grid.ExportVerticesAsSquares("areaid.frame", false, layout.get());
  routing_grid.ExportVerticesAsSquares("areaid.frameRect", true, layout.get());
  //routing_grid.ExportEdgesAsRectangles("areaid.frameRect", true, layout.get());

  grid_layout.reset(routing_grid.GenerateLayout());
  layout->AddLayout(*grid_layout, "routing");

  lut_cell->SetLayout(layout.release());
  lut_cell->SetCircuit(circuit.release());
  bfg::Cell *cell = lut_cell.release();
  cell->set_name(name);
  design_db_->ConsumeCell(cell);
  return cell;
}

}  // namespace atoms
}  // namespace bfg
