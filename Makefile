CC = riscv64-unknown-linux-gnu-gcc

OFLAGS := -O3 -DRISCV -march=rv64imafd -DUSE_PUBLIC_MALLOC_WRAPPERS
CFLAGS:= -O3 -DRISCV -march=rv64imafd

CFLAGS_CM:= -O3 -DRISCV -march=rv64imafd -Wl,--allow-multiple-definition


#CC = gcc
#CFLAGS := -O3 -std=gnu99

initialisation_pmc: initialisation_pmc.c
	${CC} ${CFLAGS} -o initialisation_pmc.riscv initialisation_pmc.c -lpthread

initialisation_sanitiser: initialisation_sanitiser.c
	${CC} ${CFLAGS} -o initialisation_sanitiser.riscv initialisation_sanitiser.c -lpthread

gc_main_pmc: gc_main_pmc.c
	${CC} ${CFLAGS} -c gc_main_pmc.c -lpthread

gc_main_none: gc_main_none.c
	${CC} ${CFLAGS} -c gc_main_none.c -lpthread

gc_main_sanitiser: gc_main_sanitiser.c
	${CC} ${CFLAGS_CM} -c gc_main_sanitiser.c -lpthread

malloc: malloc.c
	${CC} ${OFLAGS} -c malloc.c

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

