# PAID-IO Optimization of BlueTides

IO_Simulator for the BlueTides Project using the bigfile library

Run on COMA:
mpirun -n 64 ./iosim -N 4000 -n 64 -s 800000 -w 3 -p ./ -m create -d Test

++++++++++ mpirun ++++++++++

 -n ... number of ranks

++++++++++ iosim +++++++++++
 'Test' ... filename
 -d ... flag to delete files after complete run
 
 -m ... mode: <create/update/read> of files
 
 -q ... path: to access or write files in other folders
 
 -w ... width: defines if items are dim 1 or dim 3
 
 -s ... Nitems: sum of items of dim <width> and format 'i4'
 
 -> total number of bytes written in files: Nitems x width x 4bytes
 
 -n ... Nwriters: number of writers (should be <= number of ranks)
 
 -N ... Nfiles: number of files (items are equally subdivided into files)
