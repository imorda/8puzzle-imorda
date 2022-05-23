#include "board.h"

#include <algorithm>
#include <numeric>

Board Board::create_goal(const unsigned size)
{
    std::vector<std::vector<unsigned>> field(size, std::vector<unsigned>(size));

    for (unsigned i = 0; i < size * size; ++i) {
        field[i / size][i % size] = (i + 1) % (size * size);
    }
    return Board(std::move(field));
}

Board Board::create_random(const unsigned size)
{
    thread_local std::random_device rd;
    std::mt19937 engine(rd());  // NOLINT
    std::vector<unsigned> field_1d(size * size);
    std::iota(field_1d.begin(), field_1d.end(), 0);
    std::shuffle(field_1d.begin(), field_1d.end(), engine);

    std::vector<std::vector<unsigned>> field_2d(size);

    for (unsigned i = 0; i < size; ++i) {
        field_2d[i].reserve(size);
        for (unsigned j = 0; j < size; ++j) {
            field_2d[i].push_back(field_1d.back());
            field_1d.pop_back();
        }
    }

    return Board(std::move(field_2d));
}

Board::Board()
    : Board(std::vector<std::vector<unsigned>>())
{
}

unsigned Board::count(const std::function<unsigned(unsigned, unsigned, unsigned, unsigned)> & func) const
{
    unsigned counter = 0;
    for (unsigned i = 0; i < size(); ++i) {
        for (unsigned j = 0; j < size(); ++j) {
            counter += func(i, j, m_data[i][j], size());
        }
    }
    return counter;
}

unsigned Board::calculate_hamming() const
{
    return count(&calculate_single_hamming);
}
unsigned Board::calculate_manhattan() const
{
    return count(&calculate_single_manhattan);
}

unsigned Board::count_inversions_parity() const
{
    unsigned sz_squared = size() * size();
    std::vector<unsigned> copy; // 1D clone without blank element
    copy.reserve(sz_squared - 1);
    for (const auto & i : m_data) {
        for (const auto & j : i) {
            if (j != 0) {
                copy.push_back(j);
            }
        }
    }

    unsigned counter = 0;
    for (unsigned i = 0; i < sz_squared - 1; ++i) {
        unsigned goal_pos = copy[i] - 1;
        while (goal_pos != i) {
            std::swap(copy[i], copy[goal_pos]);
            counter += 1;
            counter %= 2;
            goal_pos = copy[i] - 1;
        }
    }
    return counter;
}
std::pair<unsigned, unsigned> Board::calc_blank_pos() const
{
    if (size() == 0) {
        return {0, 0};
    }
    for (unsigned i = 0; i < size(); ++i) {
        for (unsigned j = 0; j < size(); ++j) {
            if (m_data[i][j] == 0) {
                return {i, j};
            }
        }
    }
    throw std::logic_error("No empty element in board");
}

Board::Board(std::vector<std::vector<unsigned int>> && field)
    : m_data(std::move(field))
{
    calculate_props();
}
Board::Board(const std::vector<std::vector<unsigned int>> & field)
    : m_data(field)
{
    calculate_props();
}
bool Board::calculate_solvability() const
{
    if (is_goal()) {
        return true;
    }
    unsigned inversions = count_inversions_parity();
    if (size() % 2 != 0) {
        return inversions == 0;
    }
    if (blank_pos().first % 2 == 0) {
        return inversions != 0;
    }
    return inversions == 0;
}
std::size_t Board::calculate_hash() const
{
    std::size_t p = 1;
    std::size_t result = 0;
    for (unsigned i = 0; i < size(); ++i) {
        for (unsigned j = 0; j < size(); ++j) {
            result += m_data[i][j] * p;
            p *= HASH_P;
        }
    }
    return result;
}
size_t Board::size() const
{
    return m_data.size();
}
unsigned int Board::get(unsigned int x, unsigned int y) const
{
    return m_data[x][y];
}
void Board::calculate_props()
{
    m_blank_pos = calc_blank_pos();
    m_manhattan = calculate_manhattan();
    m_hamming = calculate_hamming();
    m_is_solvable = calculate_solvability();
    m_hash = calculate_hash();
}
BoardProps::BoardProps(const std::pair<unsigned int, unsigned int> & mBlankPos, unsigned int mManhattan, unsigned int mHamming, bool mIsSolvable, size_t mHash)
    : m_blank_pos(mBlankPos)
    , m_manhattan(mManhattan)
    , m_hamming(mHamming)
    , m_is_solvable(mIsSolvable)
    , m_hash(mHash)
{
}
bool BoardProps::is_goal() const
{
    return size() == 0 || hamming() == 0;
}
unsigned BoardProps::hamming() const
{
    return m_hamming;
}
unsigned BoardProps::manhattan() const
{
    return m_manhattan;
}
std::size_t BoardProps::hash() const
{
    return m_hash;
}
std::pair<unsigned, unsigned> BoardProps::blank_pos() const
{
    return m_blank_pos;
}
std::string BoardProps::to_string() const
{
    unsigned num_len = count_num_len(size() * size());
    std::string result = "";

    for (unsigned i = 0; i < size(); ++i) {
        result += '|';
        for (unsigned j = 0; j < size(); ++j) {
            std::string cur_num = std::to_string((*this)[i][j]);
            unsigned cur_num_len = cur_num.length();
            while (cur_num_len < num_len) {
                result += ' ';
                cur_num_len++;
            }
            result += cur_num;
            result += "|";
        }
        result += '\n';
    }

    return result.empty() ? "<empty>" : result;
}
bool BoardProps::is_solvable() const
{
    return m_is_solvable;
}

