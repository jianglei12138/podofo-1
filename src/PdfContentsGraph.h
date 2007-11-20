#ifndef _PODOFO_PDFCONTENTSGRAPH_H
#define _PODOFO_PDFCONTENTSGRAPH_H

#include "PdfDefines.h"

#if defined(HAVE_BOOST)

#include "PdfVariant.h"

#include <utility>
#include <string>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/variant.hpp>

namespace PoDoFo {

class PdfInputStream;
class PdfOutputStream;
class PdfContentsTokenizer;

enum PdfContentStreamKeyword
{
    // Special node for undefined/unset value. Should never get used except when default-constructing
    // a node variant.
    KW_Undefined = 0,
    // Normal PDF operators
    KW_q,
    KW_Q,
    KW_ST,
    KW_ET,
    KW_BMC,
    KW_BDC,
    KW_EMC,
    KW_m,
    KW_l,
    // Special node with no associated keyword, used to identify the root node that anchors
    // the graph.
    KW_RootNode = 0xfe,
    // Value returned by findKwByName(...) when no enum for the keyword is known.
    KW_Unknown = 0xff
};


/**
 * PdfContentsGraph provides a concrete representation of a content stream as an
 * in-memory graph. It can be created as a blank structure to be populated by hand,
 * or from an existing content stream via PdfContentsTokenizer . It can serialize
 * its state as a PDF content stream.
 *
 * This class does not track the resources used by the content stream. \see PdfCanvas .
 *
 * This class is only available when the Boost library, specifically the boost graph
 * library, has been configured for use.
 */
class PdfContentsGraph
{
public:
    /**
     * The KWType enumeration is used to identify whether a given keyword
     * should be expected to open or close a new scope (think q/Q pairs) or
     * whether it's just a plain unscoped operator.
     */
    enum KWType
    {
        KT_Undefined = 0, /**< Only used for sentinel */
        KT_Standalone,    /**< Keyword doesn't open or close a scope. */
        KT_Opening,       /**< Keyword opens a new scope. */
        KT_Closing        /**< Keyword closes an open scope. */
    };

    /**
     * KWInfo describes a single PDF keyword's characteristics. See kwInfo[] .
     */
    struct KWInfo {
        /// Keyword type ( ends scope, begins scope, or scope neutral )
        KWType kt;
        /// Keyword ID (enum)
        PdfContentStreamKeyword kw;
        /// ID enum of context closing keyword (only to be set if this
        /// is a context opening keyword), eg KW_Q if kw = KW_q .
        PdfContentStreamKeyword kwClose;
        /// null-terminated keyword text
        const char kwText[6];
        /// Short description text (optional, set to NULL if undesired).
        const char * kwDesc;
    };

    typedef std::pair<PdfContentStreamKeyword,PdfContentStreamKeyword> KWPair;
    // A variant value on a node. May contain one of:
    //    - A pair of keyword IDs representing an opening/closing pair of operators.
    //      The first is the opening, the second the closing.
    //    - A single keyword ID for a standlone operator / one with arguments.
    //    - A string value for a keyword that's not recognised by PoDoFo
    //    - A variant type (an argument)
    typedef boost::variant<PdfContentStreamKeyword,KWPair,std::string,PoDoFo::PdfVariant> NodeData;
    typedef boost::adjacency_list<boost::listS,boost::vecS,boost::directedS,NodeData> Graph;
    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;

    /**
     * Construct a new, blank PdfContentsGraph
     */
    PdfContentsGraph();

    /**
     * Construct a PdfContentsGraph from a PdfContentsTokenizer's output.
     */
    PdfContentsGraph(PdfContentsTokenizer & contentsTokenizer);

    /**
     * Destroy the PdfContentsGraph.
     */
    ~PdfContentsGraph() { }

    /**
     * Serialize the PdfContentsGraph to the passed output stream.
     * The output format is valid PDF content stream data.
     */
    void Write(PdfOutputStream& outStream);

    /**
     * For quick and easy debugging, serialize the graph to stderr as
     * a PDF content stream.
     */
    void WriteToStdErr();

    // Look up a keyword string and return a reference to the associated
    // keyword info struct If the keyword string is not known, return
    // kwInfoUnknown .
    static const KWInfo& findKwByName(const std::string & kwText);

    // Look up an operator code and return the associated keyword string. All
    // defined enums MUST exist in kwIdMap .
    static const KWInfo& findKwById(PdfContentStreamKeyword kw);

private:
    // private member variables
    Graph m_graph;
};

} // namespace PoDoFo

#endif // defined(HAVE_BOOST)

#endif