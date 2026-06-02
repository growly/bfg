#!/bin/bash

unalias cd
unalias bd

touch commands
truncate -s 0 commands

for reg_out_nfet_width in {420..660..10}; do
  for reg_out_pfet_width in {420..1000..20}; do
    sub="run_${reg_out_nfet_width}_${reg_out_pfet_width}";
    mkdir -p $sub;
    cp -r *.sp "${sub}/";
    cat <<EOF > "${sub}/params.sp"
.param reg_out_nfet_width=${reg_out_nfet_width}m
.param reg_out_pfet_width=${reg_out_pfet_width}m
.param mux_nfet_width=580m
.param mux_pfet_width=460m
.param buffer_nfet_width=540m
.param buffer_pfet_width=900m
EOF
    echo -e "bash -c \"cd ${sub}; Xyce lut_test.sp\"" >> commands;
  done
done
