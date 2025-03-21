#include "lut.h"

#include <set>

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include "../atoms/sky130_buf.h"
#include "../atoms/sky130_dfxtp.h"
#include "../atoms/sky130_hd_mux2_1.h"
#include "../atoms/sky130_mux.h"
#include "../atoms/sky130_tap.h"
#include "../checkerboard_guide.h"
#include "../equivalent_nets.h"
#include "../geometry/rectangle.h"
#include "../geometry/shape_collection.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "../poly_line_cell.h"
#include "../poly_line_inflator.h"
#include "../routing_grid.h"
#include "../routing_layer_info.h"
#include "../routing_via_info.h"
#include "../row_guide.h"

namespace bfg {
namespace tiles {

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
      "lut_tap_template");

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

  std::vector<MemoryBank> banks;

  int64_t max_row_height = 0;
  for (size_t b = 0; b < layout_config.num_banks; ++b) {
    banks.push_back(MemoryBank());
    MemoryBank &bank = banks.back();

    int64_t y_pos = 0;
    size_t num_memories = 0;

    for (size_t j = 0; j < layout_config.bank_rows; j++) {
      size_t row_width = 0;

      bank.rows().push_back(RowGuide(
          {0, y_pos},     // Row lower-left point.
          layout.get(),
          nullptr,        // FIXME
          design_db_));
      RowGuide &row = bank.rows().back();

      bank.instance_names().emplace_back();
      std::vector<std::string> &bank_memories = bank.instance_names().back();

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
          installed = row.InstantiateAndInsertFront(
              instance_name, cell->layout());
          bank_memories.insert(bank_memories.begin(), instance_name);
        } else {
          installed = row.InstantiateBack(instance_name, cell->layout());
          bank_memories.push_back(instance_name);
        }

        ++num_memories;
      }

      //row.Place();

      max_row_height = std::max(
          max_row_height, static_cast<int64_t>(row.Height()));

      y_pos += static_cast<int64_t>(row.Height());

