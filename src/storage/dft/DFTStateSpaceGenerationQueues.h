#ifndef DFTSTATESPACEGENERATIONQUEUES_H
#define	DFTSTATESPACEGENERATIONQUEUES_H

#include <list>
#include <queue>
#include <vector>
#include <deque>

#include "OrderDFTElementsById.h"

namespace storm {
    namespace storage {

        template<typename ValueType>
        class DFTGate;
        template<typename ValueType>
        class DFTElement;


        template<typename ValueType>
        class DFTStateSpaceGenerationQueues {

            using DFTElementPointer = std::shared_ptr<DFTElement<ValueType>>;
            using DFTElementVector = std::vector<DFTElementPointer>;
            using DFTGatePointer = std::shared_ptr<DFTGate<ValueType>>;
            using DFTGateVector = std::vector<DFTGatePointer>;

            std::priority_queue<DFTGatePointer, DFTGateVector, OrderElementsByRank<ValueType>> failurePropagation;
            DFTGateVector failsafePropagation;
            DFTElementVector dontcarePropagation;
            DFTElementVector activatePropagation;

        public:
            void propagateFailure(DFTGatePointer const& elem) {
                failurePropagation.push(elem);
            }

            bool failurePropagationDone() const {
                return failurePropagation.empty();
            }

            DFTGatePointer nextFailurePropagation() {
                DFTGatePointer next = failurePropagation.top();
                failurePropagation.pop();
                return next;
            }
            
            bool failsafePropagationDone() const {
                return failsafePropagation.empty();
            }
            
            void propagateFailsafe(DFTGatePointer const& gate) {
                failsafePropagation.push_back(gate);
            }

            DFTGatePointer nextFailsafePropagation() {
                DFTGatePointer next = failsafePropagation.back();
                failsafePropagation.pop_back();
                return next;
            }
            
            bool dontCarePropagationDone() const {
                return dontcarePropagation.empty();
            }
            
            void propagateDontCare(DFTElementPointer const& elem) {
                dontcarePropagation.push_back(elem);
            }
            
            void propagateDontCare(DFTElementVector const& elems) {
                dontcarePropagation.insert(dontcarePropagation.end(), elems.begin(), elems.end());
            }
            
            DFTElementPointer nextDontCarePropagation() {
                DFTElementPointer next = dontcarePropagation.back();
                dontcarePropagation.pop_back();
                return next;
            }
        };

    }
}

#endif	/* DFTSTATESPACEGENERATIONQUEUES_H */

