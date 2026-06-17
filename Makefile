# Makefile for C-Minus Compiler
# Compiler and flags
CC = gcc
CFLAGS = -std=c99 -Wall -I.
CC_C = $(CC) -x c
LDFLAGS =

# Source files and object files
CORE_SOURCES = SCAN.C UTIL.C
CORE_OBJECTS = $(CORE_SOURCES:.C=.o)

# Targets
.PHONY: all clean test test-scan test-parse help
.SUFFIXES: .C .o

all: test/scan/run_tests test/parse/run_tests

# Test runners
test-scan: test/scan/run_tests

test-parse: test/parse/run_tests

test: test-scan test-parse
test-scan:
	./test/scan/run_tests

test-parse:
	./test/parse/run_tests


# Compile object files - explicit rules for Windows compatibility
SCAN.o: SCAN.C
	$(CC_C) $(CFLAGS) -c $< -o $@

UTIL.o: UTIL.C
	$(CC_C) $(CFLAGS) -c $< -o $@

# Build scanner test executable
test/scan/scanner_test: test/scan/scanner_test.c $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Build scanner batch test executable
test/scan/run_tests: test/scan/run_tests.c $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)

# Build parser test executable
test/parse/parser_test: test/parse/parser_test.c $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $^ parse.c -o $@ $(LDFLAGS)

# Build parser batch test executable
test/parse/run_tests: test/parse/run_tests.c $(CORE_OBJECTS)
	$(CC) $(CFLAGS) $^ parse.c -o $@ $(LDFLAGS)

# Run scanner_test on a single input file
# Usage: make test_file FILE=test/inputs/simple_tokens.cm
test_file: test/scan/scanner_test
	@if [ -z "$(FILE)" ]; then \
		echo "Usage: make test_file FILE=<test-input-file>"; \
		exit 1; \
	fi
	./test/scan/scanner_test "$(FILE)"

# Clean build artifacts
clean:
	cmd /C "if exist SCAN.o del /Q /F SCAN.o"
	cmd /C "if exist UTIL.o del /Q /F UTIL.o"
	cmd /C "if exist test\\scan\\scanner_test del /Q /F test\\scan\\scanner_test"
	cmd /C "if exist test\\scan\\scanner_test.exe del /Q /F test\\scan\\scanner_test.exe"
	cmd /C "if exist test\\scan\\run_tests del /Q /F test\\scan\\run_tests"
	cmd /C "if exist test\\scan\\run_tests.exe del /Q /F test\\scan\\run_tests.exe"
	cmd /C "if exist test\\parse\\run_tests del /Q /F test\\parse\\run_tests"
	cmd /C "if exist test\\parse\\run_tests.exe del /Q /F test\\parse\\run_tests.exe"
	cmd /C "if exist test\\scan\\outputs rmdir /S /Q test\\scan\\outputs"
	cmd /C "if exist test\\parse\\outputs rmdir /S /Q test\\parse\\outputs"

# Help message
help:
	@echo "C-Minus Compiler Build System"
	@echo "=============================="
	@echo "make              - Build all test targets"
	@echo "make test-scan    - Run scanner tests"
	@echo "make test-parse   - Run parser tests"
	@echo "make test         - Run both scanner and parser tests"
	@echo "make test_file FILE=path - Run scanner on single file"
	@echo "make clean        - Remove build artifacts"
	@echo "make help         - Show this message"
	@echo ""
	@echo "Example: make test_file FILE=test/scan/inputs/simple_tokens.cm"