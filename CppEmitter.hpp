#ifndef CPPEMITTER_HPP
#define CPPEMITTER_HPP

/*
 * Monicelli: an esoteric language compiler
 * 
 * Copyright (C) 2014 Stefano Sanfilippo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Emitter.hpp"

#include <iostream>


namespace monicelli {

class CppEmitter: public Emitter {
public:
    CppEmitter(std::ostream *stream): stream(*stream), indent_chars(0) {}

    virtual bool emit(Return const&) override;
    virtual bool emit(Loop const&) override;
    virtual bool emit(VarDeclaration const&) override;
    virtual bool emit(Assignment const&) override;
    virtual bool emit(Print const&) override;
    virtual bool emit(Input const&) override;
    virtual bool emit(Abort const&) override;
    virtual bool emit(Assert const&) override;
    virtual bool emit(Branch const&) override;
    virtual bool emit(FunctionPrototype const&) override;
    virtual bool emit(Function const&) override;
    virtual bool emit(Module const&) override;
    virtual bool emit(Program const&) override;

    virtual bool emit(FunctionCall const&) override;
    virtual bool emit(Id const&) override;
    virtual bool emit(Integer const&) override;
    virtual bool emit(Float const&) override;
    virtual bool emit(BinaryExpression const&) override;

private:
    bool emitIndent();
    bool emitFunctionParams(PointerList<FunArg> const& funargs);
    bool emitFunctionArglist(PointerList<Expression> const& args);
    bool emitStatements(PointerList<Statement> const& node);
    bool emitBranchCondition(SemiExpression const& node);
    bool emitBranchCase(BranchCase const& node);
    bool emitMain(Function const& main);

    void indent();
    void dedent();

    std::ostream &stream;
    int indent_chars;
};

}

#endif

