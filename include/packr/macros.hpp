#include <cassert>

#ifdef __cpp_contracts
#include <contracts>
#endif

// As clang didn't implement contracts yet
#ifdef __cpp_contracts

#define pcontract_assert(pred) contract_assert(pred)

#define ppre(pred) pre(pred)

#define ppost(pred) post(pred)

#else

#define pcontract_assert(pred) assert(pred)

#define ppre(pred)  /*pre*/
#define ppost(pred) /*post*/

#endif
