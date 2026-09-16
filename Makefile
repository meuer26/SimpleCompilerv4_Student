# This file is licensed under the MIT License. See LICENSE for details.
#
# Toolchain required:
#   sudo apt install flex bison nasm clang
#   the LLVM `llc` binary (the LLVM IR is compiled, linked, and its answer
#   is checked for correctness — llc errors are fatal, not skipped)


CC       ?= cc
NASM     ?= nasm
LLC      ?= llc
HOST_CC  ?= clang

.PHONY: default build emit-x64 assemble link-driver clean

default: build emit-x64 assemble link-driver

build:
	mkdir -p ./build ./logs
	flex  -o ./build/lex.yy.c lex.l
	bison -d -v -Wcounterexamples --report=all --report-file=./build/parser.output -o ./build/parser.tab.c parser.y
	$(CC) -Wall -Wextra -I./build -I. \
	      ast.c symboltable.c semantic.c typecheck.c constfold.c regalloc.c liveness.c \
	      emit_llvm.c emit_x64.c main.c \
	      ./build/parser.tab.c ./build/lex.yy.c \
	      -o ./build/compiler

emit-x64:
	./build/compiler --emit=x64  $(prog) > ./build/prog.s     2> ./logs/x64.log
	./build/compiler --emit=llvm $(prog) > ./build/llvm_ir.ll 2> ./logs/llvm.log

assemble:
	$(NASM) -felf64 ./build/prog.s -o ./build/prog.o
	$(LLC) ./build/llvm_ir.ll -filetype=obj -o ./build/llvm_ir.o -O0

link-driver:
	@if [ "$(type)" = "oo-int" ]; then \
	    $(HOST_CC) -x c++ -fno-rtti -c $(prog) -o ./build/baseline_evaluator.o; \
	else \
	    $(HOST_CC) -x c -c $(prog) -o ./build/baseline_evaluator.o; \
	fi
	$(HOST_CC) -no-pie ./test-cases/$(type)/driver.cpp    ./build/prog.o                                 -o ./build/driver_x64
	$(HOST_CC) -no-pie ./test-cases/$(type)/evaluator.cpp ./build/prog.o    ./build/baseline_evaluator.o -o ./build/evaluator_x64
	$(HOST_CC) -no-pie ./test-cases/$(type)/driver.cpp    ./build/llvm_ir.o                              -o ./build/driver_llvm
	$(HOST_CC) -no-pie ./test-cases/$(type)/evaluator.cpp ./build/llvm_ir.o ./build/baseline_evaluator.o -o ./build/evaluator_llvm
	@echo
	@echo "===== $(prog) ====="
	@cat $(prog)
	@echo "================="
	@echo
	@echo "----- x86-64 backend (answer check) -----"
	./build/evaluator_x64
	@echo "----- LLVM backend (answer check) -----"
	./build/evaluator_llvm

clean:
	rm -rf ./build ./logs
