project(big_file_transfer)

add_executable(FileServer src/StartServer.cc)
add_executable(FileClient src/StartClient.cc)
