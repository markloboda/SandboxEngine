set(SOURCE_FILES
  stb_image.cpp
  stb_image_write.cpp
)

add_library(stb STATIC ${SOURCE_FILES})

target_include_directories(stb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(stb PRIVATE STB_IMAGE_IMPLEMENTATION)