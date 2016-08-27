
#include <RAIIGen/ClangInfoDump.hpp>
#include <RAIIGen/ClangUtil.hpp>

#include <string>
#include <iostream>

namespace MB
{
  using namespace ClangUtil;

  namespace ClangInfoDump
  {



    //VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(
    //  const VkInstanceCreateInfo*                 pCreateInfo,
    //  const VkAllocationCallbacks*                pAllocator,
    //  VkInstance*                                 pInstance);
    //
    //FunctionDecl(vkCreateInstance)
    //  - TypeRef(VkResult)
    //  - ParmDecl(pCreateInfo)
    //  --TypeRef(VkInstanceCreateInfo)
    //  - ParmDecl(pAllocator)
    //  --TypeRef(VkAllocationCallbacks)
    //  - ParmDecl(pInstance)
    //  --TypeRef(VkInstance)
    //
    void ExamineFunc(const CXCursor& cursor, const CXCursorKind cursorKind, const int currentLevel)
    {
      const std::string cursorKindName = GetCursorKindName(cursorKind);
      const std::string cursorSpelling = GetCursorSpelling(cursor);

      if (cursorSpelling.find("vkCreate") == 0)
      {
        std::cout << std::string(currentLevel, '-') << " " << cursorKindName << " (" << cursorSpelling << ")\n";

      }
      else if (cursorSpelling.find("vkDestroy") == 0)
        std::cout << std::string(currentLevel, '-') << " " << cursorKindName << " (" << cursorSpelling << ")\n";
      else if (cursorSpelling.find("vkAllocate") == 0)
        std::cout << std::string(currentLevel, '-') << " " << cursorKindName << " (" << cursorSpelling << ")\n";
      else if (cursorSpelling.find("vkFree") == 0)
        std::cout << std::string(currentLevel, '-') << " " << cursorKindName << " (" << cursorSpelling << ")\n";
    }


    CXChildVisitResult Visitor(CXCursor cursor, CXCursor /* parent */, CXClientData clientData)
    {
      CXSourceLocation location = clang_getCursorLocation(cursor);
      //if (clang_Location_isFromMainFile(location) == 0)
      //  return CXChildVisit_Continue;

      CXCursorKind cursorKind = clang_getCursorKind(cursor);

      unsigned int curLevel = *(reinterpret_cast<unsigned int*>(clientData));
      unsigned int nextLevel = curLevel + 1;

      bool useFallback = true;
      //clang_getCursorType(cursor);
      switch (cursorKind)
      {
      //case CXCursor_FunctionDecl:
      //  ExamineFunc(cursor, cursorKind, curLevel);
      //  break;
      case CXCursor_FieldDecl:
      {
        const CXType cursorType = clang_getCursorType(cursor);
        const auto typeSpelling = GetTypeSpelling(cursorType);
        const auto cursorSpelling = GetCursorSpelling(cursor);
        std::cout << std::string(curLevel, '-') << " " << GetCursorKindName(cursorKind) << " ('" << typeSpelling << "' '" << cursorSpelling << "')\n";
        useFallback = false;
        break;
      }
      default:
        break;
      }
      if( useFallback )
        std::cout << std::string(curLevel, '-') << " " << GetCursorKindName(cursorKind) << " (" << GetCursorSpelling(cursor) << ")\n";

      clang_visitChildren(cursor, Visitor, &nextLevel);
      return CXChildVisit_Continue;
    }

  }
}
