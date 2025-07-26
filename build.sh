mkdir -p build

gcc \
source/core/main.c \
source/graphics/luGL.c \
-o build/main \
-lGLEW -lGL -lglfw -lm \
-Isource/core -Isource/graphics -Isource/stolen
