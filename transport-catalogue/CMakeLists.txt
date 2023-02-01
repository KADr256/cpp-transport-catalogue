cmake_minimum_required (VERSION 3.11)

project ("Transport_Catalogue")
set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_SYSTEM_PREFIX_PATH "/path/install")

find_package(Protobuf REQUIRED)
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto svg.proto map_renderer.proto graph.proto transport_router.proto)

set(TRANSPORT_CATALOGUE_FILES geo.h geo.cpp json.h json.cpp json_builder.h json_builder.cpp ranges.h graph.h router.h svg.h svg.cpp domain.h domain.cpp map_renderer.h map_renderer.cpp request_handler.h request_handler.cpp transport_catalogue.h transport_catalogue.cpp transport_router.h transport_router.cpp serialization.h serialization.cpp json_reader.h json_reader.cpp main.cpp svg.proto map_renderer.proto transport_catalogue.proto)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${TRANSPORT_CATALOGUE_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue ${Protobuf_LIBRARY} Threads::Threads)
