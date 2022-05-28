XCXX=arm-none-eabi-g++
XCXXFLAGS=--specs=nosys.specs -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
XCXXFLAGS+= -ffast-math
XCXXFLAGS+=-I${OPENCM3_DIR}/include -DSTM32F4
CXXFLAGS=-DNDEBUG -Wall
#CXXFLAGS+=-O2
CXXFLAGS+=-O1 -g
#Size of the LSTM's internal matrix. This value must track the value in the model.onnx
CXXFLAGS+=-DHIDDEN_SIZE=164
SEQUENCE_LENGTH=16
CXXFLAGS+=-DSEED_LEN=$(SEQUENCE_LENGTH)

XLDFLAGS=-L${OPENCM3_DIR}/lib -lopencm3_stm32f4 -Tstm32f411.ld -nostartfiles -Wl,--print-memory-usage -lm
LDFLAGS=-lm

all: melodygen 
.PHONY: flash clean

generated.c: model.onnx
	# batch size is named N, sequence length M1 in the model.onnx
	onnx2c -d N:1 -d M1:$(SEQUENCE_LENGTH)  $< > $@

melodygen: main.c audio.c midi.c vocab.c melody.c generated.c
	${XCXX} ${CXXFLAGS} ${XCXXFLAGS} $^ -o $@ ${XLDFLAGS}

flash: melodygen 
	openocd -f interface/stlink-v2.cfg -f target/stm32f4x.cfg -c "program melodygen verify reset exit"


desktop: desktop.c midi.c generated.c
	${CXX} ${CXXFLAGS} $^ -o $@ ${LDFLAGS}

clean:
	rm -f melodygen desktop generated.c
