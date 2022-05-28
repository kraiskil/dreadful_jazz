XCXX=arm-none-eabi-gcc
XCXXFLAGS=--specs=nosys.specs -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
XCXXFLAGS+= -ffast-math
XCXXFLAGS+=-I${OPENCM3_DIR}/include -DSTM32F4
CXXFLAGS=-DNDEBUG -Wall
CXXFLAGS+=-Og -g
#CXXFLAGS+=-O2 -g
#Size of the LSTM's internal matrix. This value must track the value in the model.onnx
CXXFLAGS+=-DHIDDEN_SIZE=164
SEQUENCE_LENGTH=8
CXXFLAGS+=-DSEED_LEN=$(SEQUENCE_LENGTH)

XLDFLAGS=-L${OPENCM3_DIR}/lib -lopencm3_stm32f4 -Tstm32f411.ld -nostartfiles -Wl,--print-memory-usage -lm
LDFLAGS=-lm

all: melodygen 
.PHONY: flash clean

generated.c: model.onnx
	# batch size is named N, sequence length M1 in the model.onnx
	onnx2c -d N:1 -d M1:$(SEQUENCE_LENGTH)  $< > $@
generated.o: generated.c
	${XCXX} ${CXXFLAGS} ${XCXXFLAGS} -O4 -c $^ -o $@ 

melodygen: main.c audio.c midi.c vocab.c melody.c generated.o
	${XCXX} ${CXXFLAGS} ${XCXXFLAGS} $^ -o $@ ${XLDFLAGS}

flash: melodygen 
	openocd -f interface/stlink-v2.cfg -f target/stm32f4x.cfg -c "program melodygen verify reset exit"


desktop: desktop.c midi.c generated.c
	${CXX} ${CXXFLAGS} $^ -o $@ ${LDFLAGS}

clean:
	rm -f melodygen desktop generated.c
