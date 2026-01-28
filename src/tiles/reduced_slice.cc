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

#include <absl/strings/str_join.h>
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
    const std::vector<std::string> &direction_prefixes,
    int64_t long_bundle_break_out,
    bool break_out_regular_side_first,
    bool alternate_break_out,
    InterconnectWireBlock::Parameters *iwb_params) const {
  // Vertical wires combine North and South driving wires, and each needs its
  // own bundle. Horizontal wires combine East and West driving wires.
  static constexpr int kDirectionsPerBlock = 2;

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
    for (const std::string &prefix : direction_prefixes) {
      bool alternate_side = break_out_regular_side_first;
      for (const auto &side_of_tile : parameters_.kSidesOfTile) {
        InterconnectWireBlock::Parameters::Channel channel = {
          .name = absl::StrFormat(
              "%s%d_%s", prefix, length_in_tiles, side_of_tile)
        };
        for (int i = 0; i < length_in_tiles; ++i) {
          InterconnectWireBlock::Parameters::Bundle bundle = {
            .num_wires = parameters_.kBundleSize
          };
          // TODO(aryap): This is a parameter.
          if (i == 0) {
            bundle.tap = true;
            bundle.break_out = InterconnectWireBlock::Parameters::Break();
            bundle.break_out->alternate_side = alternate_side;
            bundle.break_out->offset = std::nullopt;
            bundle.break_in = InterconnectWireBlock::Parameters::Break();
            bundle.break_in->alternate_side = alternate_side;
            bundle.break_in->offset = std::nullopt;
          }
          channel.bundles.push_back(bundle);
        }
        iwb_params->channels.push_back(channel);
        if (alternate_break_out) {
          alternate_side = !alternate_side;
        }
      }
    }
  }

  const PhysicalPropertiesDatabase &db = design_db_->physical_db();
  int64_t stride = InterconnectWireBlock::PredictPitchOfOffAxis(
      db, *iwb_params);

  // The last length, the largest, 
  std::string prefix = absl::StrJoin(direction_prefixes, "");
  int last_length = parameters_.kInterconnectLengths[num_lengths - 1];
  InterconnectWireBlock::Parameters::Channel channel = {
    .name = absl::StrFormat("%s%d", prefix, last_length),
  };
  for (int d = 0; d < kDirectionsPerBlock; ++d) {
    for (int i = 0; i < last_length; ++i) {
      InterconnectWireBlock::Parameters::Bundle bundle = {
        .num_wires = parameters_.kBundleSize
      };
      // TODO(aryap): This is a parameter.
      if (i == 0) {
        bundle.tap = true;
        bundle.break_out = InterconnectWireBlock::Parameters::Break();
        bundle.break_out->alternate_side = false;
        bundle.break_out->offset = long_bundle_break_out +
            d * stride * bundle.num_wires;
        bundle.break_in = InterconnectWireBlock::Parameters::Break();
        bundle.break_in->alternate_side = alternate_break_out;
        bundle.break_in->offset = std::nullopt;
      }
      channel.bundles.push_back(bundle);
    }
  }
  iwb_params->channels.push_back(channel);
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

  LutB::Parameters default_lut_params = {
      .lut_size = 4
  };
  LutB default_lut_gen(default_lut_params, design_db_);
  std::string lut_name = "lut";
  Cell *default_lut_cell = default_lut_gen.GenerateIntoDatabase(lut_name);

  static constexpr int kLutsPerRow = 4;
  for (size_t i = 0; i < parameters_.kNumLUTs; ++i) {
    geometry::Instance *instance = luts.InstantiateRight(
        i / kLutsPerRow, absl::StrCat(lut_name, "_i", i), default_lut_cell);
  }

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

  static constexpr int kNumRightSkinnyRows = 3;

  FillClockwise(
      0,
      0,
      kNumRightSkinnyRows,
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

  // With the LUT group, IIB and OIBS2 generated, we can now position the LUT
  // group in the centre of the gap we've created:
  const RowGuide &row_upper_left = iib.Row(kNumLeftSkinnyRows - 1);
  const RowGuide &row_lower_right = oib_s2.Row(
      oib_s2.NumRows() - kNumRightSkinnyRows);
  geometry::Point cavity_centre = {
    (row_upper_left.GetTilingBounds()->upper_right().x() +
     row_lower_right.GetTilingBounds()->lower_left().x()) / 2,
    (row_upper_left.GetTilingBounds()->upper_right().y() +
     row_lower_right.GetTilingBounds()->lower_left().y()) / 2};
      
  int64_t luts_x = iib.Row(0).GetTilingBounds()->upper_right().x();
  int64_t luts_y = iib.Row(0).GetTilingBounds()->upper_right().y() -
      static_cast<int64_t>(luts.GetTilingBounds()->Height()) / 2;
  luts.MoveTo(cavity_centre - geometry::Point(
        static_cast<int64_t>(luts.GetTilingBounds()->Width()) / 2,
        static_cast<int64_t>(luts.GetTilingBounds()->Height()) / 2));

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
      parameters_.kNumOIBS1 + parameters_.kNumOIBS1Shared / 2,
      10000,
      10000,
      &oib_s1);

  oib_s1.MoveTo(
      {0, 
       oib_s2.GetTilingBounds()->lower_left().y() -
          static_cast<int64_t>(oib_s1.GetTilingBounds()->Height())});

  uint64_t current_height = west_layout->GetTilingBounds().Height();
  uint64_t current_width = west_layout->GetTilingBounds().Width();

  InterconnectWireBlock::Parameters horizontal_wire_block_params = {
    .layout_mode =
        InterconnectWireBlock::Parameters::LayoutMode::kModestlyClever,
        //InterconnectWireBlock::Parameters::LayoutMode::kConservative,
    .direction = RoutingTrackDirection::kTrackHorizontal,
    .horizontal_wire_offset_nm = db.ToExternalUnits(
        db.Rules("met1.drawing").min_pitch),
    .vertical_wire_pitch_nm = db.ToExternalUnits(
        2 * db.Rules("met1.drawing").min_pitch)
  };
  GenerateInterconnectChannels(
      {"EE", "WW"},
      current_width - oib_s1_cell->layout()->GetTilingBounds().Width(),
      false,
      false,
      &horizontal_wire_block_params);

  // "NN2_b0_w0_A" is by convention outgoing wire 0, bundle 0, size, to the
  // north. "SS2_b0_w0_A" is by convention incoming wire 0, bundle 0, from the
  // north.

  InterconnectWireBlock::Parameters vertical_wire_block_params = {
    .layout_mode =
        InterconnectWireBlock::Parameters::LayoutMode::kModestlyClever,
        //InterconnectWireBlock::Parameters::LayoutMode::kConservative,
    .direction = RoutingTrackDirection::kTrackVertical,
    .horizontal_wire_pitch_nm = db.ToExternalUnits(
        2 * db.Rules("met1.drawing").min_pitch)
  };
  GenerateInterconnectChannels(
      {"NN", "SS"},
      current_height - oib_s1_cell->layout()->GetTilingBounds().Height(),
      true,
      true,
      &vertical_wire_block_params);

  horizontal_wire_block_params.length =
      2 * current_width + InterconnectWireBlock::PredictWidth(
          db, vertical_wire_block_params);
  vertical_wire_block_params.length =
      current_height + InterconnectWireBlock::PredictHeight(
          db, horizontal_wire_block_params);

  {
    std::string horizontal_wire_block_name = "horizontal_wire_block";
    InterconnectWireBlock horizontal_wire_block_generator(
        horizontal_wire_block_params, design_db_);
    Cell *horizontal_wire_block =
        horizontal_wire_block_generator.GenerateIntoDatabase(
            horizontal_wire_block_name);
    geometry::Instance horizontal_wire_block_instance(
        horizontal_wire_block->layout(), {0, 0});
    horizontal_wire_block_instance.FlipVertical();
    horizontal_wire_block_instance.ResetOrigin();
    horizontal_wire_block_instance.Translate(
        {0,  // FIXME(aryap)
         west_layout->GetTilingBounds().lower_left().y()});
    //horizontal_wire_block_instance.ResetOrigin();
    //horizontal_wire_block_instance.Translate(
    //    {0,
    //     west_layout->GetTilingBounds().lower_left().y() - static_cast<int64_t>(
    //         horizontal_wire_block_instance.GetTilingBounds().Height())});
    horizontal_wire_block_instance.set_name(
        absl::StrCat(horizontal_wire_block_name, "_i"));
    geometry::Instance *actual_instance = cell->layout()->AddInstance(
        horizontal_wire_block_instance);
  }
  geometry::Instance *central_wire_block = nullptr;
  {
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
  east_layout->AddLayout(*west_layout);
  LOG(INFO) << "east layout tiling bounds: " << east_layout->GetTilingBounds();
  east_layout->FlipHorizontal();
  LOG(INFO) << "east layout tiling bounds: " << east_layout->GetTilingBounds();
  geometry::Point reference =
      east_layout->GetTilingBounds().lower_left();
  geometry::Point target = {
      central_wire_block->GetTilingBounds().upper_right().x(),
      west_layout->GetTilingBounds().lower_left().y()};
  LOG(INFO) << "aligning " << reference << " target: " << target;
  east_layout->AlignPointTo(reference, target);
  cell->layout()->AddLayout(*east_layout, "east");


  cell->layout()->AddLayout(*west_layout, "west");

  return cell.release();
}

}   // namespace tiles
}   // namespace bfg
