add_library(GameSpy STATIC
    Stub/PeerStub.cpp
    Stub/GPStub.cpp
    Stub/PersistStub.cpp
    Stub/HTTPStub.cpp
    Stub/ChatQR2Stub.cpp
)

target_include_directories(GameSpy PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Stub
)
