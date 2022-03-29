CXX = arm-linux-g++
CLAGS = -ansi -Wall -g -Llib  
INCLUDE = -Iinclude -Ipi_mobile/axclib -Isqlite3_arm/include
LIBS = -laxclib  -lsqlite3


test: main.o ulis_encode.o net.o dbm.o spi.o adlr064.o 
	$(CXX) $(CLAGS)  main.o ulis_encode.o net.o dbm.o spi.o adlr064.o  $(LIBS) -o test

main.o: 
	$(CXX) $(INCLUDE) -c  src/main.cpp -o main.o

ulis_encode.o:
	$(CXX) $(INCLUDE) -c src/ulis_encode.cpp -o ulis_encode.o

net.o:
	$(CXX) $(INCLUDE) -c src/net.cpp -o net.o

dbm.o:
	$(CXX) $(INCLUDE) -c src/dbm.cpp -o dbm.o

spi.o:
	$(CXX) $(INCLUDE) -c src/spi.cpp -o spi.o

adlr064.o:
	$(CXX) $(INCLUDE) -c src/adlr064.cpp -o adlr064.o


.PHONY: clean
clean:
	rm *.o test




