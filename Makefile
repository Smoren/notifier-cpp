CMD := $(firstword $(MAKECMDGOALS))
ARGS := $(wordlist 2,$(words $(MAKECMDGOALS)),$(MAKECMDGOALS))

%::
	@true

CXX = g++
CXXFLAGS = -g -Wall -O0

.PHONY: all

build-release:
	cmake --build cmake-build-release --target notifier_cpp --config Release -j 18
	@echo "[ COMPLETE: RELEASE BUILD ]"

build-debug:
	cmake --build cmake-build-debug --target notifier_cpp --config Debug -j 10
	@echo "[ COMPLETE: DEBUG BUILD ]"

build-tests:
	cmake --build cmake-build-debug --target notifier_tests --config Debug -j 10
	@echo "[ COMPLETE: TESTS BUILD ]"

run-release:
	cd ./cmake-build-release/ && ./notifier_cpp

run-debug:
	cd ./cmake-build-debug/ && ./notifier_cpp

run-tests:
	cd ./cmake-build-debug/ && ./notifier_tests

test:
	cmake --build cmake-build-debug --target notifier_tests --config Debug -j 10
	cd ./cmake-build-debug/ && ./notifier_tests

init:
	cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release -DCMAKE_CXX_FLAGS="$(CXXFLAGS)"
	cmake -S . -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_FLAGS="$(CXXFLAGS)"
	@echo "[ COMPLETE ]"

clean:
	rm -rf cmake-build-debug cmake-build-release
	@echo "[ CLEANED ]"
