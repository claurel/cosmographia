# CMake file for building the lib3ds library

set (SRCS 
    lib3ds_atmosphere.c
    lib3ds_background.c
    lib3ds_camera.c
    lib3ds_chunk.c
    lib3ds_chunktable.c
    lib3ds_file.c
    lib3ds_io.c
    lib3ds_light.c
    lib3ds_material.c
    lib3ds_math.c
    lib3ds_matrix.c
    lib3ds_mesh.c
    lib3ds_node.c
    lib3ds_quat.c
    lib3ds_shadow.c
    lib3ds_track.c
    lib3ds_util.c
    lib3ds_vector.c
    lib3ds_viewport.c
)

add_library (lib3ds STATIC ${SRCS})
