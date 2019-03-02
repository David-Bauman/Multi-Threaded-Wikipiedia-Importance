CC = clang++
FLAGS = --std=c++11 -Werror -O3
LIBS = -lcurl -pthread 

runme: creatingImportance.cpp
	$(CC) $(FLAGS) $(LIBS) creatingImportance.cpp -o runme

usable: makeImportanceUsable.cpp
	$(CC) $(FLAGS) makeImportanceUsable.cpp -o usable

clean:
	${RM} runme usable
