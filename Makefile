# Makefile for C-Minus Compiler

CC = gcc
CFLAGS = -std=c99 -Wall -I.
LDFLAGS =

CORE_OBJECTS = globals.o scan.o util.o
FRONTEND_OBJECTS = parse.o analyze.o
TEST_COMMON_OBJECT = test/test_runner.o
TEST_RUNNERS = \
	test/scan/run_tests \
	test/parse/run_tests \
	test/analyze/run_tests \
	test/compiler/run_tests

.PHONY: all clean test test-scan test-parse test-analyze test-compiler test-complier help
.SUFFIXES: .c .o

all: $(TEST_RUNNERS)

test: test-scan test-parse test-analyze test-compiler

test-scan: test/scan/run_tests
	./test/scan/run_tests

test-parse: test/parse/run_tests
	./test/parse/run_tests

test-analyze: test/analyze/run_tests
	./test/analyze/run_tests

test-compiler: test/compiler/run_tests
	./test/compiler/run_tests

# Compatibility alias for the spelling used in the assignment notes.
test-complier: test-compiler

globals.o: globals.c globals.h
	$(CC) $(CFLAGS) -c $< -o $@

scan.o: scan.c scan.h globals.h util.h
	$(CC) $(CFLAGS) -c $< -o $@

util.o: util.c util.h globals.h
	$(CC) $(CFLAGS) -c $< -o $@

parse.o: parse.c parse.h scan.h util.h globals.h
	$(CC) $(CFLAGS) -c $< -o $@

analyze.o: analyze.c analyze.h util.h globals.h
	$(CC) $(CFLAGS) -c $< -o $@

compiler.o: compiler.c compiler.h analyze.h parse.h scan.h globals.h
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST_COMMON_OBJECT): test/test_runner.c test/test_runner.h globals.h
	$(CC) $(CFLAGS) -c $< -o $@

test/scan/run_tests: test/scan/run_tests.c $(TEST_COMMON_OBJECT) $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test/parse/run_tests: test/parse/run_tests.c $(TEST_COMMON_OBJECT) parse.o $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test/analyze/run_tests: test/analyze/run_tests.c $(TEST_COMMON_OBJECT) analyze.o parse.o $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test/compiler/run_tests: test/compiler/run_tests.c $(TEST_COMMON_OBJECT) compiler.o analyze.o parse.o $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test/scan/scanner_test: test/scan/scanner_test.c $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test/parse/parser_test: test/parse/parser_test.c parse.o $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

gen_parse_output: gen_parse_output.c parse.o $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_file: test/scan/scanner_test
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make test_file FILE=<test-input-file>"; \
		exit 1; \
	fi
	./test/scan/scanner_test "$(FILE)"

clean:
	rm -f *.o test/*.o
	rm -f $(TEST_RUNNERS)
	rm -f test/scan/scanner_test test/parse/parser_test gen_parse_output
	rm -rf test/scan/outputs test/parse/outputs test/analyze/outputs test/compiler/outputs

help:
	@echo "C-Minus Compiler Build System"
	@echo "=============================="
	@echo "make                 - Build all test runners"
	@echo "make test-scan       - Test lexical analysis only"
	@echo "make test-parse      - Test parsing with lexically valid inputs"
	@echo "make test-analyze    - Test semantic analysis with valid front-end inputs"
	@echo "make test-compiler   - Test the complete compiler pipeline"
	@echo "make test-complier   - Compatibility alias for test-compiler"
	@echo "make test            - Run all four test suites"
	@echo "make test_file FILE=path - Run scanner on one input file"
	@echo "make clean           - Remove build artifacts and generated outputs"
