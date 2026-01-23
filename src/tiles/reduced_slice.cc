#include "reduced_slice.h"

#include <string>

#include "../atoms/sky130_interconnect_mux1.h"
#include "../atoms/sky130_interconnect_mux2.h"
#include "../circuit.h"
#include "../geometry/instance.h"
#include "../layout.h"
#include "../memory_bank.h"
#include "lut_b.h"
#include "interconnect_wire_block.h"
#include "proto/parameters/lut_b.pb.h"
#include "proto/parameters/reduced_slice.pb.h"
#include "../utility.h"

#include <absl/strings/str_format.h>

namespace bfg {
namespace tiles {

void ReducedSlice::Parameters::ToProto(proto::parameters::ReducedSlice *pb)
    const {
  // TODO(aryap): Complete.
}

void ReducedSlice::Parameters::FromProto(
    const proto::parameters::ReducedSlice &pb) {
  // TODO(aryap): Complete.
}


// Ok the thing I want to capture is the generation logic for slices "like
// this", meaning nothing more general than that.
//
// Given a bank, and parameterised by:
//  - the number of left, right columns, top, bottom rows
//  - cells to instantiate in each of the top, bottom, left, right positions
//  - the width of the central blank space
//  - the height of the central blank space
//  - ...
// then we want a thing that will fill the columns/rows clockwise, starting at
// the lower left.
//
//
// +-----+-----+-----+-----+-----+-----+-----+-----+
// |     |     |     |     |     |     |     |     |
// +-----+-----+-----+-----+-----+-----+-----+-----+
// |     |     |     |     |     |     |     |     |
// +-----+-----+-----+-----+-----+-----+-----+-----+
// |     |     |                       |     |     |
// +-----+-----+                       +-----+-----+
// |     |     |                       |     |     |
// +-----+-----+                       +-----+-----+
// |     |     |                       |     |     |
// +-----+-----+                       +-----+-----+
// |     |     |                       |     |     |
// +-----+-----+-----+-----+-----+-----+-----+-----+
// |     |     |     |     |     |     |     |     |
// +-----+-----+-----+-----+-----+-----+-----+-----+
// |     |     |     |     |     |     |     |     |
// +-----+-----+-----+-----+-----+-----+-----+-----+

void FillClockwise(
    int columns_left,
    int rows_top,
    int columns_right,
    int rows_bottom,
    Cell *cell_left,
    Cell *cell_top,
    Cell *cell_right,
    Cell *cell_bottom,
    int target_count,
    int64_t centre_height,
    int64_t centre_width,
    MemoryBank *bank) {
  std::vector<geometry::Instance*> instances;

  // Start above the bottom group:
  int64_t row = static_cast<int64_t>(rows_bottom);

  int64_t tiling_unit_height = std::max({
      cell_left->layout()->GetTilingBounds().Height(),
      cell_right->layout()->GetTilingBounds().Height()});

  int64_t tiling_unit_width = std::max({
      cell_top->layout()->GetTilingBounds().Width(),
      cell_bottom->layout()->GetTilingBounds().Width()});

  int64_t num_rows_left = Utility::NextMultiple(
      centre_height, tiling_unit_height) / tiling_unit_height;

  int count = 0;
  for (size_t i = 0; i < num_rows_left; ++i) {
    for (size_t j = 0; j < columns_left && count < target_count; ++j, ++count) {
      geometry::Instance *instance = bank->InstantiateLeft(
          row + i, absl::StrCat(cell_left->name(), "_i", count), cell_left);
      instances.push_back(instance);
    }
  }
  row += num_rows_left;

  int64_t num_columns_top = Utility::NextMultiple(
      static_cast<int64_t>(
          centre_width + (
              columns_left * cell_left->layout()->GetTilingBounds().Width()) + (
              columns_right * cell_right->layout()->GetTilingBounds().Width())
      ),
      tiling_unit_width);
  for (size_t j = 0; j < num_columns_top; ++j) {
    for (size_t i = 0; i < rows_top && count < target_count; ++i, ++count) {
      geometry::Instance *instance = bank->InstantiateLeft(
          row + i, absl::StrCat(cell_top->name(), "_i", count), cell_top);
      instances.push_back(instance);
    }
  }

  int64_t num_rows_right = num_rows_left;
  if (columns_right > 0) {
    for (int i = num_rows_right; i > 0; --i) {
      bank->Row(row - i).AddBlankSpaceBack(centre_width);
      LOG(INFO) << "Adding blank space (" << centre_width << ") to row " << i;
    }
  }

  for (int i = num_rows_right; i >= 0; --i) {
    for (size_t j = 0;
        j < columns_right && count < target_count;
        ++j, ++count) {
      geometry::Instance *instance = bank->InstantiateRight(
          row + i, absl::StrCat(cell_left->name(), "_i", count), cell_left);
      instances.push_back(instance);
    }
  }
  row -= num_rows_right;

  int64_t num_columns_bottom = num_columns_top;
  for (size_t j = 0; j < num_columns_bottom; ++j) {
    for (int i = 0; i < rows_bottom && count < target_count; ++i, ++count) {
      geometry::Instance *instance = bank->InstantiateLeft(
          row - i - 1, absl::StrCat(cell_left->name(), "_i", count), cell_left);
      instances.push_back(instance);
    }
  }

  LOG(INFO) << "Instance count: " << count;
}

void ReducedSlice::GenerateInterconnectChannels(
    InterconnectWireBlock::Parameters *iwb_params) const {
  iwb_params->channels.clear();
  // For wire lengths not 1 and not the greatest, we create a bundle per side of
  // the tile, as well as a bundle for every other block that must pass through
  // this one.
  size_t num_lengths = sizeof(parameters_.kInterconnectLengths) / sizeof(int);
  for (size_t i = 0; i < num_lengths - 1; ++i) {
    int length_in_tiles = parameters_.kInterconnectLengths[i];
    if (length_in_tiles == 1) {
      // Length 1 connections do not go in the block, they are more directly
      // routed.
      continue;
    }
    for (const auto &side_of_tile : parameters_.kSidesOfTile) {
      iwb_params->channels.push_back({
        .name = absl::StrFormat("%dX_%s", length_in_tiles, side_of_tile),
        .break_out = {0},   // FIXME(aryap): This is a parameter.
        .num_bundles = length_in_tiles,
        .bundle = {
          .num_wires = parameters_.kBundleSize
        }
      });
    }
  }

  // The last length, the largest, 
  int last_length = parameters_.kInterconnectLengths[num_lengths - 1];
  iwb_params->channels.push_back({
    .name = absl::StrFormat("%dX", last_length),
    .break_out = {0},   // FIXME(aryap): This is a parameter.
    .num_bundles = last_length,
    .bundle = {
      .num_wires = parameters_.kBundleSize
    }
  });
}

Cell *ReducedSlice::GenerateIntoDatabase(const std::string &name) {
  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  std::unique_ptr<Cell> cell(new Cell(name));
  cell->SetCircuit(new bfg::Circuit());
  cell->SetLayout(new bfg::Layout(db));

  std::unique_ptr<bfg::Layout> west_layout(new bfg::Layout(db));

  MemoryBank luts = MemoryBank(west_layout.get(),
                               cell->circuit(),
                               design_db_,
                               nullptr,    // No tap cells.
                               false,      // Rotate alternate rows.
                               false,      // Rotate first row.
                               geometry::Compass::LEFT);

  //LutB::Parameters default_lut_params = {
  //    .lut_size = 4
  //};
  //LutB default_lut_gen(default_lut_params, design_db_);
  //std::string lut_name = "lut";
  //Cell *default_lut_cell = default_lut_gen.GenerateIntoDatabase(lut_name);

  //for (size_t i = 0; i < parameters_.kNumLUTs; ++i) {
  //  geometry::Instance *instance = luts.InstantiateRight(
  //      i / 2, absl::StrCat(lut_name, "_i"), default_lut_cell);
  //}

  //std::unique_ptr<bfg::Layout> middle_layout(new bfg::Layout(db));
  //Interconnect::Parameters interconnect_params;
  //Interconnect interconnect_gen(interconnect_params, design_db_);
  //Cell *interconnect_cell =
  //    interconnect_gen.GenerateIntoDatabase("interconnect");
  //geometry::Instance interconnect_instance(
  //    interconnect_cell->layout(), {0, 0});
  //middle_layout->AddInstance(interconnect_instance);
  //middle_layout->MoveTo({left_layout->GetTilingBounds().upper_right().x(), 0});

  std::vector<std::vector<geometry::Instance*>> muxes;

  MemoryBank iib = MemoryBank(west_layout.get(),
                              cell->circuit(),
                              design_db_,
                              nullptr,    // No tap cells.
                              false,      // Rotate alternate rows.
                              false,      // Rotate first row.
                              geometry::Compass::LEFT);

  static constexpr int kNumLeftSkinnyRows = 2;

  static const atoms::Sky130InterconnectMux1::Parameters defaults = {
    .vertical_pitch_nm = 340,
    .vertical_offset_nm = 170,
    .horizontal_pitch_nm = 460
  };

  atoms::Sky130InterconnectMux1::Parameters iib_s2_params = defaults;
  iib_s2_params.num_inputs = 6;
  iib_s2_params.num_outputs = 1;

  std::string iib_s2_mux_name = "iib_s2_mux";
  atoms::Sky130InterconnectMux1 iib_s2_generator(iib_s2_params, design_db_);
  Cell *iib_s2_cell = iib_s2_generator.GenerateIntoDatabase(iib_s2_mux_name);
  FillClockwise(
      kNumLeftSkinnyRows,
      2,
      0,
      0,
      iib_s2_cell, 
      iib_s2_cell, 
      iib_s2_cell, 
      iib_s2_cell,
      parameters_.kNumIIBS2,
      45000,  // TODO(aryap): This is the height of the central LUT block.
      1000,   // ?
      &iib);

  atoms::Sky130InterconnectMux1::Parameters iib_s1_params = defaults;
  iib_s1_params.num_inputs = 7;
  iib_s1_params.num_outputs = 2;

  std::string iib_s1_mux_name = "iib_s1_mux";
  atoms::Sky130InterconnectMux2 iib_s1_generator(iib_s1_params, design_db_);
  Cell *iib_s1_cell = iib_s1_generator.GenerateIntoDatabase(iib_s1_mux_name);

  FillClockwise(
      1,
      1,
      0,
      0,
      iib_s1_cell, 
      iib_s1_cell, 
      iib_s1_cell, 
      iib_s1_cell,
      parameters_.kNumIIBS1,
      iib.GetTilingBounds()->Height(),
      1000,   // ?
      &iib);


  MemoryBank oib_s2 = MemoryBank(west_layout.get(),
                                 cell->circuit(),
                                 design_db_,
                                 nullptr,    // No tap cells.
                                 false,      // Rotate alternate rows.
                                 false,      // Rotate first row.
                                 geometry::Compass::RIGHT);

  atoms::Sky130InterconnectMux1::Parameters oib_s2_params;
  oib_s2_params.num_inputs = 5;
  oib_s2_params.num_outputs = 2;
  oib_s2_params.inside_out = true;

  std::string oib_s2_mux_name = "oib_s2_mux";
  atoms::Sky130InterconnectMux2 oib_s2_generator(oib_s2_params, design_db_);
  Cell *oib_s2_cell = oib_s2_generator.GenerateIntoDatabase(oib_s2_mux_name);

  FillClockwise(
      0,
      0,
      3,
      1,
      oib_s2_cell,
      oib_s2_cell,
      oib_s2_cell,
      oib_s2_cell,
      parameters_.kNumOIBS2 - 3,
      30000,
      30000,
      &oib_s2);
  
  //std::string oib_s2_mux_name = "oib_s2_mux_tall";
  //atoms::Sky130InterconnectMux1::Parameters oib_s2_params = {
  //  .num_inputs = 5,
  //  .num_outputs = 2,
  //};
  //atoms::Sky130InterconnectMux2 oib_s2_generator(oib_s2_params, design_db_);
  //Cell *oib_s2_cell = oib_s2_generator.GenerateIntoDatabase(oib_s2_mux_name);

  oib_s2.MoveTo(
      {0,
      iib.Row(kNumLeftSkinnyRows).GetTilingBounds()->lower_left().y() -
          static_cast<int64_t>(oib_s2.GetTilingBounds()->Height())});

  MemoryBank oib_s1 = MemoryBank(west_layout.get(),
                                 cell->circuit(),
                                 design_db_,
                                 nullptr,    // No tap cells.
                                 false,      // Rotate alternate rows.
                                 false,      // Rotate first row.
                                 geometry::Compass::LEFT);

  atoms::Sky130InterconnectMux1::Parameters oib_s1_params = iib_s1_params;
  oib_s1_params.num_inputs = 6;
  oib_s1_params.num_outputs = 1;

  std::string oib_s1_mux_name = "oib_s1_mux";
  atoms::Sky130InterconnectMux1 oib_s1_generator(oib_s1_params, design_db_);
  Cell *oib_s1_cell = oib_s1_generator.GenerateIntoDatabase(oib_s1_mux_name);

  FillClockwise(
      0,
      0,
      0,
      2,
      oib_s1_cell,
      oib_s1_cell,
      oib_s1_cell,
      oib_s1_cell,
      parameters_.kNumOIBS1,
      10000,
      10000,
      &oib_s1);

  oib_s1.MoveTo(
      {0, 
       oib_s2.GetTilingBounds()->lower_left().y() -
          static_cast<int64_t>(oib_s1.GetTilingBounds()->Height())});

  uint64_t current_height = west_layout->GetTilingBounds().Height();
  uint64_t current_width = west_layout->GetTilingBounds().Width();

  uint64_t horizontal_wire_block_height = 0;
  {
    InterconnectWireBlock::Parameters horizontal_wire_block_params = {
      .layout_mode =
          InterconnectWireBlock::Parameters::LayoutMode::kModestlyClever,
      .direction = RoutingTrackDirection::kTrackHorizontal,
      .length = 2 * current_width   // FIXME(aryap).
    };
    GenerateInterconnectChannels(&horizontal_wire_block_params);
    std::string horizontal_wire_block_name = "horizontal_wire_block";
    InterconnectWireBlock horizontal_wire_block_generator(
        horizontal_wire_block_params, design_db_);
    Cell *horizontal_wire_block =
        horizontal_wire_block_generator.GenerateIntoDatabase(
            horizontal_wire_block_name);
    horizontal_wire_block_height =
        horizontal_wire_block->layout()->GetTilingBounds().Height();
    geometry::Instance horizontal_wire_block_instance(
        horizontal_wire_block->layout(), {0, 0});
    horizontal_wire_block_instance.FlipVertical();
    horizontal_wire_block_instance.ResetOrigin();
    horizontal_wire_block_instance.Translate(
        {0,  // FIXME(aryap)
         west_layout->GetTilingBounds().lower_left().y()});
    horizontal_wire_block_instance.set_name(
        absl::StrCat(horizontal_wire_block_name, "_i"));
    geometry::Instance *actual_instance = cell->layout()->AddInstance(
        horizontal_wire_block_instance);
  }
  geometry::Instance *central_wire_block = nullptr;
  {
    InterconnectWireBlock::Parameters vertical_wire_block_params = {
      .layout_mode =
          InterconnectWireBlock::Parameters::LayoutMode::kModestlyClever,
          //InterconnectWireBlock::Parameters::LayoutMode::kConservative,
      .direction = RoutingTrackDirection::kTrackVertical,
      .length = current_height + horizontal_wire_block_height
    };
    GenerateInterconnectChannels(&vertical_wire_block_params);
    std::string vertical_wire_block_name = "vertical_wire_block";
    InterconnectWireBlock vertical_wire_block_generator(
        vertical_wire_block_params, design_db_);
    Cell *vertical_wire_block =
        vertical_wire_block_generator.GenerateIntoDatabase(
            vertical_wire_block_name);
    geometry::Instance vertical_wire_block_instance(
        vertical_wire_block->layout(), {0, 0});
    vertical_wire_block_instance.FlipVertical();
    vertical_wire_block_instance.ResetOrigin();
    vertical_wire_block_instance.Translate(
        west_layout->GetTilingBounds().upper_right());
    vertical_wire_block_instance.set_name(
        absl::StrCat(vertical_wire_block_name, "_i"));
    central_wire_block = cell->layout()->AddInstance(
        vertical_wire_block_instance);
  }

  // Is the east layout just a dumb copy/mirror of the west layout?
  std::unique_ptr<Layout> east_layout(new bfg::Layout(db));
  //east_layout->AddLayout(*west_layout);
  //east_layout->FlipHorizontal();
  //// FIXME(aryap): FlipHorizontal is broken?
  ////east_layout->FlipHorizontal();
  //east_layout->AlignPointTo(
  //    east_layout->GetTilingBounds().lower_left(),
  //    {central_wire_block->GetTilingBounds().upper_right().x(),
  //     west_layout->GetTilingBounds().lower_left().y()});


  //// FIXME(aryap): This is dumb.
  cell->layout()->AddLayout(*west_layout);
  cell->layout()->AddLayout(*east_layout);

  return cell.release();
}

}   // namespace tiles
}   // namespace bfg
