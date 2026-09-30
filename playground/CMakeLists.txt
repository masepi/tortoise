

add_executable(playground
	src/main.cpp
	src/divide.cpp
	src/divide.h
	src/epd.cpp
	src/epd.h
)

target_include_directories(playground
	PRIVATE ${CMAKE_SOURCE_DIR}/rules/inc
)

target_link_libraries(playground
	PRIVATE rules
)

