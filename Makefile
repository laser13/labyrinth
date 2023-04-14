.DEFAULT_GOAL = build

NAME := Labyrinth

CC := gcc
SRC := src
TEST := test
BUILD := build
BIN = `echo $(NAME) | tr A-Z a-z`

clean:
	rm -rf $(BUILD)/

compile: clean
	mkdir $(BUILD)
	$(CC) -o $(BUILD)/$(BIN) $(SRC)/app.c

run: compile
	@$(BUILD)/$(NAME)

test-compile: compile
	$(CC) -I$(SRC) -o $(BUILD)/run_tests $(TEST)/laby_test.c

test: test-compile
	@$(BUILD)/run_tests

build: compile test
