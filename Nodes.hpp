#ifndef NODES_HPP
#define NODES_HPP

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
#include "Pointers.hpp"

#include <functional>
#include <unordered_set>
#include <boost/optional.hpp>

#define maybe_return(val) \
    if ((val) != nullptr) return *(val); else return boost::none;

namespace monicelli {

enum class Type {
    INT,
    CHAR,
    FLOAT,
    BOOL,
    DOUBLE,
    VOID
};


class Emittable {
public:
    virtual ~Emittable() {}
    virtual void emit(Emitter *emitter) const = 0;
};


class Statement: public Emittable {
public:
    virtual void emit(Emitter *) const {}
};

class SemiExpression: public Emittable {
public:
    virtual void emit(Emitter *) const {}
};

class Expression: public Emittable {
public:
    virtual void emit(Emitter *) const {}
};

class SimpleExpression: public Expression {
public:
    virtual void emit(Emitter *) const {}
};


class Id: public SimpleExpression {
public:
    explicit Id(std::string *c): value(c) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    std::string const& getValue() const {
        return *value;
    }

private:
    Pointer<std::string> value;
};



class Number: public SimpleExpression {};

class Integer: public Number {
public:
    Integer(long i): value(i) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    long getValue() const {
        return value;
    }

private:
    long value;
};


class Float: public Number {
public:
    Float(double f): value(f) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    double getValue() const {
        return value;
    }

private:
    double value;
};


class Return: public Statement {
public:
    explicit Return(Expression *e): expression(e) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    boost::optional<Expression const&> getExpression() const {
        maybe_return(expression);
    }

private:
    Pointer<Expression> expression;
};


class Loop: public Statement {
public:
    Loop(PointerList<Statement> *b, Expression *c): body(b), condition(c) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    PointerList<Statement> const& getBody() const {
        return *body;
    }

    Expression const& getCondition() const {
        return *condition;
    }

private:
    Pointer<PointerList<Statement>> body;
    Pointer<Expression> condition;
};


class VarDeclaration: public Statement {
public:
    VarDeclaration(Id *n, Type t, bool p, Expression *i):
        name(n), point(p), init(i), type(t) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    Id const& getId() const {
        return *name;
    }

    bool isPointer() const {
        return point;
    }

    boost::optional<Expression const&> getInitializer() const {
        maybe_return(init);
    }

    Type getType() const {
        return type;
    }

private:
    Pointer<Id> name;
    bool point;
    Pointer<Expression> init;
    Type type;
};


class Assignment: public Statement {
public:
    Assignment(Id *n, Expression *v): name(n), value(v) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    Id const& getName() const {
        return *name;
    }

    Expression const& getValue() const {
        return *value;
    }

private:
    Pointer<Id> name;
    Pointer<Expression> value;
};


class Print: public Statement {
public:
    explicit Print(Expression *e): expression(e) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    Expression const& getExpression() const {
        return *expression;
    }

private:
    Pointer<Expression> expression;
};


class Input: public Statement {
public:
    explicit Input(Id *v): variable(v) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    Id const& getVariable() const {
        return *variable;
    }

private:
    Pointer<Id> variable;
};


class Abort: public Statement {
public:
    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }
};


class Assert: public Statement {
public:
    explicit Assert(Expression *e): expression(e) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    Expression const& getExpression() const {
        return *expression;
    }

private:
    Pointer<Expression> expression;
};


class FunctionCall: public Statement, public Expression {
public:
    FunctionCall(Id *n, PointerList<Expression> *a): name(n), args(a) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    Id const& getName() const {
        return *name;
    }

    PointerList<Expression> const& getArgs() const {
        return *args;
    }

private:
    Pointer<Id> name;
    Pointer<PointerList<Expression>> args;
};


class BranchCase: public Emittable {
public:
    BranchCase(SemiExpression *c, PointerList<Statement> *b): condition(c), body(b) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    SemiExpression const& getCondition() const {
        return *condition;
    }

