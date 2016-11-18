# create symlinks in binary dir
foreach(mesh
    feti_beam_fine
    basic
    cube
    decomposed
    fourPointBending
    2d_miehe_symmetric_three_point_bending
    feti_linear_benchmark_4_subdomains
    feti_beam_fine_tri
    )
    execute_process(COMMAND "${CMAKE_COMMAND}" "-E" "create_symlink" "${CMAKE_SOURCE_DIR}/mesh_files/${mesh}.msh" "${CMAKE_CURRENT_BINARY_DIR}/${mesh}.msh")
endforeach()

# add executables and link library
foreach(example
    first_example
    BasicMesh
    DecomposedMesh
    fourPointBending
    threePointBending
    )
    add_executable(${example} ${example}.cpp)
    target_link_libraries(${example} LINK_PUBLIC GmshReader)
endforeach()