      layout->SavePoint(absl::StrCat("row_", j, "_lr"),
                        geometry::Point(row_width, y_pos));
    }
  }

  LOG_IF(FATAL, banks.size() < 1) << "Expected at least 1 bank by this point.";

  banks[0].MoveTo(geometry::Point(0, 0));
  geometry::Rectangle left_bounds = layout->GetBoundingBox();

  bfg::atoms::Sky130Mux::Parameters mux_params;
  mux_params.extend_inputs_top = true;
  mux_params.extend_inputs_bottom = false;

  bfg::atoms::Sky130Mux mux(mux_params, design_db_);
  bfg::Cell *base_mux_cell = mux.GenerateIntoDatabase("sky130_mux");

  // A second version of the mux has its inputs on the bottom instead of the
  // top:
  bfg::atoms::Sky130Mux::Parameters alt_mux_params;
  alt_mux_params.extend_inputs_top = false;
  alt_mux_params.extend_inputs_bottom = true;

  bfg::Cell *alt_mux_cell = bfg::atoms::Sky130Mux(
      alt_mux_params, design_db_).GenerateIntoDatabase("alt_sky130_mux");

  // Muxes are positioned like so:
  //
  // | 4-LUT | 5-LUT | 6-LUT
  //                 
  // |       |   x   |   x x
  // |       | x     | x     x
  // |   x   |   x   |   x x
  // | x     | x     | x     x
  //
  // The number of columns is defined in the LayoutConfig struct in
  // kLayoutConfigurations. Here we must compute the position based on where
  // they are in this chain.
  int64_t mux_height = base_mux_cell->layout()->GetBoundingBox().Height();
  int64_t x_pos = static_cast<int64_t>(
      left_bounds.Width()) + layout_config.mux_area_padding;
  int64_t y_offset = -(mux_height - 2 * max_row_height) / 2;
  int64_t y_pos = y_offset;

  std::vector<bfg::Cell*> mux_templates = {base_mux_cell, alt_mux_cell};

  CheckerboardGuide mux_grid(geometry::Point(x_pos, y_pos),
                             "mux",
                             layout_config.mux_area_rows,
                             layout_config.mux_area_columns,
                             layout.get(),
                             nullptr,
                             design_db_);
  mux_grid.set_template_cells(&mux_templates);
  // TODO(aryap): This is a function of track pitch, really.
  mux_grid.set_horizontal_overlap(-300);
  mux_grid.set_vertical_overlap(-1500);
  const std::vector<geometry::Instance*> &mux_order = mux_grid.InstantiateAll();

  {
    // NOTE(aryap): Can only gracefully deal with two banks.
    int64_t x_pos =
        layout->GetBoundingBox().Width() + layout_config.mux_area_padding;
    for (size_t i = 1; i < banks.size(); ++i) {
      banks[i].MoveTo(geometry::Point(x_pos, 0));
      x_pos += banks[i - 1].GetBoundingBox()->Width();
    }
  }

  std::unordered_map<std::string, geometry::Instance *const>
      all_instances_by_name;
  layout->GetInstancesByName(&all_instances_by_name);

  {
    for (size_t b = 0; b < banks.size(); ++b) {
      MemoryBank &bank = banks[b];
      for (size_t j = 0; j < bank.instance_names().size(); ++j) {
        std::vector<std::string> &row = bank.instance_names()[j];
        bank.instances().emplace_back();
        for (size_t i = 0; i < row.size(); ++i) {
          const std::string &name = row[i];
          auto it = all_instances_by_name.find(name);
          LOG_IF(FATAL, it == all_instances_by_name.end())
              << "Could not find memory \"" << name << "\" in main layout";
          geometry::Instance *memory = it->second;
          // LOG(INFO) << "row " << j << ", col " << i << ": " << name
          //           << " -> " << memory;
          bank.instances().back().push_back(memory);
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
      MemoryBank &bank = banks[b];

      for (size_t j = 0; j < bank.instances().size(); ++j) {
        bool rotate_this_row = layout_config.rotate_first_row ?
            j % 2 == 0 : j % 2 != 0;
        std::vector<geometry::Instance*> &row = bank.instances()[j];

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
        std::vector<geometry::Instance*> &last_row = bank.instances()[j - 1];

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

        if (j == bank.instances().size() - 1) {
          if (end_of_last_bank) {
            scan_chain_pairs.insert(
                {end_of_last_bank->name(), start_of_this_row->name()});
          }

          end_of_last_bank = end_of_this_row;
        }
      }
    }
  }

  std::vector<geometry::Instance*> buf_order;
  std::vector<geometry::Instance*> active_mux2s; 

  {
    // Add input buffers. We need one buffer per LUT selector input, i.e. k
    // buffers for a k-LUT.
    RowGuide &upper_row = banks[0].rows()[3];
    int64_t buf_count = 0;
    for (size_t i = 0; i < lut_size_ - 1; ++i) {
      std::string instance_name = absl::StrFormat("buf_%d", buf_count);
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
      geometry::Instance *instance = upper_row.InstantiateBack(
          instance_name, buf_cell->layout());
      buf_order.push_back(instance);
      buf_count++;
    }
    RowGuide &lower_row = banks[1].rows()[0];
    for (size_t i = 0; i < 1; ++i) {
      std::string instance_name = absl::StrFormat("hd_mux2_1_%d", i);
      std::string cell_name = absl::StrCat(instance_name, "_template");
      atoms::Sky130HdMux21 active_mux2_generator({}, design_db_);
      bfg::Cell *active_mux2_cell = active_mux2_generator.GenerateIntoDatabase(
          cell_name);
      active_mux2_cell->layout()->ResetY();
      geometry::Instance *instance = lower_row.InstantiateFront(
          instance_name, active_mux2_cell->layout());
      active_mux2s.push_back(instance);
    }
    // Add more input buffers.
    for (size_t i = 0; i < 1; ++i) {
      std::string instance_name = absl::StrFormat("buf_%d", buf_count);
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
      geometry::Instance *instance = lower_row.InstantiateFront(
          instance_name, buf_cell->layout());
      buf_order.push_back(instance);
      buf_count++;
    }
  }

  //// FIXME(aryap): remove
  ///DEBUG
  //lut_cell->SetLayout(layout.release());
  //lut_cell->SetCircuit(circuit.release());
  //bfg::Cell *pre = lut_cell.release();
  //pre->set_name(name);
  //design_db_->ConsumeCell(pre);
  //return pre;

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

  bfg::RoutingLayerInfo met1_layer_info =
      db.GetRoutingLayerInfoOrDie("met1.drawing");
  met1_layer_info.set_direction(bfg::RoutingTrackDirection::kTrackHorizontal);
  met1_layer_info.set_area(pre_route_bounds);
  // TODO(aryap): If we want y = 735 to be on the grid, and we know the offset
  // is relative to the pre_route_bounds lower-left y = -600, 
  // (735 - (-190)) / 340 (the pitch) = 3.9265
  //    offset = .3.9265 * 340
  //           = 315
  met1_layer_info.set_offset(330);

  bfg::RoutingLayerInfo met2_layer_info =
      db.GetRoutingLayerInfoOrDie("met2.drawing");
  met2_layer_info.set_direction(bfg::RoutingTrackDirection::kTrackVertical);
  met2_layer_info.set_area(pre_route_bounds);
  met2_layer_info.set_offset(50);

  // TODO(aryap): Store connectivity information (which layers connect through
  // which vias) in the PhysicalPropertiesDatabase's via_layers_.
  bfg::RoutingViaInfo routing_via_info =
      db.GetRoutingViaInfoOrDie("met1.drawing", "met2.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid.AddRoutingViaInfo(
      met1_layer_info.layer(), met2_layer_info.layer(), routing_via_info)
      .IgnoreError();
  //alt_routing_grid.AddRoutingViaInfo(
  //    met1_layer_info.layer, met2_layer_info.layer, routing_via_info);

  routing_via_info = db.GetRoutingViaInfoOrDie("li.drawing", "met1.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid.AddRoutingViaInfo(
      met1_layer_info.layer(), db.GetLayer("li.drawing"), routing_via_info)
      .IgnoreError();
  //alt_routing_grid.AddRoutingViaInfo(
  //    met1_layer_info.layer, db.GetLayer("li.drawing"), routing_via_info);

  routing_via_info = db.GetRoutingViaInfoOrDie("met2.drawing", "met3.drawing");
  routing_via_info.set_cost(0.5);
  routing_grid.AddRoutingViaInfo(
      db.GetLayer("met3.drawing"), met2_layer_info.layer(), routing_via_info)
      .IgnoreError();
  //alt_routing_grid.AddRoutingViaInfo(
  //    db.GetLayer("met3.drawing"), met2_layer_info.layer, routing_via_info);

  //routing_grid.AddRoutingLayerInfo(li_layer_info);
  routing_grid.AddRoutingLayerInfo(met1_layer_info).IgnoreError();
  routing_grid.AddRoutingLayerInfo(met2_layer_info).IgnoreError();

  routing_grid.ConnectLayers(met1_layer_info.layer(), met2_layer_info.layer())
      .IgnoreError();

  // Swap direction for the alt routing grid:
  ////std::swap(met1_layer_info.direction, met2_layer_info.direction);
  //alt_routing_grid.AddRoutingLayerInfo(met1_layer_info);
  //alt_routing_grid.AddRoutingLayerInfo(met2_layer_info);
  //alt_routing_grid.ConnectLayers(
  //    met1_layer_info.layer, met2_layer_info.layer);

  {
    // Add blockages from all existing shapes.
    geometry::ShapeCollection shapes;
    layout->CopyNonConnectableShapesOnLayer(
        db.GetLayer("met1.drawing"), &shapes);
    // LOG(INFO) << "met1 shapes: \n" << shapes.Describe();
    routing_grid.AddBlockages(shapes);
    //alt_routing_grid.AddBlockages(
    //    shapes, db.Rules("met1.drawing").min_separation);
  }
  {
    geometry::ShapeCollection shapes;
    layout->CopyNonConnectableShapesOnLayer(
        db.GetLayer("met2.drawing"), &shapes);
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
      const auto &memories = banks[b].instances();
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
        
        std::vector<geometry::Port*> ports;
        source->GetInstancePorts("Q", &ports);
        geometry::Port *start = *ports.begin();
        ports.clear();

        sink->GetInstancePorts("D", &ports);
        geometry::Port *end = *ports.begin();

        EquivalentNets net_names = 
            EquivalentNets({end->net(), start->net()});
        memory_output_net_names[source] = net_names.primary();

        geometry::ShapeCollection non_net_connectables;
        layout->CopyConnectableShapesNotOnNets(
            net_names,
            &non_net_connectables);

        routing_grid.AddRouteBetween(
            *start, *end, non_net_connectables, net_names).IgnoreError();

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
  // step nicer with CopyShapesOnLayer returning the ShapeCollection directly,
  // and would be 2 steps nicer with RoutingGrid having the facility to
  // automatically add blockages from a Layout (for all the layers it itself
  // uses for routing).
  //{
  //  // Add blockages from all existing shapes.
  //  geometry::ShapeCollection shapes;
  //  grid_layout->CopyShapesOnLayer(db.GetLayer("met1.drawing"), &shapes);
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

    std::vector<geometry::Port*> ports;
    source->GetInstancePorts("Q", &ports);
    geometry::Port *start = *ports.begin();
    ports.clear();

    sink->GetInstancePorts("D", &ports);
    geometry::Port *end = *ports.begin();

    EquivalentNets net_names = EquivalentNets({end->net(), start->net()});
    memory_output_net_names[source] = net_names.primary();

    geometry::ShapeCollection non_net_connectables;
    layout->CopyConnectableShapesNotOnNets(
        net_names,
        &non_net_connectables);
    routing_grid.AddRouteBetween(*start, *end, non_net_connectables, net_names)
                .IgnoreError();
  }

  // FIXME(aryap): I want to solve the general problem of connecting to a port
  // on an instance which is comprised of many, possibly connected, shapes on
  // many, possibly connected, layers. The tricky thing is that connecting on
  // one layer might create DRC violations on an adjacent layer (e.g. if you
  // connect on met2 but jump up from met1 just before, and there's a met1
  // shape near, you get a problem).
  //
  // A related and important consideration is that all shapes with the same
  // port name label should be considered connected, even if they are not port
  // objects. Or should they be port objects?
  //
  // Is it desirable in general to treat any shape given a net name as
  // belonging to, and connectable by, that net?
  //
  // And what namespaces do these net names occupy? Their parent instance?
  // Unless exported by being labelled a port with the same name?

  // Connect the input buffers on the selector lines.
  struct PortKey {
    geometry::Instance *instance;
    std::string port_name;
  };
  std::vector<std::vector<PortKey>> auto_connections = {
    {{buf_order[0], "P"}, {mux_order[0], "S0_B"}, {mux_order[1], "S0_B"}},
    {{buf_order[0], "X"}, {mux_order[0], "S0"}, {mux_order[1], "S0"}},
    {{buf_order[1], "P"}, {mux_order[0], "S1_B"}, {mux_order[1], "S1_B"}},
    {{buf_order[1], "X"}, {mux_order[0], "S1"}, {mux_order[1], "S1"}},
    {{buf_order[2], "P"}, {mux_order[0], "S2_B"}, {mux_order[1], "S2_B"}},
    {{buf_order[2], "X"}, {mux_order[0], "S2"}, {mux_order[1], "S2"}},
    {{buf_order[3], "X"}, {active_mux2s[0], "S"}},
    {{mux_order[0], "Z"}, {active_mux2s[0], "A0"}},
    {{mux_order[1], "Z"}, {active_mux2s[0], "A1"}},
    {{active_mux2s[0], "X"}, {buf_order[3], "A"}},
  };

  // Add automatic connections for memory clock and inverted clock inputs.
  for (size_t bank = 0; bank < banks.size(); ++bank) {
    for (size_t column = 0; column < layout_config.bank_columns; ++column) {
      std::vector<PortKey> &clk_connections = auto_connections.emplace_back();
      for (size_t row = 0; row < layout_config.bank_rows; ++row) {
        clk_connections.push_back({
            .instance = banks[bank].instances()[row][column],
            .port_name = "CLK"
        });
      }
      std::vector<PortKey> &clk_i_connections = auto_connections.emplace_back();
      for (size_t row = 0; row < layout_config.bank_rows; ++row) {
        clk_i_connections.push_back({
            .instance = banks[bank].instances()[row][column],
            .port_name = "CLKI"
        });
      }
    }
  }

  for (auto &connections : auto_connections) {
    std::vector<std::vector<geometry::Port*>> route_targets;
    for (auto &port_key : connections) {
      std::vector<geometry::Port*> &port_list =
          route_targets.emplace_back();
      geometry::Instance *instance = port_key.instance;

      std::vector<geometry::Port*> matching_ports;
      instance->GetInstancePorts(port_key.port_name, &matching_ports);
      if (matching_ports.empty()) {
        LOG(WARNING) << "No port found named \"" << port_key.port_name
                     << "\" on instance \"" << instance->name();
        continue;
      }
      port_list.insert(
          port_list.end(), matching_ports.begin(), matching_ports.end());
    }

    for (const auto &port_list : route_targets) {
      LOG(INFO) << geometry::Port::DescribePorts(port_list);
    }

    bool paths_found = routing_grid.AddMultiPointRoute(
        *layout, route_targets, std::nullopt).ok();
  }


  // Connect memory outputs to the muxes in order:
  // - Connect the closest output with

  // Connect flip-flops to mux.
  struct AutoMemoryMuxConnection {
    geometry::Instance *source_memory;
    geometry::Instance *target_mux;
    std::string mux_port_name;
  };

  // The mux input order is, from top to bottom:
  // input_5  --+---------
  // input_4  --|
  // input_6  --|
  // input_7  --|  8:1 mux
  // input_3  --|
  // input_2  --|
  // input_0  --|
  // input_1  --+---------

  std::vector<AutoMemoryMuxConnection> auto_mem_connections = {
    // Manually ordered:
    {banks[0].instances()[2][1], mux_order[0], "input_6"},
    {banks[0].instances()[3][0], mux_order[0], "input_4"},
    {banks[0].instances()[3][1], mux_order[0], "input_5"},
    {banks[0].instances()[2][0], mux_order[0], "input_7"},

    {banks[0].instances()[1][1], mux_order[0], "input_3"},
    {banks[0].instances()[1][0], mux_order[0], "input_2"},
    {banks[0].instances()[0][0], mux_order[0], "input_0"},
    {banks[0].instances()[0][1], mux_order[0], "input_1"},

    // Sort of:
    {banks[1].instances()[1][0], mux_order[1], "input_2"},

    {banks[1].instances()[2][0], mux_order[1], "input_7"},
    {banks[1].instances()[2][1], mux_order[1], "input_6"},
    {banks[1].instances()[3][0], mux_order[1], "input_4"},
    {banks[1].instances()[3][1], mux_order[1], "input_5"},

    {banks[1].instances()[0][0], mux_order[1], "input_1"},
    {banks[1].instances()[0][1], mux_order[1], "input_0"},
    {banks[1].instances()[1][1], mux_order[1], "input_3"},
  };

  for (auto &auto_connection : auto_mem_connections) {
    geometry::Instance *memory = auto_connection.source_memory;
    geometry::Instance *mux = auto_connection.target_mux;
    const std::string &input_name = auto_connection.mux_port_name;

    // Heuristically determine which mux port to use based on which which is
    // closest to the memory output, even if we're routing to the memory output
    // net instead of the port specifically.
    std::vector<geometry::Port*> memory_ports;
    memory->GetInstancePorts("Q", &memory_ports);
    geometry::Port *memory_output = *memory_ports.begin();

    std::vector<geometry::Port*> mux_ports_on_net;
    mux->GetInstancePorts(input_name, &mux_ports_on_net);

    geometry::Port *mux_port = mux->GetNearestPortNamed(*memory_output,
                                                        input_name);
    if (!mux_port) {
      continue;
    }
    LOG_IF(
        FATAL,
        std::find(mux_ports_on_net.begin(), mux_ports_on_net.end(), mux_port) ==
            mux_ports_on_net.end())
        << "Nearest port named " << input_name
        << " did not appear in list of all ports for same name";

    while (mux_port) {
      EquivalentNets net_names = EquivalentNets(
          {memory_output->net(), mux_port->net()});
      geometry::ShapeCollection non_net_connectables;
      layout->CopyConnectableShapesNotOnNets(net_names, &non_net_connectables);
      LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
                << " avoiding " << non_net_connectables.Describe();

      bool path_found = false;
      auto named_output_it = memory_output_net_names.find(memory);
      if (named_output_it == memory_output_net_names.end()) {
        memory_output_net_names[memory] = net_names.primary();
        LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
                  << " to " << memory->name();
        path_found = routing_grid.AddRouteBetween(
            *mux_port, *memory_output, non_net_connectables, net_names).ok();
      } else {
        // FIXME(aryap): I am stupid. The set of names given to the router to
        // determine which shapes are connectable is different to the target
        // set; in fact we must make sure that the net has a distinct name from
        // either start/end port so that routed wires can be differentiated from
        // start/end obstacles and ports!
        const std::string &target_net = named_output_it->second;
        net_names.set_primary(target_net);
        LOG(INFO) << "Connecting " << mux->name() << " port " << input_name
                  << " to net " << target_net;
        path_found = routing_grid.AddRouteToNet(
            *mux_port, target_net, net_names, non_net_connectables).ok();
      }
      if (path_found) {
        break;
      }
      mux_ports_on_net.erase(
          std::remove(mux_ports_on_net.begin(),
                      mux_ports_on_net.end(),
                      mux_port),
          mux_ports_on_net.end());
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