    PointerList<Statement> const& getBody() const {
        return *body;
    }

private:
    Pointer<SemiExpression> condition;
    Pointer<PointerList<Statement>> body;
};


class Branch: public Statement {
public:
    class Body {
    public:
        Body(PointerList<BranchCase> *c, PointerList<Statement> *e = nullptr): cases(c), els(e) {}

        PointerList<BranchCase> const& getCases() const {
            return *cases;
        }

        boost::optional<PointerList<Statement> const&> getElse() const {
            maybe_return(els);
        }

    private:
        Pointer<PointerList<BranchCase>> cases;
        Pointer<PointerList<Statement>> els;
    };

    Branch(Id *v, Branch::Body *b): var(v), body(b) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    Id const& getVar() const {
        return *var;
    }

    Branch::Body const& getBody() const {
        return *body;
    }

private:
    Pointer<Id> var;
    Pointer<Branch::Body> body;
};


class Main: public Emittable {
public:
    Main(PointerList<Statement> *s): body(s) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    PointerList<Statement> const& getBody() const {
        return *body;
    }

private:
    Pointer<PointerList<Statement>> body;
};


class FunArg {
public:
    FunArg(Id *n, Type t, bool p): name(n), type(t), pointer(p) {}

    Id const& getName() const {
        return *name;
    }

    Type getType() const {
        return type;
    }

    bool isPointer() const {
        return pointer;
    }

private:
    Pointer<Id> name;
    Type type;
    bool pointer;
};


class Function: public Emittable {
public:
    Function(Id *n, Type r, PointerList<FunArg> *a, PointerList<Statement> *b):
        name(n), type(r), args(a), body(b) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    Id const& getName() const {
        return *name;
    }

    Type getType() const {
        return type;
    }

    PointerList<FunArg> const& getArgs() const {
        return *args;
    }

    PointerList<Statement> const& getBody() const {
        return *body;
    }

private:
    Pointer<Id> name;
    Type type;
    Pointer<PointerList<FunArg>> args;
    Pointer<PointerList<Statement>> body;
};


class Module: public Emittable {
public:
    enum Type {
        SYSTEM, USER
    };

    Module(const std::string &n, Type s): name(n), type(s) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    bool operator==(const Module &other) const noexcept {
        return (name == other.name) && (type == other.type);
    }

    size_t hash() const noexcept {
        return std::hash<std::string>()(name) ^ std::hash<bool>()(type);
    }

    std::string const& getName() const {
        return name;
    }

    Type getType() const {
        return type;
    }

private:
    std::string name;
    Type type;
};

} // namespace

namespace std {

template<>
struct hash<monicelli::Module> {
public:
    size_t operator ()(const monicelli::Module &e) const noexcept {
        return e.hash();
    }
};

}

namespace monicelli {

class Program: public Emittable {
public:
    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    void setMain(Main *m) {
        main = Pointer<Main>(m);
    }

    void addFunction(Function *f) {
        functions.push_back(f);
    }

    void addModule(Module *m) {
        modules.insert(std::move(*m));
        delete m;
    }

    boost::optional<Main const&> getMain() const {
        maybe_return(main);
    }

    PointerList<Function> const& getFunctions() const {
        return functions;
    }

    std::unordered_set<Module> const& getModules() const {
        return modules;
    }

private:
    Pointer<Main> main;
    PointerList<Function> functions;
    std::unordered_set<Module> modules;
};


enum class Operator {
    PLUS, MINUS, TIMES, DIV,
    SHL, SHR,
    LT, GT, GTE, LTE, EQ
};


class BinaryExpression: public Expression {
public:
    BinaryExpression(Expression *l, Operator op, Expression *r):
        left(l), op(op), right(r) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    Expression const& getLeft() const {
        return *left;
    }

    Expression const& getRight() const {
        return *right;
    }

