/*
 * Program to solve any level of the game Pushing Machine
 * Abraham Santos amsantosr@gmail.com
 */

#include <vector>
#include <utility>
#include <iostream>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>

using namespace std;

enum Square { Empty, Block };
enum Direction { Up, Down, Left, Right };

struct Position {
    int column, row;
};

struct Machine {
    Position position;
    Direction direction;
};

struct MachineState {
    Position position;
    int length;
};

struct MazeState {
    std::vector<Position> boxes;
    std::vector<MachineState> machines;
};

inline bool operator<(const MazeState &a, const MazeState &b)
{
    return a.boxes != b.boxes ? a.boxes < b.boxes : a.machines < b.machines;
}

inline bool operator==(const Position &a, const Position &b)
{
    return a.column == b.column && a.row == b.row;
}

inline bool operator<(const Position &a, const Position &b)
{
    return a.row != b.row ? a.row < b.row : a.column < b.column;
}

inline bool operator<(const MachineState &a, const MachineState &b)
{
    return a.length != b.length ? a.length < b.length : a.position < b.position;
}

Square maze[8][6]; // the maze of empty or block squares
std::vector<Position> boxes; // start position of the boxes
std::vector<Machine> machines; // start position of machines
std::vector<Position> targets; // position of targets
std::vector<Position> solution; // the solution where each step is a row and column element in the array
const string level_names[] = { "Practice", "Beginner", "Amateur", "Average",
                              "Experienced", "Skilled", "Professional", "Expert",
                              "Genius", "Extreme", "Ultimate", "Impossible" };

void error(const char *msg)
{
    cerr << msg << endl;
    exit(0);
}

void read_input()
{
    cout << " *** PUSHING MACHINE SOLVER by Abraham Santos ***\n";
    cout << "Levels:\n";
    for (int i = 0; i < 12; ++i) {
        cout << i+1 << ". " << level_names[i] << "\n";
    }
    cout << "Enter the number of the level: ";
    int name_num = 0;
    cin >> name_num;
    if (cin.bad() || name_num <= 0 || name_num > 12)
        error("Wrong number read");

    cout << "Which puzzle number do you want to solve? ";
    int puzzle_num = 0;
    cin >> puzzle_num;
    ostringstream oss;
    oss << "./levels/" << level_names[name_num-1] << "/" << puzzle_num;
    ifstream file(oss.str());
    if (!file.is_open())
        error("Level file not available");
    string line;
    getline(file, line);
    getline(file, line);
    for (int i = 0; i < 8; ++i) {
        getline(file, line);
        for (int j = 0; j < 6; ++j) {
            if (line[2*j + 1] == 'T')
                targets.push_back({ j, i });
        }
    }
    getline(file, line);
    for (int i = 0; i < 8; ++i) {
        getline(file, line);
        for (int j = 0; j < 6; ++j) {
            switch (line[2*j + 1]) {
            case '@': maze[i][j] = Block; break;
            case '.': maze[i][j] = Empty; break;
            case 'v':
                machines.push_back({ { j, i }, Down });
                maze[i][j] = Empty;
                break;
            case '^':
                machines.push_back({ { j, i }, Up });
                maze[i][j] = Empty;
                break;
            case '<':
                machines.push_back({ { j, i }, Left });
                maze[i][j] = Empty;
                break;
            case '>':
                machines.push_back({ { j, i }, Right });
                maze[i][j] = Empty;
                break;
            case 'T':
                boxes.push_back({ j, i });
                maze[i][j] = Empty;
                break;
            default:
                error("Invalid character");
            }
        }
    }
}

void create_start_state(MazeState &state)
{
    state.boxes = boxes;
    auto machineCount = machines.size();
    state.machines.resize(machineCount);
    for (unsigned i = 0; i < machineCount; ++i) {
        state.machines[i].position = machines[i].position;
        state.machines[i].length = 1;
    }
}

void front_squares(const MachineState &machine,
                               Direction direction, Position &pos1, Position &pos2)
{
    pos1 = pos2 = machine.position;
    switch (direction) {
    case Up:
        pos1.row -= machine.length;
        pos2.row -= machine.length + 1;
        break;
    case Down:
        pos1.row += machine.length;
        pos2.row += machine.length + 1;
        break;
    case Left:
        pos1.column -= machine.length;
        pos2.column -= machine.length + 1;
        break;
    case Right:
        pos1.column += machine.length;
        pos2.column += machine.length + 1;
        break;
    }
}

bool out_of_limits(const Position &position)
{
    return (position.column < 0 || position.column >= 6) || (position.row < 0 || position.row >= 8);
}

bool is_block(const Position &position)
{
    return maze[position.row][position.column] == Block;
}

bool value_between(int x, int a, int b)
{
    return a <= x && x <= b;
}

