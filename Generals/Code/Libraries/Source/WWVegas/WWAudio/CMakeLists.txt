# WWAudio is the Miles Sound System-era Westwood audio engine. It has been superseded
# by the XAudio2-based GameAudio implementation and nothing in GameEngine,
# GameEngineDevice, WW3D2 or Main references it any more. Kept as an INTERFACE target
# so the existing link references still resolve.
add_library(WWAudio INTERFACE)
