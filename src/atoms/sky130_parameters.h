#ifndef ATOMS_SKY130_PARAMETERS_H_
#define ATOMS_SKY130_PARAMETERS_H_

struct Sky130Parameters {
  // TODO(aryap): Maybe these are queried from the PDK database?
  // and/or do they belong to a more generic "TransistorParameters" mix-in?
  std::string fet_model_length_parameter = "l";
  std::string fet_model_width_parameter = "w";

  bool draw_vpwr_vias = true;
  bool draw_vgnd_vias = true;

  std::string power_net = "VPWR";
  std::string ground_net = "VGND";
};

#endif  // ATOMS_SKY130_PARAMETERS_H_
