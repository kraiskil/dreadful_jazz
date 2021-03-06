XCXX=arm-none-eabi-gcc
XCXXFLAGS=--specs=nosys.specs -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
XCXXFLAGS+=-I${OPENCM3_DIR}/include -DSTM32F4
CXXFLAGS=-DNDEBUG -Wall
CXXFLAGS+=-Os
#CXXFLAGS+=-O1 -g
CXXFLAGS+=-DHIDDEN_SIZE=164

XLDFLAGS=-L${OPENCM3_DIR}/lib -lopencm3_stm32f4 -Tstm32f411.ld -nostartfiles -Wl,--print-memory-usage -lm
LDFLAGS=-lm

all: melodygen 
.PHONY: flash clean

generated.c: model.onnx
	# the driving code relies on onnx2c generating "buggy" code for LSTM.
	# Don't generate the code now since onnx2c fixed the bug.
	#onnx2c -v $< > $@

melodygen: main.c audio.c midi.c vocab.c melody.c generated.c
	${XCXX} ${CXXFLAGS} ${XCXXFLAGS} $^ -o $@ ${XLDFLAGS}

flash: melodygen 
	openocd -f interface/stlink-v2.cfg -f target/stm32f4x.cfg -c "program melodygen verify reset exit"


desktop: desktop.c midi.c generated.c
	${CXX} ${CXXFLAGS} $^ -o $@ ${LDFLAGS}

clean:
	rm -f melodygen desktop
