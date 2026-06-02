#!/bin/bash

unalias cd
unalias bd

for mux_nfet_width in {420..660..10}; do
  for mux_pfet_width in {420..1000..20}; do
    sub=run_${mux_nfet_width}_${mux_pfet_width};
    mkdir -p $sub;
    cp -r *.sp "${sub}/";
    echo -e ".param mux_nfet_width=${mux_nfet_width}m\n.param mux_pfet_width=${mux_pfet_width}m\n" > "${sub}/params.sp";
    echo -e "bash -c \"cd ${sub}; Xyce lut_test.sp\"" >> commands;
  done
done

