# Copyright © 2020 Advanced Micro Devices, Inc. All rights reserved 


set(JSON_BuildTests OFF CACHE INTERNAL "")


enable_testing()

set(test_list "")

function(add_embers_test TEST_NAME)
  add_executable(${TEST_NAME} ${TEST_NAME}.cc)

  target_compile_options(${TEST_NAME}
    PRIVATE
    -march=x86-64
    -Wall
    -Wfatal-errors)
  target_include_directories(${TEST_NAME}
    PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>/include
  )
  target_link_libraries(${TEST_NAME} PRIVATE hip)
  add_test(${TEST_NAME} ${TEST_NAME})

  message(DEBUG "Appending ${TEST_NAME} to test_list")
  list(APPEND test_list ${TEST_NAME})
  set(test_list ${test_list} PARENT_SCOPE)

endfunction()



add_embers_test(test_status)
add_embers_test(unique_ptr_test)
add_embers_test(atomic_test)
add_embers_test(rand_test)
add_embers_test(arch_test)
add_embers_test(hwid_test)
add_embers_test(chipid_test)
target_link_libraries(chipid_test PRIVATE pciaccess)
add_embers_test(memcpy_test)
add_embers_test(memset_test)
add_embers_test(memcmp_test)
add_embers_test(spinlock_test)
add_embers_test(ticketlock_test)
add_embers_test(rwlock_test)
add_embers_test(counter_test)
add_embers_test(barrier_test) 
add_embers_test(grid_barrier_test)
add_embers_test(locking_queue_test)
add_embers_test(nonlocking_queue_test)
add_embers_test(nonlocking_queue_pathological_test)
add_embers_test(cubehash256)
add_embers_test(keccak256)
add_embers_test(ethash)
add_embers_test(almost_equal_test)
add_embers_test(xorshift_test)
add_embers_test(fill_buffer_rand_test)
find_package(Threads REQUIRED)
target_link_libraries(fill_buffer_rand_test PRIVATE Threads::Threads)
add_embers_test(realtime_test)

install(TARGETS ${test_list}
  DESTINATION lib/${EMBERS_TEST_PKG_NAME}/tests COMPONENT tests
)
