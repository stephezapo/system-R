# -----------------------------------------------------------------------------
# Unit tests (Catch2 v3 + CTest).
#
# This is a juce_add_console_app (not a plain add_executable) so that the JUCE
# module unity sources pulled in transitively by systemr::core are compiled into
# the test binary. Catch2WithMain supplies main(); JUCE console apps do not.
# -----------------------------------------------------------------------------
include(FetchContent)

FetchContent_Declare(Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG        v3.9.1
    GIT_SHALLOW    TRUE)
FetchContent_MakeAvailable(Catch2)

juce_add_console_app(systemr_tests
    PRODUCT_NAME "systemr_tests")

target_sources(systemr_tests PRIVATE
    VersionTests.cpp
    TripleBufferTests.cpp
    E131PacketTests.cpp
    ArtNetPacketTests.cpp
    OutputManagerTests.cpp
    InputMergerTests.cpp
    ReceiverTests.cpp
    EngineTests.cpp)

target_link_libraries(systemr_tests PRIVATE
    systemr::core
    Catch2::Catch2WithMain
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags)

# Register every TEST_CASE with CTest.
list(APPEND CMAKE_MODULE_PATH "${catch2_SOURCE_DIR}/extras")
include(Catch)
catch_discover_tests(systemr_tests)
