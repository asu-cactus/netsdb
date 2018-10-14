
#ifndef SELECTION_CC
#define SELECTION_CC

#include "Selection.h"
#include "SelectionQueryProcessor.h"
#include "FilterQueryProcessor.h"
#include "ProjectionQueryProcessor.h"
#include "FilterBlockQueryProcessor.h"
#include "ProjectionBlockQueryProcessor.h"
#include "SimpleSingleTableQueryProcessor.h"
#include "BlockQueryProcessor.h"
#include <memory>

namespace pdb {

template <typename Out, typename In>
SimpleSingleTableQueryProcessorPtr Selection<Out, In>::getProcessor() {
    return std::make_shared<SelectionQueryProcessor<Out, In>>(*this);
}

template <typename Out, typename In>
SimpleSingleTableQueryProcessorPtr Selection<Out, In>::getFilterProcessor() {
    return std::make_shared<FilterQueryProcessor<Out, In>>(*this);
}


template <typename Out, typename In>
SimpleSingleTableQueryProcessorPtr Selection<Out, In>::getProjectionProcessor() {
    return std::make_shared<ProjectionQueryProcessor<Out, In>>(*this);
}

template <typename Out, typename In>
BlockQueryProcessorPtr Selection<Out, In>::getFilterBlockProcessor() {
    return std::make_shared<FilterBlockQueryProcessor<Out, In>>(*this);
}


template <typename Out, typename In>
BlockQueryProcessorPtr Selection<Out, In>::getProjectionBlockProcessor() {
    return std::make_shared<ProjectionBlockQueryProcessor<Out, In>>(*this);
}


template <typename In, typename Out>
void Selection<In, Out>::match(function<void(QueryBase&)> forSelection,
                               function<void(QueryBase&)> forSet,
                               function<void(QueryBase&)> forQueryOutput) {
    forSelection(*this);
};
}

#endif
