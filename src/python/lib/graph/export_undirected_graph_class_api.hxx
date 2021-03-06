#pragma once
#ifndef NIFTY_PYTHON_GRAPH_EXPORT_UNDIRECTED_GRAPH_CLASS_API_HXX
#define NIFTY_PYTHON_GRAPH_EXPORT_UNDIRECTED_GRAPH_CLASS_API_HXX

#include <pybind11/pybind11.h>
#include <pybind11/cast.h>

#include "nifty/python/converter.hxx"

namespace py = pybind11;







namespace pybind11 {
namespace detail {

template <typename T1, typename T2> class type_caster<nifty::graph::detail_graph::UndirectedAdjacency<T1, T2>> {
    typedef nifty::graph::detail_graph::UndirectedAdjacency<T1, T2> type;
public:
    bool load(handle src, bool convert) {
        if (!isinstance<sequence>(src))
            return false;
        const auto seq = reinterpret_borrow<sequence>(src);
        if (seq.size() != 2)
            return false;
        return first.load(seq[0], convert) && 
               second.load(seq[1], convert);
    }

    static handle cast(const type &src, return_value_policy policy, handle parent) {
        auto o1 = reinterpret_steal<object>(make_caster<T1>::cast(src.node(), policy, parent));
        auto o2 = reinterpret_steal<object>(make_caster<T2>::cast(src.edge(), policy, parent));
        if (!o1 || !o2)
            return handle();
        tuple result(2);
        PyTuple_SET_ITEM(result.ptr(), 0, o1.release().ptr());
        PyTuple_SET_ITEM(result.ptr(), 1, o2.release().ptr());
        return result.release();
    }

    static PYBIND11_DESCR name() {
        return type_descr(
            _("Adjacency[") + make_caster<T1>::name() + _(", ") + make_caster<T2>::name() + _("]")
        );
    }

    template <typename T> using cast_op_type = type;

    operator type() {
        return type(cast_op<T1>(first), cast_op<T2>(second));
    }
protected:
    make_caster<T1> first;
    make_caster<T2> second;
};

}
}










namespace nifty{
namespace graph{


    template<class G, class ITER, class TAG>
    class PyGraphIter{
    public:
        typedef ITER Iter;
        typedef typename std::iterator_traits<ITER>::value_type ReturnType;
        PyGraphIter( 
            const G & g, 
            py::object gRef,
            const Iter beginIter,
            const Iter endIter
        )   :
            g_(g),
            current_(beginIter),
            end_(endIter),
            gRef_(gRef)
        {
        }

        ReturnType next(){
            if(current_ == end_){
                throw py::stop_iteration();
            }
            else{
            }
            const auto ret = *current_;
            ++current_;
            return ret;
        }


    private:
        const G & g_;
        py::object gRef_;
        Iter current_,end_;

    };

    template<class G, class MAP_TYPE>
    void exportEdgeMap(
        py::module & graphModule,
        const std::string & clsName
    ){

        py::class_<MAP_TYPE>(graphModule, clsName.c_str())
        ;

    }

    template<class G, class MAP_TYPE>
    void exportNodeMap(
        py::module & graphModule,
        const std::string & clsName
    ){

        py::class_<MAP_TYPE>(graphModule, clsName.c_str())
        ;

    }

