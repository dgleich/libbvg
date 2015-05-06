# Julia wrapper for header: include/bitfile.h
# Automatically generated using Clang.jl wrap_c, version 0.0.0
# edited to only have the raw calls

include("BVG_h.jl")

type BVGraph
    g::bvgraph
    
    function BVGraph(gpath::AbstractString)
        
        finalizer(b, bvgraph_close) 
    end
end

