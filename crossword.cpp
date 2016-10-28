#include <type_traits>
#include <utility>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <array>
#include <map>
#include <stack>
#include <climits>
using namespace std;

namespace detail {
    typedef unsigned int mask;
    static mask const up = 1 << 0,
        down = 1 << 1,
        left = 1 << 2,
        right = 1 << 3,
        diag_up_left = 1 << 4,
        diag_up_right = 1 << 5,
        diag_down_left = 1 << 6,
        diag_down_right = 1 << 7, none = 0;
    static mask const default_mask = up | down | left | right | diag_up_left | diag_up_right | diag_down_left | diag_down_right;

    int rand(int min, int max) {
        return min + (std::rand() % (int)(max - min + 1));
    }

    template<class T,int N>
    T randchoose(array<T,N> choices) {
        return choices[std::rand() % choices.size()];
    }

    template<class T, class... Args>
    auto make_array(T&& first, Args&&... args) {
        return array<T, sizeof...(Args)+1>{forward<T>(first),forward<Args>(args)...};
    }

    auto new_direction(detail::mask possibleDirections) {
        mask dir = none;
        if (possibleDirections != none) do { dir = 1 << (1 + (::rand() % 8)); } while ((possibleDirections & dir) == 0);
        return dir;
    };
}

struct node {
    int x, y;
    detail::mask dir;
    node(int x, int y, detail::mask dir = detail::none) : x(x), y(y), dir(dir) { }
    bool operator==(node rhs) const
    {
        return tie(x, y) == tie(rhs.x, rhs.y);
    }
    bool operator!=(node rhs) const
    {
        return!operator==(rhs);
    }
    bool operator<(node rhs) const
    {
        return tie(x, y) < tie(rhs.x, rhs.y);
    }
    bool operator>(node rhs) const
    {
        return tie(x, y) > tie(rhs.x, rhs.y);
    }
    bool operator<=(node rhs) const
    {
        return!operator>(rhs);
    }
    bool operator>=(node rhs) const
    {
        return!operator<(rhs);
    }
};

auto generate_board(int n, int m) {
    vector<vector<char>> board(n, vector<char>(m));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            board[i][j] = "abcdefghijklmnopqrstuvwxyz"[rand() % 26];
        }
    }
    return board;
}

auto output_to_board(vector<vector<char>>& board, int boardWidth, int boardHeight, string const& word, detail::mask possibleDirections, node initialPos, map<node, string>& cache) {
    using namespace detail;
    stack<node> nodes;
    vector<node> path;
    int k = 0;
    initialPos.dir = detail::new_direction(possibleDirections);
    nodes.push(initialPos);
    while (!nodes.empty()) {
        node n = nodes.top();
        nodes.pop();
        path.push_back(n);
        if (path.size() == word.size()) {
            break;
        }
        if (cache.find(n) != cache.end()) {
            if (cache[n][k] != word[k]) {
                // find another random direction (somehow)
                while (!nodes.empty()) {
                    cache.erase(nodes.top());
                    nodes.pop();
                }
                path.clear(); // clear path
                possibleDirections &= ~n.dir; // clear direction
                k = 0;
                if (possibleDirections == none)
                    break;
                initialPos.dir = new_direction(possibleDirections);
                nodes.push(initialPos);
                continue;
            }
        }
        k++;
        cache[n] = word; // map node to string
        int x = n.x, y = n.y;
        switch (n.dir) {
        case up:
            nodes.push(node(x - 1, y, n.dir)); break;
        case down:
            nodes.push(node(x + 1, y, n.dir)); break;
        case detail::left:
            nodes.push(node(x, y - 1, n.dir)); break;
        case detail::right:
            nodes.push(node(x, y + 1, n.dir)); break;
        case diag_up_left:
            nodes.push(node(x - 1, y - 1, n.dir)); break;
        case diag_up_right:
            nodes.push(node(x - 1, y + 1, n.dir)); break;
        case diag_down_left:
            nodes.push(node(x + 1, y - 1, n.dir)); break;
        case diag_down_right:
            nodes.push(node(x + 1, y + 1, n.dir)); break;
        }
    }
    k = 0;
    for (auto it = path.begin(); it != path.end(); ++it) {
        board[it->x][it->y] = word[k++];
    }
}

auto find_direction(int len, int x, int y, int boardWidth, int boardHeight) {
    using namespace detail;
    mask dir = default_mask;
    if (x < 0)   dir &= ~(up | down | diag_up_left | diag_up_right | diag_down_left | diag_down_right);
    if (x + len >= boardHeight) dir &= ~(down | diag_down_left | diag_down_right);
    if (x - len < 0) dir &= ~(up | diag_up_left | diag_up_right);
    if (y < 0)   dir &= ~(detail::left | detail::right | diag_up_left | diag_up_right | diag_down_left | diag_down_right);
    if (y + len >= boardWidth) dir &= ~(detail::right | diag_up_right | diag_down_right);
    if (y - len < 0) dir &= ~(detail::left | diag_up_left | diag_down_left);
    return dir;
}

auto crossword(vector<string> const& words, int boardWidth, int boardHeight) {
    using namespace detail;
    auto board = generate_board(boardWidth, boardHeight);
    map<node, string> cache;
    for (auto const& word : words) {
        int len = word.size();
        int x = rand() % boardHeight, y = rand() % boardWidth;
        auto dir = find_direction(len-1, x, y, boardWidth, boardHeight);
        // tighten the indiices to positions where we know they will fit
        if (dir == detail::none) {
            int xRestrict = randchoose(make_array(rand() % (boardHeight - len), rand(len-1, boardHeight-1)));
            int yRestrict = randchoose(make_array(rand() % (boardWidth - len), rand(len-1, boardWidth-1)));

            node pos = randchoose(
                make_array(
                    node(rand() % boardHeight, yRestrict),
                    node(xRestrict, rand() % boardWidth),
                    node(xRestrict, yRestrict)
                )
            );
            tie(x, y) = tie(pos.x, pos.y);
            dir = find_direction(len-1, x, y, boardWidth, boardHeight);
        }

        if (dir != detail::none) {
            node initialPos(x, y);
            output_to_board(board, boardWidth, boardHeight, word, dir, initialPos, cache);
        }
        else {
            cout << "<" << x << "," << y << ">\n";
        }
    }
    return board;
}

int main()
{
    srand((unsigned)time(0));
    vector<string> words = { "crossword", "cry" };
    int n = 10, m = 10;
    auto board = crossword(words, n, m);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            cout << board[i][j] << " ";
        }
        cout << '\n';
    }
    cin.get();
}
