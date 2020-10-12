if [ -z "$SBCONFIG" ]; then
export SBCONFIG="$SS/ss-scheduler/configs/revel-1x1.sbmodel"
fi

make clean
timeout 120 make $1
BACKCGRA=1 FU_FIFO_LEN=15                          \
gem5.opt $SS/gem5/configs/example/se.py            \
--cpu-type=MinorCPU --l1d_size=16kB --l1d_assoc=16 \
--l1i_size=16kB --l2_size=128kB --caches           \
--ruby --num-cpus=8 --num-dirs=8 --network=simple  \
--topology=Mesh_XY --mesh-rows=2                   \
--cmd=./$1
