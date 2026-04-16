#!/bin/bash

unalias cd
unalias bd

truncate -s 0 commands

for s0_nfet_width in {420..660..10}; do
  for s0_pfet_width in {420..1000..20}; do
    for s1s2_nfet_width in {490..550..20}; do
      for s1s2_pfet_width in {900..1000..40}; do
        sub="run_${s0_nfet_width}_${s0_pfet_width}_${s1s2_nfet_width}_${s1s2_pfet_width}"
        mkdir -p $sub;
        cp -r *.sp "${sub}/";
        echo -e "bash -c \"cd ${sub}; Xyce lut_test.sp\"" >> commands;
        cat <<EOF > "${sub}/params.sp"
.param s1s2_pfet_width=${s1s2_pfet_width}m
.param s1s2_nfet_width=${s1s2_nfet_width}m
.param s0_pfet_width=${s0_pfet_width}m
.param s0_nfet_width=${s0_nfet_width}m
.param mux_nfet_width=580m
.param mux_pfet_width=460m
.param buffer_nfet_width=540m
.param buffer_pfet_width=900m
EOF
      done
    done
  done
done
