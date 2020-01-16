BUILD_DIR?=build
MINISATP_RELSYM?=
MINISATP_REL?=-std=c++11 -O3 -D NDEBUG -Wno-strict-aliasing -D COMINISATPS 
MINISATP_DEB?=-std=c++11 -O0 -D DEBUG  -Wno-strict-aliasing -D COMINISATPS 
MINISATP_PRF?=-std=c++11 -O3 -D NDEBUG -Wno-strict-aliasing -D COMINISATPS 
MINISATP_FPIC?=-fpic
MINISAT_INCLUDE?=-I/include -I/include/minisat -I../cominisatps
MINISAT_LIB?=-L/lib -L../cominisatps/simp -l_release
MCL_INCLUDE?=
MCL_LIB?=
prefix?=
