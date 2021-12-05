Demo that plays melodies generated on-the-fly using neural networks on a STM32F4-Discovery board (ARM CortexM4). [Sample video here](melodygen.mp4).

This is the network based on the one from Valerio Velardo's [video tutorials](https://www.youtube.com/playlist?list=PL-wATfeyAMNr0KMutwtbeDCmpwvtul-Xz). The example network from that video would not fit in memory, so the size of the LSTM had to be decreased. This seems to affect the "quality" of the generated melody. 

The network training code is not provided, only a final .onnx file of the trained network is available here. The network is compiled to C with [onnx2c](https://github.com/kraiskil/onnx2c).


Notes a year later
------------------
This project was "the end project" for my studies into neural networks and AI during the corona lockdown of 2020. A side effect was that this project was finished before it got finalized. E.g. the 'desktop' simulator target has bitrotted.

Also adding `-ffast-math` compiler option would decrease the flash usage (and likely processing time) a lot - so the original LSTM size from Velardo's example would have fit into memory after all.

Finally the onnx2c implementation had a bug in the LSTM implementation caused by a lack of understanding on how LSTM works (internal states should be reset at start of a sequence). The original network had a sequence length of 1, and reset the LSTM state from outside. This bug has been fixed in onnx2c since, and code generated with a contemporary onnx2c does not work correctly anymore.
