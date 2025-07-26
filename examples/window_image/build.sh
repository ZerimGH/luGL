mkdir -p build

gcc \
core/main.c \
../../luGL/luGL.c \
-o build/main \
-lGLEW -lGL -lglfw -lm \
-I../../luGL -Istolen
