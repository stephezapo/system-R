# -----------------------------------------------------------------------------
# systemr_core — headless engine + domain model (no GUI).
#
# core is a plain static library that *interface-consumes* JUCE modules: it gets
# their include dirs and module-available defines, but JUCE only compiles a
# module's unity source into targets created by the juce_add_* helpers. So the
# module unity sources are built exactly once, inside each final executable
# (the app via juce_add_gui_app, the tests via juce_add_console_app) — never
# duplicated. Keep core a plain add_library for this to hold.
# -----------------------------------------------------------------------------
add_library(systemr_core STATIC
    src/Version.cpp
    src/ShowDocument.cpp
    src/Engine.cpp
    src/OutputManager.cpp
    src/net/InputMerger.cpp
    src/artnet/ArtNetPacket.cpp
    src/artnet/ArtNetSender.cpp
    src/artnet/ArtNetReceiver.cpp
    src/sacn/E131Packet.cpp
    src/sacn/SacnSender.cpp
    src/sacn/SacnReceiver.cpp)

add_library(systemr::core ALIAS systemr_core)

target_include_directories(systemr_core PUBLIC include)
target_compile_features(systemr_core PUBLIC cxx_std_20)

target_link_libraries(systemr_core PUBLIC
    juce::juce_core
    juce::juce_events
    juce::juce_data_structures)