    Operator getOperator() const {
        return op;
    }

private:
    Pointer<Expression> left;
    Operator op;
    Pointer<Expression> right;
};


class ExpLt: public BinaryExpression {
public:
    ExpLt(Expression *l, Expression *r): BinaryExpression(l, Operator::LT, r) {}
};


class ExpGt: public BinaryExpression {
public:
    ExpGt(Expression *l, Expression *r): BinaryExpression(l, Operator::GT, r) {}
};


class ExpLte: public BinaryExpression {
public:
    ExpLte(Expression *l, Expression *r): BinaryExpression(l, Operator::LTE, r) {}
};


class ExpGte: public BinaryExpression {
public:
    ExpGte(Expression *l, Expression *r): BinaryExpression(l, Operator::GTE, r) {}
};


class ExpPlus: public BinaryExpression {
public:
    ExpPlus(Expression *l, Expression *r): BinaryExpression(l, Operator::PLUS, r) {}

};


class ExpMinus: public BinaryExpression {
public:
    ExpMinus(Expression *l, Expression *r): BinaryExpression(l, Operator::MINUS, r) {}
};


class ExpTimes: public BinaryExpression {
public:
    ExpTimes(Expression *l, Expression *r): BinaryExpression(l, Operator::TIMES, r) {}
};


class ExpDiv: public BinaryExpression {
public:
    ExpDiv(Expression *l, Expression *r): BinaryExpression(l, Operator::DIV, r) {}
};


class ExpShl: public BinaryExpression {
public:
    ExpShl(Expression *l, Expression *r): BinaryExpression(l, Operator::SHL, r) {}
};


class ExpShr: public BinaryExpression {
public:
    ExpShr(Expression *l, Expression *r): BinaryExpression(l, Operator::SHR, r) {}
};


class BinarySemiExpression: public SemiExpression {
public:
    BinarySemiExpression(Operator op, Expression *l): op(op), left(l) {}

    virtual void emit(Emitter *emitter) const {
        emitter->emit(*this);
    }

    Expression const& getLeft() const {
        return *left;
    }

    Operator getOperator() const {
        return op;
    }

private:
    Operator op;
    Pointer<Expression> left;
};


class SemiExpEq: public BinarySemiExpression {
public:
    SemiExpEq(Expression *l): BinarySemiExpression(Operator::EQ, l) {}
};


class SemiExpLt: public BinarySemiExpression {
public:
    SemiExpLt(Expression *l): BinarySemiExpression(Operator::LT, l) {}
};


class SemiExpGt: public BinarySemiExpression {
public:
    SemiExpGt(Expression *l): BinarySemiExpression(Operator::GT, l) {}
};


class SemiExpLte: public BinarySemiExpression {
public:
    SemiExpLte(Expression *l): BinarySemiExpression(Operator::LTE, l) {}
};


class SemiExpGte: public BinarySemiExpression {
public:
    SemiExpGte(Expression *l): BinarySemiExpression(Operator::GTE, l) {}
};


class SemiExpPlus: public BinarySemiExpression {
public:
    SemiExpPlus(Expression *l): BinarySemiExpression(Operator::PLUS, l) {}
};


class SemiExpMinus: public BinarySemiExpression {
public:
    SemiExpMinus(Expression *l): BinarySemiExpression(Operator::MINUS, l) {}
};


class SemiExpTimes: public BinarySemiExpression {
public:
    SemiExpTimes(Expression *l): BinarySemiExpression(Operator::TIMES, l) {}
};


class SemiExpDiv: public BinarySemiExpression {
public:
    SemiExpDiv(Expression *l): BinarySemiExpression(Operator::DIV, l) {}
};


class SemiExpShl: public BinarySemiExpression {
public:
    SemiExpShl(Expression *l): BinarySemiExpression(Operator::SHR, l) {}
};


class SemiExpShr: public BinarySemiExpression {
public:
    SemiExpShr(Expression *l): BinarySemiExpression(Operator::SHL, l) {}
};

} // namespace

#undef maybe_return

#endif

