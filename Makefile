# This file is licensed under the MIT License. See LICENSE for details.
#
# Toolchain required:
#   sudo apt install flex bison nasm clang
# Optional (used only to assemble the LLVM IR for inspection):
#   the LLVM `llc` binary (skipped silently if missing)


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
	      ast.c symboltable.c semantic.c constfold.c regalloc.c liveness.c \
	      emit_llvm.c emit_x64.c main.c \
	      ./build/parser.tab.c ./build/lex.yy.c \
	      -o ./build/compiler

emit-x64:
	./build/compiler --emit=x64  $(prog) > ./build/prog.s 2> ./logs/x64.log
	./build/compiler --emit=llvm $(prog) > ./build/frontend_ir.ll 2> ./logs/llvm.log
	@command -v $(LLC) >/dev/null && $(LLC) ./build/frontend_ir.ll -filetype=obj -o ./build/frontend_ir.o -O0 || true

assemble:
	$(NASM) -felf64 ./build/prog.s -o ./build/prog.o

link-driver:
	$(HOST_CC) -no-pie ./test-cases/$(type)/driver.cpp ./build/prog.o -o ./build/driver
	@if [ "$(type)" = "oo-int" ]; then \
	    $(HOST_CC) -x c++ -fno-rtti -c $(prog) -o ./build/baseline_evaluator.o; \
	else \
	    $(HOST_CC) -x c -c $(prog) -o ./build/baseline_evaluator.o; \
	fi
	$(HOST_CC) -no-pie ./test-cases/$(type)/evaluator.cpp ./build/prog.o ./build/baseline_evaluator.o -o ./build/evaluator
	@echo
	@echo "===== $(prog) ====="
	@cat $(prog)
	@echo "================="
	@echo
	./build/evaluator

clean:
	rm -rf ./build ./logs
