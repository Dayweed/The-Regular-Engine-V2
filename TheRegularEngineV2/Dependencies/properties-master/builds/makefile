all: clean
	@mkdir ../release
	@${CXX} -Wall ${CXXFLAGS} -o ../release/UnitTest ../src/Examples/UnitTest/*.cpp ${CXXOPTIONS}

clean:
	@rm -rf ../release