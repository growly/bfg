#!/bin/bash

unalias cd
unalias bd

touch commands
truncate -s 0 commands

for mux_nfet_width in {420..660..40}; do
  for mux_pfet_width in {420..1000..40}; do
    for buffer_nfet_width in {420..660..40}; do
      for buffer_pfet_width in {420..1000..40}; do
        sub="run_${buffer_nfet_width}_${buffer_pfet_width}_${mux_nfet_width}_${mux_pfet_width}";
        mkdir -p $sub;
        cp -r *.sp "${sub}/";
        cat <<EOF > "${sub}/params.sp"
.param mux_nfet_width=${mux_nfet_width}m
.param mux_pfet_width=${mux_pfet_width}m
.param buffer_nfet_width=${buffer_nfet_width}m
.param buffer_pfet_width=${buffer_pfet_width}m
EOF
        echo -e "bash -c \"cd ${sub}; Xyce lut_test.sp\"" >> commands;
      done
    done
  done
done