/// Constructs a new instance of Board (if possible) only recalculating items that actually moved
std::shared_ptr<MovedBoard> BoardProps::move_relative(const std::shared_ptr<BoardProps> & par, unsigned int dx, unsigned int dy)
{
    const std::pair<signed, signed> target = {par->blank_pos().first + dx, par->blank_pos().second + dy};
    if (0 <= target.first && target.first < static_cast<int>(par->size()) && 0 <= target.second && target.second < static_cast<int>(par->size())) {
        return std::shared_ptr<MovedBoard>(new MovedBoard(par, target));
    }
    return std::shared_ptr<MovedBoard>();
}
unsigned BoardProps::calculate_single_hamming(unsigned int x, unsigned int y, unsigned int label, unsigned int size)
{
    return (label != (x * size + y + 1) % (size * size)) ? 1 : 0;
}
unsigned BoardProps::calculate_single_manhattan(unsigned int x, unsigned int y, unsigned int label, unsigned int size)
{
    if (label == 0) {
        return 0;
    }
    unsigned real_index = label - 1;
    return abs(static_cast<signed>(real_index / size) - static_cast<signed>(x)) + abs(static_cast<signed>(real_index % size) - static_cast<signed>(y));
}
unsigned BoardProps::count_num_len(unsigned int num)
{
    unsigned counter = 1;
    while (num > 9) {
        num /= 10;
        counter++;
    }
    return counter;
}
ProxyRow<BoardProps, unsigned int> BoardProps::operator[](unsigned int i) const
{
    return ProxyRow<BoardProps, unsigned>(*this, i);
}
std::size_t BoardProps::bin_pow(std::size_t base, std::size_t exp)
{
    if (exp == 0) {
        return 1;
    }
    if (exp == 1) {
        return base;
    }
    if (exp % 2 == 0) {
        std::size_t half = bin_pow(base, exp / 2);
        return half * half;
    }
    return bin_pow(base, exp - 1) * base;
}
size_t MovedBoard::size() const
{
    return m_size;
}
unsigned int MovedBoard::get(unsigned int x, unsigned int y) const
{
    std::pair<unsigned, unsigned> pos = {x, y};
    if (pos == blank_pos()) {
        return 0;
    }
    if (pos == m_parent->blank_pos()) {
        return m_parent->get(blank_pos().first, blank_pos().second);
    }
    return m_parent->get(x, y);
}
MovedBoard::MovedBoard(std::shared_ptr<BoardProps> parent, const std::pair<unsigned, unsigned> new_blank)
    : BoardProps(new_blank,
                 parent->manhattan() - calculate_single_manhattan(new_blank.first, new_blank.second, (*parent)[new_blank.first][new_blank.second], parent->size()) + calculate_single_manhattan(parent->blank_pos().first, parent->blank_pos().second, (*parent)[new_blank.first][new_blank.second], parent->size()),
                 parent->hamming() - calculate_single_hamming(new_blank.first, new_blank.second, (*parent)[new_blank.first][new_blank.second], parent->size()) - calculate_single_hamming(parent->blank_pos().first, parent->blank_pos().second, (*parent)[parent->blank_pos().first][parent->blank_pos().second], parent->size()) + calculate_single_hamming(parent->blank_pos().first, parent->blank_pos().second, (*parent)[new_blank.first][new_blank.second], parent->size()) + calculate_single_hamming(new_blank.first, new_blank.second, (*parent)[parent->blank_pos().first][parent->blank_pos().second], parent->size()),
                 parent->is_solvable(),
                 parent->hash() - (*parent)[new_blank.first][new_blank.second] * bin_pow(HASH_P, new_blank.first * parent->size() + new_blank.second) + (*parent)[new_blank.first][new_blank.second] * bin_pow(HASH_P, parent->blank_pos().first * parent->size() + parent->blank_pos().second))
    , m_parent(std::move(parent))
    , m_size(m_parent->size())
{
}