    template<class G, class CLS_T>
    void exportUndirectedGraphClassAPI(
        py::module & graphModule,
        CLS_T & cls,
        const std::string & clsName
    ){
        
        typedef typename G::EdgeIter EdgeIter;
        typedef PyGraphIter<G,EdgeIter, EdgeTag> PyEdgeIter;
        auto edgeIterClsName = clsName + std::string("EdgeIter");
        py::class_<PyEdgeIter>(graphModule, edgeIterClsName.c_str())
            .def("__iter__", [](PyEdgeIter &it) -> PyEdgeIter& { return it; })
            .def("__next__", &PyEdgeIter::next);
        ;

        typedef typename G::NodeIter NodeIter;
        typedef PyGraphIter<G,NodeIter,NodeTag> PyNodeIter;
        auto nodeIterClsName = clsName + std::string("NodeIter");
        py::class_<PyNodeIter>(graphModule, nodeIterClsName.c_str())
            .def("__iter__", [](PyNodeIter &it) -> PyNodeIter& { return it; })
            .def("__next__", &PyNodeIter::next);
        ;
        
        typedef typename G::AdjacencyIter AdjacencyIter;
        typedef PyGraphIter<G,AdjacencyIter,AdjacencyTag> PyAdjacencyIter;
        auto adjacencyIterClsName = clsName + std::string("AdjacencyIter");
        py::class_<PyAdjacencyIter>(graphModule, adjacencyIterClsName.c_str())
            .def("__iter__", [](PyAdjacencyIter &it) -> PyAdjacencyIter& { return it; })
            .def("__next__", &PyAdjacencyIter::next);
        ;

        typedef typename G:: template EdgeMap<double> EdgeMapFloat64;
        exportEdgeMap<G, EdgeMapFloat64>(graphModule, clsName + std::string("EdgeMapFloat64"));

        typedef typename G:: template NodeMap<double> NodeMapFloat64;
        exportEdgeMap<G, NodeMapFloat64>(graphModule, clsName + std::string("NodeMapFloat64"));

        cls
            .def_property_readonly("numberOfNodes",&G::numberOfNodes)
            .def_property_readonly("numberOfEdges",&G::numberOfEdges)
            .def_property_readonly("nodeIdUpperBound",&G::nodeIdUpperBound)
            .def_property_readonly("edgeIdUpperBound",&G::edgeIdUpperBound)

            .def("findEdge",[](const G & self, std::pair<uint64_t, uint64_t> uv){
                return self.findEdge(uv.first, uv.second);
            })
            .def("findEdges",[](
                const G & self,
                nifty::marray::PyView<uint64_t, 2> uv
            ){
                nifty::marray::PyView<int64_t> edgeIds({uv.shape(0)});
                NIFTY_CHECK_OP(uv.shape(1),==,2,"uv.shape(1) must be 2");

                for(auto i=0; i<uv.shape(0); ++i){
                   edgeIds(i) = self.findEdge(uv(i,0), uv(i,1)); 
                }
                return edgeIds;
            })
            .def("findEdge",&G::findEdge)
            .def("u",&G::u)
            .def("v",&G::v)
            .def("uv",&G::uv)
            .def("edges", [](py::object g) { 
                const auto & gg = g.cast<const G &>();
                return PyEdgeIter(gg,g,gg.edgesBegin(),gg.edgesEnd()); 
            })
            .def("nodes", [](py::object g) { 
                const auto & gg = g.cast<const G &>();
                return PyNodeIter(gg,g,gg.nodesBegin(),gg.nodesEnd()); 
            })
            .def("nodeAdjacency", [](py::object g, const uint64_t nodeId) { 
                const auto & gg = g.cast<const G &>();
                return PyAdjacencyIter(gg,g,gg.adjacencyBegin(nodeId),gg.adjacencyEnd(nodeId)); 
            })

            .def("__str__",
                [](const G & g) {
                    std::stringstream ss;
                    ss<<"#Nodes "<<g.numberOfNodes()<<" #Edges "<<g.numberOfEdges();
                    return ss.str();
                }
            )
            .def("__repr__",
                [](const G & g) {
                    std::stringstream ss;
                    auto first = true;
                    for(auto edge : g.edges()){
                        if(first){
                            first = false;
                            ss<<g.u(edge)<<"-"<<g.v(edge);
                        }
                        else
                            ss<<"\n"<<g.u(edge)<<"-"<<g.v(edge);
                    }
                    return ss.str();
                }
            )

        ;
    }
    


} // namespace nifty::graph
} // namespace nifty

#endif  // NIFTY_PYTHON_GRAPH_EXPORT_UNDIRECTED_GRAPH_CLASS_API_HXX
