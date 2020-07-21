
S.verticies      =%8u
S.normals        =%8u
S.triangles      =%8u, %u unused, %u fixed, %u invalid
S.octree_size    =%8u
S.total_size     =%8u

H.unknown_0x10   = %11.3f
H.unknown_0x38   = %11.3f

H.min_octree     = %11.3f %11.3f %11.3f  # minimum coord for octree entrance
S.min_point      = %11.3f %11.3f %11.3f  # minimum coord of all points
S.mean_point     = %11.3f %11.3f %11.3f  # arithmetic mean of all points
S.max_point      = %11.3f %11.3f %11.3f  # maximum coord of all points
S.max_octree     = %11.3f %11.3f %11.3f  # maximum coord for octree entrance

H.mask           =  0x%08x  0x%08x  0x%08x  # masks (header)
H.shifts         =    %8u>   %8u<   %8u< # header shift values
S.null_bits      =    %8u    %8u    %8u  # number of low 0 bits
S.bcube_bits     =    %8u    %8u    %8u  # number of base cube bits
S.n_bcubes       =    %8u    %8u    %8u  # number of base cubes

S.min_distance   = %14.6f # minimum distance between points of a triangle
S.max_distance   = %11.3f    # maximum distance between points of a triangle

S.bcube_width    = %8u       # width of a single base cube = 2^rshift
S.max_cube_width = %8u       # maximum width of a cube with triangles
S.min_cube_width = %8u       # minimum width of a cube with triangles
~S.blow_size      = %8u       # suggestion: what KCL_BLOW value was used
S.n_base_cubes   = %8u       # number of base cubes
S.n_cube_nodes   = %8u       # number of cube nodes (each with 8 cube links)
S.total_cubes    = %8u       # total number of cubes
S.max_depth      = %8u       # maximum octree depth
S.ave_depth      = %11.2f    # average octree depth

S.n_null_lists   = %8u       # number of triangle lists with 0 triangles
S.n_tri_lists    = %8u       # number of triangle lists with >0 triangles
S.n_null_links   = %8u       # number of triangle links to lists with 0 triangles
S.n_tri_links    = %8u       # number of triangle links to lists with >0 triangles

