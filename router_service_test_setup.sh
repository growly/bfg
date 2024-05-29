#!/bin/bash

~/go/bin/grpcurl -plaintext -d @ \
    localhost:8222 bfg.router_service.RouterService/CreateRoutingGrid << EOM
{
  "predefined_technology": "TECHNOLOGY_SKY130",
  "grid_definition": {
    "layers": [
      {
        "name": "met1.drawing",
        "direction": "TRACK_DIRECTION_HORIZONTAL",
        "area": {
          "lower_left": { "x": 0, "y": 0 },
          "upper_right": { "x": 1000, "y": 1000 }
        },
        "offset": 330
      },
      {
        "name": "met2.drawing",
        "direction": "TRACK_DIRECTION_VERTICAL",
        "area": {
          "lower_left": { "x": 0, "y": 0 },
          "upper_right": { "x": 1000, "y": 1000 }
        },
        "offset": 50
      }
    ],
    "vias": [
      {
        "between_layer": "li.drawing",
        "and_layer": "met1.drawing",
        "cost": 0.5
      },
      {
        "between_layer": "met1.drawing",
        "and_layer": "met2.drawing",
        "cost": 0.5
      },
      {
        "between_layer": "met2.drawing",
        "and_layer": "met3.drawing",
        "cost": 0.5
      }
    ]
  }
}
EOM
