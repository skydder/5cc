all:
	cd src && make
	cd test && python3 test.py

clean:
	cd src && make clean
	cd test && rm -f tmp*