configure_file(testdir.h.in ${CMAKE_CURRENT_BINARY_DIR}/testdir.h @ONLY)
add_subdirectory(addon)
add_subdirectory(inputmethod)
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(testhangul testhangul.cpp)
target_link_libraries(testhangul Fcitx5::Core Fcitx5::Module::TestFrontend)
add_dependencies(testhangul copy-addon copy-im)
add_test(NAME testhangul COMMAND testhangul)

# dubeolsik-sunarae project: end-to-end check of the Sun-arae keyboard
# option added to this addon.
add_executable(testhangulsunarae testhangulsunarae.cpp)
target_link_libraries(testhangulsunarae Fcitx5::Core Fcitx5::Module::TestFrontend)
add_dependencies(testhangulsunarae copy-addon copy-im)
add_test(NAME testhangulsunarae COMMAND testhangulsunarae)
