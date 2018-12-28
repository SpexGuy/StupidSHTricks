# Stupid SH Tricks

A visualizer for spherical harmonics

## Building

This project can be built with cmake.
If you don't want to deal with that though, prebuilt binaries for mac and windows are available in the github releases.

```
mkdir build
cd build
cmake ..
make
./shview
```

## Controls

- 1-9: Select a coefficient
- up/down arrow: Modify selected coefficient
- 0: Zero selected coefficient
- p: Print parameters to console
- w: Toggle wireframe
- t: Toggle texturing
- numpad plus: increase model resolution
- - alternate: equals
- shift, numpad plus: really increase model resolution
- - alternate: shift, equals
- numpad minus: decrease model resolution
- - alternate: minus
- shift, numpad minus: really decrease model resolution
- - alternate: shift, minus
- numpad 8/4/2/6: rotate camera
- - alternate: j/m/l/i
- numpad 5: stop rotating camera
- - alternate: k
- e: change the parameters to rotate the harmonic
- r: do (e) but also rotate the camera with the model

Parameter rotations are done "the dumb way". The model is integrated against a
rotated version of itself to produce new coefficients.  This integration
process is imperfect and causes some degredation when applied many
times, especially for low-resolution models.  The model is rotated by
exactly one spine per frame, so it can be sped up or slowed down by
adjusting the resolution of the model.

