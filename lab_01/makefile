CC := gcc-14

OUT_DIR := ./out
SRC_DIR := ./code/src
INC_DIR := ./code/inc

CFLAGS := -std=c99 -Wall -Werror -Wpedantic -Wextra -Wfloat-equal -O0 -Wfloat-conversion -I $(INC_DIR) -I /opt/homebrew/include
LFLAGS := -lm
CC := gcc

SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c, %.o, $(SRCS))

app.exe: $(OBJS)
	$(CC) $(patsubst %, $(OUT_DIR)/%, $(OBJS)) -o ./$@ $(LFLAGS)

%.o : $(SRC_DIR)/%.c
	mkdir -p $(OUT_DIR)
	$(CC) $(CFLAGS) -c $< -o $(OUT_DIR)/$@

%.d : $(SRC_DIR)/%.c
	mkdir -p $(OUT_DIR)
	$(CC) -MM $< > $(OUT_DIR)/$@


.PHONY : release
release: CFLAGS += -O0 -Wvla -Walloca
release: app.exe

.PHONY : debug
debug: CFLAGS  += -Wvla -Walloca -O0 -g3 -ggdb2 -fprofile-arcs -ftest-coverage
debug: LFLAGS  += -lgcov
debug: app.exe

.PHONY : debug_asan
debug_asan : CC := clang
debug_asan : CFLAGS += -O0 -fsanitize=address -fno-omit-frame-pointer -g
debug_asan : LFLAGS += -fsanitize=address -static-libsan
debug_asan : app.exe

.PHONY : debug_ubsan
debug_ubsan : CC := clang
debug_ubsan : CFLAGS += -O0 -fsanitize=undefined -fno-omit-frame-pointer -g
debug_ubsan : LFLAGS += -fsanitize=undefined
debug_ubsan : app.exe

# not supported on MacOS
# .PHONY : debug_msan
# debug_msan : CC := clang
# debug_msan : CFLAGS += -O0 -fsanitize=memory -fno-omit-frame-pointer -g
# debug_msan : LFLAGS += -fsanitize=memory -fPIE -pie
# debug_msan : app.exe

.PHONY : clean
clean :
	rm -f -- *.exe ./out/* *.gcno  *.gcda *.gcov temp.out 
