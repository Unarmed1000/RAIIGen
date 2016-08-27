#ifndef MB_CLANGINFODUMP_HPP
#define MB_CLANGINFODUMP_HPP
#include <clang-c/Index.h>

namespace MB
{
  namespace ClangInfoDump
  {
    extern CXChildVisitResult Visitor(CXCursor cursor, CXCursor /* parent */, CXClientData clientData);
  }
}
#endif
