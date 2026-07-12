
set(TORTOISE_RULES_HEADERS
	inc/perft.h
	inc/bitboard.h
	inc/fen.h
	inc/move.h
	inc/perft.h
	inc/types.h
	inc/zobrist.h
	inc/debug.h
	inc/position.h
	inc/piece_attack.h
	inc/rules.h
)

set(TORTOISE_RULES_SOURCES
	src/fen.cpp
	src/perft.cpp
	src/piece_attack.cpp
	src/position.cpp
	src/zobrist.cpp
	src/debug.cpp
	src/rules.cpp
)


add_library(rules STATIC
	${TORTOISE_RULES_HEADERS}
	${TORTOISE_RULES_SOURCES}
	src/thread_pool_executor.h
)

target_include_directories(rules PRIVATE ${CMAKE_SOURCE_DIR}/rules/inc)