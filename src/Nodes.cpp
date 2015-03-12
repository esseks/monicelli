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

#include "Nodes.hpp"
#include <string>

using namespace monicelli;

Function *monicelli::makeMain(PointerList<Statement> *body) {
    PointerList<FunArg> *noargs = new PointerList<FunArg>();

    FunctionPrototype *proto = new FunctionPrototype(
        new Id("main"), Type::VOID, noargs
    );

    return new Function(proto, body);
}

std::ostream& monicelli::operator<<(std::ostream &stream, Type const& type) {
    switch (type) {
        case Type::INT:
            stream << "int";
            break;
        case Type::CHAR:
            stream << "char";
            break;
        case Type::FLOAT:
            stream << "float";
            break;
        case Type::BOOL:
            stream << "bool";
            break;
        case Type::DOUBLE:
            stream << "double";
            break;
        case Type::VOID:
            stream << "void";
            break;
        case Type::UNKNOWN:
            stream << "???????????";
            break;
    }

    return stream;
}

std::ostream& monicelli::operator<<(std::ostream &stream, Operator const& op) {
    switch (op) {
        case Operator::PLUS:
            stream << '+';
            break;
        case Operator::MINUS:
            stream << '-';
            break;
        case Operator::TIMES:
            stream << '*';
            break;
        case Operator::DIV:
            stream << '/';
            break;
        case Operator::SHL:
            stream << "<<";
            break;
        case Operator::SHR:
            stream << ">>";
            break;
        case Operator::LT:
            stream << '<';
            break;
        case Operator::GT:
            stream << '>';
            break;
        case Operator::GTE:
            stream << ">=";
            break;
        case Operator::LTE:
            stream << "<=";
            break;
        case Operator::EQ:
            stream << "==";
            break;
    }

    return stream;
}

