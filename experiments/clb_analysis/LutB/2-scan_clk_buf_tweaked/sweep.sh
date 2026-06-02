#!/bin/bash

unalias cd
unalias bd

for cbn in {420..660..10}; do
  for cbp in {420..1000..20}; do
    sub=run_${cbn}_${cbp};
    mkdir -p $sub;
    cp -r *.sp "${sub}/";
    echo -e ".param cbn=${cbn}m\n.param cbp=${cbp}m\n" > "${sub}/params.sp";
    echo -e "bash -c \"cd ${sub}; Xyce lut_test.sp\"" >> commands;
  done
done

