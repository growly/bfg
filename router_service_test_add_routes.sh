#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Specify grid ID as first argument"
    exit 1
fi

~/go/bin/grpcurl -plaintext -d @ \
    localhost:8222 bfg.router_service.RouterService/AddRoutes << EOM
{
  "grid_id": ${1},
  "net_route_orders": [
    {
      "net": "A",
      "points": [
        {
          "point": { "x": 10, "y": 10 },
          "layer_name": "met1.pin"
        },
        {
          "point": { "x": 100, "y": 100 },
          "layer_name": "met1.pin"
        },
        {
          "point": { "x": 300, "y": 50 },
          "layer_name": "li.pin"
        },
        {
          "point": { "x": 900, "y": 900 },
          "layer_name": "li.pin"
        }
      ]
    },
    {
      "net": "B",
      "points": [
        {
          "point": { "x": 500, "y": 600 },
          "layer_name": "met1.pin"
        },
        {
          "point": { "x": 800, "y": 250 },
          "layer_name": "li.pin"
        }
      ]
    }
  ]
}
EOM
