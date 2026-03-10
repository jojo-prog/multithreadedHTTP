cmake_minimum_required(VERSION 3.16)

project(HTTP VERSION 1.0.0 LANGUAGES C)

include(GNUInstallDirs)

option(HTTP_ENABLE_WARNINGS "Enable compiler warnings" ON)
option(HTTP_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)
option(HTTP_ENABLE_TESTING "Enable test infrastructure" OFF)
option(HTTP_ENABLE_INSTALL "Enable install rules" OFF)
option(HTTP_ENABLE_DEBUG_SYMBOLS "Enable debug symbols and disable optimization in Debug builds" ON)
option(HTTP_ENABLE_ASAN "Enable AddressSanitizer in Debug builds (GCC/Clang)" OFF)
option(HTTP_ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer in Debug builds (GCC/Clang)" OFF)

set(HTTPSERVER_TARGET_NAME "Server" CACHE STRING "Executable target name")
set(HTTPCLIENT_TARGET_NAME "Client" CACHE STRING "Executable target name")

set(HTTP_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src" CACHE PATH "Source directory")
set(HTTP_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include" CACHE PATH "Public include directory")

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR}")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_BINDIR}")

find_package(Curses REQUIRED)

set(HTTPSERVER_SOURCES
	"src/server.c"
	"src/http.c"
)

set(HTTPCLIENT_SOURCES
	"src/client.c"
	"src/http.c"
)
add_executable(http 
	src/http.c
	src/main.c
)

add_executable(${HTTPSERVER_TARGET_NAME}
	${HTTPSERVER_SOURCES}
)

add_executable(${HTTPCLIENT_TARGET_NAME}
	${HTTPCLIENT_SOURCES}
)

target_include_directories(http
	PRIVATE
		"${HTTP_INCLUDE_DIR}"
)

target_include_directories(${HTTPSERVER_TARGET_NAME}
	PRIVATE
		"${HTTP_INCLUDE_DIR}"
)

target_include_directories(${HTTPCLIENT_TARGET_NAME}
	PRIVATE
		"${HTTP_INCLUDE_DIR}"
)

target_link_libraries(${HTTPSERVER_TARGET_NAME} PRIVATE ${CURSES_LIBRARIES})
target_link_libraries(${HTTPCLIENT_TARGET_NAME} PRIVATE ${CURSES_LIBRARIES})
target_include_directories(${HTTPSERVER_TARGET_NAME} PRIVATE ${CURSES_INCLUDE_DIRS})
target_include_directories(${HTTPCLIENT_TARGET_NAME} PRIVATE ${CURSES_INCLUDE_DIRS})

if(HTTP_ENABLE_WARNINGS)
	if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
		target_compile_options(${HTTPSERVER_TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
		target_compile_options(${HTTPCLIENT_TARGET_NAME} PRIVATE -Wall -Wextra -Wpedantic)
	elseif(MSVC)
		target_compile_options(${HTTP_TARGET_NAME} PRIVATE /W4)
	endif()
endif()

if(HTTP_WARNINGS_AS_ERRORS)
	if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
		target_compile_options(${HTTPSERVER_TARGET_NAME} PRIVATE -Werror)
		target_compile_options(${HTTPCLIENT_TARGET_NAME} PRIVATE -Werror)
	endif()
endif()

if(HTTP_ENABLE_DEBUG_SYMBOLS)
	if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
		target_compile_options(${HTTPSERVER_TARGET_NAME} PRIVATE
			$<$<CONFIG:Debug>:-g3 -O0>
		)
		target_compile_options(${HTTPCLIENT_TARGET_NAME} PRIVATE
			$<$<CONFIG:Debug>:-g3 -O0>
		)
	endif()
endif()

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
	if(HTTP_ENABLE_ASAN)
		target_compile_options(${HTTPSERVER_TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer>)
		target_link_options(${HTTPSERVER_TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
		target_compile_options(${HTTPCLIENT_TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address -fno-omit-frame-pointer>)
		target_link_options(${HTTPCLIENT_TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=address>)
	endif()

	if(HTTP_ENABLE_UBSAN)
		target_compile_options(${HTTPCLIENT_TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=undefined -fno-omit-frame-pointer>)
		target_link_options(${HTTPCLIENT_TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=undefined>)
		target_compile_options(${HTTPSERVER_TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=undefined -fno-omit-frame-pointer>)
		target_link_options(${HTTPSERVER_TARGET_NAME} PRIVATE $<$<CONFIG:Debug>:-fsanitize=undefined>)
	endif()
endif()

if(HTTP_ENABLE_TESTING)
	enable_testing()
endif()
