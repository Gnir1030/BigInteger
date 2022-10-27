# $Id: Makefile,v 1.8 2022-04-05 11:32:20-07 - - $

MKFILE      = Makefile
DEPSFILE    = ${MKFILE}.deps
NOINCL      = clean tidy tar submit
NEEDINCL    = ${filter ${NOINCL}, ${MAKECMDGOALS}}
GMAKE       = ${MAKE} --no-print-directory
GPPOPTS     = -std=gnu++20 -fdiagnostics-color=always
GPPWARN     = -Wall -Wextra -Wpedantic -Wshadow -Wold-style-cast
GPP         = g++ ${GPPOPTS} ${GPPWARN}
COMPILECPP  = ${GPP} -g -O0 ${GPPOPTS}
MAKEDEPSCPP = ${GPP} -MM ${GPPOPTS}

MODULES     = ubigint bigint libfns scanner debug util
CPPHEADER   = ${MODULES:=.h} iterstack.h relops.h
CPPSOURCE   = ${MODULES:=.cpp} main.cpp
EXECBIN     = ydc
OBJECTS     = ${CPPSOURCE:.cpp=.o}
MODULESRC   = ${foreach MOD, ${MODULES}, ${MOD}.h ${MOD}.cpp}
OTHERSRC    = ${filter-out ${MODULESRC}, ${CPPHEADER} ${CPPSOURCE}}
ALLSOURCES  = ${MODULESRC} ${OTHERSRC} ${MKFILE}
TARFILE     = ../asgn4.tar.gz

all : ${EXECBIN}

${EXECBIN} : ${OBJECTS}
	${COMPILECPP} -o $@ ${OBJECTS}

%.o : %.cpp
	${COMPILECPP} -c $<

submit: tar

tar : ${ALLSOURCES}
	tar cvfz ${TARFILE} $^

tidy :
	$(RM) ${OBJECTS} ${DEPSFILE}

clean : tidy
	$(RM) ${EXECBIN}


deps : ${CPPSOURCE} ${CPPHEADER}
	@ echo "# ${DEPSFILE} created $$(LC_TIME=C date)" >${DEPSFILE}
	${MAKEDEPSCPP} ${CPPSOURCE} >>${DEPSFILE}

${DEPSFILE} :
	@ touch ${DEPSFILE}
	${GMAKE} deps

again :
	${GMAKE} clean deps all

ifeq (${NEEDINCL}, )
include ${DEPSFILE}
endif

