if [ -z "$SBCONFIG" ]; then
export SBCONFIG="$SS/ss-scheduler/configs/revel-1x1.sbmodel"
fi

if [ -z "$BACKCGRA"]; then
export BACKCGRA=1 
fi

make clean
timeout 120 make
FU_FIFO_LEN=15 DSCDIVLD=1 BACKCGRA=1 LINEAR_SCR=1              \
gem5.opt $SS/gem5/configs/example/se.py        \
--cpu-type=MinorCPU --l1d_size=256kB --l1d_assoc=16 \
--l1i_size=16kB --l2_size=128kB --caches           \
--ruby --num-cpus=8 --num-dirs=8 --network=simple  \
--topology=Mesh_XY --mesh-rows=2                   \
--cmd=main --mem-size=4GB
