# gftcpp

Minimal cli utility to pack/unpack tencent GFT image containers.

## building
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## usage
```bash
# pack an image into a gft container
./gftcpp --pack image.png output.gft

# unpack an image from a gft container
./gftcpp --unpack input.gft output.png
```
