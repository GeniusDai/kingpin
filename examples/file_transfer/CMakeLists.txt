project(big_file_transfer)

add_executable(FileServer StartServer.cc)
add_executable(FileClient StartClient.cc)
