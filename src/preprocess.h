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

#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

#include "itrs.h"

struct Transition;

/**
 * Methods useful for preprocessing / simplifying the transitions
 */
namespace Preprocess
{
    /**
     * Tries to remove the constraint "cost >= 0" from the guard
     * (which is assumed to be the last element of guard!)
     * @note if no "cost >= 0" was added, then the last guard constraint might be (soundly) removed
     * @return true iff "cost >= 0" is implied by the guard and was removed
     */
    bool tryToRemoveCost(const ITRSProblem &itrs, GuardList &guard);

    /**
     * Expensive preprocessing of the given transition.
     * This includes finding equalities, removing free variables, removing trivial constraints.
     * @param trans the transition, modified.
     * @return true iff trans was modified
     */
    bool simplifyTransition(const ITRSProblem &itrs, Transition &trans);

    /**
     * Removes trivial terms from the given guard, i.e. 42 <= 1337 or x <= x+1
     * @note this does _not_ involve any SMT queries and thus only removes very trivial terms
     * @return true iff guard was modified
     */
    bool removeTrivialGuards(GuardList &guard);

    /**
     * Removes terms for which stronger variants appear in the guard, i.e. x >= 0, x > 0 --> x > 0
     * @note this _does_ involve many SMT queries (though only for every pair, transitivity is not checked)
     * @return true iff guard was modified
     */
    bool removeWeakerGuards(GuardList &guard);

    /**
     * Removes trivial updates of the form x <- x.
     * @return true iff update was modified
     */
    bool removeTrivialUpdates(const ITRSProblem &itrs, UpdateMap &update);

    /**
     * Expensive preprocessing step to remove all free variables from the update and,
     * where possible, also from the guard.
     * @param trans the transition, modified.
     * @return true iff trans was modified
     */
    bool eliminateFreeVars(const ITRSProblem &itrs, Transition &trans);
}

#endif // PREPROCESSOR_H
