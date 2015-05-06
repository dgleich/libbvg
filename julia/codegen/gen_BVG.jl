using Clang.wrap_c

context = wrap_c.init(; output_file="BVG.jl", header_library=x->"libbvg", common_file="BVG_h.jl", clang_diagnostics=true)
context.options.wrap_structs = true
wrap_c.wrap_c_headers(context, ["include/bitfile.h","include/bvgraph.h"])