bool expanded_machine_placed(const MazeState &state, const Position &position)
{
    auto machineCount = machines.size();
    for (unsigned i = 0; i < machineCount; ++i) {
        auto &machine = state.machines[i];
        switch (machines[i].direction) {
        case Up:
            if (machine.position.column == position.column &&
                    value_between(position.row, machine.position.row - machine.length + 1, machine.position.row))
                return true;
            break;
        case Down:
            if (machine.position.column == position.column &&
                    value_between(position.row, machine.position.row, machine.position.row + machine.length - 1))
                return true;
            break;
        case Left:
            if (machine.position.row == position.row &&
                    value_between(position.column, machine.position.column - machine.length + 1, machine.position.column))
                return true;
            break;
        case Right:
            if (machine.position.row == position.row &&
                    value_between(position.column, machine.position.column, machine.position.column + machine.length - 1))
                return true;
            break;
        }
    }
    return false;
}

bool unexpanded_machine_placed(const MazeState &state,
                                      const Position &position, unsigned *machineIndex)
{
    unsigned index = 0;
    for (const auto &machine : state.machines) {
        if (machine.length == 1 && machine.position == position) {
            if (machineIndex != nullptr)
                *machineIndex = index;
            return true;
        }
        ++index;
    }
    return false;
}

bool box_placed(const MazeState &state, const Position &position, unsigned *boxIndex = nullptr)
{
    auto boxCount = state.boxes.size();
    for (unsigned i = 0; i < boxCount; ++i) {
        if (state.boxes[i] == position) {
            if (boxIndex != nullptr)
                *boxIndex = i;
            return true;
        }
    }
    return false;
}

bool grow_machine(MazeState &state, int machineIndex)
{
    auto &machine = state.machines[machineIndex];
    Position frontalPos1, frontalPos2;
    Direction direction = machines[machineIndex].direction;
    unsigned index;

    front_squares(machine, direction, frontalPos1, frontalPos2);
    if (out_of_limits(frontalPos1) || is_block(frontalPos1))
        return false;
    if (box_placed(state, frontalPos1, &index)) {
        if (out_of_limits(frontalPos2) || is_block(frontalPos2) || box_placed(state, frontalPos2)
                || expanded_machine_placed(state, frontalPos2))
            return false;
        ++machine.length;
        state.boxes[index] = frontalPos2;
        return true;
    }
    if (unexpanded_machine_placed(state, frontalPos1, &index)) {
        if (out_of_limits(frontalPos2) || is_block(frontalPos2) || box_placed(state, frontalPos2)
                || expanded_machine_placed(state, frontalPos2))
            return false;
        ++machine.length;
        state.machines[index].position = frontalPos2;
        return true;
    }
    if (expanded_machine_placed(state, frontalPos1))
        return false;
    ++machine.length;
    return true;
}

bool raise_machine(MazeState &state, int machineIndex)
{
    auto &machineLength = state.machines[machineIndex].length;
    if (machineLength > 1) {
        machineLength = 1;
        return true;
    }
    if (machineLength == 1) {
        while (grow_machine(state, machineIndex));
        return machineLength > 1;
    }
    return false;
}

bool is_solved(const MazeState &state)
{
    for (const auto &box : state.boxes) {
        auto begin = targets.begin();
        auto end = targets.end();

        if (find(begin, end, box) == end) {
            return false;
        }
    }
    return true;
}

bool solve()
{
    typedef map<MazeState, vector<Position>> MapStates;
    MapStates mapStates;
    vector<MazeState> vector1, vector2;
    MazeState start;
    auto machineCount = machines.size();

    create_start_state(start);
    vector1.push_back(start);
    mapStates[start];

    do {
        vector2.clear();
        for (const auto &parentState : vector1) {
            if (is_solved(parentState)) {
                solution = mapStates[parentState];
                return true;
            }
            for (unsigned i = 0; i < machineCount; ++i) {
                MazeState childState = parentState;
                if (raise_machine(childState, i)) {
                    if (mapStates.find(childState) == mapStates.end()) {
                        auto newpath = mapStates[parentState];
                        newpath.push_back(parentState.machines[i].position);
                        mapStates[childState] = newpath;
                        vector2.push_back(childState);
                    }
                }
            }
        }
        vector1.swap(vector2);
    } while (!vector1.empty());
    return false;
}

void show_solution()
{
    cout << "The solution is the following (rows and columns start from 1):\n";
    for (const auto &pos : solution) {
        cout << "Tap machine on row " << pos.row + 1 << " and column " << pos.column + 1 << "\n";
    }
    cout << solution.size() << " moves\n";
}

int main()
{
    read_input();
    cout << "Solving maze...\n";
    if (solve())
        show_solution();
    else
        error("Unable to solve the given puzzle");
    return 0;
}
