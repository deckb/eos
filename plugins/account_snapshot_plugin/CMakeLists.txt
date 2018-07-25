file(GLOB HEADERS "include/eosio/account_snapshot_plugin/*.hpp")
add_library( account_snapshot_plugin
             account_snapshot_plugin.cpp
             ${HEADERS} )

target_link_libraries( account_snapshot_plugin chain_plugin appbase )
target_include_directories( account_snapshot_plugin PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
