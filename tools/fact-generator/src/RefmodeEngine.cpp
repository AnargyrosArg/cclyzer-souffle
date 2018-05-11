#include <sstream>
#include <llvm/Support/raw_ostream.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/DebugInfoMetadata.h>
#include "RefmodeEngine.hpp"
#include "RefmodeEngineImpl.hpp"
#include "llvm_enums.hpp"


using std::string;
using llvm::cast;
using llvm::dyn_cast;
using llvm::isa;
using llvm::raw_string_ostream;
using cclyzer::RefmodeEngine;
using cclyzer::refmode_t;

namespace enums = cclyzer::utils;


//------------------------------------------------------------------------------
// Template Specializations for various types
//------------------------------------------------------------------------------

namespace cclyzer {

    template<typename T> refmode_t
    RefmodeEngine::Impl::refmode(const T& obj) // const
    {
        return enums::to_string(obj);
    }


    template<> refmode_t
    RefmodeEngine::Impl::refmode(const llvm::Type& type) // const
    {
        string type_str;
        raw_string_ostream rso(type_str);

        if (type.isStructTy()) {
            const llvm::StructType *STy = cast<llvm::StructType>(&type);

            if (STy->isLiteral()) {
                type.print(rso);
                return rso.str();
            }

            if (STy->hasName()) {
                rso << "%" << STy->getName();
                return rso.str();
            }
            rso << "%\"type " << STy << "\"";
        }
        else {
            type.print(rso);
        }
        return rso.str();
    }


    template<> refmode_t
    RefmodeEngine::Impl::refmode(const llvm::Instruction& insn) // const
    {
        std::ostringstream refmode;

        // BasicBlock context is intended so as not to qualify instruction
        // id by its surrounding basic block's id

        withContext<llvm::Function>(refmode) << std::to_string(ctx->instrCount() - 1);
        return refmode.str();
    }


    template<> refmode_t
    RefmodeEngine::Impl::refmode(const llvm::Constant& constant) // const
    {
        std::ostringstream refmode;

        withContext<llvm::Instruction>(refmode)
            << ctx->constantCount() << ':' << refmodeOf(&constant);

        return refmode.str();
    }


    template<> refmode_t
    RefmodeEngine::Impl::refmode(const llvm::BasicBlock& basicblock) // const
    {
        string bbName = refmodeOf(&basicblock);
        std::ostringstream refmode;

        withContext<llvm::Function>(refmode) << "[basicblock]" << bbName;
        return refmode.str();
    }


    template<> refmode_t
    RefmodeEngine::Impl::refmode(const llvm::Function& func) // const
    {
        string functionName = string(func.getName());
        std::ostringstream refmode;

        withGlobalContext(refmode) << functionName;
        return refmode.str();
    }


    template<> refmode_t
    RefmodeEngine::Impl::refmode(const llvm::InlineAsm& val) // const
    {
        std::ostringstream refmode;

        withContext<llvm::Instruction>(refmode)
            << ':' << "<asm>";

        return refmode.str();
    }


    template<> refmode_t
    RefmodeEngine::Impl::refmode(const llvm::GlobalValue& val) // const
    {
        string id = refmodeOf(&val);
        std::ostringstream refmode;

        withGlobalContext(refmode) << id;
        return refmode.str();
    }


    template<> refmode_t
    RefmodeEngine::Impl::refmode(const llvm::Value& val) // const
    {
        if (const llvm::BasicBlock *bb = dyn_cast<llvm::BasicBlock>(&val))
            return refmode<llvm::BasicBlock>(*bb);

        refmode_t id = refmodeOf(&val);
        std::ostringstream refmode;

        withContext<llvm::Function>(refmode) << id;
        return refmode.str();
    }


    template<> refmode_t
    RefmodeEngine::Impl::refmode(const llvm::DINode& node) // const
    {
        std::ostringstream refmode;

        string rv;
        raw_string_ostream rso(rv);
        appendMetadataId(rso, node);

        withGlobalContext(refmode) << rso.str();
        return refmode.str();
    }


    template<> refmode_t
    RefmodeEngine::Impl::refmode(const llvm::MDNode& node) // const
    {
        std::ostringstream refmode;

        string rv;
        raw_string_ostream rso(rv);
        appendMetadataId(rso, node);

        withGlobalContext(refmode) << rso.str();
        return refmode.str();
    }
}


//------------------------------------------------------------------------------
// Opaque Pointer Idiom Implementation
//------------------------------------------------------------------------------

RefmodeEngine::RefmodeEngine() {
    impl = new Impl();
}

RefmodeEngine::~RefmodeEngine() {
    delete impl;
}

template<typename T>
refmode_t RefmodeEngine::refmode(const T& obj) const {
    return impl->refmode(obj);
}

void RefmodeEngine::enterContext(const llvm::Value& val) {
    impl->enterContext(val);
}

void RefmodeEngine::exitContext() {
    impl->exitContext();
}

void RefmodeEngine::enterModule(const llvm::Module& module, const std::string& path) {
    impl->enterModule(module, path);
}

void RefmodeEngine::exitModule() {
    impl->exitModule();
}

//------------------------------------------------------------------------------
// Explicit template instantiations
//------------------------------------------------------------------------------

template refmode_t
RefmodeEngine::refmode<llvm::Type>(const llvm::Type & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::Instruction>(const llvm::Instruction & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::Constant>(const llvm::Constant & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::BasicBlock>(const llvm::BasicBlock & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::Function>(const llvm::Function & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::InlineAsm>(const llvm::InlineAsm & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::GlobalValue>(const llvm::GlobalValue & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::Value>(const llvm::Value & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::GlobalValue::LinkageTypes>(
    const llvm::GlobalValue::LinkageTypes & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::GlobalValue::VisibilityTypes>(
    const llvm::GlobalValue::VisibilityTypes & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::GlobalVariable::ThreadLocalMode>(
    const llvm::GlobalVariable::ThreadLocalMode &) const;

template refmode_t
RefmodeEngine::refmode<llvm::CallingConv::ID>(
    const llvm::CallingConv::ID & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::AtomicOrdering>(
    const llvm::AtomicOrdering & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::DINode>(
    const llvm::DINode & ) const;

template refmode_t
RefmodeEngine::refmode<llvm::MDNode>(
    const llvm::MDNode & ) const;
