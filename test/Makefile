functional_test: functional_test.cpp
	g++ -std=c++11 -I/Users/nick/repos -L/Users/nick/repos/optimus/test/build -lgtest_main -lgtest functional_test.cpp -I/Users/nick/repos/optimus/test/gtest-1.7.0/include -o /Users/nick/repos/optimus/test/build/functional_test
	
standard_operator_test: standard_operator_test.cpp
	g++ -std=c++11 standard_operator_test.cpp -I/Users/nick/repos -L/Users/nick/repos/optimus/test/build -lgtest_main -lgtest -I/Users/nick/repos/optimus/test/gtest-1.7.0/include -o /Users/nick/repos/optimus/test/build/standard_operator_test

transformer_test: transformer_test.cpp
	g++ -std=c++11 -I/Users/nick/repos -L/Users/nick/repos/optimus/test/build -lgtest_main -lgtest transformer_test.cpp -I/Users/nick/repos/optimus/test/gtest-1.7.0/include -o /Users/nick/repos/optimus/test/build/transformer_test

utility_test: utility_test.cpp
	g++ -std=c++11 -I/Users/nick/repos -L/Users/nick/repos/optimus/test/build -lgtest_main -lgtest utility_test.cpp -I/Users/nick/repos/optimus/test/gtest-1.7.0/include -o /Users/nick/repos/optimus/test/build/utility_test

tuple_test: tuple_test.cpp
	g++ -std=c++11 -I/Users/nick/repos -L/Users/nick/repos/optimus/test/build -lgtest_main -lgtest tuple_test.cpp -I/Users/nick/repos/optimus/test/gtest-1.7.0/include -o /Users/nick/repos/optimus/test/build/tuple_test

.PHONY:
all: functional_test standard_operator_test transformer_test utility_test tuple_test
	./build/functional_test
	./build/standard_operator_test
	./build/transformer_test
	./build/utility_test
	./build/tuple_test

.PHONY:
clean:
	rm -f build/functional_test
	rm -f build/standard_operator_test
	rm -f build/transformer_test
	rm -f build/utility_test
	rm -f build/tuple_test
