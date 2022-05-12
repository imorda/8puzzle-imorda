#pragma once

#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

class MovedBoard;

template <typename T, typename R>
class ProxyRow
{
private:
    const T & m_object;
    const std::size_t m_row;

public:
    ProxyRow(const T & object, const std::size_t row)
        : m_object(object)
        , m_row(row)
    {
    }

    R operator[](std::size_t j) const
    {
        return m_object.get(m_row, j);
    }
};

class BoardProps
{
protected:
    static const std::size_t HASH_P = 31;
    std::pair<unsigned, unsigned> m_blank_pos;
    unsigned m_manhattan;
    unsigned m_hamming;
    bool m_is_solvable;
    std::size_t m_hash;

    static std::size_t bin_pow(std::size_t base, std::size_t exp);

public:
    BoardProps() = default;
    BoardProps(const std::pair<unsigned int, unsigned int> & mBlankPos, unsigned int mManhattan, unsigned int mHamming, bool mIsSolvable, size_t mHash);

    static unsigned calculate_single_hamming(unsigned int x, unsigned int y, unsigned label, unsigned size);
    static unsigned calculate_single_manhattan(unsigned int x, unsigned int y, unsigned label, unsigned size);
    static unsigned count_num_len(unsigned num);
    static std::shared_ptr<MovedBoard> move_relative(const std::shared_ptr<BoardProps> & par, unsigned dx, unsigned dy);

    virtual ~BoardProps() = default;
    virtual std::size_t size() const = 0;

    virtual unsigned get(unsigned x, unsigned y) const = 0;
    bool is_goal() const;
    ProxyRow<BoardProps, unsigned> operator[](unsigned i) const;
    unsigned hamming() const;
    unsigned manhattan() const;
    std::size_t hash() const;
    std::pair<unsigned, unsigned> blank_pos() const;
    std::string to_string() const;
    bool is_solvable() const;

    friend bool operator==(const BoardProps & lhs, const BoardProps & rhs)
    {
        if (lhs.hash() != rhs.hash()) {
            return false;
        }
        for (unsigned i = 0; i < lhs.size(); ++i) {
            for (unsigned j = 0; j < rhs.size(); ++j) {
                if (lhs[i][j] != rhs[i][j]) {
                    return false;
                }
            }
        }
        return true;
    }
    friend bool operator!=(const BoardProps & lhs, const BoardProps & rhs)
    {
        return !operator==(lhs, rhs);
    }
    friend std::ostream & operator<<(std::ostream & out, const BoardProps & board)
    {
        return out << board.to_string();
    }
};

class MovedBoard : public BoardProps
{
private:
    friend class Solver;
    std::shared_ptr<BoardProps> m_parent;
    unsigned m_size;

public:
    explicit MovedBoard(std::shared_ptr<BoardProps> parent, std::pair<unsigned, unsigned> new_blank);
    std::size_t size() const override;
    unsigned int get(unsigned int x, unsigned int y) const override;
};

class Board : public BoardProps
{
private:
    unsigned count_inversions_parity() const;
    std::pair<unsigned, unsigned> calc_blank_pos() const;
    unsigned count(const std::function<unsigned(unsigned, unsigned, unsigned, unsigned)> &) const;
    unsigned calculate_hamming() const;
    unsigned calculate_manhattan() const;
    bool calculate_solvability() const;
    std::size_t calculate_hash() const;
    void calculate_props();

    std::vector<std::vector<unsigned>> m_data;

public:
    /// Lightweight unchecked constructor from a raw array of numbers with precalculated cache
    template <class T>
    explicit Board(T && field, unsigned manhattan, unsigned hamming, std::pair<unsigned, unsigned> blank_pos, bool is_solvable, std::size_t hash)
        : BoardProps(blank_pos, manhattan, hamming, is_solvable, hash)
        , m_data(std::forward<T>(field))
    {
    }

    static Board create_goal(unsigned size);

    static Board create_random(unsigned size);

    Board();

    /// Heavy constructor from a raw array of numbers
    explicit Board(std::vector<std::vector<unsigned>> && field);
    explicit Board(const std::vector<std::vector<unsigned>> & field);

    size_t size() const override;
    unsigned int get(unsigned int x, unsigned int y) const override;
};
