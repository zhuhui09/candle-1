CC = cc
LD = cc
AR = ar

emscripten: CC = emcc
emscripten: LD = emcc
emscripten: AR = emar

DIR = build

EMS_OPTS = -s USE_GLFW=3 -s USE_WEBGL2=1 \
-s FULL_ES3=1 -s EMULATE_FUNCTION_POINTER_CASTS=1 \
-s WASM_MEM_MAX=2GB -O3 -s WASM=1 -s ASSERTIONS=1 \
-s ALLOW_MEMORY_GROWTH=1 -s SAFE_HEAP=1 -s ERROR_ON_UNDEFINED_SYMBOLS=0

LIBS = -lm -lGL -pthread -ldl -lX11 -lrt
LDFLAGS_REL = $(LIBS) candle/$(DIR)/candle.a \
-Wl,--format=binary -Wl,candle/$(DIR)/data.zip -Wl,--format=default
LDFLAGS_DEB = $(LIBS) candle/$(DIR)/candle_debug.a \
-Wl,--format=binary -Wl,candle/$(DIR)/data.zip -Wl,--format=default
LDFLAGS_EMS = -lm -lGL -ldl -lrt candle/$(DIR)/candle_emscripten.a \
			  $(EMS_OPTS) --preload-file candle/$(DIR)/data.zip@/ --shell-file index.html

GLFW = third_party/glfw/src
TINYCTHREAD = third_party/tinycthread/source

PLUGINS = $(wildcard ../*.candle)
PLUGINS_DIR = $(patsubst %, %/$(DIR), $(PLUGINS))
PLUGINS_REL = $(patsubst %, %/libs, $(PLUGINS_DIR))
PLUGINS_DEB = $(patsubst %, %/libs_debug, $(PLUGINS_DIR))
PLUGINS_EMS = $(patsubst %, %/libs_emscripten, $(PLUGINS_DIR))

THIRD_PARTY_SRC = \
		   $(GLFW)/context.c \
		   $(GLFW)/init.c \
		   $(GLFW)/input.c \
		   $(GLFW)/monitor.c \
		   $(GLFW)/window.c \
		   $(GLFW)/vulkan.c \
		   $(GLFW)/egl_context.c \
		   $(GLFW)/glx_context.c \
		   $(GLFW)/linux_joystick.c \
		   $(GLFW)/osmesa_context.c \
		   $(GLFW)/posix_thread.c \
		   $(GLFW)/posix_time.c \
		   $(GLFW)/x11_init.c \
		   $(GLFW)/x11_monitor.c \
		   $(GLFW)/x11_window.c \
		   $(GLFW)/xkb_unicode.c \
		   $(TINYCTHREAD)/tinycthread.c

SRCS = $(wildcard *.c) $(wildcard components/*.c) $(wildcard systems/*.c) \
	   $(wildcard formats/*.c) $(wildcard utils/*.c) $(wildcard vil/*.c) \
	   $(wildcard ecs/*.c) third_party/miniz.c

OBJS_REL = $(patsubst %.c, $(DIR)/%.o, $(SRCS) $(THIRD_PARTY_SRC))
OBJS_DEB = $(patsubst %.c, $(DIR)/%.debug.o, $(SRCS) $(THIRD_PARTY_SRC))
OBJS_EMS = $(patsubst %.c, $(DIR)/%.emscripten.o, $(SRCS))

CFLAGS = -std=c89 -pedantic -Wall -I. -Wno-unused-function \
         -Ithird_party/glfw/include -I$(TINYCTHREAD) -D_GLFW_X11 \
         -Wno-format-truncation -Wno-stringop-truncation \
         -DTHREADED -DUSE_VAO

CFLAGS_REL = $(CFLAGS) -O3

CFLAGS_DEB = $(CFLAGS) -g3 -DDEBUG

CFLAGS_EMS = -Wall -I. -Wno-unused-function \
			 -Ithird_party/glfw/include -D_GLFW_X11 $(EMS_OPTS)

PARALLEL_REL = $(DIR)/candle.a data_zip
PARALLEL_DEB = $(DIR)/candle_debug.a data_zip
PARALLEL_EMS = $(DIR)/candle_emscripten.a data_zip

##############################################################################

all: init $(PLUGINS_REL) $(PARALLEL_REL)
	-cat "" $(PLUGINS_REL) | paste -sd " " > $(DIR)/libs
	echo -n " $(LDFLAGS_REL)" >> $(DIR)/libs

$(DIR)/candle.a: $(OBJS_REL)
	$(AR) rs $@ $(OBJS_REL)

$(DIR)/%.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_REL)

../%/$(DIR)/libs:
	$(MAKE) -C ../$*
	echo $(patsubst %, ../$*/%, $(shell cat ../$*/$(DIR)/res)) >> $(DIR)/res

# utils/gl.c:
# 	galogen gl.xml --api gl --ver 3.0 --profile core --exts $(EXTS)
# 	mv gl.c gl.h utils/

##############################################################################

debug: init $(BUILDS_DEB) $(PARALLEL_DEB)
	-cat "" $(PLUGINS_DEB) | paste -sd " " > $(DIR)/libs_debug
	echo -n " $(LDFLAGS_DEB)" >> $(DIR)/libs_debug

$(DIR)/candle_debug.a: $(OBJS_DEB)
	$(AR) rs $@ $(OBJS_DEB)

$(DIR)/%.debug.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_DEB)

../%/$(DIR)/libs_debug:
	$(MAKE) -C ../$* debug
	echo $(patsubst %, ../$*/%, $(shell cat ../$*/$(DIR)/res)) >> $(DIR)/res


##############################################################################

emscripten: init $(PLUGINS_EMS) $(PARALLEL_EMS)
	-cat "" $(PLUGINS_EMS) | paste -sd " " > $(DIR)/libs_emscripten
	echo -n " $(LDFLAGS_EMS)" >> $(DIR)/libs_emscripten

$(DIR)/candle_emscripten.a: $(OBJS_EMS)
	$(AR) rs $@ $(OBJS_EMS)

$(DIR)/%.emscripten.o: %.c
	$(CC) -o $@ -c $< $(CFLAGS_EMS)

../%/$(DIR)/libs_emscripten:
	$(MAKE) -C ../$* emscripten
	echo $(patsubst %, ../$*/%, $(shell cat ../$*/$(DIR)/res)) >> $(DIR)/res

##############################################################################

data_zip: $(DIR)/packager
	$(DIR)/packager ../$(SAUCES) $(shell cat $(DIR)/res)

$(DIR)/packager: buildtools/packager.c
	$(CC) buildtools/packager.c third_party/miniz.c -O3 -o $(DIR)/packager

##############################################################################

init:
	mkdir -p $(DIR)
	mkdir -p $(DIR)/components
	mkdir -p $(DIR)/systems
	mkdir -p $(DIR)/utils
	mkdir -p $(DIR)/formats
	mkdir -p $(DIR)/vil
	mkdir -p $(DIR)/ecs
	mkdir -p $(DIR)/$(GLFW)
	mkdir -p $(DIR)/$(TINYCTHREAD)
	rm -f $(DIR)/res

##############################################################################

clean:
	rm -fr $(DIR)
	rm -fr $(PLUGINS_DIR)

.PHONY: clean all init data_zip
