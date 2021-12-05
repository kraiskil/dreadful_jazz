Demo that plays melodies generated on-the-fly using neural networks on a STM32F4-Discovery board (ARM CortexM4). [Sample video here](melodygen.mp4).

This is the network based on the one from Valerio Velardo's [video tutorials](https://www.youtube.com/playlist?list=PL-wATfeyAMNr0KMutwtbeDCmpwvtul-Xz). The example network from that video would not fit in memory, so the size of the LSTM had to be decreased. This seems to affect the "quality" of the generated melody. 

The network training code is not provided, only a final .onnx file of the trained network is available here. The network is compiled to C with [onnx2c](https://github.com/kraiskil/onnx2c).

