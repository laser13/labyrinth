.DEFAULT_GOAL = build

APP = labyrinth

CC := gcc
SRC := src
TEST := test
BUILD := build

clean:
	rm -rf $(BUILD)/

compile: clean
	mkdir $(BUILD)
	$(CC) $(DEBUG) -g -o $(BUILD)/$(APP) $(SRC)/laby.c $(SRC)/game.c $(SRC)/app.c

run: compile
	@$(BUILD)/$(APP)

test-compile: compile
	$(CC) -g -I$(SRC) -o $(BUILD)/tests $(SRC)/laby.c $(TEST)/tests.c

test: test-compile
	@$(BUILD)/tests

build: test
