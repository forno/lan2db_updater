all:  lan2db_updater

lan2db_updater: main.cpp
	clang++ -std=c++14 -I/usr/local/include/cppconn main.cpp -o lan2db_updater -lmysqlcppconn

install:  all
	install -m 710 lan2db_updater /usr/local/bin/

uninstall:
	rm /usr/local/bin/lan2db_updater
