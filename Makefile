# -*- coding: utf-8; tab-width: 4 -*-
# Author: Yasutaka SHINDOH / 新堂 安孝

SOURCE	:= $(wildcard *.cpp)
HEADER	:= $(wildcard *.hpp)
EXECUTE	:= sample

CXX			:= clang++
CXXFLAGS	:= -Wall -Weffc++ -O2 -std=c++11

check: $(EXECUTE)
	./$(EXECUTE)

$(EXECUTE): $(SOURCE) $(HEADER)
	# create: $@
	$(CXX) $(CXXFLAGS) $(SOURCE) -o $@

clean:
	rm -f $(EXECUTE)
	find . -name '*~' -print0 | xargs -0 rm -f

.PHONY: check clean
