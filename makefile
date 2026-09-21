http_server:main.cc
	g++ -o $@ $^ -std=c++17 -Wall -Wextra -pedantic
.PHONY:clean
clean:
	rm -f http_server