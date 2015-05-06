include("julia/codegen/BVG.jl")

println(bvgraph_error_string(Int32(-1)))

bf = bitfile()
rval = bitfile_open_filename("README.md", bf)
println(rval)


println(bitfile_read_bit(bf))
println(bitfile_read_bit(bf))
println(bitfile_read_bit(bf))
println(bitfile_read_bit(bf))


bitfile_close(bf)