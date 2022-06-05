#include "solver.h"

#include <cassert>
#include <map>
#include <memory>
#include <unordered_set>

Solver::Solution Solver::solve(const Board & board)
{
    if (!board.is_solvable()) {
        return Solution(board);
    }
    if (board.is_goal()) {
        return Solution(board, std::vector<std::pair<unsigned, unsigned>>{board.blank_pos()});
    }

    std::shared_ptr<MovedBoard> result = a_star(board);
    assert(result);
    return Solution(*result);
}

std::shared_ptr<MovedBoard> Solver::a_star(const Board & board)
{
    const unsigned k = board.size() > 4 ? 39 : 3;

    std::multimap<std::pair<unsigned, unsigned>, std::shared_ptr<MovedBoard>> current_moves; // Maps {f(x), sum of g(x)} to {Board itself, prev.state delta}
    std::unordered_set<std::size_t> used_states;                                             // Maps Board to prev.state delta

    auto dynamic_brd = std::shared_ptr<Board>(new Board(board));
    current_moves.emplace(std::make_pair(board.manhattan() * k, 0), BoardProps::move_relative(dynamic_brd, 0, 0));

    while (!current_moves.empty()) {
        auto cur_node = current_moves.extract(current_moves.begin());
        if (cur_node.mapped()->is_goal()) {
            return std::move(cur_node.mapped());
        }

        for (const auto & move : available_moves) {
            std::shared_ptr<MovedBoard> new_pos = BoardProps::move_relative(cur_node.mapped(), move.first, move.second);
            if (new_pos && used_states.find(new_pos->hash()) == used_states.end()) {
                current_moves.emplace(std::make_pair(
                                              cur_node.key().second + 1 + new_pos->manhattan() * k,
                                              cur_node.key().second + 1),
                                      std::move(new_pos));
            }
        }
        used_states.emplace(cur_node.mapped()->hash());
    }
    return std::shared_ptr<MovedBoard>();
}
Solver::Solution::Solution(const Board & brd)
    : m_initial(brd)
{
}
Solver::Solution::Solution(const Board & brd, const std::vector<std::pair<unsigned, unsigned>> & moves)
    : m_initial(brd)
    , m_moves(moves)
{
}
Solver::Solution::Solution(const MovedBoard & result)
{
    const BoardProps * current = &result;
    const MovedBoard * cur_moved = dynamic_cast<const MovedBoard *>(current);
    while (cur_moved != nullptr) {
        m_moves.push_back(current->blank_pos());

        current = cur_moved->m_parent.get();
        cur_moved = dynamic_cast<const MovedBoard *>(current);
    }
    std::reverse(m_moves.begin(), m_moves.end());
    const Board * initial = dynamic_cast<const Board *>(current);
    assert(initial);
    m_initial = *initial;
}
