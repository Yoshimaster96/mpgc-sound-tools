rem	Make dump_pdt
gcc -o dump_pdt dump_pdt.c serialize.c
rem	Make dump_msm
gcc -o dump_msm dump_msm.c serialize.c
rem	Make dump_msm_sng
gcc -o dump_msm_sng dump_msm_sng.c serialize.c
rem	Make bank2sf2
g++ -o bank2sf2 bank2sf2.cpp serialize.c -Isf2cute/include -Lsf2cute -lsf2cute

pause
