#pragma once

#include "board.h"

#include <cassert>
#include <utility>

class Solver
{
    class Solution
    {
    public:
        class Iterator
        {
        public:
            using difference_type = std::ptrdiff_t;
            using value_type = const Board;
            using pointer = value_type *;
            using reference = value_type &;
            using iterator_category = std::forward_iterator_tag;

            Iterator(const Iterator &) = default;

            friend bool operator==(const Iterator & lhs, const Iterator & rhs)
            {
                return lhs.m_cur_pos == rhs.m_cur_pos && lhs.m_blank_nodes == rhs.m_blank_nodes;
            }
            friend bool operator!=(const Iterator & lhs, const Iterator & rhs)
            {
                return !(operator==(lhs, rhs));
            }

            reference operator*() const
            {
                return *m_board;
            }

            pointer operator->() const
            {
                return &*m_board;
            }

            Iterator & operator++()
            {
                m_cur_pos++;
                if (m_cur_pos < m_blank_nodes.size()) {
                    auto moved = MovedBoard(m_board, m_blank_nodes[m_cur_pos]);
                    std::vector<std::vector<unsigned>> internal_data(m_board->size());
                    for (unsigned i = 0; i < internal_data.size(); ++i) {
                        internal_data[i].resize(internal_data.size());
                        for (unsigned j = 0; j < internal_data.size(); ++j) {
                            internal_data[i][j] = moved[i][j];
                        }
                    }
                    m_board = std::shared_ptr<Board>(new Board(internal_data, moved.manhattan(), moved.hamming(), moved.blank_pos(), moved.is_solvable(), moved.hash()));
                }
                return *this;
            }

            Iterator operator++(int)
            {
                auto tmp = *this;
                operator++();
                return tmp;
            }

        private:
            friend class Solver;

            Iterator(const Board & board, const std::vector<std::pair<unsigned, unsigned>> & blanks, unsigned pos = 0)
                : m_board(new Board(board))
                , m_blank_nodes(blanks)
                , m_cur_pos(pos)
            {
            }

            std::shared_ptr<Board> m_board;
            const std::vector<std::pair<unsigned, unsigned>> & m_blank_nodes;
            unsigned m_cur_pos = 0;
        };

        Solution(const Board & brd);

        Solution(const Board & brd, const std::vector<std::pair<unsigned, unsigned>> & moves);

        Solution(const Solution &) = default;

        Solution(const MovedBoard & result);

        std::size_t moves() const { return m_moves.empty() ? 0 : m_moves.size() - 1; }

        Iterator begin() const { return Iterator(m_initial, m_moves); }

        Iterator end() const { return Iterator(m_initial, m_moves, m_moves.size()); }

    private:
        Board m_initial;
        std::vector<std::pair<unsigned, unsigned>> m_moves;
    };

    constexpr const static std::array<std::pair<int, int>, 4> available_moves = {std::make_pair(1, 0),
                                                                                 {0, 1},
                                                                                 {-1, 0},
                                                                                 {0, -1}};
    static std::shared_ptr<MovedBoard> a_star(const Board &);

public:
    static Solution solve(const Board & initial);
};
