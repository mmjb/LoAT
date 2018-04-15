/*  This file is part of LoAT.
 *  Copyright (c) 2015-2016 Matthias Naaf, RWTH Aachen University, Germany
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program. If not, see <http://www.gnu.org/licenses>.
 */

#include "relation.h"


namespace Relation {

    bool isRelation(const Expression &ex) {
        return GiNaC::is_a<GiNaC::relational>(ex)
               && ex.nops() == 2
               && !ex.info(GiNaC::info_flags::relation_not_equal);
    }

    bool isEquality(const Expression &ex) {
        return isRelation(ex) && ex.info(GiNaC::info_flags::relation_equal);
    }

    bool isInequality(const Expression &ex) {
        return isRelation(ex) && !isEquality(ex);
    }

    bool isNormalizedInequality(const Expression &ex) {
        return isInequality(ex)
               && ex.info(GiNaC::info_flags::relation_greater)
               && ex.rhs().is_zero();
    }

    bool isLinearInequality(const Expression &ex, const GiNaC::lst &vars) {
        if (!isInequality(ex)) return false;
        return Expression(ex.lhs()).isLinear(vars) && Expression(ex.rhs()).isLinear(vars);
    }

    Expression replaceLhsRhs(const Expression &rel, Expression lhs, Expression rhs) {
        assert(isRelation(rel));
        if (rel.info(GiNaC::info_flags::relation_less_or_equal)) return lhs <= rhs;
        if (rel.info(GiNaC::info_flags::relation_greater)) return lhs > rhs;
        if (rel.info(GiNaC::info_flags::relation_less)) return lhs < rhs;
        if (rel.info(GiNaC::info_flags::relation_greater_or_equal)) return lhs >= rhs;
        if (rel.info(GiNaC::info_flags::relation_equal)) return lhs = rhs;
        unreachable();
    }

    Expression transformInequalityLessEq(Expression rel) {
        assert(isInequality(rel));

        //flip > or >=
        if (rel.info(GiNaC::info_flags::relation_greater)) {
            rel = rel.rhs() < rel.lhs();
        } else if (rel.info(GiNaC::info_flags::relation_greater_or_equal)) {
            rel = rel.rhs() <= rel.lhs();
        }

        //change < to <=, assuming integer arithmetic
        if (rel.info(GiNaC::info_flags::relation_less)) {
            rel = rel.lhs() <= (rel.rhs() - 1);
        }

        assert(rel.info(GiNaC::info_flags::relation_less_or_equal));
        return rel;
    }

    Expression transformInequalityGreater(Expression rel) {
        assert(isInequality(rel));

        //flip < or <=
        if (rel.info(GiNaC::info_flags::relation_less)) {
            rel = rel.rhs() > rel.lhs();
        } else if (rel.info(GiNaC::info_flags::relation_less_or_equal)) {
            rel = rel.rhs() >= rel.lhs();
        }

        //change >= to >, assuming integer arithmetic
        if (rel.info(GiNaC::info_flags::relation_greater_or_equal)) {
            rel = (rel.lhs() + 1) > rel.rhs();
        }

        assert(rel.info(GiNaC::info_flags::relation_greater));
        return rel;
    }

    Expression normalizeInequality(Expression rel) {
        assert(isInequality(rel));

        Expression greater = transformInequalityGreater(rel);
        Expression normalized = (greater.lhs() - greater.rhs()) > 0;

        assert(isNormalizedInequality(normalized));
        return normalized;
    }

    Expression transformInequalityLessOrLessEq(Expression rel) {
        assert(rel.info(GiNaC::info_flags::relation_equal)
               || isInequality(rel));

        if (rel.info(GiNaC::info_flags::relation_greater_or_equal)) {
            rel = rel.rhs() <= rel.lhs();
        } else if (rel.info(GiNaC::info_flags::relation_greater)) {
            rel = rel.rhs() < rel.lhs();
        }

        return rel;
    }

    Expression splitVariablesAndConstants(const Expression &rel) {
        assert(isInequality(rel));

        //move everything to lhs
        GiNaC::ex newLhs = rel.lhs() - rel.rhs();
        GiNaC::ex newRhs = 0;

        //move all numerical constants back to rhs
        newLhs = newLhs.expand();
        if (GiNaC::is_a<GiNaC::add>(newLhs)) {
            for (int i=0; i < newLhs.nops(); ++i) {
                if (GiNaC::is_a<GiNaC::numeric>(newLhs.op(i))) {
                    newRhs = newRhs - newLhs.op(i);
                }
            }
        }

        // TODO: What if newLhs is not an add, but only a numeric or only a symbol? (or e.g. 2*x)
        // TODO: should probably apply the loop body to newLhs itself!

        newLhs = newLhs + newRhs;
        return replaceLhsRhs(rel, newLhs, newRhs);
    }

    Expression negateLessEqInequality(const Expression &relLessEq) {
        assert(isInequality(relLessEq));
        assert(relLessEq.info(GiNaC::info_flags::relation_less_or_equal));
        return (-relLessEq.lhs()) <= (-relLessEq.rhs())-1;
    }

    bool isTrivialLessEqInequality(const Expression &relLessEq) {
        using namespace GiNaC;
        assert(relLessEq.info(info_flags::relation_less_or_equal));

        // TODO: This can possibly be improved by checking if rhs-lhs is a nonnegative numeric

        ex lhs = relLessEq.lhs();
        ex rhs = relLessEq.rhs();

        if (is_a<numeric>(lhs) && is_a<numeric>(rhs)) {
            numeric lhsNum = ex_to<numeric>(lhs);
            numeric rhsNum = ex_to<numeric>(rhs);

            if (lhsNum.is_equal(rhsNum)) {
                return true;
            }

            if (lhsNum.is_integer() && rhsNum.is_integer() && lhsNum.to_int() <= rhsNum.to_int()) {
                return true;
            }

        } else if ((lhs - rhs).is_zero()) {
            return true;
        }

        return false;
    }
}