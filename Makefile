CC = riscv64-unknown-linux-gnu-gcc
CPP = riscv64-unknown-linux-gnu-g++

OFLAGS := -O3 -DRISCV -march=rv64imafd -DUSE_PUBLIC_MALLOC_WRAPPERS
CFLAGS:= -O3 -DRISCV -march=rv64imafd

CFLAGS_CM:= -O3 -DRISCV -march=rv64imafd -Wl,--allow-multiple-definition


#CC = gcc
#CFLAGS := -O3 -std=gnu99

initialisation_pmc: initialisation_pmc.c
	${CC} ${CFLAGS} -o initialisation_pmc.riscv initialisation_pmc.c -lpthread

initialisation_sanitiser: initialisation_sanitiser.c
	${CC} ${CFLAGS} -o initialisation_sanitiser.riscv initialisation_sanitiser.c -lpthread

initialisation_ss: initialisation_ss.cpp
	${CPP} ${CFLAGS} -o initialisation_ss.riscv initialisation_ss.cpp -lpthread

initialisation_ss_mc: initialisation_ss_mc.cpp
	${CPP} ${CFLAGS} -o initialisation_ss_mc.riscv initialisation_ss_mc.cpp -lpthread



malloc: malloc.c
	${CC} ${OFLAGS} -c malloc.c

gc_main_none: gc_main_none.c
	${CC} ${CFLAGS} -c gc_main_none.c -lpthread

gc_main_pmc: gc_main_pmc.c
	${CC} ${CFLAGS} -c gc_main_pmc.c -lpthread

gc_main_sanitiser: gc_main_sanitiser.c
	${CC} ${CFLAGS_CM} -c gc_main_sanitiser.c -lpthread

gc_main_ss: gc_main_ss.cpp
	${CPP} ${CFLAGS_CM} -c gc_main_ss.cpp -lpthread

gc_main_ss_mc: gc_main_ss_mc.c
	${CC} ${CFLAGS_CM} -c gc_main_ss_mc.c -lpthread

gc_main_ss_mc_c1: gc_main_ss_mc_c1.cpp
	${CPP} ${CFLAGS} -c gc_main_ss_mc_c1.cpp -lpthread

gc_main_ss_mc_c2: gc_main_ss_mc_c2.cpp
	${CPP} ${CFLAGS} -c gc_main_ss_mc_c2.cpp -lpthread

gc_main_ss_mc_c3: gc_main_ss_mc_c3.cpp
	${CPP} ${CFLAGS} -c gc_main_ss_mc_c3.cpp -lpthread

gc_main_ss_mc_agg: gc_main_ss_mc_agg.cpp
	${CPP} ${CFLAGS} -c gc_main_ss_mc_agg.cpp -lpthread

################################################
# Some test casese
################################################
tc_pmc: tc_pmc.c
	${CC} ${CFLAGS} -o tc_pmc.riscv tc_pmc.c -lpthread

tc_sanitiser: tc_sanitiser.c
	${CC} ${CFLAGS_CM} -o tc_sanitiser.riscv tc_sanitiser.c malloc.o -lpthread

tc_shadowstack-s: tc_shadowstack-s.c
	${CC} ${CFLAGS} -o tc_shadowstack-s.riscv tc_shadowstack-s.c -lpthread

tc_shadowstack-m: tc_shadowstack-m.c
	${CC} ${CFLAGS} -o tc_shadowstack-m.riscv tc_shadowstack-m.c -lpthread

clean:
	rm -f *.o *.riscv *.so

