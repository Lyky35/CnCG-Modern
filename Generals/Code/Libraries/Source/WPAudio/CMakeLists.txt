# WPAudio (Miles-era Westwood audio) is dead code in this rewrite: its headers were
# never present in the public EA dump and its only consumer, GameSpeech.cpp, is
# superseded by the XAudio2 engine and excluded from the GameEngine build.
# Kept as an INTERFACE target so existing link references still resolve.
add_library(WPAudio INTERFACE)
